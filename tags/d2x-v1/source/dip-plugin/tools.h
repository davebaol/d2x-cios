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

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "types.h"

/* Macros */
#define BIT_SET(x, y)	(x |=  y)
#define BIT_DEL(x, y)	(x &= ~y)
#define BIT_CHK(x, y)	(x & y)


/* Direct syscalls */
void DCInvalidateRange(void* ptr, int size);
void DCFlushRange(void* ptr, int size);
void ICInvalidate(void);

/* MLoad syscalls */
s32 Swi_MLoad(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* ARM permissions */
u32  Perms_Read(void);
void Perms_Write(u32 flags);

/* MEM2 routines */
void MEM2_Prot(u32 flag);

/* Tools */
void *VirtToPhys(void *address);
void *PhysToVirt(void *address);

#endif

