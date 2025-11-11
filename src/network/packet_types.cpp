#include "network/packet_types.h"

void HelloPacket::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(hwid());
}

void HelloPacket::deserialize(ReadBuffer& buffer) {}


void Preved::serialize(WriteBuffer& buffer) {}

void Preved::deserialize(ReadBuffer& buffer) {}


void Medved::serialize(WriteBuffer& buffer) {
    buffer.str(mcu());
    buffer.i8(rssi());
    buffer.i8(snr());
}

void Medved::deserialize(ReadBuffer& buffer) {
    mcu(buffer.str());
    rssi(buffer.i8());
    snr(buffer.i8());
}

void NodeLocate::serialize(WriteBuffer& buffer) {
    buffer.u32(node());
    buffer.u8(pathLength());
    for (uint8_t i = 0; i < pathLength(); i++) buffer.u32(path()[i]);
}

void NodeLocate::deserialize(ReadBuffer& buffer) {
    node(buffer.u32());
    uint8_t l = buffer.u8();
    std::vector<uint32_t> p(l);
    for (uint8_t i = 0; i < l; i++) p.at(i) = buffer.u32();
    path(p);
}

void NodeFound::serialize(WriteBuffer& buffer) {
    buffer.u32(node());
    buffer.u8(pathLength());
    for (uint8_t i = 0; i < pathLength(); i++) buffer.u32(path()[i]);
}

void NodeFound::deserialize(ReadBuffer& buffer) {
    node(buffer.u32());
    uint8_t l = buffer.u8();
    std::vector<uint32_t> p(l);
    for (uint8_t i = 0; i < l; i++) p.at(i) = buffer.u32();
    path(p);
}

// TODO: find a way to make it look not that ugly
namespace {
    const bool _registered = [](){
        Packet::registerType(HelloPacket::PACKET_TYPE, [](){ return std::make_unique<HelloPacket>(); });
        Packet::registerType(Preved::PACKET_TYPE, [](){ return std::make_unique<Preved>(); });
        Packet::registerType(Medved::PACKET_TYPE, [](){ return std::make_unique<Medved>(); });
        Packet::registerType(NodeLocate::PACKET_TYPE, [](){ return std::make_unique<NodeLocate>(); });
        Packet::registerType(NodeFound::PACKET_TYPE, [](){ return std::make_unique<NodeFound>(); });
        return true;
    }();
}
