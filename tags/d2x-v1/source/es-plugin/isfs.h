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

#ifndef _ISFS_H_
#define _ISFS_H_

#include "ipc.h"
#include "types.h"

/* Constants */
#define ISFS_MAXPATH		64

#define ISFS_OPEN_READ		0x01
#define ISFS_OPEN_WRITE		0x02
#define ISFS_OPEN_RW		(ISFS_OPEN_READ | ISFS_OPEN_WRITE)

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
			u32 a;
			u32 b;
			u32 c;
			u32 d;
			u32 e;
			u32 f;
			u32 g;
		} fsstats;
	};
};


/* Prototypes */
s32  ISFS_Open(void);
void ISFS_Close(void);
s32  ISFS_CreateFile(const char *filename);

#endif

