/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.
	Copyright (C) 2011 davebaol.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

	.section ".init"
	.arm

	.EQU	ios_thread_arg,		4
	.EQU	ios_thread_priority,	0x79
	.EQU	ios_thread_stacksize,	0x2000

	.EQU	__exe_mem_size,		0x80000
	.EQU	swi_stacksize,		0x900


	.global _start
_start:
	mov	r0, #0		@ int argc
	mov	r1, #0		@ char *argv[]
	ldr	r3, =main
	bx	r3

// CAUTION!!!!
// Don't change the code above because the installer patches  
// the jump table for IRQ9 in order to point here throught 
// the absolute address 0x136D0010 + 1.
// So the value of the label IRQ9 must be 0x136D0010,
// i.e. offset 16 from the start address of the module.  

	.code 16
	.thumb_func

	.global IRQ9
IRQ9:
	push	{r6}
	bl	__MLoad_System
	add	r5, r0, #0
	pop     {r6}
	bl	IRQ9_Exit
	add	r0, r5, #0
	pop	{r4-r6}
	pop	{r1}
        bx	r1

	.code 16
	.thumb_func

IRQ9_Exit:
	bx pc

	.align 4
	.code 32

	add	r0, r6, #0
	mrs	r1, cpsr
	bic	r1, r1, #0xc0
	orr	r1, r1, r0
	msr	cpsr_c, r1
	bx	lr


/*
 * Modules space size
 */
	.align 4

	.global exe_mem_size
exe_mem_size:
	.long __exe_mem_size



/*
 * Modules space
 */
	.section ".exe_mem", "aw", %progbits 

	.global exe_mem
exe_mem:
	.space	__exe_mem_size


/*
 * IOS BSS section
 */
	.section ".ios_bss", "a", %nobits

	.space	ios_thread_stacksize
	.global ios_thread_stack	/* stack decrements from high address.. */
ios_thread_stack:

	.space	swi_stacksize
	.global swi_stack
swi_stack:


/*
 * IOS module table
 */
	.section ".ios_info_table", "ax", %progbits

	.global ios_info_table
ios_info_table:
	.long	0x0
	.long	0x28			@ numentries * 0x28
	.long	0x6

	.long	0xB
	.long	ios_thread_arg		@ passed to thread entry func, maybe module id

	.long	0x9
	.long	_start

	.long	0x7D
	.long	ios_thread_priority

	.long	0x7E
	.long	ios_thread_stacksize

	.long	0x7F
	.long	ios_thread_stack
