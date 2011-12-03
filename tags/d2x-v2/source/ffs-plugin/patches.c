/*
 * FFS plugin for Custom IOS.
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

#include "fs_calls.h"
#include "tools.h"

/* Macros */
#define Write8(addr, val)	\
	*(u8 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u8));

#define Write16(addr, val)	\
	*(u16 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u16));

#define Write32(addr, val)	\
	*(u32 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u32));


/* FFS jump table */
static u32 fsTable[8]=
{
	(u32)fs_unk,
	(u32)fs_open,
	(u32)fs_close,
	(u32)fs_read,
	(u32)fs_write,
	(u32)fs_seek,
	(u32)fs_ioctl,
	(u32)fs_ioctlv,
};

/* Addresses */
u32 addrSysOpen = 0;
u32 addrPrintf  = 0;
u32 addrReentry = 0;
u32 addrTable   = 0;


void Patch_FfsModule(u32 version)
{
	switch (version) {
	/** 12/24/08 13:48:17 **/
	case 0x49523DA1:
		/* Set addresses */
		addrTable   = *(u32 *)0x20005F38;
		addrPrintf  = 0x20006084 + 1;
		addrReentry = 0x20005F0A;

		/* Patch command handler */
		Write32(0x20005F38, (u32)fsTable);

		break;

	/** 12/23/08 17:26:21 **/
	case 0x49511F3D:
		/* Set addresses */
		addrTable   = *(u32 *)0x200021D0;
		addrPrintf  = 0x200060BC + 1;
		addrReentry = 0x2000219C;

		/* Patch command handler */
		Write32(0x200021D0, (u32)fsTable);

		break;

	/** 11/24/08 15:36:10 **/
	case 0x492AC9EA:
		/* Set addresses */
		addrTable   = *(u32 *)0x200061B8;
		addrPrintf  = 0x20006304 + 1;
		addrReentry = 0x2000618A;

		/* Patch command handler */
		Write32(0x200061B8, (u32)fsTable);

		break;
	}
}

void Patch_IopModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:29 **/
	/** 03/01/10 03:28:58 **/
	case 0x48776F75:
		/* Set addresses */
		addrSysOpen = 0xFFFF2E5C;

		/* Patch syscall open */
		Write32(0xFFFF2E50, 0x477846C0);
		Write32(0xFFFF2E54, 0xE51FF004);
		Write32(0xFFFF2E58, (u32)syscall_open);

		break;

	/** 12/23/08 17:28:32 **/
	case 0x49511FC0:
		/* Set addresses */
		addrSysOpen = 0xFFFF2D4C;

		/* Patch syscall open */
		Write32(0xFFFF2D40, 0x477846C0);
		Write32(0xFFFF2D44, 0xE51FF004);
		Write32(0xFFFF2D48, (u32)syscall_open);

		break;

	/** 03/01/10 03:13:17 **/
	case 0x4B8B30CD:
		/* Set addresses */
		addrSysOpen = 0xFFFF2E10;

		/* Patch syscall open */
		Write32(0xFFFF2E04, 0x477846C0);
		Write32(0xFFFF2E08, 0xE51FF004);
		Write32(0xFFFF2E0C, (u32)syscall_open);

		break;

	/** 11/24/08 15:39:12 **/
	/** 06/03/09 07:49:12 **/
	/** 03/03/10 10:43:18 **/
	case 0x492ACAA0:
	case 0x4B8E3D46:
		/* Set addresses */
		addrSysOpen = 0xFFFF302C;

		/* Patch syscall open */
		Write32(0xFFFF3020, 0x477846C0);
		Write32(0xFFFF3024, 0xE51FF004);
		Write32(0xFFFF3028, (u32)syscall_open);

		break;
	}
}
