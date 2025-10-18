/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include <usax/common/string_util.h>

s32 usax_strtol32(const char *s, const char **endptr, int base, int *err);
s64 usax_strtol64(const char *s, const char **endptr, int base, int *err);
u32 usax_strtoul32(const char *s, const char **endptr, int base, int *err);
u64 usax_strtoul64(const char *s, const char **endptr, int base, int *err);
