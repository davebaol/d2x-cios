/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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

#include <stdio.h>

#include "config.h"
#include "ipc.h"
#include "main.h"
#include "patches.h"
#include "plugin.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "tools.h"
#include "types.h"

/* IOS information */
iosInfo ios = { 0 };


s32 __ES_System(u32 arg1, u32 arg2)
{
	u32 perms;

	/* Invalidate cache */
	ICInvalidate();

	/* Apply permissions */
	perms = Perms_Read();
	Perms_Write(0xFFFFFFFF);

	/* Patch modules */
	Patch_EsModule(ios.esVersion);

	/* Restore permissions */
	Perms_Write(perms);

	return 0;
}


int main(void)
{
	/* Print info */
	write("$IOSVersion: ESP: " __DATE__ " " __TIME__ " 64M$\n");

	/* Load config */
	Config_Load(&config, sizeof(config));

	/* Get IOS info */
	Swi_GetIosInfo(&ios);

	/* Initialize plugin */
	Swi_CallFunc((void *)__ES_System, NULL, NULL);

	return 0;
}
