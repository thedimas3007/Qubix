#pragma once

#include <functional>
#include <map>
#include <memory>
#include <queue>

#include "RadioLib.h"
#include "buffer.h"

class Packet : public Serializable {
    uint32_t _pktid = 0;
    uint32_t _hwid = 0;
    uint32_t _target = 0;
protected:
    virtual size_t localSize() = 0;

public:
    using Factory = std::function<std::unique_ptr<Packet>()>;

private:
    static inline std::map<uint8_t, Factory> registry{};

public:
    virtual ~Packet() = default;
    virtual uint8_t type() = 0;
    size_t size() override { return localSize() + 1 + sizeof(uint32_t)*3; };

    static void registerType(uint8_t id, const Factory& factory) {
        registry[id] = factory;
    }

    static std::unique_ptr<Packet> create(uint8_t id) {
        auto it = registry.find(id);
        return (it != registry.end()) ? it->second() : nullptr;
    }

    uint32_t pktid() const      { return _pktid; }
    void pktid(uint32_t pktid)  { _pktid = pktid; }
    uint32_t hwid() const       { return _hwid; }
    void hwid(uint32_t id)      { _hwid = id; }
    uint32_t target() const     { return _target; }
    void target(uint32_t id)    { _target = id; }
};

class NetManager {
    struct PendingPacket { std::unique_ptr<Packet> packet; uint32_t target; int8_t priority; };
    struct ComparePriority {
        bool operator()(const PendingPacket& a, const PendingPacket& b) const {
            return a.priority < b.priority;
        }
    };

    PhysicalLayer* radio = nullptr;
    std::map<uint8_t, std::vector<std::function<void(NetManager&, Packet&)>>> listeners; // type_id -> vector of listeners
    std::priority_queue<PendingPacket, std::vector<PendingPacket>, ComparePriority> packet_queue;

    volatile bool* irq_en = nullptr;
    volatile bool* irq_rx = nullptr;

    uint32_t timed_out = 0;
    uint8_t retries = 0; // max retries maybe
public:
    void begin(PhysicalLayer* r, volatile bool* en, volatile bool* rx);
    void queue(std::unique_ptr<Packet> packet, uint32_t target, int8_t priority = 0);
    int16_t send(Packet& packet, uint32_t target) const; // should I make it private?
    void dispatch(Packet& p);
    int16_t tick();
    bool isTimedOut() const { return timed_out != 0 && timed_out > millis(); }
    void wait(long time_ms) { timed_out = millis() + time_ms; };

    template <typename T>
    void reg(std::function<void(NetManager&, T&)> fn) {
        uint8_t id = T::PACKET_TYPE;
        listeners[id].push_back(
            [fn](NetManager& nm, Packet& p) {
                fn(nm, static_cast<T&>(p));
            }
        );
    }
};
