/*
**************************************************************************************************************
*                                                 NXP USB Host Stack
*
*                                     (c) Copyright 2008, NXP SemiConductors
*                                     (c) Copyright 2008, OnChip  Technologies LLC
*                                                 All Rights Reserved
*
*                                                  www.nxp.com
*                                               www.onchiptech.com
*
* File           : usbhost_ms.h
* Programmer(s)  : Ravikanth.P
* Version        :
*
**************************************************************************************************************
*/

#ifndef  USBHOST_MS_H
#define  USBHOST_MS_H

/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include  "usbhost_inc.h"
#include <stdbool.h>


/*
**************************************************************************************************************
*                               MASS STORAGE SPECIFIC DEFINITIONS
**************************************************************************************************************
*/

#define    MS_GET_MAX_LUN_REQ            0xFE
#define    MASS_STORAGE_CLASS            0x08
#define    MASS_STORAGE_SUBCLASS_SCSI    0x06
#define    MASS_STORAGE_PROTOCOL_BO      0x50

#define    INQUIRY_LENGTH                36
/*
**************************************************************************************************************
*                                  SCSI SPECIFIC DEFINITIONS
**************************************************************************************************************
*/

#define  CBW_SIGNATURE               0x43425355
#define  CSW_SIGNATURE               0x53425355
#define  CBW_SIZE                      31
#define  CSW_SIZE                      13
#define  CSW_CMD_PASSED              0x00
#define  SCSI_CMD_REQUEST_SENSE      0x03
#define  SCSI_CMD_TEST_UNIT_READY    0x00
#define  SCSI_CMD_INQUIRY            0x12
#define  SCSI_CMD_READ_10            0x28
#define  SCSI_CMD_READ_CAPACITY      0x25
#define  SCSI_CMD_WRITE_10           0x2A

#define FSUSB_DiskGetBlockSz(hDisk)         (4 * 1024)


/*
**************************************************************************************************************
*                                       TYPE DEFINITIONS
**************************************************************************************************************
*/

typedef enum  ms_data_dir {

    MS_DATA_DIR_IN     = 0x80,
    MS_DATA_DIR_OUT    = 0x00,
    MS_DATA_DIR_NONE   = 0x01

} MS_DATA_DIR;

/* Type Defines: */
			/** @brief Mass Storage Class Host Mode Configuration and State Structure.
			 *
			 *  Class state structure. An instance of this structure should be made within the user application,
			 *  and passed to each of the Mass Storage class driver functions as the \c MSInterfaceInfo parameter. This
			 *  stores each Mass Storage interface's configuration and state information.
			 */
			typedef struct
			{
				struct
				{
					uint8_t  DataINPipeNumber; /**< Pipe number of the Mass Storage interface's IN data pipe. */
					bool     DataINPipeDoubleBank; /**< Indicates if the Mass Storage interface's IN data pipe should use double banking. */

					uint8_t  DataOUTPipeNumber; /**< Pipe number of the Mass Storage interface's OUT data pipe. */
					bool     DataOUTPipeDoubleBank; /**< Indicates if the Mass Storage interface's OUT data pipe should use double banking. */
					uint8_t  PortNumber;		/**< Port number that this interface is running.
												*/
				} Config; /**< Config data for the USB class interface within the device. All elements in this section
				           *   <b>must</b> be set or the interface will fail to enumerate and operate correctly.
				           */
				struct
				{
					bool     IsActive; /**< Indicates if the current interface instance is connected to an attached device, valid
					                    *   after @ref MS_Host_ConfigurePipes() is called and the Host state machine is in the
					                    *   Configured state.
					                    */
					uint8_t  InterfaceNumber; /**< Interface index of the Mass Storage interface within the attached device. */

					uint16_t DataINPipeSize; /**< Size in bytes of the Mass Storage interface's IN data pipe. */
					uint16_t DataOUTPipeSize;  /**< Size in bytes of the Mass Storage interface's OUT data pipe. */

					uint32_t TransactionTag; /**< Current transaction tag for data synchronizing of packets. */
				} State; /**< State data for the USB class interface within the device. All elements in this section
						  *   <b>may</b> be set to initial values, but may also be ignored to default to sane values when
						  *   the interface is enumerated.
						  */
			} USB_ClassInfo_MS_Host_t;


typedef USB_ClassInfo_MS_Host_t DISK_HANDLE_T;


/*
**************************************************************************************************************
*                                     FUNCTION PROTOTYPES
**************************************************************************************************************
*/

int32_t  MS_BulkRecv          (          uint32_t    block_number,
                                            uint16_t    num_blocks,
                                  volatile  uint8_t   *user_buffer);

int32_t  MS_BulkSend          (          uint32_t    block_number,
                                            uint16_t    num_blocks,
                                  volatile  uint8_t   *user_buffer);
int32_t  MS_ParseConfiguration(void);
int32_t  MS_TestUnitReady     (void);
int32_t  MS_ReadCapacity (uint32_t *numBlks, uint32_t *blkSize);
int32_t  MS_GetMaxLUN         (void);
int32_t  MS_GetSenseInfo      (void);
int32_t  MS_Init (uint32_t *blkSize, uint32_t *numBlks, uint8_t *inquiryResult);
int32_t  MS_Inquire (uint8_t *response);

void        Fill_MSCommand       (          uint32_t    block_number,
                                            uint32_t    block_size,
                                            uint16_t    num_blocks,
                                            MS_DATA_DIR direction,
                                            uint8_t     scsi_cmd,
                                            uint8_t     scsi_cmd_len);

int FSUSB_DiskReadyWait( int tout);

#endif
