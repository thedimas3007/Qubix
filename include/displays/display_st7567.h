#pragma once
#ifdef TARGET_ST7567 // double check and calm the compiler down

#include <ST7567.h>
#include "configuration.h"

typedef ST7567 DisplayType;
extern ST7567 display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) { display.setContrast(contrast); }
#define HAS_BACKLIGHT // comment if backlight isn't connected
inline void setBacklight(uint8_t brightness) { analogWrite(DISPLAY_BL, brightness); }
#define THEME (UITheme{BLACK, WHITE})
#define DISPLAY_MODE DISPLAY_MODE_BUFFERED

#endif