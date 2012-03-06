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

#include <string.h>

#include "dip_calls.h"
#include "syscalls.h"
#include "usbstorage.h"
#include "sdhc.h"
#include "frag.h"

#define DEV_NONE 0
#define DEV_USB  1
#define DEV_SDHC 2

#define MAX_IDX      640 // 640*16MB = 10GB
#define IDX_CHUNK  32768 // 16MB in sectors units: 16*1024*1024/512
#define IDX_SHIFT     15 // 1<<15 = 32768

#define SECTOR_SIZE(dev)		((dev)==DEV_SDHC ? 512 :	\
	((dev)==DEV_USB ? usbstorage_GetSectorSize() : 0))

/* Static variables */
static u16 frag_idx[MAX_IDX] = { 0 };
static u32 frag_inited       = 0;
static u32 frag_dev          = 0;
static u32 sector_size       = 0;
static u32 ss_num_bits       = 0;
static u8* sector_buf        = NULL;

/* Global variables */
FragList fraglist_data = { 0 };
FragList *frag_list    = NULL;

static u32 __Frag_GetNumBits(u32 value) 
{
	u32 nb = 0;
	do {
		value >>= 1;
		nb++;
	} while (value);

	return nb;
}

static void __Frag_OptimizeList(void)
{
	s32 i;
	u32 off;
	u16 idx = 0;
	for (i = 0; i < MAX_IDX; i++) {
		off = i << IDX_SHIFT;
		while ((off >= frag_list->frag[idx].offset + frag_list->frag[idx].count)
				&& (idx + 1 < frag_list->num))
		{
			idx++;
		}
		frag_idx[i] = idx;
	}
}

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
static s32 __Frag_Get(FragList *ff, u32 offset, u32 count,
		u32 *poffset, u32 *psector, u32 *pcount)
{
	u32 delta;
	u32 idx_off;
	u32 start_idx;
	s32 i;
	
	// optimize seek inside frag list
	// jump to a precalculated index
	idx_off = offset >> IDX_SHIFT;
	if (idx_off > MAX_IDX)
		idx_off = MAX_IDX - 1;

	start_idx = frag_idx[idx_off];

	for (i = start_idx; i < ff->num; i++) {
		if (ff->frag[i].offset <= offset
			&& ff->frag[i].offset + ff->frag[i].count > offset)
		{
			delta    = offset - ff->frag[i].offset;
			*poffset = offset;
			*psector = ff->frag[i].sector + delta;
			*pcount  = ff->frag[i].count - delta;
			if (*pcount > count)
				*pcount = count;
			goto out;
		}
		if (ff->frag[i].offset > offset
			&& ff->frag[i].offset < offset + count)
		{
			delta    = ff->frag[i].offset - offset;
			*poffset = ff->frag[i].offset;
			*psector = ff->frag[i].sector;
			*pcount  = ff->frag[i].count;
			count   -= delta;
			if (*pcount > count)
				*pcount = count;
			goto out;
		}
	}

	/* Not found */
	if (offset + count > ff->size) {
		/* Error: out of range! */
		return -2;
	}

	// if inside range, then it must be just sparse, zero filled
	// return empty block at the end of requested
	*poffset = offset + count;
	*psector = 0;
	*pcount  = 0;

	out:

	/* Success */
	return 0;
}

// offset and len in sectors
static s32 __Frag_ReadSect(u32 offset, u8 *data, u32 len)
{
	u32 off_ret;
	u32 sector;
	u32 count;
	u32 delta;
	s32 ret;

	while (len) {
		ret = __Frag_Get(frag_list, offset, len, &off_ret, &sector, &count);

	        /* Error */
		if (ret)
			return ret;

		delta = off_ret - offset;
		if (delta) {
			/* Sparse block, fill with 0 */
			memset(data, 0, delta << ss_num_bits);
			offset += delta;
			data   += delta << ss_num_bits;
			len    -= delta;
		}

		if (count) {
			/* Read sectors */
			if (frag_dev == DEV_USB)
				ret = __usbstorage_Read_Write(0, sector, count, data);
			else
				ret = sdhc_Read(sector, count, data);

			/* Read error */
			if (!ret)
				return -3;

			offset += count;
			data   += count << ss_num_bits;
			len    -= count;
		}

		/* Should never happen */
		if (delta + count == 0)
			return -4;
	}

	/* Success */
	return 0;
}

// offset is pointing 32bit words to address the whole dvd, len is in bytes
static s32 __Frag_ReadPartial(u32 offset, u8 *data, u32 len, u32 *read_len)
{
	u32 off_sec;
	u32 mod;
	u32 rlen;
	s32 ret;

	/* Word to sect */
	off_sec = offset >> (ss_num_bits-2);

	/* Offset from start of sector in bytes */
	mod = (offset & ((sector_size-1) >> 2)) << 2;

	/* Remaining len from mod to end of sector */
	rlen = sector_size - mod;

	if (rlen > len)
		rlen = len;

	/* Don't read whole sectors */
	if (rlen == sector_size)
		rlen = 0;

	if (rlen) {
		ret = __Frag_ReadSect(off_sec, sector_buf, 1);
		if (ret)
			return ret;

		memcpy(data, sector_buf + mod, rlen);
	}

	*read_len = rlen;

	/* Success */
	return 0;
}

s32 Frag_Init(u32 device, void *fraglist, s32 size)
{
	s32 ret;

	/* Close previous fraglist */
	if (frag_inited)
		Frag_Close();

	/* Check device */
	if (device != DEV_USB && device != DEV_SDHC)
		return -1;

	/* Check empty fraglist */
	if (!size)
		return -2;

	/* Check max size */
	if (size > sizeof(FragList))
		return -3;

	/* Init device */
	ret = (device == DEV_USB ? usbstorage_Init() : sdhc_Init());
	if (!ret)
		return -4; 

	/* Retrieve device sector size */
	sector_size = SECTOR_SIZE(device);

	/* Number of bits required by sector size */
	ss_num_bits = __Frag_GetNumBits(sector_size - 1);

	/* Allocate sector buffer */
	sector_buf = DI_Alloc(sector_size, 32);
	if (!sector_buf)
		return -5;

	/* Copy data */
	frag_dev = device;
	frag_list = &fraglist_data;
	if (fraglist != frag_list) {
		os_sync_before_read(fraglist, size);
		memset(frag_list, 0, sizeof(FragList));
		memcpy(frag_list, fraglist, size);
		os_sync_after_write(fraglist, size);
	}

	/* Optimize fraglist access */
	__Frag_OptimizeList();

	/* Set to initialized */
	frag_inited = 1;

	/* Success */
	return 0;
}

void Frag_Close(void)
{
	/* Reset data */
	frag_inited = 0;
	frag_list   = 0;
	frag_dev    = 0;

	/* Free sector buffer */
	if (sector_buf) {
		DI_Free(sector_buf);
		sector_buf = NULL;
	}
}

// woffset is pointing 32bit words to address the whole dvd, len is in bytes
s32 Frag_Read(void *data, u32 len, u32 woffset)
{
	u32 rlen;
	u32 off_sec;
	u32 len_sec;
	s32 ret;

	/* Read leading partial non sector aligned data */
	ret = __Frag_ReadPartial(woffset, data, len, &rlen);
	if (ret)
		return ret;

	woffset += rlen >> 2;
	data    += rlen;
	len     -= rlen; 
	if (len >= sector_size) {
		/* Read sector aligned data */
		off_sec = woffset >> (ss_num_bits-2); // word to sect
		len_sec = len >> ss_num_bits;    // byte to sect
		ret = __Frag_ReadSect(off_sec, data, len_sec);
		if (ret)
			return ret;
		woffset += len_sec << (ss_num_bits-2);
		data    += len_sec << ss_num_bits;
		len     -= len_sec << ss_num_bits;
	}

	/* Read trailing partial non sector aligned data */
	if (len) {
		ret = __Frag_ReadPartial(woffset, data, len, &rlen);
		if (ret)
			return ret;
		len -= rlen;
	}

	/* Should never happen */
	if (len)
		return -5;

	/* Success */
	return 0;
}

