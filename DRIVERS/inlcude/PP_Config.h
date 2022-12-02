/* 
 * File:   PP_Config.h
 * Author: skymet
 *
 * Created on November 8, 2020, 5:33 PM
 */

#ifndef PP_CONFIG_H
#define	PP_CONFIG_H  4004	/* Revision ID */

//#include "All_Header.h"

#ifdef	__cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------/
/  Skysense ARG  : system module configuration file
/----------------------------------------------------------------------------/
/
/ CAUTION! Do not forget to make clean the project after any changes to
/ the configuration options.
/
/----------------------------------------------------------------------------*/
//#ifndef _PPCONF
//#define _PPCONF 4004	/* Revision ID */

#define M4G_ULTRA_SONIC 0 /* 0: operation not allowed or 1: operation allowed */
/* Note: To use Given Firmware for mahavedh ultrasoinc 
 * make sure M4G_ULTRA_SONIC must be equal to 1
 * Firmware version : majour.minor.build 
 * Firmware version : M4US.0.1
 */

#if M4G_ULTRA_SONIC
#define _FIRMWARE_STRING__  "M4US.0.1"
#endif 
 
#define M4G_CUP_ANOMOMETER 0 /* 0: operation not allowed or 1: operation allowed */
/* Note: To use Given Firmware for mahavedh Cup_anomometer  
 * make sure M4G_CUP_ANOMOMETER must be equal to 1
 * Firmware version : majour.minor.build 
 * Firmware version : M4CA.0.1 
 */

#if M4G_CUP_ANOMOMETER
#define _FIRMWARE_STRING__  "M4CA.0.1"
#endif

#define ARG_BIHAR 1 /* 0: operation not allowed or 1: operation allowed */
/* Note: To use Given Firmware for Bihar ARG Project  
 * make sure ARG_BIHAR must be equal to 1
 * Firmware version : majour.minor.build 
 * Firmware version : ARG.0.1 
 */
 
#if ARG_BIHAR
#define _FIRMWARE_STRING__  "ARG.2.4"
#endif
 
#if ARG_BIHAR
#define M4G_ULTRA_SONIC 		0
#define M4G_CUP_ANOMOMETER 	0
#define	_PP_ATRH						0
#define _PP_EEPROM          1 

#endif
 

/*---------------------------------------------------------------------------/
/  ADC Functions and Buffer Configurations
/----------------------------------------------------------------------------*/

#define	_PP_ADC	 1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all ADC Function & ADC.c make sure _PP_ADC must be equal to 1 
 * (i.e _PP_ADC = 1)
 * When _PP_ADC is set to 1,  ADC is enabled
 * When _PP_ADC is set to 0,  ADC is Disabled
 */
    
#define	_PP_ADC_FUNCTIONS	1   /* 0: operation not allowed or 1: operation allowed */
/* When _PP_READ_ADC is set to 1,  Reads the given analog channel[ADC channels]
 * When _PP_READ_ADC is set to 0,  not Read any analog channel[ADC channels]
 */

#define	_PP_READ_BATT_SOLAR_VOLT	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: to use this option make sure that _PP_READ_ADC must be equal to 1
 * (i.e _PP_READ_ADC = 1)  To Read BATTery & Solar Voltage. 
 * When _PP_READ_BATT_SOLAR_VOLT is set to 1,  Read the Battery & Solar Voltages.
 * When _PP_READ_BATT_SOLAR_VOLT is set to 0,  not Read the Battery & Solar Voltages.
 */
    
/*---------------------------------------------------------------------------/
/  ATRH Functions and Buffer Configurations
/----------------------------------------------------------------------------*/
#if !ARG_BIHAR
#define	_PP_ATRH	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all ATRH Function & ATRH.c make sure _PP_ATRH must be equal to 1 
 * (i.e _PP_ATRH = 1)
 * When _PP_ATRH is set to 1,  ATRH is enabled
 * When _PP_ATRH is set to 0,  ATRH is Disabled
 */    
    
#define _PP_ATRH_CONFIG  1  /* 0: operation not allowed or 1: operation allowed */
/* When _PP_ATRH_CONFIG is set to 1,  ATRH is configured.
 * When _PP_ATRH_CONFIG is set to 0,  ATRH is not configured.
 */
    
#define _PP_READ_ATRH 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_READ_ATRH is set to 1,  ATRH is starts to take the sensors 
 * reading Temperature & Humidity.
 * When _PP_READ_ATRH is set to 0,  ATRH is not taking sensors reading.
 */ 
    
#define _PP_AVERAGE_ATRH_SAMPLE 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_AVERAGE_ATRH_SAMPLE is set to 1,  take Average of ATRH samples.
 * When _PP_AVERAGE_ATRH_SAMPLE is set to 0,  not take Average of ATRH samples.
 */
 
#endif 
/*---------------------------------------------------------------------------/
/  LCD Scroll Functions and Buffer Configurations
/----------------------------------------------------------------------------*/

#define	_PP_LCD_SCROLL	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all LCD_Scroll Function & LCD_Scroll.c make sure _PP_LCD_SCROLL must be equal to 1 
 * (i.e _PP_LCD_SCROLL = 1)
 * When _PP_LCD_SCROLL is set to 1,  LCD Scroll is enabled
 * When _PP_LCD_SCROLL is set to 0,  LCD Scroll is Disabled
 */
    
#define _PP_LCD_Scroll_Display 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_LCD_Scroll_Display is set to 1,  LCD Display is scroll.
 * When _PP_LCD_Scroll_Display is set to 0,  LCD Display is not scroll.
 * 
 * Note: make sure that _PP_EXTERNAL_INTERRUPT_0 must be equal to 1(i.e _PP_EXTERNAL_INTERRUPT_0 = 1)
 * 
 */

#define _PP_EXTERNAL_INTERRUPT_1 1 /* 0: operation not allowed or 1: operation allowed */
/* Note: Keep  _PP_EXTERNAL_INTERRUPT_1 always equal to 1 (i.e _PP_EXTERNAL_INTERRUPT_1 = 1)
 * this is a critical section of Interrupt_1 ISR for push button. when push button is
 * pressed its generate an interrupt and LCD Display is Scroll.
 * 
 */  
    
/*---------------------------------------------------------------------------/
/  Rain Gauge Functions and Buffer Configurations
/----------------------------------------------------------------------------*/

#define	_PP_Rain_Gauge	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all Rain_Gauge Function & Rain_Gauge.c make sure _PP_Rain_Gauge must be equal to 1 
 * (i.e _PP_Rain_Gauge = 1)
 * When _PP_Rain_Gauge is set to 1,  Rain Gauge is enabled
 * When _PP_Rain_Gauge is set to 0,  Rain Gauge is Disabled
 */
    
#define _PP_RAIN_FALL 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_RAIN_FALL is set to 1,  Rain Fall is Calculate.
 * When _PP_RAIN_FALL is set to 0,  Rain Fall is not Calculate.
 * 
 * Note: make sure that _PP_EXTERNAL_INTERRUPT_3 must be equal to 1(i.e _PP_EXTERNAL_INTERRUPT_3 = 1)
 * 
*/
 
#define _PP_EXTERNAL_INTERRUPT_3 1 /* 0: operation not allowed or 1: operation allowed */
/* Note: Keep  _PP_EXTERNAL_INTERRUPT_3 always equal to 1 (i.e _PP_EXTERNAL_INTERRUPT_3 = 1)
 * this is a critical section of Interrupt_3 ISR for calculate Rain Fall. when its Raining 
 * an ISR is generate and its calculate the no. of tips for rain drops. depending upon no. 
 * of tips. the Rain fall is calculated.
 */
    
/*---------------------------------------------------------------------------/
/  Ultrasonic Functions and Buffer Configurations
/----------------------------------------------------------------------------*/
#if M4G_ULTRA_SONIC
#define	_PP_ULTRASONIC	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all Ultrasonic Function & Ultrasonic.c make sure _PP_ULTRASONIC must be equal to 1 
 * (i.e _PP_ULTRASONIC = 1)
 * When _PP_ULTRASONIC is set to 1,  Ultrasonic is enabled
 * When _PP_ULTRASONIC is set to 0,  Ultrasonic is Disabled
 */
    
#define _PP_ULTRASONIC_CONFIG 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_ULTRASONIC_CONFIG is set to 1,  Ultrasonic is configured.
 * When _PP_ULTRASONIC_CONFIG is set to 0,  Ultrasonic is not configured.
 */
    
#define _PP_READ_WINDSENSOR 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_READ_WINDSENSOR is set to 1,  Ultrasonic start to take the
 * sensors reading, wind Speed     : min, mean, max
 *                  wind Direction : min, mean, max 
 * When _PP_READ_WINDSENSOR is set to 0,  Ultrasonic is  not taking sensors reading.
 */
    
#define _PP_AVERAGE_WIND_SAMPLE 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_AVERAGE_WIND_SAMPLE is set to 1,  take Average of WIND samples.
 * When _PP_AVERAGE_WIND_SAMPLE is set to 0,  not take Average of WIND samples.
 */  



#else
#define	_PP_ULTRASONIC	      	0
#define _PP_ULTRASONIC_CONFIG 	0
#define _PP_READ_WINDSENSOR 		0
#define _PP_AVERAGE_WIND_SAMPLE 0


#endif   
/*---------------------------------------------------------------------------/
/  Cup-Anomometer Functions and Buffer Configurations
/----------------------------------------------------------------------------*/
#if M4G_CUP_ANOMOMETER
#define	_PP_CUP_ANOMOMETER	1   /* 0: operation not allowed or 1: operation allowed */
/* Note: To use all Cup-Anomometer Function & Cup-Anomometer.c make sure _PP_CUP_ANOMOMETER must be equal to 1 
 * (i.e _PP_CUP_ANOMOMETER = 1)
 * When _PP_ULTRASONIC is set to 1,  Ultrasonic is enabled
 * When _PP_ULTRASONIC is set to 0,  Ultrasonic is Disabled
 */
 
#define _PP_EXTERNAL_INTERRUPT_0 1 /* 0: operation not allowed or 1: operation allowed */
/* Note: Keep  _PP_EXTERNAL_INTERRUPT_0 always equal to 1 (i.e _PP_EXTERNAL_INTERRUPT_0 = 1)
 * this is a critical section of Interrupt_0 ISR for calculate Rain Fall. when its Raining 
 * an ISR is generate and its calculate the no. of tips for wind speed. depending upon no. 
 * of tips. the wind speed is calculated.
 */ 


#define _PP_CUP_ANOMOMETER_DISPLAY 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_ULTRASONIC_CONFIG is set to 1,  Cup-Anomometer is configured.
 * When _PP_ULTRASONIC_CONFIG is set to 0,  Cup-Anomometer is not configured.
 */
    
#define _PP_READ_CUP_ANOMOMETER 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_READ_CUP_ANOMOMETER is set to 1,  Cup-Anomometer start to take the
 * sensors reading, wind Speed     : min, mean, max
 *                  wind Direction : min, mean, max 
 * When _PP_READ_CUP_ANOMOMETER is set to 0,  Cup-Anomometer is  not taking sensors reading.
 */
    
#define _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE is set to 1,  take Average of WIND samples.
 * When _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE is set to 0,  not take Average of WIND samples.
 */       
 
#else
#define	_PP_CUP_ANOMOMETER											0
#define _PP_EXTERNAL_INTERRUPT_0 								0
#define _PP_CUP_ANOMOMETER_DISPLAY 							0
#define _PP_READ_CUP_ANOMOMETER 								0
#define _PP_CUP_ANOMOMETER_AVERAGE_WIND_SAMPLE 	0

#endif

/*---------------------------------------------------------------------------/
/  SD card initialization Functions and Configurations
/----------------------------------------------------------------------------*/
 
#define _PP_SDCARD_INIT 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_SDCARD_INIT is set to 1,  Initialize the SD card .
 * When _PP_SDCARD_INIT is set to 0,  Does not initialize the SD card.
 */ 
 
# define _PP_SDCARD_WRITE 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_SDCARD_WRITE is set to 1,  Writr to the SD card .
 * When _PP_SDCARD_WRITE is set to 0,  Does not Write to the SD card.
 */
 
# define _PP_SDCARD_READ 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_SDCARD_READ is set to 1,  Read to the SD card .
 * When _PP_SDCARD_READ is set to 0,  Does not Read to the SD card.
 */

/*---------------------------------------------------------------------------/
/  GSM - GPRS initialization Functions and Configurations
/----------------------------------------------------------------------------*/

#define _PP_GSM_GPRS_INIT 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_GSM_GPRS_INIT is set to 1,  Initialize the GSM .
 * When _PP_GSM_GPRS_INIT is set to 0,  Does not initialize the GSM.
 */
 

 #define _PP_SMS_INIT 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_SMS_INIT is set to 1,  Initialize the SMS .
 * When _PP_SMS_INIT is set to 0,  Does not initialize the SMS.
 */
 

/*---------------------------------------------------------------------------/
/  EEPROM initialization Functions and Configurations
/----------------------------------------------------------------------------*/

#define _PP_EEPROM 1 /* 0: operation not allowed or 1: operation allowed */
/* When _PP_EEPROM is set to 1,  Initialize the EEPROM .
 * When _PP_EEPROM is set to 0,  Does not initialize the EEPROM.
 */

    
		
//#endif /* _PPCONF */
    
#ifdef	__cplusplus
}
#endif

#endif	/* PP_CONFIG_H */

