
#include "syscalls.h"
#include "types.h"
#include "sdhc.h"

#include <stdio.h>
#include <string.h>

/* IOCTL commands */
#define IOCTL_SDHC_INIT		0x01
#define IOCTL_SDHC_READ		0x02
#define IOCTL_SDHC_WRITE	0x03
#define IOCTL_SDHC_ISINSERTED	0x04

/* Constants */
#define SDHC_SECTOR_SIZE	0x200

/* Variables */
static char   fs[] ATTRIBUTE_ALIGN(32) = "/dev/sdio/sdhc";
static ioctlv io_vector[3] ATTRIBUTE_ALIGN(32);
static u32    io_buffer[3] ATTRIBUTE_ALIGN(32);
static s32 fd = -1;
static u32 sectorSz = SDHC_SECTOR_SIZE;


int sdhc_Init(void)
{
	//s32 ret;

	/* Already open */
	if (fd >= 0)
		return 0;

	/* Open USB device */
	fd = os_open(fs, 1);
	if (fd < 0)
		return -11;

	/* Initialize USB storage */
	os_ioctlv(fd, IOCTL_SDHC_INIT, 0, 0, NULL);

	sectorSz = SDHC_SECTOR_SIZE;

	return 0;
}


bool sdhc_Read(u32 sector, u32 numSectors, void *buffer)
{
	u32 cnt;
	s32 ret;

	/* Device not opened */
	if (fd < 0)
		return false;

	/* Sector info */
	io_buffer[0] = sector;
	io_buffer[1] = numSectors;

	/* Setup io_vector */
	io_vector[0].data = &io_buffer[0];
	io_vector[0].len  = sizeof(u32);
	io_vector[1].data = &io_buffer[1];
	io_vector[1].len  = sizeof(u32);
	io_vector[2].data = buffer;
	io_vector[2].len  = (sectorSz * numSectors);

	/* Flush cache */
	for (cnt = 0; cnt < 3; cnt++)
		os_sync_after_write(io_vector[cnt].data, io_vector[cnt].len);

	os_sync_after_write(io_vector, sizeof(ioctlv) * 3);

	/* Read data */
	ret = os_ioctlv(fd, IOCTL_SDHC_READ, 2, 1, io_vector);
	if (ret < 0)
		return false;

	/* Invalidate cache */
	for (cnt = 0; cnt < 3; cnt++)
		os_sync_before_read(io_vector[cnt].data, io_vector[cnt].len);

	return true;
}


bool sdhc_Shutdown(void)
{
	if (fd >= 0) {
		/* Close USB device */
		os_close(fd);
		/* Remove descriptor */
		fd = -1;
	}
	return true;
}


