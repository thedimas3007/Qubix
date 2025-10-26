#pragma once

#include <map>

#include "RadioLib.h"
#include "buffer.h"

class Packet : public Serializable {
public:
    using Factory = std::function<Packet*()>;

private:
    static inline std::map<uint8_t, Factory> registry{};

public:
    virtual ~Packet() = default;
    virtual uint8_t type() = 0;

    static void registerType(uint8_t id, const Factory& factory) {
        registry[id] = factory;
    }

    static Packet* create(uint8_t id) {
        auto it = registry.find(id);
        return (it != registry.end()) ? it->second() : nullptr;
    }
};

class NetManager {
    PhysicalLayer* radio = nullptr;
    volatile bool* irq_en;
    std::map<uint8_t, std::vector<std::function<void(Packet&)>>> listeners;
public:
    void begin(PhysicalLayer* r, volatile bool* en) {
        this->radio = r;
        this->irq_en = en;
    }

    int16_t send(Packet& packet) const {
        if (!radio || !irq_en) { return RADIOLIB_ERR_NULL_POINTER; }

        *irq_en = false;
        WriteBuffer buffer = WriteBuffer(packet.size());
        packet.serialize(buffer);
        int16_t status = radio->transmit(buffer.raw(), buffer.len());
        *irq_en = true;
        return status;
    }

    template<typename T>
    void reg(std::function<void(T&)> fn) {
        uint8_t id = T::PACKET_TYPE;
        listeners[id].push_back([fn](Packet& p) {
            fn(static_cast<T&>(p));
        });
    }

    void dispatch(Packet& p) {
        auto it = listeners.find(p.type());
        if (it != listeners.end()) {
            for (auto& f : it->second) f(p);
        }
    }
};
