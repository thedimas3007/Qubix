#include <type_traits>

#include "ui_core.h"
#include "configuration.h"
#include "keycodes.h"

bool UIElement::update(char key) {
    return false;
}


void Stack::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(title);
    } else {
        if (title != "") {
            display->println(title);
        }
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


void MenuView::checkPointer() {
    if (cursor < start) {
        start = cursor;
    } else if (cursor >= min(start + window_size, children.size())) {
        start = max(0, cursor - (window_size - 1));
    }
}

bool MenuView::update(char key) {
    if (selected == nullptr) {
        const int16_t n = children.size();
        if (key == 0 || n <= 0) return false;

        if (key == KEY_UP) {
            cursor = (cursor - 1 + n) % n;
            checkPointer();
            return true;
        } if (key == KEY_DOWN) {
            cursor = (cursor + 1) % n;
            checkPointer();
            return true;
        } if (key == KEY_RIGHT || key == KEY_ENTER) {
            selected = children[cursor];
            checkPointer();
            return true;
        }

        return false;
    } /* else if (selected != nullptr) */ {
        if (selected->update(key)) {
            return true;
        }

        if (key == KEY_LEFT || key == KEY_ESC) {
            selected = nullptr;
            return true;
        }

        return false;
    }
}

void MenuView::render(Adafruit_GFX* display, bool minimalized) {
    const int16_t n = children.size();
    const int16_t last = std::min<int16_t>(start + window_size, n);

    if (selected == nullptr) {
        if (minimalized) {
            display->println(icon + title);
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


void Label::render(Adafruit_GFX* display, bool minimalized) {
    display->println(text);
}

static int64_t pow10i(uint8_t n) {
    int64_t p = 1;
    while (n--) p *= 10;
    return p;
}

template<class T>
uint8_t NumberPicker<T>::getDigits() const {
    T v = getAbsoluteValue();
    uint8_t intDigits = v >= static_cast<T>(1) ?
                        static_cast<uint8_t>(std::floor(std::log10(v)) + 1) : 1;
    return intDigits + precision;
}

template <class T>
T NumberPicker<T>::getAbsoluteValue() const {
    if (std::is_floating_point<T>::value) {
        return fabs(getValue());
    }
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
        *number = std::round((*number) * p) / p;
    } else {}
}

template<class T>
void NumberPicker<T>::render(Adafruit_GFX* display, bool /*minimalized*/) {
    display->print(text + " ");

    T v = getValue();
    if (v < 0) display->print("-");

    const uint8_t total = getDigits();
    const int64_t scale = pow10i(precision);
    int64_t scaled = static_cast<int64_t>(std::llround(getAbsoluteValue() * scale));

    for (int i = total; i >= 1; --i) {
        if (i == precision) display->print(".");
        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);
        display->print(digit);
    }
    display->println();
}

template<class T>
void NumberPicker<T>::renderInline(Adafruit_GFX* display) {
    display->print(text + " ");

    T v = getValue();
    if (v < 0) display->print("-");

    const uint8_t total = getDigits();
    const int64_t scale = pow10i(precision);
    int64_t scaled = static_cast<int64_t>(std::llround(getAbsoluteValue() * scale));

    for (int i = total; i >= 1; --i) {
        if (i == precision) display->print(".");

        if (i == cursor + 1) {
            display->setTextColor(DISPLAY_BG, DISPLAY_FG);
        } else {
            display->setTextColor(DISPLAY_FG, DISPLAY_BG);
        }

        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);
        display->print(digit);
        display->setTextColor(DISPLAY_FG, DISPLAY_BG);
    }
    display->println();
}

template<class T>
bool NumberPicker<T>::update(char key) {
    const uint8_t total = getDigits();

    if (key == KEY_UP) {
        T step = static_cast<T>(std::pow(10.0, static_cast<int>(cursor) - static_cast<int>(precision)));
        T top = std::min(maximum, std::numeric_limits<T>::max());

        if (top - step <= *number) {
            *number = top;
        } else {
            *number += step;
        }
        roundToPrecision();

        uint8_t newTotal = getDigits();
        if (total > newTotal && cursor == total - 1 && cursor != 0) {
            cursor--;
        }
    } else if (key == KEY_DOWN) {
        T step = static_cast<T>(std::pow(10.0, static_cast<int>(cursor) - static_cast<int>(precision)));
        T bottom = std::max(minimum, std::numeric_limits<T>::min());

        if (bottom + step >= *number) {
            *number = bottom;
        } else {
            *number -= step;
        }
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

template <class T> void Property<T>::render(Adafruit_GFX* display, bool minimalized) {
    char buf[16];
    sprintf(buf, format, *ptr);

    uint8_t spaces = 20 - (text.length() + strlen(buf)); // 20 - characters per line available; 21 - 1 (arrow)

    if (icon) {
        display->print(icon);
    }
    display->print(text);
    for (uint8_t i = 0; i < spaces; i++) {
        display->print(" ");
    }
    display->println(buf);
}

void CharTable::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(text);
    } else {
        display->println("  |0123456789ABCDEF| ");
        display->println(" -+----------------+-");
        for (uint8_t y = start; y < start+max_lines; y++) {
            if (y == 0x10) {
                display->println(" -+----------------+-");
            } else {
                display->print(" ");
                display->print(y, HEX);
                display->print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = x + y * 16;
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
        if (start > 16-max_lines+1) start--;
    } else {
        return false;
    }

    return true;
}

