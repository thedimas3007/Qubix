#ifndef TERRACOTTA_CONFIGURATION_H
#define TERRACOTTA_CONFIGURATION_H

#include <Wire.h>

#define EXT_I2C_SDA     6
#define EXT_I2C_SCL     7

#define EXT_SPI_MISO    4
#define EXT_SPI_MOSI    3
#define EXT_SPI_SCK     2

#define RADIO_BUSY      15
#define RADIO_IRQ       14
#define RADIO_CS        5
#define RADIO_RESET     RADIOLIB_NC

#define DISPLAY_ADDRESS 0x3C
#define KEYBOARD_ADDRESS 0x5F

extern MbedI2C extI2C;
extern MbedSPI extSPI;


// Display type (automatically configured)
#ifdef TARGET_SH1106
#include <Adafruit_SH110X.h>
extern Adafruit_SH1106G display;
#define DISPLAY_FG SH110X_WHITE
#define DISPLAY_BG SH110X_BLACK
#endif

#ifdef TARGET_SSD1306
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display
#define DISPLAY_FG SSD1306_WHITE
#define DISPLAY_BG SSD1306_BLACK
#endif

#endif