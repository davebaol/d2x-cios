/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2011 davebaol, oggzee.
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
#include "isfs.h"
#include "syscalls.h"
#include "types.h"
#include "config.h"
#include "frag.h"

/* Constants */
#define FILENAME	"/sys/dip.cfg"


s32 __Config_Create(void)
{
	s32 ret;

	/* Open ISFS */
	ret = ISFS_Open();
	if (ret < 0)
		return ret;

	/* Create file */
	ret = ISFS_CreateFile(FILENAME);

	/* Close ISFS */
	ISFS_Close();

	return ret;
}

s32 __Config_Delete(void)
{
	s32 ret;

	/* Open ISFS */
	ret = ISFS_Open();
	if (ret < 0)
		return ret;

	/* Delete file */
	ret = ISFS_Delete(FILENAME);

	/* Close ISFS */
	ISFS_Close();

	return ret;
}

s32 Config_Load(struct dipConfigState *cfg, u32 size)
{
	s32 fd, ret, ret2;

	/* Open config file */
	fd = os_open(FILENAME, ISFS_OPEN_READ);

	if (fd < 0)
		return fd;

	/* Read config.mode */
	ret = os_read(fd, cfg, sizeof(cfg->mode));

	if (ret == sizeof(cfg->mode)) {
		size -= sizeof(cfg->mode);
		if (cfg->mode == MODE_FRAG) {
			size = sizeof(cfg->frag);
		}
		/* Read the rest of config */
		ret = os_read(fd, (void*)cfg + sizeof(cfg->mode), size);
		if (ret > 0) {
			ret += sizeof(cfg->mode);
			if (cfg->mode == MODE_FRAG) {
				/* Read frag list */
				memset(&fraglist_data, 0, sizeof(fraglist_data));
				if (cfg->frag.size > sizeof(fraglist_data)) {
					ret2 = -1;
				} else {	   
					ret2 = os_read(fd, &fraglist_data, cfg->frag.size);
				}
				if (ret2 != cfg->frag.size) ret = -1;
			}
		}
	}

	/* Close config */
	os_close(fd);

	/* Delete config file */
	__Config_Delete();

	return ret;
}

s32 Config_Save(struct dipConfigState *cfg, u32 size)
{
	s32 fd, ret, ret2;

	/* Create config file */
	__Config_Create();

	/* Open config file */
	fd = os_open(FILENAME, ISFS_OPEN_WRITE);
	if (fd < 0)
		return fd;

	/* Write config */
	ret = os_write(fd, cfg, size);

	/* Write frag list */
	if (ret > 0 && cfg->mode == MODE_FRAG) {
		ret2 = os_write(fd, &fraglist_data, cfg->frag.size);
		if (ret2 > 0) ret += ret2; else ret = ret2;
	}

	/* Close config */
	os_close(fd);

	return ret;
}
