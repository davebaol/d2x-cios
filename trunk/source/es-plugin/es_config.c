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
#include "isfs.h"
#include "plugin.h"
#include "syscalls.h"
#include "types.h"

/* Constants */
static const char *filename = "/sys/esp.cfg";


// NOTE:
// This function is called by the main before patching Nintendo's ES module. 
// Since this code is not running in ES thread don't use ES_printf for 
// debugging purpose. Use svc_write instead.
s32 ES_LoadConfig(void)
{
	return Config_Load(filename, &config, sizeof(config));
}

s32 ES_SaveConfig(void)
{
	return Config_Save(filename, &config, sizeof(config));
}                   