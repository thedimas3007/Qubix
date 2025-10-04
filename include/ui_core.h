#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include <Adafruit_GFX.h>

#include "configuration.h"

#ifndef QUBIX_UI_CORE_H
#define QUBIX_UI_CORE_H

#pragma region Base

enum class ElementType {
    BASIC, INLINE, CLICKABLE, ACTIVE
};

class UIElement {
public:
    char icon = 0x00;
    String title{};

    String getLabel() { return icon && settings.data.display_icons ? (String(icon) + title) : title; }

    virtual ~UIElement() = default;

    virtual void render(Adafruit_GFX& display, bool minimalized) = 0;
    virtual bool update(char key) { return false; }; // true when character processed; false otherwise

    [[nodiscard]] virtual ElementType getType() const { return ElementType::BASIC; };
};


class UIInline : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::INLINE; };
    virtual void renderInline(Adafruit_GFX& display) = 0;
};

class UIClickable : public UIElement {
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::CLICKABLE; };
    virtual void activate(Adafruit_GFX& display) = 0;
};

class UIActive : public UIElement {
protected:
    bool active = true;
public:
    [[nodiscard]] ElementType getType() const override { return ElementType::ACTIVE; };
    void setActive(bool a) { active = a; }
};

class UIModal {
public:
    virtual ~UIModal() = default;

    virtual void render(Adafruit_GFX& display);
    virtual bool update(char key) { return false; }; // whether it can be closed
};

class UIApp {
    UIElement* root;
    std::vector<UIModal*> modals{};

    UIModal* eraseFirstModal();
public:
    struct Config {
        String title = "";
        UIElement* root = nullptr;
    };

    class Builder {
        Config c_;
    public:
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& root(UIElement* root) { c_.root = root; return *this; }

        [[nodiscard]] UIApp build() const {
            assert(c_.root != nullptr && "Root cannot be nullptr");
            return UIApp(c_);
        }

        [[nodiscard]] UIApp* buildPtr() const {
            assert(c_.root != nullptr && "Root cannot be nullptr");
            return new UIApp(c_);
        }
    };

    String title;

    static Builder make() { return Builder{}; }

    explicit UIApp(const Config& cfg)
        : root(cfg.root) { title = cfg.title; }

    void addModal(UIModal* modal) { modals.push_back(modal); }
    bool hasModals() const { return !modals.empty(); }

    void render(Adafruit_GFX& display) const;
    bool update(char key);
};

#pragma endregion

#pragma region Stackers

enum class FillMode {
    NONE, BOTTOM, TOP
};

class MenuView : public UIActive {
    UIElement* selected = nullptr;
    std::vector<UIElement*> children;
    FillMode fill_mode;

    int16_t slice_at = 0;
    int16_t cursor = 0;
    int16_t window_size = 6;

    std::function<void()> on_exit;

public:
    struct Config {
        char icon = 0x00;
        String title = "";
        std::vector<UIElement*> children{};
        FillMode fill_mode = FillMode::NONE;
        uint8_t window_size = 6;
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
        Builder& fill(FillMode m) { c_.fill_mode = m; return *this; }
        Builder& windowSize(uint8_t s) { c_.window_size = s; return *this; }
        Builder& onExit(std::function<void()> f) { c_.on_exit = std::move(f); return *this; }

        [[nodiscard]] MenuView build() const { return MenuView(c_); }
        [[nodiscard]] MenuView* buildPtr() const { return new MenuView(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit MenuView(const Config& cfg)
        : children(cfg.children),
          fill_mode(cfg.fill_mode),
          window_size(cfg.window_size),
          on_exit(cfg.on_exit) { icon = cfg.icon; title = cfg.title; }

    void addChild(UIElement* e);
    // void removeLastChild() { children.pop_back(); }
    // void removeFirstChild() { children.erase(children.begin()); }

    void render(Adafruit_GFX& display, bool minimalized) override;
    bool update(char key) override;
};

class TabSelector : public UIElement {
    std::vector<UIElement*> children;
    uint8_t cursor = 0;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        std::vector<UIElement*> children{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& children(const std::vector<UIElement*>& v) { c_.children = v; return *this; }
        Builder& children(const std::initializer_list<UIElement*>& v) { c_.children = v; return *this; }
        Builder& addChild(UIElement* e) { c_.children.push_back(e); return *this; }

        [[nodiscard]] TabSelector build() const { return TabSelector(c_); }
        [[nodiscard]] TabSelector* buildPtr() const { return new TabSelector(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit TabSelector(const Config& cfg)
        : children(cfg.children) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Static

class Label : public UIElement {
    uint8_t max_length;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        uint8_t max_length = 0;
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& maxLength(uint8_t l) { c_.max_length = l; return *this; }

        [[nodiscard]] Label build() const { return Label(c_); }
        [[nodiscard]] Label* buildPtr() const { return new Label(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit Label(const Config& cfg)
        : max_length(cfg.max_length) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
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

    explicit Property(const Config& cfg)
        : ptr(cfg.ptr), format(cfg.format), values(cfg.values), with_values(cfg.with_values) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
};
template class Property<uint8_t>;
template class Property<int8_t>;
template class Property<float>;
template class Property<bool>;

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

    explicit StringProperty(const Config& cfg)
        : ptr(cfg.ptr) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
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

    String suffix;

    explicit NumberPicker(const Config& cfg)
        : number(cfg.number), cursor(cfg.cursor), minimum(cfg.min),
          maximum(cfg.max), precision(cfg.precision),
          suffix(cfg.suffix) { icon = cfg.icon; title = cfg.title; }

    [[nodiscard]] T getAbsoluteValue() const;
    [[nodiscard]] T getValue() const;

    void render(Adafruit_GFX& display, bool minimalized) override;
    void renderInline(Adafruit_GFX& display) override;
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
        String suffix = "";
        std::vector<String> items{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& pointer(uint8_t* s) { c_.selection = s; return *this; }
        Builder& suffix(const String& s) { c_.suffix = s; return *this; }
        Builder& items(const std::vector<String>& v) { c_.items = v; return *this; }
        Builder& items(const std::initializer_list<String>& v) { c_.items = v; return *this; }
        Builder& addItem(const String& s) { c_.items.push_back(s); return *this; }

        [[nodiscard]] Selector build() const { return Selector(c_); }
        [[nodiscard]] Selector* buildPtr() const { return new Selector(c_); }
    };

    static Builder make() { return Builder{}; }

    String suffix;

    explicit Selector(const Config& cfg)
        : selection(cfg.selection), cursor(cfg.selection != nullptr ? *cfg.selection : 0), items(cfg.items), suffix(cfg.suffix) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
    void renderInline(Adafruit_GFX& display) override;
    bool update(char key) override;
};


class Toggle : public UIClickable {
    bool* ptr;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        bool* pointer = nullptr;
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

    explicit Toggle(const Config& cfg)
        : ptr(cfg.pointer) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
    void activate(Adafruit_GFX& display) override;
};


class Button : public UIClickable {
    bool* ptr;
    std::function<void()> on_click;
public:
    struct Config {
        char icon = 0x00;
        String title = "";
        std::function<void()> on_click = []{};
    };

    class Builder {
        Config c_;
    public:
        Builder& icon(char i) { c_.icon = i; return *this; }
        Builder& title(const String& t) { c_.title = t; return *this; }
        Builder& onClick(std::function<void()> f) { c_.on_click = std::move(f); return *this; }

        [[nodiscard]] Button build() const { return Button(c_); }
        [[nodiscard]] Button* buildPtr() const { return new Button(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit Button(const Config& cfg)
        : on_click(cfg.on_click) { icon = cfg.icon; title = cfg.title; }

    void render(Adafruit_GFX& display, bool minimalized) override;
    void activate(Adafruit_GFX& display) override;
};


class TextField : public UIInline {
    char* ptr;
    int16_t cursor;
    uint8_t max_length;
    uint8_t window_size;
    uint8_t slice_at = 0;
    std::function<void(char*)> on_submit;
    bool submittable;
    bool owns_mem = false;
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

    bool spacer;

    explicit TextField(const Config& cfg)
        : ptr(cfg.ptr), cursor(-1), max_length(cfg.max_length),
          window_size(cfg.window_size ? std::min<uint8_t>(cfg.max_length, cfg.window_size) : cfg.max_length),
          on_submit(cfg.on_submit), submittable(cfg.submittable), spacer(cfg.spacer) {
        icon = cfg.icon; title = cfg.title;
        if (!ptr) {
            ptr = new char[max_length+1];
            ptr[0] = '\0';
            owns_mem = true;
        }
    }

    ~TextField() override {
        if (owns_mem) {
            delete[] ptr;
        }
    }

    void render(Adafruit_GFX& display, bool minimalized) override;
    void renderInline(Adafruit_GFX& display) override;
    bool update(char key) override;
};

#pragma endregion

#pragma region Modals

class Alert : public UIModal {
public:
    struct Config {
        String message = "";
    };

    class Builder {
        Config c_;
    public:
        Builder& message(const String& m) { c_.message = m; return *this; }

        [[nodiscard]] Alert build() const { return Alert(c_); };
        [[nodiscard]] Alert* buildPtr() const { return new Alert(c_); };
    };

    String message;

    static Builder make() { return Builder{}; }

    explicit Alert(const Config& cfg)
        : message(cfg.message) {}

    void render(Adafruit_GFX& display) override;
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

    explicit CharTable(const Config& cfg) : max_lines(cfg.max_lines) { icon = 0x00; title = "Characters"; };

    void render(Adafruit_GFX& display, bool minimalized) override;
    bool update(char key) override;
};

#pragma endregion

#endif
