#pragma once

#include <Arduino.h>
#include <functional>
#include <memory>

class Interval {
    const uint32_t rate;
    uint32_t last_update;
public:
    explicit Interval(uint32_t rate_ms) : rate(rate_ms), last_update(millis()) {}
    ~Interval() = default;

    void reset();
    bool check();
};

class Scheduler {
    struct ScheduleEntry { std::function<void()> callback; Interval interval; };
    struct TimeoutEntry { std::function<void()> callback; uint32_t timeout; };
    std::vector<std::unique_ptr<ScheduleEntry>> sch_entries;
    std::vector<std::unique_ptr<TimeoutEntry>> tmt_entries;
public:
    Scheduler() = default;
    ~Scheduler() = default;

    size_t size() const;
    size_t schedule(std::function<void()> callback, uint32_t interval_ms);
    size_t postpone(std::function<void()> callback, uint32_t interval_ms);
    bool removeScheduled(size_t index);
    bool removePostponed(size_t index);
    void tick();
};