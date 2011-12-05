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


/*
 * Macros
 */
.macro jump addr
	ldr	r3, =(\addr)
	ldr	r3, [r3]
	bx	r3
.endm


	.align 4


/*
 * ES handlers
 */
	.code 16
	.thumb_func

	.global ES_HandleIoctlv
ES_HandleIoctlv:
	push {r4-r6, lr}
	sub  sp, sp, #0x20
	ldr  r5, [r0, #8]
	add  r1, r0, #0

	jump addrIoctlv
