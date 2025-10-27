#pragma once
#include <WString.h>

union Color {
    struct {
        union { uint8_t r, red; };
        union { uint8_t g, green; };
        union { uint8_t b, blue; };
    };

    uint8_t raw[3];

    uint16_t as565() const;
    uint32_t pack() const;
    static Color unpack(uint32_t rgb);
    static Color from565(uint16_t rgb);
    static Color fromHSV(float h, float s, float v);
};


uint16_t crc16(const uint8_t* p, size_t n);
uint32_t crc32(const uint8_t* p, size_t n);
uint64_t crc64(const uint8_t* p, size_t n);
int64_t pow10i(uint8_t n);
String prettyValue(uint64_t value, const String& symbol, uint8_t precision = 0, uint16_t per_kilo = 1000);