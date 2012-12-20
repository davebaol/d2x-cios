/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include <string.h>

#include "diskio.h"
#include "isfs.h"
#include "mem.h"
#include "plugin.h"
#include "sdio.h"
#include "syscalls.h"
#include "types.h"
#include "usbstorage.h"

/* Disk drives */
#define DRIVE_SDHC	(FS_MODE_SDHC - 1)
#define DRIVE_EHCI	(FS_MODE_USB - 1)

/* Disk constants */
#define SECTOR_SZ(drv)		((drv)==DRIVE_SDHC ? 512 :	\
	((drv)==DRIVE_EHCI ? usbstorage_GetSectorSize() : 0))


DSTATUS disk_initialize(BYTE drv)
{
	/* Initialize drive */
	return drv == DRIVE_SDHC || drv == DRIVE_EHCI ? RES_OK : RES_PARERR;
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

#include "hollywood.h"
#include "swi_mload.h"

#define get_timer()  (*((vu32*)HW_TIMER))

#define TICKS_PER_MILLISEC         2048
#define MINIMUM_TIME_IN_MILLISECS  (100 * TICKS_PER_MILLISEC) 

static u32 ticks  = 0; 
static u32 led_on = 0; 

static void __LED_On(void)
{
	if (config.mode & FS_MODE_LED) {
		led_on = (get_timer() - ticks > MINIMUM_TIME_IN_MILLISECS);

		if (led_on)
			Swi_LedOn();
	}
	else
		led_on = 0;
}

static void __LED_Off(void)
{
	if (led_on) {
		Swi_LedOff();
		
		ticks = get_timer();
	}
}

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

	/* Set led on */
	__LED_On();

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

	/* Set led off */
	__LED_Off();

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
