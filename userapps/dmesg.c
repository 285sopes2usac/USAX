/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>

#ifndef SYS_syslog
#warning "SYS_syslog not defined on this platform; dmesg may fail to compile"
#endif

/* Simple dmesg: calls the sys_syslog syscall (type ignored by kernel) and
 * writes the returned log text to stdout. Designed to be pipe-friendly.
 */
int main(int argc, char **argv)
{
   const int BUFSZ = 64 * 1024; /* 64KiB buffer */
   char *buf = malloc(BUFSZ);
   if (!buf) {
      perror("malloc");
      return 1;
   }

   long rc = syscall(SYS_syslog, 0, buf, BUFSZ);

   if (rc < 0) {
      errno = -rc;
      perror("syslog");
      free(buf);
      return 1;
   }

   ssize_t tot = 0;
   while (tot < rc) {
      ssize_t w = write(STDOUT_FILENO, buf + tot, (size_t)(rc - tot));
      if (w < 0) {
         if (errno == EINTR)
            continue;
         perror("write");
         free(buf);
         return 1;
      }
      tot += w;
   }

   free(buf);
   return 0;
}
