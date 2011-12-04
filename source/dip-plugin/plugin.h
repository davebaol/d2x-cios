/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
 * Copyright (C) 2011 davebaol, oggzee.
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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "ipc.h"
#include "tools.h"
#include "types.h"

/* WOD magic word */
#define WOD_MAGIC		0x5D1C9EA3

/* GameCube disc magic word */
#define GC_MAGIC		0xC2339F3D

/* Mode codes */
#define MODE_DVDROM		0x01
#define MODE_CRYPT		0x02
#define MODE_WBFS		0x04
#define MODE_FILE		0x08
#define MODE_FRAG		0x10
#define MODE_EMUL		( MODE_FILE | MODE_WBFS | MODE_FRAG )

/* Macros */
#define DI_SetMode(bit)		BIT_SET(config.mode, (bit))
#define DI_DelMode(bit)		BIT_DEL(config.mode, (bit))
#define DI_ChkMode(bit)		BIT_CHK(config.mode, (bit))

/* Macros */
#define BIT_SET(x, y)	(x |=  y)
#define BIT_DEL(x, y)	(x &= ~y)
#define BIT_CHK(x, y)	(x & y)

/* Disc types */
enum {
	DISC_UNKNOWN = 0,
	DISC_DVD5,
	DISC_DVD9,
};


/* Config structure */
struct dipConfig {
	/* Modes */
	u32 mode;

	/* Type */
	u32 type;

	/* Offsets */
	u32 offset[2];

	/* Last error */
	u32 error;

	/* Misc variables */
	u32 cover;
	u32 noreset;
};

#define FILENAME_MAX_LEN  256

/* Config structure */
struct dipConfigState {

	/* Modes */
	u32 mode;
	
	union {

		/* WBFS device */
		u32 wbfs_device;

		/* FAT filename */
		char fat_filename[FILENAME_MAX_LEN];
		
		/* FRAG state */
		struct {
			u32   device;
			u32   size;
		} frag;

	};
};

/* IOCTL structure */
typedef struct {
	u32 command;

	u32 *inbuf;
	u32  inlen;
	u32 *iobuf;
	u32  iolen;
} ioctl;

/* Prototypes */
s32 DI_EmulateIoctl(ioctl *buffer, s32 fd);
s32 DI_EmulateCmd(u32 *inbuf, u32 *outbuf, u32 size);
void DI_EmulateInitStage2(void);

#endif
