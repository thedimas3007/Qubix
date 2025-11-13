#pragma once

#include <Arduino.h>
#include <map>

template <class Key, class Value>
class CacheMap {
    struct Container {
        Value value;
        uint32_t timeout;
    };

    std::map<Key, Container> data;
    uint32_t ttl_ms;

    bool expired(const Container& c, uint32_t now) const {
        return static_cast<int32_t>(c.timeout - now) < 0;
    }

    void refresh(Container& c, uint32_t now) {
        c.timeout = now + ttl_ms;
    }

public:
    class Iterator {
        using Base = typename std::map<Key,Container>::iterator;
        Base it, end;
        uint32_t now;

        void skip() {
            while (it != end) {
                if ((int32_t)(it->second.timeout - now) >= 0) break;
                it = std::next(it);
            }
        }

    public:
        Iterator(Base i, Base e, uint32_t now) : it(i), end(e), now(now) { skip(); }
        Iterator& operator++() { ++it; skip(); return *this; }

        bool operator!=(const Iterator& o) const { return it != o.it; }

        std::pair<const Key&, Value&> operator*() const {
            return { it->first, it->second.value };
        }
    };

    CacheMap(uint32_t ttl_ms = 30000) : ttl_ms(ttl_ms) {}

    Value& operator[](const Key& key) {
        auto& c = data[key];
        refresh(c, millis());
        return c.value;
    }

    bool contains(const Key& key) {
        auto it = data.find(key);
        if (it == data.end()) return false;
        if (expired(it->second, millis())) { data.erase(it); return false; }
        return true;
    }

    Value* findValue(const Key& key) {
        uint32_t now = millis();
        auto it = data.find(key);
        if (it == data.end()) return nullptr;
        if (expired(it->second, now)) { data.erase(it); return nullptr; }
        refresh(it->second, now);
        return &it->second.value;
    }

    const Value* findValue(const Key& key) const {
        auto it = data.find(key);
        if (it == data.end()) return nullptr;
        if (expired(it->second, millis())) return nullptr;
        return &it->second.value;
    }

    Value& at(const Key& key) {
        uint32_t now = millis();
        auto& c = data.at(key);
        if (expired(c, now)) {
            data.erase(key);
            assert(false && "CacheMap::at() key expired");
        }
        refresh(c, now);
        return c.value;
    }

    const Value& at(const Key& key) const {
        const auto& c = data.at(key);
        if (expired(c, millis())) {
            data.erase(key);
            assert(false && "CacheMap::at() key expired");
        }
        return c.value;
    }


    void put(const Key& key, const Value& value, uint32_t ttl_override = 0) {
        ttl_override = ttl_override ? ttl_override : this->ttl_ms;
        auto& c = data[key];
        c.value = value;
        c.timeout = millis() + ttl_override;
    }


    Iterator begin() {
        return Iterator(data.begin(), data.end(), millis());
    }

    Iterator end() {
        return Iterator(data.end(), data.end(), millis());
    }

    void clear() { data.clear(); }
    void erase(const Key& key) { data.erase(key); }

    bool empty() {
        uint32_t now = millis();
        for (auto it = data.begin(); it != data.end(); ) {
            if (expired(it->second, now))   it = data.erase(it);
            else                            return false;
        }
        return true;
    }

    size_t size() {
        uint32_t now = millis();
        for (auto it = data.begin(); it != data.end(); ) {
            if (expired(it->second, now))   it = data.erase(it);
            else                            ++it;
        }
        return data.size();
    }
};
