/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
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
#include "isfs.h"
#include "syscalls.h"
#include "tools.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_FAT_FILESTATS	11

/* IOCTLV commands */
#ifdef DEBUG
#define IOCTL_FAT_OPEN		0x81
#define IOCTL_FAT_CLOSE		0x82
#define IOCTL_FAT_READ		0x83
#define IOCTL_FAT_WRITE		0x84
#define IOCTL_FAT_SEEK		0x85
#endif
#define IOCTL_FAT_MKDIR		0x01
#define IOCTL_FAT_MKFILE	0x02
#define IOCTL_FAT_READDIR	0x03
#define IOCTL_FAT_READDIR_LFN	0x04
#define IOCTL_FAT_DELETE	0x05
#define IOCTL_FAT_DELETEDIR	0x06
#define IOCTL_FAT_RENAME	0x07
#define IOCTL_FAT_STATS		0x08
#define IOCTL_FAT_GETUSAGE	0x09
#define IOCTL_FAT_MOUNT_SD	0xF0
#define IOCTL_FAT_UMOUNT_SD	0xF1
#define IOCTL_FAT_MOUNT_USB	0xF2
#define IOCTL_FAT_UMOUNT_USB	0xF3

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
#ifdef DEBUG
		struct {
			s32  fd;
		} close, write;

		struct {
			s32  fd;
			s32  where;
			s32  whence;
		} seek;
#endif
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
			u8   padding[24];
			u32  files;
		} usage;
	};
} ATTRIBUTE_PACKED fatBuf;


/* Variables */
static s32 fatFd = 0;

/* I/O buffer */
static fatBuf *iobuf = NULL;

#ifdef DEBUG
/* I/O buffer for logging */
static fatBuf *logiobuf = NULL;
#endif

s32 FAT_Init(void)
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

#ifdef DEBUG
	/* Allocate memory */
	if (!logiobuf) {
		logiobuf = os_heap_alloc_aligned(0, sizeof(*logiobuf), 32);
		if (!logiobuf)
			return IPC_ENOMEM;
	}
#endif

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	return 0;
}

s32 FAT_CreateDir(const char *dirpath)
{
	/* Copy path */
	strcpy(iobuf->filename, dirpath);

	/* Setup vector */
	iobuf->vector[0].data = iobuf->filename;
	iobuf->vector[0].len  = FAT_MAXPATH;

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
		iobuf->vector[2].len  = (FAT_MAXPATH * cnt);
		iobuf->vector[3].data = &iobuf->dir.entries;
		iobuf->vector[3].len  = 4;
	}

	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Read directory */
	ret = os_ioctlv(fatFd, IOCTL_FAT_READDIR, in, io, iobuf->vector);
//	ret = os_ioctlv(fatFd, IOCTL_FAT_READDIR_LFN, in, io, iobuf->vector);


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

	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Get stats */
	ret = os_ioctlv(fatFd, IOCTL_FAT_STATS, 1, 1, iobuf->vector);

	/* Copy data */
	if (ret >= 0 && stats)
		memcpy(stats, &iobuf->stats.stats, sizeof(struct stats));

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

	os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Get usage */
	ret = os_ioctlv(fatFd, IOCTL_FAT_GETUSAGE, 1, 2, iobuf->vector);

	/* Copy data */
	if (ret >= 0) {
		*blocks = (iobuf->usage.size / 0x4000);
		*inodes = (iobuf->usage.files) ? iobuf->usage.files : 1;
	}

	return ret;
}

#ifdef DEBUG
s32 FAT_OpenLog(const char *filepath, u32 mode)
{
	/* Copy path */
	strcpy(logiobuf->open.filename, filepath);
	logiobuf->open.mode = mode;

	/* Setup vector */
	logiobuf->vector[0].data = logiobuf->open.filename;
	logiobuf->vector[0].len  = FAT_MAXPATH;
	logiobuf->vector[1].data = &logiobuf->open.mode;
	logiobuf->vector[1].len  = 8;

	os_sync_after_write(logiobuf, sizeof(*logiobuf));

	/* Open file */
	return os_ioctlv(fatFd, IOCTL_FAT_OPEN, 2, 0, logiobuf->vector);
}

s32 FAT_WriteLog(s32 fd, void *buffer, u32 len)
{
	logiobuf->write.fd = fd;

	/* Setup vector */
	logiobuf->vector[0].data = &logiobuf->write.fd;
	logiobuf->vector[0].len  = 8;
	logiobuf->vector[1].data = buffer;
	logiobuf->vector[1].len  = len;

	os_sync_after_write(logiobuf, sizeof(*logiobuf));

	/* Write file */
	return os_ioctlv(fatFd, IOCTL_FAT_WRITE, 2, 0, logiobuf->vector);
}

s32 FAT_SeekLog(s32 fd, u32 where, u32 whence)
{
	logiobuf->seek.fd = fd;
	logiobuf->seek.where = where;
	logiobuf->seek.whence = whence;

	/* Setup vector */
	logiobuf->vector[0].data = &logiobuf->seek.fd;
	logiobuf->vector[0].len  = 8;
	logiobuf->vector[1].data = &logiobuf->seek.where;
	logiobuf->vector[1].len  = 8;
	logiobuf->vector[2].data = &logiobuf->seek.whence;
	logiobuf->vector[2].len  = 8;

	os_sync_after_write(logiobuf, sizeof(*logiobuf));

	/* Seek file */
	return os_ioctlv(fatFd, IOCTL_FAT_SEEK, 3, 0, logiobuf->vector);
}

s32 FAT_CloseLog(s32 fd)
{
	logiobuf->close.fd = fd;

	/* Setup vector */
	logiobuf->vector[0].data = &logiobuf->close.fd;
	logiobuf->vector[0].len  = 8;

	os_sync_after_write(logiobuf, sizeof(*logiobuf));

	/* Close file */
	return os_ioctlv(fatFd, IOCTL_FAT_CLOSE, 1, 0, logiobuf->vector);
}
#endif
