/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "di.h"
#include "es_calls.h"
#include "ioctl.h"
#include "isfs.h"
#include "ipc.h"
#include "plugin.h"
#include "syscalls.h"
#include "types.h"

/* Constants */
#define ES_SIG_RSA4096		0x10000
#define ES_SIG_RSA2048		0x10001

#define TIK_SIZE		676


/* RSA (2048 bits) signature structure */
typedef struct {
	u32 type;
	u8 sig[256];
	u8 fill[60];
} ATTRIBUTE_PACKED sig_rsa2048;

/* RSA (4096 bits) signature structure */
typedef struct {
	u32 type;
	u8  sig[512];
	u8  fill[60];
} ATTRIBUTE_PACKED sig_rsa4096;

/* Ticket limit structure */
typedef struct {
	u32 tag;
	u32 value;
} ATTRIBUTE_PACKED tiklimit;

/* Ticket view structure */
typedef struct {
	u32 view;
	u64 ticketid;
	u32 devicetype;
	u64 titleid;
	u16 access_mask;
	u8  reserved[0x3c];
	u8  cidx_mask[0x40];
	u16 padding;
	tiklimit limits[8];
} ATTRIBUTE_PACKED tikview;

/* Ticket structure */
typedef struct {
	char issuer[0x40];
	u8  fill[63];
	u8  cipher_title_key[16];
	u8  fill2;
	u64 ticketid;
	u32 devicetype;
	u64 titleid;
	u16 access_mask;
	u8  reserved[0x3c];
	u8  cidx_mask[0x40];
	u16 padding;
	tiklimit limits[8];
} ATTRIBUTE_PACKED tik;

/* TMD content structure */
typedef struct {
	u32 cid;
	u16 index;
	u16 type;
	u64 size;
	u8 hash[20];
} ATTRIBUTE_PACKED tmd_content;

/* TMD structure */
typedef struct {
	char issuer[0x40];
	u8 version;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 fill2;
	u64 sys_version;
	u64 title_id;
	u32 title_type;
	u16 group_id;
	u16 zero;
	u16 region;
	u8 ratings[16];
	u8 reserved[12];
	u8 ipc_mask[12];
	u8 reserved2[18];
	u32 access_rights;
	u16 title_version;
	u16 num_contents;
	u16 boot_index;
	u16 fill3;
	tmd_content contents[];
} ATTRIBUTE_PACKED tmd;


/* Macros */
#define SIGNATURE_SIZE(x) (\
	((*(x))==ES_SIG_RSA2048) ? sizeof(sig_rsa2048) : ( \
	((*(x))==ES_SIG_RSA4096) ? sizeof(sig_rsa4096) : 0 ))

#define SIGNATURE_PAYLOAD(x) ((void *)(((u8*)(x)) + SIGNATURE_SIZE(x)))


/* Global config */
struct esConfig config = { 0, 0, 0, 0 };


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
	ES_snprintf(path, sizeof(path), "/ticket/%08x/", tidh);
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
	//u32     iolen  = message->ioctlv.num_io;
	u32     cmd    = message->ioctlv.command;
	
	/* Parse command */
	switch (cmd) {
	case IOCTL_ES_LAUNCH: {
		u64 tid = *(u64 *)vector[0].data;

		u32 tidh = (u32)(tid >> 32);
		u32 tidl = (u32)(tid & 0xFFFFFFFF);

		/* System title launch */
		if (tidh == 1) {

			/* System menu launch */
			if (tidl == 2) {

				/* Disable NAND emulation */
				ISFS_DisableEmulation();

				/* Launch title (fake ID) */
				if (config.sm_title_id != 0)
					return __ES_CustomLaunch((u32) (config.sm_title_id>>32), (u32) config.sm_title_id);
			}

			/* IOS launch */
			if (tidl >= 3 && tidl <= 255) {

				/* Fake launch */
				switch (config.fakelaunch) {
				case 1:
					/* Skip ios reload and return success */
					return 0;

				case 2:
					/* Reload the cIOS in place of the requested IOS */
					if (config.title_id==0) {
						s32 ret;

						/* Get title ID */
						ret = __ES_GetTitleID(&config.title_id);

						/* Disc-based games have title IDs of 00010000xxxxxxxx and 00010004xxxxxxxx */
						if (ret>=0 && (config.title_id>>32==0x00010000 || config.title_id>>32==0x00010004)) {
					
							/* Save config */
							Config_Save(&config, sizeof(config));

							/* Save DI config */
							DI_Config_Save();

							/* Launch title (fake ID) */
							return __ES_CustomLaunch(tidh, config.ios);
						}

						/* Reset title ID */
						config.title_id = 0;
					}
				}
				break;
			}
		}

		break;
	}

	case IOCTL_ES_DIVERIFY: {
		/* Check whether the cios has been reloaded by a disc-based game */
		if (config.fakelaunch==2 && config.title_id!=0) {
			u8* tmd = (u8*)(vector[3].data);
			if (tmd) {

				/* Get title ID from TitleMetaData */
				u64 title_id = *(u64 *)(tmd+0x18C);
 
				/* Check title ID */
				if(title_id != config.title_id) {

					/* Disable ios reload block */
					config.fakelaunch = 0;
				}
				else {

					/* Get the required ios from TitleMetaData */
					u64 required_ios = *(u64 *)(tmd+0x184);

					// Fix d2x v7 beta1
					// This line 
					// *(u32 *)0x00003140 = (*((u32 *)0x00003188)) | 0xFFFF;
					// has been made more general by taking the required IOS
					// from TMD rather than from MEM1. This way all those
					// games that reload different IOSs are now supported.
					// For example in Wii Fit Plus IOS53 is required for the game 
					// while IOS33 is required for the channel installation.
					/* Remove error 002 */
					*(u32 *)0x00003140 = (((u32)required_ios)<<16) | 0xFFFF;
				}
			}

			/* Reset title ID */
			config.title_id = 0;
		}

		break;
	}

	case IOCTL_ES_KOREANCHECK: {
		/* Return error */
		return -1017;
	}

	case IOCTL_ES_FAKE_IOS_LAUNCH: {
		u32 mode = *(u32 *)vector[0].data;
		u32 ios = inlen>1 ? *(u32 *)vector[1].data : 249;

		/* Set fake ios launch */
		config.fakelaunch = mode;
		config.ios        = ios;
		config.title_id   = 0;

		return 0;
	}

	case IOCTL_ES_FAKE_SM_LAUNCH: {
		u64 sm_title_id = *(u64 *)vector[0].data;
    
		/* Set fake system menu launch */
		config.sm_title_id = sm_title_id;

		return 0;
	}

	case IOCTL_ES_LEET: {
		/* Launch MIOS */
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
