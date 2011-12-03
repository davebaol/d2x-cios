/*
 * DIP plugin for Custom IOS.
 *
 * Copyright (C) 2008-2010 Waninkoko, WiiGator.
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

#include "dip.h"
#include "dip_calls.h"
#include "dma.h"
#include "errno.h"
#include "ioctl.h"
#include "syscalls.h"

/* Constants */
#define MAX_READ_RETRIES	16


s32 __DI_ReadA8(void *outbuf, u32 len, u32 offset)
{
	u32 dic[8];

	/* Clear buffer */
	memset(dic, 0, sizeof(dic));

	/* Prepare input buffer */
	dic[0] = IOCTL_DI_READ_A8 << 24;
	dic[1] = len;
	dic[2] = offset;

	/* Invalidate cache */
	os_sync_before_read(outbuf, len);

	/* Call command */
	return DI_HandleCmd(dic, outbuf, len);
}

s32 __DI_ReadD0(void *outbuf, u32 len, u32 lba)
{
	u32 cnt;
	s32 ret = DIP_EIO;

	/* Do DVD read */
	for (cnt = 0; ret && (cnt < MAX_READ_RETRIES); cnt++) {
		u32 dic[8];

		/* Clear buffer */
		memset(dic, 0, sizeof(dic));

		/* Prepare input buffer */
		dic[0] = IOCTL_DI_READ_D0 << 24;
		dic[1] = 0;
		dic[2] = 0;
		dic[3] = len >> 11;
		dic[4] = lba;

		/* Invalidate cache */
		os_sync_before_read(outbuf, len);

		/* Call command */
		ret = DI_HandleCmd(dic, outbuf, len);
	}

	return ret;
}

s32 __DI_ReadFromSector(void *outbuf, u32 len, u32 pos, u32 lba)
{
	u8 *buf = NULL;
	s32 ret;

	/* Check length */
	if ((len + pos) > SECTOR_SIZE)
		return IPC_EINVAL;
	
	/* Allocate memory */
	buf = DI_Alloc(SECTOR_SIZE, 32);
	if (!buf)
		return IPC_ENOMEM;

	/* Read sector */
	ret = __DI_ReadD0(buf, SECTOR_SIZE, lba);

	/* Extract data */
	if (!ret)
		memcpy(outbuf, buf + pos, len);

	/* Free memory */
	DI_Free(buf);

	return ret;
}


u32 DI_CustomCmd(void *inbuf, void *outbuf)
{
	u32 *diRegs = (u32 *)0x0D006000;

	/* Set registers */
	memcpy(diRegs, inbuf, 32);

	/* Wait */
	while (diRegs[7] & 1);

	/* Copy registers */
	if (outbuf)
		memcpy(outbuf, diRegs, 32);

	return diRegs[8];
}

s32 DI_StopMotor(void)
{
	u32 dic[8];

	/* Prepare input buffer */
	dic[0] = IOCTL_DI_STOP_MOTOR << 24;
	dic[1] = 0;
	dic[2] = 0;

	/* Call command */
	return DI_HandleCmd(dic, NULL, 0);
}

s32 DI_ReadDvd(u8 *outbuf, u32 len, u32 offset)
{
	u32 cnt, lba, size;
	s32 ret = 0;

	/* Initial LBA */
	lba = offset >> 9;

	/* Do reads */
	for (cnt = 0; cnt < len; cnt += size) {
		u32 dmasize, offlba, pos;

		/* Get offset/size */
		size   = len - cnt;
		offlba = lba << 9;

		/* Position in sector */
		pos = (offset > offlba) ? (offset - offlba) << 2 : 0;

		/* Check DMA range */
		dmasize = DMA_CheckRange(outbuf + cnt, size, SECTOR_SIZE);

		/* Use proper read method */
		if (!dmasize || pos) {
			/* Check block size limit */
			if ((size + pos) > SECTOR_SIZE)
				size = SECTOR_SIZE - pos;

			/* Read sector */
			ret = __DI_ReadFromSector(outbuf + cnt, size, pos, lba);
		} else {
			/* Set block size */
			size = (dmasize > MAX_SECTOR_SIZE) ? MAX_SECTOR_SIZE : dmasize;

			/* Read data */
			ret = __DI_ReadD0(outbuf + cnt, size, lba);
		}

		/* Next LBA */
		lba += (size + pos) >> 11;
	}

	return ret;
}

s32 DI_ReadWod(void *outbuf, u32 len, u32 offset)
{
	/* Read data */
	return __DI_ReadA8(outbuf, len, offset);
}
