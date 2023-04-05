
#include "PP_Config.h"
#if _PP_Rain_Gauge

/************** ALL header files *******************************/
#include "rain.h"
#include "lpc17xx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "uart.h" 
#include "rtc.h" 
#include "convert.h"
#include "extintr.h"
#include "gpio.h"
#include "IOExtInt.h"
#include "eeprom.h"
#include "Log.h"
#include "mmc_176x_ssp.h"

/************** Globle Variable Declerations *********************/
char _Rain[8] = { 0 }, _LCDRain[8] = { 0 } , _PeriodicRain[8] = { 0 };
double rain , LCDrain=0 , total_DailyRain=0 , temp_rain =0, yearly_rain=0 , PeriodicRain;
volatile unsigned int numTipsRain = 0,LCDnumTipsRain = 0;
uint32_t checkvalue, detector;

/************** EEPROM related variables *********************/
uint32_t   eeprom_address_Rain = 0x23; //Next address should be 0x24

extern char _buffer[512];
/*****************************************************************
*         External Interrupt Rain Guage - sensor       					 *
******************************************************************/
#if _PP_EXTERNAL_INTERRUPT_3
void myExtIntrIsr_3(void)
{
	checkvalue = IOCheckInterrupts(0, 0);
  if (checkvalue == (1 << 26))
	{
		//IOSetValue(2, 3, 1);
		detector++;
	}
	IOClearInterrupt();
	
	if(!checkvalue)
	{
   numTipsRain++;	LCDnumTipsRain++;
	}
}

#endif /*_PP_EXTERNAL_INTERRUPT_3*/

/*****************************************************************
*         calculate Rain Fall       					                   *
******************************************************************/
#if _PP_RAIN_FALL

void calcRainFall(void) 
{ 
	rtc_t rtc;	/* rtc structure declare */
	//rain = EEPROM_ReadByte(eeprom_address_Rain);
	//console_log("\n\rRTC_ReadGPREG(0) : %f\n\r" , RTC_ReadGPREG(2));
	rain = RTC_ReadGPREG(2);
	//EEPROM_ReadNBytes(eeprom_address_Rain ,(uint32_t)rain , 2);
	//console_log("EEPROM Rain Read in calcRainFall = %f \n\r",rain);
	//console_log("EEPROM Rain Read in calcRainFall = %f \n\r",rain);
	//console_log("Size of rain Double type = %d \n\r",sizeof(rain));
	PeriodicRain = (numTipsRain * 0.50); 
	rain = rain + (numTipsRain * 0.50); 
	temp_rain = rain;
	//EEPROM_WriteByte(eeprom_address_Rain, rain);
	RTC_WriteGPREG(2,rain);
	console_log("\n\rRTC_ReadGPREG(2) rain : %f\n\r" , RTC_ReadGPREG(2));
	//EEPROM_WriteNBytes(eeprom_address_Rain , (uint32_t)rain , 2);
	ftoa(rain,_Rain,2);
	ftoa(PeriodicRain,_PeriodicRain ,2);
	// copy _Rain to EEPROM Here
	
	console_log("No.of tips:%d RainFall:%s mm\n\r",numTipsRain,_Rain);// print response in terminal
	numTipsRain = 0;
	LCDnumTipsRain = 0;
	/* Get RTC */
	RTC_GetDateTime(&rtc);
//	if(rtc.hour == 0) { /* Check RTC is 24 hours & reset the rain value to 0*/
//		rain = 0;
//		RTC_WriteGPREG(2,0);   // RTC GPREG will reset as it contains rain value
//		//EEPROM_WriteByte(eeprom_address_Rain, rain);
//		LCDnumTipsRain = 0;
//		LCDrain = 0;
//	}

		if(rtc.hour == 8 && (rtc.min == 15 || rtc.min == 16|| rtc.min == 17 || rtc.min == 18 || rtc.min == 19 || rtc.min == 20 || rtc.min == 21 || rtc.min == 22 || rtc.min == 23 || rtc.min == 24))
			{
				RTC_GetDateTime(&rtc);
				memset(_buffer, 0, 512);
				sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:RL_BEFORE_RAIN_RESET:8_30:CumRain = %4s;",
						(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,_Rain);
				Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
				//print_DebugMsg(" BATTERY LOW \n\r");
					
					Reset24HrRainAt8_16();
					
				RTC_GetDateTime(&rtc);
				sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:RL_AFTER_RAIN_RESET:8_30:CumRain = %4s;",
						(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,_Rain);
				Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
			}

}

void Reset24HrRainAt8_16(void)
{
		print_DebugMsg(" Reset24HrRainAt8_16 Function Called \n\r");
		rain = 0;
		RTC_WriteGPREG(2,0);   // RTC GPREG will reset as it contains rain value
		//EEPROM_WriteByte(eeprom_address_Rain, rain);
		LCDnumTipsRain = 0;
		LCDrain = 0;

}

void Reset24HrRainIfBattLow(void)
{
		rtc_t rtc;
		RTC_GetDateTime(&rtc);
		console_log("Enter IN BATEERY_LOW_RTC_FLAG_RESET_LOOP = %2f\n\r",RTC_ReadGPREG(0));
		if ( (RTC_ReadGPREG(0) == 1) && ( rtc.hour > 8) && (rtc.min > 0) )
		{
				print_DebugMsg(" Reset24HrRainIfBattLow Function Called \n\r");
				rain = 0;
				RTC_WriteGPREG(2,0);   // RTC GPREG will reset as it contains rain value
				//EEPROM_WriteByte(eeprom_address_Rain, rain);
				LCDnumTipsRain = 0;
				LCDrain = 0;
			  RTC_WriteGPREG(0,0); //Battery Low flag reset as it
			
				RTC_GetDateTime(&rtc);
				sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:Rain_Reset_BattLo:Flag = %f:CumRain = %4s;",
						(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,RTC_ReadGPREG(0),_Rain);
				Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
		}
		else
		{
		  console_log("No Rain Reset Due To Batt Low as Flag = %2f\n\r",RTC_ReadGPREG(0));
		}
}



/*****************************************************************
*         calculate Daily Rain Fall [ every 24 hours]            *
******************************************************************/
void _24hr_RainFall(void)
{	
	 console_log("\n\rIn function 24 hr\n\r");
	// RTC_WriteGPREG(0,0);  //uncomment this to write 0 to RTC register for 1st time use 
 	 console_log("\n\rtemp_rain : %f mm\n\r" , temp_rain);// print response in terminal
	 console_log("\n\rRTC_ReadGPREG(0) : %f\n\r" , RTC_ReadGPREG(0));
	 total_DailyRain = RTC_ReadGPREG(0) + temp_rain;
	 console_log("\n\rTotal Daily Rain : %f\n\r" , total_DailyRain);
	 RTC_WriteGPREG(0,total_DailyRain);
   total_DailyRain=0;
}


/*****************************************************************
*         calculate Yearly Rain Fall [ 12 months]                *
******************************************************************/
void Yearly_Rain(void)
{
	console_log(" \n\rIn function Yearly_Rain\n\r");
	//RTC_WriteGPREG(1,0);  //uncomment this to write 0 to RTC register for 1st time use
	console_log(" \n\rtemp_rain : %f mm\n\r" , temp_rain);// print response in terminal
	console_log(" \n\rRTC_ReadGPREG(1) : %f\n\r" , RTC_ReadGPREG(1));
	yearly_rain = RTC_ReadGPREG(1) + temp_rain;
  console_log(" \n\rTotal Yearly_Rain : %f\n\r" , yearly_rain);
	RTC_WriteGPREG(1,yearly_rain);
	yearly_rain=0;	
}

#endif /*_PP_RAIN_FALL*/
#endif /*_PP_Rain_Gauge*/



