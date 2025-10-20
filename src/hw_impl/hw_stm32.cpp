#ifdef ARDUINO_ARCH_STM32
#include "hw_impl/hw_stm32.h"
#include "configuration.h"

TwoWire* extI2C = new TwoWire(EXT_I2C_SDA, EXT_I2C_SCL);
SPIClass* extSPI = new SPIClass(EXT_SPI0_MOSI, EXT_SPI0_MISO, EXT_SPI0_SCK);
#ifdef EXTRA_SPI
SPIClass* extSPI1 = new SPIClass(EXT_SPI1_MOSI, EXT_SPI1_MISO, EXT_SPI1_SCK);
#else
SPIClass* extSPI1 = extSPI;
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
    extern uint32_t _etext;
    return (uint32_t)&_etext - FLASH_BASE;
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

#ifdef STM32WB55xx
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /* This prevents concurrent access to RCC registers by CPU2 (M0+) */
    // hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);

    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* This prevents the CPU2 (M0+) to disable the HSI48 oscillator */
    // hsem_lock(CFG_HW_CLK48_CONFIG_SEMID, HSEM_LOCK_DEFAULT_RETRY);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI
                                       | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 32;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4 | RCC_CLOCKTYPE_HCLK2
                                  | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_LPUART1
                                               | RCC_PERIPHCLK_SMPS | RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.PLLSAI1.PLLN = 82;
    PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV8;
    PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
    PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV8;
    PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADCCLK;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
    PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
    PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    LL_PWR_SMPS_SetStartupCurrent(LL_PWR_SMPS_STARTUP_CURRENT_80MA);
    LL_PWR_SMPS_SetOutputVoltageLevel(LL_PWR_SMPS_OUTPUT_VOLTAGE_1V40);
    LL_PWR_SMPS_Enable();

    /* Select HSI as system clock source after Wake Up from Stop mode */
    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

    // hsem_unlock(CFG_HW_RCC_SEMID);
}
#endif

#endif
