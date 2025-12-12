#pragma once
#include "RTClib.h"

class InternalRTC {
    bool _synchronized = false;
    uint32_t _last_tick = 0;
    DateTime _date_time;
public:
    bool begin();
    void adjust(const DateTime& date_time) ;
    DateTime now();
    void update();
    bool synchronized() const { return _synchronized; };
};
