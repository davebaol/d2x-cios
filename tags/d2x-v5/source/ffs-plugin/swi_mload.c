#include "tools.h"
#include "types.h"


void Swi_Memcpy(void *dst, void *src, s32 len)
{
	/* Wrong length */
	if (len <= 0)
		return;

	/* Call function */
	Swi_MLoad(2, (u32)dst, (u32)src, (u32)len);
}

void Swi_uMemcpy(void *dst, void *src, s32 len)
{
	/* Wrong length */
	if (len <= 0)
		return;

	/* Call function */
	Swi_MLoad(9, (u32)dst, (u32)src, (u32)len);
}

s32 Swi_CallFunc(s32 (*func)(void *in, void *out), void *in, void *out)
{
	/* Call function */
	return Swi_MLoad(16, (u32)func, (u32)in, (u32)out);
}

u32 Swi_GetSyscallBase(void)
{
	return Swi_MLoad(17, 0, 0, 0);
}

u32 Swi_GetIosInfo(void *buffer)
{
	return Swi_MLoad(18, (u32)buffer, 0, 0);
}
