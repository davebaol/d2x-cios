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
#define DIP_CFG_FILENAME	"/sys/dip.cfg"
#define FFS_CFG_FILENAME	"/sys/ffs.cfg"


s32 __Config_Create(char *filename)
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

s32 __Config_Delete(char *filename)
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

s32 DI_Config_Load(struct dipConfigState *cfg)
{
	s32 fd, ret;

#ifdef DEBUG
	svc_write("DIP: Config_Load(): Loading config file "DIP_CFG_FILENAME"\n");
#endif

	/* Open config file */
	fd = os_open(DIP_CFG_FILENAME, ISFS_OPEN_READ);

#ifdef DEBUG
	svc_write("DIP: Config_Load(): Config file ");svc_write(fd<0? "NOT found\n": "found\n");
#endif

	if (fd < 0)
		return fd;

	/* Read config */
	ret = os_read(fd, cfg, sizeof(struct dipConfigState));

	if (ret == sizeof(struct dipConfigState)) {
		if (cfg->mode == MODE_FRAG) {
			s32 ret2;

			/* Read frag list */
			memset(&fraglist_data, 0, sizeof(fraglist_data));
			if (cfg->frag_size > sizeof(fraglist_data)) {
				ret2 = -1;
			} else {	   
				ret2 = os_read(fd, &fraglist_data, cfg->frag_size);
			}
			if (ret2 != cfg->frag_size)
				ret = -1;
		}
	}
	else if (ret >= 0)
		ret = -1;

	/* Close config */
	os_close(fd);

	/* Delete config file */
	__Config_Delete(DIP_CFG_FILENAME);

#ifdef DEBUG
	if (ret < 0)
		svc_write("DIP: Config_Load(): Config file has unexpected size!!!\n");
#endif

	return ret;
}

s32 DI_Config_Save(struct dipConfigState *cfg)
{
	s32 fd, ret;

	/* Create config file */
	__Config_Create(DIP_CFG_FILENAME);

	/* Open config file */
	fd = os_open(DIP_CFG_FILENAME, ISFS_OPEN_WRITE);
	if (fd < 0)
		return fd;

	/* Write config */
	ret = os_write(fd, cfg, sizeof(*cfg));

	/* Write frag list */
	if (ret == sizeof(*cfg)) {
		if (cfg->mode == MODE_FRAG) {
			s32 ret2 = os_write(fd, &fraglist_data, cfg->frag_size);
			if (ret2 < 0)
				ret = ret2;
			else
				ret += ret2;
		}
	}

	/* Close config */
	os_close(fd);

	return ret;
}

s32 FFS_Config_Load(struct ffsConfigState *cfg)
{
	s32 fd, ret;

#ifdef DEBUG
	svc_write("DIP: Config_Load(): Loading config file "FFS_CFG_FILENAME"\n");
#endif

	/* Open config file */
	fd = os_open(FFS_CFG_FILENAME, ISFS_OPEN_READ);

#ifdef DEBUG
	svc_write("DIP: Config_Load(): Config file ");svc_write(fd<0? "NOT found\n": "found\n");
#endif

	if (fd < 0)
		return fd;
                    
	/* Read config */
	ret = os_read(fd, cfg, sizeof(struct ffsConfigState));
	if (ret != sizeof(struct ffsConfigState)) {
#ifdef DEBUG
		svc_write("DIP: Config_Load(): Config file has unexpected size!!!\n");
#endif
		ret = -1;
	}

	/* Close config */
	os_close(fd);

	/* Delete config file */
	__Config_Delete(FFS_CFG_FILENAME);

	return ret;
}

s32 FFS_Config_Save(struct ffsConfigState *cfg)
{
	s32 fd, ret;

	/* Create config file */
	__Config_Create(FFS_CFG_FILENAME);

	/* Open config file */
	fd = os_open(FFS_CFG_FILENAME, ISFS_OPEN_WRITE);
	if (fd < 0)
		return fd;

	/* Write config */
	ret = os_write(fd, cfg, sizeof(*cfg));
	if (ret != sizeof(*cfg))
		ret = -1;

	/* Close config */
	os_close(fd);

	return ret;
}
