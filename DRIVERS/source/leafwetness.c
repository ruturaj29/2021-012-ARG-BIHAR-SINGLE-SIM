
/************** ALL header files ***************************************/
#include <string.h>
#include "leafwetness.h"
#include "uart.h" 
#include "delay.h"
#include "adc.h"
#include "gpio.h"
#include "convert.h"
#include "lcd.h"

/************** Globle Variable Declerations ***************************/
leafWetness_t leafWetness;
char firstRead_LW = 1, count_LW = 0;
unsigned short int sum_leafWetness = 0;
int adcValue4;

/************* Read Leaf Wetness Function ********************/
void Read_LeafWetness(void)
{
	   /* Converting the raw adc value to equivalent solar radiation with 3.3v as ADC reference using 12bit resolution.
        Step size of ADC= (3.3v/2^12)= 3.3/4096 = 0.0008056640625 = 0.0806mv

        For every Wetness on leaf change the sensor provides 53.333mv  voltagechange.
        leafwetness = 53.333 (range) = 53.333/0.0806mv = 661.69 uinits            
        Hence the Raw ADC value can be divided by 661.69 to get equivalent leaf wetness range
        */ 
		  
	adcValue4 = ADC_GetAdcValue(4); // Read the ADC value of channel zero
	leafWetness.current =  adcValue4/282.52;

	console_log("Leaf Wetness:%01d range ADCvalue:%d\n\r",leafWetness.current,adcValue4);

	if(firstRead_LW)
	{
		leafWetness.min = leafWetness.current;
		leafWetness.max = leafWetness.current;
		firstRead_LW = 0;	
	}
	/* Check for max solar radiation */
	if(leafWetness.current > leafWetness.max)
	{
		 leafWetness.max =  leafWetness.current;
	}
	/* check for min solar radiation */
	if(leafWetness.current < leafWetness.min)
	{
	  leafWetness.min =  leafWetness.current;
	}

	count_LW = 	count_LW + 1;
	sum_leafWetness = sum_leafWetness + (leafWetness.current);

}

/************* Average of Leaf Wetness Function ********************/
void averageof_leafWetness(void)
{
	/* Average formula: Average = (Sum of Smaples/Count)*/

	leafWetness.average = sum_leafWetness/count_LW;

	if(leafWetness.average > 15)
	{
	  leafWetness.average = 15;
	}
	if(leafWetness.min > 15)
	{
	  leafWetness.min = 15;
	}
		if(leafWetness.max > 15)
	{
	  leafWetness.max = 15;
	}

}

/************* Clear Leaf Wetness Flags Function ********************/
void ClearleafWetnessFlags(void)
{
  	/* clear varaibles and counts for next samples */


	sum_leafWetness = 0;
	count_LW = 0;
	firstRead_LW = 1;
	
}

