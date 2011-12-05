/*
 * ES plugin for Custom IOS.
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
#include "syscalls.h"
#include "types.h"
#include "es_calls.h"

/* Constants */
#define FILENAME	"/sys/esp.cfg"


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

// NOTE:
// This function is called by the main before patching Nintendo's ES module. 
// Since this code is not running in ES thread don't use ES_printf for 
// debugging purpose. Use svc_write instead.
s32 Config_Load(void *cfg, u32 size)
{
	s32 fd, ret;

#ifdef DEBUG
	svc_write("ESP: Config_Load(): Loading config file "FILENAME"\n");
#endif

	/* Open config file */
	fd = os_open(FILENAME, ISFS_OPEN_READ);
#ifdef DEBUG
	svc_write("ESP: Config_Load(): Config file ");svc_write(fd<0? "NOT found\n": "found\n");
#endif
	if (fd < 0)
		return fd;

	/* Read config */
	ret = os_read(fd, cfg, size);

	/* Close config */
	os_close(fd);

	/* Delete config file */
	__Config_Delete();

	return ret;
}

s32 Config_Save(void *cfg, u32 size)
{
	s32 fd, ret;

	/* Create config file */
	__Config_Create();

	/* Open config file */
	fd = os_open(FILENAME, ISFS_OPEN_WRITE);
	if (fd < 0)
		return fd;

	/* Write config */
	ret = os_write(fd, cfg, size);

	/* Close config */
	os_close(fd);

	return ret;
}                   