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
    return sch_entries.size();
}

size_t Scheduler::schedule(std::function<void()> callback, uint32_t interval_ms) {
    sch_entries.push_back(std::make_unique<ScheduleEntry>(ScheduleEntry{std::move(callback), Interval(interval_ms)}));
    return sch_entries.size() - 1;
}

size_t Scheduler::postpone(std::function<void()> callback, uint32_t interval_ms) {
    tmt_entries.push_back(std::make_unique<TimeoutEntry>(TimeoutEntry{std::move(callback), millis()+interval_ms}));
    return tmt_entries.size() - 1;
}

bool Scheduler::removeScheduled(size_t index) {
    if (index >= sch_entries.size() || index < 0) return false;
    sch_entries.erase(sch_entries.begin() + index);
    return true;
}

bool Scheduler::removePostponed(size_t index) {
    if (index >= tmt_entries.size() || index < 0) return false;
    tmt_entries.erase(tmt_entries.begin() + index);
    return true;
}

void Scheduler::tick() {
    for (auto& entryPtr : sch_entries) {
        auto& [callback, interval] = *entryPtr;
        if (!interval.check()) continue;
        callback();
    }

    for (size_t i = tmt_entries.size(); i-- > 0;) {
        auto& [callback, interval_ms] = *tmt_entries[i];
        if (millis() < interval_ms) continue;
        callback();
        tmt_entries.erase(tmt_entries.begin() + i);
    }
}
