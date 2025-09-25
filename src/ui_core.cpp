#include "ui_core.h"

#include "configuration.h"
#include "keycodes.h"

bool UIElement::update(char key) {
    return false;
}


void Stack::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(title);
    } else {
        if (title != "") {
            display->println(title);
        }
        for (auto &i : children) {
            i->render(display, false);
        }
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
        start = max(0, pointer - (window_size - 1));
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
            display->println(icon + title);
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

uint8_t NumberPicker::getDigits() const {
    if (number > 0) {
        return log10(number) + 1;
    }
    if (number < 0) {
        return log10(-number) + 1;
    }
    return 1; // for zeros
}

void NumberPicker::render(Adafruit_GFX* display, bool minimalized) {
    display->print(text + " ");
    if (number < 0) display->print("-");

    num_t num_copy = abs(number);
    for (uint8_t i = getDigits(); i > 0; i--) {
        if (i == scale) display->print(".");
        num_t digit = num_copy / pow(10, i-1);
        num_copy -= digit * pow(10, i-1);
        display->print(digit);
    }
}

void NumberPicker::renderInline(Adafruit_GFX* display) {
    display->print(text + " ");
    if (number < 0) display->print("-");

    num_t num_copy = abs(number);
    for (uint8_t i = getDigits(); i > 0; i--) {
        if (i == scale) display->print(".");

        if (i == pointer+1) display->setTextColor(DISPLAY_BG, DISPLAY_FG);
        else                display->setTextColor(DISPLAY_FG, DISPLAY_BG);
        num_t digit = num_copy / pow(10, i-1);
        num_copy -= digit * pow(10, i-1);
        display->print(digit);
        display->setTextColor(DISPLAY_FG, DISPLAY_BG);
    }
    display->setTextColor(DISPLAY_FG, DISPLAY_BG);
}

bool NumberPicker::update(char key) {
    if (key == KEY_UP) {
        number += pow(10, pointer);
    } else if (key == KEY_DOWN) {
        uint8_t digits = getDigits();
        number -= pow(10, pointer);
        if (digits > getDigits() && pointer == digits - 1 && pointer != 0) {
            pointer--;
        }
    } else if (key == KEY_RIGHT) {
        pointer--;
        if (pointer < 0) {
            pointer = getDigits() - 1;
        }
    } else if (key == KEY_LEFT) {
        pointer++;
        if (pointer >= getDigits()) {
            pointer = 0;
        }
    } else {
        return false;
    }

    return true;
}

void CharTable::render(Adafruit_GFX* display, bool minimalized) {
    if (minimalized) {
        display->println(text);
    } else {
        display->println("  |0123456789ABCDEF| ");
        display->println(" -+----------------+-");
        for (uint8_t y = start; y < start+max_lines; y++) {
            if (y == 0x10) {
                display->println(" -+----------------+-");
            } else {
                display->print(" ");
                display->print(y, HEX);
                display->print("|");
                for (uint8_t x = 0; x < 16; x++) {
                    char c = x + y * 16;
                    if (c == 0x0A || c == 0x0D || c == '\n' || c == '\t') c = ' ';
                    display->print(c);
                }
                display->print("|");
                display->println(y, HEX);
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
        if (start > 16-max_lines+1) start--;
    } else {
        return false;
    }

    return true;
}

