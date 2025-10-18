/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>
#include <usax/kernel/hal.h>
#include <usax/kernel/sched.h>

ulong vdso_begin = 0; /* fake value */

void copy_main_tss_on_regs(regs_t *ctx)
{
   NOT_IMPLEMENTED();
}

void init_segmentation(void)
{
   NOT_IMPLEMENTED();
}
