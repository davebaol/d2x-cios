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

#include "mem.h"
#include "syscalls.h"
#include "types.h"

/* Variables */
static s32 queuehandle = -1;
static s32 timerId     = -1;


void Timer_Init(void)
{
	void *queuespace = NULL;

	/* Queue already created */
	if (queuehandle >= 0)
		return;

	/* Create queue */
	queuespace  = Mem_Alloc(0x40);
	queuehandle = os_message_queue_create(queuespace, 16);

	/* Create timer */
	timerId = os_create_timer(0, 0, queuehandle, 0x666);

	/* Stop timer */
	os_stop_timer(timerId);
}

void Timer_Sleep(u32 time)
{
	u32 message;

	/* Restart timer */
	os_restart_timer(timerId, 0, time);

	while (1) {
		/* Wait to receive message */
		os_message_queue_receive(queuehandle, (void *)&message, 0);

		/* Message received */
		if (message == 0x666)
			break;
	}

	/* Stop timer */
	os_stop_timer(timerId);
}
