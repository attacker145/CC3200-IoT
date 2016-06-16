//*****************************************************************************
// Exosite Modifications:
// Copyright (C) 2014 Exosite LLC
//
// Original File:
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
// Application Name     - Exosite Cloud Demo
// Application Overview - This Application demonstrates using the CC3200
//                        LaunchPad with Exosite's cloud platform.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup Exosite
//! @{
//
//****************************************************************************

// Standard includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Simplelink includes
#include "simplelink.h"
#include "netcfg.h"

// Driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "adc.h"

#include "rom_map.h"
#include "gpio.h"

// OS includes
#include "osi.h"

extern unsigned int GETChar(unsigned char *ucBuffer);
extern void basic_Interpreter(void *pvParameters);

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "common.h"

// App Includes
#include "exosite.h"
#include "device_status.h"
#include "smartconfig.h"
#include "tmp006drv.h"
#include "bma222drv.h"
#include "pinmux.h"
#include "serial_wifi.h"

#define APPLICATION_VERSION              "0.1.0"
#define APP_NAME                         "Exosite Cloud Demo"
//#define EXOSITE_TASK_PRIORITY            2
#define EXOSITE_TASK_PRIORITY            3
#define LOW_TASK_PRIORITY                1
#define HIGH_TASK_PRIORITY               2
#define SPAWN_TASK_PRIORITY              9
#define OSI_STACK_SIZE                   2048
#define SMALL_STACK_SIZE                 512
#define AP_SSID_LEN_MAX                 32
#define SH_GPIO_3                       3       /* P58 - Device Mode */
#define AUTO_CONNECTION_TIMEOUT_COUNT   50      /* 5 Sec */
#define SL_STOP_TIMEOUT                 200

unsigned char printflag = 0;

typedef enum
{
  LED_OFF = 0,
  LED_ON,
  LED_BLINK
}eLEDStatus;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static const char pcDigits[] = "0123456789";
static unsigned char POST_token[] = "__SL_P_ULD";
static unsigned char GET_token_TEMP[]  = "__SL_G_UTP";
static unsigned char GET_token_ACC[]  = "__SL_G_UAC";
static unsigned char GET_token_UIC[]  = "__SL_G_UIC";
static unsigned char GET_token_URT[]  = "__SL_G_URT";
static int g_iInternetAccess = -1;
static unsigned char g_ucDryerRunning = 0;
static unsigned int g_uiDeviceModeConfig = ROLE_STA; //default is STA mode
static unsigned char g_ucLEDStatus = LED_OFF;
static unsigned long  g_ulStatus = 0;//SimpleLink Status
static unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
static unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
char acCmdStore[512];
int iRetVal = FAILURE;

unsigned long pulAdcSamples[4096];

// Exosite Demo Globals
volatile static unsigned long long g_SW2Counter = 0;
volatile static unsigned long long g_uptimeSec = 0;


volatile static float g_accXIntervalSum = 0;
volatile static float g_accYIntervalSum = 0;
volatile static float g_accZIntervalSum = 0;
volatile static long long g_accSampleCount = 0;

volatile static float g_accTotalAvg;
volatile static float g_accXAvg;
volatile static float g_accYAvg;
volatile static float g_accZAvg;

void AccSample();

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


#ifdef USE_FREERTOS
//*****************************************************************************
//
//! Application defined hook (or callback) function - the tick hook.
//! The tick interrupt can optionally call this
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationTickHook( void )
{
}

//*****************************************************************************
//
//! Application defined hook (or callback) function - assert
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    while(1)
    {

    }
}

//*****************************************************************************
//
//! Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void )
{
	//AccSample();
}

//*****************************************************************************
//
//! Application provided stack overflow hook function.
//!
//! \param  handle of the offending task
//! \param  name  of the offending task
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationStackOverflowHook( OsiTaskHandle *pxTask, signed char *pcTaskName)
{
    ( void ) pxTask;
    ( void ) pcTaskName;

    for( ;; );
}

void vApplicationMallocFailedHook()
{
    while(1)
  {
    // Infinite loop;
  }
}
#endif

//*****************************************************************************
//
//! itoa
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters
//!
//!
//
//*****************************************************************************
static unsigned short itoa(char cNum, char *cString)
{
    char* ptr;
    char uTemp = cNum;
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


//*****************************************************************************
//
//! ReadAccSensor
//!
//!    @brief  Read Accelerometer Data from Sensor
//!
//!
//!     @return none
//!
//!
//
//*****************************************************************************
void ReadAccSensor()
{
    //Define Accelerometer Threshold to Detect Movement
    const short csAccThreshold    = 5;

    signed char cAccXT1,cAccYT1,cAccZT1;
    signed char cAccXT2,cAccYT2,cAccZT2;
    signed short sDelAccX, sDelAccY, sDelAccZ;
    int iRet = -1;
    int iCount = 0;
      
    iRet = BMA222ReadNew(&cAccXT1, &cAccYT1, &cAccZT1);	//Get the raw accelerometer data register readings
    if(iRet)
    {
        //In case of error/ No New Data return
        return;
    }
    for(iCount=0;iCount<2;iCount++)
    {
        MAP_UtilsDelay((90*80*1000)); //30msec
        iRet = BMA222ReadNew(&cAccXT2, &cAccYT2, &cAccZT2);
        if(iRet)
        {
            //In case of error/ No New Data continue
            iRet = 0;
            continue;
        }

        else
        {                       
            sDelAccX = abs((signed short)cAccXT2 - (signed short)cAccXT1);
            sDelAccY = abs((signed short)cAccYT2 - (signed short)cAccYT1);
            sDelAccZ = abs((signed short)cAccZT2 - (signed short)cAccZT1);

            //Compare with Pre defined Threshold
            if(sDelAccX > csAccThreshold || sDelAccY > csAccThreshold ||
               sDelAccZ > csAccThreshold)
            {
                //Device Movement Detected, Break and Return
                g_ucDryerRunning = 1;
                break;
            }
            else
            {
                //Device Movement Static
                g_ucDryerRunning = 0;
            }
        }
    }
       
}

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'
            // Applications can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] Device Connected to the AP: %s , "
                       "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      g_ucConnectionSSID,g_ucConnectionBSSID[0],
                      g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                      g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                      g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                UART_PRINT("[WLAN EVENT] Device disconnected from the AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on application's "
                           "request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR] Device disconnected from the AP AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
            // when device is in AP mode and any client connects to device cc3xxx
            //SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            //CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION_FAILED);

            //
            // Information about the connected client (like SSID, MAC etc) will
            // be available in 'slPeerInfoAsyncResponse_t' - Applications
            // can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.APModeStaConnected;
            //

            UART_PRINT("[WLAN EVENT] Station connected to device\n\r");
        }
        break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
        {
            // when client disconnects from device (AP)
            //CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            //CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the connected client (like SSID, MAC etc) will
            // be available in 'slPeerInfoAsyncResponse_t' - Applications
            // can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.APModestaDisconnected;
            //
            UART_PRINT("[WLAN EVENT] Station disconnected from device\n\r");
        }
        break;

        case SL_WLAN_SMART_CONFIG_COMPLETE_EVENT:
        {
            //SET_STATUS_BIT(g_ulStatus, STATUS_BIT_SMARTCONFIG_START);

            //
            // Information about the SmartConfig details (like Status, SSID,
            // Token etc) will be available in 'slSmartConfigStartAsyncResponse_t'
            // - Applications can use it if required
            //
            //  slSmartConfigStartAsyncResponse_t *pEventData = NULL;
            //  pEventData = &pSlWlanEvent->EventData.smartConfigStartResponse;
            //

        }
        break;

        case SL_WLAN_SMART_CONFIG_STOP_EVENT:
        {
            // SmartConfig operation finished
            //CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_SMARTCONFIG_START);

            //
            // Information about the SmartConfig details (like Status, padding
            // etc) will be available in 'slSmartConfigStopAsyncResponse_t' -
            // Applications can use it if required
            //
            // slSmartConfigStopAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.smartConfigStopResponse;
            //
        }
        break;

        default:
        {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                       "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));

            UNUSED(pEventData);
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the IP-Leased details(like IP-Leased,lease-time,
            // mac etc) will be available in 'SlIpLeasedAsync_t' - Applications
            // can use it if required
            //
            // SlIpLeasedAsync_t *pEventData = NULL;
            // pEventData = &pNetAppEvent->EventData.ipLeased;
            //

        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the IP-Released details (like IP-address, mac
            // etc) will be available in 'SlIpReleasedAsync_t' - Applications
            // can use it if required
            //
            // SlIpReleasedAsync_t *pEventData = NULL;
            // pEventData = &pNetAppEvent->EventData.ipReleased;
            //
        }
		break;

        default:
        {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}

void concat(char *str1,char *str2)
{
    char *conc=str1;

    while(*str1!='\0')
        str1++;

    *str1=' ';
    ++str1;

    while(*str2!='\0')
    {
        *str1=*str2;
        str1++,str2++;
    }

    *str1='\0';
    printf("Concatenated String:\n");
    puts(conc);

}

//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, 
                                SlHttpServerResponse_t *pSlHttpServerResponse)
{
    switch (pSlHttpServerEvent->Event)
    {
        case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:		// Get data from the server
        {
            unsigned char *ptr;
            unsigned char *ptraccX;
            unsigned char *ptraccY;
            unsigned char *ptraccZ;
            //unsigned char *tokenName;
            ptr = pSlHttpServerResponse->ResponseData.token_value.data;		// initialize pointer to location data: _u8     *data;
            pSlHttpServerResponse->ResponseData.token_value.len = 0;		// initialize length to zero
            UART_PRINT("\n\n\rHTTP_GET Tocken:\t %s",ptr);  // add this line only. Print string pointed by ptr. Basically .data will be printed
            if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, 
                    GET_token_TEMP, strlen((const char *)GET_token_TEMP)) == 0)	//__SL_G_UTP
            {
                float fCurrentTemp;
                TMP006DrvGetTemp(&fCurrentTemp);		// Get the temperature value and write it to fCurrentTemp
                char cTemp = (char)fCurrentTemp;		// 8 bit temperature value
                short sTempLen = itoa(cTemp,(char*)ptr);// Convert integer to ASCII in decimal base
                ptr[sTempLen++] = ' ';
                ptr[sTempLen] = 'F';
                pSlHttpServerResponse->ResponseData.token_value.len += sTempLen;

                //********************************************************************
                Report("\n\r Temperature HEX: \t");
                MAP_UARTCharPut(CONSOLE, (unsigned char) cTemp);
                Report("\n\r");
                //********************************************************************


            }

            if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, 
                      GET_token_UIC, strlen((const char *)GET_token_UIC)) == 0)
            {
                if(g_iInternetAccess==0)
                    strcpy((char*)pSlHttpServerResponse->ResponseData.token_value.data,"1");	// LED ON
                else
                    strcpy((char*)pSlHttpServerResponse->ResponseData.token_value.data,"0");	// LED OFF
                pSlHttpServerResponse->ResponseData.token_value.len =  1;
            }

            if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, 
                       GET_token_ACC, strlen((const char *)GET_token_ACC)) == 0)
            {

            	//unsigned char *str3;
            	//int i=0;
            	ReadAccSensor(); //float *pfCurrTemp
            	ptraccX = pSlHttpServerResponse->ResponseData.token_value.data;
            	//ptraccY = pSlHttpServerResponse->ResponseData.token_value.data;
            	//ptraccZ = pSlHttpServerResponse->ResponseData.token_value.data;
            	signed char cAccXT1, cAccYT1, cAccZT1;
            	//unsigned char cAccXTu, cAccYTu, cAccZTu;
            	unsigned char cAccYTu, cAccZTu;
            	//_u8     *cAccXT2;	//_u8     *data;
            	_u8     *cAccYT2;
            	_u8     *cAccZT2;	//_u8     *data;

            	BMA222ReadNew(&cAccXT1, &cAccYT1, &cAccZT1);

            	//cAccXTu = (unsigned char) cAccXT1;
            	cAccYTu = (unsigned char) cAccYT1;
            	cAccZTu = (unsigned char) cAccZT1;

            	//cAccXT2 = &cAccXTu;
            	cAccYT2 = &cAccYTu;
            	cAccZT2 = &cAccZTu;

            	//ptraccX = cAccXT2;
            	ptraccY = cAccYT2;
            	ptraccZ = cAccZT2;

                short sLenAccX = itoa(g_accXIntervalSum,(char*)ptraccX);// Convert integer to ASCII in decimal base. Convert x-axis value to a decimal string.
                short sLenAccY = itoa(g_accYIntervalSum,(char*)ptraccY);// String that contains Y-axis data
                ptraccY[sLenAccY++] = ' ';
                short sLenAccZ = itoa(g_accZIntervalSum,(char*)ptraccZ);// String that contains Z-axis data
                ptraccY[sLenAccZ++] = ' ';

                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = 'X';
                ptraccX[sLenAccX++] = '-';
                ptraccX[sLenAccX++] = 'a';
                ptraccX[sLenAccX++] = 'x';
                ptraccX[sLenAccX++] = 'e';
                ptraccX[sLenAccX++] = 's';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';


                ptraccX[sLenAccX++] = ptraccY[0];
                ptraccX[sLenAccX++] = ptraccY[1];
                ptraccX[sLenAccX++] = ptraccY[2];

                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = 'Y';
                ptraccX[sLenAccX++] = '-';
                ptraccX[sLenAccX++] = 'a';
                ptraccX[sLenAccX++] = 'x';
                ptraccX[sLenAccX++] = 'e';
                ptraccX[sLenAccX++] = 's';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';

                ptraccX[sLenAccX++] = ptraccZ[0];
                ptraccX[sLenAccX++] = ptraccZ[1];
                ptraccX[sLenAccX++] = ptraccZ[2];

                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = 'Z';
                ptraccX[sLenAccX++] = '-';
                ptraccX[sLenAccX++] = 'a';
                ptraccX[sLenAccX++] = 'x';
                ptraccX[sLenAccX++] = 'e';
                ptraccX[sLenAccX++] = 's';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';
                ptraccX[sLenAccX++] = ' ';

                //if(g_ucDryerRunning)	// g_ucDryerRunning is set in void ReadAccSensor()
                //{
                    //strcpy((char*)pSlHttpServerResponse->ResponseData.token_value.data,"Running");
                    //pSlHttpServerResponse->ResponseData.token_value.len += strlen("Running");
                	pSlHttpServerResponse->ResponseData.token_value.data = ptraccX;
                	pSlHttpServerResponse->ResponseData.token_value.len += sLenAccX;
                //}
                //else
                //{
                    //strcpy((char*)pSlHttpServerResponse->ResponseData.token_value.data,"Stopped");
                    //pSlHttpServerResponse->ResponseData.token_value.len += strlen("Stopped");
                //}
                	UART_PRINT("\t\t Accelerometer data is sent \n\r");
            }


            /*
             * New UART token *************************************************************************
			*/
            if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data,
            		GET_token_URT, strlen((const char *)GET_token_URT)) == 0)
            {

            	UART_PRINT("\n\r\rExecuting UptimeTask Enter a string and press enter\n\r\r");
            	g_UartHaveCmd=GETChar(&g_ucUARTRecvBuffer[0]); // Returns UART line read from the console

            	ptraccX = g_ucUARTRecvBuffer;
            	pSlHttpServerResponse->ResponseData.token_value.data = ptraccX;	// Pointer to the entered string
            	short sLenAccX = itoa(g_accXIntervalSum,(char*)ptraccX);		//Get length of the sring stored in g_ucUARTRecvBuffer
            	pSlHttpServerResponse->ResponseData.token_value.len += sLenAccX;
            }




        }
            break;

        case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
        {
            unsigned char led, i;
            unsigned long delay = 1000000;
            unsigned char *ptr = pSlHttpServerEvent->EventData.httpPostData.token_name.data; // Get tocken name supplied by the SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT
            UART_PRINT("\n\n\rHTTP_POST Tocken:\t %s", ptr);  // Print UART the tocken name
            //g_ucLEDStatus = 0;
            //memcmp ( buffer1, buffer2, sizeof(buffer1) );
            if(memcmp(ptr, POST_token, strlen((const char *)POST_token)) == 0) //"__SL_P_ULD"
            {
            	// static unsigned char POST_token[] = "__SL_P_ULD";
            	ptr = pSlHttpServerEvent->EventData.httpPostData.token_value.data;
                if(memcmp(ptr, "LED", 3) != 0)
                    break;
                ptr += 3;	// First 3 received characters are "LED"
                led = *ptr;	// has character '1'
                ptr += 2;	// Skips the _ character sent by the web page
                if(led == '1')
                {
                    if(memcmp(ptr, "ON", 2) == 0)
                    {
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                        g_ucLEDStatus = LED_ON;

                    }
                    else if(memcmp(ptr, "Blink", 5) == 0) // com.TI.toggleLED('_Blink');
                    {
                        for (i = 0; i < 50; i++){
                        	GPIO_IF_LedOn(MCU_RED_LED_GPIO);		// Turn On LED
                        	MAP_UtilsDelay(delay);
                        	GPIO_IF_LedOff(MCU_RED_LED_GPIO);		// Turn Off LED
                        	MAP_GPIOPinWrite(GPIOA3_BASE,0x10,0x10);
                        	MAP_UtilsDelay(delay);
                        	MAP_GPIOPinWrite(GPIOA2_BASE,0x40,0x40);
                        	MAP_UtilsDelay(delay);
                        	MAP_GPIOPinWrite(GPIOA3_BASE,0x10,0);
                        	MAP_UtilsDelay(delay);
                        	MAP_GPIOPinWrite(GPIOA2_BASE,0x40,0);
                        	MAP_UtilsDelay(delay);
                        }
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);			// Turn On LED
                        MAP_GPIOPinWrite(GPIOA2_BASE,0x40,0x40);	// LED ON
                        MAP_GPIOPinWrite(GPIOA3_BASE,0x10,0);		// LED OFF
                        g_ucLEDStatus = LED_BLINK;

                    }
                    else
                    {
                        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
                        MAP_GPIOPinWrite(GPIOA3_BASE,0x10,0);
                        MAP_GPIOPinWrite(GPIOA2_BASE,0x40,0);
                        g_ucLEDStatus = LED_OFF;
                    }
                }
                else if(led == '2')
                {

                	if(memcmp(ptr, "ON", 2) == 0)
                    {
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                        g_ucLEDStatus = LED_ON;
                    }
                    else if(memcmp(ptr, "Blink", 5) == 0)
                    {
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                        g_ucLEDStatus = 1;
                        g_ucLEDStatus = LED_BLINK;
                    }
                    else
                    {
                        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
                        g_ucLEDStatus = LED_OFF;
                    }
                }

            }
        }
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(pDevEvent == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    //
    // This application doesn't work w/ socket - Events are not expected
    //
       switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->EventData.status )
            {
                case SL_ECLOSE:
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                               "failed to transmit all queued packets\n\n",
                               pSock->EventData.sd);
                    break;
                default:
                    UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , "
                               "reason (%d) \n\n",
                               pSock->EventData.sd, pSock->EventData.status);
            }
            break;

        default:
            UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n", \
                        pSock->Event);
    }
}

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************

//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeAppVariables()
{
    g_ulStatus = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    g_iInternetAccess = -1;
    g_ucDryerRunning = 0;
    g_uiDeviceModeConfig = ROLE_STA; //default is STA mode
    g_ucLEDStatus = LED_OFF;    
}


//****************************************************************************
//
//! Confgiures the mode in which the device will work
//!
//! \param iMode is the current mode of the device
//!
//!
//! \return   SlWlanMode_t
//!                        
//
//****************************************************************************
static int ConfigureMode(int iMode)
{
    long   lRetVal = -1;

    lRetVal = sl_WlanSetMode(iMode);
    ASSERT_ON_ERROR(lRetVal);

    /* Restart Network processor */
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    // reset status bits
    CLR_STATUS_BIT_ALL(g_ulStatus);

    return sl_Start(NULL,NULL,NULL);
}


//****************************************************************************
//
//!    \brief Connects to the Network in AP or STA Mode - If ForceAP Jumper is
//!                                             Placed, Force it to AP mode
//!
//! \return  0 - Success
//!            -1 - Failure
//
//****************************************************************************
long ConnectToNetwork()
{
    long lRetVal = -1;
    unsigned int uiConnectTimeoutCnt =0;

    // staring simplelink
    lRetVal =  sl_Start(NULL,NULL,NULL);
    ASSERT_ON_ERROR( lRetVal);
    //UART_PRINT("[EXO] We connected, but timed-out waiting for a response. Try again.\r\n");

    // Device is in AP Mode and Force AP Jumper is not Connected
    if(ROLE_STA != lRetVal && g_uiDeviceModeConfig == ROLE_STA )
    {
        if (ROLE_AP == lRetVal)
        {
        	UART_PRINT("\r\nConnectToNetwork: Current AP mode, the jumper is not connected\r\n");
        	// If the device is in AP mode, we need to wait for this event
            // before doing anything 
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
            #ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
            #endif
            }
        }
        //Switch to STA Mode
        lRetVal = ConfigureMode(ROLE_STA);
        UART_PRINT("\r\nConnectToNetwork: Switch to STA mode\r\n");
        ASSERT_ON_ERROR( lRetVal);
    }

    //Device is in STA Mode and Force AP Jumper is Connected
    if(ROLE_AP != lRetVal && g_uiDeviceModeConfig == ROLE_AP )
    {
    	UART_PRINT("\r\nConnectToNetwork: Current ST mode, the jumper is connected\r\n");
    	 //Switch to AP Mode
         lRetVal = ConfigureMode(ROLE_AP);
         UART_PRINT("\r\nConnectToNetwork: Switch to AP mode\r\n");
         ASSERT_ON_ERROR( lRetVal);

    }

    //No Mode Change Required
    if(lRetVal == ROLE_AP)
    {
    	UART_PRINT("\r\nConnectToNetwork: Current AP mode\r\n");
    	//waiting for the AP to acquire IP address from Internal DHCP Server
        // If the device is in AP mode, we need to wait for this event 
        // before doing anything 
        while(!IS_IP_ACQUIRED(g_ulStatus))
        {
        #ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask(); 
        #endif
        }
        //Stop Internal HTTP Server
        lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
        ASSERT_ON_ERROR( lRetVal);

        //Start Internal HTTP Server
        lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
        ASSERT_ON_ERROR( lRetVal);

       char cCount=0;
       
       //Blink LED 3 times to Indicate AP Mode
       for(cCount=0;cCount<3;cCount++)
       {
           //Turn RED LED On
           GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           osi_Sleep(400);
           
           //Turn RED LED Off
           GPIO_IF_LedOff(MCU_RED_LED_GPIO);
           osi_Sleep(400);
       }

       char ssid[32];
	   unsigned short len = 32;
	   unsigned short config_opt = WLAN_AP_OPT_SSID;
	   sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, (unsigned char* )ssid);
	   UART_PRINT("\n\r Connect to : \'%s\'\n\r\n\r",ssid);
    }
    else
    {
    	UART_PRINT("\r\nConnectToNetwork: Current STA mode\r\n");
    	//Stop Internal HTTP Server
        lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
        ASSERT_ON_ERROR( lRetVal);

        //Start Internal HTTP Server
        lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
        ASSERT_ON_ERROR( lRetVal);

    	//waiting for the device to Auto Connect
        while(uiConnectTimeoutCnt<AUTO_CONNECTION_TIMEOUT_COUNT &&
            ((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))) 
        {
            //Turn RED LED On
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            osi_Sleep(50);
            
            //Turn RED LED Off
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            osi_Sleep(50);
            
            uiConnectTimeoutCnt++;
        }
        //Couldn't connect Using Auto Profile
        if(uiConnectTimeoutCnt == AUTO_CONNECTION_TIMEOUT_COUNT)
        {
        	UART_PRINT("\r\nConnectToNetwork: Couldn't connect Using Auto Profile\r\n");
        	//Blink Red LED to Indicate Connection Error
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            
            CLR_STATUS_BIT_ALL(g_ulStatus);

            //Connect Using Smart Config
            lRetVal = SmartConfigConnect();
            UART_PRINT("\r\nConnectToNetwork: Connecting Using Smart Config\r\n");
            ASSERT_ON_ERROR(lRetVal);

            //Waiting for the device to Auto Connect
            while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
            {
                MAP_UtilsDelay(500);              
            }
    }
    //Turn RED LED Off
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

    g_iInternetAccess = ConnectionTest();

    }
    return SUCCESS;
}


//****************************************************************************
//
//!    \brief Read Force AP GPIO and Configure Mode - 1(Access Point Mode)
//!                                                  - 0 (Station Mode)
//!
//! \return                        None
//
//****************************************************************************
static void ReadDeviceConfiguration()
{
    unsigned int uiGPIOPort;
    unsigned char pucGPIOPin;
    unsigned char ucPinValue;
        
    //Read GPIO
    GPIO_IF_GetPortNPin(SH_GPIO_3,&uiGPIOPort,&pucGPIOPin);
    ucPinValue = GPIO_IF_Get(SH_GPIO_3,uiGPIOPort,pucGPIOPin);
        
    //If Connected to VCC, Mode is AP
    if(ucPinValue == 1)
    {
        //AP Mode
        g_uiDeviceModeConfig = ROLE_AP;
    }
    else
    {
        //STA Mode
        g_uiDeviceModeConfig = ROLE_STA;
    }

}


//*****************************************************************************
//
//! AccSample
//!
//!    @brief  Read Accelerometer Data from Sensor to Globals
//!
//!
//!     @return none
//!
//!
//
//*****************************************************************************
void AccSample()
{
    signed char accX,accY,accZ;
    int iRet = -1;
    unsigned long critKey;

    critKey = osi_EnterCritical();

    iRet = BMA222ReadNew(&accX, &accY, &accZ);
    if(iRet)
    {
        //In case of error/ No New Data return
        return;
    }


    g_accXIntervalSum += accX;
    g_accYIntervalSum += accY;
    g_accZIntervalSum += accZ;

    g_accSampleCount++;
    osi_ExitCritical(critKey);
}


//*****************************************************************************
//
//! ReadAccSensor
//!
//!    @brief  Calculate Averages of Accelerometer Globals
//!
//!
//!     @return none
//!
//!
//
//*****************************************************************************
void SetAccAvg()
{
	unsigned long critKey;

    critKey = osi_EnterCritical();
    g_accXAvg = g_accXIntervalSum / g_accSampleCount;
    g_accYAvg = g_accYIntervalSum / g_accSampleCount;
    g_accZAvg = g_accZIntervalSum / g_accSampleCount;
    g_accTotalAvg = (g_accZIntervalSum + g_accYIntervalSum + g_accXIntervalSum ) /
		(g_accSampleCount * 3);

    g_accXIntervalSum = 0;
    g_accYIntervalSum = 0;
    g_accZIntervalSum = 0;
    g_accSampleCount = 0;
    osi_ExitCritical(critKey);

}

//****************************************************************************
//
//!    \brief Exosite Application Main Task - Initializes SimpleLink Driver
//!                                           and Handles HTTP Requests
//! \param[in]                  pvParameters is the data passed to the Task
//!
//! \return                        None
//
//****************************************************************************
static void ExositeTask(void *pvParameters)
{
    long   ret = -1;
    unsigned long long retry_delay = 100;
    unsigned long long counter = 0;
    char buf[1024];
    EXO_STATE state = EXO_STATE_NOT_COMPLETE;

    //Read Device Mode Configuration
    ReadDeviceConfiguration(); // Read Force AP GPIO pin 58 and Configure Mode

    //Connect to Network
    ret = ConnectToNetwork();
    if(ret < 0)
    {
        ERR_PRINT(ret);
        LOOP_FOREVER();
    }

    // Wait here if we're in AP Mode
    while(g_uiDeviceModeConfig != ROLE_STA){
        osi_Sleep(1000);
    }

    //Handle Async Events
    while(1)
    {
    	//TODO: Wrap all Exosite calls in a check to see if we're connected.
        while(state != EXO_STATE_INIT_COMPLETE){
    		UART_PRINT("[EXO] Exosite Init\r\n");
    		state = exosite_init("texasinstruments","cc3200lp_v1");
    		if(state == EXO_STATE_INIT_COMPLETE){
    			UART_PRINT("[EXO] Activated, Ready to Go\r\n");
    		}else if(state == EXO_STATE_INIT_ERROR){
    			UART_PRINT("[EXO] Error Activating on Exosite (Add to Account or Re-enable for Activation)\r\n");
    			retry_delay *= 2;
    	        osi_Sleep(retry_delay);
    		}else if(state == EXO_STATE_CONNECTION_ERROR){
    			UART_PRINT("[EXO] Error Connecting to Exosite, Check Your Connection\r\n");
    			//retry_delay *= 2;
    	        osi_Sleep(retry_delay);
    		}else if(state == EXO_STATE_NO_RESPONSE){
    			UART_PRINT("[EXO] We connected, but timed-out waiting for a response. Try again.\r\n");
    			retry_delay *= 2;
    	        osi_Sleep(retry_delay);
    		}else if(state == EXO_STATE_DEVICE_NOT_ENABLED){
    			UART_PRINT("[EXO] Couldn't Activate on Exosite (Add to Account or Re-enable for Activation)\r\n");
    			retry_delay *= 2;
    	        osi_Sleep(retry_delay);
    		}else if(state == EXO_STATE_R_W_ERROR){
    			UART_PRINT("[EXO] Read/Write Error\r\n");
    			retry_delay *= 2;
    	        osi_Sleep(retry_delay);
    		}else{
    			UART_PRINT("[EXO] Unknown State in Init");
    			return;
    		}
        }


        UART_PRINT("[EXO] Writing Values...");

    	AccSample(); // Just do a single reading for now. TODO: Make Async.
        SetAccAvg();

        float sensorTemp;
        TMP006DrvGetTemp(&sensorTemp);
        /*
         * Write formatted output to sized buffer
         * Composes a string with the same text that would be printed
         * if format was used on printf, but instead of being printed,
         * the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
         * If the resulting string would be longer than n-1 characters, the remaining characters are discarded and not stored,
         * but counted for the value returned by the function.
         * A terminating null character is automatically appended after the content written.
         * After the format parameter, the function expects at least as many additional arguments as needed for format.
         * %.0f - floating point zero decimal places
         */
        snprintf(buf, 1023, "ontime=%llu & acc=%.0f & accX=%.0f & accY=%.0f & accZ=%.0f & sensortemp=%.2f & usrsw=%llu",
                g_uptimeSec,
                g_accTotalAvg,
                g_accXAvg,
                g_accYAvg,
                g_accZAvg,
                sensorTemp,
                g_SW2Counter);					// writes buf
        // %llu - unsigned long long
        ret = exosite_write(buf,strlen(buf));	// Writes the data buf[] in writeData to Exosite. !!!!!!!!!!!!!!!
        if(ret == 0){
        	UART_PRINT("OK\r\n");
        	retry_delay = 1000;
        }else{
        	UART_PRINT("ERROR\r\n");
        	retry_delay *= 2;
        	state = EXO_STATE_NOT_COMPLETE;
        	ERR_PRINT(ret);
        }

        //LED Actions for OOB Code
        if(g_ucLEDStatus == LED_ON)
        {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);		//Turn LED On
        }
        if(g_ucLEDStatus == LED_OFF)
        {
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);		//Turn LED Off
        }
        if(g_ucLEDStatus==LED_BLINK)
        {
        	GPIO_IF_LedToggle(MCU_RED_LED_GPIO);	//Toggle the Led state
        }



        // Limit Retry Delay to 10 Seconds During Development
        if(retry_delay > 10000)
        	retry_delay = 10000;

        osi_Sleep(retry_delay);

        counter++;
    }
}

//*****************************************************************************
//
//! Sample the accelerometer.
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void AccSampleTask( void *pvParameters )
{
	while(1)
	{
		//GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
		//GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
		AccSample();
		//UART_PRINT("Just got a new accelerometer data sample \n\r\r");
		//GPIO_IF_LedToggle(MCU_GREEN_LED_GPIO);
        osi_Sleep(100);
	}
}

//*****************************************************************************
//
//! Inc the uptime counter.
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void UptimeTask( void *pvParameters )
{
	char c = 0;
	while(1)
	{
		//GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
		//GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
		g_uptimeSec++;
		c = MAP_UARTCharGetNonBlocking(CONSOLE);        // Get a single character
		if (c != 0){
			UART_PRINT("\n\r\rExecuting UptimeTask Enter a string and press enter\n\r\r");
			g_UartHaveCmd=GETChar(&g_ucUARTRecvBuffer[0]);
		}
		//GPIO_IF_LedToggle(MCU_ORANGE_LED_GPIO);
		//basic_Interpreter();
		//if(xSemaphoreTake(uart_lock, 1000)) {
		      // Use Guarded Resource
		//            Report("1");
		//            // Give Semaphore back:
		//            xSemaphoreGive(uart_lock);
		//}

		//if((g_UartCmdSent == 1) &&(g_UartHaveCmd ==0))
		//        {
		//            g_UartCmdSent = 0;
		//            memset(g_ucUARTBuffer, 0, sizeof(g_ucUARTBuffer));
		//        }
		//        g_UartHaveCmd=GETChar(&g_ucUARTRecvBuffer[0]);

		//
		// Get the user command line and load acCmdStore[512] buffer.
		//
		//iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));				// Fill acCmdStore[512] buffer. *************** Get COMMAND

		//if(iRetVal < 0)
		//{
		//
		// Error in parsing the command as length is exceeded.
		//
		//	Message("Command length exceeded 512 bytes \n\r");
		//}
		//else if(iRetVal == 0)
		//{
		//
		// No input. Just an enter pressed probably. Display a prompt.
		//
		//}
		//else
		//{

		//}




        osi_Sleep(1000);
	}
}


//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t     CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
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
#endif  //ccs
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif  //ewarm
    
#endif  //USE_TIRTOS
    
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************************************
//                            MAIN FUNCTION										MAIN FUNCTION
//*****************************************************************************************************
void main()
{
    long   lRetVal = -1;
    unsigned int  uiChannel;

    //unsigned int  uiIndex=0;

    //
    // Board Initilization
    //
    BoardInit();
    
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    // Set input pin as a pull up for AP mode
    PinConfigSet(PIN_58,PIN_STRENGTH_2MA|PIN_STRENGTH_4MA,PIN_TYPE_STD_PD);

    // Initialize Global Variables
    InitializeAppVariables();
    
    //
    // UART Init
    //
    InitTerm();
    
    DisplayBanner(APP_NAME);

    //
    // LED Init
    //
    GPIO_IF_LedConfigure(LED1);
      
    //Turn Off the LEDs
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

    //
    // I2C Init	I2C Init I2C Init I2C Init I2C Init I2C Init I2C Init
    //
    lRetVal = I2C_IF_Open(I2C_MASTER_MODE_FST);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //Init Temprature Sensor Init Temprature Sensor Init Temprature Sensor
    lRetVal = TMP006DrvOpen();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //Init Accelerometer Sensor Init Accelerometer Sensor Init Accelerometer Sensor
    lRetVal = BMA222Open();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    
    /*
     * ****************************************************************************
     * 				CONFIGURE ADC CHANNEL		CONFIGURE ADC CHANNEL
     * ****************************************************************************
     */

    //
    // Pinmux for the selected ADC input pin
    //
    MAP_PinTypeADC(PIN_59, PIN_MODE_255);

    uiChannel = ADC_CH_2;

    //
    // Configure ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerConfig(ADC_BASE, 2^17);

    //
    // Enable ADC timer which is used to timestamp the ADC data samples
    //
    MAP_ADCTimerEnable(ADC_BASE);

    //
    // Enable ADC module
    //
    MAP_ADCEnable(ADC_BASE);

    //
    // Enable ADC channel
    //

    MAP_ADCChannelEnable(ADC_BASE, uiChannel);


    // void adc (void) FUNCTION GOES HERE
    //


    MAP_ADCChannelDisable(ADC_BASE, uiChannel);
    //uiIndex = 0;

    // void adcPrint (void) function goes here

    /*
     * ************************************************************************
     * 				START MAIN THREADS		START MAIN THREADS
     * ************************************************************************
     */


    //
    // Simplelinkspawntask
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    
    
    printflag = 0;

    //
    // Create Exosite Task
    //
    lRetVal = osi_TaskCreate(ExositeTask, (signed char*)"ExositeTask", \
                                OSI_STACK_SIZE, NULL, \
                                EXOSITE_TASK_PRIORITY, NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    

    //
    // Create Uptime Task
    //
    lRetVal = osi_TaskCreate(UptimeTask, (signed char*)"UptimeTask", \
                                SMALL_STACK_SIZE, NULL, \
								HIGH_TASK_PRIORITY, NULL );


    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Create Accelerometer Task
    //
    lRetVal = osi_TaskCreate(AccSampleTask, (signed char*)"AccSampleTask", \
                                SMALL_STACK_SIZE, NULL, \
                                LOW_TASK_PRIORITY, NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // create a separate thread for reading UART
    //lRetVal = osi_TaskCreate(basic_Interpreter,(signed char *)"uart", \
    //                            OSI_STACK_SIZE, NULL,
    //                            0, NULL);

    //if(lRetVal < 0)
    //{
    //    ERR_PRINT(lRetVal);
    //    LOOP_FOREVER();
    //}

    //
    // Start OS Scheduler
    //
    osi_start();

    while (1)
    {

    }

}
