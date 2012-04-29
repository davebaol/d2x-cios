/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
	Copyright (C) 2009 Waninkoko.
	Copyright (C) 2011 rodries.
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

#include "ehci_config.h"
#include "ehci_types.h"
#include "ios.h"
#include "loop.h"
#include "mem.h"
#include "patches.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "tinyehci.h"
#include "tools.h"
#include "types.h"
#include "usb_os.h"
#include "vsprintf.h"
#include "watchdog.h"


char *moduleName = "EHCI/SDHC";

static void __Wait_MLoad(void)
{
	for(;;) {
		/* Get IOS info */
		Swi_GetIosInfo(&ios);	

		/* Check IOS info are valid */
		if(ios.syscallBase && ios.dipVersion && ios.esVersion && ios.ffsVersion && ios.iopVersion && ios.sdiVersion)
			break;
			
		//svc_write("EHCI: Wating for mload....\n");

		/* Wait a bit before retrying */
		ehci_msleep(30);
	}

	//svc_write("EHCI: Now mload is ready.\n");
}


int main(void)
{
	/* Heap space */
	static u32 heapspace[0x5000] ATTRIBUTE_ALIGN(32);

	/* System patchers */
	static patcher patchers[] = {
		{Patch_IopModule, 0}
	};

	s32 ret;

	/* Print info */
	svc_write("$IOSVersion: EHCI: " __DATE__ " " __TIME__ " 64M$\n");

	/* Initialize memory heap */
	ret = Mem_Init(heapspace, sizeof(heapspace));
	if (ret < 0)
		return ret;

	/* Initialize USB memory heap */
	USB_Mem_Init();

	/* Initialize TinyEhci (stage 1) */
	ret = EHCI_InitStage1();
	if (ret < 0)
		return ret;

	/* Load config */
	EHCI_LoadConfig();

	/* Set current USB port */
	current_port = (config.useUsbPort1 != 0);
	
	/* Wait mload */
	__Wait_MLoad();

	/* Prepare system */
	IOS_InitSystem(patchers, sizeof(patchers));
	
	/* Initialize TinyEhci (stage 2) */
	EHCI_InitStage2();

	/* Make sure EHCI config file is deleted */
	EHCI_DeleteConfig();

	/* Main loop */
	EHCI_Loop();

	return 0;
}
