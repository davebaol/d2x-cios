/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "es.h"
#include "es_calls.h"
#include "ioctl.h"
#include "ipc.h"
#include "plugin.h"
#include "syscalls.h"
#include "types.h"

/* Global config */
struct esConfig config = { 0 };


s32 __ES_GetTitleID(void *tid)
{
	ipcmessage message;
	ioctlv     vector;

	/* Clear buffer */
	memset(&message, 0, sizeof(message));

	/* Setup vector */
	vector.data = tid;
	vector.len  = 8;

	/* Setup message */
	message.ioctlv.command = 0x20;
	message.ioctlv.num_io  = 1;
	message.ioctlv.vector  = &vector;

	/* Call handler */
	return ES_HandleIoctlv(&message);
}

s32 __ES_GetTicketView(u32 tidh, u32 tidl, u8 *view)
{
	static u8 buffer[TIK_SIZE] ATTRIBUTE_ALIGN(32);

	char path[256];
	s32  fd, ret;

	/* Generate path */
	ES_snprintf(path,  sizeof(path), "/ticket/%08x/", tidh);
	ES_snprintf(path + strlen(path), sizeof(path), "%08x.tik", tidl);

	/* Open ticket */
	fd = os_open(path, 1);
	if (fd < 0)
		return fd;

	/* Read ticket */
	ret = os_read(fd, buffer, sizeof(buffer));

	/* Close ticket */
	os_close(fd);

	/* Read error */
	if (ret < 0)
		return ret;

	/* Clear ticket view */
	memset(view, 0, 0xD8);

	/* Generate ticket view */
	*(u8 *) (view + 0x00) = *(u8 *) (buffer + 0x1BC);
	*(u64 *)(view + 0x04) = *(u64 *)(buffer + 0x1D0);
	*(u32 *)(view + 0x0C) = *(u32 *)(buffer + 0x1D8);
	*(u64 *)(view + 0x10) = *(u64 *)(buffer + 0x1DC);
	*(u16 *)(view + 0x1A) = *(u16 *)(buffer + 0x1E6);
	*(u32 *)(view + 0x1C) = *(u32 *)(buffer + 0x1E8);
	*(u32 *)(view + 0x20) = *(u32 *)(buffer + 0x1EC);
	*(u8 *) (view + 0x24) = *(u8 *) (buffer + 0x1F0);
	*(u8 *) (view + 0x55) = *(u8 *) (buffer + 0x221);

	memcpy(view + 0x18, buffer + 0x1E4, 2);
	memcpy(view + 0x25, buffer + 0x1F1, 0x30);
	memcpy(view + 0x56, buffer + 0x222, 0x40);
	memcpy(view + 0x98, buffer + 0x264, 0x40);

	return 0;
}

s32 __ES_CustomLaunch(u32 tidh, u32 tidl)
{
	static tikview view ATTRIBUTE_ALIGN(32) = { 0 };
	s32 ret;

	/* Get ticket view */
	ret = __ES_GetTicketView(tidh, tidl, (void *)&view);
	if (ret < 0)
		return ret;

	/* Launch title */
	return ES_LaunchTitle(tidh, tidl, &view, 0);
}

s32 __ES_Ioctlv(ipcmessage *message)
{
	ioctlv *vector = message->ioctlv.vector;
	u32     inlen  = message->ioctlv.num_in;
	u32     iolen  = message->ioctlv.num_io;
	u32     cmd    = message->ioctlv.command;

	/* Parse command */
	switch (cmd) {
	case IOCTL_ES_LAUNCH: {
		u64 tid = *(u64 *)vector[0].data;

		u32 tidh = (u32)(tid >> 32);
		u32 tidl = (u32)(tid & 0xFFFFFFFF);

		/* IOS title launch */
		if (tidh == 1 &&
		   (tidl >= 3 && tidl <= 255)) {

			/* Fake launch */
			switch (config.fakelaunch) {
			case 1:
				/* Return success */
				return 0;

			case 2:
				/* Launch title (fake ID) */
				return __ES_CustomLaunch(tidh, 249);
			}
		}

		break;
	}

	case IOCTL_ES_KOREANCHECK: {
		/* Return error */
		return -1017;
	}

	case IOCTL_ES_FAKELAUNCH: {
		u32 val = *(u32 *)vector[0].data;

		/* Set fake launch mode */
		config.fakelaunch = val;

		/* Save config */
		Config_Save(&config, sizeof(config));

		return 0;
	}

	case IOCTL_ES_LEET: {
		/* Title launch */
		return __ES_CustomLaunch(1, 257);
	}

	default:
		break;
	}

	/* Call IOCTLV handler */
	return ES_HandleIoctlv(message);
}


s32 ES_EmulateCmd(ipcmessage *message)
{
	s32 ret = 0;

	/* Parse IPC command */
	switch (message->command) {
	case IOS_IOCTLV: {
		/* Parse IOCTLV message */
		ret = __ES_Ioctlv(message);

		break;
	}

	default:
		ret = IPC_EINVAL;
	}

	return ret;
}
