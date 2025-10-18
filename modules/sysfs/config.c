/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>


static void
sysfs_fail_to_register_obj(const char *name)
{
   panic("sysfs: unable to register object '%s'", name);
}

#include <usax_gen_headers/generated_config.h>
