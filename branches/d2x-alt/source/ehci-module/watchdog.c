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

#include <stdlib.h>

#include "ehci_config.h"
#include "ehci_types.h"
#include "syscalls.h"
#include "types.h"
#include "usb_os.h"
#include "usbstorage.h"

/* Static variables */
static u32 started  = 0;
static s32 timer_id = -1;


s32 WATCHDOG_CreateTimer(s32 queue_id, u32 message)
{
	s32 ret;
	
	/* Create watchdog timer */
	ret = os_create_timer(config.watchdogTimeout, config.watchdogTimeout, queue_id, message);
	if (ret < 0)
		return ret;

	/* Set timer handler */
	timer_id = ret;

	/* Set watchdog timer state */
	started = 1;

	return 0;
}

void WATCHDOG_StopTimer(void)
{
	if (started) {
		/* Stop watchdog timer */
		os_stop_timer(timer_id);

		/* Set watchdog timer state */
		started = 0;
	}
}

void WATCHDOG_RestartTimer(void)
{
	/* Watchdog enabled */
	if (config.watchdogTimeout != 0)  {
		/* Set watchdog timer state */
		started = 1;

		/* Restart watchdog timer */
		os_restart_timer(timer_id, config.watchdogTimeout, config.watchdogTimeout);
	}
}

void WATCHDOG_Run(void)
{
	/* Watchdog enabled */
	if (config.watchdogTimeout != 0) {
		u32 nbSectors, sectorSz;

		/* Get device info */
		nbSectors = USBStorage_Get_Capacity(&sectorSz);

		/* Device available */
		if (nbSectors) {
			void *sectorBuf;

			/* Get sector buffer */
			sectorBuf = USB_GetSectorBuffer(sectorSz);

			/* Read random sector */
			USBStorage_Read_Sectors(rand() % nbSectors, 1, sectorBuf);
		}
	}
}
