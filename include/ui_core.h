#include <utility>
#include <vector>

#include "Adafruit_GFX.h"

#ifndef TERRACOTTA_UI_CORE_H
#define TERRACOTTA_UI_CORE_H

#pragma region Base

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

#pragma endregion

#pragma region Stackers

class Stack : public UIElement {
    std::vector<UIElement*> children;

public:
    String title = "";

    Stack(String title, std::initializer_list<UIElement*> children)
        : children(children), title(std::move(title)) {}

    Stack(std::initializer_list<UIElement*> children)
        : children(children) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

// TODO:
// Updatable children
class MenuView : public UIElement {
    UIElement* selected = nullptr;
    std::vector<UIElement*> children;
    const uint8_t max_elements;

    int16_t start = 0;
    int16_t cursor = 0;
    int16_t end = min(children.size(), max_elements);
    const int16_t window_size = min(children.size(), max_elements);
    void checkPointer();

public:
    char icon;
    String title;

    MenuView(char icon, String title, std::initializer_list<UIElement*> children = {}, uint8_t max_elements = 6)
        : children(children), max_elements(max_elements), icon(icon), title(std::move(title)) {}

    MenuView(String title, std::initializer_list<UIElement*> children = {}, uint8_t max_elements = 6)
        : children(children), max_elements(max_elements), icon(0x00), title(std::move(title)) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Static

class Label : public UIElement {
public:
    explicit Label(String text)
        : text(std::move(text)) {}

    String text;
    void render(Adafruit_GFX* display, bool minimalized) override;
};

template<class T>
class Property : public UIElement {
    T* ptr;
    const char* format;

public:
    char icon;
    String text;

    Property(char icon, String text, T* ptr, const char* format = "%s")
        : ptr(ptr), format(format), icon(icon), text(std::move(text)) {}

    Property(String text, T* ptr, const char* format = "%s")
        : ptr(ptr), format(format), icon(0x00), text(std::move(text)) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
};
template class Property<uint8_t>;
template class Property<int8_t>;
template class Property<float>;

#pragma endregion

#pragma region Inputs

template<class T>
class NumberPicker : public UIInline {
    static_assert(std::is_arithmetic<T>::value, "T must be a number");

    T* number;
    int8_t cursor;
    const T minimum;
    const T maximum;
    const uint8_t precision;

    uint8_t getDigits() const;
    void roundToPrecision();

public:
    T getAbsoluteValue() const;
    T getValue() const;

    char icon;
    String text;
    String suffix;

    NumberPicker(char icon, String text, String suffix, T* number, T min = 0, T max = 0, uint8_t precision = 0, int8_t cursor = 0)
        : number(number), cursor(cursor), minimum(min), maximum(max), precision(precision), icon(icon), text(std::move(text)), suffix(std::move(suffix)) {};

    explicit NumberPicker(String text, T* number, T min = 0, T max = 0, uint8_t precision = 0, int8_t cursor = 0)
        : number(number), cursor(cursor), minimum(min), maximum(max), precision(precision), icon(0x00), text(std::move(text)), suffix("") {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    void renderInline(Adafruit_GFX* display) override;
    bool update(char key) override;
};
template class NumberPicker<uint8_t>;
template class NumberPicker<int8_t>;
template class NumberPicker<float>;

class Selector : public UIInline {
    std::vector<String> items;
    int8_t cursor = 0;
public:
    char icon;
    String text;

    Selector(char icon, String text, std::initializer_list<String> items)
        : items(items), icon(icon), text(std::move(text)) {};

    Selector(String text, std::initializer_list<String> items)
        : items(items), icon(0x00), text(std::move(text)) {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    void renderInline(Adafruit_GFX* display) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Other

class CharTable : public UIElement {
    const uint8_t max_lines = 5;
    int8_t start = 0;
public:
    String text;

    explicit CharTable(String text)
        : text(std::move(text)) {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#endif
