#ifndef _EPIC_H_
#define _EPIC_H_

#include "types.h"

/* Constants */
#define EPIC_MESSAGE	0x1337


/* Prototypes */
s32  Epic_Init(s32 queuehandle);
void Epic_Main(void);

#endif
