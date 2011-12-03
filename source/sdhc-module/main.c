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

#include <string.h>

#include "es.h"
#include "ipc.h"
#include "mem.h"
#include "module.h"
#include "sdio.h"
#include "syscalls.h"
#include "timer.h"
#include "types.h"
#include "wbfs.h"


s32 __SDHC_Ioctlv(u32 cmd, ioctlv *vector, u32 inlen, u32 iolen)
{
	s32 ret = IPC_EINVAL;

	/* Invalidate cache */
	InvalidateVector(vector, inlen, iolen);

	/* Parse IOCTLV command */
	switch (cmd) {
	/** Initialize SDHC **/
	case IOCTL_SDHC_INIT: {
		/* Initialize SDIO */
		ret = !sdio_Startup();

		break;
	}

	/** Read sectors **/
	case IOCTL_SDHC_READ: {
		u32   sector     = *(u32 *)(vector[0].data);
		u32   numSectors = *(u32 *)(vector[1].data);
		void *buffer     = vector[2].data;

		/* Read sectors */
		ret = !sdio_ReadSectors(sector, numSectors, buffer);

		break;
	}

	/** Write sectors **/
	case IOCTL_SDHC_WRITE: {
		u32   sector     = *(u32 *)(vector[0].data);
		u32   numSectors = *(u32 *)(vector[1].data);
		void *buffer     = vector[2].data;

		/* Write sectors */
		ret = !sdio_WriteSectors(sector, numSectors, buffer);

		break;
	}

	/** Check for SD card **/
	case IOCTL_SDHC_ISINSERTED: {
		/* Check if SD card is inserted */
		ret = !sdio_IsInserted();

		break;
	}

	/** Open WBFS disc **/
	case IOCTL_WBFS_OPEN_DISC: {
		u8 *discid = (u8 *)(vector[0].data);

		/* Open WBFS disc */
		ret = WBFS_OpenDisc(discid);

		break;
	}

	/** Read WBFS disc **/
	case IOCTL_WBFS_READ_DISC: {
		u32   offset = *(u32 *)(vector[0].data);
		u32   len    = *(u32 *)(vector[1].data);
		void *buffer = vector[2].data;

		/* Read WBFS disc */
		ret = WBFS_Read(buffer, len, offset);
		if (ret)
			ret = 0x8000;

		break;
	}

	default:
		break;
	}

	/* Flush cache */
	FlushVector(vector, inlen, iolen);

	return ret;
}

s32 __SDHC_Initialize(u32 *queuehandle)
{
	void *buffer = NULL;
	s32   ret;

	/* Initialize memory heap */
	Mem_Init();

	/* Initialize timer subsystem */
	Timer_Init();

	/* Allocate queue buffer */
	buffer = Mem_Alloc(0x80);
	if (!buffer)
		return IPC_ENOMEM;

	/* Create message queue */
	ret = os_message_queue_create(buffer, 32);
	if (ret < 0)
		return ret;

	/* Register devices */
	os_device_register(DEVICE_NAME, ret);

	/* Copy queue handler */
	*queuehandle = ret;

	return 0;
}


int main(void)
{
	u32 queuehandle;
	s32 ret;

	/* Print info */
	write("$IOSVersion: SDHC: " __DATE__ " " __TIME__ " 64M$\n");

	/* Initialize module */
	ret = __SDHC_Initialize(&queuehandle);
	if (ret < 0)
		return ret;

	/* Main loop */
	while (1) {
		ipcmessage *message = NULL;

		/* Wait for message */
		os_message_queue_receive(queuehandle, (void *)&message, 0);

		switch (message->command) {
		case IOS_OPEN: {
			u64 tid;

			/* Get title ID */
			ret = ES_GetTitleID(&tid);

			/* Check title ID */
			if (ret >= 0) {
				write("SDHC: Title identified. Blocking opening request.\n");

				ret = IPC_ENOENT;
				break;
			}

			/* Check device path */
			if (!strcmp(message->open.device, DEVICE_NAME))
				ret = message->open.resultfd;
			else
				ret = IPC_ENOENT;

			break;
		}

		case IOS_CLOSE: {
			/* Close SDIO */
			ret = !sdio_Shutdown();

			break;
		}

		case IOS_IOCTLV: {
			ioctlv *vector = message->ioctlv.vector;
			u32     inlen  = message->ioctlv.num_in;
			u32     iolen  = message->ioctlv.num_io;
			u32     cmd    = message->ioctlv.command;

			/* Parse IOCTLV message */
			ret = __SDHC_Ioctlv(cmd, vector, inlen, iolen);

			break;
		}

		default:
			/* Unknown command */
			ret = IPC_EINVAL;
		}

		/* Acknowledge message */
		os_message_queue_ack(message, ret);
	}
   
	return 0;
}
