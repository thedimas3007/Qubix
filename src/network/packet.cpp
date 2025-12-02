#include "network/packet.h"

#include "configuration.h"
#include "utils.h"
#include "network/packet_types.h"

void NetManager::begin(SX126x* r, volatile bool* en, volatile bool* rx) {
    this->_radio = r;
    this->_irq_en = en;
    this->_irq_rx = rx;

    auto pkt = std::make_unique<Ping>();
    request<Pong>(std::move(pkt), [](auto& manager, auto& packet) {
        Serial.println(stringf("<> Discovered: 0x%08lX - %s", packet.sender(), packet.mcu().c_str()));
        manager._path_cache.put(packet.sender(), {1, {packet.sender()}});
    }, [](bool success) {
        if (!success) {
            Serial.println("!! No neighbors found");
        }
    }, 0xFFFFFFFF, 5000);

    reg<Ping>([](auto& manager, const auto& packet) {
        // TODO: RX stats for the packet: rssi and snr
        auto pkt = std::make_unique<Pong>();
        pkt->mcu(driver->mcu());

        auto path = packet.path();
        if (path.back() == 0xFFFFFFFF) {
            path.pop_back();
            path.push_back(driver->boardId());
        }
        std::reverse(path.begin(), path.end());
        pkt->path(path);
        manager.queueDirect(std::move(pkt));
    });


    reg<NodeFound>([](auto& manager, const auto& packet) {
        int8_t pos = -1;

        auto trace = packet.path();
        trace.pop_back();
        std::reverse(trace.begin(), trace.end());

        Path path;
        for (uint32_t u : trace) path.push(u);

        manager._path_cache.put(packet.path().front(), path);
    });

    reg<NodeLocate>([](auto& manager, const auto& packet) {
        uint32_t board = driver->boardId();
        if (packet.node() == board) {
            auto pkt = std::make_unique<NodeFound>();
            auto path = packet.path();
            path.pop_back();
            path.push_back(board);
            std::reverse(path.begin(), path.end());
            pkt->path(path);
            manager.queueDirect(std::move(pkt), 1);
        } else {
            if (!packet.pathLength()) return;
            for (uint8_t i = 0; i < packet.pathLength(); i++) {
                if (packet.path()[i] == board) return;
            }

            auto pkt = std::make_unique<NodeLocate>(packet);
            auto path = packet.path();
            path.pop_back();
            path.push_back(board);
            path.push_back(0xFFFFFFFF);
            pkt->path(path);
            manager.queueDirect(std::move(pkt), 1);
        }
    });
}

void NetManager::queueDirect(std::shared_ptr<Packet> packet, int8_t priority) {
    _packet_queue.push({std::move(packet), priority});
}

void NetManager::queue(std::shared_ptr<Packet> packet, uint32_t target, int8_t priority) {
    if (packet->hops() >= MAX_HOPS) return;

    if (target == 0xFFFFFFFF) {
        packet->path({driver->boardId(), 0xFFFFFFFF});
        queueDirect(std::move(packet), priority);
        return;
    }

    locate(target, [this, packet, priority](const Path& path) mutable {
        if (!path.hops) return;

        std::vector<uint32_t> ids(path.hops + 1);
        ids[0] = driver->boardId();
        for (uint8_t i = 0; i < path.hops; i++) {
            ids[i + 1] = path.path[i];
        }
        packet->path(ids);
        _packet_queue.push({std::move(packet), priority});
    });
}

int16_t NetManager::send(Packet& packet) {
    if (!_radio || !_irq_en) { return RADIOLIB_ERR_NULL_POINTER; }
    uint32_t packet_id = packet.packetId() ? packet.packetId() : static_cast<uint32_t>(random(0, 0x7FFFFFFF)) << 1 ^ micros();

    Serial.println(stringf("<< TX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX",
        packet_names[packet.type()].c_str(), packet_id, packet.hops()+1, packet.size(), packet.current() ? packet.current() : driver->boardId(), packet.target()));

    *_irq_en = false;

    WriteBuffer buffer = WriteBuffer(packet.size());
    buffer.u32(packet_id);
    buffer.u8(packet.type());
    buffer.u8(((packet.hops()+1 & 0xF) << 4) | (packet.pathLength() & 0xF)); // TODO: assert
    for (uint32_t id : packet.path()) buffer.u32(id);

    packet.serialize(buffer);
    int16_t status = _radio->transmit(buffer.raw(), buffer.len());
    *_irq_en = true;
    // *irq_rx = false;
    _radio->startReceive();

    _bytes_tx += buffer.len();
    _packets_tx++;

    return status;
}

void NetManager::received() {
    size_t len = _radio->getPacketLength();
    auto data = std::make_unique<uint8_t[]>(len);
    _radio->readData(data.get(), 0);
    ReadBuffer buffer = ReadBuffer(data.get(), len);

    uint32_t packet_id = buffer.u32();
    uint8_t packet_type = buffer.u8();
    uint8_t hop_info = buffer.u8();
    uint8_t hops = hop_info >> 4;
    uint8_t path_length = hop_info & 0xF;

    std::vector<uint32_t> path(path_length);
    for (uint8_t i = 0; i < path_length; i++) path[i] = buffer.u32();

    auto packet = Packet::create(packet_type);

    if (!packet) {
        _radio->startReceive();
        return;
    }

    packet->rssi(_radio->getRSSI());
    packet->snr(_radio->getSNR());

    packet->packetId(packet_id);
    packet->hops(hops);
    packet->path(path);
    packet->deserialize(buffer);

    if (packet->sender() == driver->boardId() || seen(packet->sender(), packet_id) ) {
        _radio->startReceive();
        return;
    }

    if (_last_packets.size() == 32) _last_packets.pop_front();
    _last_packets.push_back({packet->sender(), packet_id});

    if (!_path_cache.contains(packet->sender())) {
        _path_cache.put(packet->sender(), {1, {packet->sender()}});
    } else {
        _path_cache.refresh(packet->sender());
    }
    _last_rssi.put(packet->sender(), packet->rssi());

    Serial.println(stringf(">> RX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX | %.1fdBm | %.1fdB",
        packet_names[packet_type].c_str(), packet_id, hops, len, packet->sender(), packet->current(), packet->rssi(), packet->snr()));

    if ((packet->current() != driver->boardId() && !packet->isBroadcast()) || hops > MAX_HOPS) {
        _radio->startReceive();
        return;
    };

    _bytes_rx += packet->size();
    _packets_rx++;

    if (!isWaiting()) {
        uint32_t hwid = driver->boardId();
        uint16_t base_jitter = ((hwid >> 8) & 0xFF) * 3;
        uint32_t jitter_delay = 0;

        if (packet_type == NodeLocate::PACKET_TYPE) {
            jitter_delay = random(200, 600) + base_jitter + (hops * 50);
        } else if (packet_type == NodeFound::PACKET_TYPE) {
            jitter_delay = random(150, 400) + base_jitter + ((MAX_HOPS - hops) * 30);
        } else if (packet->isBroadcast()) {
            jitter_delay = random(150, 800) + base_jitter;
        }

        if (jitter_delay > 0) {
            wait(jitter_delay);
        }
    }

    if (!packet->isEnd()) {
        if (packet->hops() < MAX_HOPS) queueDirect(std::move(packet));
    } else {
        dispatch(*packet);
    }
    _radio->startReceive();
}

void NetManager::dispatch(Packet& p) {
    auto it = _listeners.find(p.type());
    if (it == _listeners.end()) return;

    auto& vec = it->second;
    for (auto iter = vec.begin(); iter != vec.end();) {
        if (iter->temporary && millis() > iter->ttl) {
            ++iter;
            continue; // could theoretically be timed-out before being deleted
        }

        iter->listener(*this, p);
        iter->received = true;
        ++iter;
    }
}

int16_t NetManager::tick() {
    for (auto& [type, vec] : _listeners) {
        for (auto it = vec.begin(); it != vec.end();) {
            if (it->temporary && millis() > it->ttl) {
                it->timeout(it->received);
                it = vec.erase(it);
            } else ++it;
        }
    }

    for (auto& [target, vec] : _path_listeners) {
        Path p;
        if (_path_cache.contains(target)) { p = _path_cache.at(target); }

        for (auto it = vec.begin(); it != vec.end();) {
            if (millis() > it->ttl) {
                it->listener(p);
                it = vec.erase(it);
            } else ++it;
        }
    }

    if (!_radio) return RADIOLIB_ERR_NULL_POINTER;
    if (_packet_queue.empty() || isWaiting()) return 0;

    uint32_t duration = random(100, 250);
    uint32_t start = millis();
    bool busy = false;

    while (millis() < start+duration) {
        float rssi = _radio->getRSSI(false);
        // Serial.println(stringf("## RSSI: %.1f dBm", rssi));
        if (rssi >= -85) {
            // Serial.println(stringf("## Activity detected, %.1fdBm", rssi));
            busy = true;
            break;
        }

        int16_t ch_status = _radio->scanChannel();

        if (ch_status != RADIOLIB_CHANNEL_FREE && ch_status != RADIOLIB_LORA_DETECTED) {
            return ch_status;
        }

        if (ch_status == RADIOLIB_LORA_DETECTED) {
            // Serial.println("## LoRa detected");
            busy = true;
            break;
        }
    }

    if (busy) {
        uint16_t base_delay = 30;
        uint16_t max_delay = 500;
        uint16_t backoff = min<uint16_t>(base_delay * (1 << min<uint16_t>(_retries, 4)), max_delay);
        uint8_t jitter = random(0, backoff / 2);
        uint16_t total_delay = backoff + jitter;

        _retries++;
        wait(total_delay);
        // Serial.println(stringf("## Postpone #%i, +%i ms", retries, total_delay));
        return 0;
    }

    auto pkt = std::move(const_cast<PendingPacket&>(_packet_queue.top()));
    _packet_queue.pop();

    _timed_out = 0;
    _retries = 0; // TODO: per-packet retries

    int16_t status = send(*pkt.packet);
    if (status != RADIOLIB_ERR_NONE) {
        Serial.println(stringf("!! TX failed: %i, re-queuing", status));

        // increased priority since the packet wasn't sent
        // ch-hopping maybe? or not, since nodes have their places
        if (pkt.priority < 127) pkt.priority++;
        _packet_queue.push(std::move(pkt));
        wait(100);
        _retries = 1;
    }

    return status;
}

void NetManager::locate(uint32_t target, std::function<void(Path& path)> callback, uint32_t timeout_ms) {
    if (_path_cache.contains(target)) {
        callback(_path_cache.at(target));
        return;
    }

    if (!_path_listeners.count(target)) {
        _path_listeners[target] = {};
    }
    _path_listeners.at(target).push_back({std::move(callback), millis() + timeout_ms});

    auto pkt = std::make_unique<NodeLocate>();
    pkt->node(target);
    queue(std::move(pkt), 0xFFFFFFFF, 1);
}

CacheMap<uint32_t, NetManager::Path>& NetManager::cache() {
    return _path_cache;
}

float NetManager::avgRssi() {
    if (_last_rssi.size() == 0) return -140;
    float sum = 0;
    for (auto [_, rssi] : _last_rssi) { sum+=rssi; }
    return sum / _last_rssi.size();
}

float NetManager::avgScore() {
    float avg = avgRssi();
    if (avg <= -120) return 0;
    if (avg >= -30) return 100;

    return (avg+120) / 90 * 100;
}
