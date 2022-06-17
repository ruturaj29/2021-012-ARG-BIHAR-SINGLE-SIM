
/***********************************************************************************************
                             	USER Defined header files 
************************************************************************************************/	
#include "All_HeaderFiles.h"

/***********************************************************************************************/

/*  Watchodog time out is 10 minutes, as calculated
 *  there will be a wait time for response module on command. as calculated it is 600 seconds 
 *  2100000000	==> 35 minutes
 *   600000000  ==>	10 minutes
 *	 240000000  ==> 04 minutes
 *	 180000000	==> 03 minutes
 *   120000000  ==> 02 minutes
 *    60000000  ==> 01 minutes
 */

#define WDT_TIMEOUT  	180000000

/***********************************************************************************************/

/***********************************************************************
                           Global Variables
***********************************************************************/

/******************* variables use in main ****************************/	
//FATFS fs1;
rtc_t rtc;	   										/* rtc structure declare */
uint8_t returnStatus = 0, GPSCheck = 1;

/******************* SD card related variables ************************/
int Card_Failed = 0;
char _buffer[512] = {'\0'}; 			// main buffer for Storing as a packet sending to Portal
char iniContent[512] = {'\0'}; 		// storage for ini contemnt from sd card
extern char USBbuffer[512];
extern bool LCD_Enable;
/******************* RTC realted variables ****************************/
extern volatile uint32_t alarm_on;

/******************* EEPROM realted variables *************************/
extern uint32_t   eeprom_address_TH1, eeprom_address_TH2, 
	                eeprom_address_master_number, eeprom_address_slave_number1,
                  eeprom_address_slave_number2,
									eeprom_address_Rain;
extern double rain;
extern int  MaxRain_threshold_val1 , MaxRain_threshold_val2 ;
extern char master_Number[20];
extern char slave_Number1[20];
extern char slave_Number2[20];
extern char Mobile_no_1[20];

extern uint32_t numBlks, blkSize;
extern uint8_t inquiryResult[INQUIRY_LENGTH];
extern uint32_t detector;
/***********************************************************************************************/

/*********** set Interrupt Priority function ***************************/
void NVIC_SetPriorities( void )
{
	NVIC_SetPriority(UART1_IRQn,0); // gsm comm uart
	//NVIC_SetPriority(UART0_IRQn,1); // sonic sensor
	NVIC_SetPriority(RTC_IRQn  ,2);	// one minute alarm interrupt
	//NVIC_SetPriority(EINT3_IRQn,3);	// Ext interrupt 3 - rainguage
	//NVIC_SetPriority(EINT2_IRQn,4);	// Ext interrupt 0 - cup anemmometer
	NVIC_SetPriority(EINT1_IRQn,5);	// Ext interrupt 1 - button	

	/* Enable IRQ routiens */
	NVIC_EnableIRQ(RTC_IRQn);				// Enable RTC IRQ routine

}
/***********************************************************************************************/

/*********** configure all GPIO pins function *************/
void Configure_GPIO_Peripherals ( void )
{
	/* Configure GSM vlotage regulator pin */
	GPIO_PinFunction(GSM_VOLTAGE_REG_EN,PINSEL_FUNC_0);				// Configure Pin for Gpio
	GPIO_PinDirection(GSM_VOLTAGE_REG_EN,OUTPUT);       			// Configure the pin as OUTPUT

	/* Configure DTR pin for SLEEP Mode */
	GPIO_PinFunction(MCU_DTR,PINSEL_FUNC_0);									// Configure Pin for Gpio
	GPIO_PinDirection(MCU_DTR,OUTPUT);				 								// Configure the pin as OUTPUT
	
	/* Configure SMI select pin as output (SIM1,SIM2) */
	GPIO_PinFunction(SIM_SEL,PINSEL_FUNC_0);									// Configure Pin for Gpio
	GPIO_PinDirection(SIM_SEL,OUTPUT);												// Configure the pin as OUTPUT

//	/* Configure PWRKEY pin for power on/off Mode */					// currently not in use
//	GPIO_PinFunction(GSM_MOD_EN,PINSEL_FUNC_0);								// Configure Pin for Gpio
//	GPIO_PinDirection(GSM_MOD_EN,OUTPUT);											// Configure the pin as OUTPUT
	
  /* Configure RI interrupt pin */													// currently not in use
 // GPIO_PinFunction(P0_11,PINSEL_FUNC_0);  									//Configure Pin for Gpio 
//	GPIO_PinDirection(P0_11,INPUT);														//Configure the SwitchPin as input 

	/* Configure USB EN pin */
	GPIO_PinFunction(P1_19,PINSEL_FUNC_0);										//Configure Pin for Gpio
  GPIO_PinDirection(P1_19,OUTPUT);                          // Configure the pin as OUTPUT
	
	/* Configure RS485 pin*/
//	GPIO_PinFunction(RS485_Enable,PINSEL_FUNC_0);							// Configure Pin for Gpio
//	GPIO_PinDirection(RS485_Enable,OUTPUT);										// Configure the pin as OUTPUT 
	
	/* Configure LCD Enable pin to control display Vcc */			// currently not in use
	GPIO_PinFunction(LCD_EN,PINSEL_FUNC_0);										// Configure Pin for Gpio
	GPIO_PinDirection(LCD_EN,OUTPUT);													// Configure the pin as OUTPUT
	
	/* Configure adc winddirection pin*/											// currently not in use
//	set_ResistorMode(1,31,PINSEL_PINMODE_PULLDOWN); 					// adc winddirection.
//	set_OpenDrainMode(1,31,PINSEL_PINMODE_NORMAL);						// no open drain mode
	
	/* Configure adc battery pin*/
	set_ResistorMode(0,23,PINSEL_PINMODE_PULLDOWN); 					// adc battery.
	set_OpenDrainMode(0,23,PINSEL_PINMODE_NORMAL);						// no open drain mode
	
	/* Configure adc solar power pin*/
	set_ResistorMode(0,24,PINSEL_PINMODE_PULLDOWN); 					// adc solar power.
	set_OpenDrainMode(0,24,PINSEL_PINMODE_NORMAL);						// no open drain mode	

}
/**************************************************************************************************/

/*********** Initialize all Peripherals function *************/
void Initialize_ALL_Peripherals ( void )
{
	/* Initialize all peripherals */
	WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);	/* Initialize WDT, IRC OSC, Reset mode */
	
	WDT_Start(WDT_TIMEOUT);	/* Start watchdog with given timeout */ 
	
	BOD_Init();	/* brown out detect Initialize */
	
	ADC_Init();	/* ADC Initialize */

	/* Setup-Map the controller pins for LCD operation, In 4bit mode D0-D3 are P_NC(Not Connected)
	          RS    RW    EN    D0    D1    D2    D3    D4     D5     D6     D7                 */
	LCD_SetUp(P2_5, P2_6, P2_7, P_NC, P_NC, P_NC, P_NC, P0_19, P0_20, P0_21, P0_22);	  
	
	lcd_enable();	/* Specify the LCD type(2x16, 4x16 etc) for initialization [ Enable 16x2 LCD]*/
	
	RTC_Init();	/* Internal RTC Initialize */

	SysTick_Init();	/* SysTick for 1ms(default) Initialize */

	/* Initialize All UARTs with Specific Baud rate */
//	UART0_Init(19200);	/* Initialize uart0	- Sonic Sensor */
	UART1_Init(115200);	/* Initialize uart1	- GSM Module */
	UART2_Init(115200);	/* Initialize uart2 - Debugging Port */
      
	EEPROM_Init(AT24C512,P0_0,P0_1);	/* Eeprom IC:AT24C16   SDA:P0_0    SCL:P0_1 */ 
}
/**************************************************************************************************/


/**************************************************************************************
 * @brief				Main	: Main Program Body
 * @param[in]		None
 * @return 			int
 **************************************************************************************/	
int main()
{
  
	/* status flags are declaerd */
	uint8_t watchDogReset = 0, brownOutReset = 0 , BatteryLow_Flag = 0, _LogBatteryLog = 1;	
	uint8_t responseStatus=0;
	uint32_t retries = 0 ;// switchStatus = 0;
	DSTATUS disk_state = STA_NOINIT;		/* SDcard flag declare */
						
	
/**************************************************************************************************/

	SystemInit();												/* Clock and PLL configuration */
	
	/*	If BOD was detected set flag to 1*/
	if ((LPC_SC->RSID & 0x9) == 0x8) {
		LPC_SC->RSID |= 0x8;							// check Flag for BOD
		brownOutReset = 1;								// Error writing to a Err log
	}
	else
		brownOutReset = 0;								// BOD reset not happend

	/* Read back TimeOut flag to determine previous timeout reset */
	if (WDT_ReadTimeOutFlag()){  
		WDT_ClrTimeOutFlag();							// Clear WDT TimeOut 
		watchDogReset = 1;
	} 
	else 
		watchDogReset = 0;								// WDT reset not happend
	
 
 
	Initialize_ALL_Peripherals (); 			/* Initialize and setup all peripheral */

	Configure_GPIO_Peripherals (); 			/* Configure ALL peripherals and GPIO pins */ 

	NVIC_SetPriorities();								/* set priorities for interrupt */
	
	 /* initialize GPIO pins as external interrupts */          
	IOSetDir(2, 3, 1);
	IOSetPull(0, 26, 1);
	IOSetInterrupt(0, 26, 0);
	
 
 
/**************************************************************************************************/

	/* Print Welcome message on UART terminal	*/
	print_DebugMsg(" ---- System Reboot ---- \n\r");
	print_DebugMsg("## SkySense Tech Pvt Ltd. \n\r");
	console_log("## Project:	%s \n\r",_PROJECT__); 
	console_log("## Firmware Ver:	%s \n\r",_FIRMWARE_STRING__);
	console_log("## Hardware Ver:	%s \n\r",_HWVERSION__);
	console_log("## Release date:	%s \n\r",_DATE_STRING__);
	
	/* Print Welcome message on LCD 16x2 */
	Init_display_name();								/* Display Skysense on LCD */
	Progress_displayName(1);
	
	ShutDownModule();										/* Disable module voltage regulator */
	startModule_vREG();									/* Enable module volatge regulator */
	DELAY_ms(2000);				/* wait to start module */
	
/**************************************************************************************************/
  #if _PP_ATRH_CONFIG
		WDT_Feed();
		Init_ATRH();												/*Initialize ATRH Sensor --> I2C0 */
		DELAY_ms(1000);
																																								
		/* Congifure ATRH with register adddress */	
		do {
					response = start_ATRH(ATRH_Write_ADD,Config_register_add);
					retries++;
		}while((response != ATRH_INIT_SUCCESSFULL) && (retries!=10) );

		if(response) {	/* If ATRH Communication Failed */
			print_DebugMsg(" ERROR : NO ATRH COMMINICATION \n\r");	
			LCD_Clear();
			LCD_Printf("Check ATRH Conn.");
			DELAY_ms(2000); 		
		}
	#endif /*_PP_ATRH_CONFIG*/


	#if _PP_SDCARD_INIT
	WDT_Feed();
	/* Initialize SD Card and FAT file system */
  print_DebugMsg(" Initializing disk 0 (SDC)... \r\n");
	SysTick_Start();
  for (retries = 5; retries && disk_state; --retries) {
  	disk_state = disk_initialize(0); /* changes are done for USB*/
  }
	if (disk_state) { /* If SD Card failed or Disk initializ failed */
  	print_DebugMsg("Disk initialization failed.\r\n");
		LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("SD Failed..");
		DELAY_ms(2000);
		Card_Failed = 1;
  }
	else {
		print_DebugMsg("Disk initialization success.\r\n");
		Card_Failed = 0;
	}
  SysTick_Stop();	
	
/**************************************************************************************************/
	
	RTC_GetDateTime(&rtc); /* Get RTC Time and Date */

	/* save system boot time in SD card */
	memset(_buffer, 0, 512);
	sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[INFO]:System Reboot;",
					(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
  console_log("buff : %s",_buffer);
	Createlog( _buffer , "err.txt");	/* Save log as a error - for reference */

	/* if system resets due to watchdog, write error log */
	if (watchDogReset) {
		RTC_GetDateTime(&rtc);
		memset(_buffer, 0, 512);
		sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:WDT RESET;",
						(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
		Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
		print_DebugMsg(" Last MCU reset caused by WDT TimeOut..!\n\r");
	}

	/* if system resets due to brownOutReset, write error log */
	if (brownOutReset) {
	  RTC_GetDateTime(&rtc);
		memset(_buffer, 0, 512);
		sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:BrownOut RESET;",
						(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);
		Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
		print_DebugMsg("Last MCU reset caused by BrownOut TimeOut!\n\r");
	}
	
/**************************************************************************************************/
	
	
									  
				 
	
	/* Read INI file content */
	print_DebugMsg("\r\n Read SD card configuration file...\r\n");
	returnStatus = readconfigfile(iniContent);
	if (returnStatus) { /* IF INI file not Found */
    print_DebugMsg(" File Opening Failed \n\r");
	  LCD_Clear();
		LCD_GoToLine(0);LCD_Printf("Config ");
	  LCD_GoToLine(1);LCD_Printf("file not found.");
	  DELAY_ms(2000);
    
		#if !ARG_BIHAR
			strcpy(URLbuffer,"http://iot.skymetweather.com/uniparser/mahavedh/mns"); // store url from sd card
			strcpy(_logInterval, "1");	  			// store log interval from sd card
			strcpy(_PacketlogInterval, "10");  	// store send interval from sd card
			strcpy(_PacketsendInterval, "10"); 	// store pending log interval from sd card
		#else
			strcpy(URLbuffer,"http://iot.skymetweather.com/uniparser/skysense/rainguage"); // store url from sd card
			strcpy(_logInterval, "1");	  			// store log interval from sd card
			strcpy(_PacketlogInterval, "15");  	// store send interval from sd card
			strcpy(_PacketsendInterval, "15"); 	// store pending log interval from sd card
		#endif
  }
	else {
		console_log("iniContent:%s \n\r",iniContent);
		strcpy(URLbuffer, strtok(iniContent , "|"));		// store url from sd card
		strcpy(_logInterval, strtok(NULL, "|"));	  		// store log interval from sd card
		strcpy(_PacketlogInterval, strtok(NULL, "|"));  // store send interval from sd card
		strcpy(_PacketsendInterval, strtok(NULL, "|")); // store pending log interval from sd card
		strcpy(_apnBuffer, strtok(NULL, "|")); // store pending log interval from sd card
	}

	logInterval = ascii_integer(_logInterval);								/* convert ascii to int	*/
	PacketlogInterval = ascii_integer(_PacketlogInterval);		/* convert ascii to int	*/
	PacketsendInterval = ascii_integer(_PacketsendInterval);	/* convert ascii to int	*/
	
	/* print the converted values on terminal */
	console_log(" URL:%s \n\r LogInterval:%d \n\r PacketlogInterval:%d \n\r PacketSendInterval:%d \n\r APN:%s \n\r",
								URLbuffer, logInterval, PacketlogInterval, PacketsendInterval,_apnBuffer);
	
	#endif /*_PP_SDCARD_INIT*/
	

	#if _PP_EEPROM
		WDT_Feed();
		/* EEPROM READ Variables */
		MaxRain_threshold_val1 = EEPROM_ReadByte(eeprom_address_TH1);  		// Read the data from memoryLocation 0x00
		console_log("Eeprom Read TH1: %d \n\r",MaxRain_threshold_val1); 	// Print the message on UART
		
		MaxRain_threshold_val2 = EEPROM_ReadByte(eeprom_address_TH2);  		// Read the data from memoryLocation 0x01
		console_log("Eeprom Read TH2: %d \n\r",MaxRain_threshold_val2); 	// Print the message on UART
	
		rain = EEPROM_ReadByte(eeprom_address_Rain);  		// Read the data from memoryLocation 0x23
		console_log("Eeprom Read rain: %f \n\r",rain); 	// Print the message on UART
			
		
		if ((MaxRain_threshold_val1==255) &&(MaxRain_threshold_val2 ==255) && (rain ==255))
		{
			// TO DO
			// if epprom empty, then store default value in variables
			// if threshold not set, then assigen default thershould values.
			MaxRain_threshold_val1 = 5;
			MaxRain_threshold_val2 = 10;
			rain = 0;
			EEPROM_WriteByte(eeprom_address_Rain, rain);
		}

	#endif /* _PP_EEPROM */
/**************************************************************************************************/

	#if _PP_GSM_GPRS_INIT
	
	WDT_Feed();
//GPIO_PinWrite(SIM_SEL,LOW);      // Set to select SIM1
//	GPIO_PinWrite(SIM_SEL,HIGH);     // Set to select SIM2
//	DELAY_ms(2000);
	
	/* Enable IRQ'S  */
	
	NVIC_EnableIRQ(UART1_IRQn);	   				/* Enable UART 1 interrupt */
	UART1_EnableBuffer_Interrupt();				/* Enable Buffer RBR UART 1 interrupt*/

	/* Excutes the GSM module Initilization related functions */
	for (retries = 2; retries > 0; retries--) {
		returnStatus = modem_initialization();
		if(returnStatus) break;	/* GSM Module initialize - return 1 on succesful */
  	else {
			print_DebugMsg("GSM Communication Error !!\r\n");
			LCD_Clear();
			LCD_GoToLine(0); LCD_Printf("GSM Comm Error !");
			LCD_GoToLine(1); LCD_Printf("Check module !?");
			DELAY_ms(1000); 
		}
	}	
	
//	Config_Module_for_RI_Interrupt();
//	while(1);
	
	if (returnStatus) {
		print_DebugMsg("Modem Engine Initialization Successful \r\n");
		LCD_Clear();
		LCD_GoToLine(0); LCD_Printf("Modem Engine");
	  LCD_GoToLine(1); LCD_Printf("Initialized !!");
		DELAY_ms(200);
		
		for (retries = 2; retries > 0; retries--){
			returnStatus = modemCheck_initialization(APN); /* Initialize GPRS commands */
			if(returnStatus) break;
		}
		
		if (returnStatus) /* IF GPRS Initialize succesful */
		{
			if (Query_Set_CLTS()){ 
				MODEM_GetNetworkTime();		/* Update date and Time */
			}
			
			GSM_Delete_All_Msg();
			DELAY_ms(200);
			
			ReadNavigation(); /* GNSS Enable and Read */
			DELAY_ms(200);

			Module_deactivateBearerProfile();	/* Deactivate the PDP context of the module */
			DELAY_ms(200);
		}
														
   
				  
																												  
					
							 
							  
																																																												 
																																													  
																																		  
														 
   
		else {	/* PDP context activating failed */
					WDT_Feed();	
					ShutDownModule();			/* MCU GPIO PIN HIGH to OFF the regulator*/
					startModule_vREG();		  /* MCU GPIO PIN LOW to ON the regulator*/
					DELAY_ms(8000);				/* wait to start module */
			
					responseStatus = Module_Power_Initializing();
					
					if(responseStatus){
						print_DebugMsg("module activate successfully!\r\n");	
					}
					else
					{
						LCD_Clear();
						LCD_GoToLine(0);LCD_Printf("PDP Activation..");
						LCD_GoToLine(1);LCD_Printf("failed");
						DELAY_ms(200);
			
						RTC_GetDateTime(&rtc);
						memset(_buffer, 0, 512);
						sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:MESSAGE DETAIL: %d;PDP failed;",
										(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,ERROR_CODE);
						Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
						print_DebugMsg(" PDP activation failed \n\r");
					}
				}	
	}
	else {	/* If GSM Module initialize failed */
		print_DebugMsg("Modem Engine Initialization Failed \r\n");
		LCD_Clear();
		LCD_GoToLine(0); LCD_Printf("Modem Engine");
	  LCD_GoToLine(1); LCD_Printf("Error Comm.. !!");
		DELAY_ms(2000);
	}
	
	Module_sleep_mode();							/* put GSM module in Sleep mode */	
	UART1_DisableBuffer_Interrupt(); 	/* Disable Buffer RBR UART 1 interrupt */  

	
	#endif /*_PP_GSM_GPRS_INIT*/
	
/**************************************************************************************************/

	/* mask off alarm mask, turn on IMYEAR in the counter increment interrupt register */
 	RTCSetAlarmMask(RTC_AMR_AMRSEC|RTC_AMR_AMRMIN|RTC_AMR_AMRHOUR|RTC_AMR_AMRDOM|RTC_AMR_AMRDOW|RTC_AMR_AMRDOY|RTC_AMR_AMRMON|RTC_AMR_AMRYEAR);
  LPC_RTC->CIIR = RTC_CIIR_IMMIN;  

 	
	/* Set -- configure and read the wind sensor paraemeterts */
	
	#if _PP_ULTRASONIC_CONFIG
	
	Configure_WindSensor();
	CheckCommunication_WindSensor();
	
	#endif /*_PP_ULTRASONIC_CONFIG*/
	
							 
							 
			   
 
 
/**************************************************************************************************/
	while(1) 
	{
		WDT_UpdateTimeOut(WDT_TIMEOUT);			/* Update WDT timeout */
    #if _PP_READ_BATT_SOLAR_VOLT		
		Read_ADCchannels();									/* Read Battery voltage and Solar voltage */
		#endif
		
		if(volt0 >= 1.47) {	/* Check battery voltage is above 9.30 V to activate the below commands */
//     if(volt0>=1.75){/* Check battery voltage is above 11.00 V to activate the below commands */
/**************************************************************************************************/		
			
			RTCStart();		
			EINT_AttachInterrupt(EINT1,myExtIntrIsr_1,FALLING);	/* Enable Ext interrupt for push button */
			#if _PP_CUP_ANOMOMETER
			EINT_AttachInterrupt(EINT2,myExtIntrIsr_0,FALLING); /* Enable External interrupt for Wind Speed */
			#endif
			EINT_AttachInterrupt(EINT3,myExtIntrIsr_3,FALLING); /* Enable Ext interrupt for Rain guage */
		  SysTick_Start();                 										/* Start SysTick Timer */
			//lcd_disable();
		
			do {
				if (KeyInterrupt)
        {		
				   if(LCD_Enable == 0) //&& Display_Flag == 1)
				   lcd_enable();
					LCD_ScrollDisplay();	/* Check *KeyInterrupt ext interrupt button is pressed */				
				}
				if(!KeyInterrupt) 
				{
					if(LCD_Enable){
						//LCDnumTipsRain = 0;
						LCD_Clear(); 
						LCD_GoToLine(0);LCD_Printf(" -- SkySense -- ");
						LCD_GoToLine(1);LCD_Printf(" --BIHAR[ARG]--");		
						DELAY_ms(50);	
					}
				} 
			}while(alarm_on == 0); // Comparing the check point - One minute interval for data transmission
	
		console_log("\n\rRTC_ReadGPREG(2) rain : %f\n\r" , RTC_ReadGPREG(2));
		//rain = EEPROM_ReadByte(eeprom_address_Rain);  		// Read the data from memoryLocation 0x23
		//console_log("Eeprom Read rain: %f \n\r",rain);
			
			//lcd_disable();	 
			alarm_on = 0;  				/* Clear the RTC 1 minute Interrupt Counter alarm */
			KeyInterrupt = 0;			/* Clear the External interrupt flag */
			scrollScreen = 0;			/* Clear the External interrupt flag */
			//LCDnumTipsRain = 0;						/* Clear LCD rain count */
			detector = 0;
			RTC_IntDisable();			/* Disable Internal RTC Counter increment interrupt*/
			EINT_DetachInterrupt(EINT1);/* Disable External interrupt for push button switch */
			#if _PP_CUP_ANOMOMETER
			EINT_DetachInterrupt(EINT2);/* Disable External interrupt for Wind Speed */
			#endif
			EINT_DetachInterrupt(EINT3);/* Disable External interrupt for Rain Guage */
			SysTick_Stop();
			
/**************************************************************************************************/			
			
			/* read sensor values */
			#if _PP_READ_ATRH    
			readATRH(ATRH_Write_ADD,ATRH_READ_ADD,Temperature_register_add);
			#endif /*_PP_READ_ATRH */
			#if _PP_READ_WINDSENSOR
			Read_WindSensor();					/* Read Wind sensor [ Ultrasonic] */
			#endif /*_PP_READ_WINDSENSOR*/
			#if _PP_READ_CUP_ANOMOMETER
			Cup_Read_WindSensor();			/* Read Wind sensor [ Cup-Anomometer] */
			#endif /*_PP_READ_CUP_ANOMOMETER*/
			
			
/**************************************************************************************************/
		
			/* Calculate average of log interval minute and save it to regular log */
			RTC_GetDateTime(&rtc); /* Read RTC to compare the change in multiple 10 minute */
			console_log("%02d/%02d/%d;%02d:%02d \n\r",(uint16_t)rtc.date,(uint16_t)rtc.month,
								  (uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min);

			if (rtc.min % logInterval == 0) {
				//Slog();				Slog Deactivated
				if(rtc.hour == 8 && rtc.min == 17)
				{
						Reset24HrRainAt8_16();
				}
			}

			if (rtc.min % PacketlogInterval == 0) {
				PacketLog();
			}

			if ((rtc.min % PacketsendInterval == 0)  && (volt0 > 1.55) ) {
				SendingLog();
			}
			
/**************************************************************************************************/
			
		}
		else {
			
			if(LCD_Enable){
			/* Create custom char	*/
    	lcd16x2_create_custom_char(0, custom_char);
			LCD_Clear();

			/* Display custom char */
	    lcd16x2_put_custom_char(0, 0, 0);
			LCD_Printf("  Battery Low");
			DELAY_ms(500);
			}
			BatteryLow_Flag = 1; /* Battery low flag set */
			
/**************************************************************************************************/

			/* Below condition is to log only once in sd card error log */
			if((BatteryLow_Flag == 1) && (_LogBatteryLog == 1)) 
			{
				RTC_GetDateTime(&rtc);
				memset(_buffer, 0, 512);
				sprintf(_buffer,"[%02d/%02d/%d;%02d:%02d]:[ERROR]:BATTERY LOW:Voltage:%2s;",
								(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year,(uint16_t)rtc.hour,(uint16_t)rtc.min,_BVolt);
				Createlog(_buffer, "err.txt");	/* Save log as a error - for reference */
				print_DebugMsg(" BATTERY LOW \n\r");
				_LogBatteryLog = 0; 			/* One time write excuted in sd card error log */
			}
/**************************************************************************************************/
	  }
	}
}

/**********************************************************************************
*                           Main program body close                               *
***********************************************************************************/

///**************************************************************************************************************
//*                                    COPYING A FILE
//*
//* Description: This function is used by the user to copy a file
//*
//* Arguments  : None
//*
//* Returns    : None
//*
//**************************************************************************************************************
//*/

//void  Main_Copy (void)
//{
//    int32_t  fdr;
//    int32_t  fdw;
//    uint32_t  bytes_read;

//	fdr = FILE_Open(FILENAME_R, RDONLY);
//    if (fdr > 0) {
//        fdw = FILE_Open(FILENAME_W, RDWR);
//        if (fdw > 0) {
//            PRINT_Log("Copying from %s to %s...\n", FILENAME_R, FILENAME_W);
//            do {
//                bytes_read = FILE_Read(fdr, UserBuffer, MAX_BUFFER_SIZE);
//                FILE_Write(fdw, UserBuffer, bytes_read);
//            } while (bytes_read);
//            FILE_Close(fdw);
//        } else {
//            PRINT_Log("Could not open file %s\n", FILENAME_W);
//            return;
//        }
//        FILE_Close(fdr);
//        PRINT_Log("Copy completed\n");
//    } else {
//        PRINT_Log("Could not open file %s\n", FILENAME_R);
//        return;
//    }
//}






//F:\Random\Stock\2021-001-ARG-BIHAR-main\main.c

/*********************************************************************************
*                            End Of File                                         *
*********************************************************************************/

