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
#include "timer.h"
#include "systick.h"


// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"

#include "pin_mux_config.h"

#include "PN532_CC3200.h"
#include "awsconnect.h"


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

char psw[16];
char PASSWORD[16] =  "0172";


unsigned int currentColor = WHITE;
long realTime;
char g_lastInt;
int g_inputMode;

volatile unsigned long pin63_intcount;
volatile unsigned char pin63_intflag;
volatile unsigned long temp;
unsigned long buffer[1000]; // buffer to store IR Data

static volatile unsigned long g_ulBase;

char gpioTimeCount;
char gpioArr[13] = "";

typedef struct PinSetting
{
    unsigned long port;
    unsigned int pin;
} PinSetting;
static PinSetting pin63 = { .port = GPIOA1_BASE, .pin = 0x1 };
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

int x = 0;
int y = 120;
int colorCounter = 0;

char decodeDEC()
{
    int code_DEC = 0;
    char *ptr;
    ptr = strchr(gpioArr, NULL);
    //char *token = strtok(gpioArr, NULL);
    //token = strtok(NULL, 0x00);
    //    if (strlen(token) == 14)
    //    {

    code_DEC = strtol(ptr + 1, NULL, 3);

    switch (code_DEC)
    {
    case 177148:
        Report("%d   %s      1 \r\n", strlen(gpioArr), gpioArr);
        return '1';                 //177148 - 59050
    case 177150:
        Report("%d   %s      3 \r\n", strlen(gpioArr), gpioArr);
        return '3';                //177150 - 59052
    case 177156:
        Report("%d   %s      7 \r\n", strlen(gpioArr), gpioArr);
        return '7';                 //177156 - 59058
    case 177147:
        Report("%d   %s      0 \r\n", strlen(gpioArr), gpioArr);
        return '0';                 //177147 - 59049

    case 59050:
        Report("%d   %s      1F \r\n", strlen(gpioArr), gpioArr);
        return '1';                 //177148 - 59050
    case 59052:
        Report("%d   %s      3F \r\n", strlen(gpioArr), gpioArr);
        return '3';               //177150 - 59052
    case 59058:
        Report("%d   %s      7F \r\n", strlen(gpioArr), gpioArr);
        return '7';                 //177156 - 59058
    case 59049:
        Report("%d   %s      0F \r\n", strlen(gpioArr), gpioArr);
        return '0';                 //177147 - 59049
        //        }
        //    }
        //    else if (strlen(gpioArr) == 11)
        //    {
        //        code_DEC = strtol(gpioArr, NULL, 3);

        //        switch (code_DEC)
        //        {
    case 59051:
        Report("%d   %s      2 \r\n", strlen(gpioArr), gpioArr);
        return '2';                  //59051 - 19685
    case 59055:
        Report("%d   %s      4 \r\n", strlen(gpioArr), gpioArr);
        return '4';                //59055 - 19689
    case 59056:
        Report("%d   %s      5 \r\n", strlen(gpioArr), gpioArr);
        return '5';                //59056 - 19690
    case 59053:
        Report("%d   %s      6 \r\n", strlen(gpioArr), gpioArr);
        return '6';                //59053 - 19687
    case 59067:
        Report("%d   %s      8 \r\n", strlen(gpioArr), gpioArr);
        return '8';                  //59067 - 19701
    case 59068:
        Report("%d   %s      9 \r\n", strlen(gpioArr), gpioArr);
        return '9';                  //59068 - 19702
    case 59062:
        Report("%d   %s      ENTER \r\n", strlen(gpioArr), gpioArr);
        return 'a';              //59062 - 19696

    case 19685:
        Report("%d   %s      2F \r\n", strlen(gpioArr), gpioArr);
        return '2';                  //59051 - 19685
    case 19689:
        Report("%d   %s      4F \r\n", strlen(gpioArr), gpioArr);
        return '4';                //59055 - 19689
    case 19690:
        Report("%d   %s      5F \r\n", strlen(gpioArr), gpioArr);
        return '5';                //59056 - 19690
    case 19687:
        Report("%d   %s      6F \r\n", strlen(gpioArr), gpioArr);
        return '6';                //59053 - 19687
    case 19701:
        Report("%d   %s      8F \r\n", strlen(gpioArr), gpioArr);
        return '8';                  //59067 - 19701
    case 19702:
        Report("%d   %s      9F \r\n", strlen(gpioArr), gpioArr);
        return '9';                  //59068 - 19702
    case 19696:
        Report("%d   %s      ENTER F \r\n", strlen(gpioArr), gpioArr);
        return 'a';              //59062 - 19696
        //        }
        //    }
        //    else if (strlen(gpioArr) == 10)
        //    {
        //        code_DEC = strtol(gpioArr, NULL, 3);
        //        switch (code_DEC)
        //        {
    case 19739:
        Report("%d   %s      DELETE \r\n", strlen(gpioArr), gpioArr);
        return 'b';             //19739 - 6617
    case 6617:
        Report("%d   %s      DELETE F \r\n", strlen(gpioArr), gpioArr);
        return 'b';             //19739 - 6617
    }

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
    OLEDPrintString(date, 35, 60, WHITE, 6, 1, 1);
    OLEDPrintString(time, 40, 75, WHITE, 6, 1, 1);
    OLEDPrintString(psw, 20, 100, WHITE, 15, 2, 2);
}

void OLEDInit()
{
    char msg1[] = "System starting...";
    OLEDPrintString(msg1, 10, 80, WHITE, 6, 1, 1);
}

void OLEDIdle(int mode)
{
    char msg1[] = "Welcome!";
    OLEDPrintString(msg1, 20, 0, WHITE, 12, 2, 2);
    char msg3[] = "Please enter password";
    OLEDPrintString(msg3, 0, 30, WHITE, 6, 1, 1);

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

    timeDisplay(realTime);
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

    ulStatus = MAP_GPIOIntStatus(pin63.port, true);
    MAP_GPIOIntClear(pin63.port, ulStatus);        // clear interrupts on GPIOA1
    pin63_intcount++;
    pin63_intflag = 1;

    //    if (g_ulTimerInts < 10)
    //    {
    //        gpioTimeCount = 0 + '0';
    //
    //    }
    //    else {
    //        gpioTimeCount = 1 + '0';
    //    }
    //    strncat(gpioArr, &gpioTimeCount, 1);
    g_ticksElapsed = edgeTickDiff();
    //g_ulTimerInts = 0;
}

int pArr = 0;
void DebugRoutine()
{
    int i = 0;
    char result;
    int delta = 0;
    //   Report("hello");
    while (i < 13)
    {

        if (g_ticksElapsed)
        {
            if (g_ticksElapsed >= 2000000UL)
            {

                if (i < 10)
                {
                    i = 0;
                    strcpy(gpioArr, "");
                }
                else
                {
                    gpioArr[i] = NULL;
                }
                //gpioArr[i] = '#';
            }
            else if (g_ticksElapsed > 240000UL)
                gpioArr[i] = '2';
            else if (g_ticksElapsed >180000UL)
                gpioArr[i] = '1';
            else if (g_ticksElapsed > 120000UL)
                gpioArr[i] = '0';
            ++i;
            g_ticksElapsed = 0UL;
        }
    }

    //get elapse
    result = decodeDEC();

    if (result < '0' || result > 'b')
        return;
    else
    {
        delta = codeCountDiff();
        Report("                                   Elapse %lu \r\n", delta);
        MAP_UtilsDelay(4000000);
    }
//    if (result != g_lastInt)
//    {
        g_inputMode = 0;
        g_lastInt = result;
        psw[pArr] = result;
        pArr ++;
        if (pArr >= 4)
        {
            if (strcmp(psw,PASSWORD) == 0)
            {
                GPIOPinWrite(GPIOA2_BASE, 0x40,0x40);
                MAP_UtilsDelay(800000);
                GPIOPinWrite(GPIOA2_BASE, 0x40,0x00);
            }
            strcpy(psw,"    ");
            pArr = 0;
        }
        return;
//    }
//    else if (delta < 450 && delta >= 200)        //long press
//    {
//        g_inputMode = 0;
//        return;
//    }
//    //    else if (delta >= 450 && delta < 1200)      //change char
//    //    {
//    //        g_inputMode += 1;
//    //        deleteChar();
//    //        printChar(g_lastInt, g_inputMode);
//    //        g_lastInt = result;
//    //        return;
//    //    }
//    else if (delta >= 450)                     //no input
//    {
//        g_inputMode = 0;
//        printChar(result, g_inputMode);
//        g_lastInt = result;
//        resul
//        return;
//    }


}
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
    MAP_GPIOIntRegister(pin63.port, GPIOA0IntHandler);

    //
    // Configure rising edge interrupts on pin63
    //
    MAP_GPIOIntTypeSet(pin63.port, pin63.pin, GPIO_RISING_EDGE); // pin63

    ulStatus = MAP_GPIOIntStatus(pin63.port, false);
    MAP_GPIOIntClear(pin63.port, ulStatus); // clear interrupts on GPIOA1

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
    if(getFirmwareVersion())
        Report("PN532 detected!\r\n");
    else
        Report("No PN532\r\n");

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


    //Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();
    //Set time so that encryption can be used
    lRetVal = set_time();
    if(lRetVal < 0) {
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }
    //Connect to the website with TLS encryption

    lRetVal = tls_connect();
    realTime = httpGetTime(lRetVal);
    sl_Close(lRetVal);
    fillScreen(BLACK);
    OLEDIdle(inputMode);
    //
    // Turn on the timers feeding values in mSec
    //
    Timer_IF_Start(g_ulBase, TIMER_A, 1000);

    MAP_GPIOIntEnable(pin63.port, pin63.pin);
    SystickInit();
    while(FOREVER)
    {
        DebugRoutine();



    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************


