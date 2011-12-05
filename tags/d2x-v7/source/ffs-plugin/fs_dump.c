/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
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

#include <string.h>

#include "fs_calls.h"
#include "ioctl.h"
#include "ipc.h"
#include "isfs.h"
#include "plugin.h"
#include "syscalls.h"
#include "types.h"

void FS_Dump_Ioctl(ipcmessage *message, s32 ret)
{
//	u32 *inbuf = message->ioctl.buffer_in;
#ifdef DEBUG
	u32 *iobuf = message->ioctl.buffer_io;
#endif
//	u32  inlen = message->ioctl.length_in;
//	u32  iolen = message->ioctl.length_io;
	u32  cmd   = message->ioctl.command;

	/* Parse command */
	switch (cmd) {
	/** Create directory **/
	case IOCTL_ISFS_CREATEDIR: {
		FS_printf("FS_CreateDir: ret = %d\n", ret);
		break;
	}

	/** Create file **/
	case IOCTL_ISFS_CREATEFILE: {
		FS_printf("FS_CreateFile: ret = %d\n", ret);
		break;
	}

	/** Delete object **/
	case IOCTL_ISFS_DELETE: {
		FS_printf("FS_Delete: ret = %d\n", ret);
		break;
	}

	/** Rename object **/
	case IOCTL_ISFS_RENAME: {
		FS_printf("FS_Rename: ret = %d\n", ret);
		break;
	}

	/** Get device stats **/
	case IOCTL_ISFS_GETSTATS: {
		FS_printf("FS_GetStats: %ret = %d\n", ret);
		break;
	}

	/** Get file stats **/
	case IOCTL_ISFS_GETFILESTATS: {
#ifdef DEBUG
		fsfilestats *filestats = (fsfilestats *)iobuf;
		FS_printf("FS_GetFileStats: ret = %d, len = %d, pos = %d\n", ret, filestats->length, filestats->pos);
#endif
		break;
	}

	/** Get attributes **/
	case IOCTL_ISFS_GETATTR: {
#ifdef DEBUG
		fsattr *attr = (fsattr *)iobuf;
		FS_printf("FS_GetAttr: ret = %d, attr = {%08X, %04X, %02X, %02X, %02X, %02X}\n", ret, attr->owner_id, attr->group_id, attr->ownerperm, attr->groupperm, attr->otherperm, attr->attributes);		
#endif
		break;
	}

	/** Set attributes **/
	case IOCTL_ISFS_SETATTR: {
		FS_printf("FS_SetAttr: ret = %d\n", ret);		
		break;
	}

	/** Format **/
	case IOCTL_ISFS_FORMAT: {
		FS_printf("FS_Format: ret = %d\n", ret);
		break;
	}

	default:
		FS_printf("FS_Ioctl(): default case reached cmd = %x\n", cmd);
		break;
	}
}

void FS_Dump_Ioctlv(ipcmessage *message, s32 ret)
{
#ifdef DEBUG
	ioctlv *vector = message->ioctlv.vector;
#endif
//	u32     inlen  = message->ioctlv.num_in;
//	u32     iolen  = message->ioctlv.num_io;
	u32     cmd    = message->ioctlv.command;

	/* Parse command */
	switch (cmd) {
	/** Read directory **/
	case IOCTL_ISFS_READDIR: {
#ifdef DEBUG
		u32 entries = *(u32 *)vector[1].data;
		FS_printf("FS_ReadDir: ret = %d, entries = %d\n", ret, entries);
#endif
		break;
	}

	/** Get device usage **/
	case IOCTL_ISFS_GETUSAGE: {
#ifdef DEBUG
		u32 blocks = *(u32 *)vector[1].data;
		u32 inodes = *(u32 *)vector[2].data;
		FS_printf("FS_GetUsage: ret = %d, inodes = %d, blocks = %d\n", ret, inodes, blocks);
#endif
		break;
	}

	default:
		FS_printf("FS_Ioctlv(): default case reached cmd = %x\n", cmd);
		break;
	}
}

void  FS_os_message_queue_ack(struct ipcmessage *message, s32 result)
{
	switch (message->command) {
	case IOS_OPEN:

#ifdef DEBUG
#ifdef FILTER_OPENING_REQUESTS
		if (strncmp("/dev", message->open.device, 4) || !strncmp("/dev/fs", message->open.device, 7))
#endif
			FS_printf("FS_Open: ret = %d\n", result);
#endif
		break;

	case IOS_CLOSE:
		FS_printf("FS_Close: ret = %d\n", result);
		break;

	case IOS_READ:
		FS_printf("FS_Read: ret = %d\n", result);
		break;

	case IOS_WRITE:
		FS_printf("FS_Write: ret = %d\n", result);
		break;

	case IOS_SEEK:
		FS_printf("FS_Seek: ret = %d\n", result);
		break;

	case IOS_IOCTL:
		FS_Dump_Ioctl(message, result);
		break;

	case IOS_IOCTLV:
		FS_Dump_Ioctlv(message, result);
		break;

	default:
		/* Unknown command */
		break;
	}

	/* Acknowledge message */
	os_message_queue_ack(message, result);
}
