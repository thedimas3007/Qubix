#pragma once

#include "base.h"

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
