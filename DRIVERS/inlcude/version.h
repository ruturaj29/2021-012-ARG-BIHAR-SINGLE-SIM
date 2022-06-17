#ifndef _VERSION_H_
#define _VERSION_H_

/** \ingroup _version
    major version number. */
#define _MAJOR__         0
    
/** \ingroup _version
    minor version number. */
#define _MINOR__         0
    
/** \ingroup _version
    revision number. */
#define _BUILD__       7

///** \ingroup _version
//   String literal representation of the current library version. */
//#define _VERSION_STRING__ "ARG.0.2"
    
/** \ingroup _version
     Numerical representation of the current version.
    
     In the numerical representation, the major number is multiplied by
     10000, the minor number by 100, and all three parts are then
     added.  It is intented to provide a monotonically increasing
     numerical value that can easily be used in numerical checks.
  */
#define _VERSION__        20000UL

#define _HWVERSION__	"ST0301"

//#define _PNO__				"MAH_ST0301"
    
/** \ingroup _version
    String literal representation of the release date YYYYMMDD. */
#define _DATE_STRING__    "2021xxxx"
    
/** \ingroup _version
    Numerical representation of the release date. */
#define _DATE__            20210212UL
    


/** \ingroup _version
    release type. */
#define _RELEASE__    "PRODUCTION-ARGB"

/** \ingroup _version
    release type. */
#define _PROJECT__    "ARG BIHAR"

#endif /* _VERSION_H_ */


/** === REVISION HISTORY ===

	  \version ARG.2.2.3 
			@Added if(!(strstr(UART1Buffer,",")))	check for +CMGR response 2.2.2 UNREAD @line2246 gprs.c
				// Log.c Line 470 Module Shut Down;
		\version ARG.0.4
			@Added Issue observed HTTP busy resposen in log file. Necessary changes are done in code @1388 @1399
							in gprc.c file
			@Added RTC_GetDateTime(&rtc) in rain.c file @64 line

		\version ARG.0.3  
			@Added		rain cummulative in every send packet. Rain value resets only after 24 hours.
			@removed	version minor, major print line from main() function
		
    \version ARG.0.2
			@DATE     14 Sep 2021
			@Issues 	Observed code halt issue 
			@Removed	UART0 interrupt enbale code
			@Removed 	Unnessecary interrupt pririoties code
			@Modified Display first screen delay reduced
			@Removed	lcd busy funtion and used delay function instead
			

		\version v0.0.6 
				@DATE       07 May 2021
				@Implement 	Sleep mode is implemented for this version. 
				@Added 			log.c 
				@Added 			systick.c for SD card functioning.
				@Modified 	GPRS.c, Main.c and mmc_176x_ssp.c functions. 
										(changes according to the Beta testing Shivajinagar after batch 4)
				@Modified 	ALL [ .c files and .h files] are modifide for inient and proper alligenment.
				@Modified 	I2C0.c ( changes for ATRH [ I2C2 --> I2C0 ] )
				@tested 		for new HW ST0301 all sensore check and tested [ATRH,Ultrasoinc,rain,LCD Scroll]
				            and also INI.txt is update thruogh SMS.
   

		\version v0.0.5
				@Added BOD
				@Modified GPRS.c functions (according to the Beta testing Shivajinagar)
				@Modified UART driver 115200
				@Modified WDT timeout value (3 min)
				@Added EEPROM (SoftI2c), SMS.c, SMS routines in GPRS.c
		
		Mahavedh 4G code version start from here upon v0.0.4 on the HW ST0301
		Date: 24/3/2021
		--#------#------#------#------#------#------#------#
		Harwdare version change from v0.0.4.Firmware changes according to new hardware version [ST0301]		
		\version v0.0.4
				@API minINI application interface 
				@Added retry log, working as expected. Added ini content function
				@Modified LCD enable & disable
		
		==================================================================== 
		Below revisions works only on ST-0101
		\version v0.0.3
				@Added function Writelog -- creates file & folder in sd card
				
		\version v0.0.2 
				@Added lcd logo & other custom characters
				@Added progress bar function to display
				@Modified _PNO__ to string
		
		\version v0.0.1 
				@Modified DWORD get_fattime() function
				@Added		Major modification for SPI sd card interface.
				@Added		LPC176x.h file only to support mmc_176x_ssp.c file
				@removed	Unused the sdcard.h/.c & spi.h/.c file for sd card read/write.
				@Added  Project number as _PNO__
				
				
		\version v0.0.0 (February 12, 2021) - Prototype (not released)
				@Added 		some functionalities in sdcard.c for testing sdcard in creating a directories.
				@Added 		All_HeaderFiles.h for header file configuration.
				@Added 		version.h file
				@Improved UART debugging to console for single DEFINE.  
		
		**/
