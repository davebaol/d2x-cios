/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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

#ifndef _DIP_H_
#define _DIP_H_

#include "types.h"

/* Disc lengths */
#define DVD5_LENGTH             0x46090000
#define DVD9_LENGTH             0x7ED38000

/* Disc sector */
#define SECTOR_SIZE		0x800
#define MAX_SECTOR_SIZE		0x7F8000


/* Prototypes */
u32 DI_CustomCmd(void *inbuf, void *outbuf);
s32 DI_StopMotor(void);
s32 DI_ReadDvd(u8 *outbuf, u32 len, u32 offset);
s32 DI_ReadWod(void *outbuf, u32 len, u32 offset);

#endif
