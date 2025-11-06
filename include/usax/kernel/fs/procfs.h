/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <usax/common/basic_defs.h>

struct mnt_fs;

/* Create a procfs mnt_fs object (caller retains ownership). */
struct mnt_fs *create_procfs(void);

/* Initialize and mount procfs at /proc/. */
void init_procfs(void);
