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

#ifndef _SWI_MLOAD_H_
#define _SWI_MLOAD_H_

#include "types.h"

/* Prototypes */
void Swi_Memcpy(void *dst, void *src, s32 len);
void Swi_uMemcpy(void *dst, void *src, s32 len);
s32  Swi_CallFunc(s32 (*func)(void *in, void *out), void *in, void *out);
u32  Swi_GetSyscallBase(void);
u32  Swi_GetIosInfo(void *buffer);

#endif
