/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>
#include <usax/common/printk.h>

#include <usax/kernel/self_tests.h>
#include <usax/kernel/debug_utils.h>
#include <usax/kernel/paging.h>
#include <usax/kernel/sched.h>
#include <usax/kernel/timer.h>

void simple_test_kthread(void *arg)
{
   u32 i;
   DEBUG_SAVE_ESP();

   printk("[kthread] This is a kernel thread, arg = %p\n", arg);

   for (i = 0; i < 128*MB; i++) {

      DEBUG_CHECK_ESP();

      if (!(i % (8*MB))) {

         printk("[kthread] i = %i\n", i/MB);

         if (se_is_stop_requested())
            break;
      }
   }

   printk("[kthread] completed\n");
}

void selftest_kthread(void)
{
   int tid = kthread_create(simple_test_kthread, 0, (void *)1);

   if (tid < 0)
      panic("Unable to create the simple test kthread");

   kthread_join(tid, true);

   if (se_is_stop_requested())
      se_interrupted_end();
   else
      se_regular_end();
}

REGISTER_SELF_TEST(kthread, se_med, &selftest_kthread)

void selftest_sleep()
{
   const u64 wait_ticks = TIMER_HZ;
   u64 before = get_ticks();

   kernel_sleep(wait_ticks);

   u64 after = get_ticks();
   u64 elapsed = after - before;

   printk("[sleeping_kthread] elapsed ticks: %" PRIu64
          " (expected: %" PRIu64 ")\n", elapsed, wait_ticks);

   VERIFY((elapsed - wait_ticks) <= TIMER_HZ/10);
   se_regular_end();
}

REGISTER_SELF_TEST(sleep, se_short, &selftest_sleep)

void selftest_join()
{
   int tid;

   printk("[selftest join] create the simple thread\n");

   if ((tid = kthread_create(simple_test_kthread, 0, (void *)0xAA0011FF)) < 0)
      panic("Unable to create simple_test_kthread");

   printk("[selftest join] join()\n");
   kthread_join(tid, true);

   printk("[selftest join] kernel thread exited\n");

   if (se_is_stop_requested())
      se_interrupted_end();
   else
      se_regular_end();
}

REGISTER_SELF_TEST(join, se_med, &selftest_join)
