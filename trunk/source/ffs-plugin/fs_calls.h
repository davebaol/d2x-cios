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

#ifndef _FS_CALLS_H_
#define _FS_CALLS_H_

//#define DEBUG

#ifndef _FS_CALLS_S_

#include "types.h"

/* Debug */
#ifdef DEBUG
void FS_printf(const char *fmt, ...);
#else
#define FS_printf(fmt, ...)
#endif

/* FFS handlers */
s32 fs_unk   (void *data);
s32 fs_open  (void *data);
s32 fs_close (void *data);
s32 fs_read  (void *data);
s32 fs_write (void *data);
s32 fs_seek  (void *data);
s32 fs_ioctl (void *data);
s32 fs_ioctlv(void *data);

/* Syscall open hook */
s32 syscall_open(char *path, s32 mode);

#endif
#endif
