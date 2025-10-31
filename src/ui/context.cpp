#include "ui/context.h"
#include "configuration.h"
#include "ui/base.h"

uint16_t rgb565(uint32_t rgb) {
    uint8_t r = (rgb & 0xFF0000) >> 16;
    uint8_t g = (rgb & 0x00FF00) >> 8;
    uint8_t b = rgb & 0x0000FF;

    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}


void UIContext::setCursor(const int16_t tx, const int16_t ty) {
    // assert(tx >= 0 && ty >= 0);
    // assert(tx < width && ty < height);
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

void UIContext::setTextColor(uint16_t c, uint16_t bg) {
    display.setTextColor(c, bg);
}

void UIContext::setTextColor(uint32_t c, uint32_t bg) {
    setTextColor(rgb565(c), rgb565(bg));
}

void UIContext::setTextColor(uint32_t c) {
    setTextColor(rgb565(c), theme.bg);
}

void UIContext::invertColors() {
    setTextColor(theme.background, theme.foreground);
}

void UIContext::resetColors() {
    setTextColor(theme.foreground, theme.background);
}

void UIContext::print(const String& text, const bool colorized) {
    display.setCursor(x, y);

    if (!colorized) {
        display.print(text);
        sync();
        return;
    }

#ifdef HAS_COLOR
    std::vector<uint16_t> color_stack{};
    int32_t bracket_pos = -1;
    String color_buf = "";
    resetColors();
    for (uint16_t i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        if (c == '[') {
            if (bracket_pos == -1) {
                bracket_pos = i;
                color_buf = "";
            } else if (bracket_pos == i - 1) {
                display.print('['); // [[ -> [
                bracket_pos = -1;
            }
            continue;
        }

        if (bracket_pos >= 0) {
            uint16_t innerPos = i - bracket_pos;

            if (innerPos == 1 && c != ']') {
                if (c == '#') {
                    color_buf = "#";
                } else {
                    display.print("[#");
                    bracket_pos = -1;
                    color_buf = "";
                }
                continue;
            }

            if (isxdigit(c)) {
                if (color_buf.length() < 7)
                    color_buf += c;
                else {
                    display.print("[");
                    display.print(color_buf);
                    bracket_pos = -1;
                    color_buf = "";
                }
                continue;
            }

            if (c == ']') {
                if (innerPos == 1) {  // []
                    if (!color_stack.empty()) color_stack.pop_back();
                    setTextColor(!color_stack.empty() ? color_stack.back() : theme.foreground);
                } else if (innerPos == 2) {  // [#]
                    color_stack.clear();
                    setTextColor(theme.foreground);
                } else {
                    uint16_t color = rgb565(strtoul(color_buf.c_str() + 1, nullptr, 16));
                    setTextColor(color);
                    color_stack.push_back(color);
                }
                bracket_pos = -1;
                color_buf = "";
                continue;
            }

            display.print("[");
            display.print(color_buf);
            bracket_pos = -1;
            color_buf = "";
        }

        display.print(c);
    }
    // resetColors();
#else
    for (uint16_t i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        if (c == '[') {
            int j = i + 1;
            if (j < text.length() && text.charAt(j) == '[') {
                display.print('[');
                i = j;
            } else {
                bool is_tag = false;
                int end = j;
                while (end < text.length() && text.charAt(end) != ']') {
                    end++;
                }
                if (end < text.length() && text.charAt(end) == ']') {
                    is_tag = true;
                    i = end;
                }

                if (!is_tag) {
                    display.print('[');
                }
            }
        } else {
            display.print(c);
        }
    }
#endif
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

void UIContext::printfColor(const char* format, ...) {
    char buffer[128];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(String(buffer), true);
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

void UIContext::render(UIApp& app) {
    reset();
#if   DISPLAY_MODE == DISPLAY_MODE_BUFFERED
    app.render(*this);
    display.display();

#elif DISPLAY_MODE == DISPLAY_MODE_EINK
    if (refresh_full || ++partial_updates > partial_cap) {
        display.setFullWindow();
        partial_updates = 0;
    } else {
        display.setPartialWindow(0, 0, width, height);
    }

    display.firstPage();
    do {
        app.render(*this);
    } while (display.nextPage());

#else
#error "Invalid display mode specified. Check implementation"
#endif

    refresh_requested = false;
    refresh_full = false;
}

void UIContext::refresh(bool full) {
    refresh_requested = true;
    refresh_full = full || refresh_full;
}
