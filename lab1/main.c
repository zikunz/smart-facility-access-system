// Application Name: Lab 1
// Application Overview:
// 1. When the program starts, it should display a message on the console window with a header and usage instructions.
// 2. The program should poll the SW3 and SW2 switches on the CC3200 LaunchPad. When SW3 is
// pressed, a binary counting sequence on the LEDs starts (which counts from 000 ¨C 111
// continuously on the three LEDs). A message to the console ¡°SW3 pressed¡± will also be shown. This message
// should not be printed again until after SW2 has been pressed. After 111, it will restart from 000 unless SW2 is pressed.
// When SW2 is pressed, the program should blink the LEDs ON and OFF in unison. Similarly, the program will print
// the message ¡°SW2 pressed¡± to the console. This message should not be printed again until after SW3 has been pressed.
// 3. The output signal P18 is set high whenever SW2 is pressed and low whenever SW3 is pressed. This can be verified
// using an oscilloscope.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup blinky
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "uart.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "pin_mux_config.h"

//*****************************************************************************
//                          MACROS
//*****************************************************************************
#define APPLICATION_VERSION "1.4.0"
#define APP_NAME "GPIO Application"
int MAX_BINARY_SEQUENCE = 7 + 1;

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
void LEDBlinkyRoutine(int);
static void BoardInit(void);

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char *AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t        CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//*****************************************************************************
// This function shows a binary counting sequence on the LEDS from 000 to 111
// Specifically:
// 000 - Green OFF, Orange OFF, Red OFF
// 001 - Green OFF, Orange OFF, Red ON
// 010 - Green OFF, Orange ON, Red OFF
// 011 - Green OFF, Orange ON, Red ON
// 100 - Green ON, Orange OFF, Red OFF
// 101 - Green ON, Orange OFF, Red ON
// 110 - Green ON, Orange ON, Red OFF
// 111 - Green ON, Orange ON, Red ON
void LEDBinaryCounting(int i)
{
    //
    // Toggle the lines initially to turn off the LEDs.
    // The values driven are as required by the LEDs on the LP.
    //
    switch (i)
    {
    case 0:
        // 000 - Green OFF, Orange OFF, Red OFF
        GPIO_IF_LedOff(MCU_ALL_LED_IND);
        MAP_UtilsDelay(3000000);
        break;
        // 001 - Green OFF, Orange OFF, Red ON
    case 1:
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 010 - Green OFF, Orange ON, Red OFF
    case 2:
        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 011 - Green OFF, Orange ON, Red ON
    case 3:
        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 100 - Green ON, Orange OFF, Red OFF
    case 4:
        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 101 - Green ON, Orange OFF, Red ON
    case 5:
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
        GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 110 - Green ON, Orange ON, Red OFF
    case 6:
        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
        // 111 - Green ON, Orange ON, Red ON
    case 7:
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
        GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
        MAP_UtilsDelay(3000000);
        break;
    }
}

//*****************************************************************************
void LEDBlinkUnison()
{
    // Turn on all LEDs first
    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    MAP_UtilsDelay(2000000);

    // Turn off all LEDs
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
    MAP_UtilsDelay(2000000);
}

//*****************************************************************************
//
//! Board Initialization &Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long) &g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*********************************w*******************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the LEDBlinkyTask
//!
//! \return None.
//
//****************************************************************************
int main()
{
    char switchTwoPressingAlert[] = "SW2 pressed";
    char switchThreePressingAlert[] = "SW3 pressed";
    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Enable GPIO for Red LED (output), Yellow LED (output), Green LED (output), SW3 (input), SW2 (input), P18 (output)
    // Mux for enabling UART_TX and UART_RX
    //
    PinMuxConfig();
    GPIO_IF_LedConfigure(LED1 | LED2 | LED3);

    GPIO_IF_LedOff(MCU_ALL_LED_IND);

    //
    // Initializing the Terminal.
    //
    InitTerm();
    //
    // Clearing the Terminal.
    //
    ClearTerm();

    DisplayBanner(APP_NAME);
    Message("\t\t****************************************************\n\r");
    Message("\t\t Push SW3 to start LED binary counting  \n\r");
    Message("\t\t Push SW2 to blink LEDs on and off \n\r");
    Message("\t\t****************************************************\n\r");
    Message("\n\n\n\r");

    int isSwitchTwoPressed = 0; // Flag to keep keep track of whether SW2 has been pressed
    int isSwitchThreePressed = 0; // Flag to keep keep track of whether SW3 has been pressed

    while (1)
    {
        if (GPIOPinRead(GPIOA1_BASE, 0x20) && isSwitchThreePressed == 0) // If SW3 is pressed for the first time or after SW2 is pressed
        {
            Report("\n\r%s", switchThreePressingAlert);
            GPIOPinWrite(GPIOA3_BASE, 0x10, 0); // Write a LOW to P18
            isSwitchThreePressed = 1;
            isSwitchTwoPressed = 0;
            int i, flag = 0;
            while (1)
            {
                for (i = 0; i < MAX_BINARY_SEQUENCE; ++i)
                {
                    LEDBinaryCounting(i);
                    if (GPIOPinRead(GPIOA2_BASE, 0x40))
                    {
                        flag = 1;
                        break;
                    }
                }

                if (flag)
                    break;
            }
        }
        else if (GPIOPinRead(GPIOA2_BASE, 0x40) && isSwitchTwoPressed == 0) // If SW2 is pressed for the first time or after SW3 is pressed
        {
            Report("\n\r%s", switchTwoPressingAlert);
            GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);  //  // Write a HIGH to P18
            isSwitchTwoPressed = 1;
            isSwitchThreePressed = 0;
            while (1)
            {
                LEDBlinkUnison();
                if (GPIOPinRead(GPIOA1_BASE, 0x20))
                {
                    break;
                }
            }
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}

//
//*****************************************************************************
