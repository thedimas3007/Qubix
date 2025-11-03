#include "network/packet.h"

void NetManager::begin(PhysicalLayer* r, volatile bool* en, volatile bool* rx) {
    this->radio = r;
    this->irq_en = en;
    this->irq_rx = rx;
}

void NetManager::queue(std::unique_ptr<Packet> packet, int8_t priority) {
    packet_queue.push({std::move(packet), priority});
}

int16_t NetManager::send(Packet& packet) {
    if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }

    *irq_en = false;
    WriteBuffer buffer = WriteBuffer(packet.size()+1);
    packet.serialize(buffer);
    int16_t status = radio->transmit(buffer.raw(), buffer.len());
    *irq_en = true;
    *irq_rx = false;
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

    auto pkt = std::move(const_cast<PendingPacket&>(packet_queue.top()));
    if (!pkt.packet) {
        packet_queue.pop();
        return 0;
    }

    // TODO: implement RSSI scan, aka LBT
    int16_t ch_status = radio->scanChannel();
    if (ch_status != RADIOLIB_CHANNEL_FREE && ch_status != RADIOLIB_LORA_DETECTED) return ch_status;

    if (ch_status == RADIOLIB_LORA_DETECTED) {
        timed_out = millis() + (++retries)*25;
        if (retries >= 5) return RADIOLIB_LORA_DETECTED;
        return 0;
    }

    timed_out = 0; retries = 0;
    int16_t status = send(*pkt.packet);
    if (!status) packet_queue.pop();
    return status;
}
