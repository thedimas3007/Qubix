#pragma once

#include <functional>

#include "base.h"

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

    void render(UIContext& ctx, bool minimalized) override;
    void renderInline(UIContext& ctx) override;
    bool update(UIContext& ctx, char key) override;
};
template class NumberPicker<uint8_t>;
template class NumberPicker<int8_t>;
template class NumberPicker<float>;


class Selector : public UIInline {
    uint8_t* selection;
    std::vector<String> items;
    int8_t cursor = -1;
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
        : selection(cfg.selection), items(cfg.items), suffix(cfg.suffix) { icon = cfg.icon; title = cfg.title; }

    void render(UIContext& ctx, bool minimalized) override;
    void renderInline(UIContext& ctx) override;
    bool update(UIContext& ctx, char key) override;
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

    void render(UIContext& ctx, bool minimalized) override;
    void activate(UIContext& ctx) override;
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

    void render(UIContext& ctx, bool minimalized) override;
    void activate(UIContext& ctx) override;
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
        if (owns_mem) delete[] ptr;
    }

    void render(UIContext& ctx, bool minimalized) override;
    void renderInline(UIContext& ctx) override;
    bool update(UIContext& ctx, char key) override;
};
