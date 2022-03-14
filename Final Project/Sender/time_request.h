/*
 * time_request.h
 *
 *  Created on: 2022Äê3ÔÂ6ÈÕ
 *      Author: L1A
 */

#ifndef TIME_REQUEST_H_
#define TIME_REQUEST_H_

// Standard includes
#include <stdlib.h>
#include <string.h>

// simplelink includes
#include "simplelink.h"

// driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"

//Free_rtos/ti-rtos includes
//#include "osi.h"

// common interface includes
#include "network_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#include "gpio_if.h"
#include "timer_if.h"
#include "udma_if.h"
#include "common.h"

#define TIME2013                3565987200u      /* 113 years + 28 days(leap) */
#define YEAR2013                2013
#define SEC_IN_MIN              60
#define SEC_IN_HOUR             3600
#define SEC_IN_DAY              86400

#define SERVER_RESPONSE_TIMEOUT 10
#define GMT_DIFF_TIME_HRS       5
#define GMT_DIFF_TIME_MINS      30

#define SLEEP_TIME              80000000
#define OSI_STACK_SIZE          2048




unsigned short itoa(short cNum, char *cString);
long GetSNTPTime(unsigned char ucGmtDiffHr, unsigned char ucGmtDiffMins);

#endif /* TIME_REQUEST_H_ */
