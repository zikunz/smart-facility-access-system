
#include "time_request.h"
#include <stdlib.h>
#include <string.h>

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



typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SERVER_GET_TIME_FAILED = -0x7D0,
    DNS_LOOPUP_FAILED = SERVER_GET_TIME_FAILED  -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

unsigned short g_usTimerInts;
SlSecParams_t SecurityParams = {0};

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif


//!    ######################### list of SNTP servers ##################################
//!    ##
//!    ##          hostname         |        IP       |       location
//!    ## -----------------------------------------------------------------------------
//!    ##   time-a.nist.gov         | 129.6.15.28     |  NIST, Gaithersburg, Maryland
//!    ##   time-b.nist.gov         | 129.6.15.29     |  NIST, Gaithersburg, Maryland
//!    ##   time-c.nist.gov         | 129.6.15.30     |  NIST, Gaithersburg, Maryland
//!    ##   nist1-macon.macon.ga.us | 98.175.203.200  |  Macon, Georgia
//!    ##   For more SNTP server link visit 'http://tf.nist.gov/tf-cgi/servers.cgi'
//!    ###################################################################################
const char g_acSNTPserver[30] = "time-a.nist.gov"; //Add any one of the above servers

// Tuesday is the 1st day in 2013 - the relative year
const char g_acDaysOfWeek2013[7][3] = {{"Tue"},
                                    {"Wed"},
                                    {"Thu"},
                                    {"Fri"},
                                    {"Sat"},
                                    {"Sun"},
                                    {"Mon"}};

const char g_acMonthOfYear[12][3] = {{"Jan"},
                                  {"Feb"},
                                  {"Mar"},
                                  {"Apr"},
                                  {"May"},
                                  {"Jun"},
                                  {"Jul"},
                                  {"Aug"},
                                  {"Sep"},
                                  {"Oct"},
                                  {"Nov"},
                                  {"Dec"}};

const char g_acNumOfDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

const char g_acDigits[] = "0123456789";

struct
{
    unsigned long ulDestinationIP;
    int iSockID;
    unsigned long ulElapsedSec;
    short isGeneralVar;
    unsigned long ulGeneralVar;
    unsigned long ulGeneralVar1;
    char acTimeStore[30];
    char *pcCCPtr;
    unsigned short uisCCLen;
}g_sAppData;

SlSockAddr_t sAddr;
SlSockAddrIn_t sLocalAddr;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************

//*****************************************************************************
//
//! Gets the current time from the selected SNTP server
//!
//! \brief  This function obtains the NTP time from the server.
//!
//! \param  GmtDiffHr is the GMT Time Zone difference in hours
//! \param  GmtDiffMins is the GMT Time Zone difference in minutes
//!
//! \return 0 : success, -ve : failure
//!
//
//*****************************************************************************

unsigned short itoa(short cNum, char *cString)
{
    const char     pcDigits[] = "0123456789";
    char* ptr;
    short uTemp = cNum;
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';

        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}

long GetSNTPTime(unsigned char ucGmtDiffHr, unsigned char ucGmtDiffMins)
{
  
/*
                            NTP Packet Header:


       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          Root  Delay                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root  Dispersion                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Reference Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Reference Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Originate Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Receive Timestamp (64)                     |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Transmit Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                 Key Identifier (optional) (32)                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                                                                |
      |                 Message Digest (optional) (128)                |
      |                                                                |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/
    char cDataBuf[48];
    long lRetVal = 0;
    int iAddrSize;
    //
    // Send a query ? to the NTP server to get the NTP time
    //
    memset(cDataBuf, 0, sizeof(cDataBuf));
    cDataBuf[0] = '\x1b';

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 0x00;
    sAddr.sa_data[1] = 0x7B;    // UDP port number for NTP is 123
    sAddr.sa_data[2] = (char)((g_sAppData.ulDestinationIP>>24)&0xff);
    sAddr.sa_data[3] = (char)((g_sAppData.ulDestinationIP>>16)&0xff);
    sAddr.sa_data[4] = (char)((g_sAppData.ulDestinationIP>>8)&0xff);
    sAddr.sa_data[5] = (char)(g_sAppData.ulDestinationIP&0xff);

    lRetVal = sl_SendTo(g_sAppData.iSockID,
                     cDataBuf,
                     sizeof(cDataBuf), 0,
                     &sAddr, sizeof(sAddr));
    if (lRetVal != sizeof(cDataBuf))
    {
        // could not send SNTP request
        ASSERT_ON_ERROR(SERVER_GET_TIME_FAILED);
    }

    //
    // Wait to receive the NTP time from the server
    //
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = 0;
    sLocalAddr.sin_addr.s_addr = 0;
    if(g_sAppData.ulElapsedSec == 0)
    {
        lRetVal = sl_Bind(g_sAppData.iSockID,
                (SlSockAddr_t *)&sLocalAddr,
                sizeof(SlSockAddrIn_t));
    }

    iAddrSize = sizeof(SlSockAddrIn_t);

    lRetVal = sl_RecvFrom(g_sAppData.iSockID,
                       cDataBuf, sizeof(cDataBuf), 0,
                       (SlSockAddr_t *)&sLocalAddr,
                       (SlSocklen_t*)&iAddrSize);
    ASSERT_ON_ERROR(lRetVal);

    //
    // Confirm that the MODE is 4 --> server
    //
    if ((cDataBuf[0] & 0x7) != 4)    // expect only server response
    {
         ASSERT_ON_ERROR(SERVER_GET_TIME_FAILED);  // MODE is not server, abort
    }
    else
    {
        unsigned char iIndex;

        //
        // Getting the data from the Transmit Timestamp (seconds) field
        // This is the time at which the reply departed the
        // server for the client
        //
        g_sAppData.ulElapsedSec = cDataBuf[40];
        g_sAppData.ulElapsedSec <<= 8;
        g_sAppData.ulElapsedSec += cDataBuf[41];
        g_sAppData.ulElapsedSec <<= 8;
        g_sAppData.ulElapsedSec += cDataBuf[42];
        g_sAppData.ulElapsedSec <<= 8;
        g_sAppData.ulElapsedSec += cDataBuf[43];

        //
        // seconds are relative to 0h on 1 January 1900
        //
        g_sAppData.ulElapsedSec -= TIME2013;

        //
        // in order to correct the timezone
        //
        g_sAppData.ulElapsedSec += (ucGmtDiffHr * SEC_IN_HOUR);
        g_sAppData.ulElapsedSec += (ucGmtDiffMins * SEC_IN_MIN);

        g_sAppData.pcCCPtr = &g_sAppData.acTimeStore[0];

        //
        // day, number of days since beginning of 2013
        //
        g_sAppData.isGeneralVar = g_sAppData.ulElapsedSec/SEC_IN_DAY;
        memcpy(g_sAppData.pcCCPtr,
               g_acDaysOfWeek2013[g_sAppData.isGeneralVar%7], 3);
        g_sAppData.pcCCPtr += 3;
        *g_sAppData.pcCCPtr++ = '\x20';

        //
        // month
        //
        g_sAppData.isGeneralVar %= 365;
        for (iIndex = 0; iIndex < 12; iIndex++)
        {
            g_sAppData.isGeneralVar -= g_acNumOfDaysPerMonth[iIndex];
            if (g_sAppData.isGeneralVar < 0)
                    break;
        }
        if(iIndex == 12)
        {
            iIndex = 0;
        }
        memcpy(g_sAppData.pcCCPtr, g_acMonthOfYear[iIndex], 3);
        g_sAppData.pcCCPtr += 3;
        *g_sAppData.pcCCPtr++ = '\x20';

        //
        // date
        // restore the day in current month
        //
        g_sAppData.isGeneralVar += g_acNumOfDaysPerMonth[iIndex];
        g_sAppData.uisCCLen = itoa(g_sAppData.isGeneralVar + 1,
                                   g_sAppData.pcCCPtr);
        g_sAppData.pcCCPtr += g_sAppData.uisCCLen;
        *g_sAppData.pcCCPtr++ = '\x20';

        //
        // time
        //
        g_sAppData.ulGeneralVar = g_sAppData.ulElapsedSec%SEC_IN_DAY;

        // number of seconds per hour
        g_sAppData.ulGeneralVar1 = g_sAppData.ulGeneralVar%SEC_IN_HOUR;

        // number of hours
        g_sAppData.ulGeneralVar /= SEC_IN_HOUR;
        g_sAppData.uisCCLen = itoa(g_sAppData.ulGeneralVar,
                                   g_sAppData.pcCCPtr);
        g_sAppData.pcCCPtr += g_sAppData.uisCCLen;
        *g_sAppData.pcCCPtr++ = ':';

        // number of minutes per hour
        g_sAppData.ulGeneralVar = g_sAppData.ulGeneralVar1/SEC_IN_MIN;

        // number of seconds per minute
        g_sAppData.ulGeneralVar1 %= SEC_IN_MIN;
        g_sAppData.uisCCLen = itoa(g_sAppData.ulGeneralVar,
                                   g_sAppData.pcCCPtr);
        g_sAppData.pcCCPtr += g_sAppData.uisCCLen;
        *g_sAppData.pcCCPtr++ = ':';
        g_sAppData.uisCCLen = itoa(g_sAppData.ulGeneralVar1,
                                   g_sAppData.pcCCPtr);
        g_sAppData.pcCCPtr += g_sAppData.uisCCLen;
        *g_sAppData.pcCCPtr++ = '\x20';

        //
        // year
        // number of days since beginning of 2013
        //
        g_sAppData.ulGeneralVar = g_sAppData.ulElapsedSec/SEC_IN_DAY;
        g_sAppData.ulGeneralVar /= 365;
        g_sAppData.uisCCLen = itoa(YEAR2013 + g_sAppData.ulGeneralVar,
                                   g_sAppData.pcCCPtr);
        g_sAppData.pcCCPtr += g_sAppData.uisCCLen;

        *g_sAppData.pcCCPtr++ = '\0';

        UART_PRINT("response from server: ");
        UART_PRINT((char *)g_acSNTPserver);
        UART_PRINT("\n\r");
        UART_PRINT(g_sAppData.acTimeStore);
        UART_PRINT("\n\r\n\r");
    }
    return SUCCESS;
}

//}
////*****************************************************************************
////
////! Periodic Timer Interrupt Handler
////!
////! \param None
////!
////! \return None
////
////*****************************************************************************
//void
//TimerPeriodicIntHandler(void)
//{
//    unsigned long ulInts;
//
//    //
//    // Clear all pending interrupts from the timer we are
//    // currently using.
//    //
//    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
//    MAP_TimerIntClear(TIMERA0_BASE, ulInts);
//
//    //
//    // Increment our interrupt counter.
//    //
//    g_usTimerInts++;
//    if(!(g_usTimerInts & 0x1))
//    {
//        //
//        // Off Led
//        //
//        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
//    }
//    else
//    {
//        //
//        // On Led
//        //
//        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
//    }
//}
//
////****************************************************************************
////
////! Function to configure and start timer to blink the LED while device is
////! trying to connect to an AP
////!
////! \param none
////!
////! return none
////
////****************************************************************************
//void LedTimerConfigNStart()
//{
//    //
//    // Configure Timer for blinking the LED for IP acquisition
//    //
//    Timer_IF_Init(PRCM_TIMERA0,TIMERA0_BASE,TIMER_CFG_PERIODIC,TIMER_A,0);
//    Timer_IF_IntSetup(TIMERA0_BASE,TIMER_A,TimerPeriodicIntHandler);
//    Timer_IF_Start(TIMERA0_BASE,TIMER_A,100);  // time is in mSec
//}
//
////****************************************************************************
////
////! Disable the LED blinking Timer as Device is connected to AP
////!
////! \param none
////!
////! return none
////
////****************************************************************************
//void LedTimerDeinitStop()
//{
//    //
//    // Disable the LED blinking Timer as Device is connected to AP
//    //
//    Timer_IF_Stop(TIMERA0_BASE,TIMER_A);
//    Timer_IF_DeInit(TIMERA0_BASE,TIMER_A);
//
//}
//
////****************************************************************************
////
////! Task function implementing the gettime functionality using an NTP server
////!
////! \param none
////!
////! This function
////!    1. Initializes the required peripherals
////!    2. Initializes network driver and connects to the default AP
////!    3. Creates a UDP socket, gets the NTP server IP address using DNS
////!    4. Periodically gets the NTP time and displays the time
////!
////! \return None.
////
////****************************************************************************
//void GetNTPTimeTask(void *pvParameters)
//{
//    int iSocketDesc;
//    long lRetVal = -1;
//
//    UART_PRINT("GET_TIME: Test Begin\n\r");
//
//    //
//    // Configure LED
//    //
//    GPIO_IF_LedConfigure(LED1|LED3);
//
//    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
//    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
//
//
//    //
//    // Reset The state of the machine
//    //
//    Network_IF_ResetMCUStateMachine();
//
//    //
//    // Start the driver
//    //
//    lRetVal = Network_IF_InitDriver(ROLE_STA);
//    if(lRetVal < 0)
//    {
//       UART_PRINT("Failed to start SimpleLink Device\n\r",lRetVal);
//       LOOP_FOREVER();
//    }
//
//    // switch on Green LED to indicate Simplelink is properly up
//    GPIO_IF_LedOn(MCU_ON_IND);
//
//    // Start Timer to blink Red LED till AP connection
//    LedTimerConfigNStart();
//
//    // Initialize AP security params
//    SecurityParams.Key = (signed char *)SECURITY_KEY;
//    SecurityParams.KeyLen = strlen(SECURITY_KEY);
//    SecurityParams.Type = SECURITY_TYPE;
//
//    //
//    // Connect to the Access Point
//    //
//    lRetVal = Network_IF_ConnectAP(SSID_NAME, SecurityParams);
//    if(lRetVal < 0)
//    {
//       UART_PRINT("Connection to an AP failed\n\r");
//       LOOP_FOREVER();
//    }
//
//    //
//    // Disable the LED blinking Timer as Device is connected to AP
//    //
//    LedTimerDeinitStop();
//
//    //
//    // Switch ON RED LED to indicate that Device acquired an IP
//    //
//    GPIO_IF_LedOn(MCU_IP_ALLOC_IND);
//
//    //
//    // Create UDP socket
//    //
//    iSocketDesc = sl_Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    if(iSocketDesc < 0)
//    {
//        ERR_PRINT(iSocketDesc);
//        goto end;
//    }
//    g_sAppData.iSockID = iSocketDesc;
//
//    UART_PRINT("Socket created\n\r");
//
//    //
//    // Get the NTP server host IP address using the DNS lookup
//    //
//    lRetVal = Network_IF_GetHostIP((char*)g_acSNTPserver, \
//                                    &g_sAppData.ulDestinationIP);
//
//    if( lRetVal >= 0)
//    {
//
//        struct SlTimeval_t timeVal;
//        timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
//        timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
//        lRetVal = sl_SetSockOpt(g_sAppData.iSockID,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
//                        (unsigned char*)&timeVal, sizeof(timeVal));
//        if(lRetVal < 0)
//        {
//           ERR_PRINT(lRetVal);
//           LOOP_FOREVER();
//        }
//
//        while(1)
//        {
//            //
//            // Get the NTP time and display the time
//            //
//            lRetVal = GetSNTPTime(GMT_DIFF_TIME_HRS, GMT_DIFF_TIME_MINS);
//            if(lRetVal < 0)
//            {
//                UART_PRINT("Server Get Time failed\n\r");
//                break;
//            }
//
//            //
//            // Wait a while before resuming
//            //
//            MAP_UtilsDelay(SLEEP_TIME);
//        }
//    }
//    else
//    {
//        UART_PRINT("DNS lookup failed. \n\r");
//    }
//
//    //
//    // Close the socket
//    //
//    close(iSocketDesc);
//    UART_PRINT("Socket closed\n\r");
//
//end:
//    //
//    // Stop the driver
//    //
//    lRetVal = Network_IF_DeInitDriver();
//    if(lRetVal < 0)
//    {
//       UART_PRINT("Failed to stop SimpleLink Device\n\r");
//       LOOP_FOREVER();
//    }
//
//    //
//    // Switch Off RED & Green LEDs to indicate that Device is
//    // disconnected from AP and Simplelink is shutdown
//    //
//    GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
//    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
//
//    UART_PRINT("GET_TIME: Test Complete\n\r");
//
//    //
//    // Loop here
//    //
//    LOOP_FOREVER();
//}


