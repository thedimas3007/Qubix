#pragma once

#include "settings.h"
#include "hardware.h"

/***************/
/* Module Pins */
/***************/
#if defined(ARDUINO_ARCH_RP2040)
#define RADIO_BUSY          15
#define RADIO_IRQ           14
#define RADIO_CS            5
#define RADIO_RESET         RADIOLIB_NC

#define DISPLAY_RESET       26
#define DISPLAY_CS          9
#define DISPLAY_DC          13
#define DISPLAY_BL          8

#elif defined(ARDUINO_ARCH_STM32)
#define RADIO_BUSY          PA2
#define RADIO_IRQ           PA3
#define RADIO_CS            PA4
#define RADIO_RESET         RADIOLIB_NC

// #define DISPLAY_RESET       -1
// #define DISPLAY_CS          -1
// #define DISPLAY_DC          -1
// #define DISPLAY_BL          -1

#endif


/*****************/
/* I2C addresses */
/*****************/
#define DISPLAY_ADDRESS     0x3C
#define KEYBOARD_ADDRESS    0x5F


/**********/
/* Consts */
/**********/
#define MESSAGE_LENGTH 128

/***********/
/* Display */
/***********/
#ifdef TARGET_SH1106
#include "displays/display_sh1106.h"

#elif defined(TARGET_SSD1306)
#include "displays/display_ssd1306.h"

#elif defined(TARGET_ST7567)
#include "displays/display_st7567.h"
#endif

inline Settings settings;


/*****************/
/* Power helpers */
/*****************/
inline void resetMCU() {
#if defined(ARDUINO_ARCH_RP2040)
    watchdog_enable(1, true);
    while (true);
#elif defined(ARDUINO_ARCH_STM32)
    NVIC_SystemReset();
#endif
}
