// Application Name: Lab 3
// Students: Ang Li, Zikun Zhu
//
//*****************************************************************************

// Standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
#include "uart.h"
#include "interrupt.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "hw_apps_rcm.h"
#include "gpio.h"
#include "test.h"
#include "utils.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "i2c_if.h"

#include "pin_mux_config.h"

#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "I2C Demo"
#define FOREVER                 1
#define UART_PRINT              Report
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// LED definitions
#define OLED_MAXIMUM_LENGTH 125

// Game definitions
#define GAME_BACKGROUND_COLOR BLACK
#define BALL_COLOR WHITE
#define RED_APPLE_COLOR RED
#define YELLOW_APPLE_COLOR YELLOW
#define BALL_SPEED_SCALING_FACTOR 8
#define YELLOW_APPLE_SPEED_SCALING_FACTOR 10
#define SPEED_INCREMENTAL_COEFFICIENT 1.2
#define BALL_STARTING_SIZE 4
#define BALL_INCREMENTAL_SIZE 1
#define RED_BALL_SIZE 2
#define YELLOW_BALL_SIZE 3

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
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

//*****************************************************************************
//
//! Main function for spi demo application
//!
//! \param none
//!
//! \return None.
//

//*****************************************************************************
//
//! Display the buffer contents over I2C
//!
//! \param  pucDataBuf is the pointer to the data store to be displayed
//! \param  ucLen is the length of the data to be displayed
//!
//! \return none
//!
//*****************************************************************************
int DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen)
{
    unsigned char ucBufIndx = 0;
    int buffer;
    while (ucBufIndx < ucLen)
    {
        buffer = (int) pucDataBuf[ucBufIndx];
        if (buffer & 0x80)
        {
            buffer = buffer | 0xffffff00;
        }
        ucBufIndx++;
    }
    return buffer;
}

//
//! Parses the read command parameters and invokes the I2C APIs
//!
//! \param pcInpString pointer to the user command parameters
//!
//! This function
//!    1. Parses the read command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessReadCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucLen;
    unsigned char aucDataBuf[256];
    char *pcErrPtr;
    int iRetVal;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);
    //
    // Get the length of data to be read
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucLen = (unsigned char) strtoul(pcInpString, &pcErrPtr, 10);
    //RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

    //
    // Read the specified length of data
    //
    iRetVal = I2C_IF_Read(ucDevAddr, aucDataBuf, ucLen);

    if (iRetVal == SUCCESS)
    {
        printf("I2C Read complete\n\r");

        //
        // Display the buffer over UART on successful write
        //
        DisplayBuffer(aucDataBuf, ucLen);
    }
    else
    {
        printf("I2C Read failed\n\r");
        return FAILURE;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Parses the readreg command parameters and invokes the I2C APIs
//! i2c readreg 0x<dev_addr> 0x<reg_offset> <rdlen>
//!
//! \param pcInpString pointer to the readreg command parameters
//!
//! This function
//!    1. Parses the readreg command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessReadRegCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucRegOffset, ucRdLen;
    unsigned char aucRdDataBuf[256];
    char *pcErrPtr;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);
    //
    // Get the register offset address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucRegOffset = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);

    //
    // Get the length of data to be read
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucRdLen = (unsigned char) strtoul(pcInpString, &pcErrPtr, 10);
    //RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

    //
    // Write the register address to be read from.
    // Stop bit implicitly assumed to be 0.
    //
    RET_IF_ERR(I2C_IF_Write(ucDevAddr, &ucRegOffset, 1, 0));

    //
    // Read the specified length of data
    //
    RET_IF_ERR(I2C_IF_Read(ucDevAddr, &aucRdDataBuf[0], ucRdLen));

    //
    // Display the buffer over UART on successful readreg
    //
    return DisplayBuffer(aucRdDataBuf, ucRdLen);
}

//****************************************************************************
//
//! Parses the writereg command parameters and invokes the I2C APIs
//! i2c writereg 0x<dev_addr> 0x<reg_offset> <wrlen> <0x<byte0> [0x<byte1> ...]>
//!
//! \param pcInpString pointer to the readreg command parameters
//!
//! This function
//!    1. Parses the writereg command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessWriteRegCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucRegOffset, ucWrLen;
    unsigned char aucDataBuf[256];
    char *pcErrPtr;
    int iLoopCnt = 0;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);

    //
    // Get the register offset to be written
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucRegOffset = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);
    aucDataBuf[iLoopCnt] = ucRegOffset;
    iLoopCnt++;

    //
    // Get the length of data to be written
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucWrLen = (unsigned char) strtoul(pcInpString, &pcErrPtr, 10);
    //RETERR_IF_TRUE(ucWrLen > sizeof(aucDataBuf));

    //
    // Get the bytes to be written
    //
    for (; iLoopCnt < ucWrLen + 1; iLoopCnt++)
    {
        //
        // Store the data to be written
        //
        pcInpString = strtok(NULL, " ");
        RETERR_IF_TRUE(pcInpString == NULL);
        aucDataBuf[iLoopCnt] = (unsigned char) strtoul(pcInpString + 2,
                                                       &pcErrPtr, 16);
    }
    //
    // Write the data values.
    //
    RET_IF_ERR(I2C_IF_Write(ucDevAddr, &aucDataBuf[0], ucWrLen + 1, 1));

    printf("I2C Write To address complete\n\r");

    return SUCCESS;
}

//****************************************************************************
//
//! Parses the write command parameters and invokes the I2C APIs
//!
//! \param pcInpString pointer to the write command parameters
//!
//! This function
//!    1. Parses the write command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessWriteCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucStopBit, ucLen;
    unsigned char aucDataBuf[256];
    char *pcErrPtr;
    int iRetVal, iLoopCnt;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char) strtoul(pcInpString + 2, &pcErrPtr, 16);

    //
    // Get the length of data to be written
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucLen = (unsigned char) strtoul(pcInpString, &pcErrPtr, 10);
    //RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

    for (iLoopCnt = 0; iLoopCnt < ucLen; iLoopCnt++)
    {
        //
        // Store the data to be written
        //
        pcInpString = strtok(NULL, " ");
        RETERR_IF_TRUE(pcInpString == NULL);
        aucDataBuf[iLoopCnt] = (unsigned char) strtoul(pcInpString + 2,
                                                       &pcErrPtr, 16);
    }

    //
    // Get the stop bit
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucStopBit = (unsigned char) strtoul(pcInpString, &pcErrPtr, 10);

    //
    // Write the data to the specified address
    //
    iRetVal = I2C_IF_Write(ucDevAddr, aucDataBuf, ucLen, ucStopBit);
    if (iRetVal == SUCCESS)
    {
        printf("I2C Write complete\n\r");
    }
    else
    {
        printf("I2C Write failed\n\r");
        return FAILURE;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Parses the user input command and invokes the I2C APIs
//!
//! \param pcCmdBuffer pointer to the user command
//!
//! This function
//!    1. Parses the user command.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ParseNProcessCmd(char *pcCmdBuffer)
{
    char *pcInpString;
    int iRetVal;

    pcInpString = strtok(pcCmdBuffer, " \n\r");
    if (pcInpString != NULL)

    {

        if (!strcmp(pcInpString, "read"))
        {
            //
            // Invoke the read command handler
            //
            iRetVal = ProcessReadCommand(pcInpString);
        }
        else if (!strcmp(pcInpString, "readreg"))
        {
            //
            // Invoke the readreg command handler
            //
            iRetVal = ProcessReadRegCommand(pcInpString);
        }
        else if (!strcmp(pcInpString, "writereg"))
        {
            //
            // Invoke the writereg command handler
            //
            iRetVal = ProcessWriteRegCommand(pcInpString);
        }
        else if (!strcmp(pcInpString, "write"))
        {
            //
            // Invoke the write command handler
            //
            iRetVal = ProcessWriteCommand(pcInpString);
        }
        else
        {
            printf("Unsupported command\n\r");
            return FAILURE;
        }
    }

    return iRetVal;
}

//*****************************************************************************
//
//! Main function handling the I2C example
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
void main()
{
    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing UART and SPI lines.
    //
    PinMuxConfig();

    I2C_IF_Open(I2C_MASTER_MODE_FST);

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MasterMain();
    fillScreen(BLACK);

    int ball_x_position = OLED_MAXIMUM_LENGTH / 2;
    int ball_y_position = OLED_MAXIMUM_LENGTH / 2;
    int ball_x_offset_position;
    int ball_y_offset_position;
    int red_apple_x_position = rand() % OLED_MAXIMUM_LENGTH; // If we want a number to be between 0 to 124 (inclusive), we need to % a random number by 125
    int red_apple_y_position = rand() % OLED_MAXIMUM_LENGTH;
    int yellow_apple_x_position = rand() % OLED_MAXIMUM_LENGTH - 1; // If we want a number to be between 0 to 123 (inclusive), we need to % a random number by 124
    int yellow_apple_y_position = rand() % OLED_MAXIMUM_LENGTH - 1; // If we want a number to be between 0 to 123 (inclusive), we need to % a random number by 124
    int game_stage = 0; // When game_stage increases, the speed of the ball increases proportionally

    int ball_current_size = BALL_STARTING_SIZE; // Starting ball size is 4 pixels

    while (1)
    {
        char x_buffer[256] = "readreg 0x18 0x5 1 \n\r";
        char y_buffer[256] = "readreg 0x18 0x3 1 \n\r";

        if (abs(ball_x_position - red_apple_x_position) <= ball_current_size
                && abs(ball_y_position - red_apple_y_position)
                        <= ball_current_size) // If the ball touches the red apple
        {
            fillCircle(red_apple_x_position, red_apple_y_position, 2,
            GAME_BACKGROUND_COLOR); // Make the red apple disappear by filling it black
            red_apple_x_position = rand() % OLED_MAXIMUM_LENGTH; // Compute a random x coordinate
            red_apple_y_position = rand() % OLED_MAXIMUM_LENGTH; // Compute a random y coordinate
            ball_current_size += BALL_INCREMENTAL_SIZE; // The size of the ball increases by 2 pixels
            game_stage++;
        }

        if (abs(ball_x_position - yellow_apple_x_position) <= ball_current_size
                && abs(ball_y_position - yellow_apple_y_position)
                        <= ball_current_size) // If the ball touches the yellow apple
        {
            fillScreen(BLACK);
            drawChar(0, 64, 'G', BLUE, BLUE, 2);
            drawChar(15, 64, 'A', BLUE, BLUE, 2);
            drawChar(30, 64, 'M', BLUE, BLUE, 2);
            drawChar(45, 64, 'E', BLUE, BLUE, 2);
            drawChar(70, 64, 'O', BLUE, BLUE, 2);
            drawChar(85, 64, 'V', BLUE, BLUE, 2);
            drawChar(100, 64, 'E', BLUE, BLUE, 2);
            drawChar(115, 64, 'R', BLUE, BLUE, 2);
            printf("GAME OVER! Game stage reached is %d :)", game_stage);
            break;
        }

        fillCircle(red_apple_x_position, red_apple_y_position, RED_BALL_SIZE,
        RED_APPLE_COLOR); // Let a red apple appear at a pseudo-random 2D spot if it is executed for the first time, otherwise, make a new red apple appear after the ball touches it
        fillCircle(yellow_apple_x_position, yellow_apple_y_position,
        YELLOW_BALL_SIZE,
                   YELLOW_APPLE_COLOR);

        // Yellow ball is performing a random walk
        int random_boolean = rand() & 1;
        if (random_boolean)
        {
            yellow_apple_x_position += 1;
            fillCircle(yellow_apple_x_position - 1, yellow_apple_y_position,
            YELLOW_BALL_SIZE,
                       GAME_BACKGROUND_COLOR);
        }
        else
        {
            yellow_apple_x_position -= 1;
            fillCircle(yellow_apple_x_position + 1, yellow_apple_y_position,
            YELLOW_BALL_SIZE,
                       GAME_BACKGROUND_COLOR);
        }

        random_boolean = rand() & 1;
        if (random_boolean)
        {
            yellow_apple_y_position += 1;
            fillCircle(yellow_apple_x_position, yellow_apple_y_position - 1,
            YELLOW_BALL_SIZE,
                       GAME_BACKGROUND_COLOR);
        }
        else
        {
            yellow_apple_y_position -= 1;
            fillCircle(yellow_apple_x_position, yellow_apple_y_position + 1,
            YELLOW_BALL_SIZE,
                       GAME_BACKGROUND_COLOR);
        }

        // Find the offset position of x and y needed and decrease it by 10 times and increase it proportional according to its current size
        ball_x_offset_position = ParseNProcessCmd(x_buffer)
                / BALL_SPEED_SCALING_FACTOR
                * pow(SPEED_INCREMENTAL_COEFFICIENT, game_stage);
        ball_y_offset_position = ParseNProcessCmd(y_buffer)
                / BALL_SPEED_SCALING_FACTOR
                * pow(SPEED_INCREMENTAL_COEFFICIENT, game_stage);

        // Increase the magnitude of the "velocity" by subtracting offset_position required needed in both dimensions according to the current tilt angle, we use "-" here because of the positioning of our OLED on the breadboard
        ball_x_position -= ball_x_offset_position;
        ball_y_position -= ball_y_offset_position;

        // Bound the ball position, note that the yellow ball is not bounded
        if (ball_x_position >= (OLED_MAXIMUM_LENGTH - ball_current_size))
        {
            ball_x_position = OLED_MAXIMUM_LENGTH - ball_current_size;
        }
        else if (ball_x_position <= ball_current_size)
        {
            ball_x_position = ball_current_size;
        }

        if (ball_y_position >= (OLED_MAXIMUM_LENGTH - ball_current_size))
        {
            ball_y_position = OLED_MAXIMUM_LENGTH - ball_current_size;
        }
        else if (ball_y_position <= ball_current_size)
        {
            ball_y_position = ball_current_size;
        }

        // Whenever the ball moves, its current location should be filled with the ball color
        fillCircle(ball_x_position, ball_y_position, ball_current_size,
        BALL_COLOR);

        fillCircle(yellow_apple_x_position, yellow_apple_y_position,
        YELLOW_BALL_SIZE,
                   YELLOW_APPLE_COLOR);

        // Whenever the ball moves, its previous location should be filled with the background color
        fillCircle(ball_x_position, ball_y_position, ball_current_size,
        GAME_BACKGROUND_COLOR);

        if (game_stage == 10)
        {
            fillScreen(BLACK);
            drawChar(0, 64, 'G', RED, RED, 2);
            drawChar(15, 64, 'A', RED, RED, 2);
            drawChar(30, 64, 'M', RED, RED, 2);
            drawChar(45, 64, 'E', RED, RED, 2);
            drawChar(70, 64, 'O', RED, RED, 2);
            drawChar(85, 64, 'V', RED, RED, 2);
            drawChar(100, 64, 'E', RED, RED, 2);
            drawChar(115, 64, 'R', RED, RED, 2);
            printf("YOU WON! Game stage reached is %d :D", game_stage);
            break;
        }
    }
}
