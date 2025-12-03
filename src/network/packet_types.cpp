#include "network/packet_types.h"

void HelloPacket::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(current());
}

void HelloPacket::deserialize(ReadBuffer& buffer) {}


void Ping::serialize(WriteBuffer& buffer) {}

void Ping::deserialize(ReadBuffer& buffer) {}


void Pong::serialize(WriteBuffer& buffer) {
    buffer.str(mcu());
}

void Pong::deserialize(ReadBuffer& buffer) {
    mcu(buffer.str());
}


void NodeLocate::serialize(WriteBuffer& buffer) {
    buffer.u32(node());
}
void NodeLocate::deserialize(ReadBuffer& buffer) {
    node(buffer.u32());
}


void NodeFound::appendInfo(float r, float s) {
    rssiAvg(rssiAvg() + (r - rssiAvg()) / hops());
    snrAvg(snrAvg() + (s - snrAvg()) / hops());
}

void NodeFound::serialize(WriteBuffer& buffer) {
    buffer.f32(rssiAvg());
    buffer.f32(snrAvg());
}

void NodeFound::deserialize(ReadBuffer& buffer) {
    rssiAvg(buffer.f32());
    snrAvg(buffer.f32());
}

// TODO: find a way to make it look not that ugly
namespace {
    const bool _registered = [](){
        Packet::registerType(HelloPacket::PACKET_TYPE, [](){ return std::make_unique<HelloPacket>(); });
        Packet::registerType(Ping::PACKET_TYPE, [](){ return std::make_unique<Ping>(); });
        Packet::registerType(Pong::PACKET_TYPE, [](){ return std::make_unique<Pong>(); });
        Packet::registerType(NodeLocate::PACKET_TYPE, [](){ return std::make_unique<NodeLocate>(); });
        Packet::registerType(NodeFound::PACKET_TYPE, [](){ return std::make_unique<NodeFound>(); });
        return true;
    }();
}
