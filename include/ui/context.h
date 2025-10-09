#pragma once

#include <Adafruit_GFX.h>

struct UITheme {
    uint8_t foreground;
    uint8_t background;

    uint8_t& fg = foreground;
    uint8_t& bg = background;
};

class UIContext {
    const uint8_t glyph_width = 6, glyph_height = 8;
    uint8_t text_size = 1;

public:
#ifdef THEME
    UITheme theme = THEME;
#else
    UITheme theme = {1,0};
#endif

    Adafruit_GFX& display;
    int16_t x, y, width, height;

    explicit UIContext(Adafruit_GFX& display)
        : display(display), x(0), y(0), width(display.width()), height(display.height()) {}

    [[nodiscard]] int16_t charWidth() const       { return glyph_width * text_size; }
    [[nodiscard]] int16_t charHeight() const      { return glyph_height * text_size; }

    [[nodiscard]] uint8_t maxCharsX() const       { return width / charWidth(); }
    [[nodiscard]] uint8_t maxCharsY() const       { return height / charHeight(); }
    [[nodiscard]] uint8_t availableCharsX() const { return (width - x) / charWidth(); }

    void sync();
    void reset();
    void setCursor(int16_t tx, int16_t ty);
    void setCharCursor(int16_t cx, int16_t cy);
    void setRotation(uint8_t r);

    void setTextColor(uint8_t c, uint8_t bg);
    void setTextColor(uint8_t c) { setTextColor(c, theme.bg); };
    void invertColors();
    void resetColors();

    void print(const String& text);
    void print(const char text)                 { print(String(text)); }

    void println(const String& text = String{}) { print(text + '\n'); }
    void println(const char* text)              { println(String(text)); }
    void println(const char text)               { println(String(text)); };

    void printf(const char* text, ...);
};
