/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>
#include <usax/common/printk.h>

#include <usax/kernel/debug_utils.h>
#include <usax/kernel/hal.h>
#include <usax/kernel/irq.h>
#include <usax/kernel/process.h>
#include <usax/kernel/elf_utils.h>
#include <usax/kernel/paging_hw.h>
#include <usax/kernel/errno.h>

#include <elf.h>
#include <multiboot.h>

void dump_stacktrace(void *ebp, pdir_t *pdir)
{
   // TODO: implement dump_stacktrace for x86-64
}

void dump_regs(regs_t *r)
{
   // TODO: implement dump_regs for x86-64
}
