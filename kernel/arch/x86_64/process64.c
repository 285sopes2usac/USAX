/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax_gen_headers/config_mm.h>
#include <usax_gen_headers/config_debug.h>

#include <usax/common/basic_defs.h>
#include <usax/common/utils.h>

#include <usax/kernel/sched.h>
#include <usax/kernel/process.h>
#include <usax/kernel/process_mm.h>
#include <usax/kernel/process_int.h>
#include <usax/kernel/kmalloc.h>
#include <usax/kernel/worker_thread.h>
#include <usax/kernel/debug_utils.h>
#include <usax/kernel/hal.h>
#include <usax/kernel/signal.h>
#include <usax/kernel/errno.h>
#include <usax/kernel/syscalls.h>
#include <usax/kernel/paging_hw.h>
#include <usax/kernel/irq.h>
#include <usax/kernel/user.h>
#include <usax/kernel/vdso.h>

#include <usax/mods/tracing.h>

void
setup_usermode_task_regs(regs_t *r, void *entry, void *stack_addr)
{
   NOT_IMPLEMENTED();
}

void
arch_specific_new_proc_setup(struct process *pi, struct process *parent)
{
   NOT_IMPLEMENTED();
}

void
arch_specific_free_proc(struct process *pi)
{
   NOT_IMPLEMENTED();
}

void
kthread_create_init_regs_arch(regs_t *r, void *func)
{
   NOT_IMPLEMENTED();
}

void
kthread_create_setup_initial_stack(struct task *ti, regs_t *r, void *arg)
{
   NOT_IMPLEMENTED();
}

void
save_curr_fpu_ctx_if_enabled(void)
{
   NOT_IMPLEMENTED();
}

void
arch_usermode_task_switch(struct task *ti)
{
   NOT_IMPLEMENTED();
}

void
set_kernel_stack(ulong addr)
{
   NOT_IMPLEMENTED();
}

int
setup_sig_handler(struct task *ti,
                  enum sig_state sig_state,
                  regs_t *r,
                  ulong user_func,
                  int signum)
{
   NOT_IMPLEMENTED();
}
