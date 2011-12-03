/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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
#define IOCTL_DI_LOW_READ		0x71
#define IOCTL_DI_READID			0x70
#define IOCTL_DI_WAITCVRCLOSE		0x79
#define IOCTL_DI_COVER_REG		0x7A
#define IOCTL_DI_COVER_CLEAR		0x86
#define IOCTL_DI_COVER_STATUS		0x88
#define IOCTL_DI_COVER_SET		0x89
#define IOCTL_DI_RESET			0x8A
#define IOCTL_DI_UNENCREAD		0x8D
#define IOCTL_DI_STATUS_REG		0x95
#define IOCTL_DI_REPORT_KEY		0xA4
#define IOCTL_DI_READ_A8		0xA8
#define IOCTL_DI_SEEK			0xAB
#define IOCTL_DI_READ_D0		0xD0
#define IOCTL_DI_OFFSET			0xD9
#define IOCTL_DI_READBCA		0xDA
#define IOCTL_DI_REQCOVER		0xDB
#define IOCTL_DI_REQERROR 		0xE0
#define IOCTL_DI_STOP_MOTOR		0xE3
#define IOCTL_DI_AUDIO_CONFIG		0xE4

/* Custom commands */
#define IOCTL_DI_OFFSET_SET		0xF0
#define IOCTL_DI_OFFSET_GET		0xF1
#define IOCTL_DI_CRYPT_SET		0xF2
#define IOCTL_DI_CRYPT_GET		0xF3
#define IOCTL_DI_WBFS_SET		0xF4
#define IOCTL_DI_WBFS_GET		0xF5
#define IOCTL_DI_RESET_DISABLE		0xF6
#define IOCTL_DI_FILE_SET		0xF7
#define IOCTL_DI_FILE_GET		0xF8
#define IOCTL_DI_CUSTOMCMD		0xFF

#endif

