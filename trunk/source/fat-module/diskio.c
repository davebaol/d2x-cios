/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include <string.h>

#include "diskio.h"
#include "mem.h"
#include "sdio.h"
#include "syscalls.h"
#include "types.h"
#include "usbstorage.h"

/* Disk drives */
#define DRIVE_SDHC	0
#define DRIVE_EHCI	1

/* Disk constants */
#define SECTOR_SZ(drv)		((drv)==DRIVE_SDHC ? 512 :	\
	((drv)==DRIVE_EHCI ? usbstorage_GetSectorSize() : 0))


DSTATUS disk_initialize(BYTE drv)
{
	/* Initialize drive */
	switch (drv) {
	case DRIVE_SDHC:
	case DRIVE_EHCI:
		break;

	default:
		return RES_PARERR;
	}

	return RES_OK;
}

DSTATUS disk_status(BYTE drv)
{
	s32 ret = 1;

	/* Check drive status */
	switch (drv) {
	case DRIVE_SDHC:
		/* Check SD card status */
		ret = sdio_IsInserted();
		break;

	case DRIVE_EHCI:
		/* Check USB device status */
		ret = usbstorage_IsInserted();
		break;
	}

	return ret ? RES_OK : STA_NODISK;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
	void *buffer;

	u32 len;
	s32 ret = 0;

	/* Buffer length */
	len = count * SECTOR_SZ(drv);
	if (!len)
		return RES_ERROR;

	/* Check buffer alignment */
	if ((u32)buff & 31) {
		/* Allocate buffer */
		buffer = Mem_Alloc(len);
		if (!buffer)
			return RES_ERROR;
	}
	else
		buffer = buff;

	/* Read sectors */
	switch (drv) {
	case DRIVE_SDHC:
		/* Read SD sectors */
		ret = sdio_ReadSectors(sector, count, buffer);
		break;

	case DRIVE_EHCI:
		/* Read USB sectors */
		ret = usbstorage_ReadSectors(sector, count, buffer);
		break;
	}

	if (buffer != buff) {
		/* Copy buffer */
		if (ret)
			memcpy(buff, buffer, len);

		/* Free buffer */
		Mem_Free(buffer);
	}

	return (ret) ? RES_OK : RES_ERROR;
}

#if _READONLY == 0
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
	void *buffer;

	u32 len;
	s32 ret = 0;

	/* Buffer length */
	len = count * SECTOR_SZ(drv);
	if (!len)
		return RES_ERROR;

	/* Check buffer alignment */
	if ((u32)buff & 31) {
		/* Allocate buffer */
		buffer = Mem_Alloc(len);
		if (!buffer)
			return RES_ERROR;

		/* Copy buffer */
		memcpy(buffer, buff, len);
	}
	else
		buffer = (void *)buff;

	/* Write sectors */
	switch (drv) {
	case DRIVE_SDHC:
		/* Write SD sectors */
		ret = sdio_WriteSectors(sector, count, buffer);
		break;

	case DRIVE_EHCI:
		/* Write USB sectors */
		ret = usbstorage_WriteSectors(sector, count, buffer);
		break;
	}

	if (buffer != buff) {
		/* Free buffer */
		Mem_Free(buffer);
	}

	return (ret) ? RES_OK : RES_ERROR;
}
#endif /* _READONLY */

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
	if (ctrl == GET_SECTOR_SIZE) {
		s32 ret = *((WORD *) buff) = (WORD) SECTOR_SZ(drv);

		return (ret) ? RES_OK : RES_ERROR;
	}

	return RES_OK;
}


u32 get_fattime(void)
{
	return 0;
}
