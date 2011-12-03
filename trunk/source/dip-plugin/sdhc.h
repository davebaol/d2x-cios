#ifndef _SDHC_H_
#define _SDHC_H_

#include "types.h"

/* Prototypes */
int sdhc_Init(void);
bool sdhc_Read(u32 sector, u32 numSectors, void *buffer);

#endif

