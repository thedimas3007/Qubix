#pragma once

#include <map>
#include <memory>
#include <queue>

#include "RadioLib.h"
#include "buffer.h"

class Packet : public Serializable {
public:
    using Factory = std::function<std::unique_ptr<Packet>()>;

private:
    static inline std::map<uint8_t, Factory> registry{};

public:
    virtual ~Packet() = default;
    virtual uint8_t type() = 0;

    static void registerType(uint8_t id, const Factory& factory) {
        registry[id] = factory;
    }

    static std::unique_ptr<Packet> create(uint8_t id) {
        auto it = registry.find(id);
        return (it != registry.end()) ? it->second() : nullptr;
    }
};

class NetManager {
    struct PendingPacket { std::unique_ptr<Packet> packet; int8_t priority; };
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
public:
    void begin(PhysicalLayer* r, volatile bool* en, volatile bool* rx);
    void queue(std::unique_ptr<Packet> packet, int8_t priority = 0);
    int16_t send(Packet& packet); // should I make it private?
    void dispatch(Packet& p);
    int16_t tick();

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
