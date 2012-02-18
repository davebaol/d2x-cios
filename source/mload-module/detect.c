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
#include "types.h"

#define DETECT_ERROR	((s32)error_msg)

static char *error_msg = "Version not detected";

s32 Detect_DipModule(void)
{
	u32 dipAddr = *(vu32 *)0x20200040;

	/* Set DIP version */
	switch (dipAddr) {

	case 0x20207F40:		// IOS: 37v5662, 53v5662, 55v5662
		/* DIP: 07/11/08 14:34:26 */
		ios.dipVersion = 0x48776F72;
		break;

	case 0x20207C2C:		// IOS: 36v3607, 38v4123
		/* DIP: 07/24/08 20:08:44 */
		ios.dipVersion = 0x4888E14C;
		break;

	case 0x20207EA8:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 70v6687, 80v6943
		/* DIP: 06/03/09 07:49:09 */
		ios.dipVersion = 0x4A262AF5;
		break;

	case 0x20207DB8:		// IOS: 60v6174
		/* DIP: 11/24/08 15:39:09 */
		ios.dipVersion = 0x492ACA9D;
		break;

	default:
		/* Unknown version */
		return DETECT_ERROR;
	}

	return 0;
}

s32 Detect_EsModule(void)
{
	u32 esAddr = *(vu32 *)0x20100044;

	/* Set ES version */
	switch (esAddr) {
	case 0x201015A5:		// IOS: 70v6687
		/* ES: 06/03/09 07:46:02 */
		ios.esVersion = 0x4A262A3A;
		break;

	case 0x201014D5:		// IOS: 60v6174
		/* ES: 11/24/08 15:36:08 */
		ios.esVersion = 0x492AC9E8;
		break;

	case 0x201015E9:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
		/* ES: 03/03/10 10:40:14 */
		ios.esVersion = 0x4B8E90EE;
		break;

	case 0x2010142D:		// IOS: 37v5662, 53v5662, 55v5662		
		/* ES: 03/01/10 03:26:03 */
		ios.esVersion = 0x4B8B882B;
		break;

	case 0x2010139D:		// IOS: 36v3607, 38v4123
		/* ES: 03/01/10 03:18:58 */
		ios.esVersion = 0x4B8B8682;
		break;

	default:
		/* Unknown version */
		return DETECT_ERROR;
	}

	return 0;
}

s32 Detect_FfsModule(void)
{
	u32 ffsAddr = *(vu32 *)0x20000044;

	/* Set FFS version */
	switch (ffsAddr) {
	case 0x20005D89:		// IOS: 37v5662, 53v5662, 55v5662	
		/* FFS: 12/24/08 13:48:17 */
		ios.ffsVersion = 0x49523DA1;
		break;

	case 0x2000200D:		// IOS: 36v3607, 38v4123
		/* FFS: 12/23/08 17:26:21 */
		ios.ffsVersion = 0x49511F3D;
		break;

	case 0x20006009:		// IOS: 56v5661, 57v5918, 58v6175, 60v6174, 61v5661, 70v6687, 80v6943 	
		/* FFS: 11/24/08 15:36:10 */
		ios.ffsVersion = 0x492AC9EA;
		break;

	default:
		/* Unknown version */
		return DETECT_ERROR;
	}

	return 0;
}

s32 Detect_IopModule(void)
{
	u32 iopAddr = *(vu32 *)0xFFFF0028;

	/* Set IOP version */
	switch (iopAddr) {
	case 0xFFFF1D60:	//               IOS: 37v5662, 53v5662, 55v5662
	case 0xFFFF79B4:	// gecko patched IOS: 37v5662, 53v5662, 55v5662
		/* IOSP: 07/11/08 14:34:29 */
		ios.iopVersion  = 0x48776F75;
		ios.syscallBase = 0xFFFF91B0;

		break;

	case 0xFFFF1D10:	//               IOS: 36v3607, 38v4123
	case 0xFFFF7938:	// gecko patched IOS: 36v3607, 38v4123
		/* IOSP: 03/01/10 03:13:17 */
		ios.iopVersion  = 0x4B8B30CD;
		ios.syscallBase = 0xFFFF9100;

		break;

	case 0xFFFF1F20:	//               IOS: 60v6174, 70v6687, 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
	case 0xFFFF7B98:	// gecko patched IOS: 60v6174, 70v6687
	case 0xFFFF7BD0:	// gecko patched IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
		iopAddr = *(vu32 *)0xFFFF2418;

		switch (iopAddr) {
		case 0xFFFF9390:		// IOS: 60v6174, 70v6687
			/* IOSP: 11/24/08 15:39:12 */
			ios.iopVersion  = 0x492ACAA0;
			ios.syscallBase = 0xFFFF9390;

			break;

		case 0xFFFF93D0:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
			/* IOSP: 03/03/10 10:43:18 */
			ios.iopVersion  = 0x4B8E3D46;
			ios.syscallBase = 0xFFFF93D0;

			break;

		default:
			/* Unknown version */
			return DETECT_ERROR;
		}

		break;

	default:
		/* Unknown version */
		return DETECT_ERROR;
	}

	return 0;
}

