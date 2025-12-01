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
    "NodeFound"     // 0x05
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


class NetManager {
public:
    struct Path {
        uint8_t hops = 0;
        uint32_t path[MAX_HOPS]{};

        void reset()            { for (auto& i : path) i = 0; }
        void push(uint32_t id)  { if (hops < MAX_HOPS) path[hops++] = id; }
        void pop()              { if (hops > 0)        path[--hops] = 0; }
    };

private:
    struct Listener {
        std::function<void(NetManager&, Packet&)> listener;
        std::function<void(bool)> timeout = [](bool){};
        uint32_t ttl = 0;
        bool temporary = false; // maybe merge into flags?
        bool received = false;
    };

    struct PathListener {
        std::function<void(Path&)> listener;
        // std::function<void()> timeout = []{};
        uint32_t ttl = 0;
    };

    struct PacketKey { uint32_t sender, id; };
    struct PendingPacket { std::shared_ptr<Packet> packet; int8_t priority; };
    struct ComparePriority {
        bool operator()(const PendingPacket& a, const PendingPacket& b) const {
            return a.priority > b.priority;
        }
    };


    SX126x* _radio = nullptr;
    std::map<uint8_t, std::vector<Listener>> _listeners; // type_id -> vector of listeners
    std::map<uint32_t, std::vector<PathListener>> _path_listeners;
    std::priority_queue<PendingPacket, std::vector<PendingPacket>, ComparePriority> _packet_queue;
    std::deque<PacketKey> _last_packets;
    CacheMap<uint32_t, Path> _path_cache{900'000}; // 15 mins

    volatile bool* _irq_en = nullptr;
    volatile bool* _irq_rx = nullptr;

    uint32_t _timed_out = 0;
    uint8_t _retries = 0; // max retries maybe


    uint64_t _bytes_tx = 0, _bytes_rx = 0;
    uint16_t _packets_tx = 0, _packets_rx = 0;
public:
    bool seen(uint32_t sender, uint32_t id) {
        return std::find_if(_last_packets.begin(), _last_packets.end(),
            [&](auto& p) { return p.sender==sender && p.id==id; }) != _last_packets.end();
    }

    bool seen(uint32_t id) {
        return std::find_if(_last_packets.begin(), _last_packets.end(),
            [&](auto& p) { return p.id==id; }) != _last_packets.end();
    }

    void begin(SX126x* r, volatile bool* en, volatile bool* rx);
    void queueDirect(std::shared_ptr<Packet> packet, int8_t priority = 0);
    void queue(std::shared_ptr<Packet> packet, uint32_t target, int8_t priority = 0);
    int16_t send(Packet& packet); // should I make it private?
    void received();
    void dispatch(Packet& p);
    int16_t tick();
    bool isWaiting() const { return _timed_out != 0 && _timed_out > millis(); }
    void wait(long time_ms) { _timed_out = millis() + time_ms; }
    void locate(uint32_t target, std::function<void(Path& path)> callback, uint32_t timeout_ms = 7500);
    CacheMap<uint32_t, Path>& cache();

    template <typename T>
    void reg(std::function<void(NetManager&, T&)> fn) {
        uint8_t id = T::PACKET_TYPE;
        _listeners[id].push_back(
            {[fn](NetManager& nm, Packet& p) {
                fn(nm, static_cast<T&>(p));
            }}
        );
    }

    template <typename T>
    void request(std::unique_ptr<Packet> packet, std::function<void(NetManager&, T&)> callback, std::function<void(bool)> timeout, uint32_t target, uint32_t timeout_ms, int8_t priority = 0) {
        uint8_t id = T::PACKET_TYPE;
        queue(std::move(packet), target, priority);
        _listeners[id].push_back(
            {[callback](NetManager& nm, Packet& p) {
                callback(nm, static_cast<T&>(p));
            }, timeout, millis() + timeout_ms, true}
        );
    }

    uint64_t bytesTx() const       { return _bytes_tx; }
    // void bytesTx(uint64_t v)       { _bytes_tx = v; }
    uint64_t bytesRx() const       { return _bytes_rx; }
    // void bytesRx(uint64_t v)       { _bytes_rx = v; }
    uint16_t packetsTx() const     { return _packets_tx; }
    // void packetsTx(uint16_t v)     { _packets_tx = v; }
    uint16_t packetsRx() const     { return _packets_rx; }
    // void packetsRx(uint16_t v)     { _packets_rx = v; }
};
