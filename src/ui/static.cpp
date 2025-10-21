#include <cstdio>

#include "ui/static.h"

/***************/
/**** Label ****/
/***************/
void Label::render(UIContext& ctx, bool minimalized) {
    if (max_length < 0) max_length = ctx.availableCharsX();
    ctx.println(minimalized ? getLabel(max_length) : title);
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

    if (!minimalized) {
        ctx.print(getLabel());
        ctx.print(": ");
        ctx.println(data);
        return;
    }

    String prefix = getLabel(ctx.availableCharsX() - data.length());
    uint8_t spaces = ctx.availableSpaces(prefix.length() + data.length());

    ctx.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) ctx.print(' ');
    ctx.println(data);
}

/************************/
/**** StringProperty ****/
/************************/
void StringProperty::render(UIContext& ctx, bool minimalized) {
    String data = ptr != nullptr ? String(ptr) : "<null>";

    if (!minimalized) {
        ctx.print(getLabel());
        ctx.print(": ");
        ctx.println(data);
        return;
    }

    String prefix = getLabel(ctx.availableCharsX() - data.length());
    uint8_t spaces = ctx.availableSpaces(prefix.length() + data.length());
    ctx.print(prefix);
    for (uint8_t i = 0; i < spaces; i++) ctx.print(' ');
    ctx.println(data);
}
