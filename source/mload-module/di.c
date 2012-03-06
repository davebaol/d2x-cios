/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.

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

#include "syscalls.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_DI_READID		0x70
#define IOCTL_DI_RESET		0x8A

/* Variables */
static u32 inbuf[8] ATTRIBUTE_ALIGN(32);


static s32 __DI_Init(void)
{
	/* Open /dev/di */
	return os_open("/dev/di", 0);
}

static void __DI_Close(s32 fd)
{
	/* Close /dev/di */
	os_close(fd);
}


s32 DI_Reset(void)
{
	s32 fd, ret;

	/* Open DI */
	fd = __DI_Init();
	if (fd < 0)
		return fd;

	/* Prepare input */
	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;

	/* Reset drive */
	ret = os_ioctl(fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), NULL, 0);

	/* Close DI */
	__DI_Close(fd);

	return ret;
}

s32 DI_ReadDiskId(void *id)
{
	s32 fd, ret;

	/* Open DI */
	fd = __DI_Init();
	if (fd < 0)
		return fd;

	/* Prepare input */
	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;

	/* Reset drive */
	ret = os_ioctl(fd, IOCTL_DI_READID, inbuf, sizeof(inbuf), id, 32);

	/* Close DI */
	__DI_Close(fd);

	return ret;
}
