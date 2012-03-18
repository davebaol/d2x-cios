/*
 * FFS plugin for Custom IOS.
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

#include <string.h>

#include "ipc.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "types.h"

s32 SDI_CheckSlot(const char *path, const char *slot, u32 n)
{
	s32 ret;

	/* Check official slot name */
	ret = strncmp(path, slot, n);

	/* Official slot name not matched */
	if (ret) {
		/* Check alternative slot name without the initial slash */
		ret = strncmp(path, slot + 1, n - 1);

		/* Alternative slot name matched */
		if(!ret) {
			/* Alternative name is hidden when a title is running */
			if (Swi_GetRunningTitle()) {
				svc_write("SDI: Title identified. Blocking opening request for custom path ");
				svc_write(path);
				svc_write("\n");
				return IPC_ENOENT;
			}
		}
	}

	return ret;
}
