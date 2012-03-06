/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 Nuke.
	Copyright (C) 2009 marcan.
	Copyright (C) 2009 dhewg.
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

#ifndef __GECKO_H__
#define __GECKO_H__

#include "types.h"

void Gecko_Init(void);
u8   Gecko_EnableConsole(const u8 enable);
u32  Gecko_SendString(const char *string);

#endif

