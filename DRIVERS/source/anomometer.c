
#include "PP_Config.h"
#if _PP_CUP_ANOMOMETER

/************************ include Header files *********************************/
#include <string.h>
#include <math.h>
#include "stdutils.h"
#include "systick.h"
#include "delay.h"
#include "lcd.h"
#include "adc.h"
#include "uart.h"
#include "convert.h"
#include "extintr.h"
#include "anomometer.h"


/************************* Globle variables *********************************/

char Cup_SN[10] = { 0 },Cup_SM[10]= { 0 },Cup_SX[10]= { 0 },Cup_Gust[10]= { 0 },
     Cup_WINDSENSOR_COUNTS = 0, Cup_Minute_WINDSENSOR_COUNTS = 0;

char Cup_DX[10]= { 0 }, Cup_DN[10]= { 0 }, Cup_FirstRead_WindSensor = 1;

char _Cup_CurrentWindDir[10] = {0}, _Cup_CurrentWindSpd[10] = {0};
char Cup_MinSpeed[10];

char Cup_Tan_Inverse_WindDirection[10];
double Cup_sumof_sin_value_direction, Cup_sumof_cosine_value_direction;


volatile unsigned int numRotateWind = 0,LCDnumRotateWind = 0;
volatile unsigned int ContactBounceTime=0;

volatile float Cup_CurrentWindDir =0,Cup_CurrentWindSpd =0;
volatile float Cup_MaxWindDir =0, Cup_MinWindDir =0 ,Cup_MaxWindSpeed =0, Cup_MinWindSpeed =0;
float Cup_AvgWindDir =0, Cup_AvgWindSpeed =0;
float Cup_sumofWindSpd =0;

char _Cup_LCDCurrentWindDirection[4]= { 0 },_Cup_LCDCurrentSpeed[6]= { 0 };


/*****************************************************************************************
*                      External Interrupt Wind Speed - sensor 	  								  		 *
******************************************************************************************/
#if _PP_EXTERNAL_INTERRUPT_0
void myExtIntrIsr_0(void)
{
  if((SysTick_GetMsTime()-ContactBounceTime) > 15){
  numRotateWind++;
  LCDnumRotateWind++;
  ContactBounceTime= SysTick_GetMsTime();
  }
}

#endif /*_PP_EXTERNAL_INTERRUPT_0*/

/*****************************************************************************************
*          Display Wind sensor value on LCD display - wind speed , wind direction    		 *
******************************************************************************************/
#if _PP_CUP_ANOMOMETER_DISPLAY

 void DisplayWindSensor(void) {
  	int adcValue5 = 0;
	volatile float DISPCurrentWindDir =0, DISPCurrentWindSpd =0;
	adcValue5 = ADC_GetAdcValue(5);
	
	/* Calculating wind speed from the external pulses */
	DISPCurrentWindDir = (adcValue5 * 360)/4095; 

	if(DISPCurrentWindDir > 360)
		DISPCurrentWindDir = DISPCurrentWindDir - 360;

	if(DISPCurrentWindDir < 0)
		DISPCurrentWindDir = DISPCurrentWindDir + 360;
	

	DISPCurrentWindSpd = LCDnumRotateWind * 2.25; // Speed = no.of Pulse*(2.25/Sample Period in seconds)

	DISPCurrentWindSpd = DISPCurrentWindSpd * 0.44704;	// converting mph to m/sec
	//DISPCurrentWindSpd = DISPCurrentWindSpd * 1.60934; // converting mph to KM/hr
	ftoa(DISPCurrentWindSpd,_Cup_CurrentWindSpd,2); // ASCII current Wind SPeed
	ftoa(DISPCurrentWindDir,_Cup_CurrentWindDir,2); // ASCII current Wind Direction
	strcpy(_Cup_LCDCurrentWindDirection ,_Cup_CurrentWindDir);
	strcpy(_Cup_LCDCurrentSpeed ,_Cup_CurrentWindSpd);
}

#endif /*_PP__CUP_ANOMOMETER_DISPLAY*/

/*****************************************************************************************
*                             Read Wind sensor                                      		 *
******************************************************************************************/

#if _PP_READ_CUP_ANOMOMETER

void Cup_Read_WindSensor(void)
{
	int adcValue5 = 0;
	
	adcValue5 = ADC_GetAdcValue(5);

	if(Cup_CurrentWindDir > 360)
		Cup_CurrentWindDir = Cup_CurrentWindDir - 360;

	if(Cup_CurrentWindDir < 0)
		Cup_CurrentWindDir = Cup_CurrentWindDir + 360;

	/* Calculating wind speed from the external pulses */
	Cup_CurrentWindDir = (adcValue5 * 360)/4095; 
	Cup_CurrentWindSpd = numRotateWind * 0.0375; // Speed = no.of Pulse*(2.25/Sample Period in seconds)

	/* #### convert miles to metre :  (Speed in miles x 1609.34)/3600 */ 
	//	Cup_CurrentWindSpd = (Cup_CurrentWindSpd*1609.34)/3600;
		Cup_CurrentWindSpd = Cup_CurrentWindSpd * 0.44704;	// converting mph to m/sec
	//Cup_CurrentWindSpd = Cup_CurrentWindSpd * 1.60934; // converting mph to KM/hr

	/* Conditon apply only first minute reading of Wind Sensor */
	if(Cup_FirstRead_WindSensor)
	{
	   Cup_MaxWindDir = Cup_CurrentWindDir;
	   Cup_MinWindDir = Cup_CurrentWindDir;
	   Cup_MaxWindSpeed = Cup_CurrentWindSpd;
	   Cup_MinWindSpeed = Cup_CurrentWindSpd;
	   Cup_FirstRead_WindSensor = 0;
	}

	/* Check for max wind direction */
	if(Cup_CurrentWindDir > Cup_MaxWindDir)
	{
		Cup_MaxWindDir = Cup_CurrentWindDir; // Max direction	
	}

	/* Check for min wind direction */
	if(Cup_CurrentWindDir < Cup_MinWindDir)
	{
		Cup_MinWindDir = Cup_CurrentWindDir; // Min direction	
	}

	/* Check for max wind speed */
	if(Cup_CurrentWindSpd > Cup_MaxWindSpeed)
	{
		Cup_MaxWindSpeed = Cup_CurrentWindSpd; // Max direction	
	}

	/* Check for min wind speed */
	if(Cup_CurrentWindSpd < Cup_MinWindSpeed)
	{
		Cup_MinWindSpeed = Cup_CurrentWindSpd; // Min direction	
	}

	Cup_sumofWindSpd = Cup_sumofWindSpd + Cup_CurrentWindSpd; // sum of wind speed

//	UART2_Printf("ADC5 value:%4d Wind Dir:%f Rotation Counter:%d Speed:%f Kmph\n\r",
//				adcValue5,CurrentWindDir,numRotateWind,CurrentWindSpd);     // Send the value on UART

	UART2_Printf("ADC5 value:%4d Wind Dir:%f Rotation Counter:%d Speed:%f mps \n\r",
				adcValue5,Cup_CurrentWindDir,numRotateWind,Cup_CurrentWindSpd);     // Send the value on UART

	/* increment the count for every, for averaging */
	Cup_WINDSENSOR_COUNTS =  Cup_WINDSENSOR_COUNTS + 1;

	Cup_Minute_WINDSENSOR_COUNTS   = 1; // log flag set to idnetify the sensor reading in a minute

	ftoa(Cup_CurrentWindDir,_Cup_CurrentWindSpd,2); // ASCII current Wind SPeed
	ftoa(Cup_CurrentWindSpd,_Cup_CurrentWindDir,2); // ASCII current Wind Direction
			
	/* conversion to sine and cosine of average wind direction */
	Cup_Trignometric_Conversion();

}

/*****************************************************************************************
*              converting wind direction - Trignometric_Conversion                   		 *
******************************************************************************************/
/* #### converting wind direction */
void Cup_Trignometric_Conversion(void)
{
 	double sin_value_direction =0,cosine_value_direction = 0;

	/* Convert degree value to radian */
	/* apply sine and cosine values */
	sin_value_direction = sin(Cup_CurrentWindDir * (3.14/180.0));
	cosine_value_direction = cos(Cup_CurrentWindDir * (3.14/180.0));

//	UART0_Printf("Sine(%f):%f deg Cosine(%f):%f deg\n\r",
//	f_DM,sin_value_direction,f_DM,cosine_value_direction); // Print in terminal 

   	Cup_sumof_sin_value_direction = Cup_sumof_sin_value_direction + sin_value_direction;
	Cup_sumof_cosine_value_direction = Cup_sumof_cosine_value_direction + cosine_value_direction;

//	UART0_Printf("Sum of Sine samples:%f Sum of cosine samples:%f SampleCount:%d\n\r",
//	sumof_sin_value_direction,sumof_cosine_value_direction,WINDSENSOR_COUNTS); // Print in terminal
}

#endif /*_PP_READ_CUP_ANOMOMETER*/


/*****************************************************************************************
*                             Read average Wind sensor value                        		 *
******************************************************************************************/
#if _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE
void Cup_Average_WindSamples(void)
{
	float Average_sumofWindSpd = 0;

   	/* Average formula: Average = (Sum of Samples/Count) */
	Average_sumofWindSpd = Cup_sumofWindSpd/Cup_WINDSENSOR_COUNTS;
	
	/* #### final packet values Wind Speed */	
	ftoa(Cup_MaxWindSpeed,Cup_SX,2); // ASCII Max of Wind Speed
	ftoa(Cup_MinWindSpeed,Cup_MinSpeed,2); // ASCII Min of Wind Speed
	ftoa(Average_sumofWindSpd,Cup_SM,2); // ASCII Mean of Wind Speed

	strcpy(Cup_Gust,Cup_SX);

	/* #### final packet values Wind Direction */
	ftoa(Cup_MaxWindDir,Cup_DX,2); // ASCII Max of Wind Direction
	ftoa(Cup_MinWindDir,Cup_DN,2); // ASCII Min of Wind Direction

	/* average the sine and cosine of wind directions*/
	Cup_average_windDirection();
}


/*****************************************************************************************
*                    Read average Wind sensor direction value                        		 *
******************************************************************************************/
void Cup_average_windDirection(void)
{
	double avg_sumof_sin_value_direction =0, avg_sumof_cosine_value_direction =0;
	double Dividend_of_sum_Sine_Cosine = 0;
	double FinalAvg_Direction = 0, RadianAvg_Direction = 0;

	avg_sumof_sin_value_direction = Cup_sumof_sin_value_direction/Cup_WINDSENSOR_COUNTS;
	avg_sumof_cosine_value_direction = 	Cup_sumof_cosine_value_direction/Cup_WINDSENSOR_COUNTS;
//	UART0_Printf("Average of Sine: %f\n\r",avg_sumof_sin_value_direction); // Print in terminal
//	UART0_Printf("Average of cosine: %f\n\r",avg_sumof_cosine_value_direction); // Print in terminal

	Dividend_of_sum_Sine_Cosine = avg_sumof_sin_value_direction/avg_sumof_cosine_value_direction;			
//	UART0_Printf("Dividend of Sine Cosine: %f\n\r",Dividend_of_sum_Sine_Cosine); // Print in terminal

	//radianAvg = Dividend_of_sum_Sine_Cosine*(3.14/180.0);
	RadianAvg_Direction = atan(Dividend_of_sum_Sine_Cosine); // tan inverse
	DELAY_ms(300);
//	UART0_Printf("TanInverse(%f): %f radian\n\r",Dividend_of_sum_Sine_Cosine,RadianAvg_Direction); // Print in terminal			
		
	/* Convert final average from radian to Degree */
	FinalAvg_Direction = (180.0/3.14)*RadianAvg_Direction;
//	UART0_Printf("TanInverse(%f): %f deg\n\r",Dividend_of_sum_Sine_Cosine,FinalAvg_Direction); // Print in terminal
							 
	if(avg_sumof_sin_value_direction > 0 && avg_sumof_cosine_value_direction > 0 )
	{
		//Do nothing
		print_DebugMsg("Both Sine and cosine are positive\r\n");
	}
	if(avg_sumof_cosine_value_direction < 0)
	{
		print_DebugMsg("Only cosine is negative\r\n");
		FinalAvg_Direction = FinalAvg_Direction+180.0;	
	}
	if(avg_sumof_sin_value_direction < 0 && avg_sumof_cosine_value_direction > 0)
	{
		print_DebugMsg("Sine is negative, Cosine is positive\r\n");
		FinalAvg_Direction = FinalAvg_Direction+360.0;	
	}

	UART2_Printf("Final Mean Direction: %f deg\n\r",FinalAvg_Direction); // Print in terminal
	ftoa(FinalAvg_Direction,Cup_Tan_Inverse_WindDirection,2);
	

	/* clear varaibles and counts for next samples */
	Cup_sumof_sin_value_direction = 0;
	Cup_sumof_cosine_value_direction = 0;
	Cup_WINDSENSOR_COUNTS = 0;
	Cup_MinWindSpeed = 0;
	Cup_MaxWindSpeed = 0;
	Cup_MaxWindDir = 0;
	Cup_MinWindDir = 0;
	Cup_sumofWindSpd = 0;
	Cup_FirstRead_WindSensor = 1;
}


#endif /* _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE */

#endif /*_PP_CUP_ANOMOMETER*/


