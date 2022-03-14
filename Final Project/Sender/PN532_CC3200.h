/*
 * PN532_CC3200.h
 *
 *  Created on: 2022/3/4
 *      Author: L1A
 */

#ifndef PN532_CC3200_H_
#define PN532_CC3200_H_

#include <stdint.h>

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

//Prefixes
#define devAddr (0x48 << 1)    //PN532 device address I2C

//Variables


//Functions
int getFirmwareVersion(void);
int SAMConfig(void);
int readPassiveTargetID(char * UID);


#endif /* PN532_CC3200_H_ */
