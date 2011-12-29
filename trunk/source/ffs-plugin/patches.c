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

#include "iop_calls.h"
#include "fs_calls.h"
#include "fs_dump.h"
#include "syscalls.h"
#include "tools.h"


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
u32 addrReentry = 0;
u32 addrTable   = 0;

/* Function pointers */
s32 (*FS_printf)(const char * fmt, ...) = 0;

#ifdef DEBUG

u32 __geThumbLongBranchWithLinkDestAddr(u32 addr)
{
	s32 offsetHigh = ((s32)((*(u16 *)addr)       & 0x07FF)) << 12;
	s32 offsetLow  = ((s32)((*(u16 *)(addr + 2)) & 0x07FF)) << 1;
	s32 offset     = ((offsetHigh + offsetLow) << 8) >> 8;
	return addr + offset + 4;
}

u32 __geArmBranchDestAddr(u32 addr)
{
	s32 offset = ((*(s32*)addr) << 8) >> 8;
	return addr + (offset * 4) + 8;
}

#define __geArmBranchDestOffset(srcAddr, dstAddr)   ((((dstAddr)-(srcAddr))/4-2) & 0xFFFFFF)

#endif

void __Patch_FfsModule(u32 aTable, u32 aReentry, u32 aPrintf)
{
	/* Set addresses */
	addrTable   = *(u32 *)aTable;
	addrReentry = aReentry;

	/* Set function pointers */
	FS_printf = (void *)aPrintf + 1;

	/* Patch command handler */
	DCWrite32(aTable, (u32)fsTable);

#ifdef DEBUG
#ifdef DUMP
/*
	// IOS: 56v5661, 57v5918, 58v6175, 60v6174, 61v5661, 70v6687, 80v6943
	// aMsgAck1 = 0x20007138
	// aMsgAck2 = 0x20006CC4
*/

	// Calculate address from long branch with link (thumb mode)
	u32 aMsgAck1 = __geThumbLongBranchWithLinkDestAddr(aReentry + 2) + 4;

	// Calculate address from branch (ARM mode)
	u32 aMsgAck2 = __geArmBranchDestAddr(aMsgAck1) + 8;
 
	/* Patch syscall os_message_queue_ack */
	if (*((u32 *) aMsgAck2) == 0xE6000570) {
		/* Patch an unused syscall to store the new FS_os_message_queue_ack entry */
		DCWrite32(aMsgAck2,     0xE51FF004);
		DCWrite32(aMsgAck2 + 4, ((u32) FS_os_message_queue_ack) | 1);

		/* Patch the jump */
		DCWrite32(aMsgAck1, 0xEA000000 | __geArmBranchDestOffset(aMsgAck1, aMsgAck2));
	}
#endif
#endif
}

void Patch_FfsModule(u32 version)
{               
	switch (version) {
	/** 12/24/08 13:48:17 **/
	case 0x49523DA1:        // IOS: 37v5662, 53v5662, 55v5662
		__Patch_FfsModule(0x20005F38, 0x20005F0A, 0x20006084);
		break;

	/** 12/23/08 17:26:21 **/
	case 0x49511F3D:        // IOS: 36v3607, 38v4123
		__Patch_FfsModule(0x200021D0, 0x2000219C, 0x200060BC);
		break;

	/** 11/24/08 15:36:10 **/
	case 0x492AC9EA:        // IOS: 56v5661, 57v5918, 58v6175, 60v6174, 61v5661, 70v6687, 80v6943
		__Patch_FfsModule(0x200061B8, 0x2000618A, 0x20006304);
		break;

	default:
		svc_write("FFSP: Error -> Can't patch FFS module (unknown version)\n");
		break;
	}
}

void __Patch_IopModule(u32 aSysOpen)
{
	/* Set addresses */
	addrSysOpen = aSysOpen + 12;

	/* Patch syscall open like that */
	/*      bx  pc                  */
	/*      ldr pc, =syscall_open   */
	DCWrite32(aSysOpen,     0x477846C0);
	DCWrite32(aSysOpen + 4, 0xE51FF004);
	DCWrite32(aSysOpen + 8, (u32)syscall_open);
}

void Patch_IopModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:29 **/
	/** 03/01/10 03:28:58 **/
	case 0x48776F75:        // IOS: 37v5662, 53v5662, 55v5662
		__Patch_IopModule(0xFFFF2E50);
		break;

	/** 03/01/10 03:13:17 **/
	case 0x4B8B30CD:        // IOS: 36v3607, 38v4123
		__Patch_IopModule(0xFFFF2E04);
		break;

	/** 11/24/08 15:39:12 **/
	/** 06/03/09 07:49:12 **/
	/** 03/03/10 10:43:18 **/
	case 0x492ACAA0:        // IOS: 60v6174, 70v6687 
	case 0x4B8E3D46:        // IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943 
		__Patch_IopModule(0xFFFF3020);
		break;

	default:
		svc_write("FFSP: Error -> Can't patch IOP module (unknown version)\n");
		break;
	}
}
