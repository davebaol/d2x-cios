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

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "gecko.h"
#include "swi.h"
#include "types.h"

#ifndef NO_DEBUG_BUFFER
/* Constants */
#define LOG_SIZE	4095

/* Log buffer */
static char buffer[LOG_SIZE+1] ATTRIBUTE_ALIGN(32);


static s32 __Debug_Buffer(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	char *msg = (char *)arg1;
	u32   end, len, pos;

	/* Check command */
	if (arg0 != 4)
		return arg0;

	/* Message length */
	len = strlen(msg);
	pos = strlen(buffer);

	/* Message end */
	end = pos + len;

	/* No free space */
	if (end > LOG_SIZE) {
		u32 len2;

		/* Bytes to move */
		len2 = (end - LOG_SIZE);

		/* Move data */
		memmove(buffer, buffer + len2, pos - len2);

		/* Update position */
		pos -= len2;
	}

	/* Copy message */
	strcpy(buffer + pos, msg);

	return 0;
}
#endif

static s32 __Debug_Gecko(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
	static char gecko_inited = 0;

	/* Check command */
	if (arg0 != 4)
		return arg0;

	/* Init USB Gecko */
	if (!gecko_inited) {
		Gecko_Init();
		gecko_inited = 1;
	}

	/* Send message */
	Gecko_SendString((char *)arg1);

	return 0;
}


s32 Debug_SetMode(u8 mode)
{
	SwiFunc *SwiText = &SwiTable[0xAB];

	/* Check mode */
	switch (mode) {
	case DEBUG_NONE:
		/* Unset text handler */
		*SwiText = NULL;

		break;

#ifndef NO_DEBUG_BUFFER
	case DEBUG_BUFFER:
		/* Clear buffer */
		memset(buffer, 0, sizeof(buffer));

		/* Set buffer text handler */
		*SwiText = __Debug_Buffer;

		break;
#endif

	case DEBUG_GECKO:
		/* Set usb gecko text handler */
		*SwiText = __Debug_Gecko;

		break;

	default:
		return -1;
	}

	return 0;
}

#ifndef NO_DEBUG_BUFFER
s32 Debug_GetBuffer(char *outbuf, u32 size)
{
	u32 len;

	/* Buffer length */
	len = strlen(buffer);

	/* Check length */
	if (len >= size)
		len = (size - 1);

	/* Copy buffer */
	memcpy(outbuf, buffer, len);
	outbuf[len] = 0;

	return len;
}
#endif

