/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
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

#include <string.h>

#include "ehci_types.h"
#include "ehci.h"
#include "mem.h"
#include "syscalls.h"
#include "timer.h"
#include "usb.h"

/* Heap */
static u32 heapspace[0x4000] ATTRIBUTE_ALIGN(32);

/* Variables */
static u8 *aligned_mem  = 0;
static u8 *aligned_base = 0;


void *ehci_maligned(int size, int alignement, int crossing)
{
	if (!aligned_mem) {
		 aligned_base = (u8 *)(((u32)heapspace + 4095) & ~4095);
		 aligned_mem  = aligned_base;
	}

	u32 addr = (u32)aligned_mem;

	alignement--;

	/* Calculate aligned address */
	addr +=  alignement;
	addr &= ~alignement;

	if (((addr + size - 1) & ~(crossing - 1)) != (addr & ~(crossing - 1)))
		addr = (addr + size - 1) & ~(crossing - 1);

	aligned_mem = (void *)(addr + size);

	/* Clear buffer */
	memset((void *)addr, 0, size);

	return (void *)addr;
}

dma_addr_t ehci_virt_to_dma(void *a)
{
	return (dma_addr_t)a;
}

dma_addr_t ehci_dma_map_to(void *buf, size_t len)
{
	os_sync_after_write(buf, len);
	return (dma_addr_t)buf;
}

dma_addr_t ehci_dma_map_from(void *buf, size_t len)
{
	os_sync_after_write(buf, len);
	return (dma_addr_t)buf;
}

dma_addr_t ehci_dma_map_bidir(void *buf, size_t len)
{
	os_sync_after_write(buf, len);
	return (dma_addr_t)buf;
}

void ehci_dma_unmap_to(dma_addr_t buf,size_t len)
{
	os_sync_before_read((void *)buf, len);
}

void ehci_dma_unmap_from(dma_addr_t buf, size_t len)
{
	os_sync_before_read((void *)buf, len);
}

void ehci_dma_unmap_bidir(dma_addr_t buf, size_t len)
{
	os_sync_before_read((void *)buf, len);
}

void ehci_usleep(int time)
{
	usleep(time);
}

void ehci_msleep(int time)
{
	msleep(time);
}


void *USB_Alloc(int size)
{
	return Mem_Alloc(size);
}

void USB_Free(void *ptr)
{
	return Mem_Free(ptr);
}
