
 
#include "PP_Config.h"
#if PP_CONFIG_H == 4004
#if _PP_ADC 

/************** ALL header files ***************************************/
#include "adc.h"

/************** Globle Variable Declerations ***************************/
int   adcValue0 = 0, adcValue1 = 0;
float volt0 = 0, volt1 = 0, ActualVoltage0 = 0, ActualVoltage1 = 0;
char _BVolt[6] = { 0 };
char _PVolt[6] = { 0 };

/************** Set and configure ADC PIN functionality ****************/
const adcChannelConfig_st AdcConfig[C_MaxAdcChannels_U8]=
{
  { P0_23, PINSEL_FUNC_1}, /* AD0[0] is on P0.23 second alternative function */
  { P0_24, PINSEL_FUNC_1}, /* AD0[1] is on P0.24 second alternative function */
  { P0_25, PINSEL_FUNC_1}, /* AD0[2] is on P0.25 second alternative function */
  { P0_26, PINSEL_FUNC_1}, /* AD0[3] is on P0.26 second alternative function */
  { P1_30, PINSEL_FUNC_3}, /* AD0[4] is on P1.30 third alternative function */
  { P1_31, PINSEL_FUNC_3}, /* AD0[5] is on P1.31 third alternative function */
  { P0_3,  PINSEL_FUNC_2}, /* AD0[6] is on P0.3  third alternative function */
  { P0_2,  PINSEL_FUNC_2}  /* AD0[7] is on P0.2  third alternative function */
};

/*************** local function declaration ****************************/
static uint32_t adc_GetPclk(void);

/***************************************************************************************************
                         void ADC_Init()
****************************************************************************************************
 * I/P Arguments: none.
 * Return value	: none

 * description :This function initializes the ADC module.
				Refer adc.h for ADC configuration

***************************************************************************************************/
void ADC_Init()
{
	uint32_t v_Pclk_u32;
	 
  /* Enable CLOCK for to controller */
	LPC_SC->PCONP |= (1 << 12);

	v_Pclk_u32 = adc_GetPclk();
	v_Pclk_u32 = M_GetClkDivValue(v_Pclk_u32);

	LPC_ADC->ADCR = ((1<<SBIT_PDN) | v_Pclk_u32);
	
	// no START, no BURST, ADC normal operation, with selected channel
}
  
/***************************************************************************************************
                         uint16_t ADC_GetAdcValue(uint8_t v_adcChannel_u8)
****************************************************************************************************
 * I/P Arguments: uint8_t(channel number).
 * Return value	: uint16_t(12 bit ADC result)

 * description  :This function does the ADC conversion for the Selected Channel
                 and returns the converted 12 bit result
                 The ADC value per bit depends on the resolution of the ADC.
***************************************************************************************************/				 
uint16_t ADC_GetAdcValue(uint8_t v_adcChannel_u8)
{
	uint16_t v_adcResult_u16;
  if(v_adcChannel_u8 < C_MaxAdcChannels_U8)
  {
		/* Select channel is with range, COnfigure the channel for ADC and DO the A/D conversion */ 
	  GPIO_PinFunction(AdcConfig[v_adcChannel_u8].pinNumber,AdcConfig[v_adcChannel_u8].PinFunSel);
    LPC_ADC->ADCR  = (LPC_ADC->ADCR  & 0xFFFFFF00) | (0x01 << v_adcChannel_u8 );	/* set the channel */
	   
		DELAY_us(10);		/* allow the channel voltage to stabilize*/
       
	  util_BitSet(LPC_ADC->ADCR,SBIT_START);			/*Start ADC conversion*/  	
	  while (util_GetBitStatus(LPC_ADC->ADGDR,SBIT_DONE)==0);	/* wait till conversion completes */
	  
		v_adcResult_u16 = (LPC_ADC->ADGDR >> SBIT_RESULT) & 0xfff; /*Read the 12bit adc result*/
  }
  else
  {
		/* Channel is out of range, return 0*/
    v_adcResult_u16 = 0;  
  }        
	
	return(v_adcResult_u16);                        			/* Return the 12-bit result */
}

/***************************************************************************************************
						static uint32_t adc_GetPclk( void )
 ***************************************************************************************************
 * I/P Arguments: -
 * Return value	: v_Pclk_u32 in terms of system frequency

 * description  :This function is used to calculate pclock in terms of system frequency.

***************************************************************************************************/
static uint32_t adc_GetPclk(void)
{
	uint32_t v_AdcPclk_u32,v_Pclk_u32;
	/** 
		PCLKSELx registers contains the PCLK info for all the clock dependent peripherals.
	  Bit6,Bit7 contains the Uart Clock(ie.UART_PCLK) information.
	  The UART_PCLK and the actual Peripheral Clock(PCLK) is calculated as below.
	  (Refer data sheet for more info)
	   
	  ADC_PCLK    PCLK
			0x00       SystemFreq/4        
			0x01       SystemFreq
			0x02       SystemFreq/2
			0x03       SystemFreq/8   
	**/

	v_AdcPclk_u32 = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
	
	switch( v_AdcPclk_u32 )
	{
		case 0x00:
								v_Pclk_u32 = SystemCoreClock/4;
								break;
		case 0x01:
								v_Pclk_u32 = SystemCoreClock;
								break; 
		case 0x02:
								v_Pclk_u32 = SystemCoreClock/2;
								break; 
		case 0x03:
								v_Pclk_u32 = SystemCoreClock/8;
								break;
	}
	return (v_Pclk_u32);
}

/*****************************************************************************************
*                                   Read ADC Values																			 *
*****************************************************************************************/
#if  _PP_READ_BATT_SOLAR_VOLT 

void Read_ADCchannels(void)
{
	/* calculate battry voltage */
	adcValue0 = ADC_GetAdcValue(0); // Read the ADC value of channel zero
	volt0 = (adcValue0*3.3)/4095;
	if(volt0>2.38){ /* if voltage is greater than 15v,then on Disply shows battery voltage 15v */
		volt0=2.38;
	}
	ActualVoltage0 = (20.00*volt0)/3.3;
	
	DELAY_ms(30);
	
	console_log("ADC step : %4d  ",adcValue0);
	console_log("ADC vout : %4f ",volt0);
	console_log("ADC vin : %4f \n\r ",ActualVoltage0);
	
	/* calculate Solar voltage */
	adcValue1 = ADC_GetAdcValue(1); // Read the ADC value of channel one
	volt1 = (adcValue1*3.3)/4095;
	ActualVoltage1 = (30.00
	*volt1)/3.3;
	
	
	/* cover to string to Display on LCD */
	ftoa_signed(ActualVoltage0,_BVolt,2);
	ftoa_signed(ActualVoltage1,_PVolt,2);
	
	/* Print values on terminal */
//	console_log("ADC0 Value:%4d  Volt0:%2f ADC1 Value:%4d  Volt1:%2f\n\r",adcValue0,volt0,adcValue1,volt1);     // Send the value on UART
	console_log("Battery:%2f  Power:%2f \n\r",ActualVoltage0,ActualVoltage1);
	
	
	
	







	/* below code for calculate battery percentage (%) */
	
//	/* this variables are use to calculate battry percentage (%) */
//	int i = 0;
//	float Max_volt = 12.30 , Min_volt = 9.30; 
//	float Total_volt = 0, Current_volt = 0; 
//	float volt_percent = 0, TotalVoltage = 0; 

//	lcd_enable(); // enabel LCD 
//	
////	console_log("Max_volt : %f \n\r",Max_volt);
////	console_log("Min_volt : %f \n\r",Min_volt);
//	Total_volt = ( Max_volt - Min_volt);
////	console_log("Total Volt : %f \n\r",Total_volt);
//	
//	for (i=10;i>0;i--) 
//	{
//		adcValue0 = ADC_GetAdcValue(0);  // Read the ADC value of channel zero
////		console_log("ADC step : %d \n\r",adcValue0);
//		volt0 = (adcValue0*3.3)/4095;
//		ActualVoltage0 = (21.23*volt0)/3.3;
//		
//// TO Do 
//// need to correct 20.14 correction factor 
//// 8.96v for [ volt0 > 1.47 ] cutoff voltage
//// 9.30v for [ volt0 > 1.52 ] cutoff voltage
//// ActualVoltage0 = (20.14*volt0)/3.3;
//		
////		console_log("ActualVoltage0 : %f \n\r",ActualVoltage0);
//		TotalVoltage = TotalVoltage + ActualVoltage0;
////		console_log("TotalVoltage : %f \n\r",TotalVoltage);
//	}
//	
//	TotalVoltage = TotalVoltage/i;
////	console_log("TotalVoltage average : %f \n\r",TotalVoltage);
//	
////	if (ActualVoltage0 > Max_volt)
////	 console_log("battery percentage : 100 \n\r");
////	else if (ActualVoltage0<Min_volt)
////	  console_log("battery percentage : 0 \n\r");
////	else {
////		Current_volt = ActualVoltage0-Min_volt;
////		console_log("Current_volt : %f \n\r",Current_volt);
////		volt_percent = (Current_volt/Total_volt)*100;
////		console_log("battery percentage : %2f \n\r",volt_percent);
////	}
//   
//	DELAY_ms(30);
//	
//	adcValue1 = ADC_GetAdcValue(1); 	// Read the ADC value of channel one
////	console_log("ADC step : %d",adcValue0);
//	volt1 = (adcValue1*3.3)/4095;
//	ActualVoltage1 = (38.89*volt1)/3.3;

//	ftoa_signed(ActualVoltage0,_BVolt,2);
//	ftoa_signed(ActualVoltage1,_PVolt,2);

////	console_log("ADC0 Value:%4d  Volt0:%2f ADC1 Value:%4d  Volt1:%2f\n\r",adcValue0,volt0,adcValue1,volt1);     // Send the value on UART
//	console_log("Battery:%2f  Power:%2f \n\r",ActualVoltage0,ActualVoltage1);

}

#endif /*_PP_READ_BATT_SOLAR_VOLT && _PP_READ_ADC*/
#endif /*_PP_ADC*/

#endif



