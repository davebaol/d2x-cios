#ifndef _USBSTORAGE_H_
#define _USBSTORAGE_H_

#include "types.h"

/* Constants */
#define	USBSTORAGE_OK			0
#define	USBSTORAGE_ENOINTERFACE		-10000
#define	USBSTORAGE_ESENSE		-10001
#define	USBSTORAGE_ESHORTWRITE		-10002
#define	USBSTORAGE_ESHORTREAD		-10003
#define	USBSTORAGE_ESIGNATURE		-10004
#define	USBSTORAGE_ETAG			-10005
#define	USBSTORAGE_ESTATUS		-10006
#define	USBSTORAGE_EDATARESIDUE		-10007
#define	USBSTORAGE_ETIMEDOUT		-10008
#define	USBSTORAGE_EINIT		-10009
#define USBSTORAGE_PROCESSING		-10010

#define USB_CLASS_MASS_STORAGE		0x08


/* USB storage structure */
typedef struct
{
	u8 configuration;
	u32 interface;
	u32 altInterface;

	u8 ep_in;
	u8 ep_out;

	u8 max_lun;
	u32 *sector_size;

	s32 usb_fd;

	s32 retval;

	u32 tag;
	u8 suspended;

	u8 *buffer;
} usbstorage_handle;

/* Prototypes */
s32 USBStorage_Initialize(void);
s32 USBStorage_Open(usbstorage_handle *dev, s32 device_id, u16 vid, u16 pid);
s32 USBStorage_Close(usbstorage_handle *dev);
s32 USBStorage_Reset(usbstorage_handle *dev);
s32 USBStorage_GetMaxLUN(usbstorage_handle *dev);
s32 USBStorage_MountLUN(usbstorage_handle *dev, u8 lun);
s32 USBStorage_Suspend(usbstorage_handle *dev);
s32 USBStorage_ReadCapacity(usbstorage_handle *dev, u8 lun, u32 *sector_size, u32 *n_sectors);
s32 USBStorage_Read(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, u8 *buffer);
s32 USBStorage_Write(usbstorage_handle *dev, u8 lun, u32 sector, u16 n_sectors, const u8 *buffer);

#endif
