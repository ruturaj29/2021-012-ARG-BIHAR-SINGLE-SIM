//F:\Random\Stock\2021-001-ARG-BIHAR-main\DRIVERS\source\LCD_Scroll.c
#include "PP_Config.h"
#if _PP_LCD_SCROLL

/********************** Include all header files ******************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"
#include "rtc.h"
#include "delay.h"
#include "adc.h"
#include "ATRH.h"
#include "convert.h"
#include "rain.h"
#include "leafwetness.h"
#include "extintr.h"
#include "gprs.h"
#include "version.h"
#include "LCD_Scroll.h"
#include "WSWD.h"
#include "lpc17xx_wdt.h"
#include "anomometer.h"
#include "usbhost_inc.h"
#include "diskio.h"
#include "mmc_176x_ssp.h"
#include "ff.h"
#include "eeprom.h"

/********************** Global variable declarations **************************/
/********************** LCD Scroll ********************************************/
volatile uint32_t KeyInterrupt = 0, scrollScreen = 0;
char Testlogbuffer[512]; // test log buffer for Storing as a packet sending to Portal

/********************** rtc.c variables ***************************************/
extern rtc_t rtc;	/* rtc structure declare */

/********************** EEPROM related variables ***************************************/
extern uint32_t eeprom_address_Rain;

/********************** main  *************************************************/
extern char  FirmwareRevision[20];
extern bool LCD_Enable;

/*********** Ultrasonic Sensors ********/
extern char _LCDTemperature[10], _LCDHumidity[10];

extern char _LCDCurrentWindDirection[4],_LCDCurrentSpeed[6];
extern char _Cup_LCDCurrentWindDirection[4],_Cup_LCDCurrentSpeed[6];

/**************************** GSM module **************************************/
extern char imei[16];
extern char CCID[25];
extern char* APN;
extern char latitude_buffer[16];
extern char longitude_buffer[16];
extern sint32_t dbm ;
extern char SignalQualityIndication[15],_sQuality[5];
extern char altitude_buffer[8];
extern char Satellites_In_View[3];
extern int PDP;

extern int Authentication;
extern int USB_Failed;
extern uint32_t detector;
/********************** Function Declaration **************************/
//extern void Read_ADCchannels(void);
void myExtIntrIsr_1( void );
void LCD_ScrollDisplay(void);
void sampleTestlog(void); // sample test log rountine


/*****************************************************************
*      External Interrupt Switch - Push Button                   *
******************************************************************/
#if _PP_EXTERNAL_INTERRUPT_1

void myExtIntrIsr_1(void)
{	
  KeyInterrupt = 1; 
 #if !ARG_BIHAR	
	if(scrollScreen>=13)
		scrollScreen = 1;
	else 
		scrollScreen++;	/* Increment scrollScreen every time EINT1 is detected */	
	#endif
	
	#if ARG_BIHAR	
	
	if(scrollScreen>=12)
		scrollScreen = 1;
	else 
		scrollScreen++;	/* Increment scrollScreen every time EINT1 is detected */
	#endif
}
#endif /*_PP_EXTERNAL_INTERRUPT_1*/

/*****************************************************************
*    LCD_ScrollDisplay [ shows all parameter on LCD DISPLY ]     *
******************************************************************/
#if _PP_LCD_Scroll_Display

void LCD_ScrollDisplay(void)					   
{
	//DELAY_ms(30);
	//lcd_enable(); 	//commented in stock
	uint8_t switchStatus ; //returnStatus;
	DELAY_ms(50);					
	
	switch(scrollScreen)
	{
		/*  Software & Hardware version */
		case 1: 
			      //lcd_enable();                // Uncommented in stock
						//DELAY_ms(30);
      			LCD_Clear();
						
//						LCD_Printf("HDW ST-0301 \nFWR v%1d.%1d.%1d",_MAJOR__,_MINOR__,_BUILD__);
		        LCD_Printf("HDW ST-0301 \nFWR %s",_FIRMWARE_STRING__);
				
						break;
		/*	Time & Date display */
		case 2:	RTC_GetDateTime(&rtc);
						LCD_Clear();
						LCD_Printf("   %2d/%2d/%d\n    %2d:%2d:%2d",(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
											(uint16_t)rtc.hour,(uint16_t)rtc.min,(uint16_t)rtc.sec);
						
						break;
		/*	Device IMEI	*/
		case 3: LCD_Clear();
						LCD_Printf("Device IMEI:\n%s",imei);
		
						break;
		/*	SIM  number CCID display	*/
		case 4: LCD_Clear();
						LCD_Printf("CCID:%s",CCID);
						break;
		/*	Operator APN	*/
		case 5: LCD_Clear();
						LCD_Printf("Operator APN:\n%s",APN);
						break;
		/*	Solar Panel & battery voltage*/
		case 6:	adcValue0 = ADC_GetAdcValue(0); // Read the ADC value of channel zero
						volt0 = (adcValue0*3.3)/4095;
						ActualVoltage0 = (20.00*volt0)/3.3;
						DELAY_ms(50);
						
						adcValue1 = ADC_GetAdcValue(1); // Read the ADC value of channel zero
						volt1 = (adcValue1*3.3)/4095;
						ActualVoltage1 = (36.00*volt1)/3.3;
						
						ftoa_signed(ActualVoltage0,_BVolt,2);
						ftoa_signed(ActualVoltage1,_PVolt,2);
						
						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Batt : %01s V",_BVolt);
						LCD_GoToLine(1);LCD_Printf("Solar: %01s V",_PVolt);
						DELAY_ms(100);											  
						break;
		#if !ARG_BIHAR
		#if _PP_READ_ATRH 
		 /* Read ATRH sensor */
		case 7:	readATRH(ATRH_Write_ADD,ATRH_READ_ADD,Temperature_register_add);
						DELAY_ms(200);  
					
						if (!Minute_ATRH_COUNTS)
						{
							strcpy(_LCDTemperature,"NA");
							strcpy(_LCDHumidity,"NA");
						}
						Minute_ATRH_COUNTS = 0;

						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Temp('C): %s",_LCDTemperature);
						LCD_GoToLine(1);LCD_Printf("RHumid  : %s",_LCDHumidity);
						break;
		#endif
						
		#if _PP_READ_WINDSENSOR
		/* Read wind sensor ultrasonic */
		case 8:	Read_WindSensor();
						DELAY_ms(200);

						if (!Minute_WINDSENSOR_COUNTS)
						{
							strcpy(_LCDCurrentWindDirection,"NA");	
							strcpy(_LCDCurrentSpeed,"NA");
						}
						Minute_WINDSENSOR_COUNTS = 0;

						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Wdir(deg): %s",_LCDCurrentWindDirection);
						LCD_GoToLine(1);LCD_Printf("Wspd(m/s): %s",_LCDCurrentSpeed);
						break;	
		#endif
						
		#if _PP_CUP_ANOMOMETER_DISPLAY
						
		/* Read wind sensor Anomometer */
		case 8:	DisplayWindSensor();
						Cup_Minute_WINDSENSOR_COUNTS = 0;
						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Wdir(deg):%s",_Cup_LCDCurrentWindDirection);
						LCD_GoToLine(1);LCD_Printf("Wspd(m/s):%s",_Cup_LCDCurrentSpeed);
						LCDnumRotateWind = 0;
						DELAY_ms(1000);
						break;	
			#endif
			
    			
		/* rain fall display */
		case 9:	LCDrain = LCDnumTipsRain * 0.25; 
						ftoa_signed(LCDrain,_LCDRain,2);
						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Rain fall");
						LCD_GoToLine(1);LCD_Printf("%s mm",_LCDRain);
//            LCD_GoToLine(0);LCD_Printf("Rain F:%s mm",_LCDRain);
//						LCD_GoToLine(1);LCD_Printf("Rain D:%2d ",detector);
						break;
		/* Signal Quality display */
		case 10:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
							LCD_GoToLine(1);LCD_Printf("%s,pdp: %1d",SignalQualityIndication, PDP);
							break;
		/* Log interval display */
		case 11:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("sLog|pLog|rLog");
							LCD_GoToLine(1);LCD_Printf("%s|%s|%s",_logInterval,_PacketlogInterval,_PacketsendInterval);
							break;
		/* GPS co-ordinates display */
		case 12:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Lat : %s",latitude_buffer);
							LCD_GoToLine(1);LCD_Printf("Lon : %s",longitude_buffer);
							break;

		/* test log sending	*/
		case 13:	EINT_DetachInterrupt(EINT1); 							/* disable External interrupt for push button */
							LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Press & Hold");
							LCD_GoToLine(1);LCD_Printf("to send test log");
							GPIO_PinFunction(P2_11,PINSEL_FUNC_0);  	/* Configure Pin for Gpio */
				    	GPIO_PinDirection(P2_11,INPUT);						/* Configure the SwitchPin as input */
							DELAY_ms(1500);
							switchStatus = GPIO_PinRead(P2_11);       /* Read the switch status */
							if (switchStatus == 0)
							{	
								DELAY_ms(1000);
								switchStatus = GPIO_PinRead(P2_11);     /* Read the switch status */
								if (switchStatus == 0)
								{
									LCD_Clear();
									LCD_GoToLine(0);LCD_Printf("+++ Test log +++");
									LCD_GoToLine(1);LCD_Printf("Uploading...");
									sampleTestlog();											/* send test packet to server */
								}
							}
							DELAY_ms(1000);
							KeyInterrupt = 0;
							scrollScreen = 0;
							EINT_AttachInterrupt(EINT1,myExtIntrIsr_1,FALLING);
							LCD_Clear();
							lcd_disable();
							break;
				#endif
		#if ARG_BIHAR
				/* rain fall display */
		case 7:	
						LCDrain = RTC_ReadGPREG(2);
						//LCDrain = EEPROM_ReadByte(eeprom_address_Rain);
						LCDrain = LCDrain + (LCDnumTipsRain * 0.50);
						//EEPROM_WriteByte(eeprom_address_Rain, LCDrain);
						ftoa_signed(LCDrain,_LCDRain,2);
						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("Rain fall");
						LCD_GoToLine(1);LCD_Printf("%s mm",_LCDRain);
//            LCD_GoToLine(0);LCD_Printf("Rain F:%s mm",_LCDRain);
//						LCD_GoToLine(1);LCD_Printf("Rain D:%2d ",detector);
						//LCDnumTipsRain = 0;
						break;
		/* Signal Quality display */
		case 8:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Signal Qlty: %s",_sQuality);
							LCD_GoToLine(1);LCD_Printf("%s,pdp: %1d",SignalQualityIndication, PDP);
							break;
		/* Log interval display */
		case 9:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("sLog|pLog|rLog");
							LCD_GoToLine(1);LCD_Printf("%s|%s|%s",_logInterval,_PacketlogInterval,_PacketsendInterval);
							break;
		/* GPS co-ordinates display */
		case 10:	LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Lat : %s",latitude_buffer);
							LCD_GoToLine(1);LCD_Printf("Lon : %s",longitude_buffer);
							break;
		/* test log sending	*/
		case 11:	EINT_DetachInterrupt(EINT1); 							/* disable External interrupt for push button */
							LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Press & Hold");
							LCD_GoToLine(1);LCD_Printf("to send test log");
							GPIO_PinFunction(P2_11,PINSEL_FUNC_0);  	/* Configure Pin for Gpio */
				    	GPIO_PinDirection(P2_11,INPUT);						/* Configure the SwitchPin as input */
							DELAY_ms(1500);
							switchStatus = GPIO_PinRead(P2_11);       /* Read the switch status */
							if (switchStatus == 0)
							{	
								DELAY_ms(1000);
								switchStatus = GPIO_PinRead(P2_11);     /* Read the switch status */
								if (switchStatus == 0)
								{
									LCD_Clear();
									LCD_GoToLine(0);LCD_Printf("+++ Test log +++");
									LCD_GoToLine(1);LCD_Printf("Uploading...");
									sampleTestlog();											/* send test packet to server */
									RTC_WriteGPREG(2,0);    //Clear RTC reg 2 to make rain 0 while first start
								}
							}
							DELAY_ms(500);
							//EINT_AttachInterrupt(EINT1,myExtIntrIsr_1,FALLING);
							scrollScreen++;
//							LCD_Clear();
//							DELAY_ms(500);
							break;
							
						/* USB READ WRITE */
		case 12:  
              //EINT_DetachInterrupt(EINT1); 
							LCD_Clear();
							LCD_GoToLine(0);LCD_Printf("Press & Hold");
							LCD_GoToLine(1);LCD_Printf("to copy data");
							GPIO_PinFunction(P2_11,PINSEL_FUNC_0);  	/* Configure Pin for Gpio */
				    	GPIO_PinDirection(P2_11,INPUT);						/* Configure the SwitchPin as input */
							DELAY_ms(1500);
							switchStatus = GPIO_PinRead(P2_11);       /* Read the switch status */
							if (switchStatus == 0)
							{
								DELAY_ms(1000);
								switchStatus = GPIO_PinRead(P2_11);     /* Read the switch status */
								if (switchStatus == 0)
								{
									LCD_Clear();
									USB ();
								}
							}
							//EINT_AttachInterrupt(EINT1,myExtIntrIsr_1,FALLING);
							DELAY_ms(500);
							KeyInterrupt = 1;
							scrollScreen = 0;
							EINT_AttachInterrupt(EINT1,myExtIntrIsr_1,FALLING);
					//		LCD_Clear();
					//		lcd_disable();
							DELAY_ms(5);
							break;			
							
							
				#endif
	}	
										  
}



/*****************************************************************
*    sampleTestlog [ Creat test packet and send to server ]      *
******************************************************************/
void sampleTestlog(void)
{
	uint8_t responseStatus,returnStatus,i;
	//char *parameter  = "#;867383050757576;7D68A57E;28/04/2021;17:42:00;12.13;9.97;-999;-999;-999;-999;-999;-999;0999;0.00;0999;0999;0999;0.00;00.00;0.00;0.00;24,0;0.00;01/01/1970;00:00:00;0";
	#if !ARG_BIHAR
  char  _TestMinTemp[10]={0}, _TestMaxTemp[10]={0}, _TestMinHumid[10]={0}, _TestMaxHumid[10]={0};
	char _TestMinWindSpd[10] = {0}, _TestMaxWindSpd[10] = {0};
	char _TestWindDirection[10] = {0} ;
	#endif
	
	RTC_GetDateTime(&rtc);	/* get date and time */
	#if _PP_READ_ATRH
	readATRH(ATRH_Write_ADD,ATRH_READ_ADD,Temperature_register_add);	/* Read ATRH sensor */
	#endif
	#if _PP_READ_WINDSENSOR
	Read_WindSensor();			/* Read Wind sensor */
	#endif
	#if _PP_READ_CUP_ANOMOMETER
	Cup_Read_WindSensor();
	#endif
	#if _PP_RAIN_FALL
	calcRainFall();					/* Read Rain Guage */
	#endif
	#if _PP_READ_BATT_SOLAR_VOLT
  Read_ADCchannels();			/* Read Solar and Power voltage */
  #endif
	
	#if _PP_READ_WINDSENSOR
	if (!Minute_WINDSENSOR_COUNTS)	/* Collect-measure Wind Sensor  */
	{
		strcpy(CurrentWindDirection,"999");	
		strcpy(CurrentSpeed,"999");			
	}
	Minute_WINDSENSOR_COUNTS = 0;
  #endif
	
	#if _PP_READ_CUP_ANOMOMETER
	if (!Cup_Minute_WINDSENSOR_COUNTS) {
		
			strcpy(_Cup_CurrentWindDir,"999");	
			strcpy(_Cup_CurrentWindSpd,"999");
	}
  #endif
	
	#if _PP_READ_ATRH 
	if (!Minute_ATRH_COUNTS)	/* Collect-measure ATRH  */
	{		
		strcpy(_Temperature,"-999"); 
		strcpy(_Humidity,"-999");	
	}
	Minute_ATRH_COUNTS = 0;
  #endif
	
	WDT_Feed();
	#if _PP_READ_ATRH
	strcpy(_TestMinTemp,_Temperature);strcpy(_TestMaxTemp,_Temperature);
	strcpy(_TestMinHumid,_Humidity);strcpy(_TestMaxHumid,_Humidity);
	#endif
	#if _PP_READ_WINDSENSOR
	strcpy(_TestWindDirection,CurrentWindDirection);
	strcpy(_TestMinWindSpd,CurrentSpeed);strcpy(_TestMaxWindSpd,CurrentSpeed);
  #endif
	#if _PP_READ_CUP_ANOMOMETER
	strcpy(_TestWindDirection,_Cup_CurrentWindDir);
	strcpy(_TestMinWindSpd,_Cup_CurrentWindSpd);strcpy(_TestMaxWindSpd,_Cup_CurrentWindSpd);
	#endif
	
	memset(Testlogbuffer, 0, 512);
//  strcpy(Testlogbuffer,parameter);
	
/* Combine the parameters and store it in _buffer - for sending it to portal and also storing it to SD card */
	#if !ARG_BIHAR
	sprintf(Testlogbuffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%4s;%4s;%4s;%4s;%4s;%2s;%2s;%2s;%4s;%4s;%4s;%4s;%4s;%4s;%4s;0.00;0.00;%s,0;0.00;01/01/1970;00:00:00;0",			              
					/*#*/imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Temperature,_TestMaxTemp,_TestMinTemp,				  
					_Humidity,_TestMaxHumid,_TestMinHumid,
					_TestMaxWindSpd,_TestMaxWindSpd,_TestMinWindSpd,_TestMinWindSpd,
					_TestWindDirection,_TestWindDirection,
					_Rain,_sQuality);
					
	#else
	
	sprintf(Testlogbuffer,"#;%s;%s;%02d/%02d/%d;%02d:%02d:00;%4s;%4s;%4s;0.00;0.00;00;%s,99;0",			              
					/*#*/imei,_FIRMWARE_STRING__,
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,
					(uint16_t)rtc.hour,(uint16_t)rtc.min,
					_PVolt,_BVolt,
					_Rain,/*24HR_Rain,Yearly_Rain,
					Rain_detect*/
					_sQuality/*,99;0*/);
	#endif

	console_log("%s\n\r",Testlogbuffer);	/* print response in terminal */
	UART1_EnableBuffer_Interrupt();				/* Enable Buffer RBR UART 1 interrupt*/	
	
  returnStatus =Module_awakeup_mode();
	
	if (returnStatus)
	{
		WDT_Feed();
		
		for (i=2;i>0;i--) {
			returnStatus = modemCheck_initialization(APN);
			if (returnStatus) break;
		}
		
		if (returnStatus)
		{
			responseStatus = PackOffLog(Testlogbuffer);	/* Send the packet to the server */
			if ((responseStatus == MODEM_RESPONSE_TIMEOUT)||(responseStatus == MODEM_RESPONSE_ERROR)||(responseStatus == 0))
			{																																																				
				if (Check_QIState())
				{
					responseStatus = PackOffLog(Testlogbuffer); /* Try 2nd time sending packet*/	
				}
			}
			if ((responseStatus == MODEM_RESPONSE_TIMEOUT) || (responseStatus == MODEM_RESPONSE_ERROR) || (responseStatus == 0))
			{
				if(LCD_Enable){
				LCD_Clear();
				LCD_GoToLine(0);LCD_Printf("Uploading failed");
				LCD_GoToLine(1);LCD_Printf("Err : %d",ERROR_CODE);
				DELAY_ms(2000);
				}
			}
			else 
			{
				if(LCD_Enable){
				LCD_GoToLine(1);
				LCD_Printf("Uploading Done");
				DELAY_ms(2000);
				}
			}
		}	
		else
		{
			if(LCD_Enable){
			LCD_Clear();
			LCD_GoToLine(0);LCD_Printf(" Post failed !");
			LCD_GoToLine(1);LCD_Printf("GPRS CONN FAIL");
			DELAY_ms(1000);
			}
		}
		
		SMS_READ_Setting();			/* SET SMS setting */
		readSMS();							/* Read SMS */
	}
	else
	{
		if(LCD_Enable){
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf(" Post failed !");
		LCD_GoToLine(1);LCD_Printf("AT CMD FAILED");
		DELAY_ms(1000);
		}
	}
	
	Module_deactivateBearerProfile();	/* Deactivate module */	
	Module_sleep_mode();							/* put GSM module in Sleep mode */  
	UART1_DisableBuffer_Interrupt(); 	/* Disable Buffer RBR UART 1 interrupt */  
}
#endif /* _PP_LCD_Scroll_Display */


/*****************************************************************
*            USB code execution                                  *
******************************************************************/
void USB (void)
{
	int returnStatus=0;
	WDT_Feed();
	USB_INITIALIZATION ();
	
	if(!USB_Failed){
		returnStatus = USB_Check_Authentication();
		if(returnStatus){
			print_DebugMsg("File Opening Failed \n\r Authentication faild..!! \n\r");
			LCD_Clear();
			LCD_GoToLine(0); LCD_Printf("Authentication");
			LCD_GoToLine(1); LCD_Printf("Failed..!!");
			DELAY_ms(2000);
		}
		else {
			print_DebugMsg("File Opened \n\r Authentication Done..!! \n\r");
			LCD_Clear();
		  LCD_GoToLine(0); LCD_Printf("Authentication");
			LCD_GoToLine(1); LCD_Printf("Done..!!");
			DELAY_ms(1000);
		}
		if((!returnStatus) && Authentication)
		{
		  copy();
		  LCD_Clear();
		  LCD_GoToLine(0); LCD_Printf("Copy");
			LCD_GoToLine(1); LCD_Printf("Completed..!!");
		  DELAY_ms(2000);
		}
	}
}



#endif /* _PP_LCD_SCROLL */





//F:\Random\Stock\2021-001-ARG-BIHAR-main\DRIVERS\source\LCD_Scroll.c