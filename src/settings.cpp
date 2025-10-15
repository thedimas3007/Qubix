#include "configuration.h"
#include "settings.h"

uint16_t Settings::crc16(const uint8_t* p, size_t n) {
    uint16_t crc = 0xFFFF;
    while (n--) {
        crc ^= static_cast<uint16_t>(*p++) << 8;
        for (uint8_t i=0; i<8; ++i)
            crc = (crc & 0x8000) ? (crc<<1) ^ 0x1021 : (crc<<1);
    }
    return crc;
}

void Settings::setDefaults() {
    data = SettingsData{};
}

bool Settings::begin() {
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(EEPROM_SIZE);
#endif

    const uint8_t ver = EEPROM.read(0);
    if (ver != CFG_VERSION) {
        setDefaults();
        save();
        return true;
    }

    return load();
}

void Settings::end() {
#if   defined(ARDUINO_ARCH_RP2040)
    EEPROM.end();
#endif
}

bool Settings::load() {
    SettingsData tmp;
    EEPROM.get(1, tmp);

    const uint16_t old_crc = tmp.crc;
    tmp.crc = 0;
    const size_t n = sizeof(SettingsData) - sizeof(tmp.crc);
    const uint16_t want = crc16(reinterpret_cast<const uint8_t*>(&tmp), n);

    if (old_crc != want) {
        setDefaults();
        save();
        return true;
    } else {
        data = tmp;
        return false;
    }
}

void Settings::save() const {
    SettingsData tmp = data;
    const size_t n = sizeof(SettingsData) - sizeof(tmp.crc);
    tmp.crc = crc16(reinterpret_cast<const uint8_t*>(&tmp), n);

    EEPROM.write(0, CFG_VERSION);
    EEPROM.put(1, tmp);
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif
}

void Settings::wipe(bool write) {
    for (int i = 0; i < EEPROM.length(); ++i) {
        EEPROM.write(i, 0);
    }
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif

    if (write) {
        setDefaults();
        save();
    }
}

void Settings::applyDisplay(Adafruit_GFX& gfx) const {
#ifdef HAS_CONTRAST
    setContrast(data.display_contrast);
#endif
    gfx.setRotation(data.display_flipped ? 2 : 0);
    gfx.invertDisplay(data.display_inverted);
#ifdef HAS_BACKLIGHT
    setBacklight(data.display_backlight);
#endif
}
