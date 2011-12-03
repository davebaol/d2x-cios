/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
 * Copyright (C) 2011 davebaol, WiiPower.
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
#include "config.h" 
#include "dip.h"
#include "dip_calls.h"
#include "errno.h"
#include "file.h"
#include "ioctl.h"
#include "plugin.h"
#include "syscalls.h"
#include "wbfs.h"

/* Global config */
struct dipConfig config = { 0 };
struct dipConfigState cfgState = { 0 };

s32 __DI_CheckOffset(u32 offset)
{
	u32 offmax;

	/* Check disc type */
	switch (config.type) {
	/* Single layer */
	case DISC_DVD5:
		offmax = DVD5_LENGTH;
		break;

	/* Dual layer */
	case DISC_DVD9:
		offmax = DVD9_LENGTH;
		break;

	default:
		return 0;
	}

	/* Check offset */
	if (offset >= offmax) {
		/* Set error */
		config.error = ERROR_BLOCK_RANGE;

		/* I/O error */
		return DIP_EIO;
	}

	return 0;
}

s32 __DI_ReadUnencrypted(void *outbuf, u32 len, u32 offset)
{
	s32 ret;

	/* Check offset */
	ret = __DI_CheckOffset(offset);
	if (ret)
		return ret;

	/* Update offset */
	offset += (config.offset[0] + config.offset[1]);

	/* File read */
	if (DI_ChkMode(MODE_FILE))
		return File_Read(outbuf, len, offset);

	/* WBFS read */
	if (DI_ChkMode(MODE_WBFS))
		return WBFS_Read(outbuf, len, offset);

	/* DVD read */
	if (DI_ChkMode(MODE_DVDROM))
		return DI_ReadDvd(outbuf, len, offset);

	/* WOD read */
	return DI_ReadWod(outbuf, len, offset);
}

s32 __DI_ReadDiscId(u32 *outbuf, u32 len)
{
	s32 ret;

	/* Read ID (first sector) */
	ret = __DI_ReadUnencrypted(outbuf, len, 0);
	if (ret < 0)
		return ret;

	/* Check WOD magic word */
	if (outbuf[6] == WOD_MAGIC) {
		extern u8 *dip_readctrl;

		/* Set read control */
		dip_readctrl[0] = 1;

		/* Read hash */
		if (!dip_readctrl[1])
			ret = DI_ReadHash();
	}

	return ret;
}

void __DI_CheckDisc(void)
{
	void *buffer;
	s32   ret, cnt;
	u32   offset[3] = {0x50000000, 0x60000000, 0x70000000};

	/* Allocate buffer */
	buffer = DI_Alloc(SECTOR_SIZE, 32);
	if (!buffer)
		return;

	// FIX d2x v5beta1
	// This is a workaround to properly detect DL games like Sakura Wars.
	// Offset 0x50000000 fails for that game, even though it works 
	// for games having bigger size. Surprisingly higher offsets succeed.
	// Not sure, but this can be related to PTP/OTP.
	// See http://gbatemp.net/t277659-ciosx-rev21d2x-yet-another-hot-fix?view=findpost&p=3667487
	for (cnt=0, ret=1; cnt<3 && ret; cnt++) {
		/* Read second layer */
		ret = __DI_ReadUnencrypted(buffer, SECTOR_SIZE, offset[cnt]);
	}

	/* Set disc type */
	config.type = (!ret) ? DISC_DVD9 : DISC_DVD5;

	/* Free buffer */
	DI_Free(buffer);
}

void __DI_ResetConfig(void)
{
	/* Reset modes */
	DI_DelMode(MODE_CRYPT);
	DI_DelMode(MODE_DVDROM);

	/* Reset offsets */
	config.offset[0] = 0;
	config.offset[1] = 0;

	/* Reset variables */
	config.type    = 0;
	config.error   = 0;
	config.cover   = 0;
	config.noreset = 0;
}


s32 DI_EmulateCmd(u32 *inbuf, u32 *outbuf, u32 size)
{
	u32 cmd = (inbuf[0] >> 24);

	s32 res;
	s32 ret = 0;

	/* Reset error */
	if (cmd != IOCTL_DI_REQERROR)
		config.error = 0;

	switch(cmd) {
	/** Reset drive **/
	case IOCTL_DI_RESET: {
		/* Check reset flag */
		if (!config.noreset) {
			/* Reset DIP config */
			__DI_ResetConfig();

			/* Non-DVD mode */
			if (DI_ChkMode(MODE_FILE | MODE_WBFS)) {
				/* Stop motor */
				DI_StopMotor();

				/* Set cover register */
				BIT_SET(config.cover, 4);
			} else {
				/* Reset drive */
				ret = DI_HandleCmd(inbuf, outbuf, size);
			}
		}

		break;
	}

	/** Read disc ID **/
	case IOCTL_DI_READID: {
		u32 offset = (config.offset[0] | config.offset[1]);

		/* Read disc ID */
		if (!DI_ChkMode(MODE_FILE | MODE_WBFS)) {
			/* Call command */
			ret = DI_HandleCmd(inbuf, outbuf, size);

			/* Set DVD mode */
			if (ret)
				DI_SetMode(MODE_DVDROM);
		}

		/* Manual read */
		if (DI_ChkMode(MODE_DVDROM | MODE_FILE | MODE_WBFS) || offset)
			ret = __DI_ReadDiscId(outbuf, size);

		/* Check disc type */
		if (!ret) {
			// Fix d2x v5beta1
			// For gamecube games it's not required to know the disc type,
			// but DL access has to be possible for GC DL multi game discs.
			// If there was a read executed here for GC games, 
			// IOCTL_DI_AUDIO_CONFIG would fail(->audio streaming error).
			if (outbuf[7] == GC_MAGIC)
				config.type = DISC_DVD9;
			else
				__DI_CheckDisc();
		}

		break;
	}

	/** Encrypted disc read **/
	case IOCTL_DI_LOW_READ: {
		/* Crypted read */
		if (DI_ChkMode(MODE_CRYPT)) {
			u32 len    = inbuf[1];
			u32 offset = inbuf[2];

			/* Do unencrypted read */
			ret = __DI_ReadUnencrypted(outbuf, len, offset);
		} else
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Unencrypted disc read **/
	case IOCTL_DI_UNENCREAD:
	case IOCTL_DI_READ_A8:
	case IOCTL_DI_READ_D0: {
		u32 len    = inbuf[1];
		u32 offset = inbuf[2];

		/* Change values unit */
		if (cmd == IOCTL_DI_READ_D0) {
			len    <<= 11;
			offset <<= 9;
		}

		/* Unencrypted read */
		ret = __DI_ReadUnencrypted(outbuf, len, offset);

		break;
	}

	/** Disc BCA read **/
	case IOCTL_DI_READBCA: {
		/* Read disc BCA */
		ret = __DI_ReadUnencrypted(outbuf, size, 0x40);

		break;
	}

	/** Set drive offset **/
	case IOCTL_DI_OFFSET: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_FILE | MODE_WBFS);

		/* Set disc offset */
		if (res) {
			/* Calculate offset */
			u32 offset = (inbuf[1] << 30) | inbuf[2];

			/* Set drive offset */
			config.offset[1] = (offset & -0x8000);
		} else
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Seek disc **/
	case IOCTL_DI_SEEK: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_FILE | MODE_WBFS);

		/* Seek disc */
		if (!res)
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Audio config **/
	case IOCTL_DI_AUDIO_CONFIG: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_FILE | MODE_WBFS);

		/* Set audio config  */
		if (!res)
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Report DVD key **/
	case IOCTL_DI_REPORT_KEY: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_FILE | MODE_WBFS);

		/* Report DVD key */
		if (res) {
			/* Wrong disc */
			config.error = ERROR_WRONG_DISC;

			/* I/O error */
			ret = DIP_EIO;
		} else
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Request cover status **/
	case IOCTL_DI_REQCOVER: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Request cover status */
		if (res)
			*outbuf = 0;
		else
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Request error code **/
	case IOCTL_DI_REQERROR: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Request error code */
		if (res || config.error)
			*outbuf = config.error;
		else
			ret = DI_HandleCmd(inbuf, outbuf, size);
		
		break;
	}

	/** Set offset base **/
	case IOCTL_DI_OFFSET_SET: {
		u32 offset = inbuf[1];

		/* Set base offset */
		config.offset[0] = offset;

		break;
	}

	/** Get offset base **/
	case IOCTL_DI_OFFSET_GET: {
		/* Return offset base */
		*outbuf = config.offset[0];

		break;
	}

	/** Set crypt mode **/
	case IOCTL_DI_CRYPT_SET: {
		u32 mode = inbuf[1];

		/* Enable crypt mode */
		if (mode)
			DI_SetMode(MODE_CRYPT);
		else
			DI_DelMode(MODE_CRYPT);

		break;
	}

	/** Get crypt mode **/
	case IOCTL_DI_CRYPT_GET: {
		/* Check crypt bit */
		*outbuf = DI_ChkMode(MODE_CRYPT);

		break;
	}

	/** Set WBFS mode **/
	case IOCTL_DI_WBFS_SET: {
		u32 device = inbuf[1];

		/* Close WBFS */
		WBFS_Close();

		/* Disable mode */
		DI_DelMode(MODE_WBFS);

		/* Check device */
		if (device) {
			u8 *discid = (u8 *)&inbuf[2];

			/* Open device */
			ret = WBFS_Open(device-1, discid);

			if (!ret) {
				/* Enable mode */
				DI_SetMode(MODE_WBFS);

				/* Set wbfs state for ios reload */
				cfgState.mode = config.mode;
				cfgState.wbfs_device = device;
			}
		}

		break;
	}

	/** Get WBFS mode **/
	case IOCTL_DI_WBFS_GET: {
		/* Check WBFS bit */
		*outbuf = DI_ChkMode(MODE_WBFS);

		break;
	}

	/** Set file mode **/
	case IOCTL_DI_FILE_SET: {
		char *filename = (char *)inbuf[1];

		/* Close file */
		File_Close();

		/* Disable mode */
		DI_DelMode(MODE_FILE);

		/* Check flag */
		if (filename) {
			/* Convert address */
			filename = VirtToPhys(filename);

			/* Open file */
			ret = File_Open(filename);

			/* Enable mode */
			if (!ret) {
				DI_SetMode(MODE_FILE);

				/* Set file state for ios reload */
				cfgState.mode = config.mode;
				strncpy((char*) &cfgState.fat_filename, filename, FILENAME_MAX_LEN);
			}
		}

		break;
	}

	/** Get file mode **/
	case IOCTL_DI_FILE_GET: {
		/* Check file bit */
		*outbuf = DI_ChkMode(MODE_FILE);

		break;
	}

	/** Save config **/
	case IOCTL_DI_SAVE_CONFIG: {
		/* Check modes */
		if (DI_ChkMode(MODE_WBFS)) {
			/* Save WBFS config */
			ret = Config_Save(&cfgState, sizeof(cfgState.mode)+sizeof(cfgState.wbfs_device));
		}
		else if (DI_ChkMode(MODE_FILE)) {
			/* Save FAT config */
			ret = Config_Save(&cfgState, sizeof(cfgState.mode)+sizeof(cfgState.fat_filename));
		}

		if (ret>0)
			ret = 0;

		break;
	}

	/** Disable reset **/
	case IOCTL_DI_RESET_DISABLE: {
		u32 value = inbuf[1];

		/* Disable reset */
		config.noreset = value;

		break;
	}

	/** Send custom DVD command **/
	case IOCTL_DI_CUSTOMCMD: {
		void *buffer = (void *)inbuf[1];

		/* Convert address to physical */
		buffer = VirtToPhys(buffer);

		/* Send custom DI command */
		ret = DI_CustomCmd(buffer, outbuf);

		break;
	}

	default:
		/* Call command */
		ret = DI_HandleCmd(inbuf, outbuf, size);
	}

	return ret;
}

s32 DI_EmulateIoctl(ioctl *buffer, s32 fd)
{
	u32 *outbuf = buffer->iobuf;
	u32  cmd    = buffer->command;

	s32 res;
	s32 ret = 1;

	/* Parse command */
	switch (cmd) {
	/** Wait for cover close **/
	case IOCTL_DI_WAITCVRCLOSE: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Wait for cover close */
		if (!res)
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	/** Get cover register **/
	case IOCTL_DI_COVER_REG: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Get cover register */
		if (res)
			*outbuf = config.cover;
		else
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	/** Clear cover interrupt **/
	case IOCTL_DI_COVER_CLEAR: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Clear cover interrupt */
		if (res)
			BIT_DEL(config.cover, 4);
		else
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	/** Get cover status **/
	case IOCTL_DI_COVER_STATUS: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Get cover status */
		if (res)
			*outbuf = 0x02;
		else
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	/** Get status register **/
	case IOCTL_DI_STATUS_REG: {
		/* Check modes */
		res = DI_ChkMode(MODE_FILE | MODE_WBFS);

		/* Get status register */
		if (res)
			*outbuf = 0x0A;
		else
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	default:
		/* Call IOCTL */
		ret = DI_HandleIoctl(buffer, fd);
	}

	return ret;
}

void __DI_InitEmulationDevice(void)
{
	/* Load config state */
	u32 ret = Config_Load(&cfgState, sizeof(cfgState)) ;

	/* Config state found */
	if (ret>sizeof(cfgState.mode)) {

		ret -= sizeof(cfgState.mode);
		
		/* Set mode */
		config.mode = cfgState.mode;

		if (DI_ChkMode(MODE_WBFS) && ret==sizeof(cfgState.wbfs_device)) {
			/* Open wbfs device */
			WBFS_Open(cfgState.wbfs_device-1, (u8 *)0x00000000);
		}
		else if (DI_ChkMode(MODE_FILE) && ret==sizeof(cfgState.fat_filename)) {
			/* Open file */
			File_Open((char*) &cfgState.fat_filename);
		}
	}
}

/*
 * Added in cIOS d2x v5 alpha1 in order 
 * to support ios reload block through USB/SD
 *  
 * NOTE:
 * This function is redirected from DI_InitStage2 included inside 
 * the original Nintendo DI module and it is invoked only once 
 * as soon as the dvd driver receives the first ipc message of type 
 * open, ioctl or ioctlv.  
 */
void DI_EmulateInitStage2(void)
{
	/* Init DVD driver */
	DI_InitStage2();

	/* Init USB/SD after the cIOS has been reloaded */
	__DI_InitEmulationDevice();
}