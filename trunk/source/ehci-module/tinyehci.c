/*   
	Custom IOS Module (EHCI)

	Copyright (C) 2008 neimod.
	Copyright (C) 2009 kwiirk.
	Copyright (C) 2009 Hermes.
	Copyright (C) 2009 Waninkoko.
	Copyright (C) 2011 davebaol.
	Copyright (C) 2011 rodries.

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
#include "usb_os.h"
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
#define ehci_readl(a)		((*((volatile u32*)(a))))
#define ehci_writel(v,a)	do{*((volatile u32*)(a))=(v);}while(0)

/* EHCI structure */
struct ehci_hcd _ehci;
struct ehci_hcd *ehci = &_ehci;

#include "ehci.c"


static struct ehci_qtd *qtd_dummy_first = NULL;
static struct ehci_qtd *qtd_header      = NULL;
static struct ehci_qh  *qh_header       = NULL;

struct ehci_qh *qh_pointer[64];

extern struct ehci_qh *in_qh;
extern struct ehci_qh *out_qh;
extern struct ehci_qh *dummy_qh;


inline dma_addr_t get_qtd_dummy(void)
{
	return qtd_dummy_first->qtd_dma;
}

static void init_qh_and_qtd(void)
{
	int n;
	struct ehci_qtd *qtd;

	struct ehci_qh *qh;

	if (!qh_header) {
		qh_header= (struct ehci_qh *) ehci->async;
		qtd_header= (struct ehci_qtd *) ehci->qtds[0];
	}

	qtd = qtd_header;

	for (n = 0; n < EHCI_MAX_QTD; n++) {
		ehci->qtds[n] = qtd;
		
		memset((void *)ehci->qtds[n], 0, sizeof(struct ehci_qtd));
		ehci_dma_map_bidir((void *)ehci->qtds[n], sizeof(struct ehci_qtd));
		
		qtd = (struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
	}

	for (n = 0; n < EHCI_MAX_QTD; n++) {
		memset((void *)qtd, 0, sizeof(struct ehci_qtd));
		ehci_dma_map_bidir((void *)qtd,sizeof(struct ehci_qtd));
		
		qtd = (struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
	}

	qtd_dummy_first = qtd;

	qh = qh_header;

	for (n = 0; n < 6; n++) {
		qh_pointer[n] = qh;

		memset((void *) qh_pointer[n], 0, sizeof(struct ehci_qh));
		qh->qh_dma = ehci_virt_to_dma(qh);
		qh_pointer[n]->hw_info1 = cpu_to_hc32((QH_HEAD*(n!=0)));
		qh_pointer[n]->hw_info2 = cpu_to_hc32(0);
		qh_pointer[n]->hw_token = cpu_to_hc32( QTD_STS_HALT);
		qh = (struct ehci_qh *) (((((u32) qh)+sizeof(struct ehci_qh)+31) & ~31));
		qh_pointer[n]->hw_next = QH_NEXT( ehci_virt_to_dma(qh));
		qh_pointer[n]->hw_qtd_next = EHCI_LIST_END();
		qh_pointer[n]->hw_alt_next = EHCI_LIST_END();
		
		ehci_dma_map_bidir((void *) qh_pointer[n],sizeof(struct ehci_qh));
	}

	n--;
	qh_pointer[n]->hw_next = QH_NEXT( ehci_virt_to_dma(qh_header));
	ehci_dma_map_bidir((void *) qh_pointer[n],sizeof(struct ehci_qh));
}

static void create_qtd_dummy(void)
{
	int n;
	struct ehci_qtd * qtd, *qtd_next;

	qtd = qtd_dummy_first;

	for (n = 0; ;n++) {
		qtd_next = (struct ehci_qtd *) (((((u32) qtd)+sizeof(struct ehci_qtd)+31) & ~31));
		ehci_qtd_init(qtd);

		//qtd_fill( qtd, 0, 0, QTD_STS_HALT, 0);
		if (n >= 3) {
			ehci_dma_map_bidir(qtd,sizeof(struct ehci_qtd));
			break;
		}

		qtd->hw_next = QTD_NEXT(qtd_next->qtd_dma);
		qtd->hw_alt_next = EHCI_LIST_END(); //QTD_NEXT(qtd_next->qtd_dma);
		ehci_dma_map_bidir((void *) qtd,sizeof(struct ehci_qtd));

		qtd = qtd_next;
	}
}

static void reinit_ehci_headers(void)
{
	init_qh_and_qtd();

	create_qtd_dummy();

	ehci->async   = qh_pointer[0];
	ehci->asyncqh = qh_pointer[1];
	in_qh         = qh_pointer[2];
	out_qh        = qh_pointer[3];
	dummy_qh      = qh_pointer[4];

	ehci_dma_unmap_bidir((dma_addr_t) ehci->async,sizeof(struct ehci_qh));

	ehci->async->ehci     = ehci;
	ehci->async->qtd_head = NULL;
	ehci->async->qh_dma   = ehci_virt_to_dma(ehci->async);
	ehci->async->hw_next  = QH_NEXT(dummy_qh->qh_dma /*ehci->async->qh_dma*/);
	ehci->async->hw_info1 = cpu_to_hc32(QH_HEAD);
	ehci->async->hw_info2 = cpu_to_hc32(0);
	ehci->async->hw_token = cpu_to_hc32(QTD_STS_HALT);

	ehci->async->hw_qtd_next = EHCI_LIST_END();
	ehci->async->hw_alt_next = EHCI_LIST_END(); //QTD_NEXT(get_qtd_dummy());

	ehci_dma_map_bidir(ehci->async,sizeof(struct ehci_qh));

	ehci_dma_unmap_bidir((dma_addr_t)ehci->asyncqh,sizeof(struct ehci_qh));
	ehci->asyncqh->ehci     = ehci;
	ehci->asyncqh->qtd_head = NULL;
	ehci->asyncqh->qh_dma   = ehci_virt_to_dma(ehci->asyncqh);

	ehci_dma_unmap_bidir((dma_addr_t)in_qh,sizeof(struct ehci_qh));
	in_qh->ehci     = ehci;
	in_qh->qtd_head = NULL;
	in_qh->qh_dma   = ehci_virt_to_dma(in_qh);
	ehci_dma_map_bidir(in_qh,sizeof(struct ehci_qh));

	ehci_dma_unmap_bidir((dma_addr_t)out_qh,sizeof(struct ehci_qh));
	out_qh->ehci     = ehci;
	out_qh->qtd_head = NULL;
	out_qh->qh_dma   = ehci_virt_to_dma(out_qh);
	ehci_dma_map_bidir(out_qh,sizeof(struct ehci_qh));
}

s32 EHCI_InitStage1(void)
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

	/* Set number of ports, i.e. only port 0 and port 1 */
	ehci->num_port = 2;

	return 0;
}

void EHCI_InitStage2(void)
{
	int i;

	/* Stop EHCI */
	ehci_writel(0x00010020, &ehci->regs->command);

	/* Wait */
	while ((ehci_readl( &ehci->regs->command) & 1) != 0);

	ehci_dma_map_bidir(ehci,sizeof(struct ehci_hcd));
	
	for (i = 0; i < DEFAULT_I_TDPS; i++) {
		ehci->periodic[i] = EHCI_LIST_END();
		ehci_dma_map_bidir((void *) ehci->periodic [i],4);
	}
			
	reinit_ehci_headers();
		
#define t125us (1)
		
	ehci_writel(ehci->async->qh_dma, &ehci->regs->async_next);
	ehci_writel(STS_PCD, &ehci->regs->intr_enable);
	ehci_writel((t125us<<16) | 0x0021, &ehci->regs->command);
	ehci_readl(&ehci->regs->command);
}

