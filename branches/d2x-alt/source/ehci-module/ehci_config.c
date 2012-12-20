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

#include "config.h"
#include "ehci_config.h"
#include "isfs.h"
#include "syscalls.h"
#include "types.h"


ehciConfig config = { 0, 10 * USECS_PER_SEC };

static const char *filename = "/sys/ehci.cfg";

s32 EHCI_LoadConfig(void)
{
	return Config_Load(filename, &config, sizeof(config));
}

s32 EHCI_SaveConfig(void)
{
	return Config_Save(filename, &config, sizeof(config));
}

s32 EHCI_DeleteConfig(void)
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
