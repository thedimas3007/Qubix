#include "ui/modals.h"
#include "keycodes.h"

const uint8_t pad_x = 3;
const uint8_t pad_y = 2;

void drawBoxed(UIContext& ctx, const String& text, int16_t& bx, int16_t& by, uint16_t& bw, uint16_t& bh) {
    ctx.display.getTextBounds(text, 0, 0, &bx, &by, &bw, &bh);

    uint16_t box_w = bw + pad_x*2;
    uint16_t box_h = bh + pad_y*2 + 2;

    bx = (ctx.width  - box_w) / 2;
    by = (ctx.height - box_h) / 2;

    ctx.display.fillRect(bx+2, by+1, box_w-1, box_h, ctx.theme.fg);
    ctx.display.fillRect(bx, by-1, box_w-1, box_h, ctx.theme.bg);
    ctx.display.drawRect(bx, by-1, box_w-1, box_h, ctx.theme.fg);

    ctx.resetColors();
    ctx.setCursor(bx + pad_x, by + pad_y);
    ctx.print(text);
}

/***************/
/**** Alert ****/
/***************/
void Alert::render(UIContext& ctx) {
    int16_t x, y; uint16_t bw, bh;
    drawBoxed(ctx, message, x, y, bw, bh);
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

    int16_t x, y; uint16_t bw, bh;
    drawBoxed(ctx, copy, x, y, bw, bh);


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
