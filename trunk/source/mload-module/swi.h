#ifndef _SWI_H_
#define _SWI_H_

#include "types.h"

/* SWI function */
typedef s32 (*SwiFunc)(u32 arg0, u32 arg1, u32 arg2, u32 arg3);


/* Prototypes */
s32 Swi_MLoad(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* Externs */
extern SwiFunc SwiTable[];
extern void    SwiVector(void);

#endif
