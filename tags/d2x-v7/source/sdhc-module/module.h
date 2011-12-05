#ifndef _MODULE_H_
#define _MODULE_H_

/* WBFS IOCTL base */
#define WBFS_BASE		(('W'<<24)|('F'<<16)|('S'<<8))

/* IOCTL commands */
#define IOCTL_SDHC_INIT		0x01
#define IOCTL_SDHC_READ		0x02
#define IOCTL_SDHC_WRITE	0x03
#define IOCTL_SDHC_ISINSERTED	0x04
#define IOCTL_WBFS_OPEN_DISC	(WBFS_BASE + 0x1)
#define IOCTL_WBFS_READ_DISC	(WBFS_BASE + 0x2)

/* Device name */
#define DEVICE_NAME		"/dev/sdio/sdhc"

#endif
 
