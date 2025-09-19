#ifndef TERRACOTTA_CONFIGURATION_H
#define TERRACOTTA_CONFIGURATION_H

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

MbedI2C extI2C(6, 7);
MbedSPI extSPI(4, 3, 2);


// Display type (automatically configured)
#ifdef TARGET_SH1106
#include <Adafruit_SH110X.h>
Adafruit_SH1106G display(128, 64, &extI2C, -1);
#define DISPLAY_FG SH110X_WHITE
#define DISPLAY_BG SH110X_BLACK
#endif

#ifdef TARGET_SSD1306
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &extI2C, -1);
#define DISPLAY_FG SSD1306_WHITE
#define DISPLAY_BG SSD1306_BLACK
#endif

#endif //TERRACOTTA_CONFIGURATION_H