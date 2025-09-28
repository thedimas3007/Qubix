#include <utility>
#include <vector>
#include <functional>
#include <type_traits>

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

        Stack build() const { return Stack(c_); }
        Stack* buildPtr() const { return new Stack(c_); }
    };

    static Builder make() { return Builder{}; }

    String title = "";

    explicit Stack(const Config& cfg)
        : children(cfg.children), title(cfg.title) {}

    Stack(String title, std::initializer_list<UIElement*> children)
        : children(children), title(std::move(title)) {}

    Stack(std::initializer_list<UIElement*> children)
        : children(children) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
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

        MenuView build() const { return MenuView(c_); }
        MenuView* buildPtr() const { return new MenuView(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit MenuView(const Config& cfg)
        : selected(nullptr),
          children(cfg.children),
          start(0),
          cursor(0),
          window_size(static_cast<int16_t>(std::min<int>(cfg.max_elements, (int)cfg.children.size()))),
          on_exit(cfg.on_exit),
          icon(cfg.icon),
          title(cfg.title) {}

    MenuView(char icon, String title, std::initializer_list<UIElement*> children = {}, uint8_t max_elements = 6, std::function<void()> on_exit = [] {})
        : selected(nullptr),
          children(children),
          start(0),
          cursor(0),
          window_size(static_cast<int16_t>(std::min<int>(max_elements, (int)children.size()))),
          on_exit(std::move(on_exit)),
          icon(icon),
          title(std::move(title)) {}

    explicit MenuView(String title, std::initializer_list<UIElement*> children = {}, uint8_t max_elements = 6, std::function<void()> on_exit = [] {})
        : MenuView(0x00, std::move(title), children, max_elements, std::move(on_exit)) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
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
        Label build() const { return Label(c_); }
        Label* buildPtr() const { return new Label(c_); }
    };

    static Builder make() { return Builder{}; }

    String title;

    explicit Label(const Config& cfg)
        : title(cfg.title) {}

    explicit Label(String title)
        : title(std::move(title)) {}

    void render(Adafruit_GFX* display, bool minimalized) override;
};

template<class T>
class Property : public UIElement {
    T* ptr;
    const char* format;

public:
    struct Config {
        char icon = 0x00;
        String title = "";
        T* ptr = nullptr;
        const char* format = "%s";
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(T* p) { c_.ptr = p; return *this; }
        Builder& fmt(const char* f) { c_.format = f; return *this; }

        Property build() const { return Property(c_); }
        Property* buildPtr() const { return new Property(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit Property(const Config& cfg)
        : ptr(cfg.ptr), format(cfg.format), icon(cfg.icon), title(cfg.title) {}

    Property(char icon, String title, T* ptr, const char* format = "%s")
        : ptr(ptr), format(format), icon(icon), title(std::move(title)) {}

    Property(String title, T* ptr, const char* format = "%s")
        : ptr(ptr), format(format), icon(0x00), title(std::move(title)) {}

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
    struct Config {
        char icon = 0x00;
        String title = "";
        String suffix = "";
        T* number = nullptr;
        T min = 0;
        T max = 0;
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

        NumberPicker build() const { return NumberPicker(c_); }
        NumberPicker* buildPtr() const { return new NumberPicker(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;
    String suffix;

    explicit NumberPicker(const Config& cfg)
        : number(cfg.number), cursor(cfg.cursor), minimum(cfg.min),
          maximum(cfg.max), precision(cfg.precision),
          icon(cfg.icon), title(cfg.title), suffix(cfg.suffix) {}

    NumberPicker(char icon, String title, String suffix, T* number, T min = 0, T max = 0, uint8_t precision = 0, int8_t cursor = 0)
        : number(number), cursor(cursor), minimum(min), maximum(max), precision(precision), icon(icon), title(std::move(title)), suffix(std::move(suffix)) {};

    explicit NumberPicker(String title, T* number, T min = 0, T max = 0, uint8_t precision = 0, int8_t cursor = 0)
        : number(number), cursor(cursor), minimum(min), maximum(max), precision(precision), icon(0x00), title(std::move(title)), suffix("") {};

    T getAbsoluteValue() const;
    T getValue() const;

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
    struct Config {
        char icon = 0x00;
        String title = "";
        std::vector<String> items{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& items(const std::vector<String>& v) { c_.items = v; return *this; }
        Builder& items(const std::initializer_list<String>& v) { c_.items = v; return *this; }
        Builder& addItem(const String& s) { c_.items.push_back(s); return *this; }

        Selector build() const { return Selector(c_); }
        Selector* buildPtr() const { return new Selector(c_); }
    };

    static Builder make() { return Builder{}; }

    char icon;
    String title;

    explicit Selector(const Config& cfg)
        : items(cfg.items), icon(cfg.icon), title(cfg.title) {}

    Selector(char icon, String title, std::initializer_list<String> items)
        : items(items), icon(icon), title(std::move(title)) {};

    Selector(String title, std::initializer_list<String> items)
        : items(items), icon(0x00), title(std::move(title)) {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    void renderInline(Adafruit_GFX* display) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Other

class CharTable : public UIElement {
    int8_t start = 0;
    uint8_t max_lines = 5;
public:
    struct Config {
        String title = "";
        int8_t start = 0;
        uint8_t max_lines = 5;
    };

    class Builder {
        Config c_;
    public:
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& start(int8_t s) { c_.start = s; return *this; }
        Builder& maxLines(uint8_t m) { c_.max_lines = m; return *this; }

        CharTable build() const { return CharTable(c_); }
        CharTable* buildPtr() const { return new CharTable(c_); }
    };

    static Builder make() { return Builder{}; }

    String title;

    explicit CharTable(const Config& cfg)
        : start(cfg.start), max_lines(cfg.max_lines), title(cfg.title) {};

    explicit CharTable(String title)
        : title(std::move(title)) {};

    void render(Adafruit_GFX* display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#endif
