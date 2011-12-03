#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "types.h"
#include "plugin.h"

/* Prototypes */
s32 Config_Load(struct dipConfigState *cfg, u32 size);
s32 Config_Save(struct dipConfigState *cfg, u32 size);

#endif

