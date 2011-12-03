/*   
	Custom IOS Module (FAT)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 WiiGator.
	Copyright (C) 2009 Waninkoko.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>

#include "ff.h"
#include "fs.h"
#include "fat_wrapper.h"
#include "ipc.h"
#include "mem.h"
#include "types.h"

/* Variables */
static FATFS fatFs[_DRIVES] ATTRIBUTE_ALIGN(32);

PARTITION Drives[_DRIVES]  ATTRIBUTE_ALIGN(32) = {
{0,0}, {1,0}
};

/* Buffer */
static char  lfnBuf[_MAX_LFN + 1] ATTRIBUTE_ALIGN(32);


s32 __FAT_OpenDir(DIR *dir, const char *dirpath)
{
	s32 ret;

	/* Open directory */
	ret = f_opendir(dir, dirpath);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return FS_SUCCESS;

	case FR_INVALID_NAME:
//	case FR_NO_PATH:
		return FS_ENOENT;
	}

	return FS_EFATAL;
}

s32 __FAT_ReadDir(DIR *dir, FILINFO *fno)
{
	s32 ret;

	/* Loop */
	while (1) {
		/* Read entry */
		ret = f_readdir(dir, fno);
		if (ret)
			return FS_ENOENT;

		/* Read end */
		if (fno->fname[0] == '\0')
			return FS_ENOENT;

		/* Check entry */
		if (fno->fname[0] != '.')
			break;
	}

	return FS_SUCCESS;
}


s32 FAT_Mount(u8 device, u8 partition)
{
	s32 ret;

	/* Set partition */
	Drives[device].pt = partition;

	/* Mount device */
	ret = f_mount(device, fatFs);
	if (ret)
		return FS_EFATAL;

	return FS_SUCCESS;
}

s32 FAT_Unmount(u8 device)
{
	s32 ret;

	/* Unmount device */
	ret = f_mount(device, NULL);
	if (ret)
		return FS_EFATAL;

	return FS_SUCCESS;
}

s32 FAT_Open(const char *path, u32 mode)
{
	FIL *fil = NULL;
	s32  ret;

	/* Allocate entry */
	fil = Mem_Alloc(sizeof(FIL));
	if (!fil)
		return IPC_ENOMEM;

	/* Open file */
	ret = f_open(fil, path, mode);
	if (ret) {
		/* Free entry */
		Mem_Free(fil);

		return FS_ENOENT;
	}

	return (u32)fil;
}

s32 FAT_Close(s32 fd)
{
	FIL *fil = (FIL *)fd;
	s32  ret;

	/* Close file */
	ret = f_close(fil);
	if (ret)
		return FS_EFATAL;

	/* Free memory */
	Mem_Free(fil);

	return FS_SUCCESS;
}

s32 FAT_Read(s32 fd, void *buffer, u32 len)
{
	FIL *fil = (FIL *)fd;

	u32 read;
	s32 ret;

	/* Read file */
	ret = f_read(fil, buffer, len, &read);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return read;

	case FR_DENIED:
		return FS_EACCESS;
	}

	return FS_EFATAL;
}

s32 FAT_Write(s32 fd, void *buffer, u32 len)
{
	FIL *fil = (FIL *)fd;

	u32 wrote;
	s32 ret;

	/* Write file */
	ret = f_write(fil, buffer, len, &wrote);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return wrote;

	case FR_DENIED:
		return FS_EACCESS;
	}

	return FS_EFATAL;
}

s32 FAT_Seek(s32 fd, s32 where, s32 whence)
{
	FIL *fil = (FIL *)fd;

	u32 offset;
	s32 ret;

	/* Calculate offset */
	switch (whence) {
	case SEEK_SET:
		offset = where;
		break;

	case SEEK_CUR:
		offset = fil->fptr  + where;
		break;

	case SEEK_END:
		offset = fil->fsize + where;
		break;

	default:
		return FS_EINVAL;
	}

	/* Seek file */
	ret = f_lseek(fil, offset);

	/* Error */
	if (ret)
		return FS_EFATAL;

	return offset;
}

s32 FAT_CreateDir(const char *dirpath)
{
	s32 ret;

	/* Create directory */
	ret = f_mkdir(dirpath);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return FS_SUCCESS;

	case FR_DENIED:
		return FS_EACCESS;

	case FR_EXIST:
		return FS_EEXIST;

	case FR_NO_FILE:
	case FR_NO_PATH:
		return FS_ENOENT;
	}

	return FS_EFATAL;
}

s32 FAT_CreateFile(const char *filepath)
{
	FIL fil;
	s32 ret;

	/* Create file */
	ret = f_open(&fil, filepath, FA_CREATE_NEW);

	/* Check result */
	switch (ret) {
	case FR_OK:
		f_close(&fil);
		return FS_SUCCESS;

	case FR_EXIST:
		return FS_EEXIST;

	case FR_NO_FILE:
	case FR_NO_PATH:
		return FS_ENOENT;
	}

	return FS_EFATAL;
}

s32 FAT_ReadDir(const char *dirpath, char *outbuf, u32 buflen, u32 *outlen, u32 entries, u8 lfn)
{
	DIR     dir;
	FILINFO fno;

	char *buffer = NULL;
	u32   cnt = 0, pos = 0;
	s32   ret;

	/* Open directory */
	ret = __FAT_OpenDir(&dir, dirpath);
	if (ret)
		return ret;

	/* Allocate memory */
	if (buflen) {
		buffer = Mem_Alloc(buflen);
		if (!buffer)
			return IPC_ENOMEM;
	}

	/* Setup LFN */
	fno.lfname = lfnBuf;
	fno.lfsize = sizeof(lfnBuf);

	/* Read directory */
	while (!entries || (entries > cnt)) {
		u32 len;

		/* Read entry */
		ret = __FAT_ReadDir(&dir, &fno);
		if (ret)
			break;

		/* Copy entry */
		if (buffer) {
			char *ptr = buffer + pos;
			char *name;

			/* Get name */
			name = (*fno.lfname) ? fno.lfname : fno.fname;

			/* Get length */
			len = strnlen(name, (lfn) ? _MAX_LFN : 12);

			/* Copy filename */
			strncpy(ptr, name, len);
			ptr[len] = 0;

			/* Update position */
			pos += len + 1;
		}

		/* Increase counter */
		cnt++;
	}

	/* Copy entries */
	if (outbuf && buffer) {
		/* Copy buffer */
		memcpy(outbuf, buffer, buflen);

		/* Free buffer */
		Mem_Free(buffer);
	}

	/* Set value */
	*outlen = cnt;

	return FS_SUCCESS;
}

s32 FAT_Delete(const char *path)
{
	s32 ret;

	/* Delete object */
	ret = f_unlink(path);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return FS_SUCCESS;

	case FR_DENIED:
		return FS_EACCESS;

	case FR_EXIST:
		return FS_EEXIST;

	case FR_NO_FILE:
	case FR_NO_PATH:
		return FS_ENOENT;
	}

	return FS_EFATAL;
}

s32 FAT_DeleteDir(const char *dirpath)
{
	DIR     dir;
	FILINFO fno;

	s32 ret;

	/* Open directory */
	ret = __FAT_OpenDir(&dir, dirpath);
	if (ret)
		return ret;

	/* Setup LFN */
	fno.lfname = lfnBuf;
	fno.lfsize = sizeof(lfnBuf);

	/* Read directory */
	for (;;) {
		char  path[_MAX_LFN + 1];
		char *name;

		/* Read entry */
		ret = __FAT_ReadDir(&dir, &fno);
		if (ret)
			break;

		/* Get name */
		name = (*fno.lfname) ? fno.lfname : fno.fname;

		/* Generate path */
		strcpy(path, dirpath);
		strcat(path, "/");
		strcat(path, name);

		/* Entry is directory */
		if (fno.fattrib & AM_DIR)
			FAT_DeleteDir(path);

		/* Delete */
		ret = FAT_Delete(path);
		if (ret)
			return ret;
	}

	return FS_SUCCESS;
}

s32 FAT_Rename(const char *oldname, const char *newname)
{
	char *ptr;
	s32   ret;

	/* Skip prefix */
	ptr = strchr(newname, ':');
	if (ptr)
		newname = ptr + 1;

	/* Rename object */
	ret = f_rename(oldname, newname);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return FS_SUCCESS;

	case FR_EXIST:
		return FS_EEXIST;

	case FR_NO_PATH:
	case FR_NO_FILE:
		return FS_ENOENT;
	}

	return FS_EFATAL;
} 

s32 FAT_GetStats(const char *path, struct stats *stats)
{
	FILINFO fno;
	s32     ret;

	/* Get stats */
	ret = f_stat(path, &fno);

	/* Fill info */
	if (!ret && stats) {
		stats->size   = fno.fsize;
		stats->date   = fno.fdate;
		stats->time   = fno.ftime;
		stats->attrib = fno.fattrib;
	}

	return ret;
}

s32 FAT_GetFileStats(s32 fd, struct fstats *stats)
{
	FIL *fil = (FIL *)fd;

	/* No file */
	if (!fil->fs)
		return FS_EFATAL;

	/* Fill stats */
	stats->length = fil->fsize;
	stats->pos    = fil->fptr;

	return FS_SUCCESS;
}

s32 FAT_GetUsage(const char *dirpath, u64 *size, u32 *files)
{
	DIR     dir;
	FILINFO fno;

	u64 totalSz  = 0;
	u32 totalCnt = 0;

	s32 ret;

	/* Open directory */
	ret = __FAT_OpenDir(&dir, dirpath);
	if (ret)
		return ret;

	/* Setup LFN */
	fno.lfname = lfnBuf;
	fno.lfsize = sizeof(lfnBuf);

	/* Read directory */
	for (;;) {
		u64 fsize;
		u32 fcount;

		/* Read entry */
		ret = __FAT_ReadDir(&dir, &fno);
		if (ret)
			break;

		/* Entry is directory */
		if (fno.fattrib & AM_DIR) {
			char  path[_MAX_LFN + 1];
			char *name;

			/* Get name */
			name = (*fno.lfname) ? fno.lfname : fno.fname;

			/* Generate path */
			strcpy(path, dirpath);
			strcat(path, "/");
			strcat(path, name);

			/* Get directory usage */
			ret = FAT_GetUsage(path, &fsize, &fcount);
			if (ret)
				return ret;
		} else {
			/* Get file info */
			fcount = 1;
			fsize  = fno.fsize;
		}

		/* Update variables */
		totalCnt += fcount;
		totalSz  += fsize;
	}

	/* Set values */
	*files = totalCnt;
	*size  = totalSz;

	return FS_SUCCESS;
}
