#ifdef ARDUINO_ARCH_STM32
#include "hw_impl/hw_stm32.h"
#include "configuration.h"

TwoWire* extI2C = new TwoWire(EXT_I2C_SDA, EXT_I2C_SCL);
SPIClass* extSPI = new SPIClass(EXT_SPI0_MOSI, EXT_SPI0_MISO, EXT_SPI0_SCK);
#ifdef EXTRA_SPI
SPIClass* extSPI1 = new SPIClass(EXT_SPI1_MOSI, EXT_SPI1_MISO, EXT_SPI1_SCK);
#endif

DriverBase* driver = new DriverSTM32();

uint32_t DriverSTM32::currentClock() const {
    return SystemCoreClock;
}

uint32_t DriverSTM32::currentRam() const {
    extern uint32_t _sdata, _edata, _sbss, _ebss;
    extern uint32_t _Min_Stack_Size, _Min_Heap_Size;

    uint32_t data = &_edata - &_sdata;
    uint32_t bss = &_ebss - &_sbss;
    uint32_t stack = (uint32_t) &_Min_Stack_Size;
    uint32_t heap = (uint32_t) &_Min_Heap_Size;

    return data + bss + stack + heap;
}

uint32_t DriverSTM32::currentFlash() const {
    // extern uint32_t _stext, _etext, _sdata, _edata;
    //
    // uint32_t code = &_etext - &_stext;
    // uint32_t data_init = &_edata - &_sdata;
    //
    // return code + data_init;
    return 0;
}

void DriverSTM32::init() {
    extI2C->begin();
    extSPI->begin();
#ifdef EXTRA_SPI
    extSPI1->begin();
#endif
}

void DriverSTM32::reboot() {
    NVIC_SystemReset();
}

#endif
