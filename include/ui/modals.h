#pragma once

#include <functional>

#include "base.h"

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

    void render(UIContext& ctx) override;
    bool update(UIContext& ctx, char key) override;
};

class ConfirmModal : public UIModal {
    bool flag = false;
    std::function<void()> on_confirm = [] {};
public:
    struct Config {
        String message = "";
        std::function<void()> on_confirm = [] {};

    };

    class Builder {
        Config c_;
    public:
        Builder& message(const String& m) { c_.message = m; return *this; }
        Builder& onConfirm(std::function<void()> f) { c_.on_confirm = std::move(f); return *this; }

        [[nodiscard]] ConfirmModal build() const { return ConfirmModal(c_); };
        [[nodiscard]] ConfirmModal* buildPtr() const { return new ConfirmModal(c_); };
    };

    String message;

    static Builder make() { return Builder{}; }

    explicit ConfirmModal(const Config& cfg)
        : message(cfg.message), on_confirm(cfg.on_confirm) {}

    void render(UIContext& ctx) override;
    bool update(UIContext& ctx, char key) override;
};