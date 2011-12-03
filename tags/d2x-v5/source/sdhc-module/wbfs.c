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

#include <stdio.h>
#include <string.h>

#include "sdio.h"
#include "syscalls.h"
#include "libwbfs/libwbfs.h"

/* Constants */
#define SECTOR_SIZE	0x200

/* Variables */
static wbfs_t      *hdd  = NULL;
static wbfs_disc_t *disc = NULL;


static int __WBFS_ReadSector(void *cbdata, u32 lba, u32 count, void *buffer)
{
	s32 ret;

	/* Read data */
	ret = sdio_ReadSectors(lba, count, buffer);
	if (!ret)
		return 1;

	/* Invalidate range */
	os_sync_before_read(buffer, SECTOR_SIZE * count);

	return 0;
}


s32 WBFS_OpenDisc(u8 *discid)
{
	s32 ret;

	/* Initialize SDIO */
	ret = sdio_Startup();
	if (!ret)
		return 1;

	/* Close disc */
	if (disc)
		wbfs_close_disc(disc);

	/* Close SD */
	if (hdd)
		wbfs_close(hdd);

	/* Open SD */
	hdd = wbfs_open_hd(__WBFS_ReadSector, NULL, NULL, SECTOR_SIZE, 0, 0);
	if (!hdd)
		return 2;

	/* Open disc */
	disc = wbfs_open_disc(hdd, discid);
	if (!disc)
		return 3;

	return 0;
}

s32 WBFS_Read(void *buffer, u32 len, u32 offset)
{
	/* No disc opened */
	if (!disc)
		return 1;

	/* Disc read */
	return wbfs_disc_read(disc, offset, buffer, len);
}
