#pragma once

#include "base.h"

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
