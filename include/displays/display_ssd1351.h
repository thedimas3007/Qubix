#pragma once
#ifdef TARGET_SSD1351 // double check and calm the compiler down

#include <Adafruit_SSD1351.h>

// TODO: specify 1-8-16
class Buffered_SSD1351 : public GFXcanvas16 {
    Adafruit_SSD1351 _display;

public:
    Buffered_SSD1351(uint16_t width, uint16_t height, int8_t cs_pin,
                     int8_t dc_pin, int8_t mosi_pin, int8_t sclk_pin,
                     int8_t rst_pin = -1)
        : GFXcanvas16(width, height, true),
          _display(width, height, cs_pin, dc_pin, mosi_pin, sclk_pin, rst_pin) {}

    Buffered_SSD1351(uint16_t width, uint16_t height, SPIClass *spi,
                     int8_t cs_pin, int8_t dc_pin, int8_t rst_pin = -1)
        : GFXcanvas16(width, height, true),
          _display(width, height, spi, cs_pin, dc_pin, rst_pin) {}

    void begin() { _display.begin(); }
    void display();
};

typedef Buffered_SSD1351 DisplayType;
extern DisplayType display;
// #define HAS_CONTRAST
// inline void setContrast(uint8_t contrast) {
//     display.sendCommand(SSD1351_CMD_CONTRASTMASTER);
//     display.sendCommand(std::floor(contrast / 16.0f)); // 0-15
// }
#define THEME (UITheme{0x1FF1, 0x0000})
#define DISPLAY_MODE DISPLAY_MODE_BUFFERED
#define HAS_COLOR

#endif
