/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include <usax_gen_headers/config_global.h>
#include <usax_gen_headers/config_debug.h>
#include <usax/kernel/list.h>
#include <usax/kernel/timer.h>

#define MAX_NO_DEADLOCK_SET_ELEMS   144
#define SELF_TEST_MAX_NAME_LEN       31

enum se_kind {
   se_manual,
   se_short,
   se_med,
   se_medlong,
   se_long
};

struct self_test {
   struct list_node node;
   const char *name;
   void (*func)(void);
   enum se_kind kind;
};

#define REGISTER_SELF_TEST(__name, __kind, __func)  \
                                                                \
   static struct self_test se_##__name##_inst = {               \
      .name = #__name,                                          \
      .kind = __kind,                                           \
      .func = __func                                            \
   };                                                           \
                                                                \
   __attribute__((constructor))                                 \
   static void __register_se_##__name(void) {                   \
      se_register(&se_##__name##_inst);                         \
   }

#if KERNEL_SELFTESTS
   void init_self_tests(void);
#else
   static inline void init_self_tests(void) { }
#endif

struct self_test *se_find(const char *name);
int se_run(struct self_test *se);
int se_runall(void);
bool se_is_stop_requested(void);
void se_register(struct self_test *se);

void se_regular_end(void);
void se_interrupted_end(void);
void simple_test_kthread(void *arg);
void selftest_kmalloc_perf(void);

/* Deadlock detection functions */
void debug_reset_no_deadlock_set(void);
void debug_add_task_to_no_deadlock_set(int tid);
void debug_remove_task_from_no_deadlock_set(int tid);
void debug_no_deadlock_set_report_progress(void);
void debug_check_for_deadlock(void);
void debug_check_for_any_progress(void);
