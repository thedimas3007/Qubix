#include <algorithm>
#include <Arduino.h>
#include <cstdio>
#include <RadioLib.h>
#include <vector>

#include "configuration.h"
#include "keycodes.h"
#include "settings.h"
#include "timing.h"
#include "utils.h"

#include "network/packet.h"
#include "network/packet_types.h"

#include "ui/base.h"
#include "ui/extra.h"
#include "ui/inputs.h"
#include "ui/modals.h"
#include "ui/stackers.h"
#include "ui/static.h"

#if   defined(TARGET_SH1106)
Adafruit_SH1106G display(128, 64, extI2C, -1);
#elif defined(TARGET_SSD1306)
Adafruit_SSD1306 display(128, 64, extI2C, -1);
#elif defined(TARGET_ST7567)
ST7567 display(128, 64, extSPI1, DISPLAY_DC, DISPLAY_RESET, DISPLAY_CS);
#elif defined(TARGET_SSD1351)
Buffered_SSD1351 display(128, 128, extSPI1, DISPLAY_CS, DISPLAY_DC, DISPLAY_RESET);
#elif defined(TARGET_SSD1681)
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display = GxEPD2_154_D67(DISPLAY_CS, DISPLAY_DC, DISPLAY_RESET, DISPLAY_BUSY);
#endif

SX1262 radio = new Module(RADIO_CS, RADIO_IRQ, RADIO_RESET, RADIO_BUSY, *extSPI);

Scheduler scheduler;
NetManager netman;
UIContext ui_context(display);

std::vector<String> bands = {"B1@LP","B2@GP","B3@GP","B4@LP","B5@HP","B6@SP","B7@GP"};
std::vector<float> bandwidths_float = {62.5, 125.0, 250.0, 500.0 };
std::vector<String> bandwidths = {"62.5kHz", "125.0kHz", "250.0kHz", "500.0kHz" };

uint32_t last_update;
uint32_t netman_update;
volatile bool received_flag = false;
volatile bool enable_interrupt = true;

auto message_menu = MenuView::make().fill(FillMode::TOP).buildPtr();
auto nodes_menu = MenuView::make().title("Nodes").buildPtr();

void setFlag() {
    if (!enable_interrupt) return;
    received_flag = true;
}

void updateNodes() {
    nodes_menu->clearChildren();
    auto pkt = std::make_unique<Preved>();
    // pkt->hwid(driver->boardId());
    netman.queue(std::move(pkt), 0xFFFFFFFF);
}

UIApp root = UIApp::make().ctx(&ui_context).title("\xAD\x99\x9A               \x9D\xA1\xA3").root(
    MenuView::make().title("Radio").children({
        TabSelector::make().icon('\x8C').title("Broadcast").children({
            TextField::make().title(">").spacer(false).maxLength(MESSAGE_LENGTH-1).onSubmit([](char* buf) {
                if (!strlen(buf)) return;
                // enable_interrupt = false;

                int16_t ch_status = radio.scanChannel();
                if (ch_status == RADIOLIB_CHANNEL_FREE) {
                    int16_t tx_status = radio.transmit(buf);
                    if (tx_status == RADIOLIB_ERR_NONE) {
                        message_menu->addChild(Label::make().icon('\xBD').title(buf).buildPtr());
                    } else {
                        root.addModal(Alert::make().message("Err: " + String(tx_status)).buildPtr());
                    }
                } else if (ch_status == RADIOLIB_LORA_DETECTED) { // maybe remove detection at all?
                    root.addModal(ConfirmModal::make().message("Busy channel" + String(ch_status)).onConfirm([buf] {
                        int16_t tx_status = radio.transmit(buf);
                        if (tx_status == RADIOLIB_ERR_NONE) {
                            message_menu->addChild(Label::make().icon('\xBD').title(buf).buildPtr());
                        } else {
                            root.addModal(Alert::make().message("Err: " + String(tx_status)).buildPtr());
                        }
                    }).buildPtr());
                } else {
                    root.addModal(Alert::make().message("Err: " + String(ch_status)).buildPtr());
                }

                // enable_interrupt = true;
                radio.startReceive();
            }).buildPtr(),
            message_menu
        }).buildPtr(),

        MenuView::make().icon('\x8D').title("Settings").children({
            MenuView::make().icon('\xAD').title("Radio").children({
                NumberPicker<float>::make().icon('\x90').title("Freq").suffix("mHz").pointer(&settings.data.radio_frequency).min(863.000).max(870.000).precision(3).cursor(3).buildPtr(),
                Selector::make().icon('\x1D').title("Bandw").pointer(&settings.data.radio_bandwidth).items(bandwidths).buildPtr(),
                NumberPicker<uint8_t>::make().icon('\x12').title("SF").pointer(&settings.data.radio_sf).min(5).max(12).buildPtr(),
                NumberPicker<uint8_t>::make().icon('\xAF').title("CR").pointer(&settings.data.radio_cr).min(5).max(8).buildPtr(),
                NumberPicker<int8_t>::make().icon('\x8C').title("Power").suffix("dBm").pointer(&settings.data.radio_power).min(-9).max(22).buildPtr(),
                Selector::make().icon('x').title("Band").pointer(&settings.data.radio_band).items(bands).buildPtr()
            }).onExit([] {
                settings.save();
                radio.setFrequency(settings.data.radio_frequency);
                radio.setBandwidth(bandwidths_float[settings.data.radio_bandwidth]);
                radio.setSpreadingFactor(settings.data.radio_sf);
                radio.setCodingRate(settings.data.radio_cr);
                radio.setOutputPower(settings.data.radio_power);
            }).buildPtr(),
            MenuView::make().icon('\x95').title("Display").children({
#ifdef HAS_CONTRAST
                NumberPicker<uint8_t>::make().title("Contrast").pointer(&settings.data.display_contrast).buildPtr(),
#endif
#ifdef HAS_BACKLIGHT
                NumberPicker<uint8_t>::make().title("Backlight").pointer(&settings.data.display_backlight).buildPtr(),
#endif
                NumberPicker<uint8_t>::make().title("Rotation").pointer(&settings.data.display_rotation).min(0).max(3).buildPtr(),
                Toggle::make().title("Inverted").pointer(&settings.data.display_inverted).buildPtr(),
                Toggle::make().title("Icons").pointer(&settings.data.display_icons).buildPtr(),
                Toggle::make().title("Alert inv").pointer(&settings.data.display_inv_alert).buildPtr()
            }).onExit([] {
                settings.save();
                settings.applyDisplay(display);
                // display.display();
            }).buildPtr(),
            MenuView::make().icon('\x91').title("Device").children({
                TextField::make().title("Name").pointer(settings.data.device_name).maxLength(15).buildPtr(),
            }).onExit([] {
                settings.save();
            }).buildPtr(),
        }).buildPtr(),

        MenuView::make().icon('*').title("Tools").children({
            nodes_menu,
            BandScanner::make().radio(&radio).buildPtr(),
        }).buildPtr(),

        MenuView::make().icon('\x91').title("Debug").children({
#ifdef HAS_COLOR
            ColorInput<uint16_t>::make().pointer(&ui_context.theme.fg).title("Foreground").buildPtr(),
            ColorInput<uint16_t>::make().pointer(&ui_context.theme.bg).title("Background").buildPtr(),
#endif
            CharTable::make().buildPtr(),
            SizeDemo::make().buildPtr(),
            MenuView::make().title("Settings").children({
                Property<float>::make().title("Freq").pointer(&settings.data.radio_frequency).fmt("%.3fmHz").buildPtr(),
                Property<uint8_t>::make().title("Bandw").pointer(&settings.data.radio_bandwidth).values(bandwidths).buildPtr(),
                Property<uint8_t>::make().title("SF").pointer(&settings.data.radio_sf).fmt("%d").buildPtr(),
                Property<uint8_t>::make().title("CR").pointer(&settings.data.radio_cr).fmt("%d").buildPtr(),
                Property<int8_t>::make().title("Power").pointer(&settings.data.radio_power).fmt("%ddBm").buildPtr(),
                Property<uint8_t>::make().title("Band").pointer(&settings.data.radio_band).values(bands).buildPtr(),
                Label::make().title("====").buildPtr(),
#ifdef HAS_CONTRAST
                Property<uint8_t>::make().title("Contrast").pointer(&settings.data.display_contrast).fmt("%d").buildPtr(),
#endif
#ifdef HAS_BACKLIGHT
                Property<uint8_t>::make().title("Backlight").pointer(&settings.data.display_backlight).fmt("%d").buildPtr(),
#endif
                Property<uint8_t>::make().title("Rotation").pointer(&settings.data.display_rotation).fmt("%d").buildPtr(),
                Property<bool>::make().title("Inverted").pointer(&settings.data.display_inverted).fmt("%d").buildPtr(),
                Label::make().title("====").buildPtr(),
                StringProperty::make().title("Name").pointer(settings.data.device_name).buildPtr(),
                Label::make().title("====").buildPtr(),
                Label::make().title(String("MCU:   ") + HW_MCU).buildPtr(),
                Label::make().title(String("Clock: ") + prettyValue(HW_F_CPU,"Hz",0,1000)).buildPtr(),
                Label::make().title(String("RAM:   ") + prettyValue(HW_RAM_BYTES,"B",0,1024)).buildPtr(),
                Label::make().title(String("Flash: ") + prettyValue(HW_FLASH_BYTES,"B",0,1024)).buildPtr(),
                Label::make().title("====").buildPtr(),
                Button::make().title("Clock").onClick([] {
                    root.addModal(Alert::make().message(
                        // prettyValue(driver->currentFlash(), "B", 1, 1024) + "/" + prettyValue(driver->maxFlash(), "B", 1, 1024)
                        prettyValue(driver->currentClock(), "Hz") + "/" + prettyValue(driver->maxClock(), "Hz")
                    ).buildPtr());
                }).buildPtr(),
                Button::make().title("RAM").onClick([] {
                    root.addModal(Alert::make().message(
                        // prettyValue(driver->currentFlash(), "B", 1, 1024) + "/" + prettyValue(driver->maxFlash(), "B", 1, 1024)
                        prettyValue(driver->currentRamHeap(), "B", 0, 1024) + "/" + prettyValue(driver->maxRam(), "B", 0, 1024)
                    ).buildPtr());
                }).buildPtr(),
            }).buildPtr(),
#ifdef HAS_COLOR
            ColorWheel::make().buildPtr(),
#endif
            Button::make().title("Wipe EEPROM").onClick([] {
                root.addModal(ConfirmModal::make().message("Are you sure?").onConfirm([] {
                    settings.wipe();
                    driver->reboot();
                }).buildPtr());
            }).buildPtr()
        }).buildPtr(),

        MenuView::make().icon('\x93').title("Power").buildPtr(),

        MenuView::make().icon('i').title("Info").children({
            MenuView::make().title("Device").children({
                LambdaProperty::make().title("MCU").func([] { return driver->mcu(); }).buildPtr(),
                LambdaProperty::make().title("Load").func([] { return stringf("%.1f%%", driver->currentLoad()*100); }).buildPtr(),
                LambdaProperty::make().title("Clock").func([] {
                    return prettyValue(driver->currentClock(),"Hz",0,1000);
                }).buildPtr(),
                // LambdaProperty::make().title("RAM").func([] {
                //     return prettyValue(driver->currentRam(),"B",0,1024) + "/" + prettyValue(driver->maxRam(),"B", 0, 1024);
                // }).buildPtr(),
                MenuView::make().title("RAM").children({
                    LambdaProperty::make().title("BSS").func([] {
                        return prettyValue(driver->currentRamBSS(),"B", 0,1024);
                    }).buildPtr(),
                    LambdaProperty::make().title("Data").func([] {
                        return prettyValue(driver->currentRamData(),"B", 0,1024);
                    }).buildPtr(),
                    LambdaProperty::make().title("Stack").func([] {
                        return prettyValue(driver->currentRamStack(),"B", 0,1024);
                    }).buildPtr(),
                    LambdaProperty::make().title("Heap").func([] {
                        return prettyValue(driver->currentRamHeap(),"B", 0,1024);
                    }).buildPtr(),
                }).buildPtr(),
                LambdaProperty::make().title("Flash").func([] {
                    return prettyValue(driver->currentFlash(),"B",0,1024) + "/" + prettyValue(driver->maxFlash(),"B", 0, 1024);
                }).buildPtr(),
            }).buildPtr(),
            MenuView::make().title("Libs").children({
                Label::make().title("Work in progress").buildPtr()
            }).buildPtr()
        }).buildPtr()
    }).buildPtr()
).build();


void setup() {
    driver->init();
    Serial.begin(115200);
    Serial.println("== Console for " + String(driver->mcu()) + " ==");

    bool settings_reset = settings.begin();

#ifdef TARGET_SH1106
    display.begin(DISPLAY_ADDRESS, true);
#elif defined(TARGET_SSD1306)
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);
#elif defined(TARGET_ST7567)
    display.begin();
#elif defined(TARGET_SSD1351)
    display.begin();
#elif defined(TARGET_SSD1681)
    display.epd2.selectSPI(*extSPI1, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    display.init();
#endif

    ui_context.display.cp437();
    settings.applyDisplay(display);
    ui_context.reset();

#if DISPLAY_MODE == DISPLAY_MODE_EINK // simple output mode for e-in
    display.setPartialWindow(0, 0, 200, 200);
    display.firstPage();
#endif

    if (settings_reset) ui_context.println("Settings reset...");
    ui_context.print("Radio...");
    // ui_context.println(prettyValue(SystemCoreClock, "Hz"));
    ui_context.flush();

    int state = radio.begin(settings.data.radio_frequency, bandwidths_float[settings.data.radio_bandwidth],
                            settings.data.radio_sf, settings.data.radio_cr,
                            RADIOLIB_SX126X_SYNC_WORD_PRIVATE,
                            settings.data.radio_power,
                            8, 1.6, false);
    radio.setCurrentLimit(60.0);
    radio.setDio2AsRfSwitch(true);
    radio.explicitHeader();
    radio.setCRC(1);
    radio.setDio1Action(setFlag);
    radio.startReceive();

    if (state == RADIOLIB_ERR_NONE) {
        ui_context.println("OK");
        ui_context.flush();
    } else {
        Serial.print("Radio error: ");
        Serial.println(state);
        ui_context.printf("ERROR %i", state);
        ui_context.flush();
        while (true) {}
    }

    ui_context.print("Events...");
    ui_context.flush();
    netman.begin(&radio, &enable_interrupt, &received_flag);

    netman.reg<HelloPacket>([](auto& /* manager */, const auto& packet) {
        char txt[11];
        snprintf(txt, sizeof(txt), "0x%08lX", (unsigned long)(packet.hwid()));
        root.addModal(Alert::make().message(txt).buildPtr());
    });

    netman.reg<Preved>([](auto& manager, const auto& packet) {
        auto pkt = std::make_unique<Medved>();
        // pkt->hwid(driver->boardId());
        pkt->mcu(driver->mcu());
        pkt->rssi(std::clamp<float>(radio.getRSSI(), -128, 127));
        pkt->snr(std::clamp<float>(radio.getSNR(), -128, 127));
        manager.queue(std::move(pkt), packet.hwid());
    });

    // netman.reg<Medved>([](auto& /* manager */, const Medved& packet) {
    //     char buf[11];
    //     snprintf(buf, sizeof(buf), "0x%08lX", packet.hwid());
    //     nodes_menu->addChild(MenuView::make().title("Node " + String(buf)).children({
    //         Label::make().title("HWID:" + String(buf)).buildPtr(),
    //         Label::make().title("MCU:" + packet.mcu()).buildPtr(),
    //         Label::make().title("RSSI:" + String(packet.rssi())).buildPtr(),
    //         Label::make().title("SNR:" + String(packet.snr())).buildPtr(),
    //     }).buildPtr());
    // });

    ui_context.println("OK");
    ui_context.flush();

    auto packet = std::make_unique<HelloPacket>();
    netman.queue(std::move(packet), 0xFFFFFFFF);

    Serial.println("Loaded");
    ui_context.println("Loaded!");
    ui_context.flush();

    scheduler.schedule([]() {
        if (netman.isTimedOut()) return;

        int16_t st = netman.tick();
        if (st) {
            // TODO: kinda error logger
            root.addModal(Alert::make().message("Error " + String(st)).buildPtr());
        }
    }, 1000 / NETMAN_UPS);

    scheduler.schedule([]() {
        if (DISPLAY_MODE == DISPLAY_MODE_EINK && !ui_context.refreshRequested()) return;
        ui_context.render(root);
    }, 1000 / DISPLAY_UPS);

    delay(1000);

    ui_context.refresh(true);
}


void loop() {
    driver->loadTick();
    scheduler.tick();

    extI2C->requestFrom(KEYBOARD_ADDRESS, 1);
    while (extI2C->available()) {
        char c = extI2C->read();
        if (c == 0) continue;

        if (c == KEY_FN_C) driver->reboot();
        else if (c == KEY_FN_R) updateNodes();
        else if (root.update(c)) ui_context.refresh();
    }

    if (received_flag) {
        enable_interrupt = false;
        received_flag = false;
        netman.received();
        enable_interrupt = true;
    }

    if (Serial.available()) {
        String s = Serial.readStringUntil('\n');
        s.trim();

        if (s == "locate") {
            Serial.println("Locating neighbors");
            auto pkt = std::make_unique<Preved>();
            netman.request<Medved>(std::move(pkt), [](auto& /* manager */, auto& packet) {
                Serial.println(stringf("Discovered: 0x%08lX - %s", packet.hwid(), packet.mcu().c_str()));
            }, [](bool success) {
                Serial.print("Found: ");
                Serial.println(success ? "yes" : "no");
            }, 0xFFFFFFFF, 5000);
        } else {
            Serial.println("Command " + s + " not found");
        }
    }
}
