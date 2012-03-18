/*
 * EHCI plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
 * Copyright (C) 2011 davebaol.
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

#ifndef _USB_OS_H_
#define _USB_OS_H_

#include "types.h"

s32 USB_Mem_Init(void);

void *ehci_maligned(int size, int alignement, int crossing);
void read_cache_data(char *in, int len);
dma_addr_t ehci_virt_to_dma(void *a);
dma_addr_t ehci_dma_map_to(void *buf, size_t len);
dma_addr_t ehci_dma_map_from(void *buf, size_t len);
dma_addr_t ehci_dma_map_bidir(void *buf, size_t len);
void ehci_dma_unmap_to(dma_addr_t buf, size_t len);
void ehci_dma_unmap_from(dma_addr_t buf, size_t len);
void ehci_dma_unmap_bidir(dma_addr_t buf, size_t len);

#define get_timer()  (*((volatile u32*)0x0D800010))

void ehci_usleep(int time);
void ehci_msleep(int msec);
void ehci_udelay(int usec);
void ehci_mdelay(int msec);

void *USB_Alloc(int size);
void USB_Free(void *ptr);

void *USB_GetSectorBuffer(u32 min_size);

#endif