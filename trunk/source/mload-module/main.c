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
#include "elf.h"
#include "epic.h"
#include "es.h"
#include "ipc.h"
#include "mem.h"
#include "module.h"
#include "patches.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "timer.h"
#include "tools.h"
#include "types.h"

#define DEBUG_MODE DEBUG_NONE
//#define DEBUG_MODE DEBUG_BUFFER
//#define DEBUG_MODE DEBUG_GECKO


/* IOS information */
iosInfo ios = { 0, 0, 0, 0, 0 };

/* Variables */
s32 offset = 0;


s32 __MLoad_Ioctlv(u32 cmd, ioctlv *vector, u32 inlen, u32 iolen)
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

	default:
		break;
	}

	/* Flush cache */
	FlushVector(vector, inlen, iolen);

	return ret;
}

void __MLoad_Detect(void)
{
	u32 dipAddr, esAddr, ffsAddr, iopAddr;

	/* Get modules addresses */
	dipAddr = *(vu32 *)0x20200040;
	esAddr  = *(vu32 *)0x20100044;
	ffsAddr = *(vu32 *)0x20000044;
	iopAddr = *(vu32 *)0xFFFF0028;

	/* Set DIP version */
	switch (dipAddr) {

	case 0x20207F40:		// IOS: 37v5662, 53v5662, 55v5662
		/* DIP: 07/11/08 14:34:26 */
		ios.dipVersion = 0x48776F72;
		break;

	case 0x20207C2C:		// IOS: 36v3607, 38v4123
		/* DIP: 07/24/08 20:08:44 */
		ios.dipVersion = 0x4888E14C;
		break;

	case 0x20207EA8:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 70v6687, 80v6943
		/* DIP: 06/03/09 07:49:09 */
		ios.dipVersion = 0x4A262AF5;
		break;

	case 0x20207DB8:		// IOS: 60v6174
		/* DIP: 11/24/08 15:39:09 */
		ios.dipVersion = 0x492ACA9D;
		break;
	}

	/* Set ES version */
	switch (esAddr) {
//	case 0x2010147D:		// IOS: ??????  --> NO MORE SUPPORTED
//		/* ES: 06/03/09 03:45:06 */
//		ios.esVersion = 0x4A25F1C2;
//		break;

//	case 0x201013F5:		// IOS: 36v3090, 38v3610  --> NO MORE SUPPORTED
//		/* ES: 06/03/09 03:36:55 */
//		ios.esVersion = 0x4A25EFD7;
//		break;

	case 0x201015A5:		// IOS: 70v6687
		/* ES: 06/03/09 07:46:02 */
		ios.esVersion = 0x4A262A3A;
		break;

	case 0x201014D5:		// IOS: 60v6174
		/* ES: 11/24/08 15:36:08 */
		ios.esVersion = 0x492AC9E8;
		break;

	case 0x201015E9:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943
		/* ES: 03/03/10 10:40:14 */
		ios.esVersion = 0x4B8E90EE;
		break;

	case 0x2010142D:		// IOS: 37v5662, 53v5662, 55v5662		
		/* ES: 03/01/10 03:26:03 */
		ios.esVersion = 0x4B8B882B;
		break;

	case 0x2010139D:		// IOS: 36v3607, 38v4123
		/* ES: 03/01/10 03:18:58 */
		ios.esVersion = 0x4B8B8682;
		break;
	}

	/* Set FFS version */
	switch (ffsAddr) {
	case 0x20005D89:		// IOS: 37v5662, 53v5662, 55v5662	
		/* FFS: 12/24/08 13:48:17 */
		ios.ffsVersion = 0x49523DA1;
		break;

	case 0x2000200D:		// IOS: 36v3607, 38v4123
		/* FFS: 12/23/08 17:26:21 */
		ios.ffsVersion = 0x49511F3D;
		break;

	case 0x20006009:		// IOS: 56v5661, 57v5918, 58v6175, 60v6174, 61v5661, 70v6687, 80v6943 	
		/* FFS: 11/24/08 15:36:10 */
		ios.ffsVersion = 0x492AC9EA;
		break;
	}

	/* Set IOP version */
	switch (iopAddr) {

	case 0xFFFF1D60:		// IOS: 37v5662, 53v5662, 55v5662	
		/* IOSP: 07/11/08 14:34:29 */
				/* IOSP: 03/01/10 03:28:58 */
		ios.iopVersion = 0x48776F75;
		ios.syscall    = 0xFFFF91B0;

		break;

	/*
	 * NOTE:  
	 * IOS38 v3610 is not supported anymore 
	 * since has been replaced by IOS38 v4123 (see below)  
	 */ 
//	case 0xFFFF1CA0:        // IOS: 38v3610 
//		/* IOSP: 12/23/08 17:28:32 */
//		ios.iopVersion = 0x49511FC0;
//		ios.syscall    = 0xFFFF8AA0;
//
//		break;

	case 0xFFFF1D10:		// IOS: 36v3607, 38v4123
		/* IOSP: 03/01/10 03:13:17 */
		ios.iopVersion = 0x4B8B30CD;
		ios.syscall    = 0xFFFF9100;

		break;

	case 0xFFFF1F20:
		iopAddr = *(vu32 *)0xFFFF2418;

		switch (iopAddr) {
		case 0xFFFF9390:		// IOS: 60v6174, 70v6687	
			/* IOSP: 11/24/08 15:39:12 */
					/* IOSP: 06/03/09 07:49:12 */
			ios.iopVersion = 0x492ACAA0;
			ios.syscall    = 0xFFFF9390;

			break;

		case 0xFFFF93D0:		// IOS: 56v5661, 57v5918, 58v6175, 61v5661, 80v6943	
			/* IOSP: 03/03/10 10:43:18 */
			ios.iopVersion = 0x4B8E3D46;
			ios.syscall    = 0xFFFF93D0;

			break;
		}

		break;
	}
}

s32 __MLoad_System(void)
{
	u32 perms;

	/* Invalidate cache */
	ICInvalidate();

	/* Apply permissions */
	perms = Perms_Read();
	Perms_Write(0xFFFFFFFF);

	/* Detect modules */
	__MLoad_Detect();

	/* Apply patches */
	Patch_DipModule(ios.dipVersion);
	Patch_EsModule (ios.esVersion);
	Patch_FfsModule(ios.ffsVersion);
	Patch_IopModule(ios.iopVersion);

	/* Disable MEM2 protection */
	MEM2_Prot(0);

	/* Restore permissions */
	Perms_Write(perms);

	return 0;
}

s32 __MLoad_Initialize(u32 *queuehandle)
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

	/* Enable PPC HW access */
	os_set_ahbprot(1);

	/* Software interrupt */
	os_software_IRQ(9);

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

	/* Initialize module */
	ret = __MLoad_Initialize(&queuehandle);
	if (ret < 0)
		return ret;

	/* Set debug mode */
 	Debug_SetMode(DEBUG_MODE);

	/* Initialize stuff */
	Epic_Init(queuehandle);

	/* Main loop */
	while (1) {
		ipcmessage *message = NULL;

		/* Wait for message */
		os_message_queue_receive(queuehandle, (void *)&message, 0);

		/* Epic stuff */
		if ((u32)message == EPIC_MESSAGE) {
			Epic_Main();
			continue;
		}

		switch (message->command) {
		case IOS_OPEN: {
			u64 tid;

			/* Get title ID */
			ret = ES_GetTitleID(&tid);

			/* Check title ID */
			if (ret >= 0) {
				svc_write("MLOAD: Title identified. Blocking opening request.\n");

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
