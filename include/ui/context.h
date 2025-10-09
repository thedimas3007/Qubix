#pragma once

#include <Adafruit_GFX.h>

class UIContext {
    const uint8_t glyph_width = 6, glyph_height = 8;
    uint8_t text_size = 1;

public:
    Adafruit_GFX& display;
    int16_t x, y, width, height;

    explicit UIContext(Adafruit_GFX& display)
        : display(display), x(0), y(0), width(display.width()), height(display.height()) {}

    int16_t charWidth() const       { return glyph_width * text_size; }
    int16_t charHeight() const      { return glyph_height * text_size; }

    uint8_t maxCharsX() const       { return width / charWidth(); }
    uint8_t maxCharsY() const       { return height / charHeight(); }
    uint8_t availableCharsX() const { return (width - x) / charWidth(); }

    void sync();
    void reset();
    void setCursor(int16_t tx, int16_t ty);
    void setCharCursor(int16_t cx, int16_t cy);
    void setRotation(uint8_t r);

    void print(const String& text);
    void print(const char text)                 { print(String(text)); }

    void println(const String& text = String{}) { print(text + '\n'); }
    void println(const char* text)              { println(String(text)); }
    void println(const char text)               { println(String(text)); };

    void printf(const char* text, ...);
};
