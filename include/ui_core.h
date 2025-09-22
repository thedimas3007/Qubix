#include <utility>
#include <vector>

#include "Adafruit_GFX.h"

#ifndef TERRACOTTA_UI_CORE_H
#define TERRACOTTA_UI_CORE_H

#define MAX_MENU_ELEMENTS 6 // elements per page

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void render(Adafruit_GFX* display, bool minimalized) = 0;
    virtual bool update(char key);
};

class Root : public UIElement {
    std::vector<UIElement*> children;
public:
    Root(std::initializer_list<UIElement*> children)
        : children(children) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

/// Updatable children
class MenuView : public UIElement {
    UIElement* selected = nullptr;
    std::vector<UIElement*> children;

    int16_t start = 0;
    int16_t pointer = 0;
    int16_t end = min(children.size(), MAX_MENU_ELEMENTS);
    const int16_t window_size = min(children.size(), MAX_MENU_ELEMENTS);
    const int margin = 0;

    void check_pointer();
public:
    String title;

    MenuView(String title, std::initializer_list<UIElement*> children)
        : children(children), title(std::move(title)) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

class Label : public UIElement {
public:
    explicit Label(String text)
        : text(std::move(text)) {}

    String text;
    void render(Adafruit_GFX* display, bool minimalized) override;
};

#endif