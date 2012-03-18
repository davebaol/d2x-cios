#ifndef _TINYEHCI_H_
#define _TINYEHCI_H_

#include "types.h"

/* Prototypes */
s32  EHCI_InitStage1(void);
void EHCI_InitStage2(void);
s32  EHCI_Loop(void);

#endif
