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

#ifndef _EHCI_INTERRUPT_H_
#define _EHCI_INTERRUPT_H_

#include "syscalls.h"
#include "ehci_types.h"
#include "ehci.h"

#define INTR_MASK (STS_IAA | STS_FATAL | STS_PCD | STS_ERR | STS_INT)

/* Prototypes */
s32  ehci_irq_init(void);
void ehci_irq_passive_callback( void (*callback)(u32 flags));
void ehci_irq_working_callback_stage1(s32 (*callback)(u32 flags), u32 timeout);
s32  ehci_irq_working_callback_stage2(void);

#endif
