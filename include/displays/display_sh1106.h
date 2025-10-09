#pragma once
#ifdef TARGET_SH1106 // double check and calm the compiler down

#include <Adafruit_SH110X.h>

extern Adafruit_SH1106G display;
#define HAS_CONTRAST
inline void setContrast(uint8_t contrast) { display.setContrast(contrast); }
#define THEME (UITheme{SH110X_WHITE, SH110X_BLACK})

#endif