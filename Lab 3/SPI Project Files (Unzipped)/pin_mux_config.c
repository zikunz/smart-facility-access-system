#include "pin_mux_config.h" 
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"

//*****************************************************************************
void PinMuxConfig(void)
{
    //
    // Set unused pins to PIN_MODE_0 with the exception of JTAG pins 16,17,19,20
    //
    PinModeSet(PIN_01, PIN_MODE_0);
    PinModeSet(PIN_02, PIN_MODE_0);
    PinModeSet(PIN_03, PIN_MODE_0);
    PinModeSet(PIN_04, PIN_MODE_0);
    PinModeSet(PIN_06, PIN_MODE_0);
    PinModeSet(PIN_08, PIN_MODE_0);
    PinModeSet(PIN_15, PIN_MODE_0);
    PinModeSet(PIN_21, PIN_MODE_0);
    PinModeSet(PIN_45, PIN_MODE_0);
    PinModeSet(PIN_52, PIN_MODE_0);
    PinModeSet(PIN_53, PIN_MODE_0);
    PinModeSet(PIN_55, PIN_MODE_0);
    PinModeSet(PIN_57, PIN_MODE_0);
    PinModeSet(PIN_58, PIN_MODE_0);
    PinModeSet(PIN_59, PIN_MODE_0);
    PinModeSet(PIN_60, PIN_MODE_0);
    PinModeSet(PIN_63, PIN_MODE_0);
    PinModeSet(PIN_64, PIN_MODE_0);

    //
    // Enable Peripheral Clocks 
    //
    PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_61 for GPIO Output, CS
    //
    PinTypeGPIO(PIN_61, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, 0x40, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_62 for GPIO Output, DC
    //
    PinTypeGPIO(PIN_62, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, 0x80, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_18 for GPIO Output, RESET (R)
    //
    PinTypeGPIO(PIN_18, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA3_BASE, 0x10, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_50 for SPI0 GSPI_CS
    //
    PinTypeSPI(PIN_50, PIN_MODE_9);

    //
    // Configure PIN_05 for SPI0 GSPI_CLK
    //
    //PinTypeSPI(PIN_05, PIN_MODE_7);

    //
    // Configure PIN_07 for SPI0 GSPI_MOSI
    //
    PinTypeSPI(PIN_07, PIN_MODE_7);
}
