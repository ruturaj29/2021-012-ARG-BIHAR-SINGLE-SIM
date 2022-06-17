
#ifndef _I2C0_H_
#define _I2C0_H_
 
/************************************************************************************************* 
																	ALL header files 
**************************************************************************************************/
#include "LPC17xx.h"
#include "stdutils.h"
#include <stdbool.h>

/***************************************************************************************************
                              macros defines 
***************************************************************************************************/
#define MODE_100kbps 			 100000
#define MODE_400kbps 			 400000
#define MODE_1Mbps 				1000000

#define I2SCLH_HS_SCLH		0x00000008  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000008  /* Fast Plus I2C SCL Duty Cycle Low Reg */

#define I2SCLH_SCLH				0x00000080  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL				0x00000080  /* I2C SCL Duty Cycle Low Reg */
 
 
 
/***************************************************************************************************
                             Function Prototypes
***************************************************************************************************/
//void i2c0_init(uint32_t i2c_freq, uint8_t int_pri);
void I2C0_Init(bool fastMode);
void i2c0_enable(void);
unsigned char i2c0_start(void);
void i2c0_stop(void);
unsigned char i2c0_Address(unsigned char add);
unsigned char i2c0_Write(char dat);
unsigned char i2c0_Read(char *retdat,char ack );
												    
#endif /* _I2C0_H_ */





