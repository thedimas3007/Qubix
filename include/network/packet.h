#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <queue>

#include "RadioLib.h"
#include "buffer.h"
#include "configuration.h"
#include "structs.h"

inline String packet_names[] = {
    "Unknown",      // 0x00
    "HelloPacket",  // 0x01
    "Ping",         // 0x02
    "Pong",         // 0x03
    "NodeLocate",   // 0x04
    "NodeFound",    // 0x05
    "TimeSync",     // 0x06
    "TimeResponse"  // 0x07
};

class Packet : public Serializable {
    uint32_t _packet_id = 0;
    uint8_t _hops = 0;
    std::vector<uint32_t> _path;

    float _rssi = -255, _snr = -255;
protected:
    virtual size_t localSize() = 0;

public:
    using Factory = std::function<std::unique_ptr<Packet>()>;

private:
    static inline std::map<uint8_t, Factory> registry{};

public:
    const static uint8_t PACKET_TYPE = 0;

    virtual ~Packet() = default;
    virtual uint8_t type() = 0;
    size_t size() override { return localSize() + sizeof(uint8_t)*2 + sizeof(uint32_t)*(1+_path.size()); };

    static void registerType(uint8_t id, const Factory& factory) {
        registry[id] = factory;
    }

    static std::unique_ptr<Packet> create(uint8_t id) {
        auto it = registry.find(id);
        return (it != registry.end()) ? it->second() : nullptr;
    }

    uint32_t packetId() const   { return _packet_id; }
    void packetId(uint32_t id)  { _packet_id = id; }

    float rssi() const          { return _rssi; }
    void rssi(float r)          { _rssi = r; }
    float snr() const           { return _snr; }
    void snr(float s)           { _snr = s; }

    uint32_t sender() const     { return !isStart() ? _path[hops()-1] : 0; }
    uint32_t current() const    { return _path[hops()]; }
    uint32_t target() const     { return !isEnd() ? _path[hops()+1] : 0; }

    bool isStart() const        { return hops() == 0; }
    bool isEnd() const          { return hops()+1 >= _path.size(); }
    bool isBroadcast() const    { return current() == 0xFFFFFFFF; }

    uint8_t hops() const        { return _hops; }
    void hops(uint8_t hops)     { _hops = std::clamp<uint8_t>(hops, 0, 0xF);  }
    void hopsInc(int8_t inc=1)  { hops(std::clamp<uint8_t>(hops()+inc, 0, 0xF)); }

    const std::vector<uint32_t>& path() const   { return _path; }
    void path(const std::vector<uint32_t>& p)   { _path = p; }
    void pushNode(uint32_t id)                  { _path.push_back(id); }
    uint8_t pathLength() const                  { return _path.size(); }
    void clearPath()                            { _path.clear(); }
};
