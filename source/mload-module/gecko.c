/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 Nuke.
	Copyright (C) 2009 marcan.
	Copyright (C) 2009 dhewg.
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

#include "types.h"
#include "utils.h"
#include "hollywood.h"
#include "gecko.h"

//#define GECKO_SAFE

static u8 console_enabled = 0;

static u32 __Gecko_Command(u32 command)
{
	u32 i;
	// Memory Card Port B (Channel 1, Device 0, Frequency 3 (32Mhz Clock))
	Write32(EXI1_CSR, 0xd0);
	Write32(EXI1_DATA, command);
	Write32(EXI1_CR, 0x19);
	while (Read32(EXI1_CR) & 1);
	i = Read32(EXI1_DATA);
	Write32(EXI1_CSR, 0);
	return i;
}

static u32 __Gecko_GetId(void)
{
	u32 i;
	// Memory Card Port B (Channel 1, Device 0, Frequency 3 (32Mhz Clock))
	Write32(EXI1_CSR, 0xd0);
	Write32(EXI1_DATA, 0);
	Write32(EXI1_CR, 0x19);
	while (Read32(EXI1_CR) & 1);
	Write32(EXI1_CR, 0x39);
	while (Read32(EXI1_CR) & 1);
	i = Read32(EXI1_DATA);
	Write32(EXI1_CSR, 0);
	return i;
}

static u32 __Gecko_SendByte(u8 sendbyte)
{
	u32 i = __Gecko_Command(0xB0000000 | (sendbyte<<20));

	/* Return 1 if byte was sent */
	return (i & 0x04000000) ? 1: 0;
}

static u32 __Gecko_ReceiveByte(u8 *recvbyte)
{
	u32 i = 0;
	*recvbyte = 0;
	i = __Gecko_Command(0xA0000000);
	if (i&0x08000000) {
		/* Return 1 if byte was received */
		*recvbyte = (i>>16)&0xff;
		return 1;
	}
	return 0;
}

static u32 __Gecko_IsAlive(void)
{
	u32 i;

	i = __Gecko_GetId();
	if (i == 0x00000000) {
		i = __Gecko_Command(0x90000000);
		if ((i & 0xFFFF0000) == 0x04700000)
			return 1;
	}

	return 0;
}

static void __Gecko_Flush(void)
{
	u8 tmp;
	while(__Gecko_ReceiveByte(&tmp));
}

#ifdef GECKO_SAFE
static u32 __Gecko_CheckSend(void)
{
	u32 i = __Gecko_Command(0xC0000000);

	/* Return 1 if safe to send */
	return i & 0x040000000 ? 1 : 0;
}

#include <string.h> 
static u32 Gecko_SendStringSafe(const char *string)
{
	char *ptr = (char*)string;

	if((Read32(HW_EXICTRL) & EXICTRL_ENABLE_EXI) == 0)
		return strlen(string);
	
	while(*ptr != '\0') {
		if(__Gecko_CheckSend()) {
			if(!__Gecko_SendByte(*ptr))
				break;
			if(*ptr == '\n' && !__Gecko_SendByte('\r'))
				break;
			ptr++;
		}
	}
	return (ptr - string);
}

#else

static u32 Gecko_SendStringUnsafe(const char *string)
{
	char *ptr = (char*)string;

	while(*ptr != '\0') {
		if(!__Gecko_SendByte(*ptr))
			break;
		if(*ptr == '\n' && !__Gecko_SendByte('\r'))
			break;
		ptr++;
	}
	return (ptr - string);
}

#endif

u32 Gecko_SendString(const char *string)
{
#ifdef GECKO_SAFE
	return console_enabled ? Gecko_SendStringSafe(string) : 0;
#else
	return console_enabled ? Gecko_SendStringUnsafe(string) : 0;
#endif
}

void Gecko_Init(void)
{
	Write32(EXI0_CSR, 0);
	Write32(EXI1_CSR, 0);
	Write32(EXI2_CSR, 0);
	Write32(EXI0_CSR, 0x2000);
	Write32(EXI0_CSR, 3<<10);
	Write32(EXI1_CSR, 3<<10);

	if (__Gecko_IsAlive()) {
		__Gecko_Flush();
		console_enabled = 1;
	}
}

u8 Gecko_EnableConsole(const u8 enable)
{
	if (enable) {
		if (__Gecko_IsAlive())
			console_enabled = 1;
	} else
		console_enabled = 0;

	return console_enabled;
}


