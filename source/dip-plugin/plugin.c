/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
 * Copyright (C) 2011 davebaol, WiiPower, oggzee.
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
#include "fat.h"
#include "ioctl.h"
#include "plugin.h"
#include "syscalls.h"
#include "wbfs.h"
#include "frag.h"
#include "string.h"

/* Global config */
static struct dipConfig config = { 0 };
static struct dipConfigState dipCfgState = { 0 };
static struct ffsConfigState ffsCfgState = { 0 };

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

	/* Frag read */
	if (DI_ChkMode(MODE_FRAG))
		return Frag_Read(outbuf, len, offset);

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
	s32   ret;

	/* Allocate buffer */
	buffer = DI_Alloc(SECTOR_SIZE, 32);
	if (!buffer)
		return;

	// FIX d2x v7beta1
	// This fix properly detects DL games like Sakura Wars.
	// The old offset 0x50000000 fails for that game, even though it works 
	// for games having bigger size.
	/* Read second layer */
	ret = __DI_ReadUnencrypted(buffer, SECTOR_SIZE, 0x47000000);

	/* Set disc type */
	config.type = (ret) ? DISC_DVD5 : DISC_DVD9;

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
			if (DI_ChkMode(MODE_EMUL)) {
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
		if (!DI_ChkMode(MODE_EMUL)) {
			/* Call command */
			ret = DI_HandleCmd(inbuf, outbuf, size);

			/* Set DVD mode */
			if (ret)
				DI_SetMode(MODE_DVDROM);
		}

		/* Manual read */
		if (DI_ChkMode(MODE_DVDROM | MODE_EMUL) || offset)
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
		res = DI_ChkMode(MODE_DVDROM | MODE_EMUL);

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
		res = DI_ChkMode(MODE_DVDROM | MODE_EMUL);

		/* Seek disc */
		if (!res)
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Audio config **/
	case IOCTL_DI_AUDIO_CONFIG: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_EMUL);

		/* Set audio config  */
		if (!res)
			ret = DI_HandleCmd(inbuf, outbuf, size);

		break;
	}

	/** Report DVD key **/
	case IOCTL_DI_REPORT_KEY: {
		/* Check modes */
		res = DI_ChkMode(MODE_DVDROM | MODE_EMUL);

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
		res = DI_ChkMode(MODE_EMUL);

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
		res = DI_ChkMode(MODE_EMUL);

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
				dipCfgState.mode      = config.mode;
				dipCfgState.device    = device;
				dipCfgState.frag_size = 0;
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

	/** Set FRAG mode **/
	case IOCTL_DI_FRAG_SET: {
		u32   device   = inbuf[1];
		void *fraglist = (void*)inbuf[2];
		u32   size     = inbuf[3];

		/* Close frag */
		Frag_Close();

		/* Disable mode */
		DI_DelMode(MODE_FRAG);

		/* Check device */
		if (device && fraglist && size) {
			/* Convert address */
			fraglist = VirtToPhys(fraglist);

			/* Open device */
			ret = Frag_Init(device, fraglist, size);
			*outbuf = ret;

			/* Enable mode */
			if (!ret) {
				DI_SetMode(MODE_FRAG);

				/* Set frag state for ios reload */
				dipCfgState.mode      = config.mode;
				dipCfgState.device    = device;
				dipCfgState.frag_size = size;
			}

			ret = 0;
		}
		break;
	}

	/** Get IO mode **/
	case IOCTL_DI_MODE_GET: {
		/* return all mode bits */
		*outbuf = config.mode;
		break;
	}

	/** Save config **/
	case IOCTL_DI_SAVE_CONFIG: {

		DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Getting nand emulation config from FFS...\n");

		/* Get nand emulation config */
		ret = ISFS_GetMode(&ffsCfgState.mode, ffsCfgState.path);
		if (ret < 0)
			break;

		DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Nand emulation is currently %senabled.\n", ffsCfgState.mode ? "": "NOT ");

		if (ffsCfgState.mode) {
			u32 device = (ffsCfgState.mode & ISFS_MODE_SDHC) ? 0 : 1;

			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Getting partition currently mounted on %s...\n", device ? "USB device" : "SD card");

			/* Get FAT partition */
			FAT_GetPartition(device, &ffsCfgState.partition);

			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Current partition is %d\n", ffsCfgState.partition);
			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Disabling nand emulation\n");

			/* Disable nand emulation */
			ISFS_SetMode(0, "");

			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Saving FFS config state..\n");

			/* Save FFS config */
			ret = FFS_Config_Save(&ffsCfgState);
			if (ret < 0)
				break;

			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: FFS config state saved\n");
		}

		/* Check DIP modes */
		if (DI_ChkMode(MODE_EMUL)) {
			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: Saving DIP config state..\n");

			/* Save DIP config */
			ret = DI_Config_Save(&dipCfgState);
			if (ret < 0)
				break;

			DI_Printf("DIP: IOCTL_DI_SAVE_CONFIG: DIP config state saved\n");
		}

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
		res = DI_ChkMode(MODE_EMUL);

		/* Wait for cover close */
		if (!res)
			ret = DI_HandleIoctl(buffer, fd);

		break;
	}

	/** Get cover register **/
	case IOCTL_DI_COVER_REG: {
		/* Check modes */
		res = DI_ChkMode(MODE_EMUL);

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
		res = DI_ChkMode(MODE_EMUL);

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
		res = DI_ChkMode(MODE_EMUL);

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
		res = DI_ChkMode(MODE_EMUL);

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

void __DI_InitEmulation(void)
{
	s32 ret;

	DI_Printf("DIP: DI_InitEmulation: Loading DIP config state...\n");

	/* Load DIP config state */
	ret = DI_Config_Load(&dipCfgState);

	DI_Printf("DIP: DI_InitEmulation: DIP config state %sfound. ret = %d\n", ret>0 ? "" : "NOT ", ret);

	/* DIP Config state found */
	if (ret > 0) {

		DI_Printf("DIP: DI_InitEmulation: Setting DIP mode to %d\n", dipCfgState.mode);

		/* Set mode */
		config.mode = dipCfgState.mode;

		if (DI_ChkMode(MODE_WBFS)) {
			/* Open wbfs device */
			WBFS_Open(dipCfgState.device-1, (u8 *)0x00000000);
		}
		else if (DI_ChkMode(MODE_FRAG)) {
			/* Open fraglist */
			Frag_Init(dipCfgState.device, &fraglist_data, dipCfgState.frag_size);
		}
	}

	DI_Printf("DIP: DI_InitEmulation: Loading FFS config state...\n");

	/* Load FFS config state */
	ret = FFS_Config_Load(&ffsCfgState);

	DI_Printf("DIP: DI_InitEmulation: FFS config state %sfound. ret = %d\n", ret>0 ? "" : "NOT ", ret);

	/* FFS Config state found */
	if (ret > 0 && ffsCfgState.mode) {
		u32 device = (ffsCfgState.mode & ISFS_MODE_SDHC) ? 0 : 1;

		DI_Printf("DIP: DI_InitEmulation: Mounting FAT partition %d on %s...\n", ffsCfgState.partition, device ? "USB device" : "SD card");

		/* Mount FAT device */
		FAT_Mount(device, ffsCfgState.partition);

		DI_Printf("DIP: DI_InitEmulation: Enabling nand emulation: mode = %d, path = &s%d\n", ffsCfgState.mode, ffsCfgState.path);

		/* Enable nand emulation */
		ISFS_SetMode(ffsCfgState.mode, ffsCfgState.path);

		DI_Printf("DIP: DI_InitEmulation: Nand emulation enabled\n");
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

	/* Init DIP and FFS after the cIOS has been reloaded */
	__DI_InitEmulation();
}
