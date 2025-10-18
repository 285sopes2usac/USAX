/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <usax_gen_headers/mod_console.h>

#include <usax/common/basic_defs.h>
#include <usax/kernel/ringbuf.h>
#include <usax/kernel/term.h>
#include <usax/kernel/sync.h>

#include <termios.h>      // system header
#include <linux/kd.h>     // system header

struct tty;
typedef bool (*tty_ctrl_sig_func)(struct tty *, bool);

void tty_reset_filter_ctx(struct tty *t);
void tty_inbuf_reset(struct tty *t);

struct tty {

   term *tstate;
   const struct term_interface *tintf;
   struct term_params tparams;
   void *console_data;
   void *devfile;
   char dev_filename[16];

   int minor;
   int fg_pgid;

   struct ringbuf input_ringbuf;
   struct kcond input_cond;     /* signal when we can read from input_rb */
   struct kcond output_cond;    /* signal when we can write to input_rb */
   int end_line_delim_count;

   bool mediumraw_mode;
   u8 curr_color;
   u16 serial_port_fwd;

   char *input_buf;
   u32 kd_gfx_mode;
   tty_ctrl_sig_func *ctrl_handlers;
   struct termios c_term;
};
