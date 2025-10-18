/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <usax/common/basic_defs.h>
#include <usax/kernel/sync.h>
#include <usax/kernel/term.h>
#include <usax/kernel/kb.h>
#include <usax/kernel/tty_struct.h>
#include <usax/kernel/tty.h>
#include <usax/kernel/fs/vfs_base.h>
#include <usax/kernel/fs/devfs.h>

#define TTY_READ_BS   4096

struct tty_handle_extra {
   offt read_pos;
   offt read_buf_used;
   char *read_buf;
   bool read_allowed_to_return;
};

STATIC_ASSERT(sizeof(struct tty_handle_extra) <= DEVFS_EXTRA_SIZE);

void tty_input_init(struct tty *t);

enum kb_handler_action
tty_keypress_handler(struct kb_dev *, struct key_event ke);

void tty_update_ctrl_handlers(struct tty *t);
void tty_update_default_state_tables(struct tty *t);

ssize_t
tty_read_int(struct tty *t, struct devfs_handle *h, char *buf, size_t size);

ssize_t
tty_write_int(struct tty *t,
              struct devfs_handle *h,
              const char *buf,
              size_t size);

int
tty_ioctl_int(struct tty *t, struct devfs_handle *h, ulong request, void *argp);

bool
tty_read_ready_int(struct tty *t, struct devfs_handle *h);

int
tty_create_extra(int minor, void *extra);

void
tty_destroy_extra(int minor, void *extra);

int
tty_on_dup_extra(int minor, void *extra);

void init_ttyaux(void);
void tty_create_devfile_or_panic(const char *filename,
                                 u16 major,
                                 u16 minor,
                                 void **devfile);

extern const struct termios default_termios;
extern struct tty *ttys[128]; /* tty0 is not a real tty */
extern int tty_worker_thread;
extern struct tty *__curr_tty;
