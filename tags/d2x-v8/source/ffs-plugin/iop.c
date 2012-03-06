/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
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

#include "direct_syscalls.h"
#include "iop_calls.h"
#include "fs_tools.h"
#include "plugin.h"
#include "stealth.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "tools.h"
#include "types.h"

//#define IOP_TRACE_OPEN


/* Function pointer */
static TRCheckFunc CheckThreadRights = NULL;


s32 __IOP_CheckOpeningRequest(char *path, u32 rights)
{
	s32 tid, ret;

	/* Get current thread id */
	tid = direct_os_get_thread_id();

	/* Check thread rights */
	ret = CheckThreadRights(tid, rights);

	/* Log unauthorized request */
	if (!ret) {
		svc_write("IOP: Unauthorized access to '");
		svc_write(path);
		svc_write("'. Blocking opening request.\n");
	}

	return ret;
}

void IOP_Init()
{
	/* Set thread rights check function */
	CheckThreadRights = Swi_GetThreadRightsCheckFunc();
}

char *__IOP_SyscallOpen(char *path, s32 mode)
{
	static char newpath[FAT_MAXPATH] ATTRIBUTE_ALIGN(32);

	s32 ret;

#ifdef IOP_TRACE_OPEN
	svc_write("IOP: open ");svc_write(path);svc_write("\n");
#endif

	/*
	 * Paths starting with '#' are always sent to real nand.
	 * This is an internal feature, only authorized threads 
	 * can use it.
	 */
	if (*path == '#') {
		/* Check opening request */
		ret = __IOP_CheckOpeningRequest(path, TID_RIGHTS_FORCE_REAL_NAND);

		/* Block request returning original invalid path */
		if (!ret)
			return path;

		/* Copy path */
		strcpy(newpath, path);

		/* Replace first character */
		*newpath = '/';

		/* Set flag for mode rev17-like */
		forceRealPath = ((config.mode & MODE_REV17) != 0);

		/* Return new path */
		return newpath;
	}

	/*
	 * When a title is running only authorized threads 
	 * can open fat.
	 *
	 * NOTE:
	 * Since v8 beta r42 relative fat paths, identified by the initial 
	 * '$', have been introduced to fix issue 16.
	 * These paths are always relative to nand emulation folder.
	 */
	if (*path == '$' || !strncmp(path, "fat", 3)) {
		u32 running_title;
		
		/* Check a title is running */
		running_title = Swi_GetRunningTitle();

		/* Check thread rigths if a title is running */
		if (running_title) {
			/* Check opening request */
			ret = __IOP_CheckOpeningRequest(path, TID_RIGHTS_OPEN_FAT);

			/* Block opening request */
			if (!ret) {
				/* Set new invalid path */
				strcpy(newpath, "GTFO");

				/* Return new path */
				return newpath;
			}
		}

		/* Return original path */
		return path;
	}

	/* Emulation mode */
	if (config.mode) {

		/* SDHC mode */
		if (config.mode & MODE_SDHC) {
			if (!strcmp(path, "/dev/sdio/slot0")) {

				/* Log */
				svc_write("IOP: The game is trying to open /dev/sdio/slot0.\n");
				svc_write("IOP: To avoid interferences with emu nand on sd card the request is redirected to /dev/null.\n");

				/* Replace path */
				strcpy(newpath, "/dev/null");

				/* Return new path */
				return newpath;
			}
		}

		/* Direct FAT mode */
		if (!(config.mode & MODE_REV17)) {
			/* Check path */
			ret = FS_CheckRealPath(path);

			/* Emulate path */
			if (!ret) {
				/* Generate relative path */
				ret = FS_GenerateRelativePath(path, newpath);

				/* Return new path */
				if (ret)
					return newpath;
			}
		}
	}

	/* Return original path */
	return path;
}
