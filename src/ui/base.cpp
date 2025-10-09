#include "ui/base.h"
#include "configuration.h"

UIModal* UIApp::eraseFirstModal() {
    if (modals.empty()) {
        return nullptr;
    }

    UIModal* m = modals.front();
    modals.erase(modals.begin());
    return m;
}

void UIApp::render(UIContext& ctx) const {
    if (title) ctx.print(title);
    root->render(ctx, false);

    if (hasModals()) {
        for (int16_t y = 0; y < ctx.height; y++) {
            for (int16_t x = 0; x < ctx.width; x++) {
                if ((x + y) % 2) ctx.display.writePixel(x, y, settings.data.display_inv_alert ? ctx.theme.foreground : ctx.theme.background);
            }
        }
        ctx.setCursor(0, 0);
        modals.front()->render(ctx);
    }
}

bool UIApp::update(UIContext& ctx, char key) {
    if (hasModals()) {
        bool result = modals[0]->update(ctx, key);
        if (!result) return false;

        delete eraseFirstModal();
        return true;
    }

    return root->update(ctx, key);
}
