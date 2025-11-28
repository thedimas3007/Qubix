#include "network/packet.h"

#include "configuration.h"
#include "utils.h"
#include "network/packet_types.h"

void NetManager::begin(SX126x* r, volatile bool* en, volatile bool* rx) {
    this->radio = r;
    this->irq_en = en;
    this->irq_rx = rx;

    auto pkt = std::make_unique<NeighborLocate>();
    request<NeighborResponse>(std::move(pkt), [](auto& manager, auto& packet) {
        Serial.println(stringf("<> Discovered: 0x%08lX - %s", packet.sender(), packet.mcu().c_str()));
        manager.path_cache.put(packet.sender(), {1, {packet.sender()}});
    }, [](bool success) {
        if (!success) {
            Serial.println("!! No neighbors found");
        }
    }, 0xFFFFFFFF, 5000);

    reg<NeighborLocate>([](auto& manager, const auto& packet) {
        // TODO: RX stats for the packet: rssi and snr
        auto pkt = std::make_unique<NeighborResponse>();
        pkt->mcu(driver->mcu());
        pkt->rssi(std::clamp<float>(manager.radio->getRSSI(), -128, 127));
        pkt->snr(std::clamp<float>(manager.radio->getSNR(), -128, 127));
        manager.queue(std::move(pkt), packet.sender());
    });

    reg<NodeFound>([](auto& manager, const auto& packet) {
        uint32_t board = driver->boardId();

        int16_t pos = -1;

        for (uint8_t i = 0; i < packet.pathLength(); i++) {
            if (packet.path()[i] == board) {
                pos = i;
                break;
            }
        }

        if (pos == -1) return;

        Path p{static_cast<uint8_t>(packet.pathLength() - pos - 1)};
        for (uint8_t i = pos+1; i < packet.pathLength(); i++) {
            p.path[i-pos-1] = packet.path()[i];
        }
        manager.path_cache.put(packet.path().back(), p);

        if (pos == 0) return;

        auto pkt = std::make_unique<NodeFound>(packet);
        uint32_t next_hop = packet.path()[pos - 1];
        manager.queue(std::move(pkt), next_hop);
    });

    reg<NodeLocate>([](auto& manager, const auto& packet) {
        uint32_t board = driver->boardId();
        if (packet.node() == board) {
            auto pkt = std::make_unique<NodeFound>();
            pkt->node(board);
            pkt->path(packet.path());
            pkt->addHop(board);
            manager.queue(std::move(pkt), packet.sender());
        } else {
            if (!packet.pathLength()) return;
            for (uint8_t i = 0; i < packet.pathLength(); i++) {
                if (packet.path()[i] == board) return;
            }

            auto pkt = std::make_unique<NodeLocate>(packet);
            pkt->addHop(board);
            manager.queue(std::move(pkt), 0xFFFFFFFF);
        }
    });
}

void NetManager::queue(std::unique_ptr<Packet> packet, uint32_t target, int8_t priority) {
    if (packet->hops() >= MAX_HOPS) return;
    packet_queue.push({std::move(packet), target, priority});
}

int16_t NetManager::send(Packet& packet) const {
    if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }
    uint32_t packet_id = packet.pktid() ? packet.pktid() : static_cast<uint32_t>(random(0, 0x7FFFFFFF)) << 1 ^ micros();

    Serial.println(stringf("<< TX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX",
        packet_names[packet.type()].c_str(), packet_id, packet.hops()+1, packet.size(), packet.current() ? packet.current() : driver->boardId(), packet.target()));

    *irq_en = false;

    WriteBuffer buffer = WriteBuffer(packet.size());
    buffer.u32(packet_id);
    buffer.u8(packet.type());
    buffer.u8(packet.hops()+1 & 0xF << packet.pathLength() & 0xF); // TODO: assert
    for (uint32_t id : packet.path()) buffer.u32(id);

    packet.serialize(buffer);
    int16_t status = radio->transmit(buffer.raw(), buffer.len());
    *irq_en = true;
    // *irq_rx = false;
    radio->startReceive();
    return status;
}

void NetManager::received() {
    size_t len = radio->getPacketLength();
    auto data = std::make_unique<uint8_t[]>(len);
    radio->readData(data.get(), 0);
    ReadBuffer buffer = ReadBuffer(data.get(), len);

    uint32_t packet_id = buffer.u32();
    uint8_t packet_type = buffer.u8();
    uint8_t hop_info = buffer.u8();
    uint8_t hops = hop_info >> 4;
    uint8_t path_length = hop_info & 0xF;

    std::vector<uint32_t> path{path_length};
    for (uint8_t i = 0; i < path_length; i++) path[i] = buffer.u32();

    auto packet = Packet::create(packet_type);

    if (!packet) {
        radio->startReceive();
        return;
    }

    packet->pktid(packet_id);
    packet->hops(hops);
    packet->path(path);
    packet->deserialize(buffer);

    if (packet->sender() == driver->boardId() || seen(packet->sender(), packet_id) ) {
        radio->startReceive();
        return;
    }

    if (last_packets.size() == 32) last_packets.pop_front();
    last_packets.push_back({packet->sender(), packet_id});

    Serial.println(stringf(">> RX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX",
        packet_names[packet_type].c_str(), packet_id, hops, len, packet->sender(), packet->current()));

    if ((packet->current() != driver->boardId() && packet->current() != 0xFFFFFFFF) || hops > MAX_HOPS) {
        radio->startReceive();
        return;
    };

    if (!isWaiting()) {
        uint32_t hwid = driver->boardId();
        uint16_t base_jitter = ((hwid >> 8) & 0xFF) * 3;
        uint32_t jitter_delay = 0;

        if (packet_type == NodeLocate::PACKET_TYPE) {
            jitter_delay = random(200, 600) + base_jitter + (hops * 50);
        } else if (packet_type == NodeFound::PACKET_TYPE) {
            jitter_delay = random(150, 400) + base_jitter + ((MAX_HOPS - hops) * 30);
        } else if (packet->current() == 0xFFFFFFFF) {
            jitter_delay = random(150, 800) + base_jitter;
        }

        if (jitter_delay > 0) {
            wait(jitter_delay);
        }
    }

    dispatch(*packet);
    radio->startReceive();
}

void NetManager::dispatch(Packet& p) {
    auto it = listeners.find(p.type());
    if (it == listeners.end()) return;

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
    for (auto& [type, vec] : listeners) {
        for (auto it = vec.begin(); it != vec.end();) {
            if (it->temporary && millis() > it->ttl) {
                it->timeout(it->received);
                it = vec.erase(it);
            } else ++it;
        }
    }

    if (!radio) return RADIOLIB_ERR_NULL_POINTER;
    if (packet_queue.empty() || isWaiting()) return 0;


    uint32_t duration = random(100, 250);
    uint32_t start = millis();
    bool busy = false;

    while (millis() < start+duration) {
        float rssi = radio->getRSSI(false);
        // Serial.println(stringf("## RSSI: %.1f dBm", rssi));
        if (rssi >= -85) {
            // Serial.println(stringf("## Activity detected, %.1fdBm", rssi));
            busy = true;
            break;
        }

        int16_t ch_status = radio->scanChannel();

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
        uint16_t backoff = min<uint16_t>(base_delay * (1 << min<uint16_t>(retries, 4)), max_delay);
        uint8_t jitter = random(0, backoff / 2);
        uint16_t total_delay = backoff + jitter;

        retries++;
        wait(total_delay);
        // Serial.println(stringf("## Postpone #%i, +%i ms", retries, total_delay));
        return 0;
    }

    auto pkt = std::move(const_cast<PendingPacket&>(packet_queue.top()));
    packet_queue.pop();

    timed_out = 0;
    retries = 0; // TODO: per-packet retries

    int16_t status = send(*pkt.packet);
    if (status != RADIOLIB_ERR_NONE) {
        Serial.println(stringf("!! TX failed: %i, re-queuing", status));

        // increased priority since the packet wasn't sent
        // ch-hopping maybe? or not, since nodes have their places
        if (pkt.priority < 127) pkt.priority++;
        packet_queue.push(std::move(pkt));
        wait(100);
        retries = 1;
    }

    return status;
}