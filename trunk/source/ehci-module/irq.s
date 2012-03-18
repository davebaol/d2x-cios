/*
 * EHCI plugin for Custom IOS.
 *
 * Copyright (C) 2009 Hermes.
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
 * Patched interrupt vector
 */
	.align 4
	.code 32

	.global irq_vector
irq_vector:
	ldr     r0, [r9]             // r9 is the IRQ register 0x0d80003c
	and     r8, r8, r0           // r8 is the value of IRQ register 0x0d800038
	tst     r8, #0x10            // check EHCI interrupt
	beq     __irq_reentry
	bic     r8, r8, #0x10        // disable next EHCI treatment
	mov     r2, #0x10
	str     r2, [r7]             // r7 is the IRQ register 0x0d800038
	mov     r2, sp               // save the stack pointer
	ldr     sp, =ehci_irq_stack  // set the new stack pointer
	stmfd   sp!, {r1-r12, lr}    // push registers
	nop
	bl      __ehci_irq_handler   // call our ehci irq handler
	ldmfd   sp!, {r1-r12, lr}    // pop registers
	nop
	mov     sp, r2               // restore the stack pointer
	
__irq_reentry:
	ldr     r2, =addrIrqReentry  // back to the original irq procedure
	ldr	pc, [r2]

__ehci_irq_handler:
	ldr     r2, =ehci_irq_handler // can't use ldr pc here because
	bx      r2                    // we have to switch to thumb mode
