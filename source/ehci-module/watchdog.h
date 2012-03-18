/*
 * EHCI plugin for Custom IOS.
 *
 * Copyright (C) 2009 Hermes.
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

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include "types.h"

/* Prototypes */
s32  WATCHDOG_CreateTimer(s32 queueId, u32 message);
void WATCHDOG_StopTimer(void);
void WATCHDOG_RestartTimer(void);
void WATCHDOG_Run(void);

#endif
