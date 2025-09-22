#include <utility>
#include <vector>

#include "Adafruit_GFX.h"

#ifndef TERRACOTTA_UI_CORE_H
#define TERRACOTTA_UI_CORE_H

#define MAX_MENU_ELEMENTS 7 // elements per page

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void render(Adafruit_GFX* display, bool minimalized) = 0;
    virtual bool update(char key);

    virtual bool isInline() const { return false; };
};

class UIInline : public UIElement {
public:
    bool isInline() const override { return true; };
    virtual void renderInline(Adafruit_GFX* display) = 0;
};

class Stack : public UIElement {
    std::vector<UIElement*> children;
public:
    Stack(std::initializer_list<UIElement*> children)
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

    void checkPointer();
public:
    String title;

    MenuView(String title, std::initializer_list<UIElement*> children = {})
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

class NumberPicker : public UIInline {
    int16_t number = 0;
public:
    String text;

    explicit NumberPicker(String text)
        : text(std::move(text)) {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    void renderInline(Adafruit_GFX* display) override;
    bool update(char key) override;
};

#endif