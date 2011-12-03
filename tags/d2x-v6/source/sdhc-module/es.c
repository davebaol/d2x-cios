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

#include "syscalls.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_ES_GETTITLEID	0x20

/* Variables */
static ioctlv vector[8] ATTRIBUTE_ALIGN(32);


s32 __ES_Init(void)
{
	/* Open /dev/es */
	return os_open("/dev/es", 0);
}

void __ES_Close(s32 fd)
{
	/* Close /dev/es */
	os_close(fd);
}

s32 ES_GetTitleID(u64 *tid)
{
	s32 fd, ret;

	/* Open ES */
	fd = __ES_Init();
	if (fd < 0)
		return fd;

	/* Setup vector */
	vector[0].data = tid;
	vector[0].len  = sizeof(u64);

	/* Get title ID */
	ret = os_ioctlv(fd, IOCTL_ES_GETTITLEID, 0, 1, vector);

	/* Close ES */
	__ES_Close(fd);

	return ret;
}
