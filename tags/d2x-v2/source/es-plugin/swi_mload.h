#ifndef _SWI_MLOAD_H_
#define _SWI_MLOAD_H_

#include "types.h"

/* Prototypes */
void Swi_Memcpy(void *dst, void *src, s32 len);
void Swi_uMemcpy(void *dst, void *src, s32 len);
s32  Swi_CallFunc(s32 (*func)(void *in, void *out), void *in, void *out);
u32  Swi_GetSyscallBase(void);
u32  Swi_GetIosInfo(void *buffer);

#endif
