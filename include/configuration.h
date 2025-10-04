#pragma once

#ifndef QUBIX_CONFIGURATION_H
#define QUBIX_CONFIGURATION_H

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
#include <Adafruit_SH110X.h>
extern Adafruit_SH1106G display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) { display.setContrast(contrast); }
#define DISPLAY_FG SH110X_WHITE
#define DISPLAY_BG SH110X_BLACK

#elif defined(TARGET_SSD1306)
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) { display.setContrast(contrast); }
#define DISPLAY_FG SSD1306_WHITE
#define DISPLAY_BG SSD1306_BLACK

#elif defined(TARGET_ST7567)
#include <ST7567.h>
extern ST7567 display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) { display.setContrast(contrast); }
#define HAS_BACKLIGHT // comment if backlight isn't connected
inline void setBacklight(uint8_t brightness) { analogWrite(DISPLAY_BL, brightness); }
#define DISPLAY_FG BLACK
#define DISPLAY_BG WHITE
#endif

// Reboot functions
#if defined(ARDUINO_ARCH_RP2040)
inline void reboot() {
    watchdog_enable(1, true);
    while (1);
}
#endif

#endif