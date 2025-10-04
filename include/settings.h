#pragma once

#ifndef QUBIX_SETTINGS_H
#define QUBIX_SETTINGS_H

#include <EEPROM.h>
#include "Adafruit_GFX.h"

#define EEPROM_SIZE     1024
#define CFG_VERSION     0x03

struct SettingsData {
    float   radio_frequency =   868.000f;
    uint8_t radio_bandwidth =   3; // 125.0kHz default
    uint8_t radio_sf =          9;
    uint8_t radio_cr =          7;
    int8_t  radio_power =       10;
    uint8_t radio_preamble =    8;
    uint8_t radio_band =        0;

    uint8_t display_contrast =  50;
    uint8_t display_backlight = 128;
    bool    display_inverted =  false;
    bool    display_flipped =   false;
    bool    display_icons =     true;

    char    device_name[16] =  "mein_radio";

    uint16_t crc = 0;
};

class Settings {
    static uint16_t crc16(const uint8_t* p, size_t n);
    void setDefaults();
public:
    SettingsData data{};

    bool begin();
    void end();
    bool load();
    void save() const;
    void wipe(bool write = false);

    void applyDisplay(Adafruit_GFX& gfx) const;
};

#endif