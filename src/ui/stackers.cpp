#include "ui/stackers.h"
#include "keycodes.h"

/******************/
/**** MenuView ****/
/******************/
void MenuView::addChild(UIElement* e) {
    children.push_back(e);
    cursor = children.size() - 1;
    if (cursor > window_size+slice_at-1) slice_at++;
}

void MenuView::render(Adafruit_GFX& display, bool minimalized) {
    const int16_t n = children.size();
    const int16_t last = std::min<int16_t>(slice_at + window_size, n);

    if (selected == nullptr) {
        if (minimalized) {
            display.println(getLabel());
        } else {
            if (title.length()) display.println(title);
            if (fill_mode == FillMode::TOP && n < window_size) {
                for (int16_t i = 0; i < window_size-n; i++) display.println();
            }

            for (int16_t i = slice_at; i < last; ++i) {
                display.print(i == cursor && active ? "\x1A" : " ");
                children[i]->render(display, true);
            }

            if (fill_mode == FillMode::BOTTOM && n < window_size) {
                for (uint16_t i = 0; i < window_size-n; i++) display.println();
            }
        }
    } else {
        if (selected->getType() == ElementType::INLINE) {
            display.println(title);
            for (int16_t i = slice_at; i < last; ++i) {
                display.print(i == cursor && active ? "\x1A" : " ");
                if (children[i] == selected) {
                    static_cast<UIInline*>(children[i])->renderInline(display);
                } else {
                    children[i]->render(display, true);
                }
            }
        } else {
            // NOTE: Clickable can't be normally selected
            selected->render(display, false);
        }
    }
}

bool MenuView::update(char key) {
    if (selected == nullptr) {
        const int16_t n = children.size();
        if (key == 0 || n <= 0) return false;

        if (key == KEY_UP) {
            if (cursor > 0) {
                cursor--;
                if (cursor < slice_at+1 && cursor > 0) slice_at--;
            } else {
                cursor = n-1;
                slice_at = (n > window_size) ? (n - window_size) : 0;
            }
            return true;
        }
        if (key == KEY_DOWN) {
            if (cursor < n-1) {
                cursor++;
                if ((cursor > slice_at+window_size-2 && cursor < n-1 ||
                    cursor > slice_at+window_size-1) &&
                    cursor < n) slice_at++;
            } else {
                cursor = 0;
                slice_at = 0;
            }
            return true;
        }
        if (key == KEY_RIGHT || key == KEY_ENTER) {
            auto element = children[cursor];
            if (element->getType() == ElementType::CLICKABLE) {
                static_cast<UIClickable*>(element)->activate(display);
                on_exit();
            } else {
                selected = children[cursor];
            }
            // checkPointer();
            return true;
        }
    } else {
        if (selected->update(key)) return true;

        if (key == KEY_LEFT || key == KEY_ESC) {
            selected = nullptr;
            on_exit();
            return true;
        }
    }

    return false;
}


/*********************/
/**** TabSelector ****/
/*********************/
void TabSelector::render(Adafruit_GFX& display, bool minimalized) {
    if (minimalized) {
        display.println(getLabel());
    } else {
        for (int i = 0; i < children.size(); ++i) {
            auto& element = children[i];

            if (element->getType() == ElementType::ACTIVE) {
                static_cast<UIActive*>(element)->setActive(i==cursor);
            }

            if (element->getType() == ElementType::INLINE && i == cursor) {
                static_cast<UIInline*>(element)->renderInline(display);
            } else {
                element->render(display, false);
            }
        }
    }
}

bool TabSelector::update(char key) {
    if (key == KEY_TAB) {
        cursor = (cursor + 1) % children.size();
        return true;
    }

    return children[cursor]->update(key);
}
