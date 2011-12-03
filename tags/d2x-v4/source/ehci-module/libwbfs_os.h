#ifndef _LIBWBFS_OS_H_
#define _LIBWBFS_OS_H_

#include "mem.h"
#include "types.h"

#define debug_printf(fmt, ...)

#define wbfs_fatal(x)		do { debug_printf("\nwbfs panic:%s\n\n", x); while(1); } while(0)
#define wbfs_error(x)		do { debug_printf("\nwbfs error:%s\n\n", x); } while(0)
#define wbfs_malloc(x)		Mem_Alloc(x)
#define wbfs_free(x)		Mem_Free(x)
#define wbfs_ioalloc(x)		Mem_Alloc(x)
#define wbfs_iofree(x)		Mem_Free(x)
#define wbfs_ntohl(x)		(x)
#define wbfs_htonl(x)		(x)
#define wbfs_ntohs(x)		(x)
#define wbfs_htons(x)		(x)

#include <string.h>

#define wbfs_memcmp(x,y,z)	memcmp(x,y,z)
#define wbfs_memcpy(x,y,z)	memcpy(x,y,z)
#define wbfs_memset(x,y,z)	memset(x,y,z)


#endif
