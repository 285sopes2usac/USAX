/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>

#include <usax/kernel/errno.h>
#include <usax/kernel/hal.h>
#include <usax/kernel/paging.h>

#include "gdt_int.h"

void load_ldt(u32 entry_index_in_gdt, u32 dpl)
{
   ASSERT(!are_interrupts_enabled());

   asmVolatile("lldt %w0"
               : /* no output */
               : "q" (X86_SELECTOR(entry_index_in_gdt, TABLE_GDT, dpl))
               : "memory");
}
