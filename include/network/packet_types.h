#pragma once

#include "network/packet.h"

// Should I have automatic HWID?

class HelloPacket : public Packet {
    uint32_t _hwid = 0;
public:
    const static uint8_t PACKET_TYPE = 0x01;

    HelloPacket() = default;
    ~HelloPacket() override = default;

    uint8_t type() override         { return PACKET_TYPE; };
    size_t size() override          { return sizeof(uint32_t); };

    uint32_t hwid() const           { return _hwid; }
    void hwid(uint32_t id)          { _hwid = id; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};

class Preved : public Packet {
    uint32_t _hwid = 0;
public:
    const static uint8_t PACKET_TYPE = 0x02;
    Preved() = default;
    ~Preved() override = default;

    uint32_t hwid() const           { return _hwid; }
    void hwid(uint32_t id)          { _hwid = id; }

    uint8_t type() override { return PACKET_TYPE; };
    size_t size() override { return sizeof(uint32_t); };

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};

class Medved : public Packet {
    uint32_t _hwid = 0;
    String _mcu{};
    int8_t _rssi = -127;
    int8_t _snr = -127;
public:
    const static uint8_t PACKET_TYPE = 0x03;
    Medved() = default;
    ~Medved() override = default;

    uint8_t type() override { return PACKET_TYPE; };
    size_t size() override { return sizeof(uint32_t) + _mcu.length()+1 + sizeof(int8_t)*2; };

    uint32_t hwid() const           { return _hwid; }
    void hwid(uint32_t id)          { _hwid = id; }
    String mcu() const              { return _mcu; }
    void mcu(String m)             { _mcu = m; }
    int8_t rssi() const             { return _rssi; }
    void rssi(int8_t r)             { _rssi = r; }
    int8_t snr() const              { return _snr; }
    void snr(int8_t s)              { _snr = s; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};
