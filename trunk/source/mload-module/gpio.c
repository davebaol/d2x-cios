/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
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

#include "gpio.h"
#include "hollywood.h"
#include "types.h"
#include "utils.h"

/* Constants */
#define GPIO_OUT	HW_GPIO1BOUT
#define GPIO_IN		HW_GPIO1IN


u8 GPIO_Read(u8 bit)
{
	/* Return bit value */
	return Get32(GPIO_IN, bit);
}

void GPIO_Write(u8 bit, u8 set)
{
	/* Set bit value */
	if (set)
		Set32(GPIO_OUT, bit);
	else
		Clear32(GPIO_OUT, bit);
}
