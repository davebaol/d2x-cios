/*   
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.

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

#include "elf.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "types.h"

/* ELF header */
typedef struct {
        u32	ident0;
	u32	ident1;
	u32	ident2;
	u32	ident3;
        u32	machinetype;
        u32	version;
        u32	entry;
        u32     phoff;
        u32     shoff;
        u32	flags;
        u16     ehsize;
        u16     phentsize;
        u16     phnum;
        u16     shentsize;
        u16     shnum;
        u16     shtrndx;
} elfheader;

/* ELF entry */
typedef struct {
       u32      type;
       u32      offset;
       u32      vaddr;
       u32      paddr;
       u32      filesz;
       u32      memsz;
       u32      flags;
       u32      align;
} elfphentry;

/* ELF data */
typedef struct {
	void *start;
	void *stack;
	int prio;
	int size_stack;
} elfdata;


/* Variables */
elfdata elf = { 0 };


static u32 be32(u8 *val)
{
	return (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
}


s32 Elf_Load(void *data)
{
	elfheader *head = (elfheader *)data;

	u32 i, j, pos;

	/* Check header */
	if (head->ident0 != 0x7F454C46 ||
	    head->ident1 != 0x01020161 ||
	    head->ident2 != 0x01000000)
		return -1;

	/* Initial position */
	pos = head->phoff;

	/* Entry point */
	elf.start = (void *)head->entry;

	/* Load sections */
	for (i = 0; i < head->phnum; i++) {
		elfphentry *entry = (data + pos);

		if (entry->type == 4) {
			u8 *addr = (data + entry->offset);

			if (be32(addr))
				return -2;

			for (j = 4; j < entry->memsz; j += 8) {
				u32 val1 = be32(&addr[j]);
				u32 val2 = be32(&addr[j+4]);

				switch (val1) {
				case 0x9:
					elf.start = (void *)val2;
					break;
				case 0x7D:
					elf.prio  = val2;
					break;
				case 0x7E:
					elf.size_stack = val2;
					break;
				case 0x7F:
					elf.stack = (void *)val2;
					break;
				}
			}
		}

		if (entry->type == 1 && entry->memsz && entry->vaddr) {
			void *dst = (void *)entry->vaddr;
			void *src = (data + entry->offset);

			/* Invalidate cache */
			os_sync_before_read(dst, entry->memsz);

			/* Copy section */
			memset(dst, 0,   entry->memsz);
			memcpy(dst, src, entry->filesz);

			/* Flush cache */
			os_sync_after_write(dst, entry->memsz);
		}

		/* Move position */
		pos += sizeof(elfphentry);
	}

	return 0;
}

s32 Elf_Run(void)
{
	/* Create thread */
	return Elf_RunThread(elf.start, NULL, elf.stack, elf.size_stack, elf.prio);
}

s32 Elf_RunThread(void *start, void *arg, void *stack, u32 stacksize, u32 priority)
{
	s32 ret;

	/* Create thread */
	ret = os_thread_create(start, arg, stack, stacksize, priority, 0);
	if (ret >= 0)
		os_thread_continue(ret);

	return ret;
}

s32 Elf_StopThread(s32 tid)
{
	/* Stop thread */
	return os_thread_stop(tid);
}

s32 Elf_ContinueThread(s32 tid)
{
	/* Continue thread */
	return os_thread_continue(tid);
}
