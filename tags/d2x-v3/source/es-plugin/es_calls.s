/*
 * ES plugin for Custom IOS.
 *
 * Copyright (C) 2010 Waninkoko.
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


	.align 4

/*
 * ES functions
 */
	.code 32
	.global ES_printf
ES_printf:
	stmfd	sp!, {r7, lr}
	ldr	r7, =addrPrintf
	ldr	r7, [r7]
	blx	r7
	ldmfd	sp!, {r7, lr}
	bx	lr
	
	.code 32
	.global ES_snprintf
ES_snprintf:
	stmfd	sp!, {r7, lr}
	ldr	r7, =addrSnprintf
	ldr	r7, [r7]
	blx	r7
	ldmfd	sp!, {r7, lr}
	bx	lr

	.code 32
	.global ES_LaunchTitle
ES_LaunchTitle:
	stmfd	sp!, {r7, lr}
	ldr	r7, =addrLaunchTitle
	ldr	r7, [r7]
	blx	r7
	ldmfd	sp!, {r7, lr}
	bx	lr


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

	ldr	r3, =addrIoctlv
	ldr	r3, [r3]
	bx	r3
