#pragma once
#include <Arduino.h>

class ReadBuffer {
    const uint8_t* buf;
    size_t size, pos;
public:
    ReadBuffer(const uint8_t* b, size_t s) : buf(b), size(s), pos(0) {}
    ~ReadBuffer()   { delete[] buf; }

    bool available(size_t n = 1) const
                            { return pos + n <= size; }

    const uint8_t* raw() const
                            { return buf; }

    size_t len() const      { return size; }
    void reset()            { pos = 0; }

    uint8_t operator[](const size_t n) const
                            { return n < size ? buf[n] : 0; }

    uint8_t u8()            { return available(1) ? buf[pos++] : 0; }
    uint16_t u16()          { return available(2) ? u8() | (u8() << 8) : 0; }
    uint32_t u32()          { return available(4) ? (u16() | (u16() << 16)) : 0; }
    uint64_t u64()          { return available(8) ? (u32() | (static_cast<uint64_t>(u32()) << 32)) : 0; }

    int8_t  i8()            { return static_cast<int8_t>(u8()); }
    int16_t i16()           { return static_cast<int16_t>(u16()); }
    int32_t i32()           { return static_cast<int32_t>(u32()); }
    int64_t i64()           { return static_cast<int64_t>(u64()); }

    char c()                { return static_cast<char>(u8()); }

    String str() {
        if (!available(1)) return "";
        String s(reinterpret_cast<const char*>(buf + pos));
        pos += s.length() + 1 <= size ? s.length() + 1 : size - pos;
        return s;
    }
};


class WriteBuffer {
    uint8_t* buf;
    size_t size, pos;
public:
    WriteBuffer(size_t s) : buf(new uint8_t[s]()), size(s), pos(0) {}
    ~WriteBuffer() { delete[] buf; }

    bool available(size_t n = 1) const
                            { return pos + n <= size; }

    uint8_t* raw() const    { return buf; }

    size_t len() const      { return pos; }
    size_t capacity() const { return size; }
    void reset()            { pos = 0; }

    void u8(uint8_t v)      { if (available(1)) buf[pos++] = v; }
    void u16(uint16_t v)    { if (available(2)) { u8(v & 0xFF); u8(v >> 8); } }
    void u32(uint32_t v)    { if (available(4)) { u16(v & 0xFFFF); u16(v >> 16); } }
    void u64(uint64_t v)    { if (available(8)) { u32(v & 0xFFFFFFFF); u32(v >> 32); } }

    void i8(int8_t v)       { u8(static_cast<uint8_t>(v)); }
    void i16(int16_t v)     { u16(static_cast<uint16_t>(v)); }
    void i32(int32_t v)     { u32(static_cast<uint32_t>(v)); }
    void i64(int64_t v)     { u64(static_cast<uint64_t>(v)); }

    void c(char v)          { u8(static_cast<uint8_t>(v)); }

    void str(const String& s) {
        size_t n = s.length() + 1;
        if (!available(n)) n = size - pos;
        memcpy(buf + pos, s.c_str(), min(n, s.length()));
        if (pos + n <= size) buf[pos + n - 1] = '\0';
        pos += n;
    }
};




