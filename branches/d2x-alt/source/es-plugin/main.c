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

#include "es_config.h"
#include "ios.h"
#include "ipc.h"
#include "patches.h"
#include "plugin.h"
#include "syscalls.h"
#include "types.h"

char *moduleName = "ES";

int main(void)
{
	/* System patchers */
	static patcher patchers[] = {{Patch_EsModule, 0}};

	s32 fd;

	/* Print info */
	svc_write("$IOSVersion: ESP: " __DATE__ " " __TIME__ " 64M$\n");

	/* Load config */
	ES_LoadConfig();

	/* Initialize plugin */
	IOS_InitSystem(patchers, sizeof(patchers));

	/* Open ES to initialize 2nd stage */
	fd = os_open("/dev/es", 0);
	if (fd < 0)
		svc_write("ESP: main: 2nd stage init failed.\n");
	else
		os_close(fd);

	return 0;
}
