#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <RadioLib.h>
#include <vector>

#include "configuration.h"
#include "keycodes.h"

SX1262 radio = new Module(RADIO_CS, RADIO_IRQ, RADIO_RESET, RADIO_BUSY, extSPI);

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag() {
    if (!enableInterrupt) return;
    receivedFlag = true;
}

String utf8(const String& s) {
    String out;
    out.reserve(s.length());

    for (uint16_t i = 0, k = s.length(); i < k; ) {
        char c = s[i++];

        if ((c == 0xD0 || c == 0xD1) && i < k) {
            char x = s[i++];

            if (c == 0xD0) {
                if (x == 0x81)      c = 0xA8;
                else if (x >= 0x90) c = x + 0x30;
                else                c = x;
            } else {
                if (x == 0x91)      c = 0xB8;
                else if (x <= 0x8F) c = x + 0x70;
                else                c = x;
            }
        }

        out += c;
    }
    return out;
}

String buffer = "";

#define MAX_ELEMENTS 6 // elements per page
std::vector<String> menus = {
    "Home",
    "Chats",
    "Settings",
    "Tools",
    "Status",
    "Debug",
    "Scanner",
    "Poweroff"
};



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
    display.println("Hello world...");
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

int16_t start = 0;
int16_t menu_pointer = 0;

void loop() {
    const int16_t n = menus.size();
    if (n <= 0) return;

    const int16_t window_size = min<int16_t>(MAX_ELEMENTS, n);

    extI2C.requestFrom(KEYBOARD_ADDRESS, 1);
    while (extI2C.available()) {
        char c = extI2C.read();
        if (c == 0) continue;

        if (c == KEY_UP) {
            menu_pointer = (menu_pointer - 1 + n) % n;
        } else if (c == KEY_DOWN) {
            menu_pointer = (menu_pointer + 1) % n;
        }

        const int margin = 1;
        int end_ex = start + window_size;
        if (end_ex > n) end_ex = n;

        if (menu_pointer < start + margin) {
            start = menu_pointer - margin;
            if (start < 0) start = 0;
        } else if (menu_pointer >= end_ex - margin) {
            start = menu_pointer - (window_size - 1 - margin);
            if (start < 0) start = 0;
        }

        Serial1.println(menu_pointer);
    }

    int end_ex = start + window_size;
    if (end_ex > n) {
        end_ex = n;
        if (start > n - window_size) start = max(0, n - window_size);
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("\xAD\x99\x9A             \xAF \x9D\xA1\xA3");
    display.println("Menu:");
    for (int16_t i = start; i < end_ex; ++i) {
        display.print(i == menu_pointer ? "\x1A" : " ");
        display.println(menus[i]);
    }
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
