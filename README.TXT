BIHAR PROJECT Module Source code

Sensors :
	  		Rain Guage - Digital
			EC20 GSM module ( 4G quectel module )

Debugging :
			UART 2

Communication :
			4G
			HTTP POST

POST URL :
			IOT.com URL

Date :
			25 Sep 2021

Project : 		
			Skysense tech pvt.ltd.


Hardware revision : 	
			SKYSENSE TECH ST-0301

Firmware : 		
			Sleep Mode 
			
Peripheral use :
			Uart 1    --> use for GSM communications
			Uart 2    --> use for debugging
			EXT_INT_3 --> Rain guage
			EXT_INT_1 --> Push button [LCD Display Scroll]
			ADC CH_0  --> battry voltage
			ADC CH_1  --> Solar voltage

Poject Major Changes :

1. Sleep mode implement for EC20 GSM module, DTR pin use for module Wakeup and Sleep.
2. SD card FatFS file system changes completly. update with new FatFs filesystem 2021 by chan.
3. major changes are done in main.c GPRS.c log.c
4. SMS implementation added in code. need some modification.


Date : 14 june 2021

1. Read SMS is removed from LCD Scroll and added in Test log.
2. pendinglog are not send if DL reset. removed that section from code.[ this is due to flag reset after reset ]
3. Error 703 occuring continusly, so if error occure then power on/off the gsm module as done for error 714.
4. majore changes are done for PP_config is implement. [same code is for Ultrasonic,cup anomometer,bihar project]

  
