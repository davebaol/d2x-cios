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

#ifndef _ISFS_H_
#define _ISFS_H_

#include "ipc.h"
#include "types.h"

/* Constants */
#define ISFS_MAXPATH		64

#define ISFS_OPEN_READ		0x01
#define ISFS_OPEN_WRITE		0x02
#define ISFS_OPEN_RW		(ISFS_OPEN_READ | ISFS_OPEN_WRITE)

/* Mode codes */
#define FS_MODE_NAND  0x00
#define FS_MODE_SDHC  0x01
#define FS_MODE_USB   0x02
#define FS_MODE_FULL  0x100
#define FS_MODE_LED   0x200

/* Macros */
#define FS_GetDevice(M)  ((M) & (FS_MODE_SDHC | FS_MODE_USB))

/* FS error codes */
#define FS_SUCCESS         0
#define FS_EINVAL         -4
#define FS_EFATAL       -101
#define FS_EACCESS      -102
#define FS_EEXIST       -105
#define FS_ENOENT       -106
#define FS_ENFILE       -107
#define FS_ENAMETOOLONG -110
#define FS_ENOTEMPTY    -115


/* ISFS structures */
typedef struct {
	char filepathOld[ISFS_MAXPATH];
	char filepathNew[ISFS_MAXPATH];
} fsrename;

typedef struct {
	u32 owner_id;
	u16 group_id;
	char filepath[ISFS_MAXPATH];
	u8 ownerperm;
	u8 groupperm;
	u8 otherperm;
	u8 attributes;
	u8 pad0[2];
} fsattr;

typedef struct {
	ioctlv vector[4];
	u32 no_entries;
} fsreaddir;

typedef struct {
	ioctlv vector[4];
	u32 usage1;
	u8  pad0[28];
	u32 usage2;
} fsusage;

typedef struct {
	u32 block_size;
	u32 free_blocks;
	u32 used_blocks;
	u32 unk3;
	u32 unk4;
	u32 free_inodes;
	u32 unk5;
} fsstats;

typedef struct {
	/* Length and position */
	u32 length;
	u32 pos;
} fsfilestats;

typedef struct {
	u32  mode;
	s32  partition;
	char nandpath[ISFS_MAXPATH];
} fsconfig;

#endif

