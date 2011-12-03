#ifndef _ES_CALLS_H_
#define _ES_CALLS_H_

#include "types.h"


/* Prototypes */
s32 ES_printf(const char *fmt, ...);
s32 ES_snprintf(char *str, u32 size, const char *fmt, ...);
s32 ES_LoadModules(u32 version);
s32 ES_LaunchTitle(u32 tidh, u32 tidl, void *view, u32 reset);

/* ES handlers */
s32 ES_HandleIoctlv(void *data);

#endif
