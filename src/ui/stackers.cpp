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

void MenuView::render(UIContext& ctx, bool minimalized) {
    const int16_t n = children.size();

    if (selected == nullptr) {
        if (minimalized) {
            // TODO: maybe trim here as well
            ctx.println(getLabel());
        } else {
            if (title.length()) ctx.println(title);

            window_size = ctx.availableCharsY();
            const int16_t last = std::min<int16_t>(slice_at + window_size, n);

            if (fill_mode == FillMode::TOP && n < window_size) {
                for (int16_t i = 0; i < window_size-n; i++) ctx.println();
            }

            for (int16_t i = slice_at; i < last; ++i) {
                ctx.print(i == cursor && active ? "\x1A" : " ");
                children[i]->render(ctx, true);
            }

            if (fill_mode == FillMode::BOTTOM && n < window_size) {
                for (uint16_t i = 0; i < window_size-n; i++) ctx.println();
            }
        }
    } else {
        if (selected->getType() == ElementType::INLINE) {
            ctx.println(title);
            window_size = ctx.availableCharsY();
            const int16_t last = std::min<int16_t>(slice_at + window_size, n);

            for (int16_t i = slice_at; i < last; ++i) {
                ctx.print(i == cursor && active ? "\x1A" : " ");
                if (children[i] == selected) {
                    static_cast<UIInline*>(children[i])->renderInline(ctx);
                } else {
                    children[i]->render(ctx, true);
                }
            }
        } else {
            // NOTE: Clickable can't be normally selected
            selected->render(ctx, false);
        }
    }
}

bool MenuView::update(UIContext& ctx, char key) {
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
                static_cast<UIClickable*>(element)->activate(ctx);
                on_exit();
            } else {
                selected = children[cursor];
                ctx.refresh(true);
            }
            return true;
        }
    } else {
        if (selected->update(ctx, key)) return true;

        if (key == KEY_LEFT || key == KEY_ESC) {
            ctx.refresh(true);
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
void TabSelector::render(UIContext& ctx, bool minimalized) {
    if (minimalized) {
        ctx.println(getLabel());
    } else {
        for (int i = 0; i < children.size(); ++i) {
            auto& element = children[i];

            if (element->getType() == ElementType::ACTIVE) {
                static_cast<UIActive*>(element)->setActive(i==cursor);
            }

            if (element->getType() == ElementType::INLINE && i == cursor) {
                static_cast<UIInline*>(element)->renderInline(ctx);
            } else {
                element->render(ctx, false);
            }
        }
    }
}

bool TabSelector::update(UIContext& ctx, char key) {
    if (key == KEY_TAB) {
        cursor = (cursor + 1) % children.size();
        return true;
    }

    return children[cursor]->update(ctx, key);
}
