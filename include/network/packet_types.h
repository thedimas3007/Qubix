#pragma once

#include "configuration.h"
#include "network/packet.h"

class [[deprecated("Use Ping instead")]] HelloPacket : public Packet {
protected:
    size_t localSize() override { return 0; }
public:
    const static uint8_t PACKET_TYPE = 0x01;

    HelloPacket() = default;
    ~HelloPacket() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};


class Ping : public Packet {
protected:
    size_t localSize() override { return 0; }
public:
    const static uint8_t PACKET_TYPE = 0x02;
    Ping() = default;
    ~Ping() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};


class Pong : public Packet {
    String _mcu{};
    int8_t _rssi = -128;
    int8_t _snr = -128;

protected:
    size_t localSize() override { return _mcu.length()+1 + sizeof(int8_t)*2; }

public:
    const static uint8_t PACKET_TYPE = 0x03;
    Pong() = default;
    ~Pong() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    String mcu() const      { return _mcu; }
    void mcu(String m)      { _mcu = m; }
    int8_t rssi() const     { return _rssi; }
    void rssi(int8_t r)     { _rssi = r; }
    int8_t snr() const      { return _snr; }
    void snr(int8_t s)      { _snr = s; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};


class NodeLocate : public Packet {
    uint32_t _node = 0x00000000;

protected:
    size_t localSize() override { return sizeof(uint32_t); }

public:
    const static uint8_t PACKET_TYPE = 0x04;
    NodeLocate() = default;
    ~NodeLocate() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    uint32_t node() const   { return _node; }
    void node(uint32_t n)   { _node = n; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};


class NodeFound : public Packet {
protected:
    size_t localSize() override { return 0; }

public:
    const static uint8_t PACKET_TYPE = 0x05;
    NodeFound() = default;
    ~NodeFound() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    // uint32_t node() const                       { return _node; }
    // void node(uint32_t n)                       { _node = n; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};