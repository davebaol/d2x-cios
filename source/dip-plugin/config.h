#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "types.h"
#include "plugin.h"

/* Prototypes */
s32 DI_Config_Load(struct dipConfigState *cfg);
s32 DI_Config_Save(struct dipConfigState *cfg);
s32 FFS_Config_Load(struct ffsConfigState *cfg);
s32 FFS_Config_Save(struct ffsConfigState *cfg);

#endif

