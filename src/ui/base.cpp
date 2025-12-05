#include "ui/base.h"
#include "configuration.h"

String UIElement::getLabel(int16_t available) const {
    String label = icon && settings.data.display_icons ? (String(icon) + title) : title;
    if (available < 0 || available >= label.length()) {
        // do nothing
    } else if (available == 0) {
        label = "";
    } else if (available < label.length()) {
        label = label.substring(0, available-1) + '\x96';
    }

    return label;
}

UIModal* UIApp::eraseFirstModal() {
    if (modals.empty()) return nullptr;

    UIModal* m = modals.front();
    modals.erase(modals.begin());
    return m;
}

UIModal* UIApp::eraseLastModal() {
    if (modals.empty()) return nullptr;
    UIModal* m = modals.back();
    modals.pop_back();
    return m;
}

void UIApp::addModal(UIModal* modal) {
    modals.push_back(modal);
    if (ctx) ctx->refresh(true);
}

void UIApp::render() {
    if (!ctx) return;

    ctx->display.drawRect(0, 0, ctx->width(), ctx->height(), ctx->theme.fg);
    if (title) {
        ctx->display.fillRect(0, 0, ctx->width(), ctx->charHeight()+1, ctx->theme.fg);
        ctx->setCursor(-3, 1); // yep, -2
        ctx->invertColors();
        ctx->println(title);
        ctx->resetColors();
    }

    ctx->setCursor(0, ctx->charHeight()+2);
    root->render(*ctx, false);

    if (last_size != modals.size()) {
        ctx->refresh(true);
        last_size = modals.size();
    }

    if (hasModals()) {
        for (int16_t y = 0; y < ctx->height(); y++) {
            for (int16_t x = 0; x < ctx->width(); x++) {
                if ((x + y) % 2) ctx->display.writePixel(x, y, settings.data.display_inv_alert ? ctx->theme.foreground : ctx->theme.background);
            }
        }
        ctx->setCursor(0, 0);
        modals.front()->render(*ctx);
    }
}

bool UIApp::update(char key) {
    if (hasModals()) {
        bool result = modals[0]->update(*ctx, key);
        if (!result) return false;

        delete eraseFirstModal();
        if (last_size != modals.size()) {
            ctx->refresh(true);
            last_size = modals.size();
        }
        return true;
    }

    return root->update(*ctx, key);
}
