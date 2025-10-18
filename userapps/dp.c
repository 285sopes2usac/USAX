/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/syscalls.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

static int tracer_tool(int argc, char **argv)
{
   if (argc > 0) {
      printf("ERROR: unknown option '%s'\n", argv[0]);
      return 1;
   }

   int rc = syscall(usax_CMD_SYSCALL, usax_CMD_TRACING_TOOL);

   if (rc < 0)
      printf("ERROR: tracing not compiled-in\n");

   return rc;
}

static int ps_tool(int argc, char **argv)
{
   if (argc > 0) {
      printf("ERROR: unknown option '%s'\n", argv[0]);
      return 1;
   }

   int rc = syscall(usax_CMD_SYSCALL, usax_CMD_PS_TOOL);

   if (rc < 0)
      printf("ERROR: the ps tool is not compiled-in\n");

   return rc;
}

static int debug_panel(int argc, char **argv)
{
   if (argc > 0) {

      if (!strcmp(argv[0], "-t"))
         return tracer_tool(argc-1, argv+1);

      printf("ERROR: unknown option '%s'\n", argv[0]);
      return 1;
   }

   int rc = syscall(usax_CMD_SYSCALL, usax_CMD_DEBUG_PANEL);

   if (rc < 0)
      printf("ERROR: debug panel not compiled-in\n");

   return rc;
}

int main(int argc, char **argv)
{
   int rc = -1;
   char *prog_name;

   if (argc == 0) {
      printf("ERROR: argc is 0\n");
      return 1;
   }

   if (!getenv("usax")) {

      printf("ERROR: the debug panel exists only on usax!\n");
      return 1;
   }

   prog_name = basename(argv[0]);

   if (!strcmp(prog_name, "dp")) {

      rc = debug_panel(argc-1, argv+1);

   } else if (!strcmp(prog_name, "tracer")) {

      rc = tracer_tool(argc-1, argv+1);

   } else if (!strcmp(prog_name, "ps")) {

      rc = ps_tool(argc-1, argv+1);

   } else {

      printf("Unknown '%s' tool\n", prog_name);
      rc = -1;
   }

   return rc != 0;
}
