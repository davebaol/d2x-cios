#ifndef _USBGLUE_H_
#define _USBGLUE_H_

#include "types.h"

/* Prototypes */
bool usbstorage_Startup(void);
bool usbstorage_IsInserted(void);
bool usbstorage_ReadSectors (u32 sector, u32 numSectors, void *buffer);
bool usbstorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer);
bool usbstorage_ReadCapacity(u32 *sectorSz, u32 *numSectors);
bool usbstorage_Shutdown(void);

#endif
