#include "ui/extra.h"
#include "keycodes.h"

/*******************/
/**** CharTable ****/
/*******************/
void CharTable::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println("Characters");
    } else {
        ctx.println("  |0123456789ABCDEF| ");
        ctx.println(" -+----------------+-");
        for (uint8_t y = start; y < start + max_lines; y++) {
            if (y == 0x10) {
                ctx.println(" -+----------------+-");
            } else {
                ctx.print(String(' '));
                ctx.printf("%x", y);
                ctx.print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = static_cast<char>(x + y * 16);
                    if (c == 0x0A || c == 0x0D || c == '\n' || c == '\t') c = ' ';
                    ctx.print(String(c));
                }
                ctx.print("|");
                ctx.printf("%x\n", y);
            }
        }
    }
}

bool CharTable::update(UIContext& ctx, char key) {
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