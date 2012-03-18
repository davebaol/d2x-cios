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

#ifndef _FAT_H_
#define _FAT_H_

#include "ff.h"
#include "types.h"

/* Constants */
#define MAX_FILE        20
#define FAT_MAXPATH     256


/* Stats structure */
struct stats {
	/* Size */
	u32 size;

	/* Date and time */
	u16 date;
	u16 time;

	/* Attributes */
	u8 attrib;

	/* Padding */
	u8 pad[3];
};

/* File stats structure */
struct fstats {
	/* Length and position */
	u32 length;
	u32 pos;
};

/* Prototypes */
void FAT_GeneratePath(const char *fspath, char *fatpath);
s32 FAT_Initialize();
s32 FAT_Mount(u8 device, s32 partition);
s32 FAT_Unmount(u8 dev);
s32 FAT_Open(FIL *fil, const char *path, u32 mode);
s32 FAT_Close(FIL *fil);
s32 FAT_Read(FIL *fil, void *buffer, u32 len);
s32 FAT_Write(FIL *fil, void *buffer, u32 len);
s32 FAT_Seek(FIL *fil, s32 where, s32 whence);
s32 FAT_CreateDir(const char *dirpath);
s32 FAT_CreateFile(const char *filepath);
s32 FAT_ReadDir(const char *dirpath, char *outbuf, u32 buflen, u32 *outlen, u32 entries);
s32 FAT_Delete(const char *path);
s32 FAT_DeleteDir(const char *dirpath);
s32 FAT_Rename(const char *oldname, const char *newname);
s32 FAT_GetStats(const char *path, struct stats *stats);
s32 FAT_GetUsage(const char *path, u32 *blocks, u32 *inodes);
s32 FAT_GetFileStats(FIL *fil, struct fstats *stats);

#endif
