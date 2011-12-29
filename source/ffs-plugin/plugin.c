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

#include <string.h>

#include "fat.h"
#include "fs_calls.h"
#include "fs_tools.h"
#include "ioctl.h"
#include "ipc.h"
#include "isfs.h"
#include "plugin.h"
#include "stealth.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "types.h"

/* Global config */
struct fsConfig config = { 0, {'\0'}, 0 };

/* Constants */
//#define FORCE_MODE_REV17
#define MAX_FAT_FD	20
#define FFS		"FFS"

static s32 fat_fd[MAX_FAT_FD];

s32 __FS_RegisterFile(s32 fd)
{
	s32 idx;
	for (idx=0; idx<MAX_FAT_FD; idx++)
		if (fat_fd[idx] < 0) {
			fat_fd[idx] = fd;
			return idx;
		}

	return -1;
}

s32 __FS_GetFileIndex(s32 fd)
{
	s32 idx;
	for (idx=0; idx<MAX_FAT_FD; idx++)
		if (fat_fd[idx] == fd)
			return idx;

	return -1;
}

void __FS_UnregisterFile(s32 idx)
{
	if (idx >= 0 && idx < MAX_FAT_FD)
		fat_fd[idx] = -1;
}

void __FS_UnregisterAllFiles(void)
{
	s32 idx;
	for (idx=0; idx<MAX_FAT_FD; idx++)
		fat_fd[idx] = -1;
}

typedef struct {
	const char *path;
	u8  delete;
	u8  make;
	u32 blocks;
	u32 inodes;
} dirinfo;

#define NUM_FOLDERS 12

static dirinfo dirs[NUM_FOLDERS] = {
	{"/import",          0, 1,   -1,  -1},
	{"/meta",            0, 0,    3,   8},
	{"/sys",             0, 1,   -1,  -1},
	{"/ticket",          0, 1,   71,  77},
	{"/ticket/00010001", 0, 1,   -1,  -1},
	{"/ticket/00010005", 0, 1,   -1,  -1},
	{"/title",           0, 1,   -1,  -1},
	{"/title/00010000",  0, 1,   75,  55},
	{"/title/00010001",  0, 1, 1505,  74},
	{"/title/00010004",  0, 1,  185,   9},
	{"/title/00010005",  0, 1,   23,  42},
	{"/tmp",             1, 1,   -1,  -1}
};

void __FS_PrepareFolders(void)
{
	char fatpath[FAT_MAXPATH];
	s32 cnt;

	/* Create directories */
	for (cnt = 0; cnt < NUM_FOLDERS; cnt++) {
		/* Generate path */
		FS_GeneratePath(dirs[cnt].path, fatpath);

		/* Delete directory */
		if (dirs[cnt].delete)
			FAT_DeleteDir(fatpath);

		/* Create directory */
		if (dirs[cnt].make)
			FAT_CreateDir(fatpath);
	}
}

u32 __FS_FakeUsage(const char *path, u32 *blocks, u32 *inodes)
{
	s32 cnt;

	for (cnt = 0; cnt < NUM_FOLDERS; cnt++) {
		if (!strcmp(dirs[cnt].path, path) && dirs[cnt].blocks >= 0 && dirs[cnt].inodes >= 0) {
			*blocks = dirs[cnt].blocks;
			*inodes = dirs[cnt].inodes;
			return 1;			
		}
	}

	return 0;			
}

s32 __FS_SetMode(u32 mode, char *path)
{
	/* FAT mode enabled */
	if (mode & (MODE_SDHC | MODE_USB)) {
		u32 ret;

		/* Nand emu can not be enabled when a title is running */
		if (Stealth_CheckRunningTitle(FFS, "IOCTL_ISFS_SETMODE(ON)"))
			return IPC_ENOENT;

		/* Initialize FAT */
		ret = FAT_Init();
		if (ret < 0)
			return ret;

#ifdef FORCE_MODE_REV17
		mode |= MODE_REV17;
#endif

		/* FAT mode rev17-like */
		if (mode & MODE_REV17) {
			s32 tid;

			/* Get current thread id */
			tid = os_get_thread_id();

			/* Add thread rights for stealth mode */
			Swi_AddThreadRights(tid, TID_RIGHTS_OPEN_FAT);
		}

		/* Set FS mode */
		config.mode = mode;

		/* Set nand path */
		strcpy(config.path, path);

		/* Set nand path length */
		config.pathlen = strlen(path);

		/* Clear open files */
		__FS_UnregisterAllFiles();

		/* Prepare folders */
		__FS_PrepareFolders();
	}
	else {
		/* When a title is running nand emu can be disabled through ES only */
		if (Swi_GetRunningTitle() && !Swi_GetEsRequest()) {
			Stealth_Log(STEALTH_RUNNING_TITLE | STEALTH_ES_REQUEST, FFS, "IOCTL_ISFS_SETMODE(OFF)");
			return IPC_ENOENT;
		}

		/* Set FS mode */
		config.mode = 0;

		/* Set nand path */
		config.path[0] = '\0';

		/* Set nand path length */
		config.pathlen = 0;
	}

	return 0;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Open(ipcmessage *message, u32 *performed)
{
	char *path = message->open.device;
	u32 mode = message->open.mode;


#ifdef DEBUG
#ifdef FILTER_OPENING_REQUESTS
	if (strncmp("/dev", path, 3) || !strncmp("/dev/fs", path, 7))
#endif
		FS_printf("FS_Open(\"%s\", %d)\n", path, mode);
#endif

	/* Clear flag */
	*performed = 0;

	/* FAT mode rev17-like */
	if (config.mode & MODE_REV17) {
		s32 ret;

		/* Check path */
		ret = FS_CheckRealPath(path);
		if (!ret) {
			char fatpath[FAT_MAXPATH];

			FS_printf("FS_Open: Emulating...\n");

			/* Set flag */
			*performed = 1;

			/* Generate path */ 
			FS_GeneratePathWithPrefix(path, fatpath);

			/* Open file */
			ret = os_open(fatpath, mode);

			/* Register file */
			if (ret >= 0)
				__FS_RegisterFile(ret);

			return ret;
		}
	}

	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Close(ipcmessage *message, u32 *performed)
{
	s32 fd = message->fd;

	FS_printf("FS_Close(%d)\n", fd);
  
	/* Clear flag */
	*performed = 0;

	/* FAT mode rev17-like */
	if (config.mode & MODE_REV17) {
		s32 fileIndex;

		fileIndex = __FS_GetFileIndex(fd);

		if (fileIndex >= 0) {
			s32 ret;
			
			/* Set flag */
			*performed = 1;
			
			/* Close file */
			ret = os_close(fd);

			/* Unregister file */
			if (ret >= 0)
				__FS_UnregisterFile(fileIndex);

			return ret;
		}
	}

	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Read(ipcmessage *message, u32 *performed)
{
	char *buffer = message->read.data;
	u32   len    = message->read.length;
	s32   fd     = message->fd;

	FS_printf("FS_Read(%d, 0x%08x, %d)\n", fd, (u32)buffer, len);
  
	/* Clear flag */
	*performed = 0;

	/* FAT mode rev17-like */
	if (config.mode & MODE_REV17) {
		s32 fileIndex;

		fileIndex = __FS_GetFileIndex(fd);

		if (fileIndex >= 0) {
			
			/* Set flag */
			*performed = 1;
			
			/* Read file */
			return os_read(fd, buffer, len);
		}
	}

	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Write(ipcmessage *message, u32 *performed)
{
	char *buffer = message->write.data;
	u32   len    = message->write.length;
	s32   fd     = message->fd;

	FS_printf("FS_Write(%d, 0x%08x, %d)\n", fd, (u32)buffer, len);
  
	/* Clear flag */
	*performed = 0;

	/* FAT mode rev17-like */
	if (config.mode & MODE_REV17) {
		s32 fileIndex;

		fileIndex = __FS_GetFileIndex(fd);

		if (fileIndex >= 0) {
			
			/* Set flag */
			*performed = 1;
			
			/* Write file */
			return os_write(fd, buffer, len);
		}
	}

	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Seek(ipcmessage *message, u32 *performed)
{
	s32 fd     = message->fd;
	s32 where  = message->seek.offset;
	s32 whence = message->seek.origin;

	FS_printf("FS_Seek(%d, %d, %d)\n", fd, where, whence);
  
	/* Clear flag */
	*performed = 0;

	/* FAT mode rev17-like */
	if (config.mode & MODE_REV17) {
		s32 fileIndex;

		fileIndex = __FS_GetFileIndex(fd);

		if (fileIndex >= 0) {
			
			/* Set flag */
			*performed = 1;
			
			/* Seek file */
			return os_seek(fd, where, whence);
		}
	}
	
	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Ioctl(ipcmessage *message, u32 *performed)
{
	static struct stats stats ATTRIBUTE_ALIGN(32);

	u32 *inbuf = message->ioctl.buffer_in;
	u32  inlen = message->ioctl.length_in;
	u32 *iobuf = message->ioctl.buffer_io;
	u32  iolen = message->ioctl.length_io;
	u32  cmd   = message->ioctl.command;

	s32 ret;

	/* Set flag */
	*performed = config.mode;

	/* Parse command */
	switch (cmd) {
	/** Create directory **/
	case IOCTL_ISFS_CREATEDIR: {
		fsattr *attr = (fsattr *)inbuf;

		FS_printf("FS_CreateDir(\"%s\", %02X, %02X, %02X, %02X)\n", attr->filepath, attr->ownerperm, attr->groupperm, attr->otherperm, attr->attributes);

		/* Check path */
		ret = FS_CheckRealPath(attr->filepath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char fatpath[FAT_MAXPATH];

			FS_printf("FS_CreateDir: Emulating...\n");

			/* Generate path */
			FS_GeneratePath(attr->filepath, fatpath);

			/* Create directory */
			return FAT_CreateDir(fatpath);
		}

		break;
	}

	/** Create file **/
	case IOCTL_ISFS_CREATEFILE: {
		fsattr *attr = (fsattr *)inbuf;

		FS_printf("FS_CreateFile(\"%s\", %02X, %02X, %02X, %02X)\n", attr->filepath, attr->ownerperm, attr->groupperm, attr->otherperm, attr->attributes);

		/* Check path */
		ret = FS_CheckRealPath(attr->filepath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char fatpath[FAT_MAXPATH];

			FS_printf("FS_CreateFile: Emulating...\n");

			/* Generate path */
			FS_GeneratePath(attr->filepath, fatpath);

			/* Create file */
			return FAT_CreateFile(fatpath); 
		}

		break;
	}

	/** Delete object **/
	case IOCTL_ISFS_DELETE: {
		char *filepath = (char *)inbuf;

		FS_printf("FS_Delete(\"%s\")\n", filepath);

		/* Check path */
		ret = FS_CheckRealPath(filepath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char fatpath[FAT_MAXPATH];

			FS_printf("FS_Delete: Emulating...\n");

			/* Generate path */
			FS_GeneratePath(filepath, fatpath);

			/* Delete */
			return FAT_Delete(fatpath); 
		}

		break;
	}

	/** Rename object **/
	case IOCTL_ISFS_RENAME: {
		fsrename *rename = (fsrename *)inbuf;

		FS_printf("FS_Rename(\"%s\", \"%s\")\n", rename->filepathOld, rename->filepathNew);

		/* Check paths */
		ret  = FS_CheckRealPath(rename->filepathOld);
		ret |= FS_CheckRealPath(rename->filepathNew);

		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char oldpath[FAT_MAXPATH];
			char newpath[FAT_MAXPATH];

//			struct stats stats;

			FS_printf("FS_Rename: Emulating...\n");

			/* Generate paths */
			FS_GeneratePath(rename->filepathOld, oldpath);
			FS_GeneratePath(rename->filepathNew, newpath);

			/* Compare paths */
			if (strcmp(oldpath, newpath)) {
				/* Check new path */
				ret = FAT_GetStats(newpath, &stats);

				/* New path exists */
				if (ret >= 0) {
					/* Delete directory */
					if (stats.attrib & AM_DIR)
						FAT_DeleteDir(newpath);

					/* Delete */
					FAT_Delete(newpath);
				}

				/* Rename */
				return FAT_Rename(oldpath, newpath); 
			}

			/* Check path exists */
			return FAT_GetStats(oldpath, NULL);
		}

		break;
	}

	/** Get device stats **/
	case IOCTL_ISFS_GETSTATS: {
		FS_printf("FS_GetStats()\n");

		/* FAT mode */
		if (config.mode) {
			fsstats *stats = (fsstats *)iobuf;

			FS_printf("FS_GetStats: Emulating...\n");

			/* Check buffer length */
			if (iolen < 0x1C)
				return -1017;

			/* Clear buffer */
			memset(iobuf, 0, iolen);

			/* Fill stats */
			stats->block_size  = 0x4000;
			stats->free_blocks = 0x5DEC;
			stats->used_blocks = 0x1DD4;
			stats->unk3        = 0x10;
			stats->unk4        = 0x02F0;
			stats->free_inodes = 0x146B;
			stats->unk5        = 0x0394;

			/* Flush cache */
			os_sync_after_write(iobuf, iolen);

			return 0;
		}

		break;
	}

	/** Get file stats **/
	case IOCTL_ISFS_GETFILESTATS: {
		s32 fd = message->fd;

		FS_printf("FS_GetFileStats(%d)\n", fd);

		/* Disable flag */
		*performed = 0;

		/* FAT mode rev17-like */
		if (config.mode & MODE_REV17) {
			s32 fileIndex;

			fileIndex = __FS_GetFileIndex(fd);

			if (fileIndex >= 0) {
				/* Set flag */
				*performed = 1;

				/* Get file stats */
				return FAT_GetFileStats(fd, (void *)iobuf);
			}
		}

		break;
	}

	/** Get attributes **/
	case IOCTL_ISFS_GETATTR: {
		char *path = (char *)inbuf;
		
		FS_printf("FS_GetAttributes(\"%s\")\n", path);

		/* Check path */
		ret = FS_CheckRealPath(path);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			fsattr *attr = (fsattr *)iobuf;
			char    fatpath[FAT_MAXPATH];

			FS_printf("FS_GetAttributes: Emulating...\n");

			/* Generate path */
			FS_GeneratePath(path, fatpath);

			/* Check path */
			ret = FAT_GetStats(fatpath, NULL);
			if (ret < 0)
				return ret;

			// This is a fix for MW3 online patch.
			// IOCTL_ES_ADDTITLEFINISH pretends that the
			// attributes of "/tmp/title.tmd" match the
			// values below.
			// 
			// NOTE:
			// Actually these attribute values are required only 
			// if the request arrives from ES. Maybe might be
			// worth considering a way to check this situation
			// before returning these attribute values.

			if(!strncmp("/tmp/", path, 5) || !strncmp("/import", path, 7)) {
				/* Fake attributes for ES */
				attr->owner_id   = 0;
				attr->group_id   = 0;
				attr->ownerperm  = ISFS_OPEN_RW;
				attr->groupperm  = ISFS_OPEN_RW;
				attr->otherperm  = 0;
				attr->attributes = 0;
			}
			else {
				s32 nocopy;

				/* Check no-copy protection */
				nocopy = FS_MatchPath(path, "/title/0001000#/########/data/nocopy", 0);

				/* Fake attributes */
				attr->owner_id   = FS_GetUID();
				attr->group_id   = FS_GetGID();
				attr->ownerperm  = ISFS_OPEN_RW;
				attr->groupperm  = ISFS_OPEN_RW;
				attr->otherperm  = nocopy ? 0 : ISFS_OPEN_RW;
				attr->attributes = 0;
			}
		  
			/* Copy filepath */
			memcpy(attr->filepath, path, ISFS_MAXPATH);

			/* Flush cache */
			os_sync_after_write(iobuf, iolen);

			return 0;
		}

		break;
	}

	/** Set attributes **/
	case IOCTL_ISFS_SETATTR: {
		fsattr *attr = (fsattr *)inbuf;

		FS_printf("FS_SetAttributes(\"%s\", %08X, %04X, %02X, %02X, %02X, %02X)\n", attr->filepath, attr->owner_id, attr->group_id, attr->ownerperm, attr->groupperm, attr->otherperm, attr->attributes);

		/* Check path */
		ret = FS_CheckRealPath(attr->filepath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char fatpath[FAT_MAXPATH];

			FS_printf("FS_SetAttributes: Emulating...\n");

			/* Generate path */
			FS_GeneratePath(attr->filepath, fatpath);

			/* Check path exists, permission ignored */
			return FAT_GetStats(fatpath, NULL);
		}

		break;
	}

	/** Format **/
	case IOCTL_ISFS_FORMAT: {
		FS_printf("FS_Format()\n");

		/* FAT mode */
		if (config.mode) {
			/* Do nothing */
			return 0;
		}

		break;
	}

	/** Set FS mode **/
	case IOCTL_ISFS_SETMODE: {
		/* Set flag */
		*performed = 1;

		/* Check input */
		if (inbuf == NULL || inlen < 4)
			return IPC_ENOENT;

		u32 mode = inbuf[0];

		FS_printf("FS_SetMode(%d, \"/\")\n", mode);
		
		return __FS_SetMode(mode, "");
	}

	default:
		FS_printf("FS_Ioctl(): default case reached cmd = %x\n", cmd);
		break;
	}

	/* Call handler */
	return -6;
}

/*
 * NOTE: 
 * The 2nd parameter is used to determine if call the original handler or not. 
 */
s32 FS_Ioctlv(ipcmessage *message, u32 *performed)
{
	ioctlv *vector = message->ioctlv.vector;
	u32     inlen  = message->ioctlv.num_in;
	u32     iolen  = message->ioctlv.num_io;
	u32     cmd    = message->ioctlv.command;

	s32 ret;

	/* Set flag */
	*performed = config.mode;

	/* Parse command */
	switch (cmd) {
	/** Read directory **/
	case IOCTL_ISFS_READDIR: {
		char *dirpath = (char *)vector[0].data;

		FS_printf("FS_Readdir(\"%s\", %d)\n", dirpath, iolen);

		/* Check path */
		ret = FS_CheckRealPath(dirpath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char *outbuf = NULL;
			u32  *outlen = NULL;
			u32   buflen = 0;
			
			char fatpath[FAT_MAXPATH];
			u32  entries;

			FS_printf("FS_Readdir: Emulating...\n");

			/* Set pointers/values */
			if (iolen > 1) {
				entries = *(u32 *)vector[1].data;
				outbuf  = (char *)vector[2].data;
				outlen  =  (u32 *)vector[3].data;
				buflen  =         vector[2].len;
			} else
				outlen  =  (u32 *)vector[1].data;

			/* Generate path */
			FS_GeneratePath(dirpath, fatpath);

			/* Read directory */
			ret = FAT_ReadDir(fatpath, outbuf, &entries);
			if (ret >= 0) {
				*outlen = entries;
				os_sync_after_write(outlen, sizeof(u32));
			}

			/* Flush cache */
			if (outbuf)
				os_sync_after_write(outbuf, buflen);

			return ret;
		}

		break;
	}

	/** Get device usage **/
	case IOCTL_ISFS_GETUSAGE: {
		char *dirpath = (char *)vector[0].data;

		FS_printf("FS_GetUsage(\"%s\")\n", dirpath);

		/* Check path */
		ret = FS_CheckRealPath(dirpath);
		if (ret) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			u32 *blocks = (u32 *)vector[1].data;
			u32 *inodes = (u32 *)vector[2].data;
			u8  syscode = *(u8*)0x0;

			// This is just an ugly workaround.
			// WiiWare and VirtualConsole seem to work
			// better with fake values, while disc-based 
			// Wii games (for example SSBB and MPT) seem
			// to work better with values taken from FAT.
			if (syscode != 'R' && syscode != 'S') {
				/* Set fake values */
				*blocks = 1;
				*inodes = 1;

				ret = 0;
			}
			else {
				char fatpath[FAT_MAXPATH];
				s32 fake;

				FS_printf("FS_GetUsage: Emulating...\n");

				*blocks = 0;
				*inodes = 1;        // empty folders return a file count of 1

				fake = __FS_FakeUsage(dirpath, blocks, inodes);
			
				/* Generate path */
				FS_GeneratePath(dirpath, fatpath);

				if (fake) {
					/* Check path */
					ret = FAT_GetStats(fatpath, NULL);
				}  
				else {
					/* Get usage */
					ret = FAT_GetUsage(fatpath, blocks, inodes);
				}  
			}  

			/* Flush cache */
			os_sync_after_write(blocks, sizeof(u32));
			os_sync_after_write(inodes, sizeof(u32));

			return ret;
		}

		break;
	}

	/** Set FS mode **/
	case IOCTL_ISFS_SETMODE: {

		/* Set flag */
		*performed = 1;

		/* Check input */
		if (vector == NULL || inlen == 0)
			return IPC_ENOENT;

		u32  mode  = *(u32 *)vector[0].data;
		char *path = "";

		/* Get path */
		if (inlen > 1)
			path = (char *)vector[1].data;

		FS_printf("FS_SetMode(%d, \"%s\")\n", mode, path);
		
		return __FS_SetMode(mode, path);
	}

	/** Get FS mode **/
	case IOCTL_ISFS_GETMODE: {

		/* Set flag */
		*performed = 1;

		/* When a title is running this command can be invoked through ES only */
		if (Swi_GetRunningTitle() && !Swi_GetEsRequest()) {
			Stealth_Log(STEALTH_RUNNING_TITLE | STEALTH_ES_REQUEST, FFS, "IOCTL_ISFS_GETMODE");
			return IPC_ENOENT;
		}

		u32  *mode     = (u32 *) vector[0].data;
		u32   mode_len = (u32)   vector[0].len;
		char *path     = (char *)vector[1].data;
		u32   path_len = (u32)   vector[1].len;

		FS_printf("FS_GetMode()\n");

		/* Copy config */
		*mode = config.mode;
		memcpy(path, config.path, path_len);

		/* Flush cache */
		os_sync_after_write(mode, mode_len);
		os_sync_after_write(path, path_len);
		
		return 0;
	}

	default:
		FS_printf("FS_Ioctlv(): default case reached cmd = %x\n", cmd);
		break;
	}

	/* Call handler */
	return -6;
}
