//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - I2C 
// Application Overview - The objective of this application is act as an I2C 
//                        diagnostic tool. The demo application is a generic 
//                        implementation that allows the user to communicate 
//                        with any I2C device over the lines. 
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup i2c_demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"
#include "gpio.h"
#include "common.h"
#include "socket.h"


// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"

#include "pin_mux_config.h"

#include "PN532_CC3200.h"
#include "time_request.h"
#include "awsconnect.h"
#include "time_request.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "I2C Demo"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
        if (SUCCESS != iRetVal) \
        return  iRetVal;}

#define SPI_IF_BIT_RATE  20000000
#define TR_BUFF_SIZE     100

#define SYSTICK_WRAP_TICKS      16777216 // 2^24

#define SYS_CLK             80000000
#define TICKS_TO_MILLISECONDS(ts)   (((double) 1000/(double) SYS_CLK) * (double) (ts))


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static volatile unsigned long g_ulBase;
static volatile unsigned long g_ulBase1;
long realTime;
int ifTime = 0;
unsigned long g_ticksLast = SYSTICK_WRAP_TICKS;
unsigned long long g_codeCountLast = 0ULL;
unsigned long long g_ticksElapsed = 0UL;
unsigned long long g_codeTicksElapsed = 0UL;
unsigned long long g_ulTimerInts = 0ULL;
volatile unsigned long pin63_intcount;
volatile unsigned char pin63_intflag;
int PASSWORD[4] =  {0,1,7,2};
typedef struct PinSetting
{
    unsigned long port;
    unsigned int pin;
} PinSetting;
static PinSetting pin63 = { .port = GPIOA1_BASE, .pin = 0x1 };
static PinSetting pin15 = { .port = GPIOA2_BASE, .pin = 0x40 };
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************
void SystickInit();
void SystickReset();
unsigned long tickDiff();


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
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



void OLEDPrintString(char * msg, int x, int y, int color, int space, int sizex, int sizey)
{
    int i = 0;
    int sz_msg = strlen(msg);
    for (i = 0; i < sz_msg; ++i)
    {
        drawChar(x, y, msg[i], color, sizex, sizey);
        x += space;
    }
}

void timeDisplay(long iTimeSec)
{

    char date[16];
    char time[16];
    getRealTime(time, date, iTimeSec);
    OLEDPrintString(date, 35, 70, WHITE, 6, 1, 1);
    OLEDPrintString(time, 40, 85, WHITE, 6, 1, 1);
}

void OLEDInit()
{
    char msg1[] = "System starting...";
    OLEDPrintString(msg1, 10, 20, WHITE, 6, 1, 1);
}

void OLEDIdle(int mode)
{
    char msg1[] = "Welcome!";
    OLEDPrintString(msg1, 20, 0, WHITE, 12, 2, 2);

    if (mode == 0)
    {
        char msg2[] = "   Please Scan card   ";
        OLEDPrintString(msg2, 0, 30, WHITE, 6, 1, 1);
    }
    else
    {
        char msg3[] = "Please enter password";
        OLEDPrintString(msg3, 0, 30, WHITE, 6, 1, 1);
    }
    //    char msg3[] = "SW2 to switch mode";
    //    OLEDPrintString(msg3, 10, 45, WHITE, 6, 1, 1);
}

void printHEX(char * UID, char * str, int y){
    int i;
    int x = 0;
    for (i = 0; i<strlen(str);i++)
    {
        char c[64];
        sprintf(c, "%02X", str[i]);
        strcat(UID, c);
        Report("%s ", c);
        OLEDPrintString(c, x, y, WHITE, 6, 0, 1);
        x+= 3*6;
    }

}

void OLEDPrint(char * sendStr, char * cardType, char * UID, int type)
{
    char msg0[] = "SUCCESS!";
    OLEDPrintString(msg0, 20, 0, WHITE, 12, 2, 2);
    if (type == 1)
    {
        char msg1[] = "Card type: ";
        OLEDPrintString(msg1, 0, 25, BLUE, 6, 0, 1);
        OLEDPrintString(cardType, 0, 35, WHITE, 6, 0, 1);
        char msg2[] = "ID: ";
        OLEDPrintString(msg2, 0, 50, BLUE, 6, 0, 1);
        printHEX(sendStr, UID, 60);
    }
}


void MasterMain()
{
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           SPI_IF_BIT_RATE,
                           SPI_MODE_MASTER, SPI_SUB_MODE_0, (SPI_SW_CTRL_CS |
                                   SPI_4PIN_MODE |
                                   SPI_TURBO_OFF |
                                   SPI_CS_ACTIVEHIGH |
                                   SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    //
    // Initialize Adafruit
    Adafruit_Init();
}



void TimerBaseIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);

    realTime += 1;


}
void SystickInit()
{
    SysTickPeriodSet(SYSTICK_WRAP_TICKS);
    SysTickIntRegister(SystickReset);
    SysTickEnable();
}

void SystickReset()
{
    SysTickPeriodSet(SYSTICK_WRAP_TICKS);
    SysTickEnable();
}

unsigned long edgeTickDiff()
{
    // Note that systick counts down, so lesser tick values
    // represent a succeeding point in time

    unsigned long ticksCurrent = SysTickValueGet();
    unsigned long ticksLast = g_ticksLast;

    // Set up for next call
    g_ticksLast = ticksCurrent;

    // No wrap; most probable case, so we short circuit a branch here
    if (ticksCurrent < ticksLast)
        return ticksLast - ticksCurrent;

    // Wrapped ticks. We assume a max wrap of SYSTICK_MAX_TICKS.
    // The order of operations here is important to prevent arithmetic overflow.
    return (SYSTICK_WRAP_TICKS - ticksCurrent) + ticksLast;
}

unsigned long codeCountDiff()
{
    // Note that systick counts down, so lesser tick values
    // represent a succeeding point in time

    unsigned long countCurrent = g_ulTimerInts;
    unsigned long countLast = g_codeCountLast;

    // Set up for next call
    g_codeCountLast = countCurrent;

    // No wrap; most probable case, so we short circuit a branch here
    if (countCurrent > countLast)
        return countCurrent - countLast;

    // Wrapped ticks. We assume a max wrap of SYSTICK_MAX_TICKS.
    // The order of operations here is important to prevent arithmetic overflow.
    //return (SYSTICK_WRAP_TICKS - countCurrent) + countLast;
}



int inputMode = 0;

static void GPIOA0IntHandler(void)
{ // pin63 handler
    unsigned long ulStatus;
    int lRetVal;

    Timer_IF_Stop(g_ulBase, TIMER_A);

    ulStatus = MAP_GPIOIntStatus(pin15.port, true);
    MAP_GPIOIntClear(pin15.port, ulStatus);        // clear interrupts on GPIOA1
    inputMode = 1;

}


//void buttonDown()
//{
//    unsigned long ulStatus;
//
//    ulStatus = MAP_GPIOIntStatus(pin15.port, true);
//    MAP_GPIOIntClear(pin15.port, ulStatus);        // clear interrupts on GPIOA1
//
//    if (inputMode == 0)
//        inputMode = 1;
//    else
//        inputMode = 0;
//}

void main()
{
    int lRetVal = 0;
    unsigned long ulStatus;
    int readState = 0;
    char UID[8];
    int cardType;
    char sendStr[256] = "";
    //
    // Initialize board configurations
    //
    BoardInit();

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
    InitTerm();
    ClearTerm();
    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    //
    // Register the interrupt handlers
    //
    MAP_GPIOIntRegister(pin15.port, GPIOA0IntHandler);

    //
    // Configure rising edge interrupts on pin63
    //
    MAP_GPIOIntTypeSet(pin15.port, pin15.pin, GPIO_RISING_EDGE); // pin63

    ulStatus = MAP_GPIOIntStatus(pin15.port, false);
    MAP_GPIOIntClear(pin15.port, ulStatus); // clear interrupts on GPIOA1



    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MasterMain();
    fillScreen(BLACK);
    OLEDInit();

    Report("---Firmware check... ---\r\n");

    char msg1[] = "PN532...";
    OLEDPrintString(msg1, 10, 35, WHITE, 6, 1, 1);
    if(getFirmwareVersion())
    {
        char msg[] = "PN532...  OK";
        OLEDPrintString(msg, 10, 35, WHITE, 6, 1, 1);
        Report("PN532 detected!\r\n");
    }
    else
    {
        char msg[] = "PN532...  FAIL";
        OLEDPrintString(msg, 10, 35, WHITE, 6, 1, 1);
        Report("No PN532\r\n");
    }
    //
    // Base address for first timer
    //
    g_ulBase = TIMERA0_BASE;


    //
    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_PERIODIC, TIMER_A, 0);


    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerBaseIntHandler);




    SAMConfig();

    char msg2[] = "AP...";
    OLEDPrintString(msg2, 10, 50, WHITE, 6, 1, 1);
    //Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();
    //Set time so that encryption can be used
    lRetVal = set_time();

    if(lRetVal < 0) {
        char msg[] = "AP...  FAIL";
        OLEDPrintString(msg, 10, 50, WHITE, 6, 1, 1);
        Report("PN532 detected!\r\n");
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }
    else
    {
        char msg[] = "AP...  OK";
        OLEDPrintString(msg, 10, 50, WHITE, 6, 1, 1);
        Report("PN532 detected!\r\n");
    }
    //Connect to the website with TLS encryption
    char msg3[] = "AWS...";
    OLEDPrintString(msg3, 10, 65, WHITE, 6, 1, 1);
    lRetVal = tls_connect();


    realTime = httpGetTime(lRetVal);
    sl_Close(lRetVal);
    if(lRetVal < 0)
    {
        char msg[] = "AWS...  FAIL";
        OLEDPrintString(msg, 10, 65, WHITE, 6, 1, 1);
        Report("PN532 detected!\r\n");
    }
    else
    {
        char msg[] = "AWS...  OK";
        OLEDPrintString(msg, 10, 65, WHITE, 6, 1, 1);
        Report("PN532 detected!\r\n");
    }
    fillScreen(BLACK);
    OLEDIdle(inputMode);
    //
    // Turn on the timers feeding values in mSec
    //
    Timer_IF_Start(g_ulBase, TIMER_A, 1000);

    MAP_GPIOIntEnable(pin15.port, pin15.pin);
    SystickInit();
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);
    while(FOREVER)
    {
        memset(sendStr, 0 ,strlen(sendStr));
        memset(UID, 0 ,strlen(UID));

        if(inputMode == 0)
        {
            GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);
            MAP_GPIOIntDisable(pin63.port, pin63.pin);
            cardType = readPassiveTargetID(UID);
            if (cardType != 0)
            {
                GPIOPinWrite(GPIOA0_BASE, 0x80, 0x0);
                Timer_IF_Stop(g_ulBase, TIMER_A);
                ifTime = 1;

                if (readState == 0)
                {
                    fillScreen(BLACK);
                    readState = 1;
                }

                if (cardType == 1)
                {
                    char str[] = "Mifare Classic";
                    Report("%s, UID: ", str);
                    OLEDPrint(sendStr, str, UID, 1);

                }
                if  (cardType == 2)
                {
                    char str[] = "Mifare Ultralight";
                    Report("%s, UID: ", str);
                    OLEDPrint(sendStr, str, UID, 1);
                }

                Report("\r\n");
                lRetVal = tls_connect();
                http_post(lRetVal, sendStr, cardType);
                sl_Close(lRetVal);

            }
            else
            {
                if(readState)
                {
                    MAP_UtilsDelay(32000000);
                    fillScreen(BLACK);
                    readState = 0;
                    Timer_IF_Start(g_ulBase, TIMER_A, 1000);
                    realTime += 5;
                }

                ifTime = 0;



                timeDisplay(realTime);
                OLEDIdle(inputMode);
            }
        }
        else
        {
            fillScreen(BLACK);

            char UID[16] = "\x00\x00";
            char str[] = "Password Entry";
            Report("%s, UID: ", str);
            OLEDPrint("", str, UID, 1);
            char sendStr[] = "0000";
            lRetVal = tls_connect();
            http_post(lRetVal, sendStr, 0);
            sl_Close(lRetVal);
            MAP_UtilsDelay(32000000);
            fillScreen(BLACK);
            realTime += 5;
            Timer_IF_Start(g_ulBase, TIMER_A, 1000);
            inputMode = 0;
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************


