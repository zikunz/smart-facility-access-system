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
    PinModeSet(PIN_03, PIN_MODE_0);
    PinModeSet(PIN_05, PIN_MODE_0);
    PinModeSet(PIN_06, PIN_MODE_0);
    PinModeSet(PIN_07, PIN_MODE_0);
    PinModeSet(PIN_08, PIN_MODE_0);
    PinModeSet(PIN_21, PIN_MODE_0);
    PinModeSet(PIN_45, PIN_MODE_0);
    PinModeSet(PIN_50, PIN_MODE_0);
    PinModeSet(PIN_52, PIN_MODE_0);
    PinModeSet(PIN_53, PIN_MODE_0);
    PinModeSet(PIN_58, PIN_MODE_0);
    PinModeSet(PIN_59, PIN_MODE_0);
    PinModeSet(PIN_60, PIN_MODE_0);
    PinModeSet(PIN_61, PIN_MODE_0);
    PinModeSet(PIN_62, PIN_MODE_0);
    PinModeSet(PIN_63, PIN_MODE_0);

    //
    // Enable Peripheral Clocks 
    //
    PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_64 for GPIO Output
    //
    PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_01 for GPIO Output
    //
    PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_02 for GPIO Output
    //
    PinTypeGPIO(PIN_02, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_04 for GPIO Input
    //
    PinTypeGPIO(PIN_04, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, 0x20, GPIO_DIR_MODE_IN);

    //
    // Configure PIN_15 for GPIO Input
    //
    PinTypeGPIO(PIN_15, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA2_BASE, 0x40, GPIO_DIR_MODE_IN);

    //
    // Configure PIN_18 for GPIO Output
    //
    PinTypeGPIO(PIN_18, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA3_BASE, 0x10, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_55 for UART0 UART0_TX
    //
    PinTypeUART(PIN_55, PIN_MODE_3);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    PinTypeUART(PIN_57, PIN_MODE_3);
}
