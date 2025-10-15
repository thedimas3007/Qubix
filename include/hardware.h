#pragma once

#include <SPI.h>
#include <Wire.h>

// NOTE: Pins per each STM32 chip could be different, hence configuration per each chip
// TODO: Review STM32 pin configuration
// Lack of the second SPI bus may lead to problems with SPI displays

/**********/
/* RP2040 */
/**********/
#if defined(ARDUINO_ARCH_RP2040)
#define EXT_I2C_SDA 6
#define EXT_I2C_SCL 7

#define EXT_SPI0_MISO 4
#define EXT_SPI0_MOSI 3
#define EXT_SPI0_SCK 2

#define EXT_SPI1_MISO 12
#define EXT_SPI1_MOSI 11
#define EXT_SPI1_SCK 10

/*****************/
/* STM32WB55CGUx */
/*****************/
#elif defined(ARDUINO_GENERIC_WB55CGUX)
#define EXT_I2C_SDA PB9
#define EXT_I2C_SCL PB8

#define EXT_SPI0_MISO PA6
#define EXT_SPI0_MOSI PA7
#define EXT_SPI0_SCK PA5

/*****************/
/* STM32F446RETx */
/*****************/
#elif defined(ARDUINO_GENERIC_F446RETX)
#define EXT_I2C_SDA PB9
#define EXT_I2C_SCL PB8

#define EXT_SPI0_MISO PA6
#define EXT_SPI0_MOSI PA7
#define EXT_SPI0_SCK PA5

// #define EXT_SPI1_MISO 12
// #define EXT_SPI1_MOSI 11
// #define EXT_SPI1_SCK 10

#else
#error "Unknown platform. Define pins in `hardware.h`"
#endif


#if defined(EXT_SPI1_MISO) && defined(EXT_SPI1_MOSI) && defined(EXT_SPI1_SCK)
#define EXTRA_SPI
#endif

/***************/
/* SPI and I2C */
/***************/
#if   defined(ARDUINO_ARCH_RP2040)
extern SPIClassRP2040*  extSPI;
extern SPIClassRP2040*  extSPI1;
extern TwoWire*   extI2C;

#elif defined(ARDUINO_ARCH_STM32)
extern SPIClass*  extSPI;
extern SPIClass*  extSPI1;
extern TwoWire*   extI2C;
#endif
// extern TwoWire&   extI2C1; // unused

void initHardware();