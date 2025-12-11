#include "hw_impl/clock.h"

bool InternalRTC::begin() {
    _last_tick = millis();
    _date_time = DateTime(2000,1,1, 0, 0, 0);
    return true;
}

void InternalRTC::adjust(const DateTime& date_time) {
    _syncronized = true;
    _date_time = date_time;
    _last_tick = millis();
}

DateTime InternalRTC::now() {
    update();
    return _date_time;
}

void InternalRTC::update() {
    uint32_t t = millis();
    uint32_t diff = 0;
    if (t < _last_tick) {
        diff = 0xFFFFFFFF - _last_tick + t;
    } else {
        diff = t - _last_tick;
    }

    if (diff < 1000) return;
    _last_tick = t;
    _date_time = _date_time + TimeSpan(diff/1000);
}
