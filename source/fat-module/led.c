/*   
	Custom IOS Module (FAT)

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

#include "syscalls.h"
#include "swi_mload.h"
#include "types.h"

//#define THREAD_PRIORITY   0x79
//#define THREAD_PRIORITY   0x62
#define THREAD_STACK_SIZE 1024

static vu32 state = 1;

void Led_BlinkOn(void)
{
	state &= ~7;
}

void Led_BlinkOff(void)
{
	state |= 2;
}

s32 __Led_BlinkThread(void)
{
	u32 queue_space[8];
	s32 queue_handle;
	s32 timer_id;
	u32 message;

	/* Create message queue */
	queue_handle = os_message_queue_create(queue_space, 8);

	/* Create timer */
	timer_id = os_create_timer(1000*5, 1000*1000*10, queue_handle, 0);

	state = 4;

	/* Main loop */
	while (1) {

		/* Wait for message */
		s32 ret = os_message_queue_receive(queue_handle, (void*)&message, 0);
		if (ret)
			continue;

		os_stop_timer(timer_id);

		if ((state & 7) == 3) {
			state |= 4;
			Swi_LedOff();
		}

		if ((state & 4) == 4) {
			os_restart_timer(timer_id, 1000*1000, 0);
			continue;
		}

		state ^= 128;
   
		if (state & 128) {
			Swi_LedOn();
			os_restart_timer(timer_id, 1000*5, 0);
			state |= 1;
		}
		else {
			Swi_LedOff();
			os_restart_timer(timer_id, 1000*1000, 0);
		}
	}

	return 0;
}

void Led_CreateBlinkThread(void)
{
	static u8 thread_stack[THREAD_STACK_SIZE] ATTRIBUTE_ALIGN(32);

	s32 thread_id, priority;
	
	/* Get current thread priority */
	priority = os_thread_get_priority(os_get_thread_id()); 

	/* Create thread (in paused state) */
	thread_id = os_thread_create( (void *) __Led_BlinkThread, NULL, &thread_stack[THREAD_STACK_SIZE], THREAD_STACK_SIZE, priority, 0);

	/* Resume thread */
	if (thread_id >= 0)
		os_thread_continue(thread_id);
}