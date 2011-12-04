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

#include "fs_tools.h"
#include "fat.h"
#include "plugin.h"
#include "types.h"

#define FAT_DEVICE       "fat"
#define FAT_DEVICE_LEN   3

char *__FS_SyscallOpen(char *path, s32 mode)
{
	static char newpath[FAT_MAXPATH] ATTRIBUTE_ALIGN(32);

	/* Emulation mode */
	if (config.mode) {
		u32 ret;

		/* SDHC mode */
		if (config.mode & MODE_SDHC) {
			if (!strcmp(path, "/dev/sdio")) {
				/* Replace path */
				strcpy(newpath, "/dev/null");

				/* Return path */
				return newpath;
			}
		}

		/* Direct FAT mode */
		if (!(config.mode & MODE_REV17)) {
			/* Check path */
			ret = FS_CheckRealPath(path);

			/* Emulate path */
			if (!ret) {
				/* Generate path */
				FS_GeneratePathWithPrefix(path, newpath);

				/* Return path */
				return newpath;
			}
		}
	}

	/* Return original path */
	return path;
}

void __FS_CopyPath(char *dst, const char *src)
{
	char c;

	/* Escape invalid FAT characters */
	while ((c = *(src++)) != '\0') {
		char *esc;

		/* Check character */
		switch (c) {
		case '"': esc = "&qt;"; break;   // Escape double quote
		case '*': esc = "&st;"; break;   // Escape star
		case ':': esc = "&cl;"; break;   // Escape colon
		case '<': esc = "&lt;"; break;   // Escape lesser than
		case '>': esc = "&gt;"; break;   // Escape greater than
		case '?': esc = "&qm;"; break;   // Escape question mark
		case '|': esc = "&vb;"; break;   // Escape vertical bar
		default: *(dst++) = c; continue; // Copy valid FAT character
		}

		/* Replace escape sequence */
		strcpy(dst, esc);
		dst += 4;
	}

	/* End of string */
	*dst = '\0';
}


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

		/* Never emulate '/tmp/launch.sys' */
		if (!strcmp(path, "/tmp/launch.sys")) return 1;

		/* Full emulation */
		if (config.mode & MODE_FULL) {
	
			/* Don't emulate paths starting with '/dev', i.e. virtual devices */
			if (!strncmp(path, "/dev", 4)) return 1;
		
			//if (!strncmp(path, "/sys/cert.sys", 13))    return 1;
		
			/* Emulate paths starting with '/', remaining ones are real */
			return *path != '/';
		}

		/* Partial emulation */

		if (!strncmp(path, "/ticket/00010001", 16)) return 0;
		if (!strncmp(path, "/ticket/00010005", 16)) return 0;
		if (!strncmp(path, "/title/00010000",  15)) return 0;
		if (!strncmp(path, "/title/00010001",  15)) return 0;
		if (!strncmp(path, "/title/00010004",  15)) return 0;
		if (!strncmp(path, "/title/00010005",  15)) return 0;
		if (!strncmp(path, "/tmp", 4))              return 0;
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

void FS_GeneratePath(const char *fspath, char *fatpath)
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

	/* Append and escape required path */
	__FS_CopyPath(fatpath + config.pathlen, fspath);
}

s32 FS_GeneratePathWithPrefix(const char *fspath, char *fatpath)
{
	/* Set FAT prefix */
	strcpy(fatpath, FAT_DEVICE);

	/* Generate path */
	FS_GeneratePath(fspath, fatpath + FAT_DEVICE_LEN);

	/* Return prefix length */
	return FAT_DEVICE_LEN;
}
