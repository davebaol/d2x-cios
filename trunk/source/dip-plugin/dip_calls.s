/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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
 * DIP handlers
 */
	.code 16
	.thumb_func

	.global DI_HandleIoctl
DI_HandleIoctl:
	push	{r4-r7, lr}
	mov	r7, r10
	mov	r6, r8
	push	{r6, r7}
	ldr	r5, [r0]
	mov	r10, r1
	jump	addr_handleIoctl

	.code 16
	.thumb_func

	.global DI_HandleCmd
DI_HandleCmd:
	push	{r4-r7, lr}
	mov	r7, r11
	mov	r6, r10
	mov	r5, r9
	mov	r4, r8
	push	{r4-r7}
	jump	addr_handleCmd


/*
 * DVD driver initializer (2nd stage)
 */
	.code 32
	.global DI_HandleInitDrive
DI_HandleInitDrive:
	push	{r4, lr}
	bl	os_check_DI_reset
	lsls	r0, r0, #0x18
	jump	addr_handleInitDrive
