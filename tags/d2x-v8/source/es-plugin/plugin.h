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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "ipc.h"
#include "types.h"


/* Config structure */
struct esConfig {
	/* Fake launch mode */
	u32 fakelaunch;

	/* ios to be launched in place of the requested one */
	u32 ios;

	/* title_id of the game requesting ios reload */
	u64 title_id;

	/* title_id to be launched in place of the system menu */
	u64 sm_title_id;
};

/* Prototypes */
s32 ES_EmulateOpen(ipcmessage *message);
s32 ES_EmulateIoctlv(ipcmessage *message);

/* Extern */
extern struct esConfig config;

#endif
