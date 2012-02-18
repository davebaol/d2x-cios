/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.
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

#include "ios.h"
#include "swi.h"
#include "tools.h"
#include "types.h"


typedef struct {
	u32 signatureCheck1;
	u32 signatureCheck2;
	u32 identifyCheck1;
	u32 identifyCheck2;
	u32 openContentPerm1;
	u32 openContentPerm2;
	u32 openContentPerm3;
	u32 readContentPerm;
	u32 closeContentPerm;
	u32 setUidCheck;
	u32 titleVersionChek;
	u32 titleDeleteChek;
	u32 decryptChek;
} esAddrInfo;

void __Patch_EsModule(esAddrInfo *aInfo)
{
	/* Signature check */
	DCWrite16(aInfo->signatureCheck1, 0x2000);
	DCWrite16(aInfo->signatureCheck2, 0x2000);

	/* Identify check */
	DCWrite16(aInfo->identifyCheck1, 0x2803);
	DCWrite16(aInfo->identifyCheck2, 0x2803);

	/* Open content permissions */
	DCWrite8(aInfo->openContentPerm1, 0xE0);
	DCWrite8(aInfo->openContentPerm2, 0xE0);
	if (aInfo->openContentPerm3)	// Missing in base 36 and 38
		DCWrite8(aInfo->openContentPerm3, 0xE0);

	/* Read content permissions */
	DCWrite16(aInfo->readContentPerm,     0x46C0);
	DCWrite16(aInfo->readContentPerm + 4, 0x46C0);
	DCWrite8 (aInfo->readContentPerm + 8, 0xE0);

	/* Close content permissions */
	DCWrite16(aInfo->closeContentPerm    , 0x46C0);
	DCWrite16(aInfo->closeContentPerm + 4, 0x46C0);
	DCWrite8 (aInfo->closeContentPerm + 8, 0xE0);

	/* Set UID check */
	DCWrite16(aInfo->setUidCheck, 0x46C0);

	/* Title version check */
	DCWrite8(aInfo->titleVersionChek, 0xE0);

	/* Title delete check */
	DCWrite8(aInfo->titleDeleteChek, 0xE0);

	/* Decrypt check */
	if (aInfo->decryptChek)		// Missing in base 36 and 38
		DCWrite8(aInfo->decryptChek, 0xE0);
}

s32 Patch_DipModule(void)
{
	switch (ios.dipVersion) {
	/** 07/11/08 14:34:26 **/
	case 0x48776F72:      // IOS: 37v5662, 53v5662, 55v5662
		/* Unencrypted read limit */
		DCWrite32(0x20206640, 0x7ED40000);

		break;

	/** 07/24/08 20:08:44 **/
	case 0x4888E14C:      // IOS: 36v3607, 38v4123
		/* Unencrypted read limit */
		DCWrite32(0x2020636C, 0x7ED40000);

		break;

	/** 11/24/08 15:39:09 **/
	case 0x492ACA9D:       // IOS: 60v6174
	/** 06/03/09 07:49:09 **/
	case 0x4A262AF5:       // IOS: 56v5661, 57v5918, 58v6175, 61v5661, 70v6687, 80v6943
		/* Unencrypted read limit */
		DCWrite32(0x202066E4, 0x7ED40000);

		break;

	default:
		/* Unknown module version */
		return IOS_ERROR_DIP;
	}

	return 0;
}

s32 Patch_EsModule(void)
{
	switch (ios.esVersion) {
	/** 06/03/09 07:46:02 **/
	case 0x4A262A3A: {	// IOS: 70v6687
		static esAddrInfo addrInfo = {
			0x13A7547A,	// signatureCheck1 (added in d2x v8 r42)
			0x13A75626,	// signatureCheck2
			0x20100E74,	// identifyCheck1
			0x20100EEC,	// identifyCheck2
			0x20105290,	// openContentPerm1
			0x201052D0,	// openContentPerm2
			0x201052F4,	// openContentPerm3
			0x201053FC,	// readContentPerm
			0x20105498,	// closeContentPerm
			0x2010576A,	// setUidCheck
			0x20102C74,	// titleVersionChek
			0x2010849A,	// titleDeleteChek
			0x2010650C	// decryptChek
		};

		__Patch_EsModule(&addrInfo);

		break;
	}

	/** 11/24/08 15:36:08 **/
	case 0x492AC9E8: {	// IOS: 60v6174
		static esAddrInfo addrInfo = {
			0x13A754FA,	// signatureCheck1
			0x13A756A6,	// signatureCheck2
			0x20100DA4,	// identifyCheck1
			0x20100E1C,	// identifyCheck2
			0x20104D60,	// openContentPerm1
			0x20104DA0,	// openContentPerm2
			0x20104DC4,	// openContentPerm3
			0x20104ECC,	// readContentPerm
			0x20104F68,	// closeContentPerm
			0x2010523A,	// setUidCheck
			0x20102800,	// titleVersionChek
			0x20107B32,	// titleDeleteChek
			0x20105FD0	// decryptChek
		};

		__Patch_EsModule(&addrInfo);

		break;
	}

	/** 03/03/10 10:40:14 **/
	case 0x4B8E90EE: {	// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
		static esAddrInfo addrInfo = {
			0x13A7547A,	// signatureCheck1  (added in d2x v8 r42)
			0x13A75626,	// signatureCheck2
			0x20100E74,	// identifyCheck1
			0x20100EEC,	// identifyCheck2
			0x201052E4,	// openContentPerm1
			0x20105324,	// openContentPerm2
			0x20105348,	// openContentPerm3
			0x20105450,	// readContentPerm
			0x201054A0,	// closeContentPerm
			0x201057BE,	// setUidCheck
			0x20102CB8,	// titleVersionChek
			0x20108562,	// titleDeleteChek
			0x20106560	// decryptChek      (added in d2x v8 r42)
		};

		__Patch_EsModule(&addrInfo);

		break;
	}

	/** 03/01/10 03:26:03 **/
	case 0x4B8B882B: {	// IOS: 37v5662, 53v5662, 55v5662
		static esAddrInfo addrInfo = {
			0x13A750DE,	// signatureCheck1  (added in d2x v8 r42)
			0x13A752E6,	// signatureCheck2
			0x20100D46,	// identifyCheck1
			0x20100DBE,	// identifyCheck2
			0x20104DF6,	// openContentPerm1
			0x20104E1A,	// openContentPerm2
			0x20104E3E,	// openContentPerm3
			0x20104F38,	// readContentPerm
			0x20104F88,	// closeContentPerm
			0x201052A6,	// setUidCheck
			0x20102818,	// titleVersionChek
			0x20107BAA,	// titleDeleteChek
			0x20106048	// decryptChek      (added in d2x v8 r42)
		};

		__Patch_EsModule(&addrInfo);

		break;
	}

	/** 03/01/10 03:18:58 **/
	case 0x4B8B8682: {	// IOS: 36v3607, 38v4123
		static esAddrInfo addrInfo = {
			0x13A750DE,	// signatureCheck1  (added in d2x v8 r42)
			0x13A75266,	// signatureCheck2
			0x20100CC4,	// identifyCheck1
			0x20100D3C,	// identifyCheck2
			0x20104B20,	// openContentPerm1
			0x20104B44,	// openContentPerm2
			0x0,		// openContentPerm3
			0x20104C3C,	// readContentPerm
			0x20104C8C,	// closeContentPerm
			0x20104FAA,	// setUidCheck
			0x201026CC,	// titleVersionChek
			0x20107642,	// titleDeleteChek
			0x0		// decryptChek
		};

		__Patch_EsModule(&addrInfo);

		break;
	}

	default:
		/* Unknown module version */
		return IOS_ERROR_ES;
	}

	return 0;
}

s32 Patch_FfsModule(void)
{
	switch (ios.ffsVersion) {
	/** 12/24/08 13:48:17 **/
	case 0x49523DA1:
		/* Permissions check */
		DCWrite8(0x200012F2, 0xE0);

		break;

	/** 12/23/09 17:26:21 **/
	case 0x49511F3D:
		/* Permissions check */
		DCWrite8(0x2000347E, 0xE0);

		break;

	/** 11/24/08 15:36:10 **/
	case 0x492AC9EA:
		/* Permissions check */
		DCWrite8(0x20001306, 0xE0);

		break;

	default:
		/* Unknown module version */
		return IOS_ERROR_FFS;
	}

	return 0;
}

s32 Patch_IopModule(void)
{
	switch (ios.iopVersion) {
	/** 07/11/08 14:34:29 **/
	/** 11/24/08 15:39:12 **/
	/** 12/23/08 17:28:32 **/
	/** 03/03/10 10:43:18 **/
	/** 03/01/10 03:28:58 **/
	/** 03/01/10 03:13:17 **/
	case 0x48776F75:        // IOS: 37v5662, 53v5662, 55v5662
	case 0x492ACAA0:        // IOS: 60v6174, 70v6687
	case 0x49511FC0:        // IOS: ???
	case 0x4B8E3D46:        // IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943 
	case 0x4B8B30CD:        // IOS: 36v3607, 38v4123
		/* SWI handler */
		DCWrite32(0xFFFF0028, (u32)SwiVector);

		break;

	default:
		/* Unknown module version */
		return IOS_ERROR_IOP;
	}

	return 0;
}

