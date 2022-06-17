
/************************ include Header files *******************************/
#include "diskio.h"		/* FatFs lower layer API */
#include "usbhost_inc.h"


/************************* Globle variables *********************************/
uint32_t numBlks, blkSize;
uint8_t inquiryResult[INQUIRY_LENGTH];
static DSTATUS usb_status = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Initialize disk drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize_USB (
	BYTE pdrv		/* Physical drive number (0..) */
)
{
	usb_status = STA_NOINIT;
//	if ( pdrv != 0)  return STA_NOINIT;      // <---- add this line
	if ( pdrv != 1)  return STA_NOINIT;      // <---- add this line
//	console_log("usb_status: %d\n\t\r STA_NOINIT: %d\n\r",usb_status,STA_NOINIT);
        
	/* USB host init and enumeration */
  if(usb_status & STA_NOINIT ) 
	{
//		usb_status = STA_NOINIT;
//		FSUSB_DiskReadyWait(50);
    Host_Init();
    if ( Host_EnumDev() == OK ) 
		{
      /* mass-storage init */
      if ( MS_Init(&blkSize, &numBlks, inquiryResult) == OK ) 
			{
				usb_status &= ~STA_NOINIT;
				return usb_status;  // <---- add this line
      } 
			else
			{
				/* MS init fail */
				return (0);
      }
    }
		else 
		{
			/* host init and/or enum fail */
			return (0);
    }
  }
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status_USB(
	BYTE pdrv		/* Physical drive number (0) */
)
{
	
//	if (pdrv) {
	if (pdrv != 1){
		return STA_NOINIT;	/* Supports only single drive */
	}
	return usb_status;
	
}


/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read_USB (
	BYTE pdrv,		/* Physical drive number (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	LBA_t sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
//	if (pdrv || !count) {
	if (pdrv != 1 || count == 0) {
		return RES_PARERR;
	}
	
	// need to change if condition
//	if (usb_status & STA_NOINIT) {
//		return RES_NOTRDY;
//	}

//	if (FSUSB_DiskReadSectors(hDisk, buff, sector, count)) {
//		return RES_OK;
//	}

	if (MS_BulkRecv ( sector, count , buff)== OK){
		return RES_OK;
	}

	return RES_ERROR;

}


/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write_USB (
	BYTE pdrv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Ponter to the data to write */
	LBA_t sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
//	if (pdrv || !count) {
	if (pdrv != 1 || count == 0){
		return RES_PARERR;
	}
	
	// need to change if condition
//	if (usb_status & STA_NOINIT) {
//		return RES_NOTRDY;
//	}

//	if (FSUSB_DiskWriteSectors(hDisk, (void *) buff, sector, count)) {
//		return RES_OK;
//	}
	
	if (MS_BulkSend (sector, count,(void *) buff)== OK) {
		return RES_OK;
	}
	
	return RES_ERROR;
	
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl_USB (
	BYTE pdrv,		/* Physical drive number (0) */
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
)
{
	DRESULT res = RES_ERROR;
	
//	if (pdrv) {
	if (pdrv != 1) {
		return RES_PARERR;
	}
	
	// need to change if condition
	if (usb_status & STA_NOINIT) {
		return RES_NOTRDY;
	}

	switch (cmd) {
	case CTRL_SYNC:	/* Make sure that no pending write process */
		if (FSUSB_DiskReadyWait(50)) {
			res = RES_OK;
		}
		break;

	case GET_SECTOR_COUNT:	/* Get number of sectors on the disk (DWORD) */
//		*(DWORD *) buff = FSUSB_DiskGetSectorCnt(hDisk);
		*(DWORD *) buff = numBlks;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:	/* Get R/W sector size (WORD) */
//		*(WORD *) buff = FSUSB_DiskGetSectorSz(hDisk);
		*(WORD *) buff = blkSize;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:/* Get erase block size in unit of sector (DWORD) */
		*(DWORD *) buff = FSUSB_DiskGetBlockSz(hDisk);
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
		break;
	}

	return res;
	
}

