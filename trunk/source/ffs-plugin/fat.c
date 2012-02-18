/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
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
#include "fs_calls.h"
#include "ipc.h"
#include "syscalls.h"
#include "tools.h"
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
#define IOCTL_FAT_SETNANDPATH	0xF5

/* FAT structure */
typedef struct {
	ioctlv vector[8];

	union {
		char filename[FAT_MAXPATH];

		struct fstats fstats;

		struct {
			char filename[FAT_MAXPATH];
			s32  mode;
		} open;

		struct {
			char filename[FAT_MAXPATH];
			s32  entries;
		} dir;

		struct {
			char oldname[FAT_MAXPATH];
			char newname[FAT_MAXPATH];
		} rename;

		struct {
			char filename[FAT_MAXPATH];
			struct stats stats;
		} stats;

		struct {
			char filename[FAT_MAXPATH];
			u64  size;
			u8   padding0[24];
			u32  files;
			u8   padding1[28];
			u32  dirs;
		} usage;
	};
} ATTRIBUTE_PACKED fatBuf;


/* Variables */
static s32 fatFd = 0;

/* I/O buffer */
static fatBuf *iobuf = NULL;


s32 FAT_Init(const char *nandpath)
{
	/* FAT module already open */
	if (fatFd > 0)
		return 0;

	/* Allocate memory */
	if (!iobuf) {
		iobuf = os_heap_alloc_aligned(0, sizeof(*iobuf), 32);
		if (!iobuf)
			return IPC_ENOMEM;
	}

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Copy path */
	strcpy(iobuf->filename, nandpath);

	/* Flush cache */
	os_sync_after_write(iobuf->filename, FAT_MAXPATH);

	/* Set nand path */
	return os_ioctl(fatFd, IOCTL_FAT_SETNANDPATH, iobuf->filename, FAT_MAXPATH, NULL, 0);
}

s32 FAT_CreateDir(const char *dirpath)
{
	/* Copy path */
	strcpy(iobuf->filename, dirpath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->filename;
	iobuf->vector[0].len  = FAT_MAXPATH;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Create directory */
	return os_ioctlv(fatFd, IOCTL_FAT_MKDIR, 1, 0, iobuf->vector);
}

s32 FAT_CreateFile(const char *filepath)
{
	/* Copy path */
	strcpy(iobuf->filename, filepath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->filename;
	iobuf->vector[0].len  = FAT_MAXPATH;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Create file */
	return os_ioctlv(fatFd, IOCTL_FAT_MKFILE, 1, 0, iobuf->vector);
}

s32 FAT_ReadDir(const char *dirpath, void *outbuf, u32 *entries)
{
	u32 in = 1, io = 1;
	s32 ret;

	/* Copy path */
	strcpy(iobuf->dir.filename, dirpath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->dir.filename;
	iobuf->vector[0].len  = FAT_MAXPATH;
	iobuf->vector[1].data = &iobuf->dir.entries;
	iobuf->vector[1].len  = 4;

	if (outbuf) {
		u32 cnt = *entries;

		/* Input/Output buffers */
		in = io = 2;

		/* Entries value */
		iobuf->dir.entries = cnt;

		/* Setup vector */
		iobuf->vector[2].data = outbuf;
		// FIX
		// Since in FFS the file name can not be longer than 12 characters, 
		// FAT_MAXPATH has been replaced with 13 (12 + 1 for the final '\0')
		// in d2x v4beta1 to prevent potential problems in the os_ioctlv below.
		iobuf->vector[2].len  = (/*FAT_MAXPATH*/13 * cnt); 
		iobuf->vector[3].data = &iobuf->dir.entries;
		iobuf->vector[3].len  = 4;
	}

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Read directory */
	ret = os_ioctlv(fatFd, IOCTL_FAT_READDIR_FS, in, io, iobuf->vector);

	os_sync_before_read(iobuf, sizeof(*iobuf));
	
	/* Copy data */
	if (ret >= 0)
		*entries = iobuf->dir.entries;

	return ret;
}

s32 FAT_Delete(const char *path)
{
	/* Copy path */
	strcpy(iobuf->filename, path);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->filename;
	iobuf->vector[0].len  = FAT_MAXPATH;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Delete */
	return os_ioctlv(fatFd, IOCTL_FAT_DELETE, 1, 0, iobuf->vector);
}

s32 FAT_DeleteDir(const char *dirpath)
{
	/* Copy path */
	strcpy(iobuf->filename, dirpath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->filename;
	iobuf->vector[0].len  = FAT_MAXPATH;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Delete directory */
	return os_ioctlv(fatFd, IOCTL_FAT_DELETEDIR, 1, 0, iobuf->vector);
}

s32 FAT_Rename(const char *oldpath, const char *newpath)
{
	/* Copy paths */
	strcpy(iobuf->rename.oldname, oldpath);
	strcpy(iobuf->rename.newname, newpath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->rename.oldname;
	iobuf->vector[0].len  = FAT_MAXPATH;
	iobuf->vector[1].data = iobuf->rename.newname;
	iobuf->vector[1].len  = FAT_MAXPATH;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Rename */
	return os_ioctlv(fatFd, IOCTL_FAT_RENAME, 2, 0, iobuf->vector);
}

s32 FAT_GetStats(const char *path, struct stats *stats)
{
	s32 ret;

	/* Copy path */
	strcpy(iobuf->stats.filename, path);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->stats.filename;
	iobuf->vector[0].len  = FAT_MAXPATH;
	iobuf->vector[1].data = &iobuf->stats.stats;
	iobuf->vector[1].len  = sizeof(struct stats);

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Get stats */
	ret = os_ioctlv(fatFd, IOCTL_FAT_STATS, 1, 1, iobuf->vector);

 	os_sync_before_read(iobuf, sizeof(*iobuf));

	/* Copy data */
	if (ret >= 0 && stats) {
		memcpy(stats, &iobuf->stats.stats, sizeof(struct stats));

		/* Flush cache */
		os_sync_after_write(stats, sizeof(struct stats));
	}

	return ret;
}

s32 FAT_GetUsage(const char *path, u32 *blocks, u32 *inodes)
{
	s32 ret;

	/* Copy path */
	strcpy(iobuf->usage.filename, path);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->usage.filename;
	iobuf->vector[0].len  = FAT_MAXPATH;
	iobuf->vector[1].data = &iobuf->usage.size;
	iobuf->vector[1].len  = 8;
	iobuf->vector[2].data = &iobuf->usage.files;
	iobuf->vector[2].len  = 4;
	iobuf->vector[3].data = &iobuf->usage.dirs;
	iobuf->vector[3].len  = 4;

	/* Flush cache */
	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Get usage */
	ret = os_ioctlv(fatFd, IOCTL_FAT_GETUSAGE_FS, 1, 3, iobuf->vector);

	os_sync_before_read(iobuf, sizeof(*iobuf));

	FS_printf("FS_GetUsage(): ret = %d, size = %llu, files = %u, dirs = %u\n", ret, iobuf->usage.size, iobuf->usage.files, iobuf->usage.dirs);

	/* Copy data */
	if (ret >= 0) {
		*blocks = (u32) (iobuf->usage.size / 0x4000);
		*inodes = iobuf->usage.files + 1;

		/*
		 * This check limits the number of used blocks to 128 MB
		 * to prevent no space error in case of big emulated nand
		 */
		if (*blocks > 0x2000) 
			*blocks = 0x1000 + (*blocks & 0xFFF);
			
		FS_printf("FS_GetUsage(): blocks = %x, inodes = %u\n", *blocks, *inodes);
	}

	return ret;
	}

s32 FAT_GetFileStats(s32 fd, struct fstats *stats)
{
	s32 ret;

	/* Get filestats */
	ret = os_ioctl(fd, IOCTL_FAT_FILESTATS, NULL, 0, iobuf, sizeof(struct fstats));

	/* Copy data */
	if (ret >= 0) {
		memcpy(stats, iobuf, sizeof(struct fstats));

		/* Flush cache */
		os_sync_after_write(stats, sizeof(struct fstats));
	}

	return ret;
}
