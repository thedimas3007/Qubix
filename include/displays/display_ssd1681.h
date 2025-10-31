#pragma once
#ifdef TARGET_SSD1681

#include <GxEPD2_BW.h>

typedef GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> DisplayType;
extern DisplayType display;
#define THEME (UITheme{GxEPD_BLACK, GxEPD_WHITE})
#define DISPLAY_MODE DISPLAY_MODE_EINK

#endif