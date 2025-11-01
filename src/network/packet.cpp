#include "network/packet.h"

void NetManager::begin(PhysicalLayer* r) {
    this->radio = r;
    // this->irq_en = en;
}

void NetManager::queue(std::unique_ptr<Packet> packet, int8_t priority) {
    packet_queue.push({std::move(packet), priority});
}

int16_t NetManager::send(Packet& packet) {
    if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }

    irq_en = false;
    // noInterrupts();
    WriteBuffer buffer = WriteBuffer(packet.size()+1);
    packet.serialize(buffer);
    int16_t status = radio->transmit(buffer.raw(), buffer.len());
    irq_en = true;
    // interrupts();
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
    if (packet_queue.empty()) return 0;

    auto pkt = std::move(const_cast<PendingPacket&>(packet_queue.top()));
    if (!pkt.packet) {
        packet_queue.pop();
        return 0;
    }

    // TODO: implement RSSI scan, aka LBT
    int16_t ch_status = radio->scanChannel();
    if (ch_status != RADIOLIB_CHANNEL_FREE) return ch_status;
    // ^^^ including RADIOLIB_LORA_DETECTED

    int16_t status = send(*pkt.packet);
    packet_queue.pop();
    return status;
}

void NetManager::irq() {
    if (irq_en) received = true;
}

bool NetManager::available() {
    return received;
}

void NetManager::clearState() {
    irq_en = true;
    received = false;
}
