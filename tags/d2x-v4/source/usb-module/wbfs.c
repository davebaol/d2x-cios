/*   
	Custom IOS Module (USB)

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

#include "syscalls.h"
#include "usbglue.h"
#include "libwbfs/libwbfs.h"

/* Variables */
static wbfs_t      *hdd  = NULL;
static wbfs_disc_t *disc = NULL;

static u32 sectorSz = 0;
static u32 nbSector = 0;


static int __WBFS_ReadSector(void *cbdata, u32 lba, u32 count, void *buffer)
{
	s32 ret;

	/* Read data */
	ret = usbstorage_ReadSectors(lba, count, buffer);
	if (!ret)
		return 1;

	/* Invalidate range */
	os_sync_before_read(buffer, sectorSz * count);

	return 0;
}


s32 WBFS_OpenDisc(u8 *discid)
{
	s32 ret;

	/* Initialize USB */
	ret = usbstorage_Startup();
	if (!ret)
		return 1;

	/* Read capacity */
	ret = usbstorage_ReadCapacity(&sectorSz, &nbSector);
	if (!ret)
		return 2;

	/* Close disc */
	if (disc)
		wbfs_close_disc(disc);

	/* Close SD */
	if (hdd)
		wbfs_close(hdd);

	/* Open SD */
	hdd = wbfs_open_hd(__WBFS_ReadSector, NULL, NULL, sectorSz, 0, 0);
	if (!hdd)
		return 3;

	/* Open disc */
	disc = wbfs_open_disc(hdd, discid);
	if (!disc)
		return 4;

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
