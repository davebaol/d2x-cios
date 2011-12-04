#ifndef _FAT_WRAPPER_H_
#define _FAT_WRAPPER_H_

#include "types.h"

/* Stats structure */
struct stats {
	/* Size */
	u32 size;

	/* Date and time */
	u16 date;
	u16 time;

	/* Attributes */
	u8 attrib;
};

/* File stats structure */
struct fstats {
	/* Length and position */
	u32 length;
	u32 pos;
};


/* Prototypes */
s32 FAT_Mount(u8 device, s32 partition);
s32 FAT_Unmount(u8 dev);
s32 FAT_Open(const char *path, u32 mode);
s32 FAT_Close(s32 fd);
s32 FAT_Read(s32 fd, void *buffer, u32 len);
s32 FAT_Write(s32 fd, void *buffer, u32 len);
s32 FAT_Seek(s32 fd, s32 where, s32 whence);
s32 FAT_CreateDir(const char *dirpath);
s32 FAT_CreateFile(const char *filepath);
s32 FAT_ReadDir(const char *dirpath, char *outbuf, u32 buflen, u32 *outlen, u32 entries, u8 lfn);
s32 FAT_Delete(const char *path);
s32 FAT_DeleteDir(const char *dirpath);
s32 FAT_Rename(const char *oldname, const char *newname);
s32 FAT_GetStats(const char *path, struct stats *stats);
s32 FAT_GetFileStats(s32 fd, struct fstats *stats);
s32 FAT_GetUsage(const char *dirpath, u64 *size, u32 *files);

#endif
