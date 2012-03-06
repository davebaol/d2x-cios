/*
 * DIP plugin for Custom IOS
 *
 * Copyright (C) 2010-2011 Waninkoko, WiiGator, oggzee, davebaol.
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

#ifndef _FRAG_H_
#define _FRAG_H_

#define FRAG_MAX 20000

typedef struct
{
	u32 offset; // file offset, in sectors unit
	u32 sector;
	u32 count;
} Fragment;

typedef struct
{
	u32 size; // num sectors
	u32 num;  // num fragments
	u32 maxnum;
	Fragment frag[FRAG_MAX];
} FragList;

extern FragList fraglist_data;
extern FragList *frag_list;

/* Prototypes */
s32  Frag_Init(u32 device, void *fraglist, s32 size);
void Frag_Close(void);
s32  Frag_Read(void *data, u32 len, u32 woffset);

#endif
