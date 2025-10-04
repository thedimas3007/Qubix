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

#pragma region UIApp

UIModal* UIApp::eraseFirstModal() {
    if (modals.empty()) {
        return nullptr;
    }

    UIModal* m = modals.front();
    modals.erase(modals.begin());
    return m;
}

void UIApp::render(Adafruit_GFX& display) const {
    if (title) display.print(title);
    root->render(display, false);

    if (hasModals()) {
        for (int16_t y = 0; y < display.height(); y++) {
            for (int16_t x = 0; x < display.width(); x++) {
                if ((x + y) % 2) display.writePixel(x, y, settings.data.display_inv_alert ? DISPLAY_FG : DISPLAY_BG);
            }
        }
        display.setCursor(0, 0);
        modals.front()->render(display);
    }
}

bool UIApp::update(char key) {
    if (hasModals()) {
        bool result = modals[0]->update(key);
        if (!result) return false;

        delete eraseFirstModal();
        return true;
    }

    return root->update(key);
}

#pragma endregion


#pragma region MenuView

void MenuView::addChild(UIElement* e) {
    children.push_back(e);
    cursor = children.size() - 1;
    if (cursor > window_size+slice_at-1) slice_at++;
}

void MenuView::render(Adafruit_GFX& display, bool minimalized) {
    const int16_t n = children.size();
    const int16_t last = std::min<int16_t>(slice_at + window_size, n);

    if (selected == nullptr) {
        if (minimalized) {
            display.println(getLabel());
        } else {
            if (title.length()) display.println(title);
            if (fill_mode == FillMode::TOP && n < window_size) {
                for (int16_t i = 0; i < window_size-n; i++) display.println();
            }

            for (int16_t i = slice_at; i < last; ++i) {
                display.print(i == cursor && active ? "\x1A" : " ");
                children[i]->render(display, true);
            }

            if (fill_mode == FillMode::BOTTOM && n < window_size) {
                for (uint16_t i = 0; i < window_size-n; i++) display.println();
            }
        }
    } else {
        if (selected->getType() == ElementType::INLINE) {
            display.println(title);
            for (int16_t i = slice_at; i < last; ++i) {
                display.print(i == cursor && active ? "\x1A" : " ");
                if (children[i] == selected) {
                    static_cast<UIInline*>(children[i])->renderInline(display);
                } else {
                    children[i]->render(display, true);
                }
            }
        } else {
            // NOTE: Clickable can't be normally selected
            selected->render(display, false);
        }
    }
}

bool MenuView::update(char key) {
    if (selected == nullptr) {
        const int16_t n = children.size();
        if (key == 0 || n <= 0) return false;

        if (key == KEY_UP) {
            if (cursor > 0) {
                cursor--;
                if (cursor < slice_at+1 && cursor > 0) slice_at--;
            } else {
                cursor = n-1;
                slice_at = (n > window_size) ? (n - window_size) : 0;
            }
            return true;
        }
        if (key == KEY_DOWN) {
            if (cursor < n-1) {
                cursor++;
                if ((cursor > slice_at+window_size-2 && cursor < n-1 ||
                    cursor > slice_at+window_size-1) &&
                    cursor < n) slice_at++;
            } else {
                cursor = 0;
                slice_at = 0;
            }
            return true;
        }
        if (key == KEY_RIGHT || key == KEY_ENTER) {
            auto element = children[cursor];
            if (element->getType() == ElementType::CLICKABLE) {
                static_cast<UIClickable*>(element)->activate(display);
                on_exit();
            } else {
                selected = children[cursor];
            }
            // checkPointer();
            return true;
        }
    } else {
        if (selected->update(key)) return true;

        if (key == KEY_LEFT || key == KEY_ESC) {
            selected = nullptr;
            on_exit();
            return true;
        }
    }

    return false;
}

#pragma endregion


#pragma region TabSelector

void TabSelector::render(Adafruit_GFX& display, bool minimalized) {
    if (minimalized) {
        display.println(getLabel());
    } else {
        for (int i = 0; i < children.size(); ++i) {
            auto& element = children[i];

            if (element->getType() == ElementType::ACTIVE) {
                static_cast<UIActive*>(element)->setActive(i==cursor);
            }

            if (element->getType() == ElementType::INLINE && i == cursor) {
                static_cast<UIInline*>(element)->renderInline(display);
            } else {
                element->render(display, false);
            }
        }
    }
}

bool TabSelector::update(char key) {
    if (key == KEY_TAB) {
        cursor = (cursor + 1) % children.size();
        return true;
    }

    return children[cursor]->update(key);
}


#pragma endregion


#pragma region Label

void Label::render(Adafruit_GFX& display, bool minimalized) {
    String data = getLabel();
    if (max_length && data.length() > max_length && minimalized) {
        display.println(data.substring(0, max_length-1) + '\x96');
    } else if (minimalized) {
        display.println(data);
    } else {
        display.println(title);
    }
}

#pragma endregion


#pragma region Property

template <class T>
void Property<T>::render(Adafruit_GFX& display, bool /*minimalized*/) {
    String data = "";
    if (with_values && std::is_integral_v<T>) {
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

    String prefix = getLabel();
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? (20 - (prefix.length() + data.length()))
                     : 0;

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display.print(' ');
    display.println(data);
}

#pragma endregion


#pragma region StringProperty

void StringProperty::render(Adafruit_GFX& display, bool minimalized) {
    String prefix = getLabel();
    String data = ptr != nullptr ? String(ptr) : "<null>";
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? (20 - (prefix.length() + data.length()))
                     : 0;
    display.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display.print(' ');
    display.println(data);
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
    T value = getValue();
    return value < 0 ? -value : value;
}

template <class T>
T NumberPicker<T>::getValue() const {
    return *number;
}

template <class T>
void NumberPicker<T>::roundToPrecision() {
    const int64_t p = pow10i(precision);
    if (std::is_floating_point_v<T>) {
        *number = static_cast<T>(std::round(static_cast<double>((*number) * p)) / p);
    }
}

template<class T>
void NumberPicker<T>::render(Adafruit_GFX& display, bool /*minimalized*/) {
    String prefix = getLabel();
    T v = getValue();
    const uint8_t total = getDigits();
    const uint8_t spaces = (20 > (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           ? (20 - (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           : 0;
    const int64_t scale = pow10i(precision);
    int64_t scaled = std::llround(static_cast<double>(getAbsoluteValue()) * scale);

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; ++i) display.print(' ');
    if (v < 0) display.print("-");

    for (int i = total; i >= 1; --i) {
        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);
        if (i == precision) display.print(".");
        display.print(digit);
    }

    display.println(suffix);
}

template<class T>
void NumberPicker<T>::renderInline(Adafruit_GFX& display) {
    String prefix = getLabel();
    T v = getValue();
    const uint8_t total = getDigits();
    const uint8_t spaces = (20 > (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           ? (20 - (prefix.length() + total + (precision > 0) + suffix.length() + (v < 0)))
                           : 0;
    const int64_t scale = pow10i(precision);
    int64_t scaled = std::llround(static_cast<double>(getAbsoluteValue()) * scale);

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; ++i) display.print(' ');
    if (v < 0) display.print("-");

    for (int i = total; i >= 1; --i) {
        int64_t div = pow10i(i - 1);
        int digit = static_cast<int>((scaled / div) % 10);

        if (i == precision) display.print(".");

        if (i == cursor + 1)    display.setTextColor(DISPLAY_BG, DISPLAY_FG);
        else                    display.setTextColor(DISPLAY_FG, DISPLAY_BG);

        display.print(digit);
        display.setTextColor(DISPLAY_FG, DISPLAY_BG);
    }

    display.println(suffix);
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
    } else if (key == KEY_FN_UP) {
        // I can actually use step here
        *number = std::min(maximum, std::numeric_limits<T>::max());
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
    } else if (key == KEY_FN_DOWN) {
        *number = std::max(minimum, std::numeric_limits<T>::min());
    } else if (key == KEY_LEFT) {
        cursor = (cursor + 1) % total;
    } else if (key == KEY_FN_LEFT) {
        cursor = total - 1;
    } else if (key == KEY_RIGHT) {
        cursor = (cursor == 0) ? (total - 1) : (cursor - 1);
    } else if (key == KEY_FN_RIGHT) {
        cursor = 0;
    } else {
        return false;
    }

    return true;
}

#pragma endregion


#pragma region Selector

void Selector::render(Adafruit_GFX& display, bool /* minimalized */) {
    String prefix = getLabel();
    String current = items.at(cursor) + suffix;
    uint8_t spaces = (20 > (prefix.length() + current.length()))
                     ? (20 - (prefix.length() + current.length()))
                     : 0;

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display.print(' ');
    display.println(current);
}

void Selector::renderInline(Adafruit_GFX& display) {
    String prefix = getLabel();
    String current = items.at(cursor) + suffix;
    uint8_t spaces = (20 > (prefix.length() + current.length()))
                     ? (20 - (prefix.length() + current.length()))
                     : 0;

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display.print(' ');

    display.setTextColor(DISPLAY_BG, DISPLAY_FG);
    display.print(current);
    display.setTextColor(DISPLAY_FG, DISPLAY_BG);
}

bool Selector::update(char key) {
    if (key == KEY_LEFT) {
        cursor--;
        if (cursor < 0) cursor = items.size() - 1;
    } else if (key == KEY_FN_LEFT) {
        cursor = 0;
    } else if (key == KEY_RIGHT) {
        cursor++;
        cursor = cursor % items.size();
    } else if (key == KEY_FN_RIGHT) {
        cursor = items.size() - 1;
    } else {
        return false;
    }
    if (selection != nullptr) *selection = cursor;
    return true;
}

#pragma endregion


#pragma region Toggle

void Toggle::render(Adafruit_GFX& display, bool minimalized) {
    String prefix = getLabel();
    uint8_t spaces = (20 > (prefix.length() + 3))
                     ? (20 - (prefix.length() + 3))
                     : 0;

    display.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) display.print(' ');
    display.print('[');
    display.print(*ptr ? 'x' : ' ');
    display.println(']');
}

void Toggle::activate(Adafruit_GFX& /* display */) {
    *ptr = !*ptr;
}

#pragma endregion


#pragma region Button

void Button::render(Adafruit_GFX& display, bool minimalized) {
    display.println("[" + getLabel() + "]");
}

void Button::activate(Adafruit_GFX& display) {
    on_click();
}


#pragma endregion


#pragma region TextField

void TextField::render(Adafruit_GFX& display, bool minimalized) {
    if (ptr == nullptr) return;
    if (cursor == -1) cursor = strlen(ptr);

    String prefix = getLabel();
    String data = ptr;
    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? (20 - (prefix.length() + data.length()))
                     : 0;
    if (prefix.length() == 0 || !spacer) spaces = 0;

    display.print(prefix);
    for (uint8_t i = 0; i < (spacer ? spaces : 0); i++) display.print(' ');

    if (data.length() <= window_size) {
        display.println(data);
    } else {
        data = data.substring(window_size-1);
        display.println(data);
        display.print('\x96');
    }
}

void TextField::renderInline(Adafruit_GFX& display) {
    if (ptr == nullptr) return;
    if (cursor == -1) cursor = strlen(ptr);

    String prefix = getLabel();
    String data = ptr;
    uint8_t size = data.length();
    bool has_cursor = data.length() < max_length && cursor == data.length();
    if (has_cursor && data.length() > window_size-1) {
        data = data.substring(slice_at+1, std::min<uint8_t>(slice_at+window_size, data.length()));
    } else {
        data = data.substring(slice_at, std::min<uint8_t>(slice_at+window_size, data.length()));
    }

    if (slice_at || (has_cursor && size > window_size-1)) data.setCharAt(0, '\x96'); // left trim
    if (slice_at+window_size < size) data.setCharAt(data.length()-1, '\x96'); // right trim

    uint8_t spaces = (20 > (prefix.length() + data.length()))
                     ? (20 - (prefix.length() + data.length() + has_cursor))
                     : 0;

    if (prefix.length() == 0 || !spacer) spaces = 0;

    display.print(prefix);
    for (uint8_t i = 0; i < (spacer ? spaces : 0); i++) display.print(' ');

    for (uint8_t i = 0; i < data.length(); i++) {
        if (i == cursor-slice_at) {
            uint16_t fg = DISPLAY_FG;
            uint16_t bg = DISPLAY_BG;
            if (millis() / 500 % 2) fg = DISPLAY_BG; bg = DISPLAY_FG;

            display.setTextColor(fg, bg);
            display.print(data.charAt(i));
            display.setTextColor(DISPLAY_FG, DISPLAY_BG);
            has_cursor = false;
        } else {
            display.print(data.charAt(i));
        }
    };

    if (has_cursor) display.print((millis() / 500 % 2) ? '_' : ' ');
    display.println();
}

bool TextField::update(char key) {
    if (ptr == nullptr) return false;

    String buf = ptr; // I know using Strings may be inefficient, but it is much more convenient
    if (key >= ' ' && key <= '~') {
        if (buf.length() < max_length) {
            String start = buf.substring(0, cursor);
            String end = cursor < buf.length() ? buf.substring(cursor) : "";
            buf = start + key + end;
            cursor++;
            if (buf.length() > window_size) slice_at++;
        }
    } else if (key == KEY_BACK) {
        if (cursor > 0) {
            String start = cursor > 0 ? buf.substring(0, cursor-1) : "";
            String end = cursor < buf.length() ? buf.substring(cursor) : "";
            buf = start + end;
            cursor--;
            if (slice_at > 0) slice_at--;
        }
    } else if (key == KEY_LEFT) {
        if (cursor > 0) cursor--;
        if (cursor < slice_at+1 && cursor > 0) slice_at--;
    } else if (key == KEY_FN_LEFT) {
        cursor = 0;
        slice_at = 0;
    } else if (key == KEY_RIGHT) {
        if (cursor < buf.length()) cursor++;
        if ((cursor > slice_at+window_size-2 && cursor < buf.length()-1 ||
            cursor > slice_at+window_size-1) &&
            cursor < buf.length()) slice_at++;
    } else if (key == KEY_FN_RIGHT) {
        cursor = buf.length();
        slice_at = window_size-1 > buf.length() ? 0 : buf.length()-window_size;
    } else if (key == KEY_FN_BACK) {
        String start = cursor > 0 ? buf.substring(0, cursor) : "";
        String end = cursor+1 < buf.length() ? buf.substring(cursor+1) : "";
        buf = start + end;
    } else if (key == KEY_SHIFT_BACK) {
        String start = "";
        String end = cursor < buf.length() ? buf.substring(cursor) : "";
        buf = start + end;
        cursor = 0;
        slice_at = 0;
    } else if (key == KEY_ENTER && submittable) {
        char copy[max_length];
        strcpy(copy, buf.c_str());
        buf = "";
        on_submit(copy);
        cursor = 0;
        slice_at = 0;
    } else {
        return false;
    }

    strcpy(ptr, buf.c_str());
    return true;
}

#pragma endregion


#pragma region Alert

void Alert::render(Adafruit_GFX& display) {
    // TODO: move to a separate method

    int16_t bx, by; uint16_t bw, bh;
    display.getTextBounds(message, 0, 0, &bx, &by, &bw, &bh);
    const int pad_x = 3;
    const int pad_y = 2;
    uint16_t box_w = bw + pad_x*2;
    uint16_t box_h = bh + pad_y*2 + 2;

    int16_t x = (display.width()  - box_w) / 2;
    int16_t y = (display.height() - box_h) / 2;

    display.fillRect(x, y, box_w-1, box_h-1, DISPLAY_BG);
    display.drawRect(x-1, y-1, box_w, box_h, DISPLAY_FG);

    display.setTextColor(DISPLAY_FG, DISPLAY_BG);
    display.setCursor(x + pad_x, y + pad_y);
    display.print(message);
}

bool Alert::update(char key) {
    return key == KEY_ENTER || key == KEY_ESC;
}

#pragma endregion


#pragma region CharTable

void CharTable::render(Adafruit_GFX& display, bool minimalized) {
    if (minimalized) {
        display.println("Characters");
    } else {
        display.println("  |0123456789ABCDEF| ");
        display.println(" -+----------------+-");
        for (uint8_t y = start; y < start + max_lines; y++) {
            if (y == 0x10) {
                display.println(" -+----------------+-");
            } else {
                display.print(' ');
                display.print(y, HEX);
                display.print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = static_cast<char>(x + y * 16);
                    if (c == 0x0A || c == 0x0D || c == '\n' || c == '\t') c = ' ';
                    display.print(c);
                }
                display.print("|");
                display.println(y, HEX);
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
