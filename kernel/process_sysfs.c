/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/mods/sysfs.h>
#include <usax/mods/sysfs_utils.h>
#include <usax/kernel/process.h>
#include <usax/common/basic_defs.h>
#include <usax/common/printk.h>

/*
 * Minimal /syst/proc/<pid>/status implementation using sysfs.
 *
 * We create a dynamic sysobj per process with a single property `status`.
 * The property's `prop_data` points to the `struct process *` so the
 * loader callback can format the information on demand.
 *
 * NOTE: for simplicity we do not currently unregister the object when the
 * process exits; instead we NULL the prop_data entry to avoid dangling
 * pointers. This avoids requiring complex sysfs removal logic while still
 * keeping the interface usable.
 */

static offt proc_status_load(struct sysobj *obj,
                             void *data,
                             void *buf,
                             offt buf_sz,
                             offt off)
{
   struct process *pi = (struct process *)data;
   offt len = 0;

   if (!pi)
      return 0; /* no data available */

   /* keep the process alive while we format */
   retain_obj(pi);

   const char *state = pi->did_call_execve ? "R" : "S";
   const char *cmd = pi->debug_cmdline ? pi->debug_cmdline : "";

   len = snprintk(buf, (size_t)buf_sz,
                  "Pid:\t%d\nPPid:\t%d\nState:\t%s\nCmd:\t%s\n",
                  pi->pid,
                  pi->parent_pid,
                  state,
                  cmd);

   release_obj(pi);
   return len;
}

static const struct sysobj_prop_type proc_status_ptype = {
   .load = &proc_status_load,
};

static struct sysobj_prop prop_status = {
   .name = "status",
   .type = (struct sysobj_prop_type *)&proc_status_ptype,
};

void sysfs_proc_add(struct process *pi)
{
   char name[16];
   struct sysobj *obj;

   if (!pi)
      return;

   snprintk(name, sizeof(name), "%d", pi->pid);

   /* Create an object with one property: status -> pi */
   obj = sysfs_create_custom_obj("proc_proc", NULL,
                                 &prop_status, pi,
                                 NULL);

   if (!obj)
      return;

   if (sysfs_register_obj(NULL, &sysfs_proc_obj, name, obj) < 0) {
      sysfs_destroy_unregistered_obj(obj);
      return;
   }

   /* store pointer for later cleanup */
   pi->sysfs_obj = obj;
}

void sysfs_proc_remove(struct process *pi)
{
   if (!pi)
      return;

   if (!pi->sysfs_obj)
      return;

   /* Null the prop_data so callbacks won't dereference freed memory */
   if (pi->sysfs_obj->prop_data)
      pi->sysfs_obj->prop_data[0] = NULL;

   pi->sysfs_obj = NULL;
}
