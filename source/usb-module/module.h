#ifndef _MODULE_H_
#define _MODULE_H_

#include "ipc.h"

/* IOCTL commands */
#define IOCTL_USB_INIT			0x01
#define IOCTL_USB_READ			0x02
#define IOCTL_USB_WRITE			0x03
#define IOCTL_USB_ISINSERTED		0x04
#define IOCTL_USB_UNMOUNT		0x05

/* UMS IOCTL commands */
#define UMS_BASE			(('U'<<24)|('M'<<16)|('S'<<8))
#define IOCTL_UMS_INIT			(UMS_BASE + 0x1)
#define IOCTL_UMS_GET_CAPACITY		(UMS_BASE + 0x2)
#define IOCTL_UMS_READ_SECTORS		(UMS_BASE + 0x3)
#define IOCTL_UMS_WRITE_SECTORS		(UMS_BASE + 0x4)
#define IOCTL_UMS_UMOUNT		(UMS_BASE + 0x10)

/* WBFS IOCTL commands */
#define WBFS_BASE			(('W'<<24)|('F'<<16)|('S'<<8))
#define IOCTL_WBFS_OPEN_DISC		(WBFS_BASE + 0x1)
#define IOCTL_WBFS_READ_DISC		(WBFS_BASE + 0x2)

/* Message constants */
#define MESSAGE_DEVCHANGE		(u32)&usbCb[0]
#define MESSAGE_ATTACH			(u32)&usbCb[1]
#define MESSAGE_MOUNT			0x10000001

/* USB device names */
#define DEVICE_USB_NO_SLASH   "dev/usb2"
#define DEVICE_USB            "/"DEVICE_USB_NO_SLASH

/* Externs */
extern s32    queuehandle;
extern areply usbCb[];

#endif
