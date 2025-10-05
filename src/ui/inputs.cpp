#include "ui/inputs.h"

#include "keycodes.h"

static int64_t pow10i(uint8_t n) {
    int64_t p = 1;
    while (n--) p *= 10;
    return p;
}

/**********************/
/**** NumberPicker ****/
/**********************/
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


/******************/
/**** Selector ****/
/******************/
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


/****************/
/**** Toggle ****/
/****************/
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


/****************/
/**** Button ****/
/****************/
void Button::render(Adafruit_GFX& display, bool minimalized) {
    display.println("[" + getLabel() + "]");
}

void Button::activate(Adafruit_GFX& display) {
    on_click();
}


/*******************/
/**** TextField ****/
/*******************/
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
