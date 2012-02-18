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

#include "fat_tools.h"
#include "fs_tools.h"
#include "fat.h"
#include "plugin.h"
#include "types.h"


u16 FS_GetUID(void)
{
	/* Return user ID */
	return 0x1001;
}

u16 FS_GetGID(void)
{
	u16 *tid = (u16 *)0x04;

	/* Return title ID */
	return *tid;
}

u32 FS_CheckRealPath(const char *path)
{
	/* Emulation is ON */
	if (config.mode) {

		/* Never emulate '/tmp/launch.sys' (used by ES_LAUNCHTITLE) */
		if (!strcmp(path, "/tmp/launch.sys")) return 1;

		/*
		 * Full emulation
		 */
		if (config.mode & MODE_FULL) {
	
			/* Don't emulate paths starting with '/dev', i.e. virtual devices */
			if (!strncmp(path, "/dev", 4)) return 1;
		
			//if (!strncmp(path, "/sys/cert.sys", 13))    return 1;
		
			/* Emulate paths starting with '/', remaining ones are real */
			return *path != '/';
		}

		/*
		 * Partial emulation
		 */

		/* Check /ticket paths */
		if (!strncmp(path, "/ticket/", 8)) {
			const char *p = path + 8;
			if (!strncmp(p, "00010001", 8)) return 0;
			if (!strncmp(p, "00010005", 8)) return 0;
			return 1;
		}

		/* Check /title paths */
		if (!strncmp(path, "/title/", 7)) {
			const char *p = path + 7;
			if (!strncmp(p, "00010000", 8)) return 0;
			if (!strncmp(p, "00010001", 8)) return 0;
			if (!strncmp(p, "00010004", 8)) return 0;
			if (!strncmp(p, "00010005", 8)) return 0;
			return 1;
		}

		/* Check /tmp paths */
		if (!strncmp(path, "/tmp", 4)) {
			const char *p = path + 4;
			/* Fix issue 14.
			 * We have to use /tmp/uid.sys on real nand because
			 * /sys/uid.sys is no more emulated, see system file
			 * check below.
			 * This way FS_Rename("/tmp/uid.sys","/sys/uid.sys")
			 * called by ES will work. In fact file renaming
			 * between emulated and real paths is not supported.
			 */
			if (!strcmp(p, "/uid.sys")) return 1;
			return 0;
		}

		/* Check /import path which is used by ES during title install */
		if (!strncmp(path, "/import", 7))           return 0;

		/* Check some system files used by ES */
		if (!strcmp(path, "/sys/disc.sys"))         return 0;
//		if (!strcmp(path, "/sys/uid.sys"))          return 0;
	}

	/* All unmatched paths are on real nand */
	return 1;
}

u32 FS_MatchPath(char *path, const char *pattern, s32 strict)
{
	while (*path != 0 || *pattern != 0) {
		if (*path == 0) return 0;
		if (*pattern == 0) return !strict;
		if (*pattern != '#' && *path != *pattern) return 0;
		path++;
		pattern++;
	}

	return 1;
}

/*
 * NOTE:
 * - Absolute paths are typically used to work on FAT files through ioctl commands.
 * - Only the invalid FAT characters after the nand folder are escaped.
 */
void FS_GenerateAbsolutePath(const char *fspath, char *fatpath)
{
	u32 device = config.mode & (MODE_SDHC | MODE_USB);

	/* Copy device prefix */
	switch (device) {
	case MODE_SDHC:
		strcpy(fatpath, "0:");
		fatpath += 2;
		break;

	case MODE_USB:
		strcpy(fatpath, "1:");
		fatpath += 2;
		break;
	}

	/* Append nand folder */
	strcpy(fatpath, config.path);

	/* Append escaped path */
	FAT_Escape(fatpath + config.pathlen, fspath);
}

/*
 * NOTES:
 * - Relative paths can be used used only to open FAT files inside the nand folder.
 * - Relative paths always start with '$' which must NOT be followed by '/'.
 * - Invalid fat characters inside a relative path are NOT escaped.
 * - Cunnings above allow you not to waste any character of the 64 allowed
 *   by IOS on opening requests.
 */
s32 FS_GenerateRelativePath(const char *fspath, char *fatpath)
{
	if (*fspath != '/')
		return 0;

	/* Copy path */
	strcpy(fatpath, fspath);

	/* Replace first char */
	*fatpath = '$';

	return 1;
}
