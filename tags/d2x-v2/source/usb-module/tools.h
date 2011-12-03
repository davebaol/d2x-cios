#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "types.h"


static inline s32 Swap32(s32 val)
{
	return ((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) |
	       ((val & 0xFF000000) >> 24) | ((val & 0x00FF0000) >> 8);
}

#endif

