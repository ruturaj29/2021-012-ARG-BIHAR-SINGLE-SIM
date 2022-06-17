#ifndef _GPRS_H_
#define _GPRS_H_

/************************************************************************************************* 
																	ALL header files 
**************************************************************************************************/
#include <stdbool.h>
#include "stdutils.h"

/***************************************************************************************************
                             Commonly used macros 
***************************************************************************************************/

//#define DEFAULT_TIMEOUT		15000			/* Define default timeout */
#define DEFAULT_CRLF_COUNT	2				/* Define default CRLF count */

/* Define method */
#define POST			1	 								
#define GET				0

/* Select Demo  */
//#define GET_DEMO									/* Define GET demo */
#define POST_DEMO										/* Define POST demo */
#define USERNAME		""
#define PASSWORD		""

#define _PP_GSM_SIM2 0

/* ALL GSM PIN Configure  */
#define GSM_VOLTAGE_REG_EN		P2_2 	/* Host GPIO enable voltage regulator of GSM Module*/
#define MCU_DTR								P1_21 /* Host GPIO set the module sleep */
#define SIM_SEL								P1_26 /* Host GPIO select/switch dual sim */
#define GSM_MOD_EN						P1_25 /* Host GPIO enable module comm with a pulse sequence */
#define	MCU_WAKEUP_IN					P0_10 /* Host GPIO to wake up the module */
#define MCU_SLEEP_STATUS			P1_24 /* Host GPIO takes input from Module as an alert to wakeup sleep */
#define GSM_RI_INT						P0_11 /* Host GPIO signal to MCU when msg/call arrives */
#define	GSM_RST_MODULE				P1_23 /* Host GPIO to reset the module [Dual sim switch, ntwk unresponsive] */



/***************************************************************************************************
                            Constants and Structures
***************************************************************************************************/
#define GNSS_STILL_POWER_ON 						3
#define TIME_SYNCHRONIZE_NOT_COMPLETE		4

enum MODEM_RESPONSE_STATUS		/* Enumerate response status */
{
	MODEM_RESPONSE_WAITING,
	MODEM_RESPONSE_FINISHED,
	MODEM_RESPONSE_TIMEOUT,
	MODEM_RESPONSE_BUFFER_FULL,
	MODEM_RESPONSE_STARTING,
	MODEM_RESPONSE_ERROR
};

/**************************************************************************************************
                           extern all global variables 
***************************************************************************************************/
extern char URLbuffer[100];  // Buffer - Stores the received bytes from Sd card ini.txt file
extern char imei[16];
extern char latitude_buffer[16],longitude_buffer[16];
extern char altitude_buffer[8], Satellites_In_View[3];
extern char _sQuality[5];
extern char _apnBuffer[15];
extern char ntwkUrl[100];
extern char* APN;
extern int ERROR_CODE;

/* Send interval variable for packet to send on this defined variable time*/
extern	char logInterval, PacketlogInterval, PacketsendInterval;
extern	char _logInterval[4], _PacketlogInterval[4], _PacketsendInterval[4];

/***************************************************************************************************
                             Function Prototypes
***************************************************************************************************/

/****** MODULE START & ENABLE FUNCTIONS ********/
void startModule_vREG ( void ); /* Power on module voltage regulator */
void En_GSM_PWRKEY( void );			/* Enable PWRKEY GSM Module */
void Module_PowerOff(void);			/* Module power off by using AT commands */
void ShutDownModule(void);      /* Power off module voltage regulator */

/************************** INITIALIZATION REALTED ***************************************************/
bool modem_initialization(void); 	/* GSM related functions excutes under this function */
bool MODEM_CheckAttention(void);	/* check GSM module AT response */
bool Query_Sim_Insert(void); 			/* Function to check SIM availaibilty */
bool Wait_Till_SimModule_Communication(void); 

/* wait till it become 3 which means sim and module have 
 * communicated and ready for sms ,call and data , 
 * we can send it multiple times till we get +QINISTAT:3 
 */
 
bool MODEMGetIMEI(void);
bool Query_Set_CLTS(void);
bool MODEM_GetNetworkTime(void);
bool MODEM_SetPhoneFunctionality(void);


/************************** GNSS RELATED **********************************************/
bool GNSS_PowerON(void);
bool GNSS_PowerOff(void);
uint8_t GNSS_Connect(char* _APN); /* Connect to GPRS */
bool ReadNavigation(void); 				/* Read Navigation */


/************************** GPRS INITIALIZATION RELATED **********************************************/
bool modemCheck_initialization(char* _APN);
bool MODEMGetSignalStrength(void);
bool Query_NetworkRegistration(void);
bool Query_GPRSRegistrationStatus(void);
bool MODEMGetAPN(void);

/************************** HTTP POST RELATED **********************************************/
uint8_t PackOffLog(char* ServerPack);
bool Check_LocalIP(void);
bool HTTP_SetURL(char * url);
uint8_t HTTP_Post(char* Parameters,char input_time, char read_time );
bool Check_QIState(void);
bool Module_deactivateBearerProfile(void);
bool Module_PDP_Activation(void);

/*************** AT commands send and Read response related function  *****************************/
bool WaitForExpectedResponse(char* ExpectedResponse,unsigned int default_timeout);
void Read_Response(unsigned int default_timeout);
void Start_Read_Response(unsigned int default_timeout);
void Buffer_Flush(void);
//void GetResponseBody(char* Response, uint16_t ResponseLength);
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse, unsigned int Wait_time);
bool WaitForStatus200_302(char* ExpectedResponse1, char* ExpectedResponse2 , unsigned int default_timeout);
//bool WaitForStatus302(char* ExpectedResponse, unsigned int default_timeout);

/*************************** some extra function ****************************************/
bool Update_RTC(char *time); // Update the RTC  - clock time stamp recieved modem network
static void store_gps( char *response );   // Store GPS information in a buffer
static void gsm_gnss_get_param( char* response, char* out, uint8_t index);

//void LCD_ScrollDisplay(void); // scrolling the screen on LCD

/************************* GSM Sleep mode Function Declaration ****************/
void Module_sleep_mode(void);
bool Module_awakeup_mode(void);
bool Module_Power_Initializing(void);
bool Wakup_PDP_Activation(char* APN);

/************************* SMS Function Declarations **************************/
void SMSDefaultSetting(void);							
void GSM_Calling(char *);
void GSM_HangCall(void);
void GSM_Response(void);
void GSM_Response_Display(void);
void GSM_Msg_Read(int);
bool GSM_Wait_for_Msg(void);
void GSM_Msg_Display(void);
void GSM_Msg_Delete(unsigned int);
bool GSM_Send_Msg(char* , char*);
void GSM_Delete_All_Msg(void);
void Get_SMSC_Number(void);
void SMS_READ_Setting (void);
void readSMS(void);
void Config_Module_for_RI_Interrupt(void);
bool Initialize_SIM_1 (void);
bool Initialize_SIM_2 (void);
void GPS_INITIALIZE(void);

#endif



