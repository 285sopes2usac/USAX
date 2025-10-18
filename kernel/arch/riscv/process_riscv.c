/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax_gen_headers/config_mm.h>
#include <usax_gen_headers/config_debug.h>

#include <usax/common/basic_defs.h>
#include <usax/common/utils.h>
#include <usax/common/unaligned.h>

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
#include <usax/kernel/switch.h>

#include <usax/mods/tracing.h>

void asm_trap_entry_resume(void);

STATIC_ASSERT(
   OFFSET_OF(struct task, fault_resume_regs) == TI_F_RESUME_RS_OFF
);
STATIC_ASSERT(
   OFFSET_OF(struct task, faults_resume_mask) == TI_FAULTS_MASK_OFF
);

STATIC_ASSERT(sizeof(struct task_and_process) <= 2048);

int setup_sig_handler(struct task *ti,
                      enum sig_state sig_state,
                      regs_t *r,
                      ulong user_func,
                      int signum)
{
   if (ti->nested_sig_handlers == 0) {

      int rc;

      if (sig_state == sig_pre_syscall)
         set_return_register(r, -EINTR);

      if ((rc = save_regs_on_user_stack(r)) < 0)
         return rc;
   }

   regs_set_ip(r, user_func);
   regs_set_usersp(r,
                   regs_get_usersp(r) -
                   SIG_HANDLER_ALIGN_ADJUST -
                   sizeof(ulong));
   set_return_register(r, signum);
   set_return_addr(r, post_sig_handler_user_vaddr);
   ti->nested_sig_handlers++;

   ASSERT((regs_get_usersp(r) & (USERMODE_STACK_ALIGN - 1)) == 0);
   return 0;
}

void
kthread_create_init_regs_arch(regs_t *r, void *func)
{
   *r = (regs_t) {
      .kernel_resume_pc = (ulong)&asm_trap_entry_resume,
      .sepc = (ulong)func,
      .sstatus = SR_SPIE | SR_SPP | SR_SIE | SR_SUM,
   };
}

void
kthread_create_setup_initial_stack(struct task *ti, regs_t *r, void *arg)
{
   set_return_register(r, (ulong)arg);
   set_return_addr(r, (ulong)&kthread_exit);
   regs_set_sp(r, (ulong)ti->state_regs);
   ti->state_regs = (void *)ti->state_regs - sizeof(regs_t);
   memcpy(ti->state_regs, r, sizeof(*r));
}

void
setup_usermode_task_regs(regs_t *r, void *entry, void *stack_addr)
{
   *r = (regs_t) {
      .kernel_resume_pc = (ulong)&asm_trap_entry_resume,
      .sepc = (ulong)entry,
      .sp = 0,
      .usersp = (ulong)stack_addr,
      .sstatus = (ulong)SR_SPIE | SR_SUM, /* User mode, enable interrupt */
   };
}

/*
 * Sched functions that are here because of arch-specific statements.
 */

static inline bool
is_fpu_enabled_for_task(struct task *ti)
{
   return get_task_arch_fields(ti)->fpu_regs &&
          (ti->state_regs->sstatus & SR_FS);
}

void
save_curr_fpu_ctx_if_enabled(void)
{
   if (is_fpu_enabled_for_task(get_curr_task())) {
      save_current_fpu_regs(false);
   }
}

void set_kernel_stack(ulong stack_ptr)
{
   /*
    * Do nothing, on RISCV.
    *
    * On some architectures like i368, this function sets the kernel stack
    * pointer that will be used when we return to kernel mode, immediately
    * before returning to a usermode task. On RISCV we don't need to implement
    * that.
    */
}

void
arch_usermode_task_switch(struct task *ti)
{
   regs_t *state = ti->state_regs;
   ASSERT(!is_kernel_thread(ti));

   if (get_curr_pdir() != ti->pi->pdir) {
      set_curr_pdir(ti->pi->pdir);
   }

   if (!running_in_kernel(ti)) {
      process_signals(ti, sig_in_usermode, state);
   }

   if (is_fpu_enabled_for_task(ti)) {
      restore_fpu_regs(ti, false);
   }
}

void
arch_specific_new_proc_setup(struct process *pi, struct process *parent)
{
   if (!parent)
      return;      /* we're done */

   pi->set_child_tid = NULL;
}

void
arch_specific_free_proc(struct process *pi)
{
   /* do nothing */
   return;
}

static void
handle_fatal_error(regs_t *r, int signum)
{
   send_signal(get_curr_tid(), signum, SIG_FL_PROCESS | SIG_FL_FAULT);
}

/* Access fault handler */
void handle_generic_fault_int(regs_t *r, const char *fault_name)
{
   if (!get_curr_task() || is_kernel_thread(get_curr_task()))
      panic("FAULT. Error: %s\n", fault_name);

   handle_fatal_error(r, SIGSEGV);
}

/* Instruction illegal fault handler */
void handle_inst_illegal_fault_int(regs_t *r, const char *fault_name)
{
   if (!get_curr_task() || is_kernel_thread(get_curr_task()))
      panic("FAULT. Error: %s\n", fault_name);

   handle_fatal_error(r, SIGILL);
}

/* Misaligned fault handler */
void handle_bus_fault_int(regs_t *r, const char *fault_name)
{
   if (!get_curr_task() || is_kernel_thread(get_curr_task()))
      panic("FAULT. Error: %s\n", fault_name);

   handle_fatal_error(r, SIGBUS);
}

