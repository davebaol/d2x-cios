/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
	Copyright (C) 2009 Waninkoko.
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

#include <string.h>

#include "ehci_types.h"
#include "ehci.h"
#include "mem.h"
#include "syscalls.h"
#include "usb_os.h"
#include "usb.h"

/* Heap */
static u8  usb_heapspace[0xE000] ATTRIBUTE_ALIGN(32);
static s32 usb_heap = -1;


s32 USB_Mem_Init(void)
{
	/* Heap already created */
	if (usb_heap >= 0)
		return 0;

	/* Create heap */
	usb_heap = os_heap_create(usb_heapspace, sizeof(usb_heapspace));

	return (usb_heap < 0) ? -1 : 0;
}

static void USB_BUG(char *log_msg, int blink_freq)
{
	svc_write(log_msg);
	for(;;) {
		Swi_LedOn();
		ehci_msleep(blink_freq);
		Swi_LedOff();
		ehci_msleep(blink_freq);
	}
}

void *ehci_maligned(int size, int alignement, int crossing)
{
	/* Static variables */
	static u8 ehci_heapspace[0x5000] ATTRIBUTE_ALIGN(32);
	static u8 *aligned_mem  = 0;
	static u8 *aligned_base = 0;

	if (!aligned_mem) {
		 aligned_base = (u8 *)(((u32)ehci_heapspace + 4095) & ~4095);
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

	if (aligned_mem > aligned_base + 0x4000)
		USB_BUG("EHCI: ehci_maligned out of aligned memory!\n", 1000);

	/* Clear buffer */
	memset((void *)addr, 0, size);

	return (void *)addr;
}

void read_cache_data(char *in, int len)
{
	int n;
	char t;

	for (n = 0; n < len; n++)
		t = *in++;
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

void ehci_dma_unmap_to(dma_addr_t buf, size_t len)
{
	os_sync_before_read((void *)buf, len);
	read_cache_data((void *) buf, len);
}

void ehci_dma_unmap_from(dma_addr_t buf, size_t len)
{
	os_sync_before_read((void *)buf, len);
	read_cache_data((void *) buf, len);
}

void ehci_dma_unmap_bidir(dma_addr_t buf, size_t len)
{
	os_sync_before_read((void *)buf, len);
	read_cache_data((void *) buf, len);
}

void ehci_usleep(int time)
{
	static s32 ehci_timer_queuehandle = -1;

	s32 ehci_timer_id;
	u32 message;

	if (ehci_timer_queuehandle == -1) {
		/* Create ehci timer queue */
		void *queuespace = Mem_Alloc(0x80);
		ehci_timer_queuehandle = os_message_queue_create(queuespace, 32);
	}
	
	/* Create ehci timer */
	ehci_timer_id = os_create_timer(time, 1000*1000*10, ehci_timer_queuehandle, 0x0);

	/* Wait to receive message */
	os_message_queue_receive(ehci_timer_queuehandle,(void *) &message, 0);

	/* Stop and destroy ehci timer */
	os_stop_timer(ehci_timer_id);
	os_destroy_timer(ehci_timer_id);
}

void ehci_msleep(int msec)
{
	ehci_usleep(((u32) msec)*1000);
}

static void __ehci_delay(int t, int x)  //@todo not really sleeping..
{
	u32 tmr, temp, time_usec;

	tmr = get_timer();
	time_usec = t * x;

	while (1) {
		temp = get_timer() - tmr;
		if(((int) temp) < 0)
			tmr = get_timer();
		if(((int)temp) > time_usec)
			break;
	}
}

void ehci_udelay(int usec)
{
	__ehci_delay(usec, 2);
}

void ehci_mdelay(int msec)
{
	__ehci_delay(msec, 2048);
}

void *USB_Alloc(int size)
{
	void *ret;
	
	ret = os_heap_alloc_aligned(usb_heap, size, 32);
	if (!ret)
		USB_BUG("EHCI: USB_Alloc out of memory!\n", 200);

	return ret;
}

void USB_Free(void *ptr)
{
	return os_heap_free(usb_heap, ptr);
}

void *USB_GetSectorBuffer(u32 min_size)
{
	static void *buffer = NULL;
	static u32 size = 0;

	/* Check the current buffer size is not enough */
	if (min_size > size) {

		/* Set new buffer size */
		size = min_size;

		/* Release buffer (if any) */
		if (buffer) {
			USB_Free(buffer);    
			buffer = NULL;
		}    
	}

	/* Alloc buffer (if needed) */
	if (!buffer)
  		buffer = USB_Alloc(size);

	return buffer;
}

