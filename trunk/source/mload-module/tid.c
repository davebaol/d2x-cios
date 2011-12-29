/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.
	Copyright (C) 2010 davebaol.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "module.h"
#include "types.h"

/* Constants */
#define MAX_NUM_TIDS   8

typedef struct {
	s32 tid;
	u32 rights;
} TID;

/* Static variables */
static u32 num_tids = 0;
static TID tids[MAX_NUM_TIDS];

s32 TID_AddRights(s32 tid, u32 rights)
{
	s32 cnt;

	if (tid < 0) {
		/* Error: invalid tid */
		return -1;
	}

	for (cnt = 0; cnt < num_tids; cnt++) {
		if (tids[cnt].tid == tid) {
			/* Tid already registered, add rights */
			tids[cnt].rights |= rights;

			/* Success: already registered */
			return 0;
		}
	}

	/* Register new tid */
	if (num_tids < MAX_NUM_TIDS) {
		/* Set rights */
		tids[num_tids].tid    = tid;
		tids[num_tids].rights = rights;

		/* Increment counter */
		num_tids++;

		/* Success: just registered */
		return 0;
	}

	/* Error: Too many tids registered */
	return -2;
}

s32 TID_CheckRights(s32 tid, u32 rights)
{
	s32 cnt;

	/* Always authorized access when stealth mode is off */
	if (!stealth_mode)
		return 1;

	/* Check thread rights */
	for (cnt = 0; cnt < num_tids; cnt++) {
		if (tids[cnt].tid == tid) {
			/* Tid found, check rights */
			return (tids[cnt].rights & rights) != 0;
		}
	}

	/* Unknown tid */
	return 0;
}

