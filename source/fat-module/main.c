/*   
	Custom IOS Module (FAT)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 WiiGator.
	Copyright (C) 2010 Waninkoko.
	Copyright (C) 2011 davebaol.

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

#include <string.h>

#include "ehci.h"
#include "fat_wrapper.h"
#include "ipc.h"
#include "mem.h"
#include "module.h"
#include "sdio.h"
#include "syscalls.h"
#include "timer.h"
#include "types.h"


s32 __FAT_Ioctl(s32 fd, u32 cmd, void *inbuf, u32 inlen, void *iobuf, u32 iolen)
{
	s32 ret = IPC_EINVAL;

	/* Invalidate cache */
	if (inbuf)
		os_sync_before_read(inbuf, inlen);

	/* Parse command */
	switch (cmd) {
	/** Get file stats **/
	case IOCTL_FAT_FILESTATS: {
		void *stats = iobuf;

		/* Get file stats */
		ret = FAT_GetFileStats(fd, stats);

		break;
	}

	default:
		break;
	}

	/* Flush cache */
	if (iobuf)
		os_sync_after_write(iobuf, iolen);

	return ret;
}

s32 __FAT_Ioctlv(s32 fd, u32 cmd, ioctlv *vector, u32 inlen, u32 iolen)
{
	s32 ret = IPC_EINVAL;

	/* Invalidate ache */
	InvalidateVector(vector, inlen, iolen);

	/* Parse command */
	switch (cmd) {
//----------------------------------------------------------------
//--------- COMMANDS USED BY FFS MODULE FOR LOGGING --------------
//----------------------------------------------------------------
	/** Open file **/
	case IOCTL_FAT_OPEN: {
		char *filepath = (char *)vector[0].data;
		u32   mode     = *(u32 *)vector[1].data;

		/* Open file */
		ret = FAT_Open(filepath, mode);

		break;
	}

	/** Close file **/
	case IOCTL_FAT_CLOSE: {
		s32 cfd = *(s32 *)vector[0].data;

		/* Close file */
		ret = FAT_Close(cfd);

		break;
	}

	/** Read file **/
	case IOCTL_FAT_READ: {
		s32 cfd = *(s32 *)vector[0].data;

		void *buffer = vector[1].data;
		u32   len    = vector[1].len;

		/* Read file */
		ret = FAT_Read(cfd, buffer, len);

		break;
	}

	/** Write file **/
	case IOCTL_FAT_WRITE: {
		s32 cfd = *(s32 *)vector[0].data;

		void *buffer = vector[1].data;
		u32   len    = vector[1].len;

		/* Write file */
		ret = FAT_Write(cfd, buffer, len);

		break;
	}

	/** Seek file **/
	case IOCTL_FAT_SEEK: {
		s32 cfd = *(s32 *)vector[0].data;

		u32 where  = *(u32 *)vector[1].data;
		u32 whence = *(u32 *)vector[2].data;

		/* Seek file */
		ret = FAT_Seek(cfd, where, whence);

		break;
	}
//----------------------------------------------------------------
//-------- END OF COMMANDS USED BY FFS MODULE FOR LOGGING --------
//----------------------------------------------------------------
	
	/** Create directory **/
	case IOCTL_FAT_MKDIR: {
		char *dirpath = (char *)vector[0].data;

		/* Create directory */
		ret = FAT_CreateDir(dirpath);

		break;
	}

	/** Create file **/
	case IOCTL_FAT_MKFILE: {
		char *filepath = (char *)vector[0].data;

		/* Create file */
		ret = FAT_CreateFile(filepath);

		break;
	}

	/** Read directory **/
	case IOCTL_FAT_READDIR_FS:
	case IOCTL_FAT_READDIR: {
		char *dirpath = (char *)vector[0].data;
		char *outbuf  = NULL;
		u32  *outlen  = NULL;

		u32 buflen  = 0;
		u32 entries = 0, lfn;

		/* Input values */
		if (iolen > 1) {
			entries = *(u32 *)vector[1].data;
			outbuf  = (char *)vector[2].data;
			outlen  =  (u32 *)vector[3].data;
			buflen  =         vector[2].len;
		} else
			outlen =  (u32 *)vector[1].data;

		/* Set LFN flag */
		lfn = (cmd == IOCTL_FAT_READDIR);

		/* Read directory */
		ret = FAT_ReadDir(dirpath, outbuf, buflen, outlen, entries, lfn);

		break;
	}

	/** Delete object **/
	case IOCTL_FAT_DELETE: {
		char *path = (char *)vector[0].data;

		/* Delete file/directory */
		ret = FAT_Delete(path);

		break;
	}

	/** Delete directory **/
	case IOCTL_FAT_DELETEDIR: {
		char *dirpath = (char *)vector[0].data;

		/* Delete directory */
		ret = FAT_DeleteDir(dirpath);

		break;
	}

	/** Rename object **/
	case IOCTL_FAT_RENAME: {
		char *oldname = (char *)vector[0].data;
		char *newname = (char *)vector[1].data;

		/* Rename object */
		ret = FAT_Rename(oldname, newname);

		break;
	}

	/** Get stats **/
	case IOCTL_FAT_STATS: {
		char *path  = (char *)vector[0].data;
		void *stats = (void *)vector[1].data;

		/* Get stats */
		ret = FAT_GetStats(path, stats);

		break;
	}

	/** Get usage **/
	case IOCTL_FAT_GETUSAGE: {
		char *path  = (char *)vector[0].data;
		u64  *size  =  (u64 *)vector[1].data;
		u32  *files =  (u32 *)vector[2].data;

		/* Get usage */
		ret = FAT_GetUsage(path, size, files);

		break;
	}

	/** Mount SD card **/
	case IOCTL_FAT_MOUNT_SD: {
		u32 partition = 0;

		/* Initialize SDIO */
		ret = sdio_Startup();
		if (!ret) {
			ret = IPC_EINVAL;
			break;
		}

		/* Set partition */
		if(inlen > 0)
			partition = *(u32 *)vector[0].data;

		/* Mount SD card */
		ret = FAT_Mount(0, (u8)partition);

		break;
	}

	/** Unmount SD card **/
	case IOCTL_FAT_UMOUNT_SD: {
		/* Unmount SD card */
		ret = FAT_Unmount(0);
		if (ret < 0)
			break;

		/* Deinitialize SDIO */
		ret = sdio_Shutdown();
		if (!ret)
			ret = IPC_EINVAL;

		break;
	}

	/** Mount USB device **/
	case IOCTL_FAT_MOUNT_USB: {
		u32 partition = 0;
	
		/* Initialize EHCI */
		ret = ehci_Init();
		if (!ret) {
			ret = IPC_EINVAL;
			break;
		}

		/* Set partition */
		if(inlen > 0)
			partition = *(u32 *)vector[0].data;

		/* Mount USB device */
		ret = FAT_Mount(1, (u8)partition);

		break;
	}

	/** Unmount USB card **/
	case IOCTL_FAT_UMOUNT_USB: {
		/* Unmount USB */
		ret = FAT_Unmount(1);
		if (ret < 0)
			break;

		/* Deinitialize USB */
		ret = ehci_Shutdown();
		if (!ret)
			ret = IPC_EINVAL;

		break;
	}

	default:
		break;
	}

	/* Flush cache */
	FlushVector(vector, inlen, iolen);

	return ret;
}

s32 __FAT_Initialize(u32 *queuehandle)
{
	void *buffer = NULL;
	s32   ret;

	/* Initialize memory heap */
	Mem_Init();

	/* Initialize timer subsystem */
	Timer_Init();

	/* Allocate queue buffer */
	buffer = Mem_Alloc(0x80);
	if (!buffer)
		return IPC_ENOMEM;

	/* Create message queue */
	ret = os_message_queue_create(buffer, 32);
	if (ret < 0)
		return ret;

	/* Register device */
	os_device_register(DEVICE_FAT, ret);

	/* Copy queue handler */
	*queuehandle = ret;

	return 0;
}


int main(void)
{
	u32 queuehandle;
	s32 ret;

	/* Print info */
	write("$IOSVersion: FAT: " __DATE__ " " __TIME__ " 64M$\n");

	/* Initialize module */
	ret = __FAT_Initialize(&queuehandle);
	if (ret < 0)
		return ret;

	/* Main loop */
	while (1) {
		ipcmessage *message = NULL;

		/* Wait for message */
		os_message_queue_receive(queuehandle, (void *)&message, 0);

		switch (message->command) {
		case IOS_OPEN: {
			char *devpath = message->open.device;
			u32   mode    = message->open.mode;
			u32   len;

			/* Prefix length */
			len = strlen(DEVICE_FAT);

			/* Open module */
			if (!strcmp(devpath, DEVICE_FAT)) {
				ret = 0;
				break;
			}

			/* Open file */
			if (!strncmp(devpath, DEVICE_FAT, len)) {
				/* Open file */
				ret = FAT_Open(devpath + len, mode);
				break;
			}

			/* Error */
			ret = IPC_EINVAL;

			break;
		}

		case IOS_CLOSE: {
			/* Close file */
			ret = FAT_Close(message->fd);

			break;
		}

		case IOS_READ: {
			void *buffer = message->read.data;
			u32   len    = message->read.length;

			/* Read file */
			ret = FAT_Read(message->fd, buffer, len);

			/* Flush cache */
			os_sync_after_write(buffer, len);

			break;
		}

		case IOS_WRITE: {
			void *buffer = message->write.data;
			u32   len    = message->write.length;

			/* Invalidate cache */
			os_sync_before_read(buffer, len);

			/* Write file */
			ret = FAT_Write(message->fd, buffer, len);

			break;
		}

		case IOS_SEEK: {
			s32 where  = message->seek.offset;
			s32 whence = message->seek.origin;

			/* Seek file */
			ret = FAT_Seek(message->fd, where, whence);

			break;
		}

		case IOS_IOCTL: {
			void *inbuf = message->ioctl.buffer_in;
			void *iobuf = message->ioctl.buffer_io;
			u32   inlen = message->ioctl.length_in;
			u32   iolen = message->ioctl.length_io;
			u32   cmd   = message->ioctl.command;

			/* Parse IOCTL */
			ret = __FAT_Ioctl(message->fd, cmd, inbuf, inlen, iobuf, iolen);

			break;
		}

		case IOS_IOCTLV: {
			ioctlv *vector = message->ioctlv.vector;
			u32     inlen  = message->ioctlv.num_in;
			u32     iolen  = message->ioctlv.num_io;
			u32     cmd    = message->ioctlv.command;

			/* Parse IOCTLV */
			ret = __FAT_Ioctlv(message->fd, cmd, vector, inlen, iolen);

			break;
		}

		default:
			/* Unknown command */
			ret = IPC_EINVAL;
		}

		/* Acknowledge message */
		os_message_queue_ack((void *)message, ret);
	}
   
	return 0;
}
