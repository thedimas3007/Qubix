#ifndef TERRACOTTA_CONFIGURATION_H
#define TERRACOTTA_CONFIGURATION_H

#include <Wire.h>
#include <SPI.h>

#define EXT_I2C_SDA     6
#define EXT_I2C_SCL     7

#define EXT_SPI1_MISO   4
#define EXT_SPI1_MOSI   3
#define EXT_SPI1_SCK    2

#define EXT_SPI2_MISO   12
#define EXT_SPI2_MOSI   11
#define EXT_SPI2_SCK    10

#define RADIO_BUSY      15
#define RADIO_IRQ       14
#define RADIO_CS        5
#define RADIO_RESET     RADIOLIB_NC

#define DISPLAY_RESET   26
#define DISPLAY_CS      9
#define DISPLAY_DC      13
#define DISPLAY_BL      8

#define DISPLAY_ADDRESS 0x3C
#define KEYBOARD_ADDRESS 0x5F

extern MbedI2C extI2C;
extern MbedSPI extSPI1;
extern MbedSPI extSPI2;


// Display type (automatically configured)
#ifdef TARGET_SH1106
#include <Adafruit_SH110X.h>
extern Adafruit_SH1106G display;
#define DISPLAY_FG SH110X_WHITE
#define DISPLAY_BG SH110X_BLACK

#elif defined(TARGET_SSD1306)
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;
#define DISPLAY_FG SSD1306_WHITE
#define DISPLAY_BG SSD1306_BLACK

#elif defined(TARGET_ST7567)
#include <ST7567.h>
extern ST7567 display;
#define HAS_BACKLIGHT
#define DISPLAY_FG BLACK
#define DISPLAY_BG WHITE
#endif

#endif