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

#ifndef _FAT_H_
#define _FAT_H_

#include "types.h"

/* Stats structure */
struct stats {
	/* Size */
	u32 size;

	/* Date and time */
	u16 date;
	u16 time;

	/* Attributes */
	u8 attrib;
};

/* File stats structure */
struct fstats {
	/* Length and position */
	u32 length;
	u32 pos;
};

/* Constants */
#define FAT_MAXPATH	256

/* Attributes */
#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */


/* Prototypes */
s32 FAT_Init(void);
s32 FAT_CreateDir(const char *dirpath);
s32 FAT_CreateFile(const char *filepath);
s32 FAT_ReadDir(const char *dirpath, void *outbuf, u32 *entries);
s32 FAT_Delete(const char *path);
s32 FAT_DeleteDir(const char *dirpath);
s32 FAT_Rename(const char *oldpath, const char *newpath);
s32 FAT_GetStats(const char *path, struct stats *stats);
s32 FAT_GetUsage(const char *path, u32 *blocks, u32 *inodes);
 
#endif
