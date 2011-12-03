#ifndef _MODULE_H_
#define _MODULE_H_

/* Device name */
#define DEVICE					"/dev/usb2"

/* USB IOCTL commands */
#define USB_IOCTL_CTRLMSG			0
#define USB_IOCTL_BLKMSG			1
#define USB_IOCTL_INTRMSG			2
#define USB_IOCTL_SUSPENDDEV			5
#define USB_IOCTL_RESUMEDEV			6
#define USB_IOCTL_GETDEVLIST			12
#define USB_IOCTL_DEVREMOVALHOOK		26
#define USB_IOCTL_DEVINSERTHOOK			27

/* UMS IOCTL commands */
#define UMS_BASE				(('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT	        	(UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      	(UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      	(UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS		(UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS		(UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE		(UMS_BASE+0x6)
#define USB_IOCTL_UMS_UMOUNT			(UMS_BASE+0x10)
#define USB_IOCTL_UMS_WATCHDOG			(UMS_BASE+0x80)

/* WBFS IOCTL commands */
#define WBFS_BASE				(('W'<<24)|('F'<<16)|('S'<<8))
#define USB_IOCTL_WBFS_OPEN_DISC	        (WBFS_BASE+0x1)
#define USB_IOCTL_WBFS_READ_DISC	        (WBFS_BASE+0x2)

#endif

