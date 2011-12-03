/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "dip_calls.h"
#include "syscalls.h"

/* Commands */
#define IOCTL_WBFS_BASE		(('W'<<24) | ('F'<<16) | ('S'<<8))
#define IOCTL_WBFS_OPEN		(IOCTL_WBFS_BASE + 0x1)
#define IOCTL_WBFS_READ		(IOCTL_WBFS_BASE + 0x2)

/* Variables */
static char *devFs[] = { "/dev/usb2", "/dev/sdio/sdhc" };
static s32   devFd   = -1;


/* I/O buffer */
static ioctlv vector[4] ATTRIBUTE_ALIGN(32);
static u32    buffer[9] ATTRIBUTE_ALIGN(32);


s32 WBFS_Open(u32 device, u8 *discid)
{
	/* Open device */
	devFd = os_open(devFs[device], 1);
	if (devFd < 0)
		return devFd;

	/* Copy disc ID */
	memcpy(buffer, discid, 6);

	/* Setup vector */
	vector[0].data = buffer;
	vector[0].len  = 6;

	/* Open disc */
	return os_ioctlv(devFd, IOCTL_WBFS_OPEN, 1, 0, vector);
}

void WBFS_Close(void)
{
	/* Close device */
	if (devFd >= 0)
		os_close(devFd);

	/* Reset descriptor */
	devFd = -1;
}

s32 WBFS_Read(void *outbuf, u32 len, u32 offset)
{
	s32 ret;

	/* Set buffers */
	buffer[0] = offset;
	buffer[8] = len;

	/* Setup vector */
	vector[0].data = &buffer[0];
	vector[0].len  = 4;
	vector[1].data = &buffer[8];
	vector[1].len  = 4;
	vector[2].data = outbuf;
	vector[2].len  = len;

	/* Flush cache */
	os_sync_after_write(vector, sizeof(vector));
	os_sync_after_write(buffer, sizeof(buffer));

	/* Read data */
	ret = os_ioctlv(devFd, IOCTL_WBFS_READ, 2, 1, vector);
	if (ret)
		return ret;

	/* Invalidate cache */
	os_sync_before_read(outbuf, len);

	return 0;
}
