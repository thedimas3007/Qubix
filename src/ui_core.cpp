#include "ui_core.h"

#include "keycodes.h"

bool UIElement::update(char key) {
    return false;
}


void Stack::render(Adafruit_GFX* display, bool minimalized) {
    for (auto &i : children) {
        i->render(display, false);
    }
}

bool Stack::update(char key) {
    bool updated = false;
    for (auto &i : children) {
        if (i->update(key)) updated = true;
    }
    return updated;
}


void MenuView::checkPointer() {
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
            checkPointer();
            return true;
        } if (key == KEY_DOWN) {
            pointer = (pointer + 1) % n;
            checkPointer();
            return true;
        } if (key == KEY_RIGHT || key == KEY_ENTER) {
            selected = children[pointer];
            checkPointer();
            return true;
        }

        return false;
    } /* else if (selected != nullptr) */ {
        if (selected->update(key)) {
            return true;
        }

        if (key == KEY_LEFT || key == KEY_ESC) {
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
            display->println(title);
            for (int16_t i = start; i < last; ++i) {
                display->print(i == pointer ? "\x1A" : " ");
                children[i]->render(display, true);
            }
        }
    } else {
        if (selected->isInline()) {
            display->println(title);
            for (int16_t i = start; i < last; ++i) {
                display->print(i == pointer ? "\x1A" : " ");
                if (children[i] == selected) {
                    static_cast<UIInline*>(children[i])->renderInline(display);
                } else {
                    children[i]->render(display, true);
                }
            }

        } else {
            selected->render(display, false);
        }
    }
}


void Label::render(Adafruit_GFX* display, bool minimalized) {
    display->println(text);
}

void NumberPicker::render(Adafruit_GFX* display, bool minimalized) { // Minimalized is
    display->println(text + "  " + number);
}

void NumberPicker::renderInline(Adafruit_GFX* display) {
    display->println(text + " >" + number + "<");
}

bool NumberPicker::update(char key) {
    switch (key) {
        case KEY_UP:
            number += 1;
            return true;
        case KEY_DOWN:
            number -= 1;
            return true;
        case KEY_RIGHT:
            number += 10;
            return true;
        case KEY_LEFT:
            number -= 10;
            return true;
        default:
            return false;
    }
}

