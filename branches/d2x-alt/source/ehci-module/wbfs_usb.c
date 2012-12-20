/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
	Copyright (C) 2009 Waninkoko.
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

#include "ehci_types.h"
#include "ehci.h"
#include "syscalls.h"
#include "libwbfs/libwbfs.h"

/* Variables */
static wbfs_t      *hdd  = NULL;
static wbfs_disc_t *disc = NULL;

static u32 nbSector, sectorSize;


static int __WBFS_USB_ReadSector(void *cbdata, u32 lba, u32 count, void *buffer)
{
	s32 ret;

	/* Read data */
	ret = USBStorage_Read_Sectors(lba, count, buffer);
	if (!ret)
		return 1;

	/* Invalidate range */
	os_sync_before_read(buffer, sectorSize * count);

	return 0;
}


s32 WBFS_USB_OpenDisc(u8 *discid)
{
	s32 ret;

	/* Initialize USB storage */
	ret = USBStorage_Init();
	if (ret)
		return 1;

	/* Get sector size */
	nbSector = USBStorage_Get_Capacity(&sectorSize);
	if (!nbSector)
		return 2;

	/* Close disc */
	if (disc)
		wbfs_close_disc(disc);

	/* Close device */
	if (hdd)
		wbfs_close(hdd);

	/* Open device */
	hdd = wbfs_open_hd(__WBFS_USB_ReadSector, NULL, NULL, sectorSize, nbSector, 0);
	if (!hdd)
		return 3;

	/* Open disc */
	disc = wbfs_open_disc(hdd, discid);
	if (!disc)
		return 4;

	return 0;
}

s32 WBFS_USB_Read(void *buffer, u32 len, u32 offset)
{
	/* No disc opened */
	if (!disc)
		return 1;

	/* Disc read */
	return wbfs_disc_read(disc, offset, buffer, len);
}
