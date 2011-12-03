#ifndef _IPC_H_
#define _IPC_H_

#include "types.h"

/* IPC error codes */
#define IPC_ENOENT		-6
#define IPC_ENOMEM		-22
#define IPC_EINVAL		-101
#define IPC_EACCESS		-102
#define IPC_EEXIST		-105
#define IPC_NOENT		-106

/* IOS calls */
#define IOS_OPEN		0x01
#define IOS_CLOSE		0x02
#define IOS_READ		0x03
#define IOS_WRITE		0x04
#define IOS_SEEK		0x05
#define IOS_IOCTL		0x06
#define IOS_IOCTLV		0x07

/* IOCTLV vector */
typedef struct iovec {
	void *data;
	u32   len;
} ioctlv;

/* IOCTL structure */
typedef struct {
	u32 command;

	u32 *inbuf;
	u32  inlen;
	u32 *iobuf;
	u32  iolen;
} ioctl;


/* Prototypes */
void InvalidateVector(ioctlv *vector, u32 inlen, u32 iolen);
void FlushVector(ioctlv *vector, u32 inlen, u32 iolen);

#endif
