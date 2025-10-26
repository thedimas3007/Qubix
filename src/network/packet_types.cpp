#include "network/packet_types.h"

void HelloPacket::serialize(WriteBuffer& buffer) {
    buffer.u8(type());
    buffer.u32(hwid());
}

void HelloPacket::deserialize(ReadBuffer& buffer) {
    hwid(buffer.u32());
}


namespace {
    const bool _registered = [](){
        Packet::registerType(HelloPacket::PACKET_TYPE, [](){ return new HelloPacket(); });
        return true;
    }();
}