#pragma once

#include "packet.h"

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
    CacheMap<uint32_t, float> _last_rssi{900'000};

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
    float avgRssi();
    float avgScore();

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
