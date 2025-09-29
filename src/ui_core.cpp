#include <type_traits>
#include <cmath>
#include <cstdio>
#include <limits>
#include <algorithm>

#include "ui_core.h"
#include "configuration.h"
#include "keycodes.h"

static int64_t pow10i(uint8_t n) {
    int64_t p = 1;
    while (n--) p *= 10;
    return p;
}

#pragma region UIElement

bool UIElement::update(char /*key*/) {
    return false;
}

#pragma endregion


#pragma region Stack

void Stack::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(title);
    } else {
        if (title != "") display->println(title);
        for (auto &i : children) {
            i->render(display, false);
        }
    }
}

bool Stack::update(char key) {
    bool updated = false;
    for (auto &i : children) {
        if (i->update(key)) updated = true;
    }
    return updated;
}

#pragma endregion


#pragma region MenuView

void MenuView::checkPointer() {
    const int16_t n = static_cast<int16_t>(children.size());
    const int16_t lastStart = std::max<int16_t>(0, n - window_size);

    if (cursor < start) {
        start = cursor;
    } else if (cursor >= start + window_size) {
        start = cursor - (window_size - 1);
    }

    if (start < 0) start = 0;
    if (start > lastStart) start = lastStart;
}

bool MenuView::update(char key) {
    if (selected == nullptr) {
        const int16_t n = static_cast<int16_t>(children.size());
        if (key == 0 || n <= 0) return false;

        if (key == KEY_UP) {
            cursor = (cursor - 1 + n) % n;
            checkPointer();
            return true;
        }
        if (key == KEY_DOWN) {
            cursor = (cursor + 1) % n;
            checkPointer();
            return true;
        }
        if (key == KEY_RIGHT || key == KEY_ENTER) {
            selected = children[cursor];
            checkPointer();
            return true;
        }

        return false;
    } else {
        if (selected->update(key)) return true;

        if (key == KEY_LEFT || key == KEY_ESC) {
            selected = nullptr;
            on_exit();
            return true;
        }

        return false;
    }
}

void MenuView::render(Adafruit_GFX* display, bool minimalized) {
    const int16_t n = static_cast<int16_t>(children.size());
    const int16_t last = std::min<int16_t>(start + window_size, n);

    if (selected == nullptr) {
        if (minimalized) {
            display->println(String(icon) + title);
        } else {
            display->println(title);
            for (int16_t i = start; i < last; ++i) {
                display->print(i == cursor ? "\x1A" : " ");
                children[i]->render(display, true);
            }
        }
    } else {
        if (selected->isInline()) {
            display->println(title);
            for (int16_t i = start; i < last; ++i) {
                display->print(i == cursor ? "\x1A" : " ");
                if (children[i] == selected) {
                    static_cast<UIInline*>(children[i])->renderInline(display);
                } else {
                    children[i]->render(display, true);
                }
            }
        } else {
            selected->render(display, false);
        }
    }
}

#pragma endregion


#pragma region Label

void Label::render(Adafruit_GFX* display, bool /*minimalized*/) {
    display->println(title);
}

#pragma endregion


#pragma region Property

template <class T>
void Property<T>::render(Adafruit_GFX* display, bool /*minimalized*/) {
    String data = "";
    if (with_values && std::is_integral<T>::value) {
        if (*ptr < 0) {
            data = values.empty() ? "<null>" : values.front();
        } else if (*ptr >= values.size()) {
            data = values.back();
        } else {
            data = values[*ptr];
        }
    } else {
        char buf[16];
        std::snprintf(buf, sizeof(buf), format, *ptr);
        data = String(buf);
    }

    String prefix = icon ? (String(icon) + title) : title;
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? uint8_t(20 - (prefix.length() + data.length()))
                     : 0;

    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');
    display->println(data);
}

template void Property<uint8_t>::render(Adafruit_GFX*, bool);
template void Property<int8_t>::render(Adafruit_GFX*, bool);
template void Property<float>::render(Adafruit_GFX*, bool);

#pragma endregion


#pragma region StringProperty

void StringProperty::render(Adafruit_GFX* display, bool minimalized) {
    String prefix = icon ? (String(icon) + title) : title;
    String data = ptr != nullptr ? *ptr : "<null>";
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? uint8_t(20 - (prefix.length() + data.length()))
                     : 0;
    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');
    display->println(data);
}

#pragma endregion


#pragma region NumberPicker

template<class T>
uint8_t NumberPicker<T>::getDigits() const {
    T v = getAbsoluteValue();
    uint8_t intDigits = v >= static_cast<T>(1)
                        ? static_cast<uint8_t>(std::floor(std::log10(static_cast<double>(v))) + 1)
                        : 1;
    return static_cast<uint8_t>(intDigits + precision);
}

template <class T>
T NumberPicker<T>::getAbsoluteValue() const {
    return abs(getValue());
}

template <class T>
T NumberPicker<T>::getValue() const {
    return *number;
}

template <class T>
void NumberPicker<T>::roundToPrecision() {
    const int64_t p = pow10i(precision);
    if (std::is_floating_point<T>::value) {
        *number = static_cast<T>(std::round(static_cast<double>((*number) * p)) / p);
    }
}

template<class T>
void NumberPicker<T>::render(Adafruit_GFX* display, bool /*minimalized*/) {
    String prefix = icon ? (String(icon) + title) : title;
    T v = getValue();
    const uint8_t total = getDigits();
    const uint8_t spaces = (20 > (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           ? uint8_t(20 - (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           : 0;
    const int64_t scale = pow10i(precision);
    int64_t scaled = static_cast<int64_t>(std::llround(static_cast<double>(getAbsoluteValue()) * scale));

    display->print(prefix);
    for (uint8_t i = 0; i < spaces; ++i) display->print(' ');
    if (v < 0) display->print("-");

    for (int i = total; i >= 1; --i) {
        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);
        if (i == precision) display->print(".");
        display->print(digit);
    }

    display->println(suffix);
}

template<class T>
void NumberPicker<T>::renderInline(Adafruit_GFX* display) {
    String prefix = icon ? (String(icon) + title) : title;
    T v = getValue();
    const uint8_t total = getDigits();
    const uint8_t spaces = (20 > (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           ? uint8_t(20 - (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           : 0;
    const int64_t scale = pow10i(precision);
    int64_t scaled = static_cast<int64_t>(std::llround(static_cast<double>(getAbsoluteValue()) * scale));

    display->print(prefix);
    for (uint8_t i = 0; i < spaces; ++i) display->print(' ');
    if (v < 0) display->print("-");

    for (int i = total; i >= 1; --i) {
        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);

        if (i == precision) display->print(".");

        if (i == cursor + 1)    display->setTextColor(DISPLAY_BG, DISPLAY_FG);
        else                    display->setTextColor(DISPLAY_FG, DISPLAY_BG);

        display->print(digit);
        display->setTextColor(DISPLAY_FG, DISPLAY_BG);
    }

    display->println(suffix);
}

template<class T>
bool NumberPicker<T>::update(char key) {
    const uint8_t total = getDigits();

    if (key == KEY_UP) {
        T step = static_cast<T>(std::pow(10.0, static_cast<int>(cursor) - static_cast<int>(precision)));
        T top = std::min(maximum, std::numeric_limits<T>::max());

        if (top - step <= *number)  *number = top;
        else                        *number += step;
        roundToPrecision();

        uint8_t newTotal = getDigits();
        if (total > newTotal && cursor == total - 1 && cursor != 0) {
            cursor--;
        }
    } else if (key == KEY_DOWN) {
        T step = static_cast<T>(std::pow(10.0, static_cast<int>(cursor) - static_cast<int>(precision)));
        T bottom = std::max(minimum, std::numeric_limits<T>::min());

        if (bottom + step >= *number)   *number = bottom;
        else                            *number -= step;
        roundToPrecision();

        uint8_t newTotal = getDigits();
        if (total > newTotal && cursor == total - 1 && cursor != 0) {
            cursor--;
        }
    } else if (key == KEY_RIGHT) {
        cursor = (cursor == 0) ? (total - 1) : (cursor - 1);
    } else if (key == KEY_LEFT) {
        cursor = (cursor + 1) % total;
    } else {
        return false;
    }

    return true;
}

#pragma endregion


#pragma region Selector

void Selector::render(Adafruit_GFX* display, bool /* minimalized */) {
    String prefix = icon ? (String(icon) + title) : title;
    const String& current = items.at(cursor);
    uint8_t spaces = (20 > (prefix.length() + current.length()))
                     ? uint8_t(20 - (prefix.length() + current.length()))
                     : 0;

    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');
    display->println(current);
}

void Selector::renderInline(Adafruit_GFX* display) {
    String prefix = icon ? (String(icon) + title) : title;
    const String& current = items.at(cursor);
    uint8_t spaces = (20 > (prefix.length() + current.length()))
                     ? uint8_t(20 - (prefix.length() + current.length()))
                     : 0;

    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');

    display->setTextColor(DISPLAY_BG, DISPLAY_FG);
    display->print(current);
    display->setTextColor(DISPLAY_FG, DISPLAY_BG);
}

bool Selector::update(char key) {
    if (key == KEY_LEFT) {
        cursor--;
        if (cursor < 0) {
            cursor = static_cast<int8_t>(items.size()) - 1;
        }
    } else if (key == KEY_RIGHT) {
        cursor++;
        cursor = static_cast<int8_t>(cursor % static_cast<int>(items.size()));
    } else {
        return false;
    }
    if (selection != nullptr) *selection = cursor;
    return true;
}

#pragma endregion


#pragma region Input

void Input::render(Adafruit_GFX* display, bool minimalized) {
    String prefix = icon ? (String(icon) + title) : title;
    String data = ptr != nullptr ? *ptr : "<null>";
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                 ? uint8_t(20 - (prefix.length() + data.length()))
                 : 0;
    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');
    display->println(data);
}

void Input::renderInline(Adafruit_GFX* display) {
    String prefix = icon ? (String(icon) + title) : title;
    String data = ptr != nullptr ? *ptr : "<null>";
    bool has_cursor = ptr != nullptr && data.length() < max_length;
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                 ? uint8_t(20 - (prefix.length() + data.length() + has_cursor))
                 : 0;
    display->print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display->print(' ');
    display->print(data);
    if (has_cursor) display->println((millis() / 500 % 2) ? '_' : ' ');
}

bool Input::update(char key) {
    if (ptr == nullptr) return false;

    if (key >= ' ' && key <= '~') {
        if (ptr->length() < max_length) {
            *ptr += key;
            cursor++;
        }
    } else if (key == KEY_BACK) {
        if (cursor > 0) {
            *ptr = ptr->substring(0, --cursor);
        }
    } else {
        return false;
    }
    return true;
}

#pragma endregion


#pragma region CharTable

void CharTable::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(title);
    } else {
        display->println("  |0123456789ABCDEF| ");
        display->println(" -+----------------+-");
        for (uint8_t y = static_cast<uint8_t>(start); y < static_cast<uint8_t>(start + max_lines); y++) {
            if (y == 0x10) {
                display->println(" -+----------------+-");
            } else {
                display->print(' ');
                display->print(y, HEX);
                display->print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = static_cast<char>(x + y * 16);
                    if (c == 0x0A || c == 0x0D || c == '\n' || c == '\t') c = ' ';
                    display->print(c);
                }
                display->print("|");
                display->println(y, HEX);
            }
        }
    }
}

bool CharTable::update(char key) {
    if (key == KEY_UP) {
        start--;
        if (start < 0) start++;
    } else if (key == KEY_DOWN) {
        start++;
        if (start > 16 - static_cast<int>(max_lines) + 1) start--;
    } else {
        return false;
    }

    return true;
}

#pragma endregion
