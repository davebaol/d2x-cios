/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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
 

/* Addresses */
u32 addrIoctlv      = 0;
u32 addrLaunchTitle = 0;
u32 addrPrintf      = 0;
u32 addrSnprintf    = 0;


void Patch_EsModule(u32 version)
{
	switch (version) {
	/** 06/03/09 07:46:02 **/
	case 0x4A262A3A:
		/* Patch IOCTLV handler */
		Write32(0x201000CC, 0x4B004718);
		Write32(0x201000D0, (u32)ES_EmulateCmd);

		/* Set addresses */
		addrIoctlv      = 0x201000D4 + 1;
		addrLaunchTitle = 0x20104CA0 + 1;
		addrPrintf      = 0x2010A728 + 1;
		addrSnprintf    = 0x2010ABD0 + 1;

		break;

	/** 03/03/10 10:40:14 **/
	case 0x4B8E90EE:
		/* Patch IOCTLV handler */
		Write32(0x201000CC, 0x4B004718);
		Write32(0x201000D0, (u32)ES_EmulateCmd);

		/* Set addresses */
		addrIoctlv      = 0x201000D4 + 1;
		addrLaunchTitle = 0x20104CF4 + 1;
		addrPrintf      = 0x2010A7F0 + 1;
		addrSnprintf    = 0x2010AC98 + 1;

		break;

	/** 03/01/10 03:28:58 **/
	case 0x4B8B88DA:
		/* Patch IOCTLV handler */
		Write32(0x201000CC, 0x4B004718);
		Write32(0x201000D0, (u32)ES_EmulateCmd);

		/* Set addresses */
		addrIoctlv      = 0x201000D4 + 1;
		addrLaunchTitle = 0x20104818 + 1;
		addrPrintf      = 0x201099B8 + 1;
		addrSnprintf    = 0x20109E60 + 1;

		break;

	/** 03/01/10 03:18:58 **/
	case 0x4B8B8682:
		/* Patch IOCTLV handler */
		Write32(0x201000CC, 0x4B004718);
		Write32(0x201000D0, (u32)ES_EmulateCmd);

		/* Set addresses */
		addrIoctlv      = 0x201000D4 + 1;
		addrLaunchTitle = 0x20104544 + 1;
		addrPrintf      = 0x2010933C + 1;
		addrSnprintf    = 0x201097E4 + 1;

		break;
	}
}
