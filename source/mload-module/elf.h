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

#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

/* Prototypes */
s32 Elf_Load(void *data);
s32 Elf_Run(void);
s32 Elf_RunThread(void *start, void *arg, void *stack, u32 stacksize, u32 priority);
s32 Elf_StopThread(s32 tid);
s32 Elf_ContinueThread(s32 tid);

#endif
