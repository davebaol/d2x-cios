	.text

	.align 4
	.code 32

/* Direct syscalls */
	.global DCInvalidateRange
DCInvalidateRange:
	mcr	p15, 0, r0, c7, c6, 1
	add	r0, #0x20
	subs	r1, #1
	bne	DCInvalidateRange
	bx	lr

	.global DCFlushRange
DCFlushRange:
	mcr	p15, 0, r0, c7, c10, 1
	add	r0, #0x20
	subs	r1, #1
	bne	DCFlushRange
	bx	lr

	.global ICInvalidate
ICInvalidate:
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0
	bx	lr


/* MLoad syscalls */
	.global Swi_MLoad
Swi_MLoad:
	svc	0xcc
	bx	lr


/* ARM permissions */
	.global Perms_Read
Perms_Read:
	mrc	p15, 0, r0, c3, c0
	bx	lr

	.global Perms_Write
Perms_Write:
	mcr	p15, 0, r0, c3, c0
	bx	lr


/* MEM2 routines */
	.global MEM2_Prot
MEM2_Prot:
	ldr	r1, =0xD8B420A
	strh	r0, [r1]
	bx	lr


/* Tools */
	.global VirtToPhys
VirtToPhys:
	and	r0, #0x7fffffff
	bx	lr

	.global PhysToVirt
PhysToVirt:
	orr	r0, #0x80000000
	bx	lr
