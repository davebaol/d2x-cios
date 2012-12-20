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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "types.h"

/* Constants */
#define DEBUG_NONE	0
#define DEBUG_BUFFER	1
#define DEBUG_GECKO	2

#define NO_DEBUG_BUFFER

/* Prototypes */
s32 Debug_SetMode(u8 mode);
#ifndef NO_DEBUG_BUFFER
s32 Debug_GetBuffer(char *outbuf, u32 len);
#endif

#endif
