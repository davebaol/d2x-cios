#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "ipc.h"
#include "types.h"


/* Config structure */
struct esConfig {
	/* Fake launch mode */
	u32 fakelaunch;

	/* ios to be launched in place of the requested one */
	u32 ios;

	/* title_id of the game requesting ios reload */
	u64 title_id;

	/* title_id to be launched in place of the system menu */
	u64 sm_title_id;
};

/* Prototypes */
s32 ES_EmulateCmd(ipcmessage *message);

/* Extern */
extern struct esConfig config;

#endif
