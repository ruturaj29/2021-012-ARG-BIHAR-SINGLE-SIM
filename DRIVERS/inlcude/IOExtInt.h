/****************************************************************************
 *   $Id:: extint.h 5670 2010-11-19 01:33:16Z usb00423                      $
 *   Project: NXP LPC17xx EXTINT example
 *
 *   Description:
 *     This file contains EINT code header definition.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef __IOEXTINT_H 
#define __IOEXTINT_H

#include "stdutils.h"

#define  NOPULL  	0
#define  PULLUP  	1
#define  PULLDOWN  	2

//#define  INPUT 		0
//#define  OUTPUT 	1

//#define  FALLING	0
//#define  RISING  	1

//#define  LOW		0
//#define  HIGH		1

extern void IOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir );
extern void IOSetValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal );
extern void IOSetPull(uint32_t portNum, uint32_t bitPosi, uint32_t dir);
extern uint32_t IOGetValue (uint32_t portNum, uint32_t bitPosi);
void IOSetInterrupt (  uint32_t portNum, uint32_t bitPosi, uint32_t dir );
void IOClearInterrupt( void );
uint32_t IOCheckInterrupts ( uint32_t portNum, uint32_t dir);

#endif /* end __EXTINT_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/

