/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.

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

	.text

	.code 16
	.global SwiVector
SwiVector:
	bx	 pc

	.code 32
	ldr	sp, =swi_stack
	nop
	stmfd	sp!, {r1-r12,lr}
	nop
	mrs	r12, cpsr
	stmfd	sp!, {r12}
	nop

	ldr	r12, =SwiAddr
	str	lr, [r12]
	nop

	bl	_Swi_Handler

	ldmfd	sp!, {r12}
	nop
	msr	cpsr_c, r12
	ldmfd	sp!, {r1-r12,lr}
	nop

	movs	pc, lr

_Swi_Handler:
	ldr	r12, =Swi_Handler
	bx	r12 
