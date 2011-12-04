/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2011 davebaol.
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

#include "fat.h"
#include "dip_calls.h"
#include "ipc.h"
#include "syscalls.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_FAT_FILESTATS	11

/* IOCTLV commands */
#define IOCTL_FAT_MKDIR		0x01
#define IOCTL_FAT_MKFILE	0x02
#define IOCTL_FAT_READDIR_FS	0x03
#define IOCTL_FAT_READDIR	0x04
#define IOCTL_FAT_DELETE	0x05
#define IOCTL_FAT_DELETEDIR	0x06
#define IOCTL_FAT_RENAME	0x07
#define IOCTL_FAT_STATS		0x08
#define IOCTL_FAT_GETUSAGE_FS	0x09
#define IOCTL_FAT_GETUSAGE	0x0A
#define IOCTL_FAT_MOUNT_SD	0xF0
#define IOCTL_FAT_UMOUNT_SD	0xF1
#define IOCTL_FAT_MOUNT_USB	0xF2
#define IOCTL_FAT_UMOUNT_USB	0xF3
#define IOCTL_FAT_GETPARTITION	0xF4

/* FAT structure */
typedef struct {
	ioctlv vector[4];

	u32 device;

	u32 partition;

} ATTRIBUTE_PACKED fatBuf;


s32 FAT_Mount(u32 device, u32 partition)
{
	s32 fatFd, ret;
	fatBuf *iobuf;

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Allocate memory */
	iobuf = DI_Alloc(sizeof(fatBuf), 32);

	if (iobuf) {
		/* Set command */
		u32 cmd = (device == 0 ? IOCTL_FAT_MOUNT_SD : IOCTL_FAT_MOUNT_USB); 

		/* Set partition */
		iobuf->partition = partition;

		/* Setup vector */
		iobuf->vector[0].data = &iobuf->partition;
		iobuf->vector[0].len = sizeof(u32);

		/* Flush cache */
		//os_sync_after_write(iobuf, sizeof(*iobuf)); 

		/* Mount device */
		ret = os_ioctlv(fatFd, cmd, 1, 0, iobuf->vector);

		/* Free memory */
		DI_Free(iobuf);
	}
	else
		ret = IPC_ENOMEM;

	/* Close FAT module */
	os_close(fatFd);

	return ret;
}

s32 FAT_GetPartition(u32 device, u32 *partition)
{
	s32 fatFd, ret;
	fatBuf *iobuf;

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Allocate memory */
	iobuf = DI_Alloc(sizeof(fatBuf), 32);

	if (iobuf) {
		/* Set device */
		iobuf->device = device;

		/* Setup vector */
		iobuf->vector[0].data = &iobuf->device;
		iobuf->vector[0].len = sizeof(u32);
		iobuf->vector[1].data = &iobuf->partition;
		iobuf->vector[1].len = sizeof(u32);

		/* Flush cache */
		//os_sync_after_write(iobuf, sizeof(*iobuf)); 

		/* Retrieve  partition */
		ret = os_ioctlv(fatFd, IOCTL_FAT_GETPARTITION, 1, 1, iobuf->vector);

		/* Invalidate cache */
		//os_sync_before_read(iobuf, sizeof(*iobuf)); 

		/* Set partition */
		*partition = iobuf->partition;

		/* Free memory */
		DI_Free(iobuf);
	}
	else
		ret = IPC_ENOMEM;

	/* Close FAT module */
	os_close(fatFd);

	return ret;
}
