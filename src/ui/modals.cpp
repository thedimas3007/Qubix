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

    ctx.display.fillRect(x, y-1, box_w-1, box_h, ctx.theme.bg);
    ctx.display.drawRect(x, y-1, box_w-1, box_h, ctx.theme.fg);

    ctx.resetColors();
    ctx.setCursor(x + pad_x, y + pad_y);
    ctx.print(message);
}

bool Alert::update(UIContext& ctx, char key) {
    return key == KEY_ENTER || key == KEY_ESC;
}


/**********************/
/**** ConfirmModal ****/
/**********************/
void ConfirmModal::render(UIContext& ctx) {
    String copy = message;
    if (copy.length() < 6) {
        for (int i = 0; i < 6 - copy.length() + 1; i++) {
            copy += " ";
        }
    }
    int16_t bx, by; uint16_t bw, bh;
    ctx.display.getTextBounds(copy, 0, 0, &bx, &by, &bw, &bh);
    const int pad_x = 3;
    const int pad_y = 2;
    uint16_t box_w = bw + pad_x*2;
    uint16_t box_h = bh + pad_y*2 + 2;

    int16_t x = (ctx.width  - box_w) / 2;
    int16_t y = (ctx.height - box_h) / 2;

    ctx.display.fillRect(x, y-1, box_w-1, box_h, ctx.theme.bg);
    ctx.display.drawRect(x, y-1, box_w-1, box_h, ctx.theme.fg);

    ctx.resetColors();
    ctx.setCursor(x + pad_x, y + pad_y);
    ctx.print(copy);


    if (!flag) {
        ctx.invertColors();
        ctx.setCursor(x + pad_x, y + pad_y + ctx.charHeight());
        ctx.print("[x]");

        ctx.resetColors();
        ctx.setCursor(x + pad_x + bw - ctx.charWidth() * 3 - 1, y + pad_y + ctx.charHeight());
        ctx.print("[\xBA]");
    } else {
        ctx.resetColors();
        ctx.setCursor(x + pad_x, y + pad_y + ctx.charHeight());
        ctx.print("[x]");

        ctx.invertColors();
        ctx.setCursor(x + pad_x + bw - ctx.charWidth() * 3 - 1, y + pad_y + ctx.charHeight());
        ctx.print("[\xBA]");
    }
}


bool ConfirmModal::update(UIContext& ctx, char key) {
    if (key == KEY_TAB || key == KEY_LEFT || key == KEY_RIGHT) {
        flag = !flag;
        return false;
    }

    if (key == KEY_ESC) return true;

    if (key == KEY_ENTER) {
        if (flag) on_confirm();
        return true;
    }

    return false;

}
