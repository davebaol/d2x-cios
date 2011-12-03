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

#include "tools.h"
#include "types.h"


void Swi_Memcpy(void *dst, void *src, s32 len)
{
	/* Wrong length */
	if (len <= 0)
		return;

	/* Call function */
	Swi_MLoad(2, (u32)dst, (u32)src, (u32)len);
}

void Swi_uMemcpy(void *dst, void *src, s32 len)
{
	/* Wrong length */
	if (len <= 0)
		return;

	/* Call function */
	Swi_MLoad(9, (u32)dst, (u32)src, (u32)len);
}

void Swi_LedOn(void)
{
	Swi_MLoad(128, 0, 0, 0);
}

void Swi_LedOff(void)
{
	Swi_MLoad(129, 0, 0, 0);
}

void Swi_LedBlink(void)
{
	Swi_MLoad(130, 0, 0, 0);
}