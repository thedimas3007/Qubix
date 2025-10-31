#pragma once
#ifdef TARGET_SSD1306 // double check and calm the compiler down

#include <Adafruit_SSD1306.h>

typedef Adafruit_SSD1306 DisplayType;
extern DisplayType display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(contrast);
}
#define THEME (UITheme{SSD1306_WHITE, SSD1306_BLACK})
#define DISPLAY_MODE DISPLAY_MODE_BUFFERED

#endif