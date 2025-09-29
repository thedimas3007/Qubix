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
    uint8_t radio_band;
    String device_name;
};

std::vector<String> bands = {"B1@LP","B2@GP","B3@GP","B4@LP","B5@HP","B6@SP","B7@GP"};
Settings settings {868.000, 125.000, 9, 7, 10, 8, 0};

Stack root = Stack::make().children({
    Label::make().title("\xAD\x99\x9A             \xAF \x9D\xA1\xA3").buildPtr(),

    MenuView::make().title("Radio").children({
        MenuView::make().icon('\x8C').title("Broadcast").buildPtr(),
        MenuView::make().icon('\x8D').title("Settings").children({
            MenuView::make().icon('\xAD').title("Radio").children({
                NumberPicker<float>::make().icon('\x90').title("Freq").suffix("mHz").pointer(&settings.radio_frequency).min(868.000).max(915.000).precision(3).cursor(3).buildPtr(),
                NumberPicker<float>::make().icon('\x1D').title("Bandw").suffix("kHz").pointer(&settings.radio_bandwidth).min(31.25).max(500.00).precision(2).cursor(2).buildPtr(),
                NumberPicker<uint8_t>::make().icon('\x12').title("SF").pointer(&settings.radio_sf).min(5).max(12).buildPtr(),
                NumberPicker<uint8_t>::make().icon('\xAF').title("CR").pointer(&settings.radio_cr).min(5).max(8).buildPtr(),
                NumberPicker<int8_t>::make().icon('\x8C').title("Power").suffix("dBm").pointer(&settings.radio_power).min(-15).max(22).buildPtr(),
                Selector::make().icon('x').title("Band").pointer(&settings.radio_band).items(bands).buildPtr()
            }).onExit([] {
                radio.setFrequency(settings.radio_frequency);
                radio.setBandwidth(settings.radio_bandwidth);
                radio.setSpreadingFactor(settings.radio_sf);
                radio.setCodingRate(settings.radio_cr);
                radio.setOutputPower(settings.radio_power);
                Serial1.println("Radio updated");
            }).buildPtr(),
            MenuView::make().icon('\x95').title("Display").buildPtr(),
            MenuView::make().icon('\x91').title("Device").children({
                Input::make().title("Name").pointer(&settings.device_name).buildPtr()
            }).buildPtr(),
        }).buildPtr(),

        MenuView::make().icon('*').title("Tools").buildPtr(),

        MenuView::make().icon('\x91').title("Debug").children({
            CharTable::make().title("Characters").buildPtr(),
            MenuView::make().title("Settings").children({
                Property<float>::make().title("Freq").pointer(&settings.radio_frequency).fmt("%.3fmHz").buildPtr(),
                Property<float>::make().title("Bandw").pointer(&settings.radio_bandwidth).fmt("%.2fkHz").buildPtr(),
                Property<uint8_t>::make().title("SF").pointer(&settings.radio_sf).fmt("%d").buildPtr(),
                Property<uint8_t>::make().title("CR").pointer(&settings.radio_cr).fmt("%d").buildPtr(),
                Property<int8_t>::make().title("Power").pointer(&settings.radio_power).fmt("%ddBm").buildPtr(),
                Property<uint8_t>::make().title("Band").pointer(&settings.radio_band).values(bands).buildPtr(),
                StringProperty::make().title("Name").pointer(&settings.device_name).buildPtr(),
            }).buildPtr()
        }).buildPtr(),

        MenuView::make().icon('\x93').title("Power").buildPtr(),

        MenuView::make().icon('i').title("Info").children({
            MenuView::make().title("Device").children({
                Label::make().title(String("MCU:   ") + HW_MCU).buildPtr(),
                Label::make().title(String("Clock: ") + prettyValue(HW_F_CPU,"Hz",0,1000)).buildPtr(),
                Label::make().title(String("RAM:   ") + prettyValue(HW_RAM_BYTES,"B",0,1024)).buildPtr(),
                Label::make().title(String("Flash: ") + prettyValue(HW_FLASH_BYTES,"B",0,1024)).buildPtr()
            }).buildPtr(),
            MenuView::make().title("Libs").children({
                Label::make().title("Work in progress").buildPtr()
            }).buildPtr()
        }).buildPtr()
    }).buildPtr()
}).build();

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
