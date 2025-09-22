#include "ui_core.h"

#include "keycodes.h"

bool UIElement::update(char key) {
    return false;
}


void Root::render(Adafruit_GFX* display, bool minimalized) {
    for (auto &i : children) {
        i->render(display, false);
    }
}

bool Root::update(char key) {
    for (auto &i : children) {
        if (i->update(key)) return true;
    }
    return false;
}


void MenuView::check_pointer() {
    if (pointer < start) {
        start = pointer;
    } else if (pointer >= min(start + window_size, children.size())) {
        start = pointer - (window_size - 1); if (start < 0) start = 0;
    }
}

bool MenuView::update(char key) {
    if (selected == nullptr) {
        const int16_t n = children.size();
        if (key == 0 || n <= 0) return false;

        if (key == KEY_UP) {
            pointer = (pointer - 1 + n) % n;
            check_pointer();
            return true;
        } if (key == KEY_DOWN) {
            pointer = (pointer + 1) % n;
            check_pointer();
            return true;
        } if (key == KEY_RIGHT) {
            selected = children[pointer];
            check_pointer();
            return true;
        }

        return false;
    } /* else if (selected != nullptr) */ {
        if (selected->update(key)) {
            return true;
        }

        if (key == KEY_LEFT) {
            selected = nullptr;
            return true;
        }

        return false;
    }
}

void MenuView::render(Adafruit_GFX* display, bool minimalized) {
    const int16_t n = children.size();
    const int16_t last = std::min<int16_t>(start + window_size, n);

    if (selected == nullptr) {
        if (minimalized) {
            display->println(title);
        } else {
            for (int16_t i = start; i < last; ++i) {
                display->print(i == pointer ? "\x1A" : " ");
                children[i]->render(display, true);
            }
        }
    } else {
        selected->render(display, false);
    }
}


void Label::render(Adafruit_GFX* display, bool minimalized) {
    display->println(text);
}

