#pragma once

#include <functional>

#include "base.h"

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

    void render(UIContext& ctx, bool minimalized) override;
    bool update(UIContext& ctx, char key) override;
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

    void render(UIContext& ctx, bool minimalized) override;
    bool update(UIContext& ctx, char key) override;
};
