#include <cstdio>

#include "ui/static.h"

/***************/
/**** Label ****/
/***************/
void Label::render(UIContext& ctx, bool minimalized) {
    if (max_length < 0) max_length = ctx.availableCharsX();
    String data = getLabel();
    if (max_length && data.length() > max_length && minimalized) {
        ctx.println(data.substring(0, max_length-1) + '\x96');
    } else if (minimalized) {
        ctx.println(data);
    } else {
        ctx.println(title);
    }
}

/******************/
/**** Property ****/
/******************/
template <class T>
void Property<T>::render(UIContext& ctx, bool minimalized) {
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
    uint8_t spaces = ctx.availableSpaces(prefix.length() + data.length());
    if (prefix.length() + data.length() > ctx.availableCharsX() && minimalized) {
        prefix = prefix.substring(0, std::max(0u, ctx.availableCharsX()-data.length()-1)) + '\x96';
    }

    ctx.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) ctx.print(' ');
    ctx.println(data);
}

/************************/
/**** StringProperty ****/
/************************/
void StringProperty::render(UIContext& ctx, bool minimalized) {
    String prefix = getLabel();
    String data = ptr != nullptr ? String(ptr) : "<null>";
    uint8_t spaces = ctx.availableSpaces(prefix.length() + data.length());
    if (prefix.length() + data.length() > ctx.availableCharsX() && minimalized) {
        prefix = prefix.substring(0, std::max(0u, ctx.availableCharsX()-data.length()-1)) + '\x96';
    }

    ctx.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) ctx.print(' ');
    ctx.println(data);
}
