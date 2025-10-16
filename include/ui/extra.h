#pragma once

#include <RadioLib.h>

#include "base.h"

class CharTable : public UIElement {
    int8_t start = 0;
public:
    struct Config {};

    class Builder {
        Config c_;
    public:
        [[nodiscard]] CharTable build() const { return CharTable(c_); }
        [[nodiscard]] CharTable* buildPtr() const { return new CharTable(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit CharTable(const Config& cfg) { icon = 0x00; title = "Characters"; };

    void render(UIContext& ctx, bool minimalized) override;
    bool update(UIContext& ctx, char key) override;
};


struct ScanEntry {
    float freq, rssi;
};

class BandScanner : public UIElement {
    // ScanEntry last_scan[2048]{};
    float last_scan[128];
    uint16_t scan_channels = 0;
    uint64_t scan_time = 0;
    SX1262* radioPtr;

    const float start = 863.000;
    const float end =   870.000;
    const float step =  (end - start) / 128;

    float min_rssi = 999;
    float max_rssi = -999;
public:
    struct Config {
        SX1262* radioPtr = nullptr;
    };

    class Builder {
        Config c_;
    public:
        Builder& radio(SX1262* r) { c_.radioPtr = r; return *this; }

        [[nodiscard]] BandScanner build() const { return BandScanner(c_); }
        [[nodiscard]] BandScanner* buildPtr() const { return new BandScanner(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit BandScanner(const Config& cfg)
        : radioPtr(cfg.radioPtr) {
        icon = 0x00; title = "Scanner";
        for (uint16_t i = 0; i < 128; i++) {
            last_scan[i] = -150.0f;
        }
    };

    void render(UIContext& ctx, bool minimalized) override;
    bool update(UIContext& ctx, char key) override;
};


class ColorWheel : public UIElement {
    int8_t start = 0;
public:
    struct Config {};

    class Builder {
        Config c_;
    public:
        [[nodiscard]] ColorWheel build() const { return ColorWheel(c_); }
        [[nodiscard]] ColorWheel* buildPtr() const { return new ColorWheel(c_); }
    };

    static Builder make() { return Builder{}; }

    explicit ColorWheel(const Config& cfg) { icon = 0x00; title = "Colors"; };

    void render(UIContext& ctx, bool minimalized) override;
    bool update(UIContext& ctx, char key) override;
};