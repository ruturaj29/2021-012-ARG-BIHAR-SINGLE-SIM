
/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2020
/-----------------------------------------------------------------------*/

#ifndef DISKIO_DEFINED
#define DISKIO_DEFINED
#include "ff.h"

#define DISKIO_ISDIO	0	/* 1: Enable iSDIO control fucntion */
#define	VIRTUAL_DISK	1	/* PD# associated with disk image file (set 0 to disable virtual disk feature) */


#ifdef __cplusplus
extern "C" {
#endif

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


#if	DISKIO_ISDIO
/* Command structure for iSDIO ioctl command */
typedef struct {
	BYTE	func;	/* Function number: 0..7 */
	WORD	ndata;	/* Number of bytes to transfer: 1..512, or mask + data */
	DWORD	addr;	/* Register address: 0..0x1FFFF */
	void*	data;	/* Pointer to the data (to be written | read buffer) */
} SDIO_CMD;
#endif


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);


DSTATUS disk_initialize_MMC_SD (BYTE pdrv);
DSTATUS disk_status_MMC_SD(BYTE pdrv);
DRESULT disk_read_MMC_SD (BYTE pdrv,BYTE *buff,LBA_t sector,UINT count);
DRESULT disk_write_MMC_SD (BYTE pdrv,	const BYTE *buff,	LBA_t sector,UINT count);
DRESULT disk_ioctl_MMC_SD (BYTE pdrv,BYTE cmd,void *buff);

DSTATUS disk_initialize_USB (BYTE pdrv);
DSTATUS disk_status_USB(BYTE pdrv);
DRESULT disk_read_USB (BYTE pdrv,BYTE *buff,LBA_t sector,UINT count);
DRESULT disk_write_USB (BYTE pdrv,const BYTE *buff,LBA_t sector,UINT count);
DRESULT disk_ioctl_USB (BYTE pdrv,BYTE cmd,void *buff);

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */


/* Generic command (Not used by FatFs) */
#define CTRL_FORMAT			5	/* Create physical format on the media */
#define CTRL_POWER_IDLE		6	/* Put the device idle state */
#define CTRL_POWER_OFF		7	/* Put the device off state */
#define CTRL_LOCK			8	/* Lock media removal */
#define CTRL_UNLOCK			9	/* Unlock media removal */
#define CTRL_EJECT			10	/* Eject media */
#define CTRL_GET_SMART		11	/* Read SMART information */

/* MMC/SDC specific ioctl command (Not used by FatFs) */
#define MMC_GET_TYPE		50	/* Get card type */
#define MMC_GET_CSD			51	/* Read CSD */
#define MMC_GET_CID			52	/* Read CID */
#define MMC_GET_OCR			53	/* Read OCR */
#define MMC_GET_SDSTAT		54	/* Read SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command (Not used by FatFs) */
#define ATA_GET_REV			60	/* Get F/W revision */
#define ATA_GET_MODEL		61	/* Get model name */
#define ATA_GET_SN			62	/* Get serial number */

/* Disk image feature (VIRTUAL_DISK >= 1) */
#define OPEN_VIRTUAL_DISK	90


/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC3		0x01		/* MMC ver 3 */
#define CT_MMC4		0x02		/* MMC ver 4+ */
#define CT_MMC		0x03		/* MMC */
#define CT_SDC1		0x04		/* SDC ver 1 */
#define CT_SDC2		0x08		/* SDC ver 2+ */
#define CT_SDC		0x0C		/* SDC */
#define CT_BLOCK	0x10		/* Block addressing */


#ifdef __cplusplus
}
#endif

#endif


