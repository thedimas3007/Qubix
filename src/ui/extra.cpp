#include "ui/extra.h"
#include "keycodes.h"

/*******************/
/**** CharTable ****/
/*******************/
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