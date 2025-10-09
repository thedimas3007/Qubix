#pragma once

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
