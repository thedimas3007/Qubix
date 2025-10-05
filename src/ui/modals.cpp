#include "ui/modals.h"
#include "keycodes.h"

/***************/
/**** Alert ****/
/***************/
void Alert::render(Adafruit_GFX& display) {
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

