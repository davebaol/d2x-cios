#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

/* Prototypes */
s32 Elf_Load(void *data);
s32 Elf_Run(void);
s32 Elf_RunThread(void *start, void *arg, void *stack, u32 stacksize, u32 priority);
s32 Elf_StopThread(s32 tid);
s32 Elf_ContinueThread(s32 tid);

#endif
