/*
 * EHCI plugin for Custom IOS.
 *
 * Copyright (C) 2009 Hermes.
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

#include "ehci_irq.h"
#include "hollywood.h"
#include "irq.h"
#include "swi_mload.h"
#include "types.h"

#define ehci_readl(a) (*((vu32*)(a)))
#define ehci_writel(v,a) do {*((vu32*)(a)) = (v);} while (0)

#define DEV_EHCI 4

#define enable_EHCI_IRQ()   (*((vu32 *) 0x0d0400cc) |= 1<<15)
#define disable_EHCI_IRQ()  (*((vu32 *) 0x0d0400cc) &= ~(1<<15))


/* Variables */
static s32 queue_handle   = -1;
static s32 timer_id       = -1;
static s32 remote_message = 0;

/* Callback functions */
static s32  (*working_callback)(u32 flags) = NULL;
static void (*passive_callback)(u32 flags) = NULL;


s32 ehci_irq_init(void)
{
	s32 ret;
	void *queuespace;

	/* Disable EHCI IRQ */
	disable_EHCI_IRQ();

	/* Allocate queue space */
	queuespace = USB_Alloc(4*32);
	if (!queuespace)
		return -1;

	/* Create message queue */
	queue_handle = os_message_queue_create(queuespace, 32);
	if (queue_handle < 0)
		return -1;

	/* Unregister old event handler */
	os_unregister_event_handler(DEV_EHCI);

	/* Register interrupt event handler */
	ret = os_register_event_handler(DEV_EHCI, queue_handle, 0);
	if (ret < 0)
		return -1;

	/* Re-enable EHCI IRQ */
	enable_EHCI_IRQ();
	os_software_IRQ(DEV_EHCI);

	return 0;
}

void ehci_irq_working_callback_stage1( s32 (*callback)(u32 flags), u32 timeout)
{

	/* Create timer */
	timer_id = os_create_timer(timeout, 1000*1000*100, queue_handle, 1);

	/* Clear interrupt flag */
	Swi_SetRegister(HW_ARMIRQFLAG,(1<<DEV_EHCI));

	/* Unmask interrupt flag */
	Swi_ClearRegister(HW_ARMIRQMASK,(1<<DEV_EHCI));

	/* Set callback */
	working_callback = callback;
	
	ehci_writel(INTR_MASK, &ehci->regs->intr_enable);

	/* Enable and mask interrupt flag */
	os_software_IRQ(DEV_EHCI);
	
}

s32 ehci_irq_working_callback_stage2(void)
{
	static s32 message = 0;

	message = -ETIMEDOUT;

	/* Wait for interrupt or timeout */
	os_message_queue_receive(queue_handle, (void*)&message, 0);

	/* Disable interrupts flags */
	ehci_writel(0, &ehci->regs->intr_enable);

	/* Disable callback */
	working_callback = NULL;

	/* Stop and destroy the timer */
	os_stop_timer(timer_id);
	os_destroy_timer(timer_id);
	timer_id = -1;
	
	/* Set response message */
	message = (message) ? -ETIMEDOUT : remote_message;
	
	/* Enable and mask interrupt flag */
	os_software_IRQ(DEV_EHCI);
 
	return message;
}

void ehci_irq_passive_callback( void (*callback)(u32 flags))
{
	/* Set passive callback */
	passive_callback = callback;

	/* Disable callback */
	working_callback = NULL;
}

void ehci_irq_handler(void)
{
	u32 flags;

	s32 message = 1;

	/* Disable EHCI interrupt */
	*((vu32 *)HW_ARMIRQMASK ) &= ~(1<<DEV_EHCI);

	flags = ehci_readl(&ehci->regs->status);
	
	if (working_callback) {

		/* Run working callback */
		message = working_callback(flags);
		
		if (message <= 0) {

			/* Disable callback */
			working_callback = NULL;
		
			/* Set remote message */
			remote_message = message;

			/* Send message */
			irq_send_device_message(DEV_EHCI);
		}
	}
	else if (passive_callback) {

		/* Run passive callback */
		passive_callback(flags);

	}

	ehci_writel(flags & INTR_MASK, &ehci->regs->status);

	if (message > 0) {
		*((vu32 *)HW_ARMIRQMASK) |= 1<<DEV_EHCI;
		*((vu32 *)HW_ARMIRQFLAG) |= 1<<DEV_EHCI;
	}
}
