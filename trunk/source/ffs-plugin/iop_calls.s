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


	.text

/*
 * Syscall open hook
 *
 * NOTE:
 * The original vector works in thumb mode,
 * but it has been patched with bx pc and ldr pc, =addr 
 * to jump without modifying any register.
 * So we need to switch to thumb mode again.
 */
	.align 4
	.code 32

	.arm
	.global syscall_open
syscall_open:
	stmfd	sp!, {r4-r7, lr}
	stmfd	sp!, {r1-r3}
	nop
	ldr	r4, =(_syscall_open_thumb + 1)
	bx	r4

	.align 4
	.code 16

	.thumb
_syscall_open_thumb:
	bl	__IOP_SyscallOpen
	pop	{r1-r3}
	mov	r7, r11
	mov	r6, r10
	mov	r5, r9
	mov	r4, r8
	push	{r4-r7}
	ldr	r4, =addrSysOpen
	ldr	r4, [r4]
	nop
	mov	pc, r4

