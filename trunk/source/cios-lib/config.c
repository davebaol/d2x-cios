/*
 * Custom IOS Library.
 *
 * Copyright (C) 2010 Waninkoko.
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

#include "isfs.h"
#include "ios.h"
#include "syscalls.h"
#include "types.h"


static s32 __Config_Create(const char *filename)
{
	s32 ret;

	/* Open ISFS */
	ret = ISFS_Open();
	if (ret < 0)
		return ret;

	/* Create file */
	ret = ISFS_CreateFile(filename);

	/* Close ISFS */
	ISFS_Close();

	return ret;
}

static s32 __Config_Delete(const char *filename)
{
	s32 ret;

	/* Open ISFS */
	ret = ISFS_Open();
	if (ret < 0)
		return ret;

	/* Delete file */
	ret = ISFS_Delete(filename);

	/* Close ISFS */
	ISFS_Close();

	return ret;
}

s32 Config_Load(const char *filename, void *cfg, u32 size)
{
	s32 fd, ret;

	/* Open config file */
	fd = os_open(filename, ISFS_OPEN_READ);

	svc_write(moduleName);
	svc_write(": Config_Load(\"");
	svc_write(filename);
	svc_write(fd < 0? "\" -> NOT found\n": "\" -> found\n");

	if (fd < 0)
		return fd;

	/* Read config */
	ret = os_read(fd, cfg, size);

	/* Close config */
	os_close(fd);

	/* Delete config file */
	__Config_Delete(filename);

	return ret;
}

s32 Config_Save(const char *filename, void *cfg, u32 size)
{
	s32 fd, ret;

	/* Create config file */
	__Config_Create(filename);

	/* Open config file */
	fd = os_open(filename, ISFS_OPEN_WRITE);
	if (fd < 0)
		return fd;

	/* Write config */
	ret = os_write(fd, cfg, size);

	/* Close config */
	os_close(fd);

	return ret;
}                   