#include "ipc.h"
#include "syscalls.h"
#include "types.h"


void InvalidateVector(ioctlv *vector, u32 inlen, u32 iolen)
{
	u32 cnt;

	for (cnt = 0; cnt < (inlen + iolen); cnt++) {
		void *buffer = vector[cnt].data;
		u32   len    = vector[cnt].len;

		/* Invalidate cache */
		os_sync_before_read(buffer, len);
	}
}

void FlushVector(ioctlv *vector, u32 inlen, u32 iolen)
{
	u32 cnt;

	for (cnt = inlen; cnt < (inlen + iolen); cnt++) {
		void *buffer = vector[cnt].data;
		u32   len    = vector[cnt].len;

		/* Flush cache */
		os_sync_after_write(buffer, len);
	}
}
