
/************************************************************************************************* 
																	ALL header files 
**************************************************************************************************/

//#include "stdutils.h"
//#include "lcd.h"    
//#include "gprs.h"
//#include "delay.h"	
//#include "adc.h"


/**************************************************************************************************
                           extern all global variables 
***************************************************************************************************/

extern char  _LCDRain[8] , _Rain[8] , _PeriodicRain[8]; 
extern double  LCDrain;
extern volatile unsigned int LCDnumTipsRain ;


/***************************************************************************************************
                             Function Prototypes
***************************************************************************************************/
void myExtIntrIsr_3( void );
void calcRainFall(void);
void _24hr_RainFall(void);
void Yearly_Rain(void);
void Reset24HrRainAt8_16(void);
void Reset24HrRainIfBattLow(void);




