#include "network/packet.h"

#include "configuration.h"

void NetManager::begin(PhysicalLayer* r, volatile bool* en, volatile bool* rx) {
    this->radio = r;
    this->irq_en = en;
    this->irq_rx = rx;
}

void NetManager::queue(std::unique_ptr<Packet> packet, uint32_t target, int8_t priority) {
    packet_queue.push({std::move(packet), target, priority});
}

int16_t NetManager::send(Packet& packet, uint32_t target = 0xFFFFFFFF) const {
    if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }
    uint32_t packet_id = static_cast<uint32_t>(random(0, 0x7FFFFFFF)) << 1 ^ micros();

    Serial.print("TX #");
    Serial.print(packet_id);
    Serial.print(", ");
    Serial.print(packet.type());
    Serial.print("@");
    Serial.print(packet.size());
    Serial.print(" bytes | 0x");
    Serial.print(packet.hwid(), HEX);
    Serial.print(" -> 0x");
    Serial.println(target, HEX);

    *irq_en = false;
    WriteBuffer buffer = WriteBuffer(packet.size());
    buffer.u32(packet_id);
    buffer.u32(driver->boardId());
    buffer.u32(target);
    buffer.u8(packet.type());
    packet.serialize(buffer);
    int16_t status = radio->transmit(buffer.raw(), buffer.len());
    *irq_en = true;
    // *irq_rx = false;
    radio->startReceive();
    return status;
}

void NetManager::dispatch(Packet& p) {
    auto it = listeners.find(p.type());
    if (it != listeners.end()) {
        for (auto& f : it->second) f(*this, p);
    }
}

int16_t NetManager::tick() {
    if (!radio) return RADIOLIB_ERR_NULL_POINTER;
    if (packet_queue.empty() || isTimedOut()) return 0;

    // TODO: implement RSSI scan, aka LBT
    int16_t ch_status = radio->scanChannel();
    if (ch_status != RADIOLIB_CHANNEL_FREE && ch_status != RADIOLIB_LORA_DETECTED) return ch_status;

    if (ch_status == RADIOLIB_LORA_DETECTED) {
        retries++;

        uint8_t ms = random(20, 60);
        wait(ms);
        Serial.print("Postpone #");
        Serial.print(retries);
        Serial.print(" +");
        Serial.print(ms);
        Serial.println(" ms");

        return 0;
    }

    auto pkt = std::move(const_cast<PendingPacket&>(packet_queue.top()));
    packet_queue.pop();

    timed_out = 0;
    retries = 0; // TODO: per-packet retries

    int16_t status = send(*pkt.packet, pkt.target);
    if (status != RADIOLIB_ERR_NONE) {
        Serial.print("TX failed ");
        Serial.print(status);
        Serial.println(", re-queuing");

        // increased priority since the packet wasn't sent
        if (pkt.priority < 127) pkt.priority++;
        packet_queue.push(std::move(pkt));
        wait(100);
        retries = 1;
    }

    return status;
}