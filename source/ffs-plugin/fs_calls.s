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


/*
 * Constants
 */
#define FS_CMD_OPEN   1
#define FS_CMD_CLOSE  2
#define FS_CMD_READ   3
#define FS_CMD_WRITE  4
#define FS_CMD_SEEK   5
#define FS_CMD_IOCTL  6
#define FS_CMD_IOCTLV 7

/*
 * Macros
 */
.macro fs_begin
	add	r0, r4, #0
	push	{r1-r7}
.endm

.macro fs_end cmd
	cmp	r0, #0
	bge	fs_exit

	// jump to the original fs command
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #((\cmd) * 4)]
	pop	{r1-r7}
	mov	pc, r0
.endm

.macro fs_call func cmd
	fs_begin
	bl	\func
	fs_end	\cmd
.endm

.macro fs_call_ext func cmd
	fs_begin
	push	{r0}
	mov	r1, sp
	bl	\func
	pop	{r3}
	cmp	r3, #0
	bne	fs_exit	
	fs_end	\cmd
.endm


	.text


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
	fs_call FS_Open, FS_CMD_OPEN

	.global fs_close
fs_close:
	fs_call FS_Close, FS_CMD_CLOSE

	.global fs_read
fs_read:
	fs_call FS_Read, FS_CMD_READ

	.global fs_write
fs_write:
	fs_call FS_Write, FS_CMD_WRITE

	.global fs_seek
fs_seek:
	fs_call FS_Seek, FS_CMD_SEEK

	.global fs_ioctl
fs_ioctl:
	fs_call_ext FS_Ioctl, FS_CMD_IOCTL

	.global fs_ioctlv
fs_ioctlv:
	fs_call_ext FS_Ioctlv, FS_CMD_IOCTLV

fs_exit:
#ifdef DEBUG
	bl	FS_Exit
#endif
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
