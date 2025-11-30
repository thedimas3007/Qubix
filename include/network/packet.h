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
    "Unknown",          // 0x00
    "HelloPacket",      // 0x01
    "NeighborLocate",   // 0x02
    "NeighborResponse", // 0x03
    "NodeLocate",       // 0x04
    "NodeFound"         // 0x05
};

class Packet : public Serializable {
    uint32_t _pktid = 0;
    uint8_t _hops = 0;
    std::vector<uint32_t> _path;
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

    uint32_t pktid() const      { return _pktid; }
    void pktid(uint32_t pktid)  { _pktid = pktid; }

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


    SX126x* radio = nullptr;
    std::map<uint8_t, std::vector<Listener>> listeners; // type_id -> vector of listeners
    std::map<uint32_t, std::vector<PathListener>> path_listeners;
    std::priority_queue<PendingPacket, std::vector<PendingPacket>, ComparePriority> packet_queue;
    std::deque<PacketKey> last_packets;
    CacheMap<uint32_t, Path> path_cache{900'000}; // 15 mins

    volatile bool* irq_en = nullptr;
    volatile bool* irq_rx = nullptr;

    uint32_t timed_out = 0;
    uint8_t retries = 0; // max retries maybe

public:
    bool seen(uint32_t sender, uint32_t id) {
        return std::find_if(last_packets.begin(), last_packets.end(),
            [&](auto& p) { return p.sender==sender && p.id==id; }) != last_packets.end();
    }

    bool seen(uint32_t id) {
        return std::find_if(last_packets.begin(), last_packets.end(),
            [&](auto& p) { return p.id==id; }) != last_packets.end();
    }

    void begin(SX126x* r, volatile bool* en, volatile bool* rx);
    void queueDirect(std::shared_ptr<Packet> packet, int8_t priority = 0);
    void queue(std::shared_ptr<Packet> packet, uint32_t target, int8_t priority = 0);
    int16_t send(Packet& packet) const; // should I make it private?
    void received();
    void dispatch(Packet& p);
    int16_t tick();
    bool isWaiting() const { return timed_out != 0 && timed_out > millis(); }
    void wait(long time_ms) { timed_out = millis() + time_ms; }
    void locate(uint32_t target, std::function<void(Path& path)> callback, uint32_t timeout_ms = 7500);
    CacheMap<uint32_t, Path>& cache();

    template <typename T>
    void reg(std::function<void(NetManager&, T&)> fn) {
        uint8_t id = T::PACKET_TYPE;
        listeners[id].push_back(
            {[fn](NetManager& nm, Packet& p) {
                fn(nm, static_cast<T&>(p));
            }}
        );
    }

    template <typename T>
    void request(std::unique_ptr<Packet> packet, std::function<void(NetManager&, T&)> callback, std::function<void(bool)> timeout, uint32_t target, uint32_t timeout_ms, int8_t priority = 0) {
        uint8_t id = T::PACKET_TYPE;
        queue(std::move(packet), target, priority);
        listeners[id].push_back(
            {[callback](NetManager& nm, Packet& p) {
                callback(nm, static_cast<T&>(p));
            }, timeout, millis() + timeout_ms, true}
        );
    }
};
