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
 * Macros
 */
.macro direct_syscall num name
	.align 4
	.code 32
	.global \name
\name:
	mov	r11, #(\num)
	b	invoke_direct_syscall
.endm


#define syscall_base     ios


	.text

/*
 * Direct syscall invocation
 *
 * NOTE:
 * Input register r11 specifies the syscall number to jump to.
 */
	.align 4
	.code 32

invoke_direct_syscall:
	ldr	r12, =syscall_base
	ldr	r12, [r12]
	nop
	ldr	r12, [r12, r11, lsl#2]
	nop
	bx	r12
	
/*
 * Direct syscalls 
 *
 * NOTE:
 * Direct syscalls are required when you have to call
 * a syscall from inside a syscall.
 */
	direct_syscall 0x03, direct_os_get_thread_id
	direct_syscall 0x3f, direct_os_sync_before_read
	direct_syscall 0x40, direct_os_sync_after_write
