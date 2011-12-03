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

#include <stdarg.h>

#include "di.h"
#include "epic.h"
#include "es.h"
#include "gpio.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "timer.h"
#include "types.h"

/* Constants */
#define EPIC_TIME	125000

/* Variables */
static u32 cnt = 0;
static u32 tmr = 0;
static s32 timerId = -1;


void __Epic_Trigger(void)
{
	/* Blink LED */
	Swi_LedBlink();

	/* Sleep 5 seconds */
	msleep(5000);

	/* Reset DVD drive */
	DI_Reset();

	/* Sleep 5 seconds */
	msleep(5000);

	/* Do some ES stuff */
	ES_LeetStuff();
}


s32 Epic_Init(s32 queuehandle)
{
	/* Already inited */
	if (timerId >= 0)
		return 0;

	/* Create timer */
	timerId = os_create_timer(0, 0, queuehandle, EPIC_MESSAGE);
	if (timerId < 0)
		return timerId;

	/* Restart timer */
	os_restart_timer(timerId, 0, EPIC_TIME * 4);

	return 0;
}

void Epic_Main(void)
{
	s32 ret;

	/* Stop timer */
	os_stop_timer(timerId);

	/* Check GPIO */
	ret = GPIO_Read(6);

	if (ret) {
		/* Update counter */
		if (++cnt > 5)
			__Epic_Trigger();

		/* Reset stuff */
		tmr = 0;
	} else if (cnt) {
		/* Update stuff */
		if (++tmr > 5)
			cnt = tmr = 0;
	}

	/* Restart timer */
	os_restart_timer(timerId, 0, EPIC_TIME);
}
