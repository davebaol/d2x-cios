/*---------------------------------------------------------------------------------------------
 * USB Gecko Development Kit - http://www.usbgecko.com
 * --------------------------------------------------------------------------------------------
 * 
 *
 * usb.h - functions for the USB Gecko adapter (www.usbgecko.com).
 *
 * Copyright (c) 2008 - Nuke - <wiinuke@gmail.com>
 * 
 *---------------------------------------------------------------------------------------------*/

#ifndef __USB_H__
#define __USB_H__

#define exi_chan0sr		*(volatile unsigned int*) 0x0D006800 // Channel 0 Status Register
#define exi_chan1sr		*(volatile unsigned int*) 0x0D006814 // Channel 1 Status Register
#define exi_chan2sr		*(volatile unsigned int*) 0x0D006828 // Channel 2 Status Register
#define exi_chan0cr		*(volatile unsigned int*) 0x0D00680c // Channel 0 Control Register
#define exi_chan1cr		*(volatile unsigned int*) 0x0D006820 // Channel 1 Control Register
#define exi_chan2cr		*(volatile unsigned int*) 0x0D006834 // Channel 2 Control Register
#define exi_chan0data		*(volatile unsigned int*) 0x0D006810 // Channel 0 Immediate Data
#define exi_chan1data		*(volatile unsigned int*) 0x0D006824 // Channel 1 Immediate Data
#define exi_chan2data		*(volatile unsigned int*) 0x0D006838 // Channel 2 Immediate Data
#define exi_chan0dmasta		*(volatile unsigned int*) 0x0D006804 // Channel 0 DMA Start address
#define exi_chan1dmasta		*(volatile unsigned int*) 0x0D006818 // Channel 1 DMA Start address
#define exi_chan2dmasta		*(volatile unsigned int*) 0x0D00682c // Channel 2 DMA Start address
#define exi_chan0dmalen		*(volatile unsigned int*) 0x0D006808 // Channel 0 DMA Length
#define exi_chan1dmalen		*(volatile unsigned int*) 0x0D00681c // Channel 1 DMA Length
#define exi_chan2dmalen		*(volatile unsigned int*) 0x0D006830 // Channel 2 DMA Length

// Function prototypes
void usb_flush(void);
int  usb_checkgecko(void);
void usb_sendbuffer (const void *buffer, int size);
void usb_receivebuffer (void *buffer, int size);
void usb_sendbuffersafe (const void *buffer, int size);
void usb_receivebuffersafe (void *buffer, int size);

#endif // __USB_H__
