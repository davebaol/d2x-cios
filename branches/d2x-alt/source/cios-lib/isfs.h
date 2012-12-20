/*   
	Custom IOS Library

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 WiiGator.
	Copyright (C) 2009 Waninkoko.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2011 davebaol.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

/* ISFS Mode codes */
#define ISFS_MODE_NAND		0x00
#define ISFS_MODE_SDHC		0x01
#define ISFS_MODE_USB		0x02
#define ISFS_MODE_FULL		0x100

/* Macros */
#define ISFS_GetDevice(M)  ((M) & (ISFS_MODE_SDHC | ISFS_MODE_USB))

/* FS error codes */
#define FS_SUCCESS	 0
#define FS_EINVAL	-4
#define FS_EFATAL	-101
#define FS_EACCESS	-102
#define FS_EEXIST	-105
#define FS_ENOENT	-106
#define FS_ENFILE	-107
#define FS_ENAMETOOLONG	-110
#define FS_ENOTEMPTY	-115

typedef struct {
	u32  mode;
	s32  partition;
	char nandpath[ISFS_MAXPATH];
} fsconfig;

/* ISFS structure */
struct isfs
{
	char filepath[ISFS_MAXPATH];
	
	union {
		struct {
			char filepathOld[ISFS_MAXPATH];
			char filepathNew[ISFS_MAXPATH];
		} fsrename;

		struct {
			char filepath[ISFS_MAXPATH];
		} fsdelete;

		struct {
			u32  owner_id;
			u16  group_id;
			char filepath[ISFS_MAXPATH];
			u8   ownerperm;
			u8   groupperm;
			u8   otherperm;
			u8   attributes;
			u8   pad0[2];
		} fsattr;

		struct {
			ioctlv vector[4];
			u32    no_entries;
		} fsreaddir;

		struct {
			ioctlv vector[4];
			u32 usage1;
			u8  pad0[28];
			u32 usage2;
		} fsusage;

		struct {
			u32 block_size;
			u32 free_blocks;
			u32 used_blocks;
			u32 unk3;
			u32 unk4;
			u32 free_inodes;
			u32 unk5;
		} fsstats;

		fsconfig fsconfig;
	};
};


/* Prototypes */
s32  ISFS_Open(void);
void ISFS_Close(void);
s32  ISFS_CreateFile(const char *filename);
s32  ISFS_Delete(const char *filename);
s32  ISFS_SetConfig(u32 mode, s32 partition, char *path);
s32  ISFS_GetConfig(u32 *mode, s32 *partition, char *path);

#endif

