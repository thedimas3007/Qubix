#pragma once
#ifdef TARGET_SSD1306 // double check and calm the compiler down
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(contrast);
}
#define THEME (UITheme{SSD1306_WHITE, SSD1306_BLACK})

#endif