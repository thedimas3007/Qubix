#include "ui/modals.h"
#include "keycodes.h"

/***************/
/**** Alert ****/
/***************/
void Alert::render(UIContext& ctx) {
    int16_t bx, by; uint16_t bw, bh;
    ctx.display.getTextBounds(message, 0, 0, &bx, &by, &bw, &bh);
    const int pad_x = 3;
    const int pad_y = 2;
    uint16_t box_w = bw + pad_x*2;
    uint16_t box_h = bh + pad_y*2 + 2;

    int16_t x = (ctx.width  - box_w) / 2;
    int16_t y = (ctx.height - box_h) / 2;

    ctx.display.fillRect(x, y, box_w-1, box_h-1, ctx.theme.bg);
    ctx.display.drawRect(x-1, y-1, box_w, box_h, ctx.theme.fg);

    ctx.resetColors();
    ctx.setCursor(x + pad_x, y + pad_y);
    ctx.print(message);
}

bool Alert::update(UIContext& ctx, char key) {
    return key == KEY_ENTER || key == KEY_ESC;
}

