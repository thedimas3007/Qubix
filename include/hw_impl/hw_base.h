#pragma once

#include <Arduino.h>

struct RAMInfo { uint32_t bss, data, stack, heap; };

class DriverBase {
    const float rise_alpha = 0.35f;
    const float fall_alpha = 0.10f;

    uint32_t loop_start = micros();
    float last_load = 0;
public:
    virtual ~DriverBase() = default;

    void loadTick() {
        uint32_t now = micros();
        float dt = (now - loop_start) / 1'000'000.0f;
        loop_start = now;

        float load = dt <= 1.0f ? dt : 1.0f;

        if (load > last_load)   last_load = last_load * (1.0f - rise_alpha) + load * rise_alpha;
        else                    last_load = last_load * (1.0f - fall_alpha) + load * fall_alpha;
    }

    float currentLoad() const { return last_load; }

    const char* platform() const { return PIO_PLATFORM; }
    const char* board() const { return PIO_BOARD; }
    const char* mcu() const { return HW_MCU; }

    virtual uint32_t maxClock() const { return HW_F_CPU; }
    virtual uint32_t currentClock() const = 0;

    uint32_t maxRam() const { return HW_RAM_BYTES; }
    virtual uint32_t currentRamBSS() const = 0;
    virtual uint32_t currentRamData() const = 0;
    virtual uint32_t currentRamStack() const = 0;
    virtual uint32_t currentRamHeap() const = 0;
    RAMInfo currentRamInfo() const { return {currentRamBSS(), currentRamData(), currentRamStack(), currentRamHeap()}; };

    uint32_t maxFlash() const { return HW_FLASH_BYTES; }
    virtual uint32_t currentFlash() const = 0;

    virtual uint32_t boardId() const = 0;

    float batteryVoltage() const { return 3.76; }
    float batteryPercent() const {
        float v = batteryVoltage();
        float p = 23.81f*std::pow(v, 2) - 88.1*v + 50;
        return std::max(std::min(p, 100.0f), 0.0f);
    }

    virtual void init() = 0;
    virtual void reboot() = 0;
};