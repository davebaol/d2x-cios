#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"


static inline u32 Read32(u32 addr)
{
	u32 ret;

	__asm__ volatile("ldr\t%0, [%1]" : "=l"(ret) : "l"(addr));

	return ret;
}

static inline void Write32(u32 addr, u32 val)
{
	__asm__ volatile("str\t%0, [%1]" : : "l"(val), "l"(addr));
}

static inline u32 Get32(u32 addr, u32 bit)
{
	u32 val = (1 << bit);
	u32 ret;

	__asm__ volatile(
		"ldr\t%0, [%1]\n"
		"and\t%0, %2\n"
		"lsr\t%0, %3\n"
		: "=&l"(ret)
		: "l"(addr), "l"(val), "l"(bit)
	);

	return ret;
}

static inline u32 Set32(u32 addr, u32 bit)
{
	u32 val = (1 << bit);
	u32 ret;

	__asm__ volatile(
		"ldr\t%0, [%1]\n"
		"orr\t%0, %2\n"
		"str\t%0, [%1]\n"
		: "=&l"(ret)
		: "l"(addr), "l"(val)
	);

	return ret;
}

static inline u32 Clear32(u32 addr, u32 bit)
{
	u32 val = (1 << bit);
	u32 ret;

	__asm__ volatile(
		"ldr\t%0, [%1]\n"
		"bic\t%0, %2\n"
		"str\t%0, [%1]\n"
		: "=&l"(ret)
		: "l"(addr), "l"(val)
	);

	return ret;
}

#endif

