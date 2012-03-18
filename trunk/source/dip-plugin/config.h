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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "isfs.h"
#include "plugin.h"
#include "types.h"

/* Prototypes */
s32 DI_Config_Load(struct dipConfigState *cfg);
s32 DI_Config_Save(struct dipConfigState *cfg);
s32 FFS_Config_Load(fsconfig *cfg);
s32 FFS_Config_Save(fsconfig *cfg);

#endif

