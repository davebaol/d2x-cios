#ifndef _SWI_MLOAD_H_
#define _SWI_MLOAD_H_

#include "types.h"

/* Prototypes */
void Swi_Memcpy(void *dst, void *src, s32 len);
void Swi_uMemcpy(void *dst, void *src, s32 len);
void Swi_LedOn(void);
void Swi_LedOff(void);
void Swi_LedBlink(void);

#endif
