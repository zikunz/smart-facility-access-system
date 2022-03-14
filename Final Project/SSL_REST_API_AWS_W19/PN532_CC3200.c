/*
 * PN532_CC3200.c
 *
 *  Created on: 2022/3/4
 *      Author: Ang Li
 */

/*
 *   Transcribed from Adafruit PN532 library for Arduino
 *   Hard coded PN532 I2C library for CC3200 capability.
 *   USE FOR EEC172 LAB
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "PN532_CC3200.h"
#include "i2c_if.h"



int getFirmwareVersion(void)
{
    unsigned char ucPacketBuffer[64] = "";   //sending packet
    unsigned char aucDataBuf[256] = "";      //received data

    unsigned char parsedData[16];

    unsigned char firmwareData[4] = {0x32, 0x01, 0x06, 0x07};

    //00 00 FF 02 FE D4 02 2A 00
    ucPacketBuffer[0] = 0x00;
    ucPacketBuffer[1] = 0x00;
    ucPacketBuffer[2] = 0xFF;
    ucPacketBuffer[3] = 0x02;
    ucPacketBuffer[4] = 0xFE;
    ucPacketBuffer[5] = 0xD4;
    ucPacketBuffer[6] = 0x02;
    ucPacketBuffer[7] = 0x2A;
    ucPacketBuffer[8] = 0x00;

    I2C_IF_Write(0x24,ucPacketBuffer,9,1);
    I2C_IF_Read(0x24,aucDataBuf, 7);
    MAP_UtilsDelay(80000);

    I2C_IF_Read(0x24,aucDataBuf, 14);
    MAP_UtilsDelay(80000);

    I2C_IF_Read(0x24,aucDataBuf, 14);
    MAP_UtilsDelay(80000);

    strncpy(parsedData, &aucDataBuf[8], 4);

    if (strcmp(parsedData,firmwareData))
        return 1;
    else
        return 0;
}




int SAMConfig(void)
{
    unsigned char ucPacketBuffer[64] = "";   //sending packet
    unsigned char aucDataBuf[256] = "";      //received data

    unsigned char firmwareData[4] = {0x32, 0x01, 0x06, 0x07};

    //00 00 FF 05 FB D4 14 01 14 01 02 00
    ucPacketBuffer[0] = 0x00;
    ucPacketBuffer[1] = 0x00;
    ucPacketBuffer[2] = 0xFF;
    ucPacketBuffer[3] = 0x05;
    ucPacketBuffer[4] = 0xFB;
    ucPacketBuffer[5] = 0xD4;
    ucPacketBuffer[6] = 0x14;
    ucPacketBuffer[7] = 0x01;
    ucPacketBuffer[8] = 0x14;
    ucPacketBuffer[9] = 0x01;
    ucPacketBuffer[10] = 0x02;
    ucPacketBuffer[11] = 0x00;

    I2C_IF_Write(0x24,ucPacketBuffer,12,1);
    I2C_IF_Read(0x24,aucDataBuf, 7);
    MAP_UtilsDelay(80000);

    I2C_IF_Read(0x24,aucDataBuf, 10);
    MAP_UtilsDelay(80000);

    I2C_IF_Read(0x24,aucDataBuf, 10);
    MAP_UtilsDelay(80000);


    return 1;
}


int readPassiveTargetID(char * UID)
{
    unsigned char ucPacketBuffer[64] = "";   //sending packet
    unsigned char aucDataBuf[256] = "";      //received data

    unsigned char parsedData[16];


    unsigned char emptyReturn[6] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF};

    int isDetected = 0;

    //00 00 FF 04 FC D4 4A 01 00 E1 00
    ucPacketBuffer[0] = 0x00;
    ucPacketBuffer[1] = 0x00;
    ucPacketBuffer[2] = 0xFF;
    ucPacketBuffer[3] = 0x04;
    ucPacketBuffer[4] = 0xFC;
    ucPacketBuffer[5] = 0xD4;
    ucPacketBuffer[6] = 0x4A;
    ucPacketBuffer[7] = 0x01;
    ucPacketBuffer[8] = 0x00;
    ucPacketBuffer[9] = 0xE1;
    ucPacketBuffer[10] = 0x00;

    I2C_IF_Write(0x24,ucPacketBuffer,11,1);
    I2C_IF_Read(0x24,aucDataBuf, 7);
    MAP_UtilsDelay(80000);

//    I2C_IF_Read(0x24,aucDataBuf, 25);
//    MAP_UtilsDelay(80000);

    while(isDetected == 0)
    {
        memcpy(parsedData, &aucDataBuf, 6);

        if (aucDataBuf[0] == 1 && ~strcmp(parsedData,emptyReturn))
            isDetected  = 1;
        I2C_IF_Read(0x24,aucDataBuf, 25);
        MAP_UtilsDelay(800000);
    }

    int i = 0;

    while (aucDataBuf[i+14] != 0x00)
    {
        UID[i] = aucDataBuf[i+14];
        i++;
    }

    if(strlen(UID) == 5)
    {
        UID[4] = 0;
        return 1;
    }
    if(strlen(UID) == 8)
    {
        UID[7] = 0;
        return 2;
    }

    return 0;

}

