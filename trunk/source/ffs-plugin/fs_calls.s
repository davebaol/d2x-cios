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
 * FFS functions
 */

#ifdef DEBUG
	.align 4
        .code 32

        .global FS_printf
FS_printf:
        stmfd   sp!, {r7, lr}
        ldr     r7, =addrPrintf
        ldr     r7, [r7]
        blx     r7
        ldmfd   sp!, {r7, lr}
        bx      lr
#endif


/*
 * FFS handlers
 */
	.align 4
	.code 16

	.thumb

	.global fs_unk
fs_unk:
	ldr     r0, =addrTable
	ldr	r0, [r0]
	nop
	ldr	r0, [r0, #0]
	nop
	mov	pc, r0

	.global fs_open
fs_open:
	add	r0, r4, #0
	push	{r1-r7}
	bl	FS_Open
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS open
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #4]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_close
fs_close:
	add	r0, r4, #0
	push	{r1-r7}
	bl	FS_Close
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS close
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #8]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_read
fs_read:
	add	r0, r4, #0
	push	{r1-r7}
	bl	FS_Read
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS read
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #12]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_write
fs_write:
	add	r0, r4, #0
	push	{r1-r7}
	bl	FS_Write	
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS write
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #16]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_seek
fs_seek:
	add	r0, r4, #0
	push	{r1-r7}
	bl	FS_Seek
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS seek
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #20]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_ioctl
fs_ioctl:
	add	r0, r4, #0
	push	{r1-r7}
	push	{r0}
	mov	r1, sp
	bl	FS_Ioctl
	pop	{r3}
	cmp	r3, #0
	bne	fs_exit	
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS ioctl
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #24]
	pop	{r1-r7}
	mov	pc, r0

	.global fs_ioctlv
fs_ioctlv:
	add	r0, r4, #0
	push	{r1-r7}
	push	{r0}
	mov	r1, sp
	bl	FS_Ioctlv
	pop	{r3}
	cmp	r3, #0
	bne	fs_exit	
	cmp	r0, #0
	bge	fs_exit

	// jump to the original FFS ioctlv
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #28]
	pop	{r1-r7}
	mov	pc, r0

fs_exit:
	bl	FS_Exit
	pop	{r1-r7}
	add	r1, r0, #0
	ldr	r0, =addrReentry
	ldr	r0, [r0]
	mov	pc, r0


/*
 * Syscall open hook
 *
 * NOTE:
 * The original vector works in thumb mode,
 * but it has been patched with bx pc and ldr pc, =addr 
 * to jump without modifying any register.
 * So we need to change correctly to thumb mode again.
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
	bl	__FS_SyscallOpen
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
