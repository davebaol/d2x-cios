#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "types.h"

/* Constants */
#define DEBUG_NONE	0
#define DEBUG_BUFFER	1
#define DEBUG_GECKO	2

/* Prototypes */
s32 Debug_SetMode(u8 mode);
s32 Debug_GetBuffer(char *outbuf, u32 len);

#endif
