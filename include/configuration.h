#pragma once

#include "settings.h"

#define EXT_I2C_SDA         6
#define EXT_I2C_SCL         7

#define EXT_SPI0_MISO       4
#define EXT_SPI0_MOSI       3
#define EXT_SPI0_SCK        2

#define EXT_SPI1_MISO       12
#define EXT_SPI1_MOSI       11
#define EXT_SPI1_SCK        10


#define RADIO_BUSY          15
#define RADIO_IRQ           14
#define RADIO_CS            5
#define RADIO_RESET         RADIOLIB_NC

#define DISPLAY_RESET       26
#define DISPLAY_CS          9
#define DISPLAY_DC          13
#define DISPLAY_BL          8

#define DISPLAY_ADDRESS     0x3C
#define KEYBOARD_ADDRESS    0x5F

#define MESSAGE_LENGTH 128

extern Settings settings;

// Display type (automatically configured)
#ifdef TARGET_SH1106
#include "displays/display_sh1106.h"

#elif defined(TARGET_SSD1306)
#include "displays/display_ssd1306.h"

#elif defined(TARGET_ST7567)
#include "displays/display_st7567.h"

#endif

// Reboot functions
#if defined(ARDUINO_ARCH_RP2040)
inline void resetMCU() {
    watchdog_enable(1, true);
    while (1);
}
#endif
