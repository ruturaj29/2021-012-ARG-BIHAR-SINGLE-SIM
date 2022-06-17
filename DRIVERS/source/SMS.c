
//#include "gprs.h"
//#include "delay.h"
//#include "uart.h"
//#include "convert.h"
//#include "stdutils.h"
//#include "eeprom.h"
//#include <string.h>


//extern char message_received[60];
//extern char _SMS_Buffer[512];

//void SMS(){

//    char logInterval = 1, PacketlogInterval = 10, PacketsendInterval = 10;
//	char _logInterval[4] = {'\0'}, _PacketlogInterval[4] = {'\0'}, _PacketsendInterval[4] = {'\0'};

//  unsigned char  read_char ; //write_char = 'A', read_ch[30];
//	uint32_t   eeprom_address = 0x1A;     //0x1A;

//	int choice = 0,MaxRain_threshold_val1 ;//rain;
//	char _MaxRain_threshold_val1[4] = {'\0'};
//	char _SMS_Flag_Status[10] = {'\0'};

//	char _apnBuffer[10];

// 	while(1)
//	{

//	print_DebugMsg(" SMS code execution is start..!!\r\n");

//	Get_SMSC_Number();
//	DELAY_ms(500);

//	print_DebugMsg("\r\n");
//	print_DebugMsg("AT+CMGF=1\r");
//	UART1_TxString("AT+CMGF=1\r");	/* select message format as text */
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

//	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
//	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* select memory storages to be used for reading, writing, receiving (default setting: AT+CPMS="SM","SM","SM") */
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

//	print_DebugMsg("AT+CSDH=1\r");
//	UART1_TxString("AT+CSDH=1\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

//	print_DebugMsg("AT+CSCS=\"GSM\"\r");
//	UART1_TxString("AT+CSCS=\"GSM\"\r");
//	WaitForExpectedResponse("OK",3000);
//	DELAY_ms(500);

////	print_DebugMsg("AT+CMGR=?\r");
////	UART1_TxString("AT+CMGR=?\r");
////	WaitForExpectedResponse("OK",3000);
////	DELAY_ms(500);
////
////	print_DebugMsg("AT+CMGL=?\r");
////	UART1_TxString("AT+CMGL=?\r");
////	WaitForExpectedResponse("OK",3000);
////	DELAY_ms(500);

////	print_DebugMsg("AT+CMGL=\"ALL\"\r");
////	UART1_TxString("AT+CMGL=\"ALL\"\r");
////	WaitForExpectedResponse("OK",3000);
////	DELAY_ms(500);

////	print_DebugMsg("AT+CMGR=0\r");
////	UART1_TxString("AT+CMGR=0\r");
////	WaitForExpectedResponse("OK",3000);
////	DELAY_ms(500);


////	GSM_Msg_Delete(0);
////	DELAY_ms(500);
////
////	GSM_Msg_Delete(2);
////	DELAY_ms(500);

//	GSM_Msg_Read(0);
//	DELAY_ms(500);

//	GSM_Msg_Read(1);
//	DELAY_ms(500);


////   	print_DebugMsg("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
////	UART1_TxString("AT+CPMS=\"SM\",\"SM\",\"SM\"\r");	/* select memory storages to be used for reading, writing, receiving (default setting: AT+CPMS="SM","SM","SM") */
////	WaitForExpectedResponse("OK",3000);
////	DELAY_ms(500);

// 
//    if((strstr(message_received,"Status")))
//	choice = 1;
//	else if((strstr(message_received,"Config")))
//	choice = 2;
//	else if((strstr(message_received,"Threshold")))
//	choice = 3;
//	else if((strstr(message_received,"Log")))
//	choice = 4;


//	console_log("choice : %d ",choice);

//	switch(choice)
//	{
//		case 1: //SMS_Status_send();
//				break;
//		case 2: //strcpy(_SMS_Flag_Status , strtok(message_received , ";"));
//		        strcpy(URLbuffer , strtok(message_received , ";"));
//				//strcpy(URLbuffer, strtok(NULL , ";"));			// store url from sd card
//				strcpy(_logInterval, strtok(NULL, ";"));	  		// store log interval from sd card
//				strcpy(_PacketlogInterval, strtok(NULL, ";"));  	// store send interval from sd card
//				strcpy(_PacketsendInterval, strtok(NULL, ";")); 	// store pending log interval from sd card
//				strcpy(_apnBuffer, strtok(NULL, ";"));

//				logInterval = ascii_integer(_logInterval);				//	/* convert ascii to int	///
//				PacketlogInterval = ascii_integer(_PacketlogInterval);	//	/* convert ascii to int	///
//				PacketsendInterval = ascii_integer(_PacketsendInterval);//	/* convert ascii to int	///

//				console_log("URL:%s \n\rLogInterval:%d \n\rPacketlogInterval:%d \n\rPacketSendInterval:%d \n\rAPN:%s \n\r",
//				URLbuffer,logInterval,PacketlogInterval, 
//				PacketsendInterval,
//				_apnBuffer);

//			//	Writelog_SMS(_SMS_Buffer,"INI.txt");
//				break;

//		case 3: strcpy(_SMS_Flag_Status , strtok(message_received , ";"));
//		 		strcpy(_MaxRain_threshold_val1 , strtok(NULL , ";"));
//				MaxRain_threshold_val1 = ascii_integer(_MaxRain_threshold_val1);				//	/* convert ascii to int	///
//				console_log(" Rain Threshold set to : %d ",MaxRain_threshold_val1);
//				eeprom_address = 0x31;
//				DELAY_ms(100);
//				console_log("EEprom address : %d",eeprom_address);
//				EEPROM_WriteByte(eeprom_address, MaxRain_threshold_val1); // Write the data at memoryLocation	0x00
//				DELAY_ms(100);
//				read_char = EEPROM_ReadByte(eeprom_address);  // Read the data from memoryLocation 0x00
//				console_log("EEPROM read data : %d ",read_char);
//				break;

//		case 4: 
//		        break;
//	}


//		//		rain = ascii_integer(_Rain);			/* convert ascii to int	*/
//		       
////		        rain = 2;
////				if(	rain >= MaxRain_threshold_val1)
////				{
////				   print_DebugMsg(" threshold is exceed..\n\r");
////				   SMSDefaultSetting();
////	    	       GSM_Send_Msg(master_Number,"rain value is exceed to its threshold ");

////				   DELAY_ms(2000);
////				}else
////				{
////				   print_DebugMsg(" rain value is in limit..\n\r");
////				}
////
////				rain = 10;
////				if(	rain >= MaxRain_threshold_val1)
////				{
////				   print_DebugMsg(" threshold is exceed..\n\r");
////				   SMSDefaultSetting();
////	    	       GSM_Send_Msg(master_Number,"rain value is exceed to its threshold ");

////				   DELAY_ms(2000);
////				}
////				else
////				{
////				   print_DebugMsg(" rain value is in limit..\n\r");
////				}
////  


////		
////  print_DebugMsg("1st MSG is :  \r\n");
////	print_DebugMsg(message_received);
////	print_DebugMsg("\r\n");
////
////	GSM_Msg_Read(1);
////	DELAY_ms(500);
////
////	print_DebugMsg("2nd MSG is :  \r\n");
////	print_DebugMsg(message_received);
////	print_DebugMsg("\r\n");

////	GSM_Msg_Read(2);
////	DELAY_ms(500);

////	print_DebugMsg("3rd MSG is :  \r\n");
////	print_DebugMsg(message_received);
////	print_DebugMsg("\r\n");
////
////	if((strstr(message_received,"Status")))		/*check for +CMGR response */
////	{
////		print_DebugMsg("Status message is sending \r\n");
////		SMS_Status_send();
////		print_DebugMsg("\n\r-------- SMS send done  --------\n\r");
////	}
////	else
////	{
////		  	print_DebugMsg("Status message is not recived \r\n");
////	}

////	GSM_Msg_Delete(0);
////	DELAY_ms(500);
////
////	GSM_Msg_Delete(1);
////	DELAY_ms(500);
////
////	GSM_Msg_Delete(2);
////	DELAY_ms(500);


//	}
//}




