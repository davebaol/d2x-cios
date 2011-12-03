/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.

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

#include "swi.h"
#include "tools.h"
#include "types.h"

/* Macros */
#define Write8(addr, val)	\
	*(vu8 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u8));

#define Write16(addr, val)	\
	*(vu16 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u16));

#define Write32(addr, val)	\
	*(vu32 *)addr = val;	\
	DCFlushRange((void *)addr, sizeof(u32));


void Patch_DipModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:26 **/
	case 0x48776F72:
		/* Unencrypted read limit */
		Write32(0x20206640, 0x7ED40000);

		break;

	/** 07/24/08 20:08:44 **/
	case 0x4888E14C:
		/* Unencrypted read limit */
		Write32(0x2020636C, 0x7ED40000);

		break;

	/** 11/24/08 15:39:09 **/
	/** 06/03/09 07:49:09 **/
	case 0x492ACA9D:
	case 0x4A262AF5:
		/* Unencrypted read limit */
		Write32(0x202066E4, 0x7ED40000);

		break;
	}
}

void Patch_EsModule(u32 version)
{
	switch (version) {
	/** 06/03/09 03:45:06 **/
	case 0x4A25F1C2:
		/* Signature check */
		Write16(0x13A752E6, 0x2000);

		/* Identify check */
		Write16(0x20100D4A, 0x2803);
		Write16(0x20100DC2, 0x2803);

		/* Open content permissions */
		Write8(0x20104D7A, 0xE0);
		Write8(0x20104D9E, 0xE0);
		Write8(0x20104DC2, 0xE0);

		/* Read content permissions */
		Write16(0x20104EBC, 0x46C0);
		Write16(0x20104EC0, 0x46C0);
		Write8 (0x20104EC4, 0xE0);

		/* Close content permissions */
		Write16(0x20104F58, 0x46C0);
		Write16(0x20104F5C, 0x46C0);
		Write8 (0x20104F60, 0xE0);

		/* Set UID check */
		Write16(0x2010522A, 0x46C0);

		/* Title version check */
		Write8(0x201027AC, 0xE0);

		/* Title delete check */
		Write8(0x20107B22, 0xE0);

		break;

	/** 06/03/09 03:36:55 **/
	case 0x4A25EFD7:
		/* Signature check */
		Write16(0x13A750A6, 0x2000);

		/* Identify check */
		Write16(0x20100CC8, 0x2803);
		Write16(0x20100D40, 0x2803);

		/* Open content permissions */
		Write8(0x20104B68, 0xE0);
		Write8(0x20104B8C, 0xE0);

		/* Read content permissions */
		Write16(0x20104C84, 0x46C0);
		Write16(0x20104C88, 0x46C0);
		Write8 (0x20104C8C, 0xE0);

		/* Close content permissions */
		Write16(0x20104D20, 0x46C0);
		Write16(0x20104D24, 0x46C0);
		Write8 (0x20104D28, 0xE0);

		/* Set UID check */
		Write16(0x20104FF2, 0x46C0);

		/* Title version check */
		Write8(0x20102724, 0xE0);

		/* Title delete check */
		Write8(0x20107682, 0xE0);

		break;

	/** 06/03/09 07:46:02 **/
	case 0x4A262A3A:
		/* Signature check */
		Write16(0x13A75626, 0x2000);

		/* Identify check */
		Write16(0x20100E74, 0x2803);
		Write16(0x20100EEC, 0x2803);

		/* Open content permissions */
		Write8(0x20105290, 0xE0);
		Write8(0x201052D0, 0xE0);
		Write8(0x201052F4, 0xE0);

		/* Read content permissions */
		Write16(0x201053FC, 0x46C0);
		Write16(0x20105400, 0x46C0);
		Write8 (0x20105404, 0xE0);

		/* Close content permissions */
		Write16(0x20105498, 0x46C0);
		Write16(0x2010549C, 0x46C0);
		Write8 (0x201054A0, 0xE0);

		/* Set UID check */
		Write16(0x2010576A, 0x46C0);

		/* Title version check */
		Write8(0x20102C74, 0xE0);

		/* Title delete check */
		Write8(0x2010849A, 0xE0);

		/* Decrypt check */
		Write8(0x2010650C, 0xE0);

		break;

	/** 11/24/08 15:36:08 **/
	case 0x492AC9E8:
		/* Signature check */
		Write16(0x13A754FA, 0x2000);
		Write16(0x13A756A6, 0x2000);

		/* Identify check */
		Write16(0x20100DA4, 0x2803);
		Write16(0x20100E1C, 0x2803);

		/* Open content permissions */
		Write8(0x20104D60, 0xE0);
		Write8(0x20104DA0, 0xE0);
		Write8(0x20104DC4, 0xE0);

		/* Read content permissions */
		Write16(0x20104ECC, 0x46C0);
		Write16(0x20104ED0, 0x46C0);
		Write8 (0x20104ED4, 0xE0);

		/* Close content permissions */
		Write16(0x20104F68, 0x46C0);
		Write16(0x20104F6C, 0x46C0);
		Write8 (0x20104F70, 0xE0);

		/* Set UID check */
		Write16(0x2010523A, 0x46C0);

		/* Title version check */
		Write8(0x20102800, 0xE0);

		/* Title delete check */
		Write8(0x20107B32, 0xE0);

		/* Decrypt check */
		Write8(0x20105FD0, 0xE0);

		break;

	/** 03/03/10 10:40:14 **/
	case 0x4B8E90EE:
		/* Signature check */
		Write16(0x13A75626, 0x2000);

		/* Identify check */
		Write16(0x20100E74, 0x2803);
		Write16(0x20100EEC, 0x2803);

		/* Open content permissions */
		Write8(0x201052E4, 0xE0);
		Write8(0x20105324, 0xE0);
		Write8(0x20105348, 0xE0);

		/* Read content permissions */
		Write16(0x20105450, 0x46C0);
		Write16(0x20105454, 0x46C0);
		Write8 (0x20105458, 0xE0);

		/* Close content permissions */
		Write16(0x201054A0, 0x46C0);
		Write16(0x201054A4, 0x46C0);
		Write8 (0x201054A8, 0xE0);

		/* Set UID check */
		Write16(0x201057BE, 0x46C0);

		/* Title version check */
		Write8(0x20102CB8, 0xE0);

		/* Title delete check */
		Write8(0x20108562, 0xE0);

		break;

	/** 03/01/10 03:26:03 **/
	case 0x4B8B882B:
		/* Signature check */
		Write16(0x13A752E6, 0x2000);

		/* Identify check */
		Write16(0x20100D46, 0x2803);
		Write16(0x20100DBE, 0x2803);

		/* Open content permissions */
		Write8(0x20104DF6, 0xE0);
		Write8(0x20104E1A, 0xE0);
		Write8(0x20104E3E, 0xE0);

		/* Read content permissions */
		Write16(0x20104F38, 0x46C0);
		Write16(0x20104F3C, 0x46C0);
		Write8 (0x20104F40, 0xE0);

		/* Close content permissions */
		Write16(0x20104F88, 0x46C0);
		Write16(0x20104F8C, 0x46C0);
		Write8 (0x20104F90, 0xE0);

		/* Set UID check */
		Write16(0x201052A6, 0x46C0);

		/* Title version check */
		Write8(0x20102818, 0xE0);

		/* Title delete check */
		Write8(0x20107BAA, 0xE0);

		break;

	/** 03/01/10 03:18:58 **/
	case 0x4B8B8682:
		/* Signature check */
		Write16(0x13A75266, 0x2000);

		/* Identify check */
		Write16(0x20100CC4, 0x2803);
		Write16(0x20100D3C, 0x2803);

		/* Open content permissions */
		Write8(0x20104B20, 0xE0);
		Write8(0x20104B44, 0xE0);

		/* Read content permissions */
		Write16(0x20104C3C, 0x46C0);
		Write16(0x20104C40, 0x46C0);
		Write8 (0x20104C44, 0xE0);

		/* Close content permissions */
		Write16(0x20104C8C, 0x46C0);
		Write16(0x20104C90, 0x46C0);
		Write8 (0x20104C94, 0xE0);

		/* Set UID check */
		Write16(0x20104FAA, 0x46C0);

		/* Title version check */
		Write8(0x201026CC, 0xE0);

		/* Title delete check */
		Write8(0x20107642, 0xE0);

		break;
	}
}

void Patch_FfsModule(u32 version)
{
	switch (version) {
	/** 12/24/08 13:48:17 **/
	case 0x49523DA1:
		/* Permissions check */
		Write8(0x200012F2, 0xE0);

		break;

	/** 12/23/09 17:26:21 **/
	case 0x49511F3D:
		/* Permissions check */
		Write8(0x2000347E, 0xE0);

		break;

	/** 11/24/08 15:36:10 **/
	case 0x492AC9EA:
		/* Permissions check */
		Write8(0x20001306, 0xE0);

		break;
	}
}

void Patch_IopModule(u32 version)
{
	switch (version) {
	/** 07/11/08 14:34:29 **/
	/** 11/24/08 15:39:12 **/
	/** 12/23/08 17:28:32 **/
	/** 03/03/10 10:43:18 **/
	/** 03/01/10 03:28:58 **/
	/** 03/01/10 03:13:17 **/
	case 0x48776F75:
	case 0x492ACAA0:
	case 0x49511FC0:
	case 0x4B8E3D46:
	case 0x4B8B30CD:
		/* SWI handler */
		Write32(0xFFFF0028, (u32)SwiVector);

		break;
	}
}

