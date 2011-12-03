#ifndef _MODULE_H_
#define _MODULE_H_

/* IOCTL commands */
#define IOCTL_FAT_FILESTATS	11

/* IOCTLV commands */
#define IOCTL_FAT_MKDIR		0x01
#define IOCTL_FAT_MKFILE	0x02
#define IOCTL_FAT_READDIR	0x03
#define IOCTL_FAT_READDIR_LFN	0x04
#define IOCTL_FAT_DELETE	0x05
#define IOCTL_FAT_DELETEDIR	0x06
#define IOCTL_FAT_RENAME	0x07
#define IOCTL_FAT_STATS		0x08
#define IOCTL_FAT_GETUSAGE	0x09
#define IOCTL_FAT_MOUNT_SD	0xF0
#define IOCTL_FAT_UMOUNT_SD	0xF1
#define IOCTL_FAT_MOUNT_USB	0xF2
#define IOCTL_FAT_UMOUNT_USB	0xF3


/* IOCTLV commands used by FFS module for logging */
#define IOCTL_FAT_OPEN		0x81
#define IOCTL_FAT_CLOSE		0x82
#define IOCTL_FAT_READ		0x83
#define IOCTL_FAT_WRITE		0x84
#define IOCTL_FAT_SEEK		0x85

/* Device name */
#define DEVICE_FAT		"fat"

#endif
