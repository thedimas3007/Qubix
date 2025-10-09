#include "ui/context.h"

#include "configuration.h"

void UIContext::setCursor(const int16_t tx, const int16_t ty) {
    assert(tx >= 0 && ty >= 0);
    assert(tx < width && ty < height);
    display.setCursor(tx, ty);
    sync();
}

void UIContext::setCharCursor(const int16_t cx, const int16_t cy) {
    setCursor(cx*charWidth(), cy*charHeight());
}

void UIContext::setRotation(uint8_t r) {
    display.setRotation(r);
    sync();
}

void UIContext::setTextColor(uint8_t c, uint8_t bg) {
    display.setTextColor(c, bg);
}

void UIContext::invertColors() {
    setTextColor(theme.background, theme.foreground);
}

void UIContext::resetColors() {
    setTextColor(theme.foreground, theme.background);
}

void UIContext::print(const String& text) {
    display.setCursor(x, y);
    display.print(text);
    sync();
}

void UIContext::printf(const char* format, ...) {
    char buffer[128];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(String(buffer));
}

uint8_t UIContext::availableSpaces(uint8_t chars) const {
    return availableCharsX() > chars ? availableCharsX() - chars : 0;
}

void UIContext::sync() {
    x = display.getCursorX();
    y = display.getCursorY();
    width = display.width();
    height = display.height();
}

void UIContext::reset() {
    display.fillScreen(theme.background);
    setCursor(0, 0);
    resetColors();
    sync();
}
