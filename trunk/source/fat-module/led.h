/*   
	Custom IOS Module (FAT)

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

#ifndef _LED_H_
#define _LED_H_

#define LED_ACTIVITY

#ifdef LED_ACTIVITY
void Led_BlinkOn(void);
void Led_BlinkOff(void);
void Led_CreateBlinkThread(void);
#else
#define Led_BlinkOn()             do {} while (0)
#define Led_BlinkOff()            do {} while (0)
#define Led_CreateBlinkThread()   do {} while (0)
#endif

#endif
