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

#define _ES_CALLS_S_
#include "es_calls.h"

/*
 * Macros
 */
.macro call addr
	stmfd	sp!, {r7, lr}
	ldr	r7, =\addr
	ldr	r7, [r7]
	blx	r7
	ldmfd	sp!, {r7, lr}
	bx	lr
.endm

.macro jump addr
	ldr	r3, =\addr
	ldr	r3, [r3]
	bx	r3
.endm


	.align 4

/*
 * ES functions
 */
#ifdef DEBUG
	.code 32
	.global ES_printf
ES_printf:
	call addrPrintf
#endif
	
	.code 32
	.global ES_snprintf
ES_snprintf:
	call addrSnprintf

	.code 32
	.global ES_LaunchTitle
ES_LaunchTitle:
	call addrLaunchTitle


/*
 * ES handlers
 */
	.code 16
	.thumb_func

	.global ES_HandleIoctlv
ES_HandleIoctlv:
	push	{r4-r6, lr}
	sub	sp, sp, #0x20
	ldr	r5, [r0, #8]
	add	r1, r0, #0

	jump addrIoctlv
