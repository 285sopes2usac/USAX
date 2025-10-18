/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/kernel/fs/vfs.h>
#include <usax/kernel/syscalls.h>

#include "fs_int.h"

int
sys_mount(const char *user_source,
          const char *user_target,
          const char *user_fstype,
          unsigned long mountflags,
          const void *user_data)
{
   return -ENOSYS;
}

int sys_umount(const char *target, int flags)
{
   return -ENOSYS;
}
