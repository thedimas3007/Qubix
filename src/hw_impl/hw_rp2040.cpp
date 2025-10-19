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

uint32_t DriverRP2040::currentRam() const {
    extern char __StackLimit, __bss_end__;
    extern char *__brkval;

    uint32_t static_used = &__bss_end__ - &__StackLimit;
    uint32_t heap_used = 0;

    struct mallinfo mi = mallinfo();
    heap_used = mi.uordblks;

    return static_used + heap_used;
}

uint32_t DriverRP2040::currentFlash() const {
    extern char __flash_binary_start, __flash_binary_end;

    return (uint32_t)(&__flash_binary_end - &__flash_binary_start);
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
