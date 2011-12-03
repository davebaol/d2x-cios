/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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
#include "isfs.h"
#include "syscalls.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_ISFS_FORMAT		1
#define IOCTL_ISFS_GETSTATS		2
#define IOCTL_ISFS_CREATEDIR		3
#define IOCTL_ISFS_READDIR		4
#define IOCTL_ISFS_SETATTR		5
#define IOCTL_ISFS_GETATTR		6
#define IOCTL_ISFS_DELETE		7
#define IOCTL_ISFS_RENAME		8
#define IOCTL_ISFS_CREATEFILE		9
#define IOCTL_ISFS_SETFILEVERCTRL	10
#define IOCTL_ISFS_GETFILESTATS		11
#define IOCTL_ISFS_GETUSAGE		12
#define IOCTL_ISFS_SHUTDOWN		13

/* Buffer */
static struct isfs isfsBuf ATTRIBUTE_ALIGN(32);

/* Variables */
static s32 fd = -1;


s32 ISFS_Open(void)
{
	/* Already open */
	if (fd >= 0)
		return 0;

	/* Open /dev/fs */
	fd = os_open("/dev/fs", 0);
	if (fd < 0)
		return fd;

	return 0;
}

void ISFS_Close(void)
{
	/* Close /dev/fs */
	if (fd >= 0)
		os_close(fd);

	/* Reset descriptor */
	fd = -1;
}

s32 ISFS_CreateFile(const char *filename)
{
	/* Not opened */
	if (fd < 0)
		return IPC_ENOENT;

	/* Set filename */
	strcpy(isfsBuf.fsattr.filepath, filename);

	/* Set attributes */
	isfsBuf.fsattr.owner_id = 0;
	isfsBuf.fsattr.group_id = 0;
	isfsBuf.fsattr.ownerperm = ISFS_OPEN_RW;
	isfsBuf.fsattr.groupperm = ISFS_OPEN_RW;
	isfsBuf.fsattr.otherperm = ISFS_OPEN_RW;

	/* Create file */
	return os_ioctl(fd, IOCTL_ISFS_CREATEFILE, &isfsBuf.fsattr, sizeof(isfsBuf.fsattr), NULL, 0);
}

