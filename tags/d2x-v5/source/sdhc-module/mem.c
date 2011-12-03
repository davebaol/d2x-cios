/*   
	Custom IOS Module (SDHC)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 WiiGator.
	Copyright (C) 2009 Waninkoko.

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

#include "syscalls.h"

/* Heap */
static u32 heapspace[0x2000] ATTRIBUTE_ALIGN(32);

/* Variables */
static s32 hid = -1;


s32 Mem_Init(void)
{
	/* Heap already created */
	if (hid >= 0)
		return 0;

	/* Create heap */
	hid = os_heap_create(heapspace, sizeof(heapspace));

	return (hid >= 0) ? 0 : -1;
} 

void *Mem_Alloc(u32 size)
{
	/* Allocate memory */
	return os_heap_alloc_aligned(hid, size, 32);
}

void Mem_Free(void *ptr)
{
	/* Free memory */
	os_heap_free(hid, ptr);
}
