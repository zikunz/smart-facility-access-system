// Lab 4
// Students: Ang Li, Zikun Zhu

#include <stdbool.h>
#define CONSOLE                     UARTA0_BASE
#define PAIRDEV                     UARTA1_BASE
#define PAIRDEV_PERIPH          PRCM_UARTA1

#define UartConsoleGetChar()        MAP_UARTCharGet(CONSOLE)
#define UartConsolePutChar(c)       MAP_UARTCharPut(CONSOLE,c)
#define UartPairDevGetChar()        MAP_UARTCharGet(PAIRDEV)
#define UartPairDevPutChar(c)       MAP_UARTCharPut(PAIRDEV,c)

bool rec_new_message = 0;

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"
#include "uart.h"
#include "interrupt.h"
#include "systick.h"
#include "timer.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "timer_if.h"
#include "pin_mux_config.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "test.h"

#define SPI_IF_BIT_RATE  100000

extern void (* const g_pfnVectors[])(void);

volatile unsigned long pin63_intcount;
volatile unsigned char pin63_intflag;
volatile unsigned long temp;
unsigned long buffer[1000]; // buffer to store IR Data

static volatile unsigned long g_ulBase;

char gpioTimeCount;
char gpioArr[13] = "";

void SystickInit();
void SystickReset();
unsigned long tickDiff();

#define SYSTICK_WRAP_TICKS      16777216 // 2^24

#define SYS_CLK             80000000
#define TICKS_TO_MILLISECONDS(ts)   (((double) 1000/(double) SYS_CLK) * (double) (ts))

#define NO_PROCESSING_MODE  0
#define SHORT_PROCESSING_MODE   1
#define LONG_PROCESSING_MODE    2

#define EDGE_TYPE_FALLING   0
#define EDGE_TYPE_RISING    1

unsigned int g_signalMode = NO_PROCESSING_MODE;
unsigned int g_finishMode = NO_PROCESSING_MODE;
unsigned long g_ticksLast = SYSTICK_WRAP_TICKS;
unsigned long long g_codeCountLast = 0ULL;
unsigned long long g_ticksElapsed = 0UL;
unsigned long long g_codeTicksElapsed = 0UL;
unsigned long long g_ulTimerInts = 0ULL;
unsigned int currentColor = WHITE;
int g_lastInt;
int g_inputMode;

typedef struct PinSetting
{
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting pin63 = { .port = GPIOA1_BASE, .pin = 0x1 };

static void BoardInit(void);

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

void TimerBaseIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);

    g_ulTimerInts++;

}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************

static void BoardInit(void)
{
    MAP_IntVTableBaseSet((unsigned long) &g_pfnVectors[0]);

    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//***************************************************************************
//
//! Main function
//!
//! \param none
//!
//!
//! \return None.
//
//****************************************************************************
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

char charMessage[20];
int colorMessage[20];
int currentMessageIndex = 0;

int decodeDEC()
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
        return 1;                 //177148 - 59050
    case 177150:
        Report("%d   %s      3 \r\n", strlen(gpioArr), gpioArr);
        return 3;                //177150 - 59052
    case 177156:
        Report("%d   %s      7 \r\n", strlen(gpioArr), gpioArr);
        return 7;                 //177156 - 59058
    case 177147:
        Report("%d   %s      0 \r\n", strlen(gpioArr), gpioArr);
        return 0;                 //177147 - 59049

    case 59050:
        Report("%d   %s      1F \r\n", strlen(gpioArr), gpioArr);
        return 1;                 //177148 - 59050
    case 59052:
        Report("%d   %s      3F \r\n", strlen(gpioArr), gpioArr);
        return 3;               //177150 - 59052
    case 59058:
        Report("%d   %s      7F \r\n", strlen(gpioArr), gpioArr);
        return 7;                 //177156 - 59058
    case 59049:
        Report("%d   %s      0F \r\n", strlen(gpioArr), gpioArr);
        return 0;                 //177147 - 59049
        //        }
        //    }
        //    else if (strlen(gpioArr) == 11)
        //    {
        //        code_DEC = strtol(gpioArr, NULL, 3);

        //        switch (code_DEC)
        //        {
    case 59051:
        Report("%d   %s      2 \r\n", strlen(gpioArr), gpioArr);
        return 2;                  //59051 - 19685
    case 59055:
        Report("%d   %s      4 \r\n", strlen(gpioArr), gpioArr);
        return 4;                //59055 - 19689
    case 59056:
        Report("%d   %s      5 \r\n", strlen(gpioArr), gpioArr);
        return 5;                //59056 - 19690
    case 59053:
        Report("%d   %s      6 \r\n", strlen(gpioArr), gpioArr);
        return 6;                //59053 - 19687
    case 59067:
        Report("%d   %s      8 \r\n", strlen(gpioArr), gpioArr);
        return 8;                  //59067 - 19701
    case 59068:
        Report("%d   %s      9 \r\n", strlen(gpioArr), gpioArr);
        return 9;                  //59068 - 19702
    case 59062:
        Report("%d   %s      ENTER \r\n", strlen(gpioArr), gpioArr);
        return 10;              //59062 - 19696

    case 19685:
        Report("%d   %s      2F \r\n", strlen(gpioArr), gpioArr);
        return 2;                  //59051 - 19685
    case 19689:
        Report("%d   %s      4F \r\n", strlen(gpioArr), gpioArr);
        return 4;                //59055 - 19689
    case 19690:
        Report("%d   %s      5F \r\n", strlen(gpioArr), gpioArr);
        return 5;                //59056 - 19690
    case 19687:
        Report("%d   %s      6F \r\n", strlen(gpioArr), gpioArr);
        return 6;                //59053 - 19687
    case 19701:
        Report("%d   %s      8F \r\n", strlen(gpioArr), gpioArr);
        return 8;                  //59067 - 19701
    case 19702:
        Report("%d   %s      9F \r\n", strlen(gpioArr), gpioArr);
        return 9;                  //59068 - 19702
    case 19696:
        Report("%d   %s      ENTER F \r\n", strlen(gpioArr), gpioArr);
        return 10;              //59062 - 19696
        //        }
        //    }
        //    else if (strlen(gpioArr) == 10)
        //    {
        //        code_DEC = strtol(gpioArr, NULL, 3);
        //        switch (code_DEC)
        //        {
    case 19739:
        Report("%d   %s      DELETE \r\n", strlen(gpioArr), gpioArr);
        return 11;             //19739 - 6617
    case 6617:
        Report("%d   %s      DELETE F \r\n", strlen(gpioArr), gpioArr);
        return 11;             //19739 - 6617
    }

}

int x = 0;
int y = 120;
int colorCounter = 0;

void changeColor()
{
    switch (colorCounter)
    {
    case 0:
        currentColor = WHITE;
        break;
    case 1:
        currentColor = GREEN;
        break;
    case 2:
        currentColor = CYAN;
        break;
    case 3:
        currentColor = RED;
        break;
    case 4:
        currentColor = MAGENTA;
        break;
    }
    colorCounter++;
    if (colorCounter > 4)
    {
        colorCounter = 0;
    }
}

void deleteChar()
{
    currentMessageIndex -= 1;
    if (y == 120 && x == 0) // Line 1, at the start
    {
        fillRect(x, y, 5, 7, BLACK);
    }
    else
    { // Any line, not at the start
        x -= 6; // Move back "cursor"
        fillRect(x, y, 5, 7, BLACK); // Delete the current char

    }
}

int my = 0;

void SendMsg()
{
    int i = 0;

for (i = 0; i < currentMessageIndex; ++i)
{
    UartPairDevPutChar(charMessage[i]);
    switch (colorMessage[i])
    {
    case WHITE:
        UartPairDevPutChar('1');
        break;
    case GREEN:
        UartPairDevPutChar('2');
        break;
    case CYAN:
        UartPairDevPutChar('3');
        break;
    case RED:
        UartPairDevPutChar('4');
        break;
    case MAGENTA:
        UartPairDevPutChar('5');
        break;
    }

    }
    UartPairDevPutChar('%');
    UartPairDevPutChar('%');

//    if (my < 110)
//        my += 10;
//    else
//    {
//        fillScreen(BLACK);
//        drawFastHLine(0, 110, 120, WHITE);
//    }

    currentMessageIndex = 0;
//    int i = 0;
//    for(i = 0; i<strlen(msg); i++)
//    {
//        drawChar(x, msg_y, msg[i], currentColor, 0, 1);
//        x += 6;
//    }
//    x = 0;
//
    char arr1[] = { 'M', 'e', 's', 's', 'a', 'g', 'e' };
    char arr2[] = { 's', 'e', 'n', 't' };
    char arr3[] = { 's', 'u', 'c', 'c', 'e', 's', 's', 'f', 'u', 'l', 'l', 'y',
                    ' ', ':', ')' };

    size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
    size_t n2 = sizeof(arr2) / sizeof(arr2[0]);
    size_t n3 = sizeof(arr3) / sizeof(arr3[0]);
    int j = 0;
    int x1 = 37;
    for (j = 0; j < n1; ++j)
    {
        drawChar(x1, 0, arr1[j], WHITE, 0, 1);
        MAP_UtilsDelay(5000000);
        x1 += 6;
    }

    int x2 = 48;
    for (j = 0; j < n2; ++j)
    {
        drawChar(x2, 20, arr2[j], WHITE, 0, 1);
        MAP_UtilsDelay(5000000);
        x2 += 6;
    }

    int x3 = 13;
    for (j = 0; j < n3; ++j)
    {
        drawChar(x3, 40, arr3[j], WHITE, 0, 1);
        MAP_UtilsDelay(5000000);
        x3 += 6;
    }
    fillScreen(BLACK);
    drawFastHLine(0, 110, 120, WHITE);
}

void ClearInput()
{
    fillRect(0, 120, 120, 7, BLACK);
    x = 0;
    y = 120;
}

int letterMode = 0;

void addChar(char c)
{
    charMessage[currentMessageIndex] = c;
    colorMessage[currentMessageIndex] = currentColor;
    currentMessageIndex++;
}

void printChar(int result, int mode)
{
    if (result == 1 && mode == 0)
    {
        //        delta = codeCountDiff();
        changeColor();
        return;
    }
    else if (result == 10 && mode == 0)
    {
        SendMsg();
        ClearInput();
        return;
    }
    else if (result == 11 && mode == 0)           //delete
    {
        deleteChar();

        // Uncomment below to use the LAST button to switch between letter and number mode
        /*if (letterMode == 1)
        {
            letterMode = 0;
        }
        else
        {
            letterMode = 1;
        }*/
        
        return;
    }
    else if (result != 7 && result != 9 && letterMode == 1)
    {
        switch (result)
        {
        case 0:
            drawChar(x, y, '0', currentColor, 0, 1);
            addChar('0');
            break;
        case 1:
            drawChar(x, y, '1', currentColor, 0, 1);
            addChar('1');
            break;
        case 2:
            drawChar(x, y, '2', currentColor, 0, 1);
            addChar('2');
            break;
        case 3:
            drawChar(x, y, '3', currentColor, 0, 1);
            addChar('3');
            break;
        case 4:
            drawChar(x, y, '4', currentColor, 0, 1);
            addChar('4');
            break;
        case 5:
            drawChar(x, y, '5', currentColor, 0, 1);
            addChar('5');
            break;
        case 6:
            drawChar(x, y, '6', currentColor, 0, 1);
            addChar('6');
            break;
        case 7:
            drawChar(x, y, '7', currentColor, 0, 1);
            addChar('7');
            break;
        case 8:
            drawChar(x, y, '8', currentColor, 0, 1);
            addChar('8');
            break;
        case 9:
            drawChar(x, y, '9', currentColor, 0, 1);
            addChar('9');
            break;
        }
    }
    else if (result != 7 && result != 9 && letterMode == 0)
    {
        if (mode % 3 == 0)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 2:
                drawChar(x, y, 'A', currentColor, 0, 1);
                addChar('A');
                break;
            case 3:
                drawChar(x, y, 'D', currentColor, 0, 1);
                addChar('D');
                break;
            case 4:
                drawChar(x, y, 'G', currentColor, 0, 1);
                addChar('G');
                break;
            case 5:
                drawChar(x, y, 'J', currentColor, 0, 1);
                addChar('J');
                break;
            case 6:
                drawChar(x, y, 'M', currentColor, 0, 1);
                addChar('M');
                break;
            case 8:
                drawChar(x, y, 'T', currentColor, 0, 1);
                addChar('T');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
        else if (mode % 3 == 1)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 2:
                drawChar(x, y, 'B', currentColor, 0, 1);
                addChar('B');
                break;
            case 3:
                drawChar(x, y, 'E', currentColor, 0, 1);
                addChar('E');
                break;
            case 4:
                drawChar(x, y, 'H', currentColor, 0, 1);
                addChar('H');
                break;
            case 5:
                drawChar(x, y, 'K', currentColor, 0, 1);
                addChar('K');
                break;
            case 6:
                drawChar(x, y, 'N', currentColor, 0, 1);
                addChar('N');
                break;
            case 8:
                drawChar(x, y, 'U', currentColor, 0, 1);
                addChar('U');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
        else if (mode % 3 == 2)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 2:
                drawChar(x, y, 'C', currentColor, 0, 1);
                addChar('C');
                break;
            case 3:
                drawChar(x, y, 'F', currentColor, 0, 1);
                addChar('F');
                break;
            case 4:
                drawChar(x, y, 'I', currentColor, 0, 1);
                addChar('I');
                break;
            case 5:
                drawChar(x, y, 'L', currentColor, 0, 1);
                addChar('L');
                break;
            case 6:
                drawChar(x, y, 'O', currentColor, 0, 1);
                addChar('O');
                break;
            case 8:
                drawChar(x, y, 'V', currentColor, 0, 1);
                addChar('V');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
    }
    else if (result == 7 || result == 9 && letterMode == 1)
    {
        if (result == 7)
        {
            drawChar(x, y, '7', currentColor, 0, 1);
            addChar('7');
        }
        else if (result == 9)
        {
            drawChar(x, y, '9', currentColor, 0, 1);
            addChar('9');
        }
    }
    else if (result == 7 || result == 9 && letterMode == 0)
    {
        if (mode % 4 == 0)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;

            case 7:
                drawChar(x, y, 'P', currentColor, 0, 1);
                addChar('P');
                break;
            case 9:
                drawChar(x, y, 'W', currentColor, 0, 1);
                addChar('W');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
        else if (mode % 4 == 1)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 7:
                drawChar(x, y, 'Q', currentColor, 0, 1);
                addChar('Q');
                break;
            case 9:
                drawChar(x, y, 'X', currentColor, 0, 1);
                addChar('X');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
        else if (mode % 4 == 2)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 7:
                drawChar(x, y, 'R', currentColor, 0, 1);
                addChar('R');
                break;
            case 9:
                drawChar(x, y, 'Y', currentColor, 0, 1);
                addChar('Y');
                break;

            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
        else if (mode % 4 == 3)
        {
            switch (result)
            {

            //        case 1: drawChar(x, y, '1', currentColor, 0, 1); break;
            case 7:
                drawChar(x, y, 'S', currentColor, 0, 1);
                addChar('S');
                break;
            case 9:
                drawChar(x, y, 'Z', currentColor, 0, 1);
                addChar('Z');
                break;
            case 0:
                addChar(' ');
                break;
            default:
                x -= 6;
            }
        }
    }
    ////////////////////print char//////////////////////////////////
    x += 6;
    if (x >= 120)
    { // Go to the next line if current line cannot print more characters
        x = 114;
    }
    //    x += 6;
    //    if (x >= 120)
    //    { // Go to the next line if current line cannot print more characters
    //        xPrevious = x;
    //        x = 0;
    //        y += 10;
    //    }
    //    if (y >= 120)
    //    { // Clear the screen and start again
    //        fillScreen(BLACK);
    //        x = 0;
    //        y = 0;
    //    }
}

void DebugRoutine()
{
    int i = 0;
    int result = 0;
    int delta = 0;
    //   Report("hello");
    while (i < 13)
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
            else if (g_ticksElapsed > 180000UL)
                gpioArr[i] = '1';
            else if (g_ticksElapsed > 120000UL)
                gpioArr[i] = '0';
            ++i;
            g_ticksElapsed = 0UL;
        }

    //get elapse
    result = decodeDEC();
    strcpy(gpioArr, "");
    if (result < 0 || result > 11)
        return;
    else
    {
        delta = codeCountDiff();
        Report("                                   Elapse %lu \r\n", delta);
    }
    if (result != g_lastInt)
    {
        g_inputMode = 0;
        printChar(result, g_inputMode);
        g_lastInt = result;
        return;
    }
    else if (delta < 450 && delta >= 200)        //long press
    {
        g_inputMode = 0;
        return;
    }
    else if (delta >= 450 && delta < 1200)      //change char
    {
        g_inputMode += 1;
        deleteChar();
        printChar(g_lastInt, g_inputMode);
        g_lastInt = result;
        return;
    }
    else if (delta >= 1200)                     //no input
    {
        g_inputMode = 0;
        printChar(result, g_inputMode);
        g_lastInt = result;
        return;
    }
    else if (delta < 200)
        return;

}

//
//if ()
//}
//
//void DebugRoutine()
//{
//    unsigned long risingTickWidths[100];
//    unsigned long i = 0;
//    Report("hello");
//    while (1)
//    {
//        i = 0;
//
////        while (i < 100)
////            if (g_ticksElapsed)
////            {
////                risingTickWidths[i] = g_ticksElapsed;
////                ++i;
////                g_ticksElapsed = 0UL;
////            }
//        while (i < 25)
//                if (g_ticksElapsed)
//                {
//                    if (g_ticksElapsed >= 2000000UL)
//                    {
//                        gpioArr[i] = '#';
//                    }
//                    else if (g_ticksElapsed > 240000UL)
//                        gpioArr[i] = '2';
//                    else if (g_ticksElapsed > 180000UL)
//                        gpioArr[i] = '1';
//                    else if (g_ticksElapsed > 120000UL)
//                        gpioArr[i] = '0';
//                    ++i;
//                    g_ticksElapsed = 0UL;
//                }
//
//        Report("%d \r\n",decodeDEC());
//        memset(gpioArr, '\0', sizeof(gpioArr));
//    }
//}

void UARTIntHandler()
{
    unsigned char cChar;
    MAP_UARTIntDisable(UARTA1_BASE, UART_INT_RX);

    while (UARTCharsAvail(PAIRDEV))
    {
        char letter;
        char color;
        receiveMsg(letter, color);
    }

    MAP_UARTIntClear(UARTA1_BASE, UART_INT_RX);
    MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX);

}

void receiveMsg(char letter, char color)
{
    while (1)
    {
        letter = UartPairDevGetChar();
        color = UartPairDevGetChar();

        if (letter == '%' && color == '%')
        {
            y += 10;
            x = 0;
            continue;
        }

        if (color == '1')
        {
            color = WHITE;
        }
        else if (color == '2')
        {
            color = GREEN;
        }
        else if (color == '3')
        {
            color = CYAN;
        }
        else if (color == '4')
        {
            color = RED;
        }
        else
        {
            color = MAGENTA;
        }

        drawChar(x, y, letter, color, 0, 1);
        x += 6;

        if (y == 110)
        {
            fillScreen(BLACK);
            x = 0;
            y = 0;
        }
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
    MAP_SPIConfigSetExpClk(GSPI_BASE,
    MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           SPI_IF_BIT_RATE,
                           SPI_MODE_MASTER,
                           SPI_SUB_MODE_0, (SPI_SW_CTRL_CS |
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

int main()
{
    unsigned long ulStatus;

    BoardInit();

    PinMuxConfig();

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

    //
    // Turn on the timers feeding values in mSec
    //
    Timer_IF_Start(g_ulBase, TIMER_A, 1);

    // clear global variables
    pin63_intcount = 0;
    pin63_intflag = 0;
    gpioTimeCount = 0;

    // pin63 interrupts
    MAP_GPIOIntEnable(pin63.port, pin63.pin);

    SystickInit();

    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MasterMain();
    fillScreen(BLACK);

    MAP_UARTIntRegister(UARTA1_BASE, UARTIntHandler);

    // MAP_UARTFIFODisable(UARTA1_BASE);
    MAP_UARTEnable(UARTA1_BASE);
    MAP_UARTConfigSetExpClk(PAIRDEV, MAP_PRCMPeripheralClockGet(PAIRDEV_PERIPH),
    UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE));
    MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX);
    MAP_UARTIntClear(UARTA1_BASE, UART_INT_RX);

//
// Initialising the Terminal.
//
    InitTerm();

//
// Clearing the Terminal.
//
    ClearTerm();
    drawFastHLine(0, 110, 120, WHITE);

    while (1)
    {
        DebugRoutine();
    }

    // Polling: Receive data
//    char letter;
//    char color;
//
//    while (1)
//    {
//        letter = UartPairDevGetChar();
//        color = UartPairDevGetChar();
//
//        if (letter == '%' && color == '%')
//        {
//            y += 10;
//            x = 0;
//            continue;
//        }
//
//        if (color == '1')
//        {
//            color = WHITE;
//        }
//        else if (color == '2')
//        {
//            color = GREEN;
//        }
//        else if (color == '3')
//        {
//            color = CYAN;
//        }
//        else if (color == '4')
//        {
//            color = RED;
//        }
//        else
//        {
//            color = MAGENTA;
//        }
//
//        drawChar(x, y, letter, color, 0, 1);
//        x += 6;
//
//        if (y == 110)
//        {
//            fillScreen(BLACK);
//            x = 0;
//            y = 0;
//        }
//    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
