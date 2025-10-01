#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "Adafruit_GFX.h"

#ifndef TERRACOTTA_UI_CORE_H
#define TERRACOTTA_UI_CORE_H

#pragma region Base

enum class ElementType {
    BASIC, INLINE, CLICKABLE
};

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void render(Adafruit_GFX &display, bool minimalized) = 0;
    virtual bool update(char key);

    [[nodiscard]] virtual ElementType getType() const { return ElementType::BASIC; };
};

class UIInline : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::INLINE; };
    virtual void renderInline(Adafruit_GFX &display) = 0;
};

class UIClickable : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::CLICKABLE; };
    virtual void activate(Adafruit_GFX &display) = 0;
};

#pragma endregion

#pragma region Stackers

class Stack : public UIElement {
    std::vector<UIElement*> children;

public:
    struct Config {
        String title = "";
        std::vector<UIElement*> children{};
    };

    class Builder {
        Config c_;
    public:
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& children(const std::vector<UIElement*>& v) { c_.children = v; return *this; }
        Builder& children(const std::initializer_list<UIElement*>& v) { c_.children = v; return *this; }
        Builder& addChild(UIElement* e) { c_.children.push_back(e); return *this; }

        [[nodiscard]] Stack build() const { return Stack(c_); }
        [[nodiscard]] Stack* buildPtr() const { return new Stack(c_); }
    };

    static Builder make() { return Builder{}; }

    String title = "";

    explicit Stack(const Config& cfg)
        : children(cfg.children), title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
    bool update(char key) override;
};


class MenuView : public UIElement {
    UIElement* selected = nullptr;
    std::vector<UIElement*> children;

    int16_t start = 0;
    int16_t cursor = 0;
    int16_t window_size = 6;

    void checkPointer();
    std::function<void()> on_exit;

public:
    struct Config {
        char icon = 0x00;
        String title = "";
        std::vector<UIElement*> children{};
        uint8_t max_elements = 6;
        std::function<void()> on_exit = [] {};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& children(const std::vector<UIElement*>& v) { c_.children = v; return *this; }
        Builder& children(const std::initializer_list<UIElement*>& v) { c_.children = v; return *this; }
        Builder& addChild(UIElement* e) { c_.children.push_back(e); return *this; }
        Builder& maxElements(uint8_t m) { c_.max_elements = m; return *this; }
        Builder& onExit(std::function<void()> f) { c_.on_exit = std::move(f); return *this; }

        [[nodiscard]] MenuView build() const { return MenuView(c_); }
        [[nodiscard]] MenuView* buildPtr() const { return new MenuView(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit MenuView(const Config& cfg)
        : selected(nullptr),
          children(cfg.children),
          start(0),
          cursor(0),
          window_size(static_cast<int16_t>(std::min<int>(cfg.max_elements, static_cast<int>(cfg.children.size())))),
          on_exit(cfg.on_exit),
          icon(cfg.icon),
          title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Static

class Label : public UIElement {
public:
    struct Config {
        String title = "";
    };

    class Builder {
        Config c_;
    public:
        Builder& title(const String& t) { c_.title = t; return *this; }
        [[nodiscard]] Label build() const { return Label(c_); }
        [[nodiscard]] Label* buildPtr() const { return new Label(c_); }
    };

    static Builder make() { return Builder{}; }

    String title;

    explicit Label(const Config& cfg)
        : title(cfg.title) {}

    explicit Label(String title)
        : title(std::move(title)) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
};

template<class T>
class Property : public UIElement {
    T* ptr;
    const char* format;
    std::vector<String> values;
    bool with_values;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        T* ptr = nullptr;
        const char* format = "%s";
        std::vector<String> values = std::vector<String>();
        bool with_values = false;
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(T* p) { c_.ptr = p; return *this; }
        Builder& fmt(const char* f) { c_.format = f; return *this; }
        Builder& values(std::vector<String>& v) { c_.with_values = true, c_.values = v; return *this; }

        [[nodiscard]] Property build() const { return Property(c_); }
        [[nodiscard]] Property* buildPtr() const { return new Property(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit Property(const Config& cfg)
        : ptr(cfg.ptr), format(cfg.format), values(cfg.values), with_values(cfg.with_values), icon(cfg.icon), title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
};
template class Property<uint8_t>;
template class Property<int8_t>;
template class Property<float>;

class StringProperty : public UIElement {
    char* ptr;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        char* ptr = nullptr;
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(char p[]) { c_.ptr = p; return *this; }

        [[nodiscard]] StringProperty build() const { return StringProperty(c_); }
        [[nodiscard]] StringProperty* buildPtr() const { return new StringProperty(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit StringProperty(const Config& cfg)
        : ptr(cfg.ptr), icon(cfg.icon), title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
};

#pragma endregion

#pragma region Inputs

template<class T>
class NumberPicker : public UIInline {
    static_assert(std::is_arithmetic_v<T>, "T must be a number");

    T* number;
    int8_t cursor;
    const T minimum;
    const T maximum;
    const uint8_t precision;

    [[nodiscard]] uint8_t getDigits() const;
    void roundToPrecision();

public:
    struct Config {
        char icon = 0x00;
        String title = "";
        String suffix = "";
        T* number = nullptr;
        T min = std::numeric_limits<T>::lowest();
        T max = std::numeric_limits<T>::max();
        uint8_t precision = 0;
        int8_t cursor = 0;
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& suffix(const String& s) { c_.suffix = s; return *this; }
        Builder& pointer(T* p) { c_.number = p; return *this; }
        Builder& min(T v) { c_.min = v; return *this; }
        Builder& max(T v) { c_.max = v; return *this; }
        Builder& precision(uint8_t p) { c_.precision = p; return *this; }
        Builder& cursor(int8_t c) { c_.cursor = c; return *this; }

        [[nodiscard]] NumberPicker build() const { return NumberPicker(c_); }
        [[nodiscard]] NumberPicker* buildPtr() const { return new NumberPicker(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;
    String suffix;

    explicit NumberPicker(const Config& cfg)
        : number(cfg.number), cursor(cfg.cursor), minimum(cfg.min),
          maximum(cfg.max), precision(cfg.precision),
          icon(cfg.icon), title(cfg.title), suffix(cfg.suffix) {}

    [[nodiscard]] T getAbsoluteValue() const;
    [[nodiscard]] T getValue() const;

    void render(Adafruit_GFX &display, bool minimalized) override;
    void renderInline(Adafruit_GFX &display) override;
    bool update(char key) override;
};
template class NumberPicker<uint8_t>;
template class NumberPicker<int8_t>;
template class NumberPicker<float>;


class Selector : public UIInline {
    uint8_t* selection;
    std::vector<String> items;
    int8_t cursor = 0;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        uint8_t* selection = nullptr;
        std::vector<String> items{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(uint8_t* s) { c_.selection = s; return *this; }
        Builder& items(const std::vector<String>& v) { c_.items = v; return *this; }
        Builder& items(const std::initializer_list<String>& v) { c_.items = v; return *this; }
        Builder& addItem(const String& s) { c_.items.push_back(s); return *this; }

        [[nodiscard]] Selector build() const { return Selector(c_); }
        [[nodiscard]] Selector* buildPtr() const { return new Selector(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit Selector(const Config& cfg)
        : selection(cfg.selection), items(cfg.items), icon(cfg.icon), title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
    void renderInline(Adafruit_GFX &display) override;
    bool update(char key) override;
};


class Toggle : public UIClickable {
    bool* ptr;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        bool* pointer = nullptr;
        std::vector<String> items{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(bool* b) { c_.pointer = b; return *this; }

        [[nodiscard]] Toggle build() const { return Toggle(c_); }
        [[nodiscard]] Toggle* buildPtr() const { return new Toggle(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit Toggle(const Config& cfg)
        : ptr(cfg.pointer), icon(cfg.icon), title(cfg.title) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
    void activate(Adafruit_GFX &display) override;
};


class TextField : public UIInline {
    char* ptr;
    int16_t cursor;
    uint8_t max_length;
    uint8_t window_size;
    uint8_t slice_at = 0;
    std::function<void(char*)> on_submit;
    bool submittable;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        bool spacer = true;
        char* ptr = nullptr;
        uint8_t max_length = 64;
        uint8_t window_size = 0;
        std::function<void(char*)> on_submit = [](char*) {};
        bool submittable = false;
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& spacer(bool s) { c_.spacer = s; return *this; }
        Builder& pointer(char c[]) { c_.ptr = c; return *this; }
        Builder& maxLength(uint8_t l) { c_.max_length = l; return *this; }
        Builder& windowSize(uint8_t s) { c_.window_size = s; return *this; };
        Builder& onSubmit(std::function<void(char*)> f) { c_.on_submit = std::move(f); c_.submittable = true; return *this; }

        [[nodiscard]] TextField build() const { return TextField(c_); }
        [[nodiscard]] TextField* buildPtr() const { return new TextField(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;
    bool spacer;

    explicit TextField(const Config& cfg)
        : ptr(cfg.ptr), cursor(-1), max_length(cfg.max_length),
          window_size(cfg.window_size ? std::min<uint8_t>(cfg.max_length, cfg.window_size) : cfg.max_length),
          on_submit(cfg.on_submit), submittable(cfg.submittable),
          icon(cfg.icon), title(cfg.title), spacer(cfg.spacer) {}

    void render(Adafruit_GFX &display, bool minimalized) override;
    void renderInline(Adafruit_GFX &display) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Other

class CharTable : public UIElement {
    int8_t start = 0;
    uint8_t max_lines;
public:
    struct Config {
        uint8_t max_lines = 5;
    };

    class Builder {
        Config c_;
    public:
        Builder& maxLines(uint8_t m) { c_.max_lines = m; return *this; }

        [[nodiscard]] CharTable build() const { return CharTable(c_); }
        [[nodiscard]] CharTable* buildPtr() const { return new CharTable(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit CharTable(const Config& cfg) : max_lines(cfg.max_lines) {};

    void render(Adafruit_GFX &display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#endif
