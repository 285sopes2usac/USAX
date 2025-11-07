/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>
#include <usax/common/printk.h>
#include <string.h>

#include <usax/kernel/syscalls.h>
#include <usax/kernel/user.h>
#include <usax/mods/tracing.h>

/* Minimal sys_syslog implementation: copy available trace_printk events
 * into the user buffer. Returns number of bytes copied, or negative errno.
 * This implementation currently ignores `type` and simply dumps any
 * printk events currently buffered in the tracing ring buffer.
 */
long
sys_syslog(int type, char *u_buf, int len)
{
   struct trace_event e;
   int events, i;
   int written = 0;

   (void) type; /* unused for now */

   if (len <= 0 || u_buf == NULL)
      return -EINVAL;

   /* If tracing isn't initialized there might be nothing to read */
   events = tracing_get_in_buffer_events_count();

   if (!events)
      return 0;

   for (i = 0; i < events; i++) {
      if (!read_trace_event_noblock(&e))
         break;

      if (e.type != te_printk)
         continue;


      /* measure message length (trace_printk ensures newline) */
      size_t bs = 0;
      while (bs < sizeof(e.p_ev.buf) && e.p_ev.buf[bs])
         bs++;

      if (!bs)
         continue;

      /* build a printk-like prefix using the event timestamp */
      char prefix[32];
      u64 ts = e.sys_time;
      u32 sec = (u32)(ts / TS_SCALE);
      u32 msec = (u32)((ts % TS_SCALE) / (TS_SCALE / 1000));
      int pfx_len = snprintk(prefix, sizeof(prefix), "[%5u.%03u] ", sec, msec);

      /* copy prefix + message, truncating if needed */
      size_t avail = (size_t)len - (size_t)written;
      if (!avail)
         break;

      /* first copy prefix */
      size_t to_copy_pfx = (size_t)pfx_len;
      if (to_copy_pfx > avail)
         to_copy_pfx = avail;

      if (to_copy_pfx) {
         if (copy_to_user(u_buf + written, prefix, to_copy_pfx))
            return -EFAULT;
         written += (int)to_copy_pfx;
         avail -= to_copy_pfx;
      }

      if (!avail)
         break;

      /* then copy message body */
      size_t to_copy = MIN(bs, avail);
      if (to_copy) {
         if (copy_to_user(u_buf + written, e.p_ev.buf, to_copy))
            return -EFAULT;
         written += (int)to_copy;
      }

      if (written >= len)
         break;
   }

   return written;
}
