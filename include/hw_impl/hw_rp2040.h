#pragma once
#ifdef ARDUINO_ARCH_RP2040
#include <SPI.h>
#include <Wire.h>

#include "hw_base.h"
#include "utils.h"

/**************/
/**** Pins ****/
/**************/
#define EXT_I2C_SCL     7
#define EXT_I2C_SDA     6

#define EXT_SPI0_MISO   4
#define EXT_SPI0_MOSI   3
#define EXT_SPI0_SCK    2

#define EXT_SPI1_MISO   12
#define EXT_SPI1_MOSI   11
#define EXT_SPI1_SCK    10

/*********************/
/**** Peripherals ****/
/*********************/
extern TwoWire*         extI2C;
extern SPIClassRP2040*  extSPI;
extern SPIClassRP2040*  extSPI1;

/****************/
/**** Driver ****/
/****************/
class DriverRP2040 : public DriverBase {
public:
    DriverRP2040() = default;

    uint32_t currentClock() const override;
    uint32_t currentRam() const override;
    uint32_t currentFlash() const override;

    uint32_t boardId() const override;

    void init() override;
    void reboot() override;
};


#endif
