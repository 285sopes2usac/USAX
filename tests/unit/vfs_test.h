/* SPDX-License-Identifier: BSD-2-Clause */

#include <gtest/gtest.h>
#include "kernel_init_funcs.h"

extern "C" {

   #include <usax/kernel/fs/vfs.h>
   #include <usax/kernel/sched.h>
   #include <usax/kernel/process.h>
   #include <usax/kernel/fs/fat32.h>
   #include <usax/kernel/test/vfs.h>
   #include "kernel/fs/fs_int.h"
}

#define TEST_FATPART_FILE     PROJ_BUILD_DIR "/test_fatpart"

// Implemented in fat32_test.cpp
const char *load_once_file(const char *filepath, size_t *fsize = nullptr);
void test_dump_buf(char *buf, const char *buf_name, int off, int count);

class vfs_test_base : public ::testing::Test {

protected:

   void SetUp() override {

      init_kmalloc_for_tests();
   }

   void TearDown() override {

      /* do nothing, for the moment */
   }
};
