#include "hardware.h"

/*****************/
/* Bus variables */
/*****************/

/**** RP2040 ****/
#if defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040* extSPI =  &SPI;
SPIClassRP2040* extSPI1 = &SPI1;
TwoWire*  extI2C =  &Wire1; // Wire1 because of the small boards

/**** STM32 ****/
#elif defined(ARDUINO_ARCH_STM32)
SPIClass* extSPI =  new SPIClass(EXT_SPI0_MOSI, EXT_SPI0_MISO, EXT_SPI0_SCK);;
TwoWire*  extI2C =  new TwoWire(EXT_I2C_SDA, EXT_I2C_SCL);
#ifdef EXTRA_SPI
SPIClass* extSPI1 = new SPIClass(EXT_SPI1_MOSI, EXT_SPI1_MISO, EXT_SPI1_SCK);
#else
SPIClass* extSPI1 = extSPI;
#endif

#endif


/********************/
/* Loading hardware */
/********************/
void initHardware() {
#if defined(ARDUINO_ARCH_RP2040)
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
#elif defined(ARDUINO_ARCH_STM32)
    extI2C->begin();
    extSPI->begin();
#ifdef EXTRA_SPI
    extSPI1->begin();
#endif
#endif
}