/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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
u32 addrIoctlv = 0;

/* Function pointers */
s32 (*ES_printf)(const char * fmt, ...)                          = 0;
s32 (*ES_snprintf)(char *str, u32 size, const char *fmt, ...)    = 0;
s32 (*ES_LaunchTitle)(u32 tidh, u32 tidl, void *view, u32 reset) = 0;


void __Patch_EsModule(u32 aIoctlv, u32 aLaunchTitle, u32 aPrintf, u32 aSnprintf)
{
	/* Patch IOCTLV handler */
	DCWrite32(aIoctlv    , 0x4B004718);
	DCWrite32(aIoctlv + 4, (u32)ES_EmulateCmd);

	/* Set addresses */
	addrIoctlv      = aIoctlv + 8 + 1;

	/* Set function pointers */
	ES_LaunchTitle = (void *)aLaunchTitle + 1;
	ES_printf      = (void *)aPrintf      + 1;
	ES_snprintf    = (void *)aSnprintf    + 1;
}

void Patch_EsModule(u32 version)
{
	switch (version) {
	/** 06/03/09 07:46:02 **/
	case 0x4A262A3A:		// IOS: 70v6687
		__Patch_EsModule(0x201000CC, 0x20104CA0, 0x2010A728, 0x2010ABD0);

		break;

	// FIX d2x v7 beta1
	// Added missing base identification and patch
	/** 11/24/08 15:36:08 **/
	case 0x492AC9E8:		// IOS: 60v6174
		__Patch_EsModule(0x201000CC, 0x20104770, 0x20109A44, 0x20109EEC);

		break;

	/** 03/03/10 10:40:14 **/
	case 0x4B8E90EE:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
		__Patch_EsModule(0x201000CC, 0x20104CF4, 0x2010A7F0, 0x2010AC98);

		break;

	// FIX d2x v5 beta1
	// Fixed wrong base identification
	/** 03/01/10 03:26:03 **/
	case 0x4B8B882B:		// IOS: 37v5662, 53v5662, 55v5662
		__Patch_EsModule(0x201000CC, 0x20104818, 0x201099B8, 0x20109E60);

		break;

	/** 03/01/10 03:18:58 **/
	case 0x4B8B8682:		// IOS: 36v3607, 38v4123
		__Patch_EsModule(0x201000CC, 0x20104544, 0x2010933C, 0x201097E4);

		break;

	default:
		svc_write("ESP: Error -> Can't patch ES module (unknown version)\n");

		break;
	}
}
