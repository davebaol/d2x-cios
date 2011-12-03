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

#include "types.h"

/* DMA constants */
#define DMA1_START_ADDRESS		0x00000000
#define DMA1_END_ADDRESS		0x01800000
#define DMA2_START_ADDRESS		0x10000000
#define DMA2_END_ADDRESS		0x13618000


s32 DMA_CheckRange(void *outbuf, u32 size, u32 alignment)
{
	u32 mem;
	s32 ret = 0;

	/* Output buffer address */
	mem = (u32)outbuf;

	/* Check for memory alignment */
	if (!(mem & 31)) {
		u32 dmalen = 0;

		/* DMA1 range check */
		if ((mem >= DMA1_START_ADDRESS) && (mem < DMA1_END_ADDRESS))
			dmalen = (DMA1_END_ADDRESS - mem);

		/* DMA2 range check */
		if ((mem >= DMA2_START_ADDRESS) && (mem < DMA2_END_ADDRESS))
			dmalen = (DMA2_END_ADDRESS - mem);

		if (dmalen >= alignment)
			ret  = (dmalen < size) ? dmalen : size;
			ret -= (ret & (alignment - 1));
	}

	return ret;
} 
