#include "ui/context.h"

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

void UIContext::print(const String& text) {
    display.setCursor(x, y);
    display.print(text);
    sync();
}

void UIContext::sync() {
    x = display.getCursorX();
    y = display.getCursorY();
    width = display.width();
    height = display.height();
}