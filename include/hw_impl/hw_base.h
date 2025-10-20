#pragma once
#include <cstdint>

// TOOD: more detailed stats
class DriverBase {
public:
    virtual ~DriverBase() = default;

    virtual const char* platform() const { return PIO_PLATFORM; }
    virtual const char* board() const { return PIO_BOARD; }
    virtual const char* mcu() const { return HW_MCU; }

    virtual uint32_t maxClock() const { return HW_F_CPU; }
    virtual uint32_t currentClock() const = 0;

    virtual uint32_t maxRam() const { return HW_RAM_BYTES; }
    virtual uint32_t currentRam() const = 0;

    virtual uint32_t maxFlash() const { return HW_FLASH_BYTES; }
    virtual uint32_t currentFlash() const = 0;

    virtual void init() = 0;
    virtual void reboot() = 0;
};