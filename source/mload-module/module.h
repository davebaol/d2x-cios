#ifndef _MODULE_H_
#define _MODULE_H_

#include "types.h"

/* Module info */
#define MLOAD_VER	1
#define MLOAD_SUBVER	0

/* Device name */
#define DEVICE_NAME	"/dev/mload"

/* IOCTL commands */
#define MLOAD_GET_IOS_INFO		0x4D4C4401
#define MLOAD_GET_MLOAD_VERSION		0x4D4C4402
#define MLOAD_LOAD_ELF			0x4D4C4480
#define MLOAD_RUN_ELF			0x4D4C4481
#define MLOAD_RUN_THREAD		0x4D4C4482
#define MLOAD_STOP_THREAD		0x4D4C4484
#define MLOAD_CONTINUE_THREAD		0x4D4C4485
#define MLOAD_GET_LOAD_BASE		0x4D4C4490
#define MLOAD_MEMSET			0x4D4C4491
#define MLOAD_SET_LOG_MODE		0x4D4C44D0
#define MLOAD_GET_LOG_BUFFER		0x4D4C44D1
#define MLOAD_SET_STEALTH_MODE		0x4D4C44E0

/* Module space */
extern u8  exe_mem[];
extern u32 exe_mem_size;

extern u32 stealth_mode;

#endif
 
