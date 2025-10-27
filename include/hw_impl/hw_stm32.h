#pragma once
#ifdef ARDUINO_ARCH_STM32
#include "hw_base.h"
#include <SPI.h>
#include <Wire.h>

/**************/
/**** Pins ****/
/**************/
#define EXT_I2C_SCL     PB8
#define EXT_I2C_SDA     PB9

#define EXT_SPI0_MISO   PA6
#define EXT_SPI0_MOSI   PA7
#define EXT_SPI0_SCK    PA5

// TODO: make automatic pin rotation/detection
#if defined(PB14) && defined(PB15) && defined(PB13)
#define EXT_SPI1_MISO   PB14
#define EXT_SPI1_MOSI   PB15
#define EXT_SPI1_SCK    PB13
#endif // I'm not sure whether the pins are the same on all boards


/*********************/
/**** Peripherals ****/
/*********************/
extern TwoWire*     extI2C;
extern SPIClass*    extSPI;
extern SPIClass*    extSPI1;


/****************/
/**** Driver ****/
/****************/
class DriverSTM32 : public DriverBase {
public:
    DriverSTM32() = default;

    uint32_t currentClock() const override;
    uint32_t currentRam() const override;
    uint32_t currentFlash() const override;

    uint32_t boardId() const override;

    void init() override;
    void reboot() override;;
};

#endif

