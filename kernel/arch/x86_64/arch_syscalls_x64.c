/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax_gen_headers/mod_tracing.h>

#include <usax/common/basic_defs.h>
#include <usax/common/printk.h>

#include <usax/kernel/syscalls.h>
#include <usax/kernel/irq.h>
#include <usax/kernel/hal.h>
#include <usax/kernel/errno.h>
#include <usax/kernel/timer.h>
#include <usax/kernel/debug_utils.h>
#include <usax/kernel/fault_resumable.h>
#include <usax/kernel/user.h>
#include <usax/kernel/elf_utils.h>
#include <usax/kernel/signal.h>
#include <usax/mods/tracing.h>

void syscall_int80_entry(void);
void sysenter_entry(void);

typedef long (*syscall_type)();

#define SYSFL_NO_TRACE                      0b00000001
#define SYSFL_NO_SIG                        0b00000010
#define SYSFL_NO_PREEMPT                    0b00000100

struct syscall {

   union {
      void *func;
      syscall_type fptr;
   };

   u32 flags;
};

static void unknown_syscall_int(regs_t *r, u32 sn)
{
   trace_printk(5, "Unknown syscall %i", (int)sn);
   r->rax = (ulong) -ENOSYS;
}

static void __unknown_syscall(void)
{
   struct task *curr = get_curr_task();
   regs_t *r = curr->state_regs;
   const u32 sn = r->rax;
   unknown_syscall_int(r, sn);
}

#define DECL_SYS(func, flags) { {func}, flags }
#define DECL_UNKNOWN_SYSCALL  DECL_SYS(__unknown_syscall, 0)

/*
 * The syscall numbers are ARCH-dependent
 *
 * The numbers and the syscall names MUST BE in sync with the following file
 * in the Linux kernel:
 *
 *    ADD SYSCALL LINUX TBL FILE
 *
 * Lasy synced with Linux 5.15-rc2.
 */
static struct syscall syscalls[MAX_SYSCALLS] =
{
   [usax_CMD_SYSCALL] = DECL_SYS(sys_usax_cmd, 0),
};

void *get_syscall_func_ptr(u32 n)
{
   NOT_IMPLEMENTED();
   return syscalls[n].fptr;
}

int get_syscall_num(void *func)
{
   NOT_IMPLEMENTED();
}

void handle_syscall(regs_t *r)
{
   NOT_IMPLEMENTED();
}

void init_syscall_interfaces(void)
{
   NOT_IMPLEMENTED();
}

