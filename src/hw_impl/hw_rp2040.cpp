#ifdef ARDUINO_ARCH_RP2040
#include "hw_impl/hw_rp2040.h"
#include "configuration.h"

TwoWire*        extI2C =  &Wire1; // Wire1 because of the small boards
SPIClassRP2040* extSPI =  &SPI;
SPIClassRP2040* extSPI1 = &SPI1;

DriverBase* driver = new DriverRP2040();

uint32_t DriverRP2040::currentClock() const {
    return clock_get_hz(clk_sys);
}

uint32_t DriverRP2040::currentRamBSS() const {
    extern char __bss_start__, __bss_end__;
    return &__bss_end__ - &__bss_start__;
}

uint32_t DriverRP2040::currentRamData() const {
    extern char __data_start__, __data_end__;
    return &__data_end__ - &__data_start__;
}

uint32_t DriverRP2040::currentRamStack() const {
    extern char __StackLimit, __StackTop;
    uintptr_t sp;
    __asm volatile("mov %0, sp" : "=r"(sp));
    return reinterpret_cast<uintptr_t>(&__StackTop) - sp;
}

uint32_t DriverRP2040::currentRamHeap() const {
    return mallinfo().uordblks;
}

uint32_t DriverRP2040::currentFlash() const {
    extern char __flash_binary_start, __flash_binary_end;
    return &__flash_binary_end - &__flash_binary_start;
}

uint32_t DriverRP2040::boardId() const {
    pico_unique_board_id_t id{};
    pico_get_unique_board_id(&id);
    return crc32(id.id, sizeof(id.id));
}

void DriverRP2040::init() {
    extI2C->setSCL(EXT_I2C_SCL);
    extI2C->setSDA(EXT_I2C_SDA);

    extSPI->setMISO(EXT_SPI0_MISO);
    extSPI->setMOSI(EXT_SPI0_MOSI);
    extSPI->setSCK(EXT_SPI0_SCK);

    extSPI1->setMISO(EXT_SPI1_MISO);
    extSPI1->setMOSI(EXT_SPI1_MOSI);
    extSPI1->setSCK(EXT_SPI1_SCK);

    extI2C->begin();
    extSPI->begin();
    extSPI1->begin();
}

void DriverRP2040::reboot() {
    watchdog_enable(1, true);
    while (true);
}

#endif
