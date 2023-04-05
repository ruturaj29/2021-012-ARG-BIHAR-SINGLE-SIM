//F:\Random\Stock\2021-001-ARG-BIHAR-main\DRIVERS\source\gprs.c
#include "PP_Config.h" //F/stock
#if _PP_GSM_GPRS_INIT
/************** ALL header files *****************************/
#include "lpc17xx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdutils.h"
#include "lcd.h"    
#include "gprs.h"
#include "uart.h"
#include "delay.h"
#include "rtc.h" 
#include "convert.h"
#include "extintr.h"
#include "adc.h"
#include "gpio.h"
#include "rain.h"
#include "ATRH.h"
#include "WSWD.h"
#include "Log.h"
#include "lpc17xx_wdt.h"
#include "mmc_176x_ssp.h"
#include "eeprom.h"
//#include "anomometer.h"

/************* Global variable declarations ******************/	

extern bool LCD_Enable;
/************* Uart Extern variables *************************/
extern volatile uint32_t UART1Count;
extern char	UART1Buffer[BUFSIZE];

/********************* main.c variables **********************/
extern rtc_t rtc;								/* rtc structure declare */
extern char iniContent[512];		/* from main.c file */

/********************* log.c variables **********************/
extern int  MaxRain_threshold_val1 , MaxRain_threshold_val2 ;
extern uint32_t   eeprom_address_TH1, eeprom_address_TH2, 
									eeprom_address_master_number, eeprom_address_slave_number1,
                  eeprom_address_slave_number2;
extern char Mobile_no_1[20];
extern char master_Number[20];
extern char slave_Number1[20];
extern char slave_Number2[20];

/******************** SMS variable ***************************/
volatile int buffer_pointer;
volatile int _index = 0;
int position = 0;								/* save location of current message */
bool status_flag = false;				/* for checking any new message */

char Mobile_no[14];							/* store mobile no. of received message */
char message_received[60];			/* save received message */
char SIM_Mobile_no[14];					/* store sim mobile no. of sending message */
char Date_Time[17];  						/* stores Date & Time of Received message */
char _SMS_Buffer[512]= {'\0'};  /* Stores recive SMS and write in SD card */
char master_Mobile_no[14] = {"+918087389414"};

/********************* GPRS.c variables **************************************/
/******** GPS variables ***********/
char latitude_buffer[16]   = { 0 };
char longitude_buffer[16]  = { 0 };
char altitude_buffer[8]    = { 0 };
char Satellites_In_View[3] = { 0 };

/******** GPRS variables ***********/
char imei[16] = { 0 };
char CCID[25] = { 0 };
char SignalQualityIndication[15] = { 0 };
char _sQuality[5]={0};
char *APN;
char *SignalQuality;

/*********** flags decleard ********/
int FixQuality;
uint8_t Response_Status, CRLF_COUNT = 0,responseStatus;
uint16_t Counter = 0;
uint32_t TimeOut = 0;
int ERROR_CODE = 0;
int PDP = 0;
volatile uint32_t APN_Case=0;
//uint32_t ResponseBufferLength;

/**************** Buffer initialize **********/
char URLbuffer[100]={0};  	// Buffer - Stores the received bytes from Sd card ini.txt file
char _apnBuffer[15];
char _ErrorLogBuffer[512] = {'\0'};
char ntwkUrl[100]={0};  		// Buffer - Stores the received bytes from Sd card ini.txt file
//char getClock[30]; 				// save Network clock in to the buffer	 

/************* variables define ***********/
/* Send interval variable for packet to send on this defined variable time*/
char logInterval = 1, PacketlogInterval = 10, PacketsendInterval = 10;
char _logInterval[4] = {'\0'}, _PacketlogInterval[4] = {'\0'},  _PacketsendInterval[4] = {'\0'};
//long logInterval = 1, PacketlogInterval, PacketsendInterval;	
int _SIM_2_FALG = 0;

/*****************************************************************
*       Read_Response [ read AT commands Response ]              *
******************************************************************/
void Read_Response(unsigned int default_timeout)		/* Read response */
{
	static char CRLF_BUF[2];
	static char CRLF_FOUND;
	uint16_t i;
	uint32_t TimeCount = 0, ResponseBufferLength;
		
	while(1)
	{
		if (TimeCount >= (default_timeout+TimeOut))
		{
			CRLF_COUNT = 0; TimeOut = 0;
			Response_Status = MODEM_RESPONSE_TIMEOUT;
			print_DebugMsg("MODEM RESPONSE TIMEOUT\r\n");
			return;
		}

    if (Response_Status == MODEM_RESPONSE_STARTING)
		{
			CRLF_FOUND = 0;
			memset(CRLF_BUF, 0, 2);
			Response_Status = MODEM_RESPONSE_WAITING;
		}
		
    ResponseBufferLength = strlen(UART1Buffer);
      
		if (ResponseBufferLength)
		{
			DELAY_ms(1);
			TimeCount++;
			if (ResponseBufferLength==strlen(UART1Buffer))
	    {
				for (i=0;i<ResponseBufferLength;i++)
				{
					memmove(CRLF_BUF, CRLF_BUF + 1, 1);
					CRLF_BUF[1] = UART1Buffer[i];
					if (!strncmp(CRLF_BUF, "\r\n", 2))
					{
						if (++CRLF_FOUND == (DEFAULT_CRLF_COUNT+CRLF_COUNT))
						{
							CRLF_COUNT = 0; TimeOut = 0;
							Response_Status = MODEM_RESPONSE_FINISHED; 
						//console_log("%s\n\r",UART1Buffer); // print response in terminal
							print_DebugMsg(UART1Buffer);
							return;
						}
					}
				}
				CRLF_FOUND = 0;
			}
		}
		DELAY_ms(1);
		TimeCount++;
  }
}

/*****************************************************************
*   Start_Read_Response [ start to read AT commands Response ]   *
******************************************************************/
void Start_Read_Response(unsigned int default_timeout)
{
	Response_Status = MODEM_RESPONSE_STARTING;
	do {
		Read_Response(default_timeout);
	} while (Response_Status == MODEM_RESPONSE_WAITING);
}

/*****************************************************************
*   Buffer_Flush [ Empty the Recive buffer ]                     *
******************************************************************/
void Buffer_Flush()			/* Flush all variables */
{
	memset(UART1Buffer, 0, BUFSIZE-1);
	UART1Count=0;
}

/*****************************************************************
*   Wait For ExpectedResponse [ wait till response recive ]      *
******************************************************************/
bool WaitForExpectedResponse(char* ExpectedResponse, unsigned int default_timeout)
{
	Buffer_Flush();
	DELAY_ms(200);
	Start_Read_Response(default_timeout);		/* First read response */
	if((Response_Status != MODEM_RESPONSE_TIMEOUT) && (strstr(UART1Buffer, ExpectedResponse) != NULL))
		return true;		/* Return true for success */
	return false;			/* Else return false */
}

/*****************************************************************
*   Wait For Status - 200 [ wait till response recive ]          *
******************************************************************/
bool WaitForStatus200_302(char* ExpectedResponse1, char* ExpectedResponse2 , unsigned int default_timeout)
{
	Buffer_Flush();
	DELAY_ms(20);
	Start_Read_Response(default_timeout);		/* First read response */
	if((Response_Status != MODEM_RESPONSE_TIMEOUT) && (strstr(UART1Buffer, ExpectedResponse1) != NULL) || (strstr(UART1Buffer, ExpectedResponse2) != NULL) )
		return true;		/* Return true for success */
	return false;			/* Else return false */
}

/*****************************************************************
*   Wait For Status - 302 [ wait till response recive ]          *
******************************************************************/
//bool WaitForStatus302(char* ExpectedResponse, unsigned int default_timeout)
//{
//	Buffer_Flush();
//	DELAY_ms(20);
//	Start_Read_Response(default_timeout);		/* First read response */
//	if((Response_Status != MODEM_RESPONSE_TIMEOUT) && (strstr(UART1Buffer, ExpectedResponse) != NULL))
//		return true;		/* Return true for success */
//	return false;			/* Else return false */
//}





/*****************************************************************
*      Send AT Command and wait for Expect Response              *
******************************************************************/
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse, unsigned int Wait_time)
{
	print_DebugMsg(ATCommand);	/* Send AT command to SIM900 */
	print_DebugMsg("\r");
	UART1_TxString(ATCommand);	/* Send AT command to SIM900 */
	UART1_TxChar('\r');
	return WaitForExpectedResponse(ExpectedResponse,Wait_time);
}

/*****************************************************************************************
*                            GSM ON-OFF Functions			    			  											 *
*****************************************************************************************/
/*****************************************************************
*      GSM Module Power Off Function through AT Commands         *
******************************************************************/
void Module_PowerOff(void)
{	
	DELAY_ms(100);
	print_DebugMsg("AT+QPOWD=0\r\n");
	UART1_TxString("AT+QPOWD=0\r"); // To check the LOCAL time Stamp  activated
	WaitForExpectedResponse("OK",1000);
	print_DebugMsg("Module Power Down\r\n");
}

/*************************************************************************************** 
* GSM Module ON [ Regulator ON ]
* startModule_vREG()
* \description  :		This function is used to enable (WRITE LOW) the voltage regulator
										of GSM Module.
* \return       : 	none
****************************************************************************************/
void startModule_vREG( void ) 
{
		print_DebugMsg("Power On module...\r\n");    	
	  GPIO_PinWrite(GSM_VOLTAGE_REG_EN,LOW);     // Turn ON the GSM POWER
	  DELAY_ms(1000); 
}

/*************************************************************************************** 
* GSM Module OFF [ Regulator OFF ]
* ShutDownModule()
* \description  :		This function is used to disable (WRITE HIGH) the voltage regulator
										of GSM Module.
* \return       : 	none
****************************************************************************************/
void ShutDownModule(void)
{
	Module_PowerOff();
	print_DebugMsg("Shutting down module...\r\n");
	GPIO_PinWrite(GSM_VOLTAGE_REG_EN,HIGH);			// Turn OFF the GSM POWER
	DELAY_ms(1000);	
}

/***************************************************************************************
* GSM module Power ON/OFF by PWRKEY
* En_GSM_PWRKEY()
* \description   :		This function is used to enable (WRITE Pulse) GSM Module.
* \return        :  	none
****************************************************************************************/
void En_GSM_PWRKEY( void ) 
{
		print_DebugMsg("Enable GSM module PWRKEY...\r\n");    	
	  GPIO_PinWrite(GSM_MOD_EN,HIGH);		// Turn ON the GSM POWER
		DELAY_ms(920);										// Wait for some time
		GPIO_PinWrite(GSM_MOD_EN,LOW);   	// Turn ON the GSM POWER
		DELAY_ms(200);   									// Wait for some time
}

/**************************************************************************************
* GSM module Sleep mode Activate
* Module_sleep_mode()
* \description   :    This function is used to set GSM Module in sleep.
* \return        :    none
***************************************************************************************/
void Module_sleep_mode(void)
{
	int res;

	print_DebugMsg("MODULE SET SLEEP....\r\n");

	print_DebugMsg("AT+QSCLK=1\r\n");
	UART1_TxString("AT+QSCLK=1\r");					// make low clock enable
	res = WaitForExpectedResponse("OK",3000);
	DELAY_ms(200);

	if(res){
		GPIO_PinWrite(MCU_DTR,HIGH);     			// DTR pin High Sleep mode is on
		print_DebugMsg("MODULE IDLE STATE \r\n");
	}
	else {
		print_DebugMsg("MODULE ACTIVE STATE \r\n");	
		// TO DO
		// CREATE ERROR CODE NUMBER ON DIPSLAY & ERROR FILE.
		// [error] DD/MM/YY: ISSUE XXXX
	}
}

/**************************************************************************************
* GSM module wakeup mode Activate
* Module_awakeup_mode()
* \description   :   	This function is used to wake GSM Module from sleep.
* \return        :		none
***************************************************************************************/
bool Module_awakeup_mode(void)
{
	bool ret,returnStatus;
	
	print_DebugMsg("MODULE ACTIVATE....\r\n");
	GPIO_PinWrite(MCU_DTR,LOW);     		//	DTR pin Low Wakeup mode is on
	DELAY_ms(8000);											//  Wait for 1 sec to module wakeup
	
	print_DebugMsg("AT+QSCLK=0\r\n");
	UART1_TxString("AT+QSCLK=0\r"); 		// make high clock enable
	ret =  WaitForExpectedResponse("OK",3000);
	if(!ret)
	{
		print_DebugMsg("MODULE NOT ACTIVATE PROPERLY...\r\n");
	  ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
		startModule_vREG();		/* MCU GPIO PIN LOW to ON the regulator*/
		DELAY_ms(8000);				/* wait to start module */

		returnStatus = Module_Power_Initializing();
		return returnStatus;
	}
	return ret;
}

/*****************************************************************************************
*                       Initialize GSM Functions			    			  											 *
*****************************************************************************************/
/*****************************************************************
*         modem_initialization [ GSM Initialize ]        				 *
******************************************************************/
bool modem_initialization(void)
{
		
	/* Query Attention */
	if (MODEM_CheckAttention()) 
	{
		responseStatus = GNSS_Connect(APN);		/* Query and SET - Synchronize network time */
		if (responseStatus == GNSS_STILL_POWER_ON) { 
			print_DebugMsg("GPSS Still power ON \r\n");
		}
		if (responseStatus == TIME_SYNCHRONIZE_NOT_COMPLETE) {
			print_DebugMsg("TIME_SYNCHRONIZE_NOT_COMPLETE \r\n");
		}
		
		print_DebugMsg("\r\n");
		print_DebugMsg("AT+QGPSCFG=\"nmeasrc\",1\r");	  // enable nmeasrc functionality
		UART1_TxString("AT+QGPSCFG=\"nmeasrc\",1\r");	  // enable nmeasrc functionality
		WaitForExpectedResponse("OK",3000);
		
		
		
		/* Query sim inserted*/
		if (Query_Sim_Insert()) 
		{
			/* Query commnucaition between module and sim*/
			if (Wait_Till_SimModule_Communication()) 
				return true;
		}
	}
	APN = "NA";
	strcpy(latitude_buffer,"NA");
	strcpy(longitude_buffer,"NA");
	strcpy(Satellites_In_View,"NA");
	strcpy(altitude_buffer,"NA");
	strcpy(SignalQualityIndication,"no ntwrk");
	strcpy(_sQuality,"NA");
  //strcpy(SignalQuality_BER,"99"); 
	return false;
}

/*****************************************************************
*         MODEM_CheckAttention [ check GSM Communication ] 			 *
******************************************************************/
bool MODEM_CheckAttention()			/* Check GSM board Communication */
{
	uint8_t retry, returnStatus = 0;

//	MODEM_SetBuadRate();
	if (SendATandExpectResponse("AT+CMEE?","+CMEE: 1",300) 
			|| (SendATandExpectResponse("AT+CMEE?","+CMEE: 0",300))) 
	{
		print_DebugMsg("AT+CMEE=2\r\n");
		UART1_TxString("AT+CMEE=2\r");  // To set in verbose mode
		WaitForExpectedResponse("OK",1000);
//		return MODEM_SetPhoneFunctionality();
	}
	
	print_DebugMsg("\r\n");
	
	while(1) 
	{
		for (retry=10;retry>0;retry--) 
		{  
			print_DebugMsg("Initialising AT engine...\r\n");
			if(LCD_Enable){
			LCD_Clear();
			LCD_GoToLine(0);LCD_Printf("modem init..");
			}
			
 			WDT_Feed();
			
			if (SendATandExpectResponse("AT","OK",300) && SendATandExpectResponse("ATE1","OK",300))
			{	 	
				print_DebugMsg("AT engine initialised.\r\n");
				if(LCD_Enable){
				LCD_GoToLine(1);LCD_Printf("OK!");
				DELAY_ms(100);
				}
				returnStatus = true;
			}
			if (returnStatus) 
			{
			 	returnStatus = MODEMGetIMEI();
				return returnStatus;
			}

		}/* if number of retry failse */
 
	}
}

/*****************************************************************
*         MODEM Get IMEI [ Get IMEI number ]            				 *
******************************************************************/
bool MODEMGetIMEI()   /* Get IMEI number from Module */
{
	unsigned short int retry,i;
	memset(imei, 0, 16);
  for (retry=5;retry>0;retry--)
	{
		if (SendATandExpectResponse("AT+GSN","OK",6000))
		{
			DELAY_ms(50);
			for(i=0; i<strlen(UART1Buffer); i++)
			{
				if('0' <= UART1Buffer[i] && UART1Buffer[i] <= '9')
				{ //find start index
					strncpy(imei, UART1Buffer+i, 15);	// copy imei from uart buffer
					imei[15] = '\0'; 									// add null character at end
					i = strlen(UART1Buffer); 					// get out from for loop									
					if(LCD_Enable){
					LCD_Clear(); 
				  LCD_GoToLine(0);LCD_Printf("Device IMEI:");
					LCD_GoToLine(1);LCD_Printf("%s",imei);
					DELAY_ms(200);
					}						
					return true;
				}
			}
		}
	}
	return false;
}

/*****************************************************************
*         Query_Sim_Insert [ Check SIM Status ]          				 *
******************************************************************/
//bool Query_Sim_Insert()
//{
//	uint8_t response;

//	response =  Initialize_SIM_1 ();

//	if(response)
//	{
//		_SIM_2_FALG = 0;
//	  return response;
//	}else
//	{
//	  response =  Initialize_SIM_2 ();
//			
//		if(response)
//		{
//		  _SIM_2_FALG = 1;
//		}
//	}
//	
//	return response;

//} 


bool Query_Sim_Insert()
{
    uint8_t response;

    response =  Initialize_SIM_1 ();
    return response;

}

/*****************************************************************
*                    Initialize SMI 1          			          	 *
******************************************************************/
bool Initialize_SIM_1 ()
{
	print_DebugMsg("Checking SIM 1  \r\n");
	uint8_t retry,i;
  WDT_Feed();
	GPIO_PinWrite(SIM_SEL,LOW);      // Set to select SIM1
	DELAY_ms(2000);
	
	if(_SIM_2_FALG)
	{
		ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
	  startModule_vREG();		/* MCU GPIO PIN LOW to ON the regulator*/
	  DELAY_ms(10000);				/* wait to start module */
		
    MODEM_CheckAttention();
		GPS_INITIALIZE();
		DELAY_ms(500);				/* wait to start module */
	
	}
	
	for (retry=5;retry>0;retry--) 
	{ 
		
    if(LCD_Enable){		
		LCD_Clear();
		LCD_GoToLine(0); LCD_Printf("Checking SIM1");
		}
		WDT_Feed();
		print_DebugMsg("Checking SIM 1  \r\n");
		/* Query SIM inserted or not	*/
		if (SendATandExpectResponse("AT+CPIN?","OK",2000))
		{
			print_DebugMsg("SIM 1 Detected!\r\n");
			DELAY_ms(50);

			/* Get 19 digits SIM number */
			if (SendATandExpectResponse("AT+QCCID","OK",2000))
			{
			  DELAY_ms(50);
				for (i=0; i<strlen(UART1Buffer); i++)
				{
					if ('0' <= UART1Buffer[i] && UART1Buffer[i] <= '9')
					{ //find start index
						strncpy(CCID, UART1Buffer+i, 19);	// copy CCID from uart buffer
						CCID[19] = '\0'; 									// add null character at end
						i = strlen(UART1Buffer); 					// get out from for loop									
						//	LCD_Clear(); 
				   	//	LCD_Printf("CCID:%s",CCID);
						console_log("CCID: %s\n\r",CCID); // print CCID in terminal
						return true;
					}
				}
			}
		}
	}
	print_DebugMsg("SIM 1 not found  \r\n");
	strcpy(CCID,"NA");
	if(LCD_Enable){
	LCD_GoToLine(1);LCD_Printf("SIM 1 not found");
	DELAY_ms(2000);	
	}
	return false;	
}

/*****************************************************************
*                    Initialize SMI 2          			          	 *
******************************************************************/
bool Initialize_SIM_2 ()
{
	print_DebugMsg("Checking SIM2  \r\n");
	uint8_t retry,i,returnStatus;
	
	GPIO_PinWrite(SIM_SEL,HIGH);      // Set to select SIM1
	ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
	startModule_vREG();		/* MCU GPIO PIN LOW to ON the regulator*/
	DELAY_ms(10000);				/* wait to start module */
	
  returnStatus = MODEM_CheckAttention();
	
	GPS_INITIALIZE();
	DELAY_ms(500);				/* wait to start module */
	
	
	if(returnStatus){
		for (retry=5;retry>0;retry--) 
		{ 
			
			
			if(LCD_Enable){		
			LCD_Clear();
			LCD_GoToLine(0); LCD_Printf("Checking SIM2");
			}
			WDT_Feed();
			print_DebugMsg("Checking SIM2  \r\n");
			/* Query SIM inserted or not	*/
			if (SendATandExpectResponse("AT+CPIN?","OK",2000))
			{
				print_DebugMsg("SIM 2 Detected!\r\n");
				DELAY_ms(50);

				/* Get 19 digits SIM number */
				if (SendATandExpectResponse("AT+QCCID","OK",2000))
				{
					DELAY_ms(50);
					for (i=0; i<strlen(UART1Buffer); i++)
					{
						if ('0' <= UART1Buffer[i] && UART1Buffer[i] <= '9')
						{ //find start index
							strncpy(CCID, UART1Buffer+i, 19);	// copy CCID from uart buffer
							CCID[19] = '\0'; 									// add null character at end
							i = strlen(UART1Buffer); 					// get out from for loop									
							//	LCD_Clear(); 
							//	LCD_Printf("CCID:%s",CCID);
							console_log("CCID: %s\n\r",CCID); // print CCID in terminal
							return true;
						}
					}
				}
			}
		}
		 
		print_DebugMsg("SIM 2 not found  \r\n");
		strcpy(CCID,"NA");
		if(LCD_Enable){
		LCD_GoToLine(1);LCD_Printf("SIM 2 not found");
		DELAY_ms(2000);	
		}
  }
	return false;
	
}

/*****************************************************************
*  Wait_Till_SimModule_Communication [ Check SIM communication]  *
******************************************************************/
bool Wait_Till_SimModule_Communication(void)
{
	uint8_t retry;
	print_DebugMsg("\r\n");

	for (retry=0;retry<10;retry++)
	{  
		print_DebugMsg("Checking till sim and module have communicated..?\r\n");
		if(LCD_Enable){
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("Communication");
		LCD_GoToLine(1);LCD_Printf("Initializing...");
		DELAY_ms(100);
		}
		
		if (SendATandExpectResponse("AT+QINISTAT","+QINISTAT: 3",2000))
		{	 	
			print_DebugMsg("sim and module have communicated, Done\r\n");
			return true;
		}
		else if (SendATandExpectResponse("AT+QINISTAT","+QINISTAT: 7",2000))
		{	 	
			print_DebugMsg("sim and module have communicated, Done\r\n");
			return true;
		}
	}
	print_DebugMsg("sim and module have communicated, Failed\r\n");
	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("SIM CS Failed");
	}
	return false;
}

/*****************************************************************
*  Query_Set_CLTS [ Check network time synchronization ]         *
******************************************************************/
bool Query_Set_CLTS()   /* Set required commands for Network time */
{
		WDT_Feed();
		print_DebugMsg("AT+CTZU?\r\n");
		UART1_TxString("AT+CTZU?\r"); // Check Network Time Synchronization and update the RTC
		if (!(WaitForExpectedResponse("+CTZU: 3",3000)))  // Check if values are already set then no need to set 
		{
			print_DebugMsg("AT+CTZU=3\r\n");
			UART1_TxString("AT+CTZU=3\r"); // To check the LOCAL time Stamp  activated
			WaitForExpectedResponse("OK",3000);
			return MODEM_SetPhoneFunctionality();
		}
		return true; 
}

/*****************************************************************
*  MODEM_SetPhoneFunctionality [ Check GSM Functionality ]       *
******************************************************************/
bool MODEM_SetPhoneFunctionality()  /* Function to restart the SIM900 with Full Functionality*/
{
	uint8_t retry;
	print_DebugMsg("Set Funtionality...\r\n");
	if(LCD_Enable){
  LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("Funtionality...");
	}
	WDT_Feed();
	
	for (retry=2;retry>0;retry--)
	{
		if(SendATandExpectResponse("AT+CFUN=0","OK",15000)) //resetting RF , you can reset the module also here for first time set up
		{
			print_DebugMsg("++ Minimum ++...\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1);LCD_Printf("+ Minimum +");	
			DELAY_ms(3000);
			}
			return true; 
		}
	   
	  DELAY_sec(3);
		if(SendATandExpectResponse("AT+CFUN=1","OK",15000)) //Set module to Full functionality
		{
			print_DebugMsg("++ Full mode ++...\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1);LCD_Printf("+ Full mode +");		
			DELAY_ms(8000);
			}
			return true; 
		}
	}
	return false;
}

/*****************************************************************
*  MODEM_GetNetworkTime [ Update Date and Time ]                 *
******************************************************************/
bool MODEM_GetNetworkTime(void)			/* Get Time stamp from the module network */
{
	char *const _CLOCK_1 = UART1Buffer;

	print_DebugMsg("AT+CCLK?\r\n");
	UART1_TxString("AT+CCLK?\r"); 	// get the current time
	WaitForExpectedResponse("OK",3000); 

	print_DebugMsg(_CLOCK_1);
	return Update_RTC(_CLOCK_1);
	
	
//	int i;
//	for(i=0; i<strlen(UART1Buffer); i++)
//	{
//		if('0' <= UART1Buffer[i] && UART1Buffer[i] <= '9')
//		{ 
//		//find start index
//			strncpy(getClock, UART1Buffer+i,29);	//copy clock from uart buffer
//			getClock[29] = '\0'; //add null character at end
//			i = strlen(UART1Buffer); //get out from for loop
//		}
//	}	
//	console_log("%s\n\r",getClock); // print clock in terminal
//  Update_RTC(getClock);
//	console_log("%d\n\r",strlen(getClock));
}


/*****************************************************************************************
*             Initialize GPS location tracking Functions			     											 *
*****************************************************************************************/
/*****************************************************************
*      GNSS_PowerON [ Power On  GPS ]                            *
******************************************************************/
bool GNSS_PowerON()			/* GPS Power On */
{
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QGNSSC=1\r");
	UART1_TxString("AT+QGNSSC=1\r");								
	return WaitForExpectedResponse("OK",3000);
}

void GPS_INITIALIZE()
{
	
		responseStatus = GNSS_Connect(APN);		/* Query and SET - Synchronize network time */
		if (responseStatus == GNSS_STILL_POWER_ON) { 
			print_DebugMsg("GPSS Still power ON \r\n");
		}
		if (responseStatus == TIME_SYNCHRONIZE_NOT_COMPLETE) {
			print_DebugMsg("TIME_SYNCHRONIZE_NOT_COMPLETE \r\n");
		}
		
		print_DebugMsg("\r\n");
		print_DebugMsg("AT+QGPSCFG=\"nmeasrc\",1\r");	  // enable nmeasrc functionality
		UART1_TxString("AT+QGPSCFG=\"nmeasrc\",1\r");	  // enable nmeasrc functionality
		WaitForExpectedResponse("OK",3000);
}

/*****************************************************************
*      GNSS_PowerOff [ Power OFF  GPS ]                          *
******************************************************************/
bool GNSS_PowerOff()			/* GPS Power On */
{
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QGPSEND\r");
	UART1_TxString("AT+QGPSEND\r");
	return WaitForExpectedResponse("OK",3000);
}

/*****************************************************************
*      GNSS_Connect [ GPS Configure ]                            *
******************************************************************/
uint8_t GNSS_Connect(char* _APN) /* Connect to GPRS */
{
	uint8_t retry;
	WDT_Feed();
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QGPS?\r");
	UART1_TxString("AT+QGPS?\r");			 //Query GNSS power status.
	if (!WaitForExpectedResponse("+QGPS: 0",3000))	 //GNSS is powered off.
		return GNSS_STILL_POWER_ON;
	DELAY_ms(500);

	print_DebugMsg("\r\n");
	for (retry=2;retry>0;retry--)
	{
		if((SendATandExpectResponse("AT+QGPS=1","OK",2000)))  // GNSS  Module Power ON
		{
			return true;
		}
		DELAY_ms(500);
	}
	DELAY_ms(200);
	return false;
}

/*****************************************************************
*      ReadNavigation [ take GPS location details  ]             *
******************************************************************/
bool ReadNavigation(void) /* take satelite loction */
{ 
   char retry;
	
/*	+QGNSSRD: $GNGLL,1832.1962,N,07350.5544,E,090117.000,A,A*4C	  */

	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("Read NMEA..");
	}
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QGPSCFG=\"nmeasrc\",1\r");	  // enable nmeasrc functionality
	UART1_TxString("AT+QGPSCFG=\"nmeasrc\",1\r");	  
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(3000);
	
/* +QGNSSRD: $GNGGA,100131.000,1832.2018,N,07350.5529,E,1,10,0.91,519.7,M,-64.6,M,,*53  */

 	for (retry=5;retry>0;retry--) 
	{
		WDT_Feed();
		print_DebugMsg("AT+QGPSGNMEA=\"GGA\"\r");	   // lat, long, altitude
		UART1_TxString("AT+QGPSGNMEA=\"GGA\"\r");	  //Read Navigation	
		if (WaitForExpectedResponse("+QGPSGNMEA:",3000)) 
		{	
			//if (strlen(UART1Buffer)>50) {
				store_gps(UART1Buffer);
				if(FixQuality){
					return true;}
//				}
		}
		DELAY_ms(500);
	}
	strcpy(latitude_buffer,"NA");
	strcpy(longitude_buffer,"NA");
	strcpy(altitude_buffer,"NA");
	strcpy(Satellites_In_View,"NA");
	return false;
}

/*****************************************************************************************
*             Initialize PDP context - GPRS connection of the module  									 *
*****************************************************************************************/
/*****************************************************************
*      modemCheck_initialization [ activating PDP  ]             *
******************************************************************/
bool modemCheck_initialization(char* _APN) /* Connect to GPRS */
{
	uint8_t retry, responseStatus;

	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QIDEACT=1\r");	 
	UART1_TxString("AT+QIDEACT=1\r");	   		//To close a GPRS context.
	WaitForExpectedResponse("OK",40000);
	DELAY_ms(100);

	responseStatus = Query_NetworkRegistration();			// Check Network Registration
	if(!responseStatus) return false;
	DELAY_ms(100);

//	responseStatus = Query_GPRSRegistrationStatus();	// check GPRS network Registration 
//	if(!responseStatus) return false;
//	DELAY_ms(100);

	responseStatus = MODEMGetSignalStrength();				// Check Signal Stregnth
	if(!responseStatus) return false;
//	{
//		ERROR_CODE = 6;
//		PDP = 0;
//		return false;
//	}
	DELAY_ms(100);
	
//	APN_Case=0;
	
	for (retry=2; retry > 0; retry--) 
	{
		MODEMGetAPN(); 					/* Get APN */
		DELAY_ms(100);

		print_DebugMsg("\r\n");
		print_DebugMsg("AT+QICSGP=1,1,\"");	 /*Set “your-apn” as APN. Varies per different network */
		DELAY_ms(2);
		print_DebugMsg(APN);
		print_DebugMsg("\"\r");
		UART1_TxString("AT+QICSGP=1,1,\"");
		DELAY_ms(2);
		UART1_TxString(APN);
		UART1_TxString("\"\r");
		WaitForExpectedResponse("OK",2000);
		DELAY_ms(100);
	
		if(LCD_Enable){
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("pdp check..");
		}
		WDT_Feed();
		
		print_DebugMsg("\r\n");
		print_DebugMsg("AT+QIACT=1\r"); //To open a GPRS context.  AT+QIACT=1\r
		UART1_TxString("AT+QIACT=1\r"); //To open a GPRS context.	 AT+QIACT=1\r
		responseStatus = WaitForExpectedResponse("OK",120000);
		
		if(!responseStatus)
		{	
//			APN_Case++;
			PDP = 0;
			ERROR_CODE = 14;
		}
		if(responseStatus)
		{	
			if(LCD_Enable){
			LCD_GoToLine(1);LCD_Printf("OK !");
			}
			PDP =  1;
			return true;
		}
	}/* if number of retry fails */
	RTC_GetDateTime(&rtc);
	print_DebugMsg("PDP Activation fail \r\n");
	memset(_ErrorLogBuffer,0,512);
	sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:GPRS CONN FAIL;",
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
	PDP = 0;
	return false;		
}

/*****************************************************************
*      Query_NetworkRegistration [ check network registration ]  *
******************************************************************/
bool Query_NetworkRegistration()	/* To check Network Registration, can also set*/ 
{
	char retry = 0;//int responseStatus ;//, Check_Status;
	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("Ntwrk serching..");
	}
	/* AT+CREG AT command gives information about the 
	 * registration status and access technology of the 
	 * serving cell	 */
	
	for (retry=15;retry>0;retry--)
	{
		WDT_Feed();
		DELAY_ms(200);
		if (SendATandExpectResponse("AT+CREG?","+CREG: 0,5",300))   
		{
			print_DebugMsg("Network found, roaming !\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1);LCD_Printf("roaming, ok!");
			DELAY_ms(100);
			}
			return true;
		}
		else if (SendATandExpectResponse("AT+CREG?","+CREG: 0,1",300))
		{
			print_DebugMsg("Network found, home !\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1);LCD_Printf("home, ok!");
			DELAY_ms(100);
			}
			return true;
		}
	}/* if number of retry fails */
	print_DebugMsg("Network not found !\r\n");
	
#if _PP_GSM_SIM2	
	
	if(!_SIM_2_FALG)
	{
		print_DebugMsg("Checking for SMI 2 network !\r\n");
		Initialize_SIM_2 ();
	  _SIM_2_FALG=1;
		for (retry=15;retry>0;retry--)
		{
			WDT_Feed();
			DELAY_ms(200);
			if (SendATandExpectResponse("AT+CREG?","+CREG: 0,5",300))   
			{
				print_DebugMsg("Network found, roaming !\r\n");
				if(LCD_Enable){
				LCD_GoToLine(1);LCD_Printf("roaming, ok!");
				DELAY_ms(100);
				}
				return true;
			}
			else if (SendATandExpectResponse("AT+CREG?","+CREG: 0,1",300))
			{
				print_DebugMsg("Network found, home !\r\n");
				if(LCD_Enable){
				LCD_GoToLine(1);LCD_Printf("home, ok!");
				DELAY_ms(100);
				}
				return true;
			}
		}/* if number of retry fails */
	}
	else
	{
		print_DebugMsg("Checking for SMI 1 network !\r\n");
	  Initialize_SIM_1 ();
		_SIM_2_FALG=0;
				for (retry=15;retry>0;retry--)
		{
			WDT_Feed();
			DELAY_ms(200);
			if (SendATandExpectResponse("AT+CREG?","+CREG: 0,5",300))   
			{
				print_DebugMsg("Network found, roaming !\r\n");
				if(LCD_Enable){
				LCD_GoToLine(1);LCD_Printf("roaming, ok!");
				DELAY_ms(100);
				}
				return true;
			}
			else if (SendATandExpectResponse("AT+CREG?","+CREG: 0,1",300))
			{
				print_DebugMsg("Network found, home !\r\n");
				if(LCD_Enable){
				LCD_GoToLine(1);LCD_Printf("home, ok!");
				DELAY_ms(100);
				}
				return true;
			}
		}/* if number of retry fails */
	}
#endif	
	if(LCD_Enable){
	LCD_GoToLine(1); LCD_Printf("Reg Network Err!");
	}
	
	ERROR_CODE = 6; /* network registration failed */
  return false;
}

/*****************************************************************
*  Query_GPRSRegistrationStatus [check GPRS network registration]*
******************************************************************/
bool Query_GPRSRegistrationStatus()	/* To check Network Registration Status */ 
{
	int retry = 0;
	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0); LCD_Printf("GPRS Reg status");
	}
	
	/* AT+CREG AT command gives information about the 
	 * GPRS registration status and access technology of the serving cell	 */
	for (retry=15;retry>0;retry--)
	{
		WDT_Feed();
		DELAY_ms(200);
		
		if (SendATandExpectResponse("AT+CGREG?","+CGREG: 0,5",150000)) 
		{		
			print_DebugMsg("GPRS Connected !\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1); LCD_Printf("connect, ok !");
			}
			return true;
		}
		else if (SendATandExpectResponse("AT+CGREG?","+CGREG: 0,1",150000)) 
		{
			print_DebugMsg("GPRS Connected !\r\n");
			if(LCD_Enable){
			LCD_GoToLine(1); LCD_Printf("connect, ok !");
			}
			return true;
		}
	}/* if number of retry fails */
	
	if(LCD_Enable){
	LCD_GoToLine(1); LCD_Printf("checking...");
	DELAY_ms(1000);
	}
	ERROR_CODE = 9; /* GPRS services not allowed */	 /* ntwk reg not allowed */
	
  return false;
}

/*****************************************************************
*         MODEMGetSignalStrength [check Signal strength ]        *
******************************************************************/
bool MODEMGetSignalStrength()   /* Get Moduel signal quality report */
{
	unsigned short int retry=0,i=0,dec=0;
	
	for (retry=15;retry>0;retry--) 
	{
		if (SendATandExpectResponse("AT+CSQ","OK",3000))
		{
			DELAY_ms(10);
			SignalQuality = strtok(UART1Buffer,": ");
			SignalQuality = strtok(NULL,": ");
			SignalQuality = strtok(SignalQuality, ",");

			strcpy(_sQuality,SignalQuality);
			UART2_Printf("Signal Strength Value: %s\r\n",SignalQuality);
			UART2_Printf("Buffer Signal Strength Value: %s\r\n",_sQuality);

			for (i=0; i<strlen(SignalQuality); i++) 
			{
				dec = dec * 10 + ( SignalQuality[i] - '0' );
			}
			if (dec == 99)	 //not known or not detectable
			{
				strcpy(SignalQualityIndication,"not Avail");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);  
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);	
				}
				RTC_GetDateTime(&rtc);
	  		print_DebugMsg(" Signal strength is NOT DETECTABLE \r\n");
	  		memset(_ErrorLogBuffer,0,512);
				sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Signal strengh is not detectable: %d;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,dec);
	  		Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
			}
			if (dec == 0)
			{
				strcpy(SignalQualityIndication,"poor");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);	  
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				RTC_GetDateTime(&rtc);
				print_DebugMsg(" Signal strength is Poor \r\n");
	  		memset(_ErrorLogBuffer,0,512);
	  		sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Signal strengh is Poor: %d;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,dec);
	  		Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
			}
			if ((dec == 1) && (dec<=5))
			{
				strcpy(SignalQualityIndication,"poor");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality); 
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}					
				RTC_GetDateTime(&rtc);
				print_DebugMsg(" Signal strength is Poor \r\n");
	  		memset(_ErrorLogBuffer,0,512);
	  		sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Signal strengh is Poor: %d;",
					      (uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,dec);
	  		Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
			}
			if (dec == 31)
			{
				strcpy(SignalQualityIndication,"Excellent");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				return true;	
			}
			if ((dec>=6) && (dec<=9)) 
			{
			  strcpy(SignalQualityIndication,"Marginal");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0); LCD_Printf("Signal Qlty: %s",_sQuality);	  
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				RTC_GetDateTime(&rtc);
				print_DebugMsg(" Signal strength is Marginal \r\n");
	  		memset(_ErrorLogBuffer,0,512);
	  		sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Signal strengh is Marginal: %d;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,dec);
	  		Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
				return true;
			}
			if ((dec>=10) && (dec<=14 ))
			{
				strcpy(SignalQualityIndication,"OK");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				return true;	
			}
			if ((dec>=15) && (dec<=19 ))
			{
				strcpy(SignalQualityIndication,"GOOD");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				return true;	
			}
			if ((dec>=20) && (dec<=30 ))
			{
				strcpy(SignalQualityIndication,"Excellent");
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
				LCD_GoToLine(1);LCD_Printf("%s",SignalQualityIndication);
				}
				return true;	
			}	
		}
	}/* if number of retry fails */
	print_DebugMsg("Signal Qlty: No signal\r\n"); // print clock in terminal
	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("Signal Qlty:");
	LCD_GoToLine(1);LCD_Printf("No ntwrk"); 
	}
	strcpy(SignalQualityIndication,"no ntwrk");
	ERROR_CODE = 7; /* Error querring signal strength */
	return false;
}

/*****************************************************************
*         MODEMGetAPN [ get APN for service provider ]           *
******************************************************************/
bool MODEMGetAPN()   /* Get Service provider name for SIM */
{
	unsigned short int retry;

  for (retry=2;retry>0;retry--)
	{ 
		if (SendATandExpectResponse("AT+QSPN","OK",3000))
		{
			DELAY_ms(50);			
			if ((strstr(UART1Buffer, "Vodafone") != NULL))
			{
					if ((strstr(_apnBuffer, "m2misafe") != NULL))
						APN = "m2misafe";
					else if ((strstr(_apnBuffer, "IOT.COM") != NULL))
			  		APN = "IOT.COM";
					else if ((strstr(_apnBuffer, "www") != NULL))
			  		APN = "www";
					else if ((strstr(_apnBuffer, "m2m.skymet.com") != NULL))
			  		APN = "m2m.skymet.com";
					else 
						APN = "m2misafe";
					
				  if(LCD_Enable){
					LCD_Clear();
 	    		LCD_Printf("Operator:\nVodafone");
					}
					
					console_log("APN assigning:%s\n\r",APN);

//				switch(APN_Case){
//					case 0:	APN = "m2misafe";
//					 		 		break;
//					case 1: APN = "www";
//					 		 		break;
//					case 2: APN = "IOT.COM";
//					 		 		break;
//				}	
				return true;
			}	
			else if ((strstr(UART1Buffer, "airtel") != NULL))
			{
				if ((strstr(_apnBuffer, "airteliot.com") != NULL))
					APN = "airteliot.com";
				if ((strstr(_apnBuffer, "AIRTELIOT.COM") != NULL))
					APN = "AIRTELIOT.COM";
				else
			  	APN = "airtelgprs.com";
				
        if(LCD_Enable){
				LCD_Clear();
    		LCD_Printf("Operator:\nAirtel");
				}
				
				console_log("APN assigning:%s\n\r",APN);
				return true;
			}
			else if (strstr(UART1Buffer,"Idea") !=NULL)
			{
				APN = "m2m.skymet.com";
				//APN = "Internet"; 
				if(LCD_Enable){
				LCD_Clear();
    		LCD_Printf("Operator:\nIdea");
				}
			  console_log("APN assigning:%s\n\r",APN);
				return true;		
			}
			else if (strstr(UART1Buffer,"BSNL") !=NULL)
			{
				APN = "bsnlnet";
				if(LCD_Enable){
				LCD_Clear();
    		LCD_Printf("Operator:\nBSNL");
				}
			  console_log("APN assigning:%s\n\r",APN);
				return true;		
			}
			else if (strstr(UART1Buffer,"IOTURL") !=NULL)
			{
				APN = "IOTURL.COM";
				if(LCD_Enable){
				LCD_Clear();
    		LCD_Printf("Operator:\nIOTURL.COM");
				}
			  console_log("APN assigning:%s\n\r",APN);
				return true;		
			}
			else if (strstr(UART1Buffer,"Jio") !=NULL)
			{
				APN = "jiociot";  //Jionet
				if(LCD_Enable){
				LCD_Clear();
    		LCD_Printf("Operator:\nJio 4G");
				}
			  console_log("APN assigning:%s\n\r",APN);
				return true;		
			}
			else
			{
				APN = "NA";
				if(LCD_Enable){
				LCD_Clear();
				LCD_Printf("AT respons \n fails");
				DELAY_ms(2000);
				LCD_Clear();
				LCD_Printf("Operator:\nNA");
				}
				print_DebugMsg("Operator not detected.\r\n");	
			}
			
		}
	}	/* if number of retry fails */
	APN = "NA";
	if(LCD_Enable){
	LCD_Clear();
	LCD_Printf("retry \n fails");
	DELAY_ms(1000);
	LCD_Clear();
	LCD_Printf("Operator:\nNA");
	DELAY_ms(2000);
	}
	print_DebugMsg("Operator not detected.\r\n");
	
	return false;
}


/*****************************************************************************************
*       ALL HTTP server connection and packet sending related function       						 *
*****************************************************************************************/
/*****************************************************************
*         PackOffLog [ send packet to server ]                   *
******************************************************************/
uint8_t PackOffLog(char* ServerPack) /* Function to initiate a packet to the server */
{
	uint8_t responseStatus,retry;

	if (!Check_LocalIP())
	{
		if(LCD_Enable){
		LCD_Clear();
		LCD_Printf("IP assign Failed");
		}
		
		for (retry=2;retry>0;retry--)   // retry module PDP activation
		{
			if (modemCheck_initialization(APN))
			{
				break;
			}
			else {
				return false;
			}
		}
	}
	else {
		responseStatus = HTTP_SetURL(URLbuffer);
		DELAY_ms(50);	
		/* Post the _buffer to the server */
		if(responseStatus)
			responseStatus = HTTP_Post(ServerPack,80,80);
		return responseStatus;
	}
	responseStatus = HTTP_SetURL(URLbuffer);	
	DELAY_ms(50);
	/* Post the _buffer to the server */
	if(responseStatus)
		responseStatus = HTTP_Post(ServerPack,80,80);	
	return responseStatus;	
}

/*****************************************************************
*         Check_LocalIP [ check PDP connection ]                 *
******************************************************************/
/****************** check the local IP ****************************************/
bool Check_LocalIP()			/* Check for connected */
{
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QIACT?\r");
	UART1_TxString("AT+QIACT?\r");
	return WaitForExpectedResponse("+QIACT: 1,1",3000);
}

/*****************************************************************
*         HTTP_SetURL [ set the HTTP URL ]                       *
******************************************************************/
bool HTTP_SetURL(char* url)		/* Set URL */
{
	char _buffer[25];

	print_DebugMsg("AT+QHTTPCFG=\"contextid\",1\r");	  // configure parameters for http server
	UART1_TxString("AT+QHTTPCFG=\"contextid\",1\r");	  
	WaitForExpectedResponse("OK",3000);

	WDT_Feed();
	sprintf(_buffer, "AT+QHTTPURL=%d,30\r", strlen(url));
	print_DebugMsg(_buffer);
	UART1_TxString(_buffer);
	
	if(WaitForExpectedResponse("CONNECT",30000)) {
		return SendATandExpectResponse(url, "OK",30000);
	}
	else
		return false;
}


/*****************************************************************
*         HTTP_Post [ send packet on server ]                    *
******************************************************************/
uint8_t HTTP_Post(char* Parameters, char input_time, char read_time )
{
 	char _buffer[25], PostResponse;
	uint8_t	responseStatus,returnStatus;
	if(LCD_Enable){
	LCD_Clear();
	LCD_GoToLine(0);LCD_Printf("Sending");
	LCD_GoToLine(1);LCD_Printf("Packet...");
  }
	
	WDT_Feed();
	sprintf(_buffer, "AT+QHTTPPOST=%d,%d,%d\r", strlen(Parameters),80,read_time);
	print_DebugMsg(_buffer);
	UART1_TxString(_buffer);
		
	if (WaitForExpectedResponse("CONNECT",120000)) 
	{
		WDT_Feed();
		SendATandExpectResponse(Parameters, "OK",30000);
		if ((strstr(UART1Buffer,",200")) || (strstr(UART1Buffer,",302"))) 
		{
			if(LCD_Enable){
			LCD_Clear();
			LCD_Printf("Post Success 2/3");
			}
			return true;
		}
		
		PostResponse = WaitForStatus200_302(",200",",302",30000);
		
		if (PostResponse) 
		{
			if(LCD_Enable){
			LCD_Clear();
			LCD_Printf("Post Success !!");
			ERROR_CODE = 99;
			}
			return true;
		}
		else 
		{
			if(LCD_Enable){
			LCD_Clear();
			LCD_Printf("Post fail !!");
			print_DebugMsg("Shutting Down Module After Any GPRS Error \n\r");
			ShutDownModule();
			LCD_Clear();
			LCD_Printf("GSM ShutDWN");
			DELAY_ms(500);
			}
			ShutDownModule();
			ERROR_CODE = 702;
			
			//To-Do
			//store actual Error code from GSM response by uart
			// examples 702,703,704,modem response timeout,server busy and all.
			
			memset(_ErrorLogBuffer,0,512);
			RTC_GetDateTime(&rtc);
			sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Post faild due to server not response: %d;",
							(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
	  	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
			
			//To-Do
			//if(strstr(UART1Buffer,"+CME ERROR: 716") || strstr(UART1Buffer,"+CME ERROR: Http socket"))
			//return MODEM_RESPONSE_ERROR;
		}
		if ( strstr(UART1Buffer,"+CME ERROR: 703") || strstr(UART1Buffer,"+CME ERROR: 714") || strstr(UART1Buffer,"+CME ERROR: Http dns error") || strstr(UART1Buffer,"MODEM_RESPONSE_TIMEOUT"))
		{
			print_DebugMsg("Shutting Down Module After Any GPRS Error \n\r");
			ShutDownModule();
			WDT_Feed();
			if(strstr(UART1Buffer,"+CME ERROR: 703"))
			{
				print_DebugMsg("[ERROR]: 703\n\r");	  // enable nmeasrc functionality
				ERROR_CODE = 703;
			}
			if(strstr(UART1Buffer,"+CME ERROR: 714"))
			{
				print_DebugMsg("[ERROR]: 714\n\r");	  // enable nmeasrc functionality
				ERROR_CODE = 714;
			}
			if(strstr(UART1Buffer,"MODEM_RESPONSE_TIMEOUT"))
			{
				print_DebugMsg("[ERROR]: MODEM_RESPONSE_TIMEOUT\n\r");	  // enable nmeasrc functionality
				ERROR_CODE = 702;
			}
			memset(_ErrorLogBuffer,0,512);
			RTC_GetDateTime(&rtc);
	  	sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]: after connect +CME ERROR : %d;",
							(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
	  	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/

			ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
			startModule_vREG();		/* MCU GPIO PIN LOW to ON the regulator*/
			DELAY_ms(10000);				/* wait to start module */

			returnStatus = Module_Power_Initializing();

			if(returnStatus) 
			{
				responseStatus = PackOffLog(Parameters); 	/* Send the packet to the server */

				if((responseStatus == MODEM_RESPONSE_TIMEOUT) 
					||(responseStatus == MODEM_RESPONSE_ERROR)||(responseStatus == 0)) 
				{
					print_DebugMsg("\n\r -------- Packet sending failed --------\n\r");
					if(LCD_Enable){
					LCD_Clear();
					LCD_GoToLine(0);LCD_Printf(" Post failed !");
					LCD_Clear();
					LCD_Printf("GSM ShutDWN");
					DELAY_ms(500);
					ShutDownModule();	
          }						
					memset(_ErrorLogBuffer,0,512);
					RTC_GetDateTime(&rtc);
	  			sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:packet sending failed;",
									(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
								
					Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
					DELAY_ms(1000);
					responseStatus = MODEM_RESPONSE_ERROR;
					return false;
				}
				else
				{
					return true;
				}
			}
			else
			{
				memset(_ErrorLogBuffer,0,512);
				RTC_GetDateTime(&rtc);
				sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Module initialization failed;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
	  		Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
				print_DebugMsg("--Module initialization faild-- \n\r");
				PDP = 0;
				responseStatus = MODEM_RESPONSE_ERROR;
				return false;
			}		
		}
	}
	else if ( strstr(UART1Buffer,"+CME ERROR: 703") || strstr(UART1Buffer,"+CME ERROR: 714") || strstr(UART1Buffer,"+CME ERROR: Http dns error") || strstr(UART1Buffer,"MODEM_RESPONSE_TIMEOUT"))
	{
		ShutDownModule();
		WDT_Feed();  	
		if(strstr(UART1Buffer,"+CME ERROR: 703"))
		{
			print_DebugMsg("[ERROR]: 703\n\r");	  // enable nmeasrc functionality
			 ERROR_CODE = 703;
		}
		if(strstr(UART1Buffer,"+CME ERROR: 714"))
		{
			print_DebugMsg("[ERROR]: 714\n\r");	  // enable nmeasrc functionality
			ERROR_CODE = 714;
		}
		if(strstr(UART1Buffer,"MODEM_RESPONSE_TIMEOUT"))
		{
			print_DebugMsg("[ERROR]: MODEM_RESPONSE_TIMEOUT\n\r");	  // enable nmeasrc functionality
			ERROR_CODE = 702;
		}
	  memset(_ErrorLogBuffer,0,512);
		RTC_GetDateTime(&rtc);
	  sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]: before connect +CME ERROR : %d;",
					  (uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
	  Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/

		ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
		startModule_vREG();		/* MCU GPIO PIN LOW to ON the regulator*/
		DELAY_ms(10000);				/* wait to start module */

		returnStatus = Module_Power_Initializing();

		if(returnStatus) 
		{
			responseStatus = PackOffLog(Parameters); 	/* Send the packet to the server */

			if ((responseStatus == MODEM_RESPONSE_TIMEOUT) 
				 ||(responseStatus == MODEM_RESPONSE_ERROR)||(responseStatus == 0))
			{
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf(" Post failed !");
				}
				print_DebugMsg("\n\r -------- Packet sending failed --------\n\r");
				memset(_ErrorLogBuffer,0,512);
				RTC_GetDateTime(&rtc);
		  	sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:packet sending failed;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
		  	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
				DELAY_ms(1000);
				responseStatus = MODEM_RESPONSE_ERROR;
				return false;
			}
			else	
				return true;
		}
		else
		{
			memset(_ErrorLogBuffer,0,512);
			RTC_GetDateTime(&rtc);
			sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Module initialization faield;",
							(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
	  	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
			print_DebugMsg("--Module initialization faild-- \n\r");
			PDP = 0;
			responseStatus = MODEM_RESPONSE_ERROR;
			return false;
		}
	}

}

/*****************************************************************
*         Check_QIState [ check GPRS connection ]                *
******************************************************************/
bool Check_QIState(void)
{
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QISTATE\r"); 
	UART1_TxString("AT+QISTATE\r"); //Query Connection Status of current access
	return WaitForExpectedResponse("OK",3000);
}

/*****************************************************************
*   Module_PDP_Activation [ Activate PDP context ]               *
******************************************************************/
bool Module_PDP_Activation(void)
{	
	WDT_Feed();
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QIACT=1\r"); /* PDP Activate  */  // AT+QIACT=1\r
	UART1_TxString("AT+QIACT=1\r"); 										 // AT+QIACT=1\r
	return WaitForExpectedResponse("OK",120000);
}

/*****************************************************************
*   Module_deactivateBearerProfile [ Deactivate PDP context ]    *
******************************************************************/
bool Module_deactivateBearerProfile(void)
{
	WDT_Feed();
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+QIDEACT=1\r");	   
	UART1_TxString("AT+QIDEACT=1\r");	   //To close a GPRS context.
	return WaitForExpectedResponse("OK",40000);
}

/*****************************************************************************************
*                     Other extra function                                  						 *
*****************************************************************************************/
/*****************************************************************
*   Module_Initializing [ restart and congigure the GSM module ] *
******************************************************************/
bool Module_Power_Initializing(void)
{
	uint8_t i,returnStatus,responseStatus;

	UART1_EnableBuffer_Interrupt();		/* Enable Buffer RBR UART 1 interrupt*/

/* Excutes the MC60 Initilization related functions */
	
	for (i=2; i>0; i--)
	{
		returnStatus = modem_initialization();
		if (returnStatus) 
			break;	// GSM Module initialize succesfuly - return 1 on succesful
		else 
			return MODEM_RESPONSE_ERROR;
  }
		
	WDT_Feed();
	if (returnStatus)
	{
		print_DebugMsg("Modem Engine Initialization Successful \r\n");
		if(LCD_Enable){
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("Modem Engine");
	  LCD_GoToLine(1);LCD_Printf("Initialized !!");
		DELAY_ms(200);
		}
		
		
		returnStatus = modemCheck_initialization(APN); /* Initialize GPRS commands */
			
		
		WDT_Feed();
		if (returnStatus)
		{
			Module_deactivateBearerProfile();
			/* GNSS Enable and Read */
			responseStatus = GNSS_Connect(APN);	
			if (responseStatus == GNSS_STILL_POWER_ON){
			   print_DebugMsg("GPSS	 Still power ON \r\n");
			}
			if (responseStatus == TIME_SYNCHRONIZE_NOT_COMPLETE){
			   print_DebugMsg("TIME_SYNCHRONIZE_NOT_COMPLETE \r\n");
			}		
			ReadNavigation(); 
			DELAY_ms(200);
			GNSS_PowerOff();	/* Deactivate GNSS module */

			return Module_PDP_Activation();	/* Activate GPRS */		
		}
	}
	else 
	{
		print_DebugMsg("Modem Engine Initialization Failed \r\n");
		if(LCD_Enable){
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("Modem Engine");
	  LCD_GoToLine(1);LCD_Printf("Error Comm.. !!");
		DELAY_ms(2000);
		}
		return MODEM_RESPONSE_ERROR;
	}
	return 0;
}

/*****************************************************************
*      Update_RTC [ update date and time ]                       *
******************************************************************/
bool Update_RTC(char *time)
{
	char *time_update = time;
	char getClock[30];

	char cclk_YEAR[5];
	char cclk_MONTH[3];
	char cclk_DATE[3];
								
	char cclk_HOUR[3];
	char cclk_MINUTE[3];
	char cclk_SECONDS[3];
	
	time_update = time_update+18;
	strcpy(getClock,time_update);
	print_DebugMsg("getclock");
	print_DebugMsg(getClock);
	
	cclk_YEAR[0] = '2';
  cclk_YEAR[1] = '0';
  cclk_YEAR[2] = getClock[1];
  cclk_YEAR[3] = getClock[2];
  cclk_YEAR[4] = '\0';

  cclk_MONTH[0] = getClock[4];
  cclk_MONTH[1] = getClock[5];
  cclk_MONTH[2] = '\0';

  cclk_DATE[0] = getClock[7];
  cclk_DATE[1] = getClock[8];
  cclk_DATE[2] = '\0';

  cclk_HOUR[0] = getClock[10];
  cclk_HOUR[1] = getClock[11];
  cclk_HOUR[2] = '\0';

  cclk_MINUTE[0] = getClock[13];
  cclk_MINUTE[1] = getClock[14];
  cclk_MINUTE[2] = '\0';

  cclk_SECONDS[0] = getClock[16];
  cclk_SECONDS[1] = getClock[17];
  cclk_SECONDS[2] = '\0';

  console_log("%s/%s/%s %s:%s:%s\n\r",cclk_YEAR,cclk_MONTH
   						,cclk_DATE,cclk_HOUR,cclk_MINUTE,cclk_SECONDS); // print response in terminal	
									  
  rtc.hour = ascii_integer(cclk_HOUR); 
  rtc.min =  ascii_integer(cclk_MINUTE);
  rtc.sec =  ascii_integer(cclk_SECONDS);
  rtc.date = ascii_integer(cclk_DATE); 
  rtc.month = ascii_integer(cclk_MONTH);
  rtc.year = ascii_integer(cclk_YEAR);

  if (rtc.year !=2080 && rtc.year>=2021 ) {
  	RTC_SetDateTime(&rtc);
		return true;
  } 
  else
    return false;
}

/*****************************************************************
*      store_gps [  take GPS co-ordinates from Uart buffer ]     *
******************************************************************/
static void store_gps( char *response )
{
	char deg[4] = { 0 }, min[3] = { 0 }, dec[5] = { 0 };
	char FixQul[2]= {0};

	static char str_buffer[100] = { 0 };
  float  latitude, longitude;
//float  altitude;
	
	memset(latitude_buffer , 0, 16);
	memset(longitude_buffer, 0, 16);
	memset(altitude_buffer , 0,  8);

	gsm_gnss_get_param( response, str_buffer, 2 );
	
  strncpy(deg, str_buffer    , 2);
  strncpy(min, str_buffer + 2, 2);
  strncpy(dec, str_buffer + 5, 4);

  latitude = ascii_integer(deg) + (ascii_integer(min) + (ascii_integer(dec) / 10000.0f)) / 60.0f;
  
	gsm_gnss_get_param( response, str_buffer, 3 );

	if (str_buffer[0] == 'S') latitude *= -1.0f;
	
	gsm_gnss_get_param( response, str_buffer, 4 );
											   
  strncpy(deg, str_buffer    , 3);
  strncpy(min, str_buffer + 3, 2);
  strncpy(dec, str_buffer + 6, 4);

  longitude = ascii_integer(deg) + (ascii_integer(min) + ascii_integer(dec) / 10000.0f) / 60.0f;

  gsm_gnss_get_param( response, str_buffer, 5 );

  if(str_buffer[0] == 'W') longitude *= -1.0f;

	gsm_gnss_get_param( response, str_buffer, 6 );
	
	strcpy(FixQul,str_buffer); 
//	console_log("FixQul:%s\n\r",FixQul);

	FixQuality = ascii_integer(FixQul);
//	console_log("FixQul:%d\n\r",FixQuality);

	gsm_gnss_get_param( response, str_buffer, 7 );
	strncpy(Satellites_In_View, str_buffer, 2);

	gsm_gnss_get_param( response, str_buffer, 9 );
	strncpy(altitude_buffer, str_buffer, 7);

	ftoa_signed(latitude, latitude_buffer,4);
  ftoa_signed(longitude, longitude_buffer,4);
	console_log("Latitude:%s  Longitude:%s SatelliteView:%s Altitude:%s\n\r",
			latitude_buffer,longitude_buffer,Satellites_In_View,altitude_buffer);
}

/*****************************************************************
*      gsm_gnss_get_param [ Seperate GPS parameter from String ] *
******************************************************************/
static void gsm_gnss_get_param( char* response, char* out, uint8_t index)
{
    uint8_t i = 0;
    uint8_t j = 0;
//    char* comma;
    char* param = strchr(response, ':') + 2;

    while( index && param[i] != '\r' )
    {
        if(param[i++] == ',')
        {
            index--;
        }
    }

    while( param[i] != ',' && param[i] != '\r')
    {
        out[j++] = param[i++];
    }

    out[j] = '\0'; 
}

/*****************************************************************
*  Wakup_PDP_Activation [ After wakeup from sleep activate PDP ] *
******************************************************************/
//bool Wakup_PDP_Activation(char* APN)
//{
//	char rStatus;
//	rStatus = Query_NetworkRegistration(); // Check Network Registration
//	if (!rStatus) return false;
//	DELAY_ms(100);

//	rStatus = Query_GPRSRegistrationStatus();
//	if (!rStatus) return false;
//	DELAY_ms(100);		

//	print_DebugMsg("\r\n");
//	print_DebugMsg("AT+QICSGP=1,1,\"");	 /*Set “your-apn” as APN. Varies per different network */
//	DELAY_ms(2);
//	print_DebugMsg(APN);
//	print_DebugMsg("\"\r");
//	UART1_TxString("AT+QICSGP=1,1,\"");
//	DELAY_ms(2);
//	UART1_TxString(APN);
//	UART1_TxString("\"\r");
//	WaitForExpectedResponse("OK",2000);
//	DELAY_ms(100);
//	
//	WDT_Feed();
//	print_DebugMsg("\r\n");
//	print_DebugMsg("AT+QIACT=1\r");  // To open a GPRS context.
//	UART1_TxString("AT+QIACT=1\r");  // To open a GPRS context
//	return WaitForExpectedResponse("OK",120000);
//}

/*****************************************************************
*      Remove CRLF and other default strings from response       *
******************************************************************/ 
//void GetResponseBody(char* Response, uint16_t ResponseLength)
//{
//	uint16_t i = 12;
//	uint16_t tmp;
//	char Nav_buffer[65];
//	while(Response[i] != '\r' && i < 100)
//		++i;
//
//	strncpy(Nav_buffer, Response + 12, (i - 12));
//	ResponseLength = ascii_integer(Nav_buffer);
//
//	i += 2;
//	tmp = strlen(Response) - i;
//	memcpy(Response, Response + i, tmp);
//
//	if(!strncmp(Response + tmp - 6, "\r\nOK\r\n", 6))
//	memset(Response + tmp - 6, 0, i + 6);
//}

#endif

/*****************************************************************************************
*                   SMS code start - All SMS functions defined               						 *
*****************************************************************************************/
#if _PP_SMS_INIT
/*****************************************************************
*   SMSDefaultSetting [ set SMS format,storage type, parameter ] *
******************************************************************/
void SMSDefaultSetting(void)
{
	WDT_Feed();
	print_DebugMsg("AT+CMGF=1\r");
	UART1_TxString("AT+CMGF=1\r\n");	/* select message format as text */
	WaitForExpectedResponse("OK",3000);

	print_DebugMsg("AT+CSMP=17,167,0,0\r");
	UART1_TxString("AT+CSMP=17,167,0,0\r\n");	/* set SMS parameter for text mode */
	WaitForExpectedResponse("OK",3000);

	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r"); /* select memory storages to be used for */
	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* reading, writing, receiving */
	WaitForExpectedResponse("OK",3000);								/*(default setting: AT+CPMS="SM","SM","SM")*/

	print_DebugMsg("AT+CSCS=\"GSM\"\r");
	UART1_TxString("AT+CSCS=\"GSM\"\r");
	WaitForExpectedResponse("OK",3000);

}

/*****************************************************************
*   GSM_Msg_Delete [ Delte SMS for Given index number/location ] *
******************************************************************/
void GSM_Msg_Delete(unsigned int position) 
{
	char delete_cmd[20];
	buffer_pointer=0;
	print_DebugMsg("Deleting current SMS\r\n");
	sprintf(delete_cmd,"AT+CMGD=%d\r",position);	/* delete message at specified position */
	print_DebugMsg(delete_cmd);
	UART1_TxString(delete_cmd);
	WaitForExpectedResponse("OK",3000);
}

/*****************************************************************
*        GSM_Delete_All_Msg [ Delet ALL SMS from storage ]       *
******************************************************************/
void GSM_Delete_All_Msg(void) {
	print_DebugMsg("Deleting all SMS\r\n");

/* AT+CMGD=1,4    Delete all messages from <mem1> storage */

//  print_DebugMsg("AT+CMGDA=\"DEL ALL\"\r\n");
//	UART1_TxString("AT+CMGDA=\"DEL ALL\"\r\n");	/* delete all messages of SIM */
//	WaitForExpectedResponse("OK",3000);

	/* Use In SMS text mode */
	print_DebugMsg("AT+CMGD=1,4\r\n");
	UART1_TxString("AT+CMGD=1,4\r\n");	/* delete all messages of SIM */
	WaitForExpectedResponse("OK",3000);	
}

/*****************************************************************
*        GSM_Calling [ call on the given number ]                *
******************************************************************/
void GSM_Calling(char *Mob_no)
{
	char call[20];
	sprintf(call,"ATD%s;\r\n",Mob_no);		
	UART1_TxString(call);		/* send command ATD<Mobile_No>; for calling*/
}

/*****************************************************************
*        GSM_HangCall [ End the on going or incoming call ]      *
******************************************************************/
void GSM_HangCall(void)
{
	UART1_TxString("ATH\r\n");		/*send command ATH\r to hang call*/
}

/*****************************************************************
*        GSM_Wait_for_Msg [ Wait for SMS recive ]                *
******************************************************************/
bool GSM_Wait_for_Msg(void)
{
	char msg_location[4];
	int i;
	buffer_pointer=0;
	DELAY_ms(500);
	
	while(1)
	{
		/*eliminate "\r \n" which is start of string */
		if(UART1Buffer[buffer_pointer]=='\r' || UART1Buffer[buffer_pointer]== '\n')
		{
			buffer_pointer++;
//			print_DebugMsg(" in while \r\n");
		}
		else
		{
//		    print_DebugMsg(" break while \r\n");
			break;
		}
	}
	
  /* "CMTI:" to check if any new message received */
	if(strstr(UART1Buffer,"CMTI:"))		
	{
		while(UART1Buffer[buffer_pointer]!= ',')
		{
			buffer_pointer++;
		}
		buffer_pointer++;
		
		i=0;
		/* copy location of received message where it is stored */
		while(UART1Buffer[buffer_pointer]!= '\r')
		{
			msg_location[i]=UART1Buffer[buffer_pointer];	
			buffer_pointer++;
			i++;
		}

		/* convert string of position to integer value */
		position = atoi(msg_location);
//	    console_log(" MSG Position = %d ",position);
		buffer_pointer=0;
		return true;
	}
	 
	else
	{
		return false;
	}
}

/*****************************************************************
*        GSM_Send_Msg [ Send SMS on given number ]               *
******************************************************************/
bool GSM_Send_Msg(char *num,char *sms)
{
	char sms_buffer[35];
	bool recv_ststus = 0;
	buffer_pointer=0;
	WDT_Feed();
	sprintf(sms_buffer,"AT+CMGS=\"%s\"\r\n",num);
	print_DebugMsg(sms_buffer);
	UART1_TxString(sms_buffer);	/*send command AT+CMGS="Mobile No."\r */
	DELAY_ms(200);
  

	while(1)
	{
		if(UART1Buffer[buffer_pointer]==0x3e)		/* wait for '>' character*/
		{
			buffer_pointer = 0;
			memset(UART1Buffer,0,strlen(UART1Buffer));
			UART1_TxString(sms);		/* send msg to given no. */
			UART1_TxChar(0x1a);		/* send Ctrl+Z then only message will transmit*/
			print_DebugMsg(sms);
			recv_ststus = WaitForExpectedResponse("OK",120000);
			break;
		}
		buffer_pointer++;
	}
	DELAY_ms(300);
	buffer_pointer = 0;
	memset(sms_buffer,0,strlen(sms_buffer));
	if(recv_ststus){
		return true;
	}
	return false;
}


/*****************************************************************
*    GSM_Response_Display [ shows the response from GSM module ] *
******************************************************************/
void GSM_Response_Display(void)
{
	buffer_pointer = 0;
	while(1)
	{
		if(UART1Buffer[buffer_pointer]== '\r' || UART1Buffer[buffer_pointer]== '\n')		/* search for \r\n in string */
		{
			buffer_pointer++;
		}
		else
			break;
	}
	

	while(UART1Buffer[buffer_pointer]!='\r')		/* display response till "\r" */
	{
//		print_DebugMsg(buff[buffer_pointer]);								
		buffer_pointer++;
	}
	buffer_pointer=0;
//	memset(UART1Buffer,0,strlen(UART1Buffer));
}

/*****************************************************************
*    GSM_Msg_Read [ Read the Incoming MSG through index number ] *
******************************************************************/
void GSM_Msg_Read(int position)
{
	char read_cmd[10];
	Buffer_Flush();				//Updated
	sprintf(read_cmd,"AT+CMGR=%d\r\n",position);      //\r\n
	print_DebugMsg(read_cmd);
	UART1_TxString(read_cmd);		/* read message at specified location/position */
	DELAY_ms(25);
	WaitForExpectedResponse("\n+CMGR",3000);
	print_DebugMsg("\r\n ---- print the read SMS on terminal ---- \r\n");
	GSM_Msg_Display();		/* display message */
}

/*****************************************************************
*   GSM_Msg_Display [ Displys the Read SMS and store in SD card ]*
******************************************************************/
void GSM_Msg_Display(void)
{
	int i;
	DELAY_ms(500);

	/*  READ Commands Response
	 *         "[Status]"  "[sender's no.]"   "[   date & Time		]" 			         "[reciver's no.]" 
	 *  +CMGR: "REC READ", "+919890799318 ",  "21/01/29,12:15:58+22", 145, 4, 0, 0,"+919021019335  ", 145, 18
   *  Config;1;24;34;www				<--	"[SMS Body]"
	 *
	 *  OK
	 */
	print_DebugMsg("Before Comparing content of EC20(UART1) Buffer :");
	print_DebugMsg(UART1Buffer);
	print_DebugMsg("\r\n");
	
//	/*------------------Saving SMS in Err Log----------------------*/
//	memset(_ErrorLogBuffer,0,512);	/* Clear the Buffer */
//	RTC_GetDateTime(&rtc);					/* Get RTC time and Date*/
//	/* Combine the string */
//	sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[SMS]:MESSAGE DETAIL: [%s];SMS;",
//	(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,UART1Buffer);
//	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Errorlog - for reference*/
//	/*------------------Saving SMS in Err Log----------------------*/
	
	if(!(strstr(UART1Buffer,",")))		/*check for +CMGR response */  //2.2.2 "UNREAD"
	{
		print_DebugMsg("No message\r\n");
	}
	else
	{
		WDT_Feed();
		buffer_pointer = 0;
		
		while(1)
		{
			if(UART1Buffer[buffer_pointer]=='\r' || UART1Buffer[buffer_pointer]== '\n')		/*wait till \r\n not over*/
			{
				buffer_pointer++;
			}
			else
				break;
		}
		
		/* search for 1st ',' to get mobile no.*/
		while(UART1Buffer[buffer_pointer]!=',')
		{
			buffer_pointer++;
		}
		buffer_pointer = buffer_pointer+2;

		/* extract mobile no. of message sender */
		for(i=0;i<=12;i++)
		{
			Mobile_no[i] = UART1Buffer[buffer_pointer];
			buffer_pointer++;
		}
		print_DebugMsg("sender's mobile no :\r\n");
		print_DebugMsg(Mobile_no);
		print_DebugMsg("\r\n");

//		if(strstr(master_Mobile_no,Mobile_no))
//		{
//			print_DebugMsg("------- master is sender -----\n\r");

		buffer_pointer = buffer_pointer+3;

		i=0;

		for(i=0;i<15;i++)
		{
		  Date_Time[i]=	UART1Buffer[buffer_pointer];
		  buffer_pointer++;

		}
		Date_Time[i]='"';
		 
		print_DebugMsg(" Message arrive time \r\n");
		print_DebugMsg(Date_Time);
		print_DebugMsg("\r\n");

		do
		{
			buffer_pointer++;
		}while(UART1Buffer[buffer_pointer-1]!= '\n');
		
		i=0;

		/* display and save message */
		//while(UART1Buffer[buffer_pointer]!= '\r' && i<31)
		while(UART1Buffer[buffer_pointer]!= '\r' )
		{
			//	print_DebugMsg(buff[buffer_pointer]);
				message_received[i]=UART1Buffer[buffer_pointer];
				buffer_pointer++;
				i++;
		}
		
		buffer_pointer = 0;
		message_received[i] = '\0'; //add null character at end
		print_DebugMsg("\r\n ---- SMS Body ---- \r\n");
		print_DebugMsg(message_received);
		print_DebugMsg("\r\n");
		_index++;
		
		memset(_SMS_Buffer,0,512);
//		sprintf(_SMS_Buffer,"%d;%s;%s;%s",_index,message_received,Mobile_no,Date_Time);
		sprintf(_SMS_Buffer,"[%s];[%s];%s",Date_Time,Mobile_no,message_received);
		Createlog(_SMS_Buffer,"recv_SMS.txt");
		
//	   }//if(strstr(master_Mobile_no,Mobile_no)) loop end
		//	memset(UART1Buffer,0,strlen(UART1Buffer));
//		else{
//			print_DebugMsg("------- master is not sender -----\n\r");
//			print_DebugMsg("------- receive msg is discard -----\n\r");
//			RTC_GetDateTime(&rtc);
//	  	memset(_ErrorLogBuffer,0,512);
//	  	sprintf(_ErrorLogBuffer,"[%02d/%02d/d;%02d:%02d];[Error]: receive msg is discarded ; sender Mob. No.: %s;",
//						  (uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,Mobile_no);
//	  	Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Save log - for reference*/
//	 	}
	}
	status_flag = false;
}

/*****************************************************************
*   Get_SMSC_Number [ Get SIM number ]                           *
******************************************************************/
void Get_SMSC_Number(void)
{
	uint8_t i = 0;
	uint8_t j = 0;
	char* param; 
	
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+CSCA?\r");
	UART1_TxString("AT+CSCA?\r");	  //Read sim number	
	
	if(WaitForExpectedResponse("+CSCA:",3000)) 
	{
		param = strchr(UART1Buffer, ':') + 6;
	
		while( param[i] != '"')
		{
			SIM_Mobile_no[j++] = param[i++];
		}
		SIM_Mobile_no[j] = '\0'; 
	}
	
	print_DebugMsg("current SIM number \r\n");
	print_DebugMsg(SIM_Mobile_no);
}

/*****************************************************************
*  SMS_READ_Setting [ set the SMS read format,storage,parameter ]*
******************************************************************/
void SMS_READ_Setting (void)
{
 	WDT_Feed();
	print_DebugMsg("\r\n");
	print_DebugMsg("AT+CMGF=1\r");
	UART1_TxString("AT+CMGF=1\r");	/* select message format as text */
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* select memory storages to be used for reading, writing, receiving (default setting: AT+CPMS="SM","SM","SM") */
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

	print_DebugMsg("AT+CSDH=1\r");
	UART1_TxString("AT+CSDH=1\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

	print_DebugMsg("AT+CSCS=\"GSM\"\r");
	UART1_TxString("AT+CSCS=\"GSM\"\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

//	print_DebugMsg("AT+CMGR=?\r");
//	UART1_TxString("AT+CMGR=?\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

//	print_DebugMsg("AT+CMGR=0\r");
//	UART1_TxString("AT+CMGR=0\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

//	GSM_Msg_Read(0);
//	DELAY_ms(500);

}

/*****************************************************************
*          readSMS [ Read the incoming SMS ]                     * 
******************************************************************/
//void readSMS(void)
//{
//	char* param;char _MSG_count[3]={'\0'},_Storage_size[3]={'\0'}; 
//	uint8_t i = 0;
//	uint8_t j = 0;
//	int MSG_count=0,MSG_indx=0,Storage_size=0;
//
//	WDT_Feed();
// 	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
//	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* select memory storages to be used for reading, writing, receiving (default setting: AT+CPMS="SM","SM","SM") */
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);
//
//	// +CPMS: 3,20,3,20,3,20
//
//	if (strstr(UART1Buffer,"+CPMS:"))
//	{
//		param = strchr(UART1Buffer, ':') + 2;
//
//		while ( param[i] != ',')
//		{
//			_MSG_count[j++] = param[i++];
//	  }
//		_MSG_count[j] = '\0';
//
//		print_DebugMsg("\r\n ---- no of msg present ---- \r\n");
//		print_DebugMsg(_MSG_count);
//		MSG_count= ascii_integer(_MSG_count);				//	 convert ascii to int	 //
//		console_log("\r\n int count of MSG = %d \r\n",MSG_count);
//
//		i++;
//		j=0;
//
//		while( param[i] != ',')
//		{
//			_Storage_size[j++] = param[i++];
//	  }
//		_Storage_size[j] = '\0';
//
//		print_DebugMsg("\r\n ---- SMS storage size ---- \r\n");
//		print_DebugMsg(_Storage_size);
//		Storage_size= ascii_integer(_Storage_size);				//	 convert ascii to int	 //
//		console_log("\r\n int storage count of MSG = %d \r\n",Storage_size);
//
//		if(MSG_count)
//		{
//		  WDT_Feed();
//			MSG_indx = MSG_count-1;
//			console_log("\r\n int msg index = %d \r\n",MSG_indx); 
//			
//			GSM_Msg_Read(MSG_indx);
//			DELAY_ms(500);
//			 
//			if (strstr(message_received,"http://"))
//			{
//				strcpy(URLbuffer, strtok(message_received, ";"));
//				strcpy(_logInterval, strtok(NULL, ";"));	  						// store log interval from sd card
//				strcpy(_PacketlogInterval, strtok(NULL, ";"));  				// store send interval from sd card
//				strcpy(_PacketsendInterval, strtok(NULL, ";")); 				// store pending log interval from sd card
//
//				logInterval = ascii_integer(_logInterval);							// convert ascii to int	
//				PacketlogInterval = ascii_integer(_PacketlogInterval);	// convert ascii to int	
//				PacketsendInterval = ascii_integer(_PacketsendInterval);// convert ascii to int	
//
//				UART2_Printf("URL:%s \n\rLogInterval:%d \n\rPacketlogInterval:%d \n\rPacketSendInterval:%d\n\r",
//											URLbuffer, logInterval, PacketlogInterval,PacketsendInterval);
//
//				memset(_SMS_Buffer,0,512);
//				sprintf(_SMS_Buffer,"%s|%s|%s|%s",URLbuffer,_logInterval,_PacketlogInterval,_PacketsendInterval);
//
//				/* update the config file through received SMS */
//				smsUpdateConfig(_SMS_Buffer,"INI.txt");
//				GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
//			 
//			}//if(strstr(message_received,"http://")) loop end
//		}//if(MSG_count) loop end
//		
//		if(MSG_count>(Storage_size-2))
//		{
//			 GSM_Delete_All_Msg();
//		}
//
//	}//if(strstr(UART1Buffer,"+CPMS:")) loop end
//}


void readSMS(void)
{
	char* param;char _MSG_count[3]={'\0'},_Storage_size[3]={'\0'} , _SMS_Flag[15]={'\0'}; 
	uint8_t i = 0 , returnStatus;
	uint8_t j = 0;
	int MSG_count=0,MSG_indx=0,Storage_size=0,choice;
	char _MaxRain_threshold_val1[4] = {'\0'}, _MaxRain_threshold_val2[4] = {'\0'};
  int read_TH1,read_TH2;
	char path[20]={'\0'};
	
	WDT_Feed();
 	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* select memory storages to be used for reading, writing, receiving (default setting: AT+CPMS="SM","SM","SM") */
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

	// +CPMS: 3,20,3,20,3,20

	if (strstr(UART1Buffer,"+CPMS:"))
	{
		param = strchr(UART1Buffer, ':') + 2;

		while ( param[i] != ',')
		{
			_MSG_count[j++] = param[i++];
	  }
		_MSG_count[j] = '\0';

		print_DebugMsg("\r\n ---- no of msg present ---- \r\n");
		print_DebugMsg(_MSG_count);
		MSG_count= ascii_integer(_MSG_count);				//	 convert ascii to int	 //
		console_log("\r\n int count of MSG = %d \r\n",MSG_count);

		i++;
		j=0;

		while( param[i] != ',')
		{
			_Storage_size[j++] = param[i++];
	  }
		_Storage_size[j] = '\0';

		print_DebugMsg("\r\n ---- SMS storage size ---- \r\n");
		print_DebugMsg(_Storage_size);
		Storage_size= ascii_integer(_Storage_size);				//	 convert ascii to int	 //
		console_log("\r\n int storage count of MSG = %d \r\n",Storage_size);

		if(MSG_count)
		{
		  WDT_Feed();
			returnStatus = readconfigfile(iniContent);
			if (returnStatus) { /* IF INI file not Found */
				print_DebugMsg(" File Opening Failed \n\r");
	  		LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Config ");
	  		LCD_GoToLine(1);LCD_Printf("file not found.");
	  		DELAY_ms(2000);
			}
			else {
				console_log("iniContent:%s \n\r",iniContent);
				strcpy(URLbuffer, strtok(iniContent , "|"));		// store url from sd card
				strcpy(_logInterval, strtok(NULL, "|"));	  		// store log interval from sd card
				strcpy(_PacketlogInterval, strtok(NULL, "|"));  // store send interval from sd card
				strcpy(_PacketsendInterval, strtok(NULL, "|")); // store pending log interval from sd card
				strcpy(_apnBuffer, strtok(NULL, "|")); // store pending log interval from sd card
			}


			MSG_indx = MSG_count-1;
			console_log("\r\n int msg index = %d \r\n",MSG_indx); 
			
			GSM_Msg_Read(MSG_indx);
			DELAY_ms(500);

			if (strstr(message_received,"http:"))
				choice = 1;
			if (strstr(message_received,"Loginterval"))
				choice = 2;
			if (strstr(message_received,"APN"))
				choice = 3;
			if((strstr(message_received,"Threshold")))
				choice = 4;
			if((strstr(message_received,"Status")))
				choice = 5;
			if((strstr(message_received,"Date")))
				choice = 6;
			if((strstr(message_received,"MasterNUM")))
				choice = 7;
			if((strstr(message_received,"SlaveNUM1")))
				choice = 8;
			if((strstr(message_received,"SlaveNUM2")))
				choice = 9;
			if((strstr(message_received,"WHOU")))
				choice = 10;
			if((strstr(message_received,"WHOA")))
				choice = 11;
			if((strstr(message_received,"WHOI")))
				choice = 12;
			if((strstr(message_received,"WHOM")))
				choice = 13;
			if((strstr(message_received,"WHOS1")))
				choice = 14;
			if((strstr(message_received,"WHOS2")))
				choice = 15;
			

			WDT_Feed();
			switch(choice)						 
			{
				case 1:	strcpy(URLbuffer, strtok(message_received, ";"));			
								UART2_Printf("URL:%s \n\r",URLbuffer);
								
								memset(_SMS_Buffer,0,512);
								sprintf(_SMS_Buffer,"%s|%s|%s|%s|%s",URLbuffer,_logInterval,_PacketlogInterval,_PacketsendInterval,_apnBuffer);
								
								/* update the config file through received SMS */
								smsUpdateConfig(_SMS_Buffer,"INI.txt");
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("URL Update");
	  						DELAY_ms(2000);
                }	
                break;								

				case 2: strcpy(_SMS_Flag, strtok(message_received, ";"));
								strcpy(_logInterval, strtok(NULL, ";"));	  						// store log interval from sd card
								strcpy(_PacketlogInterval, strtok(NULL, ";"));  				// store send interval from sd card
								strcpy(_PacketsendInterval, strtok(NULL, ";")); 				// store pending log interval from sd card

								logInterval = ascii_integer(_logInterval);							// convert ascii to int	
								PacketlogInterval = ascii_integer(_PacketlogInterval);	// convert ascii to int	
								PacketsendInterval = ascii_integer(_PacketsendInterval);// convert ascii to int	

								console_log("Flag:%s \n\r",_SMS_Flag);

								console_log("URL:%s \n\r LogInterval:%d \n\r PacketlogInterval:%d \n\r PacketSendInterval:%d \n\r APN:%s \n\r",
															URLbuffer, logInterval, PacketlogInterval,PacketsendInterval,_apnBuffer);

								memset(_SMS_Buffer,0,512);
								sprintf(_SMS_Buffer,"%s|%s|%s|%s|%s",URLbuffer,_logInterval,_PacketlogInterval,_PacketsendInterval,_apnBuffer);
			
								/* update the config file through received SMS */
								smsUpdateConfig(_SMS_Buffer,"INI.txt");
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
         
				        if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("Interval Update");
	  						DELAY_ms(2000);
								}
								break;

			  case 3: strcpy(_SMS_Flag, strtok(message_received, ";"));
								strcpy(_apnBuffer, strtok(NULL, ";"));
											
								console_log("Flag:%s \n\r APN:%s \n\r",_SMS_Flag,_apnBuffer);
								
								memset(_SMS_Buffer,0,512);
								sprintf(_SMS_Buffer,"%s|%s|%s|%s|%s",URLbuffer,_logInterval,_PacketlogInterval,_PacketsendInterval,_apnBuffer);		

								/* update the config file through received SMS */
								smsUpdateConfig(_SMS_Buffer,"INI.txt");
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */

								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("APN Update");
	  						DELAY_ms(2000);
								}
								break;
								
				case 4: strcpy(_SMS_Flag , strtok(message_received , ";"));
								strcpy(_MaxRain_threshold_val1 , strtok(NULL , ";"));
								strcpy(_MaxRain_threshold_val2 , strtok(NULL , ";"));
								MaxRain_threshold_val1 = ascii_integer(_MaxRain_threshold_val1);				//	/* convert ascii to int	///
								console_log(" Rain 1st Threshold set to : %d \n\r",MaxRain_threshold_val1);
								EEPROM_WriteByte(eeprom_address_TH1, MaxRain_threshold_val1); // Write the data at memoryLocation	0x00
								read_TH1 = EEPROM_ReadByte(eeprom_address_TH1);
								console_log("EEPROM TH1 :%d \n\r",read_TH1);
								
								MaxRain_threshold_val2 = ascii_integer(_MaxRain_threshold_val2);				//	/* convert ascii to int	///
								console_log(" Rain 2nd Threshold set to : %d \n\r",MaxRain_threshold_val2);
								
								
								EEPROM_WriteByte(eeprom_address_TH2, MaxRain_threshold_val2); // Write the data at memoryLocation	0x00
								read_TH2 = EEPROM_ReadByte(eeprom_address_TH2);
								console_log("EEPROM TH2 :%d \n\r",read_TH2);
   
	              GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
								
								if(LCD_Enable){
                LCD_GoToLine(1);LCD_Printf("Threshold set");
	  						DELAY_ms(2000);
								}
								break;
								
				case 5: DL_Status();
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("Status send");
								DELAY_ms(2000);
								}
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
								break;

				case 6: console_log("case 6 \n\r");
								strcpy(_SMS_Flag , strtok(message_received , ";"));
								strcpy(path, strtok(NULL , ";"));
								console_log("path : %s \n\r",path);
								log_SMS(path);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
                break;
								
				case 7: 
								strcpy(_SMS_Flag , strtok(message_received , ";"));
								strcpy(Mobile_no_1, strtok(NULL , ";"));
								console_log("mobile number :%s \n\r",Mobile_no_1);
				        EEPROM_WriteString(eeprom_address_master_number,Mobile_no_1);
				        EEPROM_ReadString(eeprom_address_master_number,master_Number);	// Read the data from memoryLocation 0x02
//								EEPROM_WriteString(eeprom_address_master_number,(uint8_t *)Mobile_no_1);
//				        EEPROM_ReadString(eeprom_address_master_number,(uint8_t *)master_Number);	// Read the data from memoryLocation 0x02
								console_log("Eeprom Read master_Number: %s \n\r",master_Number); //Print the message on UART
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				         
				        if(LCD_Enable){
				        LCD_GoToLine(0);LCD_Printf("master_Number");
				        LCD_GoToLine(1);LCD_Printf("set");
	  						DELAY_ms(2000);
								}
								
                break;
				
			  case 8: 
								strcpy(_SMS_Flag , strtok(message_received , ";"));
								strcpy(Mobile_no_1, strtok(NULL , ";"));
								console_log("mobile number :%s \n\r",Mobile_no_1);
				        EEPROM_WriteString(eeprom_address_slave_number1,Mobile_no_1);
				        EEPROM_ReadString(eeprom_address_slave_number1,slave_Number1);	// Read the data from memoryLocation 0x0D
//								EEPROM_WriteString(eeprom_address_slave_number1,(uint8_t *)Mobile_no_1);
//				        EEPROM_ReadString(eeprom_address_slave_number1,(uint8_t *)slave_Number1);	// Read the data from memoryLocation 0x0D
								console_log("Eeprom Read slave_Number1: %s \n\r",slave_Number1); //Print the message on UART
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				
				        if(LCD_Enable){
				        LCD_GoToLine(0);LCD_Printf("slave_Number1");
				        LCD_GoToLine(1);LCD_Printf("set");
	  						DELAY_ms(2000);
								}
                break;
				
				case 9: 
								strcpy(_SMS_Flag , strtok(message_received , ";"));
								strcpy(Mobile_no_1, strtok(NULL , ";"));
								console_log("mobile number :%s \n\r",Mobile_no_1);
				        EEPROM_WriteString(eeprom_address_slave_number2,Mobile_no_1);
				        EEPROM_ReadString(eeprom_address_slave_number2,slave_Number2);	// Read the data from memoryLocation 0x18
//								EEPROM_WriteString(eeprom_address_slave_number2,(uint8_t *)Mobile_no_1);
//				        EEPROM_ReadString(eeprom_address_slave_number2,(uint8_t *)slave_Number2);	// Read the data from memoryLocation 0x18
								console_log("Eeprom Read slave_Number2: %s \n\r",slave_Number2); //Print the message on UART
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				        
				        if(LCD_Enable){
				        LCD_GoToLine(0);LCD_Printf("slave_Number2");
				        LCD_GoToLine(1);LCD_Printf("set");
	  						DELAY_ms(2000);
								}
                break;
								
				case 10:
								print_DebugMsg("\r\n Case 10 WHOU Executed \r\n");
								//SMSDefaultSetting();
								//GSM_Send_Msg("9890799318",_buffer);
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("CurrentURL:%s Sending to:%s \n\r",URLbuffer,master_Number);  //master_Mobile_no
								GSM_Send_Msg(master_Number,URLbuffer);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
								
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("URL Sent ");
	  						DELAY_ms(2000);
                }
								break;
				case 11:
								print_DebugMsg("\r\n Case 11 WHOA Executed \r\n");
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("CurrentAPN:%s Sending to:%s \n\r",_apnBuffer,master_Number); //master_Mobile_no
								GSM_Send_Msg(master_Number,_apnBuffer);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("APN Sent ");
	  						DELAY_ms(2000);
                }
								break;
				case 12:
								print_DebugMsg("\r\n Case 12 WHOI Executed \r\n");
								memset(_SMS_Buffer,0,512);
								sprintf(_SMS_Buffer,"%s|%s|%s",_logInterval,_PacketlogInterval,_PacketsendInterval);
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("CurrentIntervals:%s Sending to:%s \n\r",_SMS_Buffer,master_Number);  //master_Mobile_no
								GSM_Send_Msg(master_Number,_SMS_Buffer);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
								
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("INTRVL Sent ");
	  						DELAY_ms(2000);
                }
								break;
				case 13:
								print_DebugMsg("\r\n Case 13 WHOM Executed \r\n");
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("Current Master Number:%s Sending to:%s \n\r",master_Number,master_Number); //master_Mobile_no
								GSM_Send_Msg(master_Number,master_Number);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("Master Sent ");
	  						DELAY_ms(2000);
                }
								break;	
				case 14:
								print_DebugMsg("\r\n Case 14 WHOS1 Executed \r\n");
								EEPROM_ReadString(eeprom_address_slave_number1,slave_Number1);
								console_log("Eeprom Read slave_Number1: %s \n\r",slave_Number1);
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("Current Slave Number1:%s Sending to:%s \n\r",slave_Number1,master_Number); ////master_Mobile_no
								GSM_Send_Msg(master_Number,slave_Number1);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("Slave1 Sent ");
	  						DELAY_ms(2000);
                }
								break;
				case 15:
								print_DebugMsg("\r\n Case 15 WHOS2 Executed \r\n");
								EEPROM_ReadString(eeprom_address_slave_number2,slave_Number2);
								console_log("Eeprom Read slave_Number2: %s \n\r",slave_Number2);
								EEPROM_ReadString(eeprom_address_master_number,master_Number);
								console_log("Current Slave Number2:%s Sending to:%s \n\r",slave_Number2,master_Number);  //master_Mobile_no
								GSM_Send_Msg(master_Number,slave_Number2);
								GSM_Msg_Delete(MSG_indx);	/* Dlete the msg */
				
								if(LCD_Enable){
								LCD_GoToLine(1);LCD_Printf("Slave2 Sent ");
	  						DELAY_ms(2000);
                }
								break;
				default:	
					        if(LCD_Enable){
										LCD_GoToLine(1);LCD_Printf("No Update");
										DELAY_ms(2000);
									}
				 					break;
			
			}
		
			 
	
		}//if(MSG_count) loop end
		else {
			
			if(LCD_Enable){
					LCD_Clear();
					LCD_GoToLine(0);
					LCD_Printf("No new SMS ");	
					DELAY_ms(1000);
			}
		}
		
		if(MSG_count>(Storage_size-2))
		{
			 GSM_Delete_All_Msg();
		}

	}//if(strstr(UART1Buffer,"+CPMS:")) loop end
}


/**********************************************************************************
*   configuration of URC setting for RI interrupt [ wakeup for the incoming SMS ] * 
***********************************************************************************/

void Config_Module_for_RI_Interrupt(void)
{
	WDT_Feed();
	print_DebugMsg("AT+QCFG=?\r");
	UART1_TxString("AT+QCFG=?\r");
	WaitForExpectedResponse("OK",8000);
	DELAY_ms(500);
	
	
//	print_DebugMsg("AT+QCFG=\"apready\"\r");
//	UART1_TxString("AT+QCFG=\"apready\"\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);
	
	print_DebugMsg("AT+QCFG=\"apready\",1,0,800\r");
	UART1_TxString("AT+QCFG=\"apready\",1,0,800\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);
	
	print_DebugMsg("AT+QCFG=\"risignaltype\"\r");
	UART1_TxString("AT+QCFG=\"risignaltype\"\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);
	
	print_DebugMsg("AT+QCFG=\"risignaltype\",\"physical\"\r");
	UART1_TxString("AT+QCFG=\"risignaltype\",\"physical\"\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);

//	print_DebugMsg("AT+QCFG=\"urc/ri/smsincoming\"\r");
//	UART1_TxString("AT+QCFG=\"urc/ri/smsincoming\"\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);
	
	print_DebugMsg("AT+QCFG=\"urc/ri/smsincoming\",\"pulse\",500\r");
	UART1_TxString("AT+QCFG=\"urc/ri/smsincoming\",\"pulse\",500\r");
	WaitForExpectedResponse("OK",3000);
	DELAY_ms(500);
	
}

#endif
