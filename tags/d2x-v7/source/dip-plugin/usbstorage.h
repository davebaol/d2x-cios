#ifndef _USBSTORAGE_H_
#define _USBSTORAGE_H_

#include "types.h"

/* Prototypes */
int usbstorage_Init(void);
bool usbstorage_Shutdown(void);
bool usbstorage_IsInserted(void);
bool usbstorage_ReadSectors(u32 sector, u32 numSectors, void *buffer);
//bool usbstorage_WriteSectors(u32 sector, u32 numSectors, void *buffer);
bool usbstorage_ClearStatus(void);
bool __usbstorage_Read(u32 sector, u32 numSectors, void *buffer);
u32 usbstorage_GetSectorSize(void);

#endif

