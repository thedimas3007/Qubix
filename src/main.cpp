#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <RadioLib.h>
#include <vector>

#include "configuration.h"
#include "ui_core.h"

MbedI2C extI2C(EXT_I2C_SDA, EXT_I2C_SCL);
MbedSPI extSPI(EXT_SPI_MISO, EXT_SPI_MOSI, EXT_SPI_SCK);

#ifdef TARGET_SH1106
Adafruit_SH1106G display(128, 64, &extI2C, -1);
#elif defined(TARGET_SSD1306)
Adafruit_SSD1306 display(128, 64, &extI2C, -1);
#endif

SX1262 radio = new Module(RADIO_CS, RADIO_IRQ, RADIO_RESET, RADIO_BUSY, extSPI);

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag() {
    if (!enableInterrupt) return;
    receivedFlag = true;
}

String prettyValue(uint64_t value, const String& symbol, uint8_t precision = 0, uint16_t per_kilo = 1000) {
    String prefixes[] = {"", "K", "M", "G"};
    int power = 0;
    double scaled = static_cast<double>(value);

    while (scaled >= per_kilo && power < 3) {
        scaled /= per_kilo;
        ++power;
    }

    char fmt[16];
    std::snprintf(fmt, sizeof(fmt), "%%.%uf", precision);

    char numbuf[64];
    std::snprintf(numbuf, sizeof(numbuf), fmt, scaled);
    return numbuf + prefixes[power] + symbol;
}

String buffer = "";

struct Settings {
    float radio_frequency;
    float radio_bandwidth;
    uint8_t radio_sf;
    uint8_t radio_cr;
    int8_t radio_power;
    uint8_t radio_preamble;
};


Settings settings {868.000, 125.000, 9, 7, 10, 8};

Stack root = Stack({
    new Label("\xAD\x99\x9A             \xAF \x9D\xA1\xA3"),
    new MenuView("Radio", {
        new MenuView('\x8C', "Broadcast"),
        new MenuView('\x8D', "Settings", {
            new MenuView('\xAD', "Radio", {
                new NumberPicker<float>("Freq", &settings.radio_frequency, 868.000, 915.000, 3, 3),
                new NumberPicker<float>("Bandw", &settings.radio_bandwidth, 31.25, 500.00, 2, 2),
                new NumberPicker<uint8_t>("SF", &settings.radio_sf, 5, 12),
                new NumberPicker<uint8_t>("CR", &settings.radio_cr, 5, 8),
                new NumberPicker<int8_t>("Power", &settings.radio_power, -10, 22),
            }),
            new MenuView('\x95', "Display")
        }),
        new MenuView('*', "Tools"),
        new MenuView('\x91', "Debug", {
            new CharTable("Characters"),
            new MenuView("Settings", {
                new Property<float>("Freq", &settings.radio_frequency, "%.3fmHz"),
                new Property<float>("Bandw", &settings.radio_bandwidth, "%.2fkHz"),
                new Property<uint8_t>("SF", &settings.radio_sf, "%d"),
                new Property<uint8_t>("CR", &settings.radio_cr, "%d"),
                new Property<int8_t>("Power", &settings.radio_power, "%ddBm"),
            })
        }),
        new MenuView('\x93', "Power"),
        new MenuView('i', "Info", {
            new MenuView("Device", {
                new Label(String("MCU:   ") + HW_MCU),
                new Label(String("Clock: ") + prettyValue(HW_F_CPU, "Hz", 0, 1000)),
                new Label(String("RAM:   ") + prettyValue(HW_RAM_BYTES, "B", 0, 1024)),
                new Label(String("Flash: ") + prettyValue(HW_FLASH_BYTES, "B", 0, 1024)),
            }),
            new MenuView("Libs", {
                new Label("Work in progress")
            }),
        })
    })
});

void setup() {
    Serial1.begin(115200);
    extI2C.begin();
    extSPI.begin();

#ifdef TARGET_SH1106
    display.begin(DISPLAY_ADDRESS, true);
#endif

#ifdef TARGET_SSD1306
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);
#endif

    display.cp437();
    display.setTextColor(DISPLAY_FG);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Loading radio...");
    display.display();

    int state = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 1.6, false);
    radio.setCurrentLimit(60.0);
    radio.setDio2AsRfSwitch(true);
    radio.explicitHeader();
    radio.setCRC(true);

#ifdef RADIO_SLAVE
    radio.setDio1Action(setFlag);
    radio.startReceive();
#endif

    if (state == RADIOLIB_ERR_NONE) {
        display.println("Radio OK!");
    } else {
        display.println("Radio ERROR!");
        display.println(state);
        display.display();
        while (true) {}
    }

    display.display();
    delay(1000);
}

void loop() {
    extI2C.requestFrom(KEYBOARD_ADDRESS, 1);
    while (extI2C.available()) {
        char c = extI2C.read();
        if (c == 0) continue;
        root.update(c);
        // Serial1.println(menu_pointer);
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    root.render(&display, false);
    display.display();
}

// Void for Radio
void loop_old() {
#ifndef RADIO_SLAVE
    extI2C.requestFrom(KEYBOARD_ADDRESS, 1);
    while (extI2C.available()) {
        char c = extI2C.read();

        if (c == 0x0D) { // Enter
            if (buffer.length() > 0) {
                int state = radio.transmit(buffer);
                buffer = "";
                display.clearDisplay();
                display.setCursor(0, 0);
                display.print("[STATE ");
                display.print(state);
                display.print("]");
                display.display();
                delay(750);
            }
        } else if (c == 0x08) { // Backspace
            if (buffer.length() > 0) buffer.remove(buffer.length() - 1);
        } else if (c != 0) {
            buffer += c;
        }

        if (c != 0) {
            printf("%c\t0x%02x\n", c, c);
        }
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print(buffer);
        if ((millis() / 500) % 2) display.print("_");
        display.display();
    }
#else
    if (receivedFlag) {
        enableInterrupt = false;
        receivedFlag = false;

        String str;
        int state = radio.readData(str);

        display.clearDisplay();
        display.setCursor(0, 0);
        if (state == RADIOLIB_ERR_NONE) {
            display.println(str);
        } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
            display.println("[TIMEOUT]");
        } else {
            display.print("[ERROR ");
            display.print(state);
            display.print("]");
        }
        display.display();
        radio.startReceive();

        enableInterrupt = true;
    }
#endif
}
