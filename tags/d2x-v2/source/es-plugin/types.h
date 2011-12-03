#ifndef _IOS_TYPES_H_
#define _IOS_TYPES_H_

#include <stdint.h>

/* NULL pointer */
#ifndef NULL
# define NULL			((void *)0)
#endif

/* Data types */
typedef char			s8;
typedef short			s16;
typedef long			s32;
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

typedef int			bool;
typedef uint32_t		sec_t;

/* Boolean values */
#define true			1
#define false			0

/* Attributes */
#ifndef ATTRIBUTE_ALIGN
# define ATTRIBUTE_ALIGN(v)	__attribute__((aligned(v)))
#endif
#ifndef ATTRIBUTE_PACKED
# define ATTRIBUTE_PACKED	__attribute__((packed))
#endif

/* Stack align */
#define STACK_ALIGN(type, name, cnt, alignment)	\
	u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + (((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - ((sizeof(type)*(cnt))%(alignment))) : 0))]; \
	type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (((u32)(_al__##name))&((alignment)-1))))

#endif
