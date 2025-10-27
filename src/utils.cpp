#include "utils.h"

#include <cmath>
#include <iterator>

uint16_t Color::as565() const {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

uint32_t Color::pack() const {
    return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0);
}

Color Color::unpack(uint32_t rgb) {
    return {
        rgb >> 16 & 0xFF,
        rgb >> 8 & 0xFF,
        rgb & 0xFF
    };
}

Color Color::from565(uint16_t rgb) {
    uint8_t r5 = (rgb >> 11) & 0x1F;
    uint8_t g6 = (rgb >> 5)  & 0x3F;
    uint8_t b5 = (rgb >> 0)  & 0x1F;

    return {
        static_cast<uint8_t>(r5 / 31.0f * 255.0f),
        static_cast<uint8_t>(g6 / 63.0f * 255.0f),
        static_cast<uint8_t>(b5 / 31.0f * 255.0f)
    };
}

Color Color::fromHSV(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - std::fabs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float r, g, b;

    if (h < 60)       { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    return {(r + m) * 255.0f, (g + m) * 255.0f, (b + m) * 255.0f};
}


uint16_t crc16(const uint8_t* p, size_t n) {
    uint16_t crc = 0xFFFF;
    while (n--) {
        crc ^= static_cast<uint16_t>(*p++) << 8;
        for (uint8_t i=0; i<8; ++i)
            crc = (crc & 0x8000) ? (crc<<1) ^ 0x1021 : (crc<<1);
    }
    return crc;
}

uint32_t crc32(const uint8_t* p, size_t n) {
    uint32_t crc = 0xFFFFFFFF;
    while (n--) {
        crc ^= static_cast<uint32_t>(*p++) << 24;
        for (uint8_t i = 0; i < 8; ++i)
            crc = (crc & 0x80000000) ? (crc << 1) ^ 0x04C11DB7 : (crc << 1);
    }
    return crc ^ 0xFFFFFFFF;
}

uint64_t crc64(const uint8_t* p, size_t n) {
    uint64_t crc = 0xFFFFFFFFFFFFFFFFULL;
    while (n--) {
        crc ^= static_cast<uint64_t>(*p++) << 56;
        for (uint8_t i = 0; i < 8; ++i)
            crc = (crc & 0x8000000000000000ULL) ? (crc << 1) ^ 0x42F0E1EBA9EA3693ULL : (crc << 1);
    }
    return crc ^ 0xFFFFFFFFFFFFFFFFULL;
}

int64_t pow10i(uint8_t n) {
    int64_t p = 1;
    while (n--) p *= 10;
    return p;
}

String prettyValue(uint64_t value, const String& symbol, uint8_t precision, uint16_t per_kilo) {
    static const char* prefixes[] = {"", "K", "M", "G", "T", "P"};
    uint8_t prefix_idx = 0;
    double scaled = value;

    while (scaled >= per_kilo && prefix_idx < std::size(prefixes) - 1) {
        scaled /= per_kilo;
        prefix_idx++;
    }

    String result{};
    precision = (scaled < 10 && prefix_idx > 0 && precision > 0) ? (precision) : 0;
    result = String(scaled, precision);
    result += prefixes[prefix_idx];
    result += symbol;

    return result;
}
