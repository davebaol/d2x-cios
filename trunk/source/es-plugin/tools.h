#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "types.h"


/* Direct syscalls */
void DCInvalidateRange(void* ptr, int size);
void DCFlushRange(void* ptr, int size);
void ICInvalidate(void);

/* MLoad syscalls */
s32 Swi_MLoad(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* ARM permissions */
u32  Perms_Read(void);
void Perms_Write(u32 flags);

/* MEM2 routines */
void MEM2_Prot(u32 flag);

/* Tools */
void *VirtToPhys(void *address);
void *PhysToVirt(void *address);

#endif

