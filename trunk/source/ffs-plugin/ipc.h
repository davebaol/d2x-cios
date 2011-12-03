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

/* IOCTL vector */
typedef struct iovec {
	void *data;
	u32   len;
} ioctlv;

/* IPC message */
typedef struct ipcmessage {
	u32 command;
	u32 result;
	u32 fd;

	union 
	{
		struct
		{
			char *device;
			u32   mode;
			u32   resultfd;
		} open;
	
		struct 
		{
			void *data;
			u32   length;
		} read, write;
		
		struct 
		{
			s32 offset;
			s32 origin;
		} seek;
		
		struct 
		{
			u32 command;

			u32 *buffer_in;
			u32  length_in;
			u32 *buffer_io;
			u32  length_io;
		} ioctl;
		struct 
		{
			u32 command;

			u32 num_in;
			u32 num_io;
			ioctlv *vector;
		} ioctlv;
	};
} ATTRIBUTE_PACKED ipcmessage;


/* Prototypes */
void InvalidateVector(ioctlv *vector, u32 inlen, u32 iolen);
void FlushVector(ioctlv *vector, u32 inlen, u32 iolen);

#endif
