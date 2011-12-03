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

#ifndef _IOCTL_H_
#define _IOCTL_H_

/* IOCTL commands */
#define IOCTL_ISFS_FORMAT		1
#define IOCTL_ISFS_GETSTATS		2
#define IOCTL_ISFS_CREATEDIR		3
#define IOCTL_ISFS_READDIR		4
#define IOCTL_ISFS_SETATTR		5
#define IOCTL_ISFS_GETATTR		6
#define IOCTL_ISFS_DELETE		7
#define IOCTL_ISFS_RENAME		8
#define IOCTL_ISFS_CREATEFILE		9
#define IOCTL_ISFS_SETFILEVERCTRL	10
#define IOCTL_ISFS_GETFILESTATS		11
#define IOCTL_ISFS_GETUSAGE		12
#define IOCTL_ISFS_SHUTDOWN		13

/* Custom commands */
#define IOCTL_ISFS_SETMODE		100

#endif

