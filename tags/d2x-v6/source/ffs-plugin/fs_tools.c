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

		/* Check path */
		ret = FS_CheckPath(path);

		/* Emulate path */
		if (!ret) {
			/* Set FAT prefix */
			strcpy(newpath, "fat");

			/* Generate path */
			FS_GeneratePath(path, newpath + 3);

			/* Return path */
			return newpath;
		}
	}

	/* Return path */
	return path;
}

void __FS_CopyPath(char *dst, const char *src)
{
	u32 cnt;

	/* Move to end */
	dst += strlen(dst);

	/* Copy path */
	for (cnt = 0; src[cnt]; cnt++) {
		char c = src[cnt];

		/* Check character */
		switch (c) {
		case '"':
		case '*':
		case ':':
		case '<':
		case '>':
		case '?':
		case '|':
			/* Replace character */
			c = '_';
			break;
		}

		/* Copy character */
		dst[cnt] = c;
	}

	/* End of string */
	dst[cnt] = 0;
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

u32 FS_CheckPath(const char *path)
{
	/* Ignore 'launch.sys' */
	if (!strncmp(path, "/tmp/launch.sys", 15)) return 1;

	/* Check path */
	if (config.mode & MODE_FULL) {
		if (!strncmp(path, "/dev", 4))              return 1;
		//if (!strncmp(path, "/sys/cert.sys", 13))    return 1;
		if (!strncmp(path, "/", 1))                 return 0;
	} else {
		if (!strncmp(path, "/ticket/00010001", 16)) return 0;
		if (!strncmp(path, "/ticket/00010005", 16)) return 0;
		if (!strncmp(path, "/title/00010000",  15)) return 0;
		if (!strncmp(path, "/title/00010001",  15)) return 0;
		if (!strncmp(path, "/title/00010004",  15)) return 0;
		if (!strncmp(path, "/title/00010005",  15)) return 0;
		if (!strncmp(path, "/tmp", 4))              return 0;
		if (!strncmp(path, "/sys/disc.sys", 13))    return 0;
		if (!strncmp(path, "/sys/uid.sys",  12))    return 0;
	}

	return 1;
}

void FS_GeneratePath(const char *oldpath, char *newpath)
{
	/* Generate device */
	FS_GenerateDevice(newpath);

	/* Append nand path */
	__FS_CopyPath(newpath, config.path);

	/* Append required path */
	__FS_CopyPath(newpath, oldpath);
}

void FS_GenerateDevice(char *path)
{
	u8 device = (config.mode & 0xFF);

	/* Set prefix */
	switch (device) {
	case MODE_SDHC:
		strcpy(path, "0:");
		break;

	case MODE_USB:
		strcpy(path, "1:");
		break;

	default:
		strcpy(path, "");
	}
}
