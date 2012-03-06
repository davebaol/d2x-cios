/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.
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

#include "syscalls.h"
#include "types.h"

/* IOCTL commands */
#define IOCTL_ES_LAUNCHMIOS	0x1337


s32 ES_LaunchMIOS(void)
{
	s32 fd, ret;

	/* Open ES */
	fd = os_open("/dev/es", 0);
	if (fd < 0)
		return fd;

	/* Call IOCTLV */
	ret = os_ioctlv(fd, IOCTL_ES_LAUNCHMIOS, 0, 0, NULL);

	/* Close ES */
	os_close(fd);

	return ret;
}
