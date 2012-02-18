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

#ifndef _FS_TOOLS_H_
#define _FS_TOOLS_H_

#include "types.h"


/* Prototypes */
u16  FS_GetUID(void);
u16  FS_GetGID(void);
u32  FS_CheckRealPath(const char *path);
u32  FS_MatchPath(char *path, const char *pattern, s32 strict);
void FS_GenerateAbsolutePath(const char *realpath, char *emupath);
s32  FS_GenerateRelativePath(const char *realpath, char *emupath);

#endif
