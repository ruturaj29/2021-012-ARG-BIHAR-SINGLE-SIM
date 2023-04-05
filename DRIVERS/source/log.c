//F:\Random\Stock\2021-001-ARG-BIHAR-main\DRIVERS\source\log.c
/************************ include Header files *********************************/
#include <lpc17xx.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "adc.h"
#include "WSWD.h"
#include "ATRH.h"
#include "mmc_176x_ssp.h"
#include "gprs.h"
#include "rtc.h"
#include "rain.h"
#include "ff.h"
#include "delay.h"
#include "lcd.h" 
#include "Log.h"

#include "lpc17xx_wdt.h"
#include "extintr.h"
#include "stdutils.h"
#include "convert.h"
#include "anomometer.h"
#include "systick.h"
#include "bod.h"
#include "spi.h"
#include "diskio.h"
#include "eeprom.h"
#include "PP_Config.h"

/************************ Globle variables Define ******************************/
/************ main.c variables *******************/
extern char _buffer[512];
extern rtc_t rtc;	/* rtc structure declare */
//extern char  FirmwareRevision[20];
extern int Card_Failed;
extern char _ErrorLogBuffer[512];
extern uint8_t returnStatus,responseStatus,GPSCheck;
extern int PDP;
/************ Log.c variables *******************/	
int PlogCreate	= 1;
//char FirmwareRevision[] = "7D77E7C0";		// storage for Firmware version or software revision
int  MaxRain_threshold_val1 = 2, MaxRain_threshold_val2 = 5 , rain_val = 0;
char master_Number[20]= {'\0'};
char slave_Number1[20]= {'\0'};
char slave_Number2[20]= {'\0'};
char Mobile_no_1[20] = {'\0'};

char FirmwareRevision[11] = "v0.0.6";
extern bool LCD_Enable;
/* EEPROM variables */
//uint32_t   eeprom_address = 0x00;
uint32_t   eeprom_address_TH1 = 0x00, eeprom_address_TH2 = 0x01,
           eeprom_address_master_number = 0x02 , eeprom_address_slave_number1 = 0x0D,
           eeprom_address_slave_number2 = 0x18 ; //eeprom next write address = 0x23;
/***************************************************************************************
*    Slog [ Creat packet at every Multiple of logInterval mint - 1 mint interval ]     *
****************************************************************************************/
void Slog( void )
{
	print_DebugMsg("\n\r ---- Slog interval ---- \n\r");

	
	#if _PP_READ_BATT_SOLAR_VOLT
	Read_ADCchannels();	/* Read Solar voltage and Battery Voltage */
	#endif
/**************************************************************************************************/
	#if _PP_READ_WINDSENSOR
	
	/* Collect - measure Wind Sensor values - [ULTRASONIC Sensor] */
	if (!Minute_WINDSENSOR_COUNTS) {
		strcpy(DN,"999");												// wind direction minimum
		strcpy(CurrentWindDirection,"999");			// wind direction mean
		strcpy(DX,"999");												// wind direction maximum

		strcpy(SN,"999");												// wind speed minimum
		strcpy(CurrentSpeed,"999");							// wind speed mean
		strcpy(SX,"999");												// wind speed maximum
	}
	Minute_WINDSENSOR_COUNTS = 0;
	#endif 
	
	#if _PP_READ_CUP_ANOMOMETER
	 numRotateWind = 0;				/* Clear wind speed external pulses for next minute count */
	
	/* Collect - measure Wind Sensor values - [CUP-ANEMOMETER Sensor] */
	if (!Cup_Minute_WINDSENSOR_COUNTS) {
		strcpy(Cup_DN,"999");										// wind direction minimum
		strcpy(_Cup_CurrentWindDir,"999");			// wind direction mean
		strcpy(Cup_DX,"999");										// wind direction maximum

		strcpy(Cup_MinSpeed,"999");							// wind speed minimum
		strcpy(_Cup_CurrentWindSpd,"999");			// wind speed mean
		strcpy(Cup_SX,"999");										// wind speed maximum
	}
	Cup_Minute_WINDSENSOR_COUNTS = 0;
 #endif
	
	#if _PP_READ_ATRH
	/* Collect - measure Temperature Sensor values - [ATRH Sensor] */
	if (!Minute_ATRH_COUNTS) {
		strcpy(_Temperature,"-999");						// ATRH Temperature min , max
		strcpy(_Humidity,"-999");								// ATRH Humidity min , max	
	}
	Minute_ATRH_COUNTS = 0;
	#endif
/**************************************************************************************************/
	
	memset(_buffer, 0, 512);	/* Clear the Buffer */

	/* Combine the parameters and store it in _Sbuffer 
	   - for sending it to portal and also storing it to SD card */
	
	/* Previous Packet Format */
//	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%4s;%4s;%4s;%4s;%4s;%4s;%4s;%s;%4s;%4s;%s,0",
//					imei,FirmwareRevision,
//					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
//					(uint16_t)rtc.hour,(uint16_t)rtc.min,
//					_PVolt,_BVolt,
//					_Temperature,_Humidity,
//					CurrentSpeed,SX,SN,
//					CurrentWindDirection,DX,DN,
//					_sQuality);

/**************************************************************************************************/
	#if _PP_ULTRASONIC
	/* Ultra Sonic Sensor Packet Format */
//	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s,0",
//					imei,FirmwareRevision,
//					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
//					(uint16_t)rtc.hour,(uint16_t)rtc.min,
//					_PVolt,_BVolt,
//					_Temperature,_Humidity,
//					CurrentSpeed,SX,SN,
//					CurrentWindDirection,DX,DN,
//					_sQuality);
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s,0",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_Humidity,
					CurrentSpeed,SX,SN,
					CurrentWindDirection,DX,DN,
					_sQuality);
	#endif
					
/**************************************************************************************************/

  #if _PP_CUP_ANOMOMETER
	/* Cup - Anomometer Sensor Packet Format */
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s,0",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_Humidity,
					_Cup_CurrentWindSpd,
					_Cup_CurrentWindDir,
					_sQuality);

	#endif

/**************************************************************************************************/
	#if ARG_BIHAR
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%4s;%4s;0.00;0.00;0.00;00;%s,99;0",			              
					/*#*/imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					/*_Rain,24HR_Rain,Yearly_Rain,
					Rain_detect,*/
					_sQuality/*,99;0*/);
	#endif


/**************************************************************************************************/

	console_log("%s\n\r",_buffer);			/* print response in terminal */	 
	Createlog(_buffer, "slog.txt");			/* Save packet to Slog - for reference */
		
}



/***************************************************************************************************
*    PacketLog [ Creat packet at every Multiple of PacketlogInterval mint - 10 mint interval ]     *
****************************************************************************************************/
void PacketLog ( void )
{
  
	uint8_t i = 0;
	
	print_DebugMsg("\n\r ---- PacketLog interval ---- \n\r");

/**************************************************************************************************/
	#if _PP_AVERAGE_WIND_SAMPLE
	/* Collect - measure Average Wind Sensor values - [ULTRASONIC Sensor]  */
	if (WINDSENSOR_COUNTS)
		Average_WindSamples();												// Average the samples
	else {
		strcpy(DN,"999");															// wind direction minimum
		strcpy(DM,"999");															// wind direction mean
		strcpy(DX,"999");															// wind direction maximum
	
		strcpy(SN,"999");															// wind speed minimum
		strcpy(MinSpeed,"999");												// wind speed minimum
		strcpy(SM,"999");															// wind speed mean
		strcpy(SX,"999");															// wind speed maximum

		strcpy(Gust,"0.00");													// Gust
		strcpy(Tan_Inverse_WindDirection,"0.00");			// average of wind direection
		strcpy(CurrentWindDirection,"999");						// Current wind dierction		
	}
	#endif
	
	#if _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE
	/* Collect - measure Average Wind Sensor values - [CUP-ANOMOMETER Sensor]  */
	if (Cup_WINDSENSOR_COUNTS)
		Cup_Average_WindSamples();										// Average the samples
	else {
		strcpy(Cup_DN,"999");													// wind direction minimum
		strcpy(_Cup_CurrentWindDir,"999");						// wind direction mean
		strcpy(Cup_DX,"999");													// wind direction maximum
	
		strcpy(Cup_SN,"999");													// wind speed minimum
		strcpy(Cup_MinSpeed,"999");										// wind speed minimum
		strcpy(Cup_SM,"999");													// wind speed mean
		strcpy(Cup_SX,"999");													// wind speed maximum

		strcpy(Cup_Gust,"0.00");											// Gust
		strcpy(Cup_Tan_Inverse_WindDirection,"0.00");	// average of wind direection
		strcpy(_Cup_CurrentWindDir,"999");						// Current wind dierction		
	}
	#endif

	#if _PP_AVERAGE_ATRH_SAMPLE
	/* Collect - measure Average Temperature values - [ATRH Sensor] */	  	
	if(ATRH_COUNTS)
		Average_ATRHSamples();	 
	else {
		strcpy(_Temperature,"-999");									// ATRH Temperature Current
		strcpy(_MinTemp,"-999");											// ATRH Temperature Minimum									
		strcpy(_MaxTemp,"-999");											// ATRH Temperature maximum

		strcpy(_Humidity,"-999");											// ATRH Humidity current
		strcpy(_MinHumid,"-999");											// ATRH Humidity Minimum
		strcpy(_MaxHumid,"-999");											// ATRH Humidity Maximum
	}
  #endif
	
/**************************************************************************************************/
	#if _PP_RAIN_FALL
	calcRainFall();							/* Collect - measure Rain gauge (Rain fall value ) */
	#endif
	#if _PP_READ_BATT_SOLAR_VOLT
	Read_ADCchannels();					/* Read Solar voltage and Battery Voltage */
	#endif
/**************************************************************************************************/
	
	memset(_buffer, 0, 512);		/* Clear the Buffer */

	/* Combine the parameters and store it in _buffer 
	   - for sending it to portal and also storing it to SD card */
	
	/* Previous Packet Format */
//	sprintf(_buffer,"#;%s;%s;%2d/%2d/%d;%2d:%2d:00;%4s;%4s;%4s;%4s;%4s;%2s;%2s;%2s;%4s;%4s;%4s;%4s;%4s;%4s;%4s;0.00;0.00;%s,0;0.00;01/01/1970;00:00:00;0",
//					imei,FirmwareRevision,
//					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
//					(uint16_t)rtc.hour,(uint16_t)rtc.min,
//					_PVolt,_BVolt,
//					_Temperature,_MaxTemp,_MinTemp,				  
//					_Humidity,_MaxHumid,_MinHumid,
//					SX,Gust,MinSpeed,SM,
//					CurrentWindDirection,Tan_Inverse_WindDirection,
//					_Rain,_sQuality);

/**************************************************************************************************/
  #if _PP_ULTRASONIC
	/* Ultra Sonic Sensor packet format */
//	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;0.00;0.00;%s,0;0.00;01/01/1970;00:00:00;0",
//					imei,FirmwareRevision,
//					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
//					(uint16_t)rtc.hour,(uint16_t)rtc.min,
//					_PVolt,_BVolt,
//					_Temperature,_MaxTemp,_MinTemp,				  
//					_Humidity,_MaxHumid,_MinHumid,
//					SX,Gust,MinSpeed,SM,
//					CurrentWindDirection,Tan_Inverse_WindDirection,
//					_Rain,_sQuality);
					
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;0.00;0.00;%s,0;0.00;01/01/1970;00:00:00;0",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_MaxTemp,_MinTemp,				  
					_Humidity,_MaxHumid,_MinHumid,
					SX,Gust,MinSpeed,SM,
					CurrentWindDirection,Tan_Inverse_WindDirection,
					_Rain,_sQuality);
					
	#endif
					
/**************************************************************************************************/
  #if _PP_CUP_ANOMOMETER
	/* Cup - Annomometer Sensor packet format */
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;0.00;0.00;%s,0;0.00;01/01/1970;00:00:00;0",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_MaxTemp,_MinTemp,				  
					_Humidity,_MaxHumid,_MinHumid,
					Cup_SX,Cup_Gust,Cup_MinSpeed,Cup_SM,
					_Cup_CurrentWindDir,Cup_Tan_Inverse_WindDirection,
					_Rain,_sQuality);
   #endif

/**************************************************************************************************/
 
   #if ARG_BIHAR
   sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%4s;%4s;%4s;%4s;0.00;00;%s,99;0",			              
					/*#*/imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Rain,_PeriodicRain ,/*24HR_Rain,Yearly_Rain,
					Rain_detect*/
					_sQuality/*,99;0*/);       //24HR_Rain,Yearly_Rain replaced by _PeriodicRain
	 #endif


/**************************************************************************************************/

	console_log("%s\n\r",_buffer);			/* print response in terminal */
	Createlog(_buffer, "pdrv.txt");			/* Save packet to pdrv.txt file - for USB logs */
	returnStatus = Writelog();					/* save packet to pack.txt file*/							
	console_log("returnStatus for Write log : %d \n\r",returnStatus);
	DELAY_ms(100);

/**************************************************************************************************/

	print_DebugMsg("\n\r ---- Rain value ---- \n\r");
	console_log(" rain val : %s \n\r",_Rain);
	console_log(" Periodic rain val : %s \n\r",_PeriodicRain);			
	rain_val = ascii_integer(_Rain);		/* convert ascii to int	*/
	
	console_log(" rain integer value     : %d \n\r",rain_val);
	console_log(" rain threshold value 1 : %d \n\r",MaxRain_threshold_val1);
	console_log(" rain threshold value 2 : %d \n\r",MaxRain_threshold_val2);
	
/**************************************************************************************************/

	if (((Card_Failed)||(returnStatus)||(PacketlogInterval == PacketsendInterval)) && (volt0 > 1.83)) {
		/* if battery voltage greater than 11.50 V then only packet will be send */
		UART1_EnableBuffer_Interrupt();						/* Enable Buffer RBR UART 1 interrupt*/
		returnStatus =	Module_awakeup_mode();		/* GSM module wakeup */

		if (returnStatus ) {	/* GSM module wakeup Successful */
			
		lcd_enable();	/* Specify the LCD type(2x16, 4x16 etc) for initialization [ Enable 16x2 LCD]*/
			
//	print_DebugMsg(" send packets by date log \n\r ");
//	log_SMS("/05-2021/13052021.TXT");
//	DELAY_ms(2000);
	
			
/***********************************************************************************************/
			#if _PP_SMS_INIT
//			/* Send alert MSG if rain excceds it's threshold value */
//			if ( (rain_val >= MaxRain_threshold_val1 ) && (rain_val < MaxRain_threshold_val2) )
//			{
//				print_DebugMsg(" threshold is exceed 1st level..\n\r");
//				SMSDefaultSetting();
//				GSM_Send_Msg(master_Number,"rain value is exceed to its 1st threshold level ");
//				GSM_Send_Msg("9890799318","rain value is exceed to its 1st threshold level ");
////				DELAY_ms(2000);
//			}//if((rain > MaxRain_threshold_val1 )&&(rain < MaxRain_threshold_val2)) loop end
//			else if (	rain_val >= MaxRain_threshold_val2)
//			{
//				print_DebugMsg(" threshold is exceed 2nd level..\n\r");
//				SMSDefaultSetting();
//				GSM_Send_Msg(master_Number,"rain value is exceed to its 2nd threshold level ");
//				GSM_Send_Msg("9890799318","rain value is exceed to its 2nd threshold level ");
////				DELAY_ms(2000);
//			}//if(	rain >= MaxRain_threshold_val2) loop end
//			else
//			{
//				print_DebugMsg(" rain value is in limit..\n\r");
//			}// else loop end

			#endif
/***********************************************************************************************/

			/* Activate PDP context */
			for (i=2;i>0;i--) {
				returnStatus = modemCheck_initialization(APN);
				if (returnStatus) break;
			}
			
			if (returnStatus) { /* Activate PDP context successful */
				responseStatus = PackOffLog(_buffer); /* Send the packet to the server */ 	
				if((responseStatus == MODEM_RESPONSE_TIMEOUT)||
					 (responseStatus == MODEM_RESPONSE_ERROR)||(responseStatus == 0)) 
				{
					if(Check_QIState()) 
					{ /* check PDP context status */
						responseStatus = PackOffLog(_buffer); 	/* Try 2nd time sending packet*/	
					}
				}
			}
			else {	/* Activate PDP context unsuccessful */
				WDT_Feed();

				ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
				startModule_vREG();		  /* MCU GPIO PIN LOW to ON the regulator*/
				DELAY_ms(8000);				/* wait to start module */
		
				responseStatus = Module_Power_Initializing();
				
				if(responseStatus){
					print_DebugMsg("module activate successfully!\r\n");	
					responseStatus = PackOffLog(_buffer); /* Send the packet to the server */
				}
				else
				{
				  LCD_Clear();
					LCD_GoToLine(0);LCD_Printf("PDP Activation..");
			  	LCD_GoToLine(1);LCD_Printf("failed");
					DELAY_ms(200);
		
				 // RTC_GetDateTime(&rtc);
				 //memset(_buffer, 0, 512);
					//sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:MESSAGE DETAIL: %d;PDP failed;",
					//				(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
					//Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
					print_DebugMsg(" PDP activation failed \n\r");
					responseStatus = MODEM_RESPONSE_ERROR;
					print_DebugMsg("\n\r ---- Packet sending failed ---- \n\r");
	
				}	
			}
		
/***********************************************************************************************/
			console_log("MODEM_RESPONSE_TIMEOUT = %d \n\r",MODEM_RESPONSE_TIMEOUT);
			console_log("responseStatus = %d \n\r",responseStatus);
			console_log("MODEM_RESPONSE_ERROR = %d \n\r",MODEM_RESPONSE_ERROR);
			console_log("ERROR_CODE = %d \n\r",ERROR_CODE);
			
			/* Save packet to pending log - if packet not sent succesful */
			if((responseStatus == MODEM_RESPONSE_TIMEOUT) ||
				 (responseStatus == MODEM_RESPONSE_ERROR)||(responseStatus == 0) || ERROR_CODE ==9 
				|| ERROR_CODE == 14 || ERROR_CODE == 6) {
				 
					print_DebugMsg(" write in Plog \n\r");
				  
					/* save packet to pending log file*/
					Createlog(_buffer, "plog.txt");
					memset(_ErrorLogBuffer,0,512);	/* Clear the Buffer */
					RTC_GetDateTime(&rtc);					/* Get RTC time and Date*/
					
					 /* Combine the string */
		//			sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:MESSAGE DETAIL: %d;Post failed;",
		//							(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
		//x			Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Errorlog - for reference*/
					
          if(LCD_Enable){					 
					LCD_Clear();
					LCD_GoToLine(0);LCD_Printf("Post failed !");
					LCD_GoToLine(1);LCD_Printf("Err : %d",ERROR_CODE);
					DELAY_ms(1000);
					ShutDownModule();
					LCD_Clear();
					print_DebugMsg(" ModShtDwn \n\r");
					LCD_GoToLine(0);LCD_Printf("ModShtDwn");
					DELAY_ms(1000);	
					}
					 
					responseStatus = 0;
					PlogCreate = 1;
			}
				 
/***********************************************************************************************/
			
			if ( (responseStatus || returnStatus ) && (PlogCreate == 0)) {
				
				if (Check_LocalIP()) {
					if (Query_Set_CLTS()) {		/* update network date and time */
						MODEM_GetNetworkTime();
					}		
					MODEMGetSignalStrength(); /* Check Signal Stregnth */
					DELAY_ms(100);
				}

				/* GPS coordinates takes at 12PM and 1st time when DL is on*/
//				if (((rtc.hour==12)&&(rtc.min==0)) || (GPSCheck)) {
//					responseStatus = GNSS_Connect(APN);	/* GNSS Enable and Read */
//					if (responseStatus == GNSS_STILL_POWER_ON) {
//			  		print_DebugMsg("GPSS Still power ON \r\n");
//					}
//					if (responseStatus == TIME_SYNCHRONIZE_NOT_COMPLETE) {
//		 				print_DebugMsg("TIME_SYNCHRONIZE_NOT_COMPLETE \r\n");
//					}
//					DELAY_ms(1000);
//					GPSCheck=ReadNavigation();	/* Check GPS co-ordinations */
//					if (GPSCheck) {
//						GNSS_PowerOff();
//						GPSCheck=0;
//					}
//					else {
//						GPSCheck=1;
//					}
//					console_log("GPSCheck: %d\n\r",GPSCheck);
//				}

			/***********************************/
						#if _PP_SMS_INIT
			SMS_READ_Setting();									/* SET SMS setting */
			readSMS();													/* Read SMS */
      #endif
			
			Module_deactivateBearerProfile();		/* Deactivate the PDP context of the module */
			DELAY_ms(100);

			Module_sleep_mode();								/* put GSM module in Sleep mode */	
			/***********************************/	
			}
			
/***********************************************************************************************/

			UART1_DisableBuffer_Interrupt(); 		/* Disable Buffer RBR UART 1 interrupt */
			
			//lcd_disable();
			
/***********************************************************************************************/
			
		}
		else {	/* GSM module wakeup unsuccessful */
			returnStatus = MODEM_RESPONSE_ERROR;
			print_DebugMsg("module Still in Sleep mode,NOT wakeup  \n\r");
			UART1_DisableBuffer_Interrupt(); 		/* Disable Buffer RBR UART 1 interrupt */
		}

	}//if ((Card_Failed)||(PacketlogInterval == PacketsendInterval)) loop end
	
/***********************************************************************************************/
	
	if (((PacketlogInterval != PacketsendInterval)||(returnStatus == MODEM_RESPONSE_ERROR)) /*&& (volt0 < 1.83)*/)
	{
		print_DebugMsg(" write in Plog \n\r");
		
		/* save packet to pending log file*/
		Createlog(_buffer, "plog.txt");
		PlogCreate = 1;
		
	}
	
}


/****************************************************************************************************
*    SendingLog [ Creat packet at every Multiple of PacketsendInterval mint - hourly interval ]     *
*****************************************************************************************************/
void SendingLog ( void )
{

	uint8_t i = 0;

	print_DebugMsg("\n\r ---- SendingLog interval ---- \n\r");

	/* check & only send if plog is available */
	if (PlogCreate  && (volt0 > 1.83)) {
		/*if battery voltage greater than 11.50 V then only packet will be send */
		UART1_EnableBuffer_Interrupt();	/* Enable Buffer RBR UART 1 interrupt*/
		returnStatus =	Module_awakeup_mode();	/* GSM module wakeup */
	
		if (returnStatus ) {	/* GSM module wakeup successful */
	
		lcd_enable();	/* Specify the LCD type(2x16, 4x16 etc) for initialization [ Enable 16x2 LCD]*/
/***********************************************************************************************/
			
			/* Activate PDP context */
			for (i=2;i>0;i--) {
				returnStatus = modemCheck_initialization(APN);
				if (returnStatus) break;
			}
			
			if (returnStatus) {	/* Activate PDP context successful */
				responseStatus = retrylog();
			}
			else {	/* Activate PDP context unsuccessful */
				
				ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
				startModule_vREG();		  /* MCU GPIO PIN LOW to ON the regulator*/
				DELAY_ms(8000);				/* wait to start module */
		
				responseStatus = Module_Power_Initializing();
				
				if(responseStatus){
					print_DebugMsg("module activate successfully!\r\n");	
					responseStatus = retrylog();
				}
				else
				{
				  LCD_Clear();
					LCD_GoToLine(0);LCD_Printf("PDP Activation..");
			  	LCD_GoToLine(1);LCD_Printf("failed");
					DELAY_ms(200);
					
					print_DebugMsg(" PDP activation failed \n\r");
				  memset(_ErrorLogBuffer,0,512);	/* Clear the Buffer */
					RTC_GetDateTime(&rtc);					/* Get RTC time and Date*/
					/* Combine the string */
					sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:MESSAGE DETAIL: %d;PDP failed;",
									(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
					Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Errorlog - for reference*/
					print_DebugMsg("\n\r ---- RetryLog failed ---- \n\r");
				
				}	
			}

/***********************************************************************************************/	
			
			if ((PacketlogInterval != PacketsendInterval)||(!responseStatus)) {
				
				if (Check_LocalIP()) {
					if (Query_Set_CLTS()) {		/* update network date and time */
						MODEM_GetNetworkTime();
					}		
					MODEMGetSignalStrength(); /* Check Signal Stregnth */
					DELAY_ms(100);
				}
	
				/* GPS coordinates takes at 12PM and 1st time when DL is on*/
//				if (((rtc.hour==12)&&(rtc.min==0)) || (GPSCheck)) {
//					responseStatus = GNSS_Connect(APN);	/* GNSS Enable and Read */
//					if (responseStatus == GNSS_STILL_POWER_ON) {
//						print_DebugMsg("GPSS Still power ON \r\n");
//					}
//					if (responseStatus == TIME_SYNCHRONIZE_NOT_COMPLETE) {
//			 			print_DebugMsg("TIME_SYNCHRONIZE_NOT_COMPLETE \r\n");
//					}
//					DELAY_ms(1000);
//					GPSCheck=ReadNavigation();	/* Check GPS co-ordinations */
//					if (GPSCheck) {
//						GNSS_PowerOff();
//						GPSCheck=0;
//					}
//					else {
//						GPSCheck=1;
//					}
//					console_log("GPSCheck: %d\n\r",GPSCheck);
//				}

			}
//			else {
//				
//				memset(_ErrorLogBuffer,0,512);	/* Clear the Buffer */
//				RTC_GetDateTime(&rtc);					/* Get RTC time and Date*/
//				
//				/* Combine the string */
//				sprintf(_ErrorLogBuffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:MESSAGE DETAIL: %d;Post failed;",
//								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
//				Createlog(_ErrorLogBuffer, "err.txt");	/* Save packet to Errorlog - for reference*/
//				
//        if(LCD_Enable){				
//				LCD_Clear();
//				LCD_GoToLine(0);LCD_Printf("Post failed !");
//				LCD_GoToLine(1);LCD_Printf("Err : %d",ERROR_CODE);
//				DELAY_ms(2000);
//				}
//			} 
			
/***********************************************************************************************/
			#if _PP_SMS_INIT
			SMS_READ_Setting();	/* SET SMS setting */
			readSMS();					/* Read SMS */
	    #endif
			
			Module_deactivateBearerProfile();	/* Deactivate the PDP context of the module */
			DELAY_ms(100);
	
			Module_sleep_mode();							/* put GSM module in Sleep mode */	
			UART1_DisableBuffer_Interrupt(); 	/* Disable Buffer RBR UART 1 interrupt */
			
			//lcd_disable();
			
/***********************************************************************************************/
	
		}
		else{	/* GSM module wakeup unsuccessful */
			print_DebugMsg("module Still in Sleep mode,NOT wakeup  \n\r");
			UART1_DisableBuffer_Interrupt(); 	/* Disable Buffer RBR UART 1 interrupt */
		}
		
	}
	else
	{
		print_DebugMsg(" Plog not created ....\n\r");
	}

}


/***************************************************************************************
*    DL Status Function [ creat DL Status for send SMS ]                               *
****************************************************************************************/
void DL_Status( void )
{
	print_DebugMsg("\n\r ---- check Datalogger Status ---- \n\r");
	
/***********************************************************************************************/
	/* read sensor values */
	#if _PP_READ_BATT_SOLAR_VOLT
	Read_ADCchannels();					/* Read Solar voltage and Battery Voltage */
	#endif
	#if _PP_READ_ATRH
	readATRH(ATRH_Write_ADD,ATRH_READ_ADD,Temperature_register_add);
  #endif	
	#if _PP_READ_WINDSENSOR
	Read_WindSensor();					/* Read Wind sensor [ Ultrasonic] */
	#endif
	#if _PP_READ_CUP_ANOMOMETER
	Cup_Read_WindSensor();			/* Read Wind sensor [ Cup-Anomometer] */
	#endif
	
/***********************************************************************************************/
	
	#if _PP_READ_WINDSENSOR
	/* Collect - measure Wind Sensor values - [ULTRASONIC Sensor] */
	if (!Minute_WINDSENSOR_COUNTS) {
		strcpy(DN,"999");												// wind direction minimum
		strcpy(CurrentWindDirection,"999");			// wind direction mean
		strcpy(DX,"999");												// wind direction maximum

		strcpy(SN,"999");												// wind speed minimum
		strcpy(CurrentSpeed,"999");							// wind speed mean
		strcpy(SX,"999");												// wind speed maximum
	}
	Minute_WINDSENSOR_COUNTS = 0;
	#endif
	
	#if _PP_READ_CUP_ANOMOMETER
	/* Collect - measure Wind Sensor values - [CUP-ANEMOMETER Sensor] */
	if (!Cup_Minute_WINDSENSOR_COUNTS) {
		strcpy(Cup_DN,"999");										// wind direction minimum
		strcpy(_Cup_CurrentWindDir,"999");			// wind direction mean
		strcpy(Cup_DX,"999");										// wind direction maximum

		strcpy(Cup_MinSpeed,"999");							// wind speed minimum
		strcpy(_Cup_CurrentWindSpd,"999");			// wind speed mean
		strcpy(Cup_SX,"999");										// wind speed maximum
	}
	Cup_Minute_WINDSENSOR_COUNTS = 0;
  #endif
	
	#if _PP_READ_ATRH
	/* Collect - measure Temperature values - [ATRH Sensor] */
	if (!Minute_ATRH_COUNTS) {
		strcpy(_Temperature,"-999");						// ATRH Temperature min , max
		strcpy(_Humidity,"-999");								// ATRH Humidity min , max	
	}
	Minute_ATRH_COUNTS = 0;
	#endif
	
/***********************************************************************************************/
	
	memset(_buffer, 0, 512);	/* Clear the Buffer */

	/* Combine the parameters and store it in _Sbuffer 
	   - for sending it to portal and also storing it to SD card */
	
	#if _PP_ULTRASONIC
	/* Ultra Sonic Sensor packet format */
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%d;%d;%s;%s;%s;%1d;%1d",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_Humidity,
					CurrentSpeed,
					CurrentWindDirection,
					_sQuality,
					MaxRain_threshold_val1,MaxRain_threshold_val2,
					_logInterval,_PacketlogInterval,_PacketsendInterval,
					PDP,
	        Card_Failed);
		#endif
					
/***********************************************************************************************/
		#if _PP_CUP_ANOMOMETER			
	/* Cup - Anomometer Sensor packet format */
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%s;%s;%s;%s;%d;%d;%s;%s;%s;%1d;%1d",
					imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_Humidity,
					_Cup_CurrentWindSpd,
					_Cup_CurrentWindDir,
					_sQuality,
					MaxRain_threshold_val1,MaxRain_threshold_val2,
					_logInterval,_PacketlogInterval,_PacketsendInterval,
					PDP,
	        Card_Failed);
     #endif
					
/***********************************************************************************************/					
	#if ARG_BIHAR	
		/* ARG packet format */
	sprintf(_buffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%s;%s;%s;%d;%d;%s;%s;%s;%1d;%1d",
					/*#*/imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_sQuality,
					MaxRain_threshold_val1,MaxRain_threshold_val2,
					_logInterval,_PacketlogInterval,_PacketsendInterval,
					PDP,
	        Card_Failed);
  #endif					

	console_log("\n\r ---- Datalogger Status ---- :\n\r   %s\n\r",_buffer);			/* print response in terminal */
  //SMSDefaultSetting();
	
//	GSM_Send_Msg("9890799318",_buffer);
	//GSM_Send_Msg(master_Number,_buffer);	
//	Createlog(_buffer, "slog.txt");		/* Save packet to Slog - for reference */
		
}



/****************************************************************************************************
*                   get fattime [ Date and Time update for FatFS file system ]                      *
*****************************************************************************************************/
DWORD get_fattime(void)
{

	RTC_GetDateTime(&rtc);
	
	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.date  << 16)
			| ((DWORD)rtc.hour  << 11)
			| ((DWORD)rtc.min   << 5)
			| ((DWORD)rtc.sec   >> 1);
}


