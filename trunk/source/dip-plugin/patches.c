/*
 * DIP plugin for Custom IOS.
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

#include "plugin.h"
#include "syscalls.h"
#include "tools.h"


/* Addresses */
u32 addr_handleInitDrive = 0;
u32 addr_handleIoctl     = 0;
u32 addr_handleCmd       = 0;
u8 *dip_readctrl         = 0;

/* Function pointers */
s32   (*DI_Printf)(const char * fmt, ...) = 0;
s32   (*DI_ReadHash)(void)                = 0; 
void *(*DI_Alloc)(u32 size, u32 align)    = 0;
void  (*DI_Free)(void *ptr)               = 0;


void __Patch_DipModule(u32 aInit, u32 aIoctl, u32 aCmd, u32 aReadHash,  
		u32 aAlloc, u32 aFree, u32 aPrintf, u32 aReadCtrl)
{
	/* Patch DVD driver init stage 2 */
	DCWrite32(aInit    , 0x4B004718);
	DCWrite32(aInit + 4, (u32)DI_EmulateInitDrive);

	/* Patch IOCTL handler */
	DCWrite32(aIoctl,     0x4B004718);
	DCWrite32(aIoctl + 4, (u32)DI_EmulateIoctl);
		
	/* Patch command handler */
	DCWrite32(aCmd,     0x4B004718);
	DCWrite32(aCmd + 4, (u32)DI_EmulateCmd);

	/* Set addresses */
	addr_handleInitDrive = (aInit  +  8) + 1;
	addr_handleIoctl     = (aIoctl + 12) + 1;
	addr_handleCmd       = (aCmd   + 12) + 1;
	dip_readctrl         = (u8 *) aReadCtrl;

	/* Set function pointers */
	DI_ReadHash = (void *)aReadHash + 1;
	DI_Alloc    = (void *)aAlloc    + 1;
	DI_Free     = (void *)aFree     + 1;
	DI_Printf   = (void *)aPrintf   + 1;
}

void Patch_DipModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:26 **/
	case 0x48776F72:        // IOS: 37v5662, 53v5662, 55v5662
		__Patch_DipModule(0x20200074, 0x20200400, 0x20200EF8, 0x20202A70, 
				0x2020096C, 0x2020093C, 0x2020387C, 0x2022DD60);
		break;

	/** 07/24/08 20:08:44 **/
	case 0x4888E14C:        // IOS: 36v3607, 38v4123
		__Patch_DipModule(0x20200068, 0x202003B8, 0x20200D2C, 0x20202874, 
				0x202008C4, 0x20200898, 0x2020365C, 0x2022CDAC);
		break;

	/** 11/24/08 15:39:09 **/
	case 0x492ACA9D:        // IOS: 60v6174
	/** 06/03/09 07:49:09 **/
	case 0x4A262AF5:        // IOS: 56v5661, 57v5918, 58v6175, 61v5661, 70v6687, 80v6943
		__Patch_DipModule(0x20200074, 0x20200400, 0x20200EF8, 0x20202944, 
				0x2020096C, 0x2020093C, 0x20203750, 0x2022CD60);
		break;

	default:
		svc_write("DIPP: Error -> Can't patch DI module (unknown version)\n");

		break;
	}
}
