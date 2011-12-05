#ifndef _EHCI_H_
#define _EHCI_H_

#include "types.h"

/* Prototypes */
bool ehci_Init(void);
bool ehci_Shutdown(void);
bool ehci_IsInserted(void);
bool ehci_ReadSectors (u32 sector, u32 numSectors, void *buffer);
bool ehci_WriteSectors(u32 sector, u32 numSectors, void *buffer);
u32 ehci_GetSectorSize();
bool ehci_ClearStatus(void);

#endif
