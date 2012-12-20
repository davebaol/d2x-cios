/*
 * FFS plugin for Custom IOS.
 *
 * Copyright (C) 2009-2010 Waninkoko.
 * Copyright (C) 2011 davebaol.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "fat.h"
#include "ff.h"
#include "fs_calls.h"
#include "fs_tools.h"
#include "ipc.h"
#include "isfs.h"
#include "mem.h"
#include "plugin.h"
#include "types.h"
#include "sdio.h"
#include "usbstorage.h"

/* Constants */
#define FAT_PREFIX_SDHC    "0:"
#define FAT_PREFIX_USB     "1:"
#define FAT_PREFIX_LENGTH  2

#define FAT_NAND_PATH_LENGHT  (FAT_PREFIX_LENGTH + nandpathlen)

/* Variables */
static FATFS fatFs ATTRIBUTE_ALIGN(32);

PARTITION VolToPart[_VOLUMES]  ATTRIBUTE_ALIGN(32) = {{0,4}, {1,4}};

/* Long file name buffer */
static char  lfnBuf[_MAX_LFN + 1] ATTRIBUTE_ALIGN(32);


static char *__FAT_AllocEntryPath(const char *dirPath, char *entryName)
{
	char *entryPath; 
	u32   dirPathLen, entryNameLen;

	/* Get directory path lenght */
	dirPathLen = strlen(dirPath);

	/* Get entry name lenght */
	entryNameLen = strlen(entryName);

	/* Allocate memory */
	entryPath = Mem_Alloc(dirPathLen + entryNameLen + 2);
	if (!entryPath)
		return NULL;

	/* Copy directory path */
	strcpy(entryPath, dirPath);

	/* Append separator */
	entryPath[dirPathLen] = '/';

	/* Append entry name */
	strcpy(entryPath + dirPathLen + 1, entryName);

	return entryPath;
}

static void __FAT_Escape(char *dst, const char *src)
{
	char c;

	/* Escape invalid FAT characters */
	while ((c = *(src++)) != '\0') {
		char *esc;

		/* Check character */
		switch (c) {
		case '"': esc = "&qt;"; break;   // Escape double quote
		case '*': esc = "&st;"; break;   // Escape star
		case ':': esc = "&cl;"; break;   // Escape colon
		case '<': esc = "&lt;"; break;   // Escape lesser than
		case '>': esc = "&gt;"; break;   // Escape greater than
		case '?': esc = "&qm;"; break;   // Escape question mark
		case '|': esc = "&vb;"; break;   // Escape vertical bar
		default: *(dst++) = c; continue; // Copy valid FAT character
		}

		/* Replace escape sequence */
		strcpy(dst, esc);
		dst += 4;
	}

	/* End of string */
	*dst = '\0';
}

static s32 __FAT_Unescape(char *path)
{
	char *src = path;
	char *dst = path;
	char c;

	/* Unescape invalid FAT characters */
	while ((c = *(src++)) != '\0') {

		/* Check character */
		if (c == '&') {
			if      (!strncmp(src, "qt;", 3)) c = '"'; // Unescape double quote     
			else if (!strncmp(src, "st;", 3)) c = '*'; // Unescape star             
			else if (!strncmp(src, "cl;", 3)) c = ':'; // Unescape colon            
			else if (!strncmp(src, "lt;", 3)) c = '<'; // Unescape lesser than      
			else if (!strncmp(src, "gt;", 3)) c = '>'; // Unescape greater than     
			else if (!strncmp(src, "qm;", 3)) c = '?'; // Unescape question mark    
			else if (!strncmp(src, "vb;", 3)) c = '|'; // Unescape vertical bar     

			/* Skip matched escape sequence */
			if (c != '&')
				src += 3;
		} 

		/* Copy character */
		*(dst++) = c;
	}

	/* End of string */
	*dst = '\0';

	/* Return length */
	return dst - path;
}

static s32 __FAT_OpenDir(DIR *dir, const char *dirpath)
{
	s32 ret;

	/* Open directory */
	ret = f_opendir(dir, dirpath);

	/* Check result */
	switch (ret) {
	case FR_OK:
		return FS_SUCCESS;

	case FR_INVALID_NAME:

	// FIX:
	// Case uncommented in d2x v3beta6 for error code compatibility
	// to fix the message "corrupted data" in The Tower of Druaga.
	case FR_NO_PATH:
		return FS_ENOENT;
	}

	return FS_EFATAL;
}

static s32 __FAT_ReadDirEntry(DIR *dir, FILINFO *fno)
{
	/* Loop */
	do {
		s32 ret;

		/* Read entry */
		ret = f_readdir(dir, fno);
		if (ret)
			return FS_ENOENT;

		/* Read end */
		if (fno->fname[0] == '\0')
			return FS_ENOENT;

	} while (fno->fname[0] == '.');

	return FS_SUCCESS;
}

static s32 __FAT_MountPartition(u8 device, u8 partition)
{
	u8  dev = device - 1;
	s32 ret;

	/* Set partition */
	VolToPart[dev].pt = partition;

	/* Mount device */
	ret = f_mount(dev, &fatFs);
	if (!ret) {
		DIR d;
		char *root = (dev == 0) ? FAT_PREFIX_SDHC"/" : FAT_PREFIX_USB"/";

		/* Open root directory */
		if (f_opendir(&d, root) == FR_OK)
			return FS_SUCCESS;

		FS_printf("FS: __FAT_MountPartition failed!!! Unmounting device...\n", ret);

		/* Unmount device */
		f_mount(dev, NULL);
	}

	/* Reset partition */
	VolToPart[dev].pt = 4;

	return FS_EFATAL;
}

void FAT_GeneratePath(const char *fspath, char *fatpath)
{
	/* Copy device prefix */
	switch (FS_GetDevice(config.mode)) {
	case FS_MODE_SDHC:
		strcpy(fatpath, FAT_PREFIX_SDHC);
		fatpath += FAT_PREFIX_LENGTH;
		break;

	case FS_MODE_USB:
		strcpy(fatpath, FAT_PREFIX_USB);
		fatpath += FAT_PREFIX_LENGTH;
		break;
	}

	/* Append nand folder */
	strcpy(fatpath, config.nandpath);

	/* Append and escape required path */
	__FAT_Escape(fatpath + nandpathlen, fspath);
}

s32 FAT_Initialize()
{
	static u32 heapspace[0x7000] ATTRIBUTE_ALIGN(32);

	s32 ret;

	/* Initialize memory heap */
	ret = Mem_Init(heapspace, sizeof(heapspace));
	if (ret < 0)
		return ret;

	return 0;
}

s32 FAT_Mount(u8 device, s32 partition)
{
	s32 ret, i;
	FS_printf("FAT_Mount(%d,%d)\n", device, partition);

	/* Initialize device */
	switch (device) {
	case FS_MODE_SDHC:
		/* Initialize SDIO */
		ret = sdio_Startup();
		break;

	case FS_MODE_USB:
		/* Initialize EHCI */
		ret = usbstorage_Init();
		break;

	default:
		/* Unknown device */
		ret = 0;
		break;
	}

	/* Device not found */
	if (!ret)
		return IPC_EINVAL;

	/* Mount required partition */
	if (partition >= 0 && partition < 4) {
		ret = __FAT_MountPartition(device, partition);

		/* Return either mounted partition or error */
		return (ret ? FS_EFATAL : partition);
	}

	/* Find partition */
	for (i = 0; i < 4; i++) {
		ret = __FAT_MountPartition(device, i);
		if (!ret) {
			/* Return mounted partition */
			return i;
		}
	}

	return FS_EFATAL;
}

s32 FAT_Unmount(u8 device)
{
	u8  dev = device - 1;
	s32 ret;

	/* Unmount device */
	ret = f_mount(dev, NULL);
	if (ret)
		return FS_EFATAL;

	/* Reset partition */
	VolToPart[dev].pt = 4;

	/* Denitialize device */
	switch (device) {
	case FS_MODE_SDHC:
		/* Deinitialize SDIO */
		ret = sdio_Shutdown();
		break;

	case FS_MODE_USB:
		/* Deinitialize USB */
		ret = usbstorage_Shutdown();
		break;

	default:
		/* Unknown device */
		ret = 0;
		break;
	}

	return (ret ? FS_SUCCESS : IPC_EINVAL);
}

static s32 __FAT_OpenHack(s32 ret, FIL *fil, const char *path, u32 mode)
{
	static u32 title_ids[] = {
		0x525559	// No More Heroes 2
		/* Add more games here if needed */
	};

	if (ret == FR_NO_FILE && (mode & FA_WRITE)) {
		u32 cnt, tid;

		tid = *(vu32*)0 >> 8;

		for (cnt = 0; cnt < (sizeof(title_ids) / sizeof(u32)); cnt++) {
			if (tid == title_ids[cnt]) {
				char pattern[ISFS_MAXPATH];

				FS_snprintf(pattern, sizeof(pattern), "/title/########/%06x##/data/#", tid);

				if (FS_MatchPath(((char *) path) + FAT_NAND_PATH_LENGHT, pattern, 0)) {
					ret = f_open(fil, path, mode | FA_CREATE_ALWAYS); 
				}

				break;
			}
		}
	}

	return ret;
}

s32 FAT_Open(FIL *fil, const char *path, u32 mode)
{
	s32  ret;

	/* Open file */
	ret = f_open(fil, path, mode);

	/*
	 * This fix is required by games like No More Heroes 2 to create the save
	 * Most games don't seem to mind this fix
	 * but it breaks Monster Hunter Tri so we need a hack.
	 */
	ret = __FAT_OpenHack(ret, fil, path, mode);

//	/* This fix is required by No More Heroes 2 to create the save */
//	if (ret == FR_NO_FILE && (mode & FA_WRITE)) {
//		// Most games don't seem to mind this fix
//		// but it breaks Monster Hunter Tri so we need a hack.
//		u32 isNMH2 = ((*(vu32*)0 >> 8) == 0x525559);
//		if (isNMH2 && FS_MatchPath(((char *) path) + FAT_NAND_PATH_LENGHT, "/title/00010000/525559##/data/#", 0))
//			ret = f_open(fil, path, mode | FA_CREATE_ALWAYS); 
//	}

	/* Error */
	if (ret) {
		/* File sharing control disabled. See issue 11. */
		if (_FS_SHARE == 0)
			return FS_ENOENT;

		/* File sharing control enabled */
		return ret == FR_TOO_MANY_OPEN_FILES ? FS_ENFILE : FS_ENOENT;
	}

	return FS_SUCCESS;
}

s32 FAT_Close(FIL *fil)
{
	s32  ret;

	/* Close file */
	ret = f_close(fil);

	return (ret ? FS_EFATAL : FS_SUCCESS);
}

s32 FAT_Read(FIL *fil, void *buffer, u32 len)
{
	u32 read;
	s32 ret;

	/* Read file */
	ret = f_read(fil, buffer, len, &read);

	/* On success return the number of read bytes */
	if (ret == FR_OK)
		return read;

	/* Error */
	return (ret == FR_DENIED ? FS_EACCESS : FS_EFATAL);
}

s32 FAT_Write(FIL *fil, void *buffer, u32 len)
{
	u32 wrote;
	s32 ret;

	/* Write file */
	ret = f_write(fil, buffer, len, &wrote);

	/* On success return the number of written bytes */
	if (ret == FR_OK)
		return wrote;

	/* Error */
	return (ret == FR_DENIED ? FS_EACCESS : FS_EFATAL);
}

s32 FAT_Seek(FIL *fil, s32 where, s32 whence)
{
	u32 offset;

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
		// FIX:
 		// Modified in d2x v4beta2 to improve
 		// error code compatibility.
		return FS_EFATAL; //FS_EINVAL;
	}

	// FIX:
 	// Check added in d2x v4beta2 to prevent from increasing
 	// the file size when seeking out of the file.
	if(offset <= f_size(fil)) {
		s32 ret;

		/* Seek file */
		ret = f_lseek(fil, offset);

		/* No error */
		if (!ret)
			return offset;
	}

	return FS_EFATAL;
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

s32 FAT_ReadDir(const char *dirpath, char *outbuf, u32 buflen, u32 *outlen, u32 entries)
{
	DIR     dir;
	FILINFO fno;

	char *buffer = NULL;
	u32   cnt = 0, pos = 0;
	s32   ret;
	FIL   fil;
	
	// FIX:
	// Check added in d2x v4beta2 to improve error code compatibility.
	// Now FS_EFATAL is returned when the requested folder is an existing file.
	// With this fix some games like all Strong Bad episodes are working now.
	ret = f_open(&fil, dirpath, FA_OPEN_EXISTING);
	if (ret == FR_OK) {
		f_close(&fil);
		FS_printf("FAT_Readdir: ERROR: The requested folder is an existing file\n");
		return FS_EFATAL;
	}

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
		char *name;

		/* Read entry */
		ret = __FAT_ReadDirEntry(&dir, &fno);
		if (ret)
			break;

		/* Get name */
		name = (*fno.lfname) ? fno.lfname : fno.fname;

		/* Unescape invalid FAT characters in place */
		len = __FAT_Unescape(name);

		/* Skip file names too long for FS */
		if (len > 12)
			continue;

		/* Copy entry */
		if (buffer) {
			char *ptr = buffer + pos;

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
		char *entrypath;
		char *name;

		/* Read entry */
		ret = __FAT_ReadDirEntry(&dir, &fno);
		if (ret)
			break;

		/* Get name */
		name = (*fno.lfname) ? fno.lfname : fno.fname;

		/* Generate subdirectory path */
		entrypath = __FAT_AllocEntryPath(dirpath, name);
		if (!entrypath)
			return IPC_ENOMEM;

		/* Entry is directory */
		if (fno.fattrib & AM_DIR)
			FAT_DeleteDir(entrypath);

		/* Delete */
		ret = FAT_Delete(entrypath);

		/* Free buffer */
		Mem_Free(entrypath);

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

	// FIX:
	// Initialization code added in d2x v4beta2 because members lfname 
	// and lfsize MUST be inited before using FILEINFO structure.
	// Now games like Max & the Magic Marker, FFCC My Life as a King
	// and FFCC My Life as a Darklord are working poperly.
	/* Setup LFN */
	fno.lfname = lfnBuf;
	fno.lfsize = sizeof(lfnBuf);

	/* Get stats */
	ret = f_stat(path, &fno);
	
	// FIX:
	// Error code compatibility improved in d2x v3beta6.
	// This way LIT doesn't stall anymore
	switch(ret) {
		case FR_OK:
			/* Fill info */
			if (stats) {
				stats->size   = fno.fsize;
				stats->date   = fno.fdate;
				stats->time   = fno.ftime;
				stats->attrib = fno.fattrib;
			}
			return FS_SUCCESS;
			
		case FR_NO_FILE:
		case FR_NO_PATH:
		case FR_INVALID_NAME:
			return FS_ENOENT;
	}

	return FS_EFATAL;
}

s32 FAT_GetFileStats(FIL *fil, struct fstats *stats)
{
	stats->length = fil->fsize;
	stats->pos    = fil->fptr;

	return FS_SUCCESS;
}

s32 __FAT_GetUsage(const char *dirpath, u64 *size, u32 *files)
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
		char *name;
		u32 len;

		/* Read entry */
		ret = __FAT_ReadDirEntry(&dir, &fno);
		if (ret)
			break;

		/* Get name */
		name = (*fno.lfname) ? fno.lfname : fno.fname;

		/* Unescape invalid FAT characters in place */
		len = __FAT_Unescape(name);

		/* Skip entries too long for FS */
		if (len > 12)
			continue;

		/* Entry is subdirectory */
		if (fno.fattrib & AM_DIR) {
			char *subdirpath;

			/* Generate subdirectory path */
			subdirpath = __FAT_AllocEntryPath(dirpath, name);
			if (!subdirpath)
				return IPC_ENOMEM;

			/* Update file counter */
			*files = *files + 1;

			/* Get subdirectory usage */
			ret = __FAT_GetUsage(subdirpath, size, files);

			/* Free buffer */
			Mem_Free(subdirpath);

			if (ret)
				return ret;

		} else {
			/* Update size and file counters */
			*size  = *size + fno.fsize;
			*files = *files + 1;
		}
	}

	return FS_SUCCESS;
}

s32 FAT_GetUsage(const char *path, u32 *blocks, u32 *inodes)
{
	s32 ret;
	u64 size = 0;
	u32 files = 0;

	ret = __FAT_GetUsage(path, &size, &files);

	FS_printf("FS_GetUsage(): ret = %d, size = %lu, files = %u\n", ret, size, files);

	/* Copy data */
	if (ret >= 0) {
		*blocks = (u32) (size / 0x4000);
		*inodes = files + 1;

		/*
		 * This check limits the number of used blocks to 128 MB
		 * to prevent no space error in case of big emulated nand
		 */
		if (*blocks > 0x2000) 
			*blocks = 0x1000 + (*blocks & 0xFFF);
			
		FS_printf("FS_GetUsage(): blocks = %x, inodes = %u\n", *blocks, *inodes);
	}

	return ret;
}

