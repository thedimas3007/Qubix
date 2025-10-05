#include <cstdio>

#include "ui/static.h"

/***************/
/**** Label ****/
/***************/
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

/******************/
/**** Property ****/
/******************/
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

/************************/
/**** StringProperty ****/
/************************/
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
