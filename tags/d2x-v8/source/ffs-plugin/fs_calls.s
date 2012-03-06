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
.macro fs_call func cmd
	add	r0, r4, #0
	push	{r1-r7}
	push	{r0}
	mov	r1, sp
	bl	\func
	pop	{r3}
	cmp	r3, #0
	bne	fs_exit	
	cmp	r0, #0
	bge	fs_exit

	// jump to the original fs command
	ldr	r0, =addrTable
	ldr	r3, [r0]
	ldr	r0, [r3, #((\cmd) * 4)]
	pop	{r1-r7}
	mov	pc, r0
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
	fs_call FS_Ioctl, FS_CMD_IOCTL

	.global fs_ioctlv
fs_ioctlv:
	fs_call FS_Ioctlv, FS_CMD_IOCTLV

fs_exit:
	pop	{r1-r7}
	add	r1, r0, #0
	ldr	r0, =addrReentry
	ldr	r0, [r0]
	mov	pc, r0
