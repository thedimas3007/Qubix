#pragma once

#include <Adafruit_GFX.h>

// #include "base.h"
#include "configuration.h"

class UIApp;

struct UITheme {
    uint16_t foreground;
    uint16_t background;

    uint16_t& fg = foreground;
    uint16_t& bg = background;
};

#define DISPLAY_MODE_NONE        0
#define DISPLAY_MODE_BUFFERED    1
#define DISPLAY_MODE_BUFFERLESS  2
#define DISPLAY_MODE_EINK        3

class UIContext {
    const uint8_t glyph_width = 6, glyph_height = 8;
    uint8_t text_size = 1;
    bool refresh_requested = false;
    bool refresh_full = false;

#if DISPLAY_MODE == DISPLAY_MODE_EINK
    uint8_t partial_updates = 0;
    const uint8_t partial_cap = 15; // TODO: move into configuration.h or make config configurable
#endif
public:
#ifdef THEME
    UITheme theme = THEME;
#else
    UITheme theme = {1,0};
#endif


    DisplayType& display;
    int16_t x, y, width, height;

    explicit UIContext(DisplayType& display)
        : display(display), x(0), y(0), width(display.width()), height(display.height()) {}

    [[nodiscard]] int16_t charWidth() const       { return glyph_width * text_size; }
    [[nodiscard]] int16_t charHeight() const      { return glyph_height * text_size; }

    [[nodiscard]] uint8_t maxCharsX() const       { return width / charWidth(); }
    [[nodiscard]] uint8_t maxCharsY() const       { return height / charHeight(); }
    [[nodiscard]] uint8_t availableCharsX() const { return (width - x) / charWidth(); }
    [[nodiscard]] uint8_t availableCharsY() const { return (height - y) / charHeight(); }

    [[nodiscard]] uint8_t availableSpaces(uint8_t chars) const;

    [[nodiscard]] bool refreshRequested() const { return refresh_requested; }

    void sync();
    void reset();
    void render(UIApp& app);
    void refresh(bool full = false);
    void setCursor(int16_t tx, int16_t ty);
    void setCharCursor(int16_t cx, int16_t cy);
    void setRotation(uint8_t r);

    void setTextColor(uint16_t c, uint16_t bg);
    void setTextColor(uint16_t c) { setTextColor(c, theme.bg); }
    void setTextColor(uint32_t c, uint32_t bg);
    void setTextColor(uint32_t c);
    void invertColors();
    void resetColors();

    void print(const String& text, bool colorized = false);
    void print(const char text, bool colorized = false)                 { print(String(text), colorized); }

    void println(const String& text = String{}, bool colorized = false) { print(text + "\n", colorized); }
    void println(const char* text, bool colorized = false)              { println(String(text), colorized); }
    void println(const char text, bool colorized = false)               { println(String(text), colorized); };

    void printf(const char* format, ...);
    void printfColor(const char* format, ...);
};
