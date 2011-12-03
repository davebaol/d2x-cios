/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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

s32 Swi_CallFunc(s32 (*func)(void *in, void *out), void *in, void *out)
{
	/* Call function */
	return Swi_MLoad(16, (u32)func, (u32)in, (u32)out);
}

u32 Swi_GetSyscallBase(void)
{
	return Swi_MLoad(17, 0, 0, 0);
}

u32 Swi_GetIosInfo(void *buffer)
{
	return Swi_MLoad(18, (u32)buffer, 0, 0);
}
