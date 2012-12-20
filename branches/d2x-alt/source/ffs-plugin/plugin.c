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
#include "ff.h"
#include "fs_calls.h"
#include "fs_tools.h"
#include "ioctl.h"
#include "ipc.h"
#include "isfs.h"
#include "mem.h"
#include "plugin.h"
#include "stealth.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "types.h"

#define FORCE_LED_ACTIVITY

/* Global variables */
fsconfig config      = { 0, 0, {'\0'}};
u32      nandpathlen = 0;

/* Global flag to force real nand access */
u32 forceRealPath = 0;

/* Static variables */
static s32 __partition = 0;
static FIL fatHandle[MAX_FILE] ATTRIBUTE_ALIGN(32);

static char __fatpath[FAT_MAXPATH];

static u32 __FS_CheckHandle(s32 fd)
{
	if (fd < 0 || fd >= MAX_FILE)
		return 0;

	return (fatHandle[fd].fs != NULL);
}

static s32 __FS_GetHandle(void)
{
	s32 idx;
	for (idx = 0; idx < MAX_FILE; idx++)
		if (fatHandle[idx].fs == NULL)
			return idx;

	return FS_ENFILE;
}

static void __FS_ClearHandle(s32 idx)
{
	if (idx >= 0 && idx < MAX_FILE)
		memset(&fatHandle[idx], 0, sizeof(FIL));
}

static void __FS_ClearAllHandles(void)
{
	memset(fatHandle, 0, sizeof(fatHandle));
}

typedef struct {
	const char *path;
	u8  delete;
	u8  make;
	s32 blocks;
	s32 inodes;
} dirinfo;


static dirinfo dirs[] = {
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

#define NUM_FOLDERS (sizeof(dirs) / sizeof(dirinfo))

static void __FS_PrepareFolders(void)
{
	s32 cnt;

	/* Create directories */
	for (cnt = 0; cnt < NUM_FOLDERS; cnt++) {
		/* Generate path */
		FAT_GeneratePath(dirs[cnt].path, __fatpath);

		/* Delete directory */
		if (dirs[cnt].delete)
			FAT_DeleteDir(__fatpath);

		/* Create directory */
		if (dirs[cnt].make)
			FAT_CreateDir(__fatpath);
	}
}

static u32 __FS_FakeUsage(const char *path, u32 *blocks, u32 *inodes)
{
	s32 cnt;

	for (cnt = 0; cnt < NUM_FOLDERS; cnt++) {
		if (!strcmp(dirs[cnt].path, path)) {
			if (dirs[cnt].blocks == -1 && dirs[cnt].inodes == -1)
				return 0;			
			*blocks = dirs[cnt].blocks;
			*inodes = dirs[cnt].inodes;
			return 1;			
		}
	}

	return 0;			
}


static s32 __FS_FAT_Mount(s32 device, s32 partition)
{
	s32 ret;

	/* Initialize FAT */
	ret = FAT_Initialize();

	/* Mount FAT partition of the specified device */
	if (ret >= 0)
		ret = FAT_Mount(device, partition);

	return ret;
}

static s32 __FS_SetConfig(fsconfig *cfg, s32 mountFAT)
{
	s32 ret = 0;
	u32 device = FS_GetDevice(cfg->mode);

	FS_printf("__FS_SetConfig: device = %d\n", device);

	/* FAT mode enabled */
	if (device != FS_MODE_NAND) {

		/* Can not enable nand emu when a title is running */
		if (Stealth_CheckRunningTitle("IOCTL_ISFS_SETCONFIG(ON)"))
			return FS_EFATAL;


		/* Clear file handlers */
		__FS_ClearAllHandles();

		/* Mount FAT device/partition */
		if (mountFAT) {
			ret = __FS_FAT_Mount(device, cfg->partition);
			if (ret < 0)
				return ret;
		}

		/* Set nand path length */
		nandpathlen = strnlen(cfg->nandpath, sizeof(cfg->nandpath));

		/* Check nand path length */
		if (nandpathlen == sizeof(cfg->nandpath)) {
			ret = FS_EFATAL;
			goto set_nand_mode;
		}

		/* Ignore final '/' in nand path, if any */
		if (nandpathlen > 0 && cfg->nandpath[nandpathlen-1] == '/')
			nandpathlen--;

		/* Set nand path */
		strncpy(config.nandpath, cfg->nandpath, nandpathlen);
		config.nandpath[nandpathlen] = '\0';

		/* Set mounted partition */
		config.partition = ret;


		/* Set mode */
		config.mode = cfg->mode;

#ifdef FORCE_LED_ACTIVITY
		config.mode |= FS_MODE_LED;
#endif

		/* Prepare folders */
		__FS_PrepareFolders();

		/* Success */
		return ret;
	}
	else {
		/* When a title is running nand emu can be disabled through ES only */
		if (Swi_GetRunningTitle() && !Swi_GetEsRequest()) {
			Stealth_Log(STEALTH_RUNNING_TITLE | STEALTH_ES_REQUEST, "IOCTL_ISFS_SETCONFIG(OFF)");
			return FS_EFATAL;
		}
	}

	set_nand_mode:

	/* Set FS mode */
	config.mode = 0;

	/* Set partition */
	config.partition = 0;

	/* Set nand path */
	config.nandpath[0] = '\0';

	/* Set nand path length */
	nandpathlen = 0;

	return ret;
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
#ifdef DEBUG_FILTER_OPENING_REQUESTS
	if (strncmp(path, "/dev", 4) || !strncmp(path, "/dev/fs", 7))
#endif
		FS_printf("FS_Open(\"%s\", %d)\n", path, mode);
#endif

	/* Clear flag */
	*performed = 0;

	/* FAT mode */
	if (config.mode) {
		s32 ret;

		/* Force real path */
		if (forceRealPath) {
			/* Reset flag */
			forceRealPath = 0;

			return -6;
		}

		/* Check path */
		ret = FS_CheckRealPath(path);
		if (!ret) {
			s32 fd;

			FS_printf("FS_Open: Emulating...\n");

			/* Set flag */
			*performed = 1;

			fd = __FS_GetHandle();
			if (fd >= 0) {
				/* Generate path */ 
				FAT_GeneratePath(path, __fatpath);

				/* Open file */
				ret = FAT_Open(&fatHandle[fd], __fatpath, mode);

				/* Error */
				if (ret < 0) {
					/* Unregister file */
					__FS_ClearHandle(fd);

					return ret;
				}
			}

			return fd;
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

	/* FAT mode */
	if (config.mode) {
		s32 ret;

		ret = __FS_CheckHandle(fd);

		if (ret) {
			FS_printf("FS_Close: Emulating...\n");
			
			/* Set flag */
			*performed = 1;
			
			/* Close file */
			ret = FAT_Close(&fatHandle[fd]);

			/* Unregister file */
			if (ret >= 0)
				__FS_ClearHandle(fd);

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

#ifndef DEBUG_NO_READ_WRITE_SEEK
	FS_printf("FS_Read(%d, 0x%08x, %d)\n", fd, (u32)buffer, len);
#endif

	/* Clear flag */
	*performed = 0;

	/* FAT mode */
	if (config.mode) {
		s32 ret;

		ret = __FS_CheckHandle(fd);

		if (ret) {
#ifndef DEBUG_NO_READ_WRITE_SEEK
			FS_printf("FS_Read: Emulating...\n");
#endif

			/* Set flag */
			*performed = 1;

			/* Read file */
			ret = FAT_Read(&fatHandle[fd], buffer, len);

			/* Flush cache */
			os_sync_after_write(buffer, len);

			return ret;
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

#ifndef DEBUG_NO_READ_WRITE_SEEK
	FS_printf("FS_Write(%d, 0x%08x, %d)\n", fd, (u32)buffer, len);
 #endif
	/* Clear flag */
	*performed = 0;

	/* FAT mode */
	if (config.mode) {
		s32 ret;

		ret = __FS_CheckHandle(fd);

		if (ret) {
#ifndef DEBUG_NO_READ_WRITE_SEEK
			FS_printf("FS_Write: Emulating...\n");
#endif

			/* Set flag */
			*performed = 1;

			/* Invalidate cache */
			os_sync_before_read(buffer, len);
			
			/* Write file */
			return FAT_Write(&fatHandle[fd], buffer, len);
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

#ifndef DEBUG_NO_READ_WRITE_SEEK
	FS_printf("FS_Seek(%d, %d, %d)\n", fd, where, whence);
#endif
  
	/* Clear flag */
	*performed = 0;

	/* FAT mode */
	if (config.mode) {
		s32 ret;

		ret = __FS_CheckHandle(fd);

		if (ret) {
#ifndef DEBUG_NO_READ_WRITE_SEEK
			FS_printf("FS_Seek: Emulating...\n");
#endif
			
			/* Set flag */
			*performed = 1;
			
			/* Seek file */
			return FAT_Seek(&fatHandle[fd], where, whence);
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

			FS_printf("FSCreateDir: Emulating...\n");

			/* Generate path */
			FAT_GeneratePath(attr->filepath, __fatpath);

			/* Create directory */
			return FAT_CreateDir(__fatpath);
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

			FS_printf("FS_CreateFile: Emulating...\n");

			/* Generate path */
			FAT_GeneratePath(attr->filepath, __fatpath);

			/* Create file */
			return FAT_CreateFile(__fatpath); 
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

			FS_printf("FS_Delete: Emulating...\n");

			/* Generate path */
			FAT_GeneratePath(filepath, __fatpath);

			/* Delete */
			return FAT_Delete(__fatpath); 
		}

		break;
	}

	/** Rename object **/
	case IOCTL_ISFS_RENAME: {
		fsrename *rename = (fsrename *)inbuf;
		s32 ret2;

		FS_printf("FS_Rename(\"%s\", \"%s\")\n", rename->filepathOld, rename->filepathNew);

		/* Check paths */
		ret  = FS_CheckRealPath(rename->filepathOld);
		ret2 = FS_CheckRealPath(rename->filepathNew);

		/* Mixed paths are not supported */
		if (ret ^ ret2)
			svc_write("FS_Rename: Invalid arguments. Paths must be both real or both emulated.\n");

		if (ret | ret2) {
			*performed = 0;
			break;
		}

		/* FAT mode */
		if (config.mode) {
			char *newpath;
			struct stats stats;

			/* Generate source path */
			FAT_GeneratePath(rename->filepathOld, __fatpath);
			
			FS_printf("FS_Rename: Emulating...\n");

			/* Source and destination paths are the same */
			if (!strcmp(rename->filepathOld, rename->filepathNew)) {
				/* Check path exists */
				return FAT_GetStats(__fatpath, NULL);
			}

			/* Alloc destination path buffer */
			newpath = Mem_Alloc(FAT_MAXPATH);
			if (!newpath)
				return IPC_ENOMEM;

			/* Generate destination path */
			FAT_GeneratePath(rename->filepathNew, newpath);

			/* Check destination path */
			ret = FAT_GetStats(newpath, &stats);

			/* Destination path exists */
			if (ret >= 0) {
				/* Delete directory */
				if (stats.attrib & AM_DIR)
					FAT_DeleteDir(newpath);

				/* Delete */
				FAT_Delete(newpath);
			}

			/* Rename */
			ret = FAT_Rename(__fatpath, newpath); 

			/* Free destination path buffer */
			Mem_Free(newpath); 

			return ret; 
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

		/* FAT mode */
		if (config.mode) {

			ret = __FS_CheckHandle(fd);
			if (ret) {
				FS_printf("FS_GetFileStats: Emulating...\n");

				/* Set flag */
				*performed = 1;

				/* Get file stats */
				ret = FAT_GetFileStats(&fatHandle[fd], (void *)iobuf);

				/* Flush cache */
				os_sync_after_write(iobuf, iolen);

				return ret;
			}
		}

		break;
	}

	/** Get attributes **/
	case IOCTL_ISFS_GETATTR: {
		char *path = NULL;

		/* Set path */
		switch (inlen) {
		case 0x40:
			path = (char *)inbuf;
			break;
		case 0x4A:
			path = (char *)(inbuf + 6);
			break;
		default:
			*performed = 1;
			return FS_EFATAL;
		}
		
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

			FS_printf("FS_GetAttributes: Emulating...\n");

			/* Generate path */
			FAT_GeneratePath(path, __fatpath);

			/* Check path */
			ret = FAT_GetStats(__fatpath, NULL);
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

			FS_printf("FS_SetAttributes: Emulating...\n");

			/* Generate path */
			FAT_GeneratePath(attr->filepath, __fatpath);

			/* Check path exists, permission ignored */
			return FAT_GetStats(__fatpath, NULL);
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

	/** Set FS config **/
	case IOCTL_ISFS_SETCONFIG: {
		/* Set flag */
		*performed = 1;

		/* Check input */
		if (inbuf == NULL || inlen < sizeof(u32) || inlen > sizeof(fsconfig))
			return FS_EFATAL;

		fsconfig cfg = { 0, 0, {'\0'} };

		/* Copy input */
		memcpy(&cfg, inbuf, inlen);

		FS_printf("FS_SetConfig(%d, %d, \"%s\")\n", cfg.mode, cfg.partition, cfg.nandpath);
		
		return __FS_SetConfig(&cfg, 1);
	}

	/** Get FS config **/
	case IOCTL_ISFS_GETCONFIG: {

		FS_printf("FS_GetMode()\n");

		/* Set flag */
		*performed = 1;

		/* When a title is running this command can be invoked through ES only */
		if (Swi_GetRunningTitle() && !Swi_GetEsRequest()) {
			Stealth_Log(STEALTH_RUNNING_TITLE | STEALTH_ES_REQUEST, "IOCTL_ISFS_GETCONFIG");
			return FS_EFATAL;
		}

		/* Check output */
		if (iobuf == NULL || iolen < sizeof(fsconfig))
			return FS_EFATAL;

		/* Copy data */
		memcpy(iobuf, &config, sizeof(fsconfig));

		/* Flush cache */
		os_sync_after_write(iobuf, iolen);
		
		return 0;
	}

	default:
		FS_printf("FS_Ioctl(): Unknown command 0x%x\n", cmd);
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
			char *outbuf  = NULL;
			u32  *outlen  = NULL;
			u32   buflen  = 0;	
			u32   entries = 0;

			FS_printf("FS_Readdir: Emulating...\n");

			/* Set pointers/values */
			if (inlen == 1 && iolen == 1) {
				outlen  =  (u32 *)vector[1].data;
				FS_printf("FS_Readdir: Counting entries...\n");
			}
			else if (inlen == 2 && iolen == 2) {
				entries = *(u32 *)vector[1].data;
				outbuf  = (char *)vector[2].data;
				outlen  =  (u32 *)vector[3].data;
				buflen  =         vector[2].len;
				FS_printf("FS_Readdir: Listing entries...\n");
			}
			else {
				FS_printf("FS_Readdir: ERROR: Wrong number of input/output arguments!!!\n");
				return FS_EFATAL;
			}

			FS_printf("FS_Readdir: Generating fat path...\n");
			/* Generate path */
			FAT_GeneratePath(dirpath, __fatpath);
			FS_printf("FS_Readdir: Reading fat dir %s ...\n", __fatpath);

			/* Read directory */
			ret = FAT_ReadDir(__fatpath, outbuf, buflen, outlen, entries);
			FS_printf("FS_Readdir: ret = %d\n",ret);
			if (ret >= 0) {
				os_sync_after_write(outlen, sizeof(u32));

				/* Flush cache */
				if (outbuf)
					os_sync_after_write(outbuf, buflen);
			}

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

#ifdef DEBUG
			static char *dbgstr = "FS_GetUsage: Disc based game = %s\n";
#endif

			FS_printf("FS_GetUsage: Emulating...\n");

			// This is just an ugly workaround.
			// WiiWare and VirtualConsole seem to work
			// better with fake values, while disc-based 
			// Wii games (for example SSBB and MPT) seem
			// to work better with values taken from FAT.
			if (syscode != 'R' && syscode != 'S') {

				FS_printf(dbgstr,"FALSE");

				/* Set fake values */
				*blocks = 1;
				*inodes = 1;

				ret = FS_SUCCESS;
			}
			else {
				s32   fake;

				FS_printf(dbgstr,"TRUE");

				*blocks = 0;
				*inodes = 1;        // empty folders return a file count of 1

				fake = __FS_FakeUsage(dirpath, blocks, inodes);
			
				/* Generate path */
				FAT_GeneratePath(dirpath, __fatpath);

				if (fake) {
					/* Check path */
					ret = FAT_GetStats(__fatpath, NULL);
				}  
				else {
					/* Get usage */
					ret = FAT_GetUsage(__fatpath, blocks, inodes);
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
	case IOCTL_ISFS_SETCONFIG: {

		/* Set flag */
		*performed = 1;

		/* Check input */
		if (vector == NULL || inlen == 0)
			return FS_EFATAL;

		fsconfig cfg = { 0 };

		/* Get mode */
		cfg.mode = *(u32 *)vector[0].data;

		/* Get partition */
		cfg.partition = __partition;

		/* Get nand path */
		char *nandpath = (inlen > 1) ? (char *)vector[1].data : "";
		strncpy(cfg.nandpath, nandpath, sizeof(cfg.nandpath));
		config.nandpath[sizeof(cfg.nandpath)-1] = '\0';
		
		return __FS_SetConfig(&cfg, 0);
	}

	/** Mount USB or SD card **/
	case IOCTL_ISFS_FAT_MOUNT_USB:
	case IOCTL_ISFS_FAT_MOUNT_SD: {
		/* Set flag */
		*performed = 1;

		/* Can not mount FAT when a title is running */
		if (Stealth_CheckRunningTitle("IOCTL_ISFS_FAT_MOUNT"))
			return FS_EFATAL;

		/* Set device */
		u32 device = (cmd==IOCTL_ISFS_FAT_MOUNT_SD ? FS_MODE_SDHC : FS_MODE_USB);

		/* Set partition */
		__partition = inlen > 0 ? *(s32 *)vector[0].data : -1;

		FS_printf("FS_FatMount(%d, %d)\n", device, __partition);

		/* Mount SD card */
		return __FS_FAT_Mount(device, __partition);
	}

	/** Unmount USB or SD card **/
	case IOCTL_ISFS_FAT_UMOUNT_USB:
	case IOCTL_ISFS_FAT_UMOUNT_SD: {
		/* Set flag */
		*performed = 1;

		/* Can not mount FAT when a title is running */
		if (Stealth_CheckRunningTitle("IOCTL_ISFS_FAT_UMOUNT"))
			return FS_EFATAL;

		/* Set device */
		u32 device = (cmd==IOCTL_ISFS_FAT_UMOUNT_SD ? FS_MODE_SDHC : FS_MODE_USB);

		FS_printf("FS_FatUnmount(%d)\n", device);

		/* Unmount SD card */
		return FAT_Unmount(device);
	}

	default:
		FS_printf("FS_Ioctlv(): Unknown command 0x%x\n", cmd);
		break;
	}

	/* Call handler */
	return -6;
}
