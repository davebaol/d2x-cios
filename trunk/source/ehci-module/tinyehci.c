/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
	Copyright (C) 2009 Waninkoko.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>
#include <setjmp.h>

#include "ehci_types.h"
#include "ehci.h"
#include "syscalls.h"
#include "timer.h"
#include "utils.h"

#define static
#define inline extern

/* Macros */
#define debug_printf(a...)
#define print_hex_dump_bytes(a, b, c, d)

#define ehci_dbg(a...)		debug_printf(a)
#define printk(a...)		debug_printf(a)
#define BUG()			while(1)
#define BUG_ON(a)		while(1)
#define cpu_to_le32(a)		swab32(a)
#define le32_to_cpu(a)		swab32(a)
#define cpu_to_le16(a)		swab16(a)
#define le16_to_cpu(a)		swab16(a)
#define cpu_to_be32(a)		(a)
#define be32_to_cpu(a)		(a)
#define ehci_readl(a)		(*(volatile u32 *)a)
#define ehci_writel(v,a)	do { *(volatile u32 *)a = v; } while(0)

/* EHCI structure */
struct ehci_hcd _ehci;
struct ehci_hcd *ehci = &_ehci;

#include "ehci.c"


s32 EHCI_Init(void)
{
	s32 ret;

	ehci = &_ehci;

	/* EHCI addresses */
	ehci->caps = (void *)(0x0D040000);
	ehci->regs = (void *)(0x0D040000 + HC_LENGTH(ehci_readl(&ehci->caps->hc_capbase)));

	/* Setup EHCI */
        ehci->num_port   = 4;
	ehci->hcs_params = ehci_readl(&ehci->caps->hcs_params);

	/* Initialize EHCI */
	ret = ehci_init();
	if (ret)
		return ret; 

	/* Release non USB2 ports */
        ehci_release_ports();

	return 0;
}
