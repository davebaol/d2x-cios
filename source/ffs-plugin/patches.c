/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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

#include "fs_calls.h"
#include "syscalls.h"
#include "tools.h"

/* Macros */
#define Write8(addr, val)	\
	*(u8 *)(addr) = (val);	\
	DCFlushRange((void *)(addr), sizeof(u8));

#define Write16(addr, val)	\
	*(u16 *)(addr) = (val);	\
	DCFlushRange((void *)(addr), sizeof(u16));

#define Write32(addr, val)	\
	*(u32 *)(addr) = (val);	\
	DCFlushRange((void *)(addr), sizeof(u32));

#define __Patch_FfsModule(at, ap, ar)       \
{                                           \
	/* Set addresses */                 \
	addrTable   = *(u32 *)(at);         \
	addrPrintf  = (ap) + 1;             \
	addrReentry = (ar);                 \
	                                    \
	/* Patch command handler */         \
	Write32((at), (u32)fsTable);        \
}

#define __Patch_IopModule(aso, ap)             \
{                                              \
	/* Set addresses */                    \
	addrSysOpen = (aso);                   \
	                                       \
	/* Patch syscall open like that */     \
	/*      bx  pc                  */     \
	/*      ldr pc, =syscall_open   */     \
	Write32((ap),     0x477846C0);         \
	Write32((ap) + 4, 0xE51FF004);         \
	Write32((ap) + 8, (u32)syscall_open);  \
}

/* FFS jump table */
static u32 fsTable[8] =
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
		__Patch_FfsModule(0x20005F38, 0x20006084, 0x20005F0A);
		break;

	/** 12/23/08 17:26:21 **/
	case 0x49511F3D:
		__Patch_FfsModule(0x200021D0, 0x200060BC, 0x2000219C);
		break;

	/** 11/24/08 15:36:10 **/
	case 0x492AC9EA:
		__Patch_FfsModule(0x200061B8, 0x20006304, 0x2000618A);
		break;

	default:
		write("FFSP: Error -> Can't patch FFS module (unknown version)\n");
		break;
	}
}

void Patch_IopModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:29 **/
	/** 03/01/10 03:28:58 **/
	case 0x48776F75:
		__Patch_IopModule(0xFFFF2E5C, 0xFFFF2E50);
		break;

	/** 12/23/08 17:28:32 **/
	case 0x49511FC0:
		__Patch_IopModule(0xFFFF2D4C, 0xFFFF2D40);
		break;

	/** 03/01/10 03:13:17 **/
	case 0x4B8B30CD:
		__Patch_IopModule(0xFFFF2E10, 0xFFFF2E04);
		break;

	/** 11/24/08 15:39:12 **/
	/** 06/03/09 07:49:12 **/
	/** 03/03/10 10:43:18 **/
	case 0x492ACAA0:
	case 0x4B8E3D46:
		__Patch_IopModule(0xFFFF302C, 0xFFFF3020);
		break;

	default:
		write("FFSP: Error -> Can't patch IOP module (unknown version)\n");
		break;
	}
}
