#include "network/packet_types.h"

void HelloPacket::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(hwid());
}

void HelloPacket::deserialize(ReadBuffer& buffer) {
    hwid(buffer.u32());
}


void Preved::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(hwid());
}

void Preved::deserialize(ReadBuffer& buffer) {
    hwid(buffer.u32());
}


void Medved::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(hwid());
    buffer.str(mcu());
    buffer.i8(rssi());
    buffer.i8(snr());
}

void Medved::deserialize(ReadBuffer& buffer) {
    hwid(buffer.u32());
    mcu(buffer.str());
    rssi(buffer.i8());
    snr(buffer.i8());
}


// TODO: find a way to make it look not that ugly
namespace {
    const bool _registered = [](){
        Packet::registerType(HelloPacket::PACKET_TYPE, [](){ return std::make_unique<HelloPacket>(); });
        Packet::registerType(Preved::PACKET_TYPE, [](){ return std::make_unique<Preved>(); });
        Packet::registerType(Medved::PACKET_TYPE, [](){ return std::make_unique<Medved>(); });
        return true;
    }();
}
