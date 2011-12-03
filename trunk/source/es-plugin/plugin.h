#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "ipc.h"
#include "types.h"


/* Config structure */
struct esConfig {
	/* Fake launch mode */
	u32 fakelaunch;
};

/* Prototypes */
s32 ES_EmulateCmd(ipcmessage *message);

/* Extern */
extern struct esConfig config;

#endif
