/*
 * ES plugin for Custom IOS.
 *
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

#include "syscalls.h"
#include "types.h"

#define IOCTL_DI_SAVE_CONFIG	0xFE

static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);

s32 DI_Config_Save(void)
{
	s32 fd, ret;

	/* Open /dev/di */
	fd = os_open("/dev/di", 0);
	if (fd < 0)
		return fd;

	/* Save config */
	inbuf[0] = IOCTL_DI_SAVE_CONFIG << 24;
	ret = os_ioctl(fd, IOCTL_DI_SAVE_CONFIG, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));

	/* Close /dev/di */
	os_close(fd);

	return ret;
}
                   