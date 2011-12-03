#ifndef _WBFS_H_
#define _WBFS_H_

/* Prototypes */
s32 WBFS_OpenDisc(u8 *discid); 
s32 WBFS_Read(void *buffer, u32 len, u32 offset);

#endif

