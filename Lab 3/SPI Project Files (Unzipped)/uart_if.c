// Standard includes
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"

#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
#include "osi.h"
#endif

#include "uart_if.h"

#define IS_SPACE(x)       (x == 32 ? 1 : 0)

//*****************************************************************************
// Global variable indicating command is present
//*****************************************************************************
static unsigned long __Errorlog;

//*****************************************************************************
// Global variable indicating input length
//*****************************************************************************
unsigned int ilen = 1;

//*****************************************************************************
//
//! Initialization
//!
//! This function
//!        1. Configures the UART to be used.
//!
//! \return none
//
//*****************************************************************************
void InitTerm()
{
#ifndef NOTERM
    MAP_UARTConfigSetExpClk(CONSOLE, MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH),
    UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE));
#endif
    __Errorlog = 0;
}

//*****************************************************************************
//
//!    Outputs a character string to the console
//!
//! \param str is the pointer to the string to be printed
//!
//! This function
//!        1. prints the input string character by character on to the console.
//!
//! \return none
//
//*****************************************************************************
void Message(const char *str)
{
#ifndef NOTERM
    if (str != NULL)
    {
        while (*str != '\0')
        {
            MAP_UARTCharPut(CONSOLE, *str++);
        }
    }
#endif
}

//*****************************************************************************
//
//!    Clear the console window
//!
//! This function
//!        1. clears the console window.
//!
//! \return none
//
//*****************************************************************************
void ClearTerm()
{
    Message("\33[2J\r");
}

//*****************************************************************************
//
//! Error Function
//!
//! \param 
//!
//! \return none
//! 
//*****************************************************************************
void Error(char *pcFormat, ...)
{
#ifndef NOTERM
    char cBuf[256];
    va_list list;
    va_start(list, pcFormat);
    vsnprintf(cBuf, 256, pcFormat, list);
    Message(cBuf);
#endif
    __Errorlog++;
}

//*****************************************************************************
//
//! Get the Command string from UART
//!
//! \param  pucBuffer is the command store to which command will be populated
//! \param  ucBufLen is the length of buffer store available
//!
//! \return Length of the bytes received. -1 if buffer length exceeded.
//! 
//*****************************************************************************
int GetCmd(char *pcBuffer, unsigned int uiBufLen)
{
    char cChar;
    int iLen = 0;

    //
    // Wait to receive a character over UART
    //
    while (MAP_UARTCharsAvail(CONSOLE) == false)
    {
#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
        osi_Sleep(1);
#endif
    }
    cChar = MAP_UARTCharGetNonBlocking(CONSOLE);

    //
    // Echo the received character
    //
    MAP_UARTCharPut(CONSOLE, cChar);
    iLen = 0;

    //
    // Checking the end of Command
    //
    while ((cChar != '\r') && (cChar != '\n'))
    {
        //
        // Handling overflow of buffer
        //
        if (iLen >= uiBufLen)
        {
            return -1;
        }

        //
        // Copying Data from UART into a buffer
        //
        if (cChar != '\b')
        {
            *(pcBuffer + iLen) = cChar;
            iLen++;
        }
        else
        {
            //
            // Deleting last character when you hit backspace 
            //
            if (iLen)
            {
                iLen--;
            }
        }
        //
        // Wait to receive a character over UART
        //
        while (MAP_UARTCharsAvail(CONSOLE) == false)
        {
#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
            osi_Sleep(1);
#endif
        }
        cChar = MAP_UARTCharGetNonBlocking(CONSOLE);
        //
        // Echo the received character
        //
        MAP_UARTCharPut(CONSOLE, cChar);
    }

    *(pcBuffer + iLen) = '\0';

    Report("\n\r");

    return iLen;
}

//*****************************************************************************
//
//!    Trim the spaces from left and right end of given string
//!
//! \param  Input string on which trimming happens
//!
//! \return length of trimmed string
//
//*****************************************************************************
int TrimSpace(char * pcInput)
{
    size_t size;
    char *endStr, *strData = pcInput;
    char index = 0;
    size = strlen(strData);

    if (!size)
        return 0;

    endStr = strData + size - 1;
    while (endStr >= strData && IS_SPACE(*endStr))
        endStr--;
    *(endStr + 1) = '\0';

    while (*strData && IS_SPACE(*strData))
    {
        strData++;
        index++;
    }
    memmove(pcInput, strData, strlen(strData) + 1);

    return strlen(pcInput);
}

//*****************************************************************************
//
//!    prints the formatted string on to the console
//!
//! \param format is a pointer to the character string specifying the format in
//!           the following arguments need to be interpreted.
//! \param [variable number of] arguments according to the format in the first
//!         parameters
//! This function
//!        1. prints the formatted error statement.
//!
//! \return count of characters printed
//
//*****************************************************************************
int Report(const char *pcFormat, ...)
{
    int iRet = 0;
#ifndef NOTERM

    char *pcBuff, *pcTemp;
    int iSize = 256;

    va_list list;
    pcBuff = (char*) malloc(iSize);
    if (pcBuff == NULL)
    {
        return -1;
    }
    while (1)
    {
        va_start(list, pcFormat);
        iRet = vsnprintf(pcBuff, iSize, pcFormat, list);
        va_end(list);
        if (iRet > -1 && iRet < iSize)
        {
            break;
        }
        else
        {
            iSize *= 2;
            if ((pcTemp = realloc(pcBuff, iSize)) == NULL)
            {
                Message("Could not reallocate memory\n\r");
                iRet = -1;
                break;
            }
            else
            {
                pcBuff = pcTemp;
            }

        }
    }
    Message(pcBuff);
    free(pcBuff);

#endif
    return iRet;
}
