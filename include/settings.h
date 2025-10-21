#pragma once

#include <Adafruit_GFX.h>
#include <EEPROM.h>

#define EEPROM_SIZE     1024
#define CFG_VERSION     0x04

struct SettingsData {
    float   radio_frequency =   868.000f;
    uint8_t radio_bandwidth =   1; // 125.0kHz default
    uint8_t radio_sf =          9;
    uint8_t radio_cr =          7;
    int8_t  radio_power =       10;
    uint8_t radio_preamble =    8;
    uint8_t radio_band =        0;

    uint8_t display_contrast =  50;
    uint8_t display_backlight = 128;
    bool    display_inverted =  false;
    uint8_t display_rotation =  0;
    bool    display_icons =     true;
    bool    display_inv_alert = false;

    char    device_name[16] =  "mein_radio";

    uint16_t crc = 0;
};

class Settings {
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
