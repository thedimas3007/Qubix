#pragma once

#include "network/packet.h"

class HelloPacket : public Packet {
    uint32_t _hwid = 0;
public:
    const static uint8_t PACKET_TYPE = 0x01;

    HelloPacket() {}
    ~HelloPacket() override {}

    uint8_t type() override         { return PACKET_TYPE; };
    size_t size() override          { return sizeof(uint32_t); };

    uint32_t hwid() const           { return _hwid; }
    void hwid(uint32_t id)          { _hwid = id; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};
