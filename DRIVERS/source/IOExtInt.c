#include "lpc17xx.h"
//#include "type.h"
#include "stdutils.h"
#include "IOExtInt.h"
#include "uart.h"


static LPC_GPIO_TypeDef (* const LPC_GPIO[5]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4  };


void IOSetValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal )
{
  if (bitVal == 0)
  {
	  LPC_GPIO[portNum]->FIOCLR = (1<<bitPosi);
  }
  else if (bitVal >= 1)
  {
	  LPC_GPIO[portNum]->FIOSET = (1<<bitPosi);
  }
}

void IOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir )
{
  if(dir)
	LPC_GPIO[portNum]->FIODIR |= 1<<bitPosi;
  else
	LPC_GPIO[portNum]->FIODIR &= ~(1<<bitPosi);
}

void IOSetPull( uint32_t portNum, uint32_t bitPosi, uint32_t dir)
{
//	 print_DebugMsg("In IOSetPull Function \n\r");

	if (dir == 0) {								//no Pull
		dir = 10;
	} else if(dir == 1){   						//Pull up
		dir = 00;
	} else if(dir == 2){						//Pull down
		dir = 11;
	}

	switch (portNum)
	{
		case 0:
			  //	 print_DebugMsg("PORT0 \n\r");

			if (bitPosi < 16 ) {
				bitPosi = bitPosi * 2;
				LPC_PINCON->PINMODE0 |= dir<<bitPosi;
			} else if (bitPosi > 15){
				bitPosi = bitPosi - 16;
				bitPosi = bitPosi * 2;
				LPC_PINCON->PINMODE1 |= dir<<bitPosi;
			}

		break;

		case 1:
			//    print_DebugMsg("PORT1 \n\r");

			if (bitPosi < 16 ) {
				bitPosi = bitPosi * 2;
				LPC_PINCON->PINMODE2 |= dir<<bitPosi;
			} else if (bitPosi > 15){
				bitPosi = bitPosi - 16;
				bitPosi = bitPosi * 2;
				LPC_PINCON->PINMODE3 |= dir<<bitPosi;
			}

		break;

		case 2:
			 //   print_DebugMsg("PORT2 \n\r");

			if (bitPosi < 14 ) {
				bitPosi = bitPosi * 2;
				LPC_PINCON->PINMODE4 |= dir<<bitPosi;
			}

		break;

		case 3:
		//	   print_DebugMsg("PORT3 \n\r");

			if (bitPosi == 25){
				LPC_PINCON->PINMODE7 |= dir<<18;
			}else if (bitPosi == 26){
				LPC_PINCON->PINMODE7 |= dir<<20;
			}

		break;

		case 4:
		// 	 print_DebugMsg("PORT4 \n\r");

			if (bitPosi == 28){
				LPC_PINCON->PINMODE9 |= dir<<24;
			}else if (bitPosi == 29){
				LPC_PINCON->PINMODE9 |= dir<<26;
			}

	}
}

uint32_t IOGetValue (uint32_t portNum, uint32_t bitPosi)
{
    uint32_t val;
    LPC_GPIO[portNum]->FIOMASK = ~(1<<bitPosi);
    val = LPC_GPIO[portNum]->FIOPIN;
    val = val >> bitPosi;
    LPC_GPIO[portNum]->FIOMASK = 0x00000000;
    return val;
}

void IOSetInterrupt (  uint32_t portNum, uint32_t bitPosi, uint32_t dir )
{
	//  Dir is 0 for falling edge interrupt and 1 for rising edge interrupt
	//	 print_DebugMsg("In IOSetInterrupt Function \n\r");

	if (portNum == 0)
	{
	//	 print_DebugMsg("PORT0 \n\r");

		if (dir == 0)
		{
		//	 print_DebugMsg("Direction 0 \n\r")
			LPC_GPIOINT->IO0IntEnF |= (1<<bitPosi);

		}
		else if (dir == 1)
		{
		//	 print_DebugMsg("Direction 1 \n\r");
			LPC_GPIOINT->IO0IntEnR |= (1<<bitPosi);
		}
	}
	else if (portNum == 2)
	{
//		 print_DebugMsg("PORT2 \n\r");

		if (dir == 0)
		{
	   //		 print_DebugMsg("Direction 0 \n\r")			
			LPC_GPIOINT->IO2IntEnF |= (1<<bitPosi);
		}
		else if (dir == 1)
		{
	//	    print_DebugMsg("Direction 1 \n\r");
			LPC_GPIOINT->IO2IntEnR |= (1<<bitPosi);
		}
	}

	NVIC_EnableIRQ(EINT3_IRQn);
//	NVIC_EnableIRQ(EINT0_IRQn);
}

void IOClearInterrupt( void )
{
	LPC_GPIOINT->IO0IntClr = 0x7FFF8FFF;
	LPC_GPIOINT->IO2IntClr = 0x3fff;
}

uint32_t IOCheckInterrupts ( uint32_t portNum, uint32_t dir)
{
	//  Dir is 0 for falling edge interrupt and 1 for rising edge interrupt
		//	 print_DebugMsg("In IOCheckInterrupts Function \n\r");

	if (portNum == 2)
	{
	//	 print_DebugMsg("PORT0 \n\r");

		if (dir == 0)
		{
	 //  		 print_DebugMsg("Direction 0 \n\r")			

			return LPC_GPIOINT->IO2IntStatF;
		}
		else if (dir == 1)
		{
	//		 print_DebugMsg("Direction 1 \n\r")			

			return LPC_GPIOINT->IO2IntStatR;
		}
	}
	else if (portNum == 0)
	{
	//	 print_DebugMsg("PORT2 \n\r");

		if (dir == 0)
		{
	 //		 print_DebugMsg("Direction 0 \n\r")			

			//return LPC_GPIOINT->IO0IntStatF;
			return LPC_GPIOINT->IO0IntStatF;

		}
		else if (dir == 1)
		{
	 	//	 print_DebugMsg("Direction 1 \n\r")			

			//return LPC_GPIOINT->IO0IntStatR;
			return LPC_GPIOINT->IO0IntStatR;

		}
	}
	return FALSE;
}
