/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax_gen_headers/config_debug.h>
#include <usax/common/basic_defs.h>

#include <usax/kernel/syscalls.h>
#include <usax/kernel/self_tests.h>
#include <usax/kernel/elf_utils.h>
#include <usax/kernel/user.h>
#include <usax/kernel/errno.h>
#include <usax/kernel/gcov.h>
#include <usax/kernel/debug_utils.h>

typedef int (*usax_cmd_func)(ulong, ulong, ulong, ulong);
static int usax_sys_run_selftest(const char *user_selftest);
static int usax_call_fn_0(const char *fn_name);
static int usax_get_var_long(const char *var_name, long *buf);
static int usax_busy_wait(ulong n);

static void *usax_cmds[usax_CMD_COUNT] = {

   [usax_CMD_RUN_SELFTEST] = usax_sys_run_selftest,
   [usax_CMD_GCOV_GET_NUM_FILES] = usax_sys_gcov_get_file_count,
   [usax_CMD_GCOV_FILE_INFO] = usax_sys_gcov_get_file_info,
   [usax_CMD_GCOV_GET_FILE] = usax_sys_gcov_get_file,
   [usax_CMD_QEMU_POWEROFF] = debug_qemu_turn_off_machine,
   [usax_CMD_SET_SAT_ENABLED] = set_sched_alive_thread_enabled,
   [usax_CMD_DEBUG_PANEL] = NULL,
   [usax_CMD_TRACING_TOOL] = NULL,
   [usax_CMD_PS_TOOL] = NULL,
   [usax_CMD_DEBUGGER_TOOL] = NULL,
   [usax_CMD_CALL_FUNC_0] = NULL,
   [usax_CMD_GET_VAR_LONG] = NULL,
   [usax_CMD_BUSY_WAIT] = NULL,
};

void register_usax_cmd(int cmd_n, void *func)
{
   ASSERT(0 <= cmd_n && cmd_n < usax_CMD_COUNT);
   VERIFY(usax_cmds[cmd_n] == NULL);

   usax_cmds[cmd_n] = func;
}

static int usax_sys_run_selftest(const char *u_selftest)
{
   struct self_test *se;
   char buf[96];
   int rc;

   if (!KERNEL_SELFTESTS)
      return -EINVAL;

   rc = copy_str_from_user(buf, u_selftest, sizeof(buf) - 1, NULL);

   if (rc != 0)
      return -EFAULT;

   if (!strcmp(buf, "runall")) {
      return se_runall();
   }

   se = se_find(buf);

   if (!se)
      return -EINVAL;

   printk("Running self-test: %s\n", buf);
   return se_run(se);
}

int sys_usax_cmd(int cmd_n, ulong a1, ulong a2, ulong a3, ulong a4)
{
   usax_cmd_func func;

   if (cmd_n >= usax_CMD_COUNT)
      return -EINVAL;

   *(void **)(&func) = usax_cmds[cmd_n];

   if (!func)
      return -EINVAL;

   return func(a1, a2, a3, a4);
}
