/*
 * EHCI plugin for Custom IOS.
 *
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

#include "ios.h"
#include "irq.h"
#include "syscalls.h"
#include "tools.h"
#include "types.h"


typedef struct {
	u32 irq4check;
	u32 reentry;
	u32 send;
} iopAddrInfo;

/* Addresses */
u32 addrIrqReentry = 0;

/* Function pointers */
void (*irq_send_device_message)(u32 device) = NULL;


static void __Patch_IopModule(iopAddrInfo *aInfo)
{
	/* Set addresses */
	addrIrqReentry = aInfo->reentry;

	/* Set function pointers */
	irq_send_device_message = (void *)aInfo->send;

	/* Skip process id check in os_software_IRQ(4) */
	DCWrite32(aInfo->irq4check, (*((u32 *)aInfo->irq4check)) + 10);

	/* Patch interrupt vector with ldr pc, =irq_vector */
	DCWrite32(aInfo->reentry - 8, 0xE51FF004);
	DCWrite32(aInfo->reentry - 4, (u32) irq_vector);
}

s32 Patch_IopModule(void)
{
	switch (ios.iopVersion) {
	/** 07/11/08 14:34:29 **/
	case 0x48776F75: {       // IOS: 37v5662, 53v5662, 55v5662
		static iopAddrInfo aInfo = {
			0xFFFF8DE8,	// irq4check
			0xFFFF1F68,	// reentry
			0xFFFF1E34	// send
		};
		__Patch_IopModule(&aInfo);
		break;
	}

	/** 03/01/10 03:13:17 **/
	case 0x4B8B30CD: {       // IOS: 36v3607, 38v4123
		static iopAddrInfo aInfo = {
			0xFFFF8D34,	// irq4check
			0xFFFF1F18,	// reentry
			0xFFFF1DE4	// send
		};
		__Patch_IopModule(&aInfo);
		break;
	}

	/** 11/24/08 15:39:12 **/
	case 0x492ACAA0: {       // IOS: 60v6174, 70v6687 
		static iopAddrInfo aInfo = {
			0xFFFF8FCC,	// irq4check
			0xFFFF2128,	// reentry
			0xFFFF1FF4	// send
		};
		__Patch_IopModule(&aInfo);
		break;
	}

	/** 03/03/10 10:43:18 **/
	case 0x4B8E3D46: {       // IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943 
		static iopAddrInfo aInfo = {
			0xFFFF9004,	// irq4check
			0xFFFF2128,	// reentry
			0xFFFF1FF4	// send
		};
		__Patch_IopModule(&aInfo);
		break;
	}

	/** 04/02/12 14:03:56 **/
	case 0x4F79B1CC: {       // vIOS: 56v5918, 57v6175, 58v6432
		static iopAddrInfo aInfo = {
			0xFFFF8E8C,	// irq4check
			0xFFFF2128,	// reentry
			0xFFFF1FF4	// send
		};
		__Patch_IopModule(&aInfo);
		break;
	}

	default:
		/* Unknown version */
		return IOS_ERROR_IOP;
	}

	return 0;
}
