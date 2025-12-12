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

protected:
    size_t localSize() override { return _mcu.length()+1; }

public:
    const static uint8_t PACKET_TYPE = 0x03;
    Pong() = default;
    ~Pong() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    String mcu() const      { return _mcu; }
    void mcu(String m)      { _mcu = m; }

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
    float _rssi_avg = -140, _snr_avg = -140;

protected:
    size_t localSize() override { return sizeof(float)*2; }

public:
    const static uint8_t PACKET_TYPE = 0x05;
    NodeFound() = default;
    ~NodeFound() override = default;

    uint8_t type() override { return PACKET_TYPE; }


    float rssiAvg() const   { return _rssi_avg; }
    void rssiAvg(float v)   { _rssi_avg = v; }
    float snrAvg() const    { return _snr_avg; }
    void snrAvg(float v)    { _snr_avg = v; }

    void appendInfo(float r, float s);

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};

class TimeSync : public Packet {
protected:
    size_t localSize() override { return 0; }

public:
    const static uint8_t PACKET_TYPE = 0x06;
    TimeSync() = default;
    ~TimeSync() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};

class TimeResponse : public Packet {
    uint32_t _timestamp = 0;

protected:
    size_t localSize() override { return sizeof(uint32_t); }

public:
    const static uint8_t PACKET_TYPE = 0x07;
    TimeResponse() = default;
    ~TimeResponse() override = default;

    uint8_t type() override { return PACKET_TYPE; }

    uint32_t timestamp() const  { return _timestamp; }
    void timestamp(uint32_t v)  { _timestamp = v; }

    void serialize(WriteBuffer& buffer) override;
    void deserialize(ReadBuffer& buffer) override;
};