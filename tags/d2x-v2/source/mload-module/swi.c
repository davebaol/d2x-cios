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

#include <string.h>

#include "gpio.h"
#include "module.h"
#include "swi.h"
#include "tools.h"
#include "types.h"

/* SWI table */
SwiFunc SwiTable[256] = { NULL };

/* SWI address */
u8 *SwiAddr = NULL;


s32 Swi_Handler(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	u8 cmd;

	/* Check alignment */
	SwiAddr -= (SwiAddr[-4] == 0xDF) ? 3 : 1;

	/* Get command */
	cmd = SwiAddr[0];

	/* Check command */
	if(SwiAddr[0] != 0xcc) {
		if (SwiTable[cmd])
			return SwiTable[cmd](arg0, arg1, arg2, arg3);
		else
			return arg0;
	}

	/* Check argument */
	switch (arg0) {
	/** Add SWI handler **/
	case 0: {
		SwiTable[arg1]= (void *)arg2;
		break;
	}
	
	/** Memcpy (cached to cached) **/
	case 2: {
		void *src = (void *)arg2;
		void *dst = (void *)arg1;
		u32   len = arg3;

		u32 perms;

		/* Apply permissions */
		perms = Perms_Read();
		Perms_Write(0xFFFFFFFF);

		/* Copy data */
		memcpy(dst, src, len);
		DCFlushRange(dst, len);

		/* Restore permissions */
		Perms_Write(perms);

		break;
	}
	
	/** Get register **/
	case 3:
		return *(vu32 *)arg1;

	/** Set register **/
	case 4:
		*(vu32 *)arg1 = arg2;
		break;

	/** Set register **/
	case 5:
		*(vu32 *)arg1 |= arg2;
		break;

	/** Clear register **/
	case 6:
		*(vu32 *)arg1 &= ~arg2;
		break;

	/** Memcpy (uncached to cached) **/
	case 9: {
		void *src = (void *)arg2;
		void *dst = (void *)arg1;
		u32   len = arg3;

		u32 perms;

		/* Apply permissions */
		perms = Perms_Read();
		Perms_Write(0xFFFFFFFF);

		/* Copy data */
		DCInvalidateRange(src, len);
		memcpy(dst, src, len);
		DCFlushRange(dst, len);

		/* Restore permissions */
		Perms_Write(perms);

		break;
	}

	/** Call function **/
	case 16: {
		s32 (*Function)(void *in, void *out);

		/* Set function */
		Function = (void *)arg1;

		/* Call function */
		return Function((void *)arg2, (void *)arg3);
	}

	/** Get syscall base **/
	case 17:
		return ios.syscall;

	/** Get IOS info **/
	case 18: {
		iosInfo *buffer = (iosInfo *)arg1;

		/* Copy IOS info */
		memcpy(buffer, &ios, sizeof(ios));
		DCFlushRange(buffer, sizeof(ios));

		break;
	}

	/** Get MLOAD version **/
	case 19:
		return (MLOAD_VER << 4) | MLOAD_SUBVER;

	/** Led on **/
	case 128:
		/* Set GPIO bit */
		GPIO_Write(5, 1);
		break;
	
	/** Led off **/
	case 129:
		/* Clear GPIO bit */
		GPIO_Write(5, 0);
		break;

	/** Led blink **/
	case 130: {
		u8 bit = GPIO_Read(5);

		/* Toggle GPIO bit */
		GPIO_Write(5, !bit);

		break;
	}

	default:
		break;
	}

	return 0;
}
