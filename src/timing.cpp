#include "timing.h"

#include <utility>

#include "network/packet_types.h"

void Interval::reset() {
    last_update = millis();
}

bool Interval::check() {
    if (millis() - last_update > rate) { reset(); return true; }
    return false;
}


size_t Scheduler::size() const {
    return entries.size();
}

size_t Scheduler::schedule(std::function<void()> callback, uint32_t interval_ms) {
    entries.push_back(std::make_unique<Entry>(Entry{std::move(callback), Interval(interval_ms)}));
    return entries.size() - 1;
}

bool Scheduler::remove(size_t index) {
    if (index >= entries.size() || index < 0) return false;
    entries.erase(entries.begin() + index);
    return true;
}

void Scheduler::tick() {
    for (auto& entryPtr : entries) {
        auto& [callback, interval] = *entryPtr;
        if (!interval.check()) continue;
        callback();
    }
}
