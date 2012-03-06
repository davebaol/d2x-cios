/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.
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

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "detect.h"
#include "elf.h"
#include "epic.h"
#include "es.h"
#include "ios.h"
#include "ipc.h"
#include "mem.h"
#include "module.h"
#include "patches.h"
#include "stealth.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "timer.h"
#include "tools.h"
#include "types.h"

#define DEBUG_MODE DEBUG_NONE
//#define DEBUG_MODE DEBUG_BUFFER
//#define DEBUG_MODE DEBUG_GECKO


/* Global variables */
char *moduleName = "MLOAD";
s32 offset       = 0;
u32 stealth_mode = 1;     // Stealth mode is on by default


static s32 __MLoad_Ioctlv(u32 cmd, ioctlv *vector, u32 inlen, u32 iolen)
{
	s32 ret = IPC_ENOENT;

	/* Invalidate cache */
	InvalidateVector(vector, inlen, iolen);

	/* Check command */
	switch (cmd) {
	case MLOAD_GET_IOS_INFO: {
		iosInfo *buffer = vector[0].data;

		/* Copy IOS info */
		memcpy(buffer, &ios, sizeof(ios));

		/* Success */
		ret = 0;

		break;
	}

	case MLOAD_GET_MLOAD_VERSION: {
		/* Return MLOAD version */
		ret = (MLOAD_VER << 4) | MLOAD_SUBVER;

		break;
	}

	case MLOAD_GET_LOAD_BASE: {
		u32 *address = (u32 *)vector[0].data;
		u32 *size    = (u32 *)vector[1].data;

		/* Modules area info */
		*address = (u32)exe_mem;
		*size    = (u32)exe_mem_size;

		/* Success */
		ret = 0;

		break;
	}

	case MLOAD_LOAD_ELF: {
		void *data = vector[0].data;

		/* Load ELF */
		ret = Elf_Load(data);

		break;
	}

	case MLOAD_RUN_ELF: {
		/* Run ELF */
		ret = Elf_Run();

		break;
	}

	case MLOAD_RUN_THREAD: {
		u32 start = *(u32 *)vector[0].data;
		u32 stack = *(u32 *)vector[1].data;
		u32 slen  = *(u32 *)vector[2].data;
		u32 prio  = *(u32 *)vector[3].data;

		/* Run thread */
		ret = Elf_RunThread((void *)start, NULL, (void *)stack, slen, prio);

		break;
	}

	case MLOAD_STOP_THREAD: {
		u32 tid = *(u32 *)vector[0].data;

		/* Stop thread */
		ret = Elf_StopThread(tid);

		break;
	}

	case MLOAD_CONTINUE_THREAD: {
		u32 tid = *(u32 *)vector[0].data;

		/* Continue thread */
		ret = Elf_ContinueThread(tid);

		break;
	}

	case MLOAD_MEMSET: {
		u32 addr = *(u32 *)vector[0].data;
		u32 val  = *(u32 *)vector[1].data;
		u32 len  = *(u32 *)vector[2].data;

		/* Invalidate cache */
		os_sync_before_read((void *)addr, len);

		/* Do memset */
		memset((void *)addr, val, len);

		/* Success */
		ret = 0;

		break;
	}

	case MLOAD_SET_LOG_MODE: {
		u32 mode = *(u32 *)vector[0].data;

		/* Set debug mode */
		ret = Debug_SetMode(mode);

		break;
	}

	case MLOAD_GET_LOG_BUFFER: {
		char *buffer = (char *)vector[0].data;
		u32   len    = *(u32 *)vector[0].len;

		/* Get debug buffer */
		ret = Debug_GetBuffer(buffer, len-1);

		break;
	}

	case MLOAD_SET_STEALTH_MODE: {
		u32 mode = *(u32 *)vector[0].data;

		/* Set stealth mode */
		stealth_mode = mode;

		ret = 0;

		break;
	}

	default:
		break;
	}

	/* Flush cache */
	FlushVector(vector, inlen, iolen);

	return ret;
}

static s32 __MLoad_DisableMem2Protection(void)
{
	/* Disable MEM2 protection (so the PPC can access all 64MB) */
	MEM2_Prot(0);

	return 0;
}

/* System detectors and patchers */
static patcher moduleDetectors[] = {
	{Detect_DipModule, 0},
	{Detect_EsModule, 0},
	{Detect_FfsModule, 0},
	{Detect_IopModule, 0},
	{Patch_IopModule, 0}  // We want to patch swi vector asap
};

s32 __MLoad_System(void)
{
	/* Detect modules and patch swi vector */
	return IOS_PatchModules(moduleDetectors, sizeof(moduleDetectors));
}

static s32 __MLoad_Initialize(u32 *queuehandle)
{
	/* Heap space */
	static u32 heapspace[0x1000] ATTRIBUTE_ALIGN(32);

	void *buffer = NULL;
	s32   ret;

	/* Initialize memory heap */
	ret = Mem_Init(heapspace, sizeof(heapspace));
	if (ret < 0)
		return ret;

	/* Initialize timer subsystem */
	ret = Timer_Init();
	if (ret < 0)
		return ret;

	/* Allocate queue buffer */
	buffer = Mem_Alloc(0x20);
	if (!buffer)
		return IPC_ENOMEM;

	/* Create message queue */
	ret = os_message_queue_create(buffer, 8);
	if (ret < 0)
		return ret;

	/* System patchers */
	static patcher patchers[] = {
		{Patch_DipModule, 0},
		{Patch_EsModule, 0},
		{Patch_FfsModule, 0},
		{__MLoad_DisableMem2Protection, 0}
	};

	/* Initialize plugin */
	IOS_InitSystem(patchers, sizeof(patchers));

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

	/* Avoid GCC optimizations */
	exe_mem[0] = 0;

	/* Print info */
	svc_write("$IOSVersion: MLOAD: " __DATE__ " " __TIME__ " 64M$\n");

	/* Call __MLoad_System through software interrupt 9 */
	ret = os_software_IRQ(9);

	/* Set debug mode */
 	Debug_SetMode(DEBUG_MODE);

	/* Check modules patch */
	if (ret) {
		svc_write("MLOAD: ERROR -> Can't detect some IOS modules.\n");
		IOS_CheckPatches(moduleDetectors, sizeof(moduleDetectors));
	}

	/* Initialize module */
	ret = __MLoad_Initialize(&queuehandle);
	if (ret < 0)
		return ret;

	/* Initialize stuff */
	Epic_Init(queuehandle);

	/* Main loop */
	while (1) {
		ipcmessage *message = NULL;

		/* Wait for message */
		ret = os_message_queue_receive(queuehandle, (void *)&message, 0);
		if (ret)
			continue;

		/* Epic stuff */
		if ((u32)message == EPIC_MESSAGE) {
			Epic_Main();
			continue;
		}

		/* Parse command */
		switch (message->command) {
		case IOS_OPEN: {

			/* Block opening request if a title is running */
			ret = Stealth_CheckRunningTitle(NULL);
			if (ret) {
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
			/* Do nothing */
			break;
		}

		case IOS_READ: {
			void *dst = message->read.data;
			void *src = (void *)offset;
			u32   len = message->read.length;

			/* Read data */
			Swi_uMemcpy(dst, src, len);

			/* Update offset */
			offset += len;
		}

		case IOS_WRITE: {
			void *dst = (void *)offset;
			void *src = message->write.data;
			u32   len = message->write.length;

			/* Write data */
			Swi_Memcpy(dst, src, len);

			/* Update offset */
			offset += len;
		}

		case IOS_SEEK: {
			s32 whence = message->seek.origin;
			s32 where  = message->seek.offset;

			/* Update offset */
			switch (whence) {
			case SEEK_SET:
				offset = where;
				break;

			case SEEK_CUR:
				offset += where;
				break;

			case SEEK_END:
				offset = -where;
				break;
			}

			/* Return offset */
			ret = offset;

			break;
		}

		case IOS_IOCTLV: {
			ioctlv *vector = message->ioctlv.vector;
			u32     inlen  = message->ioctlv.num_in;
			u32     iolen  = message->ioctlv.num_io;
			u32     cmd    = message->ioctlv.command;

			/* Parse IOCTLV */
			ret = __MLoad_Ioctlv(cmd, vector, inlen, iolen);

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
