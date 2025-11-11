#include "network/packet.h"

#include "configuration.h"
#include "utils.h"
#include "network/packet_types.h"

void NetManager::begin(PhysicalLayer* r, volatile bool* en, volatile bool* rx) {
    this->radio = r;
    this->irq_en = en;
    this->irq_rx = rx;
}

void NetManager::queue(std::unique_ptr<Packet> packet, uint32_t target, int8_t priority) {
    if (packet->hops() >= MAX_HOPS) return;
    packet_queue.push({std::move(packet), target, priority});
}

int16_t NetManager::send(Packet& packet, uint32_t target = 0xFFFFFFFF) const {
    if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }
    uint32_t packet_id = packet.pktid() ? packet.pktid() : static_cast<uint32_t>(random(0, 0x7FFFFFFF)) << 1 ^ micros();

    Serial.println(stringf("<< TX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX",
        packet_names[packet.type()].c_str(), packet_id, packet.hops()+1, packet.size(), packet.hwid() ? packet.hwid() : driver->boardId(), target));

    *irq_en = false;
    WriteBuffer buffer = WriteBuffer(packet.size());
    buffer.u32(packet_id);
    buffer.u32(driver->boardId());
    buffer.u32(target);
    buffer.u8(packet.type());
    buffer.u8(packet.hops()+1);
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
    uint32_t sender = buffer.u32();
    uint32_t target = buffer.u32();
    uint8_t packet_type = buffer.u8();
    uint8_t hops = buffer.u8();

    if (sender == driver->boardId() || seen(sender, packet_id) ) {
        radio->startReceive();
        return;
    }

    if (last_packets.size() == 32) last_packets.pop_front();
    last_packets.push_back({sender, packet_id});

    Serial.println(stringf(">> RX $%s #%08lX | %d hops | %d bytes | 0x%08lX -> 0x%08lX",
        packet_names[packet_type].c_str(), packet_id, hops, len, sender, target));

    if ((target != driver->boardId() && target != 0xFFFFFFFF) || hops > MAX_HOPS) {
        radio->startReceive();
        return;
    };

    if (!isTimedOut()) {
        uint32_t hwid = driver->boardId();
        uint16_t base_jitter = ((hwid >> 8) & 0xFF) * 3;
        uint32_t jitter_delay = 0;

        if (packet_type == NodeLocate::PACKET_TYPE) {
            jitter_delay = random(200, 600) + base_jitter + (hops * 50);
        } else if (packet_type == NodeFound::PACKET_TYPE) {
            jitter_delay = random(150, 400) + base_jitter + ((MAX_HOPS - hops) * 30);
        } else if (target == 0xFFFFFFFF) {
            jitter_delay = random(150, 800) + base_jitter;
        }

        if (jitter_delay > 0) {
            wait(jitter_delay);
        }
    }

    if (auto packet = Packet::create(packet_type)) {
        packet->pktid(packet_id);
        packet->hwid(sender);
        packet->target(target);
        packet->hops(hops);
        packet->deserialize(buffer);
        dispatch(*packet);
    }

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
    if (packet_queue.empty() || isTimedOut()) return 0;


    uint32_t duration = random(100, 250);
    uint32_t start = millis();
    bool busy = false;

    while (millis() - start < duration) {
        int16_t ch_status = radio->scanChannel();

        if (ch_status != RADIOLIB_CHANNEL_FREE && ch_status != RADIOLIB_LORA_DETECTED) {
            return ch_status;
        }

        if (ch_status == RADIOLIB_LORA_DETECTED) {
            busy = true;
            break;
        }
    }

    // TODO: implement RSSI scan, aka LBT

    if (busy) {
        uint16_t base_delay = 30;
        uint16_t max_delay = 500;
        uint16_t backoff = min<uint16_t>(base_delay * (1 << min<uint16_t>(retries, 4)), max_delay);
        uint8_t jitter = random(0, backoff / 2);
        uint16_t total_delay = backoff + jitter;

        retries++;
        wait(total_delay);
        Serial.println(stringf("## Postpone #%i, +%i ms", retries, total_delay));
        return 0;
    }

    auto pkt = std::move(const_cast<PendingPacket&>(packet_queue.top()));
    packet_queue.pop();

    timed_out = 0;
    retries = 0; // TODO: per-packet retries

    int16_t status = send(*pkt.packet, pkt.target);
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