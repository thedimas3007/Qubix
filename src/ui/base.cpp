#include <type_traits>
#include <cmath>
#include <cstdio>
#include <algorithm>

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

void UIApp::render(Adafruit_GFX& display) const {
    if (title) display.print(title);
    root->render(display, false);

    if (hasModals()) {
        for (int16_t y = 0; y < display.height(); y++) {
            for (int16_t x = 0; x < display.width(); x++) {
                if ((x + y) % 2) display.writePixel(x, y, settings.data.display_inv_alert ? DISPLAY_FG : DISPLAY_BG);
            }
        }
        display.setCursor(0, 0);
        modals.front()->render(display);
    }
}

bool UIApp::update(char key) {
    if (hasModals()) {
        bool result = modals[0]->update(key);
        if (!result) return false;

        delete eraseFirstModal();
        return true;
    }

    return root->update(key);
}
