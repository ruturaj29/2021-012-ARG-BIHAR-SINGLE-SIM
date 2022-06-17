
/************************ include Header files *******************************/
#include <lpc17xx.h>
#include "i2c0.h"
//#include "type.h"
#include "stdutils.h"
#include "uart.h"
 
/************************ Globle Macro Define *******************************/
// IC2 control bits
#define AA      (1 << 2)
#define SI      (1 << 3)
#define STO     (1 << 4)
#define STA     (1 << 5)
#define I2EN    (1 << 6)
 

/*************DEBUG**************************************************************************************/
LPC_I2C_TypeDef*  regs;

 
/*********************************************************************************
*                                  i2c0 init																		 *
**********************************************************************************/
void I2C0_Init(bool fastMode) 
{
	
	LPC_SC->PCONP |= (1 << 7);
  
	/* set PIN P0.27 and PIN P0.28 to I2C0 SDA and SCL */
	/* PIN function to 01 on both SDA and SCL. */
	LPC_PINCON->PINSEL1 &= ~((0x03<<22)|(0x03<<24)); // make pin as 00 [ clear bit]
	LPC_PINCON->PINSEL1 |= ((0x01<<22)|(0x01<<24));	 // make pin as 01 [ set bit ]
 
	/*--- Clear flags ---*/
	LPC_I2C0->I2CONCLR = AA | SI | STA | I2EN;    

	/*--- Reset registers ---*/
	if(fastMode)
	{
		
		LPC_PINCON->I2CPADCFG |= ((0x1<<0)|(0x1<<2));
		LPC_I2C0->I2SCLL   = I2SCLL_HS_SCLL;
		LPC_I2C0->I2SCLH   = I2SCLH_HS_SCLH;
		print_DebugMsg("I2C fastMode configure \n\r");
	} else {
		LPC_PINCON->I2CPADCFG &= ~((0x1<<0)|(0x1<<2));
		LPC_I2C0->I2SCLL   = I2SCLL_SCLL;
		LPC_I2C0->I2SCLH   = I2SCLH_SCLH;
	}

//	/* Install interrupt handler */
//	NVIC_EnableIRQ(I2C0_IRQn);

//	LPC_I2C0->I2CONSET = I2EN;

}

/*********************************************************************************
*                     i2c0 enable				            														 *
**********************************************************************************/
void i2c0_enable(void)
{
	/*--- enable the I2C (master only) ---*/ 
    LPC_I2C0->I2CONSET = I2EN;
}

/*********************************************************************************
*                     i2c0 start				            														 *
**********************************************************************************/
unsigned char i2c0_start(void)
{
		LPC_I2C0->I2CONSET |= 1<< 5;	//START I2C0
		while (!(LPC_I2C0->I2CONSET & (1<<3)));	 // wait until done

		return (LPC_I2C0->I2STAT);
}

/*********************************************************************************
*                     i2c0 stop				              														 *
**********************************************************************************/
void i2c0_stop(void)
{
		LPC_I2C0->I2CONSET |= 1<<4;	//STOP I2C
		LPC_I2C0->I2CONCLR = 1<< 3 ;		//clear SI

	    while (LPC_I2C0->I2CONSET & (1<<4));	//wait until H/w stops I2c
}

/*********************************************************************************
*                     i2c0 Address				           														 *
**********************************************************************************/
unsigned char i2c0_Address(unsigned char add) {
 	
 	LPC_I2C0->I2DAT = add;	//the address
 	LPC_I2C0->I2CONCLR = 1<<5;	//clear start
 	LPC_I2C0->I2CONCLR = 1<< 3;	//clear SI

 	while (!(LPC_I2C0->I2CONSET & (1<<3)));	//wait until change in status
 
 	return (LPC_I2C0->I2STAT);
 	
}

 
/*********************************************************************************
*               i2c0 Write [Function to Write data to slave]				  					 *
**********************************************************************************/
unsigned char i2c0_Write(char dat) {
 		
 	LPC_I2C0->I2DAT = dat ;	//new data
 	LPC_I2C0->I2CONCLR = 1<< 3 ; // clear SI

	while (!(LPC_I2C0->I2CONSET & (1<<3)));	//wait until change in SI status
 
 	return (LPC_I2C0->I2STAT);	//the data
 }

/*********************************************************************************
*               i2c0 Read [Function to Read data from slave]				  					 *
**********************************************************************************/
unsigned char i2c0_Read(char *retdat,char ack ) {

 	
 	if (ack) LPC_I2C0->I2CONSET =1<<2;	//assert AA -ACK more bytes to come
 	     	else LPC_I2C0->I2CONCLR = 1<<2;	//No ack - last byte

 	LPC_I2C0->I2CONCLR = 1<< 3 ; // clear SI

 	while (!(LPC_I2C0->I2CONSET & (1<<3)));	//wait until change in SI status
 	*retdat = (uint8_t) (LPC_I2C0->I2DAT & 0xFF);
	
	return (LPC_I2C0->I2STAT & 0xF8);
 
 }

 

 
