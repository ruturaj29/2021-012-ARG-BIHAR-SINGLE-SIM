
#include "diskio.h"		/* FatFs lower layer API */
#include "uart.h" 
/*-----------------------------------------------------------------------*/
/* Initialize disk drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (BYTE pdrv)
{
	DSTATUS Stat = STA_NOINIT;	/* Physical drive status */
	if(!pdrv)
	{
//	print_DebugMsg("initializing MMC drive .. \n\r");
	Stat=disk_initialize_MMC_SD (pdrv);
	}
	else{
//		print_DebugMsg("initializing USB drive .. \n\r");
	Stat=disk_initialize_USB (pdrv);
	}
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv)
{
	DSTATUS Stat = STA_NOINIT;	/* Physical drive status */
	if(!pdrv)
	{
//		print_DebugMsg("take MMC drive status .. \n\r");
	Stat=disk_status_MMC_SD(pdrv);
	}else{
//		print_DebugMsg("take USB drive status .. \n\r");
	Stat=disk_status_USB(pdrv);
	}
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	DRESULT res = RES_ERROR;
	if(!pdrv)
	{
//		print_DebugMsg("read MMC drive .. \n\r");
	res = disk_read_MMC_SD (pdrv,buff,sector,count);
	}else{
//		print_DebugMsg("read USB drive .. \n\r");
	res = disk_read_USB (pdrv,buff,sector,count);
	}
	return res;
}

/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	DRESULT res = RES_ERROR;
	if(!pdrv)
	{
//		print_DebugMsg("write MMC drive .. \n\r");
	res = disk_write_MMC_SD (pdrv,buff,sector,count);
	}else{
//		print_DebugMsg("write USB drive .. \n\r");
	res = disk_write_USB (pdrv,buff,sector,count);
	}
  return res;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
	DRESULT res = RES_ERROR;
	if(!pdrv)
	{
//		print_DebugMsg("IOCTL MMC drive .. \n\r");
	res = disk_ioctl_MMC_SD (pdrv,cmd,buff);
	}else{
//		print_DebugMsg("IOCTL USB drive .. \n\r");
	res = disk_ioctl_USB (pdrv,cmd,buff);
	}
	return res;
}





