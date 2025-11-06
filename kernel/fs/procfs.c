/* SPDX-License-Identifier: BSD-2-Clause */

#include <usax/common/basic_defs.h>
#include <usax/common/string_util.h>
#include <usax/common/printk.h>

#include <usax/kernel/fs/vfs.h>
#include <usax/kernel/fs/procfs.h>
#include <usax/kernel/kmalloc.h>
#include <usax/kernel/errno.h>
#include <usax/kernel/sched.h>
#include <usax/kernel/sync.h>
#include <usax/kernel/rwlock.h>
#include <usax/kernel/datetime.h>
#include <usax/kernel/process.h>
#include <stdio.h>

/* Very small, minimal procfs implementation
 * - mounts at /proc/
 * - root contains directories named by PID (only user processes)
 * - each PID directory contains a single file "status" which when read
 *   returns a short textual state (one of task_state_str)
 */

struct procfs_root {
   REF_COUNTED_OBJECT;
   enum vfs_entry_type type; /* VFS_DIR */
   usax_ino_t ino;
};

struct procfs_pid_inode {
   REF_COUNTED_OBJECT;
   enum vfs_entry_type type; /* VFS_DIR */
   int pid;
   usax_ino_t ino;
};

struct procfs_status_inode {
   REF_COUNTED_OBJECT;
   enum vfs_entry_type type; /* VFS_FILE */
   int pid;
   usax_ino_t ino;
};

struct procfs_data {
   struct procfs_root root;
   usax_ino_t next_inode;
   time_t wrt_time;
   struct rwlock_wp rwlock;
};

static struct mnt_fs *procfs = NULL;

static inline usax_ino_t procfs_get_next_inode(struct procfs_data *d)
{
   return d->next_inode++;
}

/* Forward declarations */
static int procfs_getdents_root(fs_handle h, get_dents_func_cb cb, void *arg);
static int procfs_getdents_piddir(fs_handle h, get_dents_func_cb cb, void *arg);
static ssize_t procfs_status_read(fs_handle h, char *buf, size_t len, offt *pos);

static ssize_t procfs_dir_read(fs_handle h, char *buf, size_t len, offt *pos)
{
   return -EISDIR;
}

static offt procfs_dir_seek(fs_handle h, offt target_off, int whence)
{
   return -EINVAL;
}

static int procfs_dir_ioctl(fs_handle h, ulong request, void *arg)
{
   return -EINVAL;
}

static const struct file_ops procfs_fileops_dir = {
   .read = procfs_dir_read,
   .write = NULL,
   .seek = procfs_dir_seek,
   .ioctl = procfs_dir_ioctl,
};

static const struct file_ops procfs_fileops_status = {
   .read = procfs_status_read,
};

/* Open handlers */
static int procfs_open_root_dir(struct mnt_fs *fs, fs_handle *out)
{
   struct fs_handle_base *h;

   if (!(h = vfs_create_new_handle(fs, &procfs_fileops_dir)))
      return -ENOMEM;

   h->fl_flags = 0;
   h->fd_flags = 0;
   h->fs = fs;
   h->pi = get_curr_proc();
   *out = (fs_handle)h;
   return 0;
}

static int procfs_open_pid_dir(struct mnt_fs *fs, struct procfs_pid_inode *p, fs_handle *out)
{
   struct fs_handle_base *h;

   if (!(h = vfs_create_new_handle(fs, &procfs_fileops_dir)))
      return -ENOMEM;

   h->fl_flags = 0;
   h->fd_flags = 0;
   h->fs = fs;
   h->pi = get_curr_proc();
   *out = (fs_handle)h;
   return 0;
}

static int procfs_open_status_file(struct mnt_fs *fs, struct procfs_status_inode *i, fs_handle *out)
{
   struct fs_handle_base *h;

   if (!(h = vfs_create_new_handle(fs, &procfs_fileops_status)))
      return -ENOMEM;

   h->fl_flags = 0;
   h->fd_flags = 0;
   h->fs = fs;
   h->pi = get_curr_proc();

   /* store pid in h->lf (abuse) or other place? We'll store pid in h->dir_pos */
   h->dir_pos = (offt)i->pid;

   *out = (fs_handle)h;
   return 0;
}

CREATE_FS_PATH_STRUCT(procfs_path, vfs_inode_ptr_t, void *);

static void procfs_get_entry(struct mnt_fs *fs,
                            void *dir_inode,
                            const char *name,
                            ssize_t nl,
                            struct fs_path *fs_path)
{
   struct procfs_data *d = fs->device_data;

   if ((!dir_inode && !name) || is_dot_or_dotdot(name, (int)nl)) {

      *fs_path = (struct fs_path) {
         .inode = &d->root,
         .dir_inode = &d->root,
         .dir_entry = NULL,
         .type = VFS_DIR,
      };

      return;
   }

   if (!dir_inode) {
      /* path like /proc/<name> with no parent should not happen */
      return;
   }

   /* If parent is root, then entries are pid directories */
   if (dir_inode == &d->root) {

      /* name must be numeric PID */
      int pid = 0;
      for (ssize_t i = 0; i < nl; i++) {
         char c = name[i];
         if (!c) break;
         if (c < '0' || c > '9') {
            return; /* not a PID */
         }
         pid = pid * 10 + (c - '0');
      }

      /* verify process exists */
      disable_preemption();
      struct task *ti = get_task(pid);
      bool exists = (ti && !is_kernel_thread(ti));
      enable_preemption();

      if (!exists)
         return;

      /* allocate an inode object for this pid */
      struct procfs_pid_inode *pi = kzalloc_obj(struct procfs_pid_inode);

      if (!pi)
         return; /* OOM: pretend not found */

      pi->type = VFS_DIR;
      pi->pid = pid;
      pi->ino = procfs_get_next_inode(d);
      pi->ref_count = 1; /* one reference returned to caller */

      *fs_path = (struct fs_path) {
         .inode = (vfs_inode_ptr_t) pi,
         .dir_inode = &d->root,
         .dir_entry = pi,
         .type = VFS_DIR,
      };

      return;
   }

   /* If parent is a pid dir, accept "status" */
   struct procfs_pid_inode *pdir = dir_inode;

   if (pdir->type == VFS_DIR) {

      if (nl != 6) /* "status" length */
         return;

      if (!strncmp(name, "status", 6)) {
         struct procfs_status_inode *si = kzalloc_obj(struct procfs_status_inode);

         if (!si)
            return;

         si->type = VFS_FILE;
         si->pid = pdir->pid;
         si->ino = procfs_get_next_inode(d);
         si->ref_count = 1;

         *fs_path = (struct fs_path) {
            .inode = (vfs_inode_ptr_t) si,
            .dir_inode = pdir,
            .dir_entry = si,
            .type = VFS_FILE,
         };
      }
   }
}

static vfs_inode_ptr_t procfs_get_inode(fs_handle h)
{
   /* For this minimal implementation we don't keep a per-handle inode */
   return NULL;
}

static int procfs_stat(struct mnt_fs *fs, vfs_inode_ptr_t i, struct k_stat64 *statbuf)
{
   struct procfs_data *d = fs->device_data;

   bzero(statbuf, sizeof(*statbuf));
   statbuf->st_dev = fs->device_id;

   if (i == &d->root) {
      statbuf->st_mode = 0555 | S_IFDIR;
      statbuf->st_ino = d->root.ino;
   } else {
      /* must check if pid dir or status file */
      struct procfs_pid_inode *pi = i;

      if (pi->type == VFS_DIR) {
         statbuf->st_mode = 0555 | S_IFDIR;
         statbuf->st_ino = pi->ino;
      } else {
         struct procfs_status_inode *si = i;
         if (si->type == VFS_FILE) {
            statbuf->st_mode = 0444 | S_IFREG;
            /* approximate size */
            statbuf->st_size = 32;
            statbuf->st_ino = si->ino;
         } else {
            return -EINVAL;
         }
      }
   }

   statbuf->st_nlink = 1;
   statbuf->st_uid = 0;
   statbuf->st_gid = 0;
   statbuf->st_blksize = PAGE_SIZE;
   statbuf->st_blocks = 0;
   statbuf->st_ctim.tv_sec = d->wrt_time;
   statbuf->st_mtim = statbuf->st_ctim;
   statbuf->st_atim = statbuf->st_mtim;
   return 0;
}

static int procfs_retain_inode(struct mnt_fs *fs, vfs_inode_ptr_t inode)
{
   if (!inode)
      return -EINVAL;

   return retain_obj((struct procfs_pid_inode *)inode);
}

static int procfs_release_inode(struct mnt_fs *fs, vfs_inode_ptr_t inode)
{
   struct procfs_data *d = fs->device_data;

   if (!inode)
      return -EINVAL;

   int rc = release_obj((struct procfs_pid_inode *)inode);

   if (get_ref_count((struct procfs_pid_inode *)inode) == 0) {
      /* free dynamic inodes (exclude root) */
      if ((void *)inode != (void *)&d->root) {
         enum vfs_entry_type t = ((struct procfs_pid_inode *)inode)->type;

         if (t == VFS_DIR)
            kfree_obj((struct procfs_pid_inode *)inode, struct procfs_pid_inode);
         else
            kfree_obj((struct procfs_status_inode *)inode, struct procfs_status_inode);
      }
   }

   return rc;
}

static int procfs_getdents(fs_handle h, get_dents_func_cb cb, void *arg)
{
   return procfs_getdents_root(h, cb, arg);
}

static int procfs_getdents_root(fs_handle h, get_dents_func_cb cb, void *arg)
{
   int rc = 0;

   disable_preemption();

   for (int pid = 1; pid <= MAX_PID; pid++) {
      struct task *ti = get_task(pid);

      if (!ti)
         continue;

      if (is_kernel_thread(ti))
         continue;

      char namebuf[16];
      snprintf(namebuf, sizeof(namebuf), "%d", ti->tid);

      struct vfs_dent64 dent = {
         .ino = (usax_ino_t) ti->tid,
         .type = VFS_DIR,
         .name_len = (u8) (strlen(namebuf) + 1),
         .name = namebuf,
      };

      if ((rc = cb(&dent, arg)))
         break;
   }

   enable_preemption();
   return rc;
}

static ssize_t procfs_status_read(fs_handle h, char *buf, size_t len, offt *pos)
{
   struct fs_handle_base *hb = (struct fs_handle_base *)h;
   int pid = (int) hb->dir_pos;

   disable_preemption();
   struct task *ti = get_task(pid);

   if (!ti || is_kernel_thread(ti)) {
      enable_preemption();
      return 0; /* no such process */
   }

   const char *s = task_state_str[(int) ti->state];
   /* return just the status string followed by newline */
   char tbuf[64];
   int wrote = snprintf(tbuf, sizeof(tbuf), "%s\n", s);
   enable_preemption();

   size_t to_copy = (size_t) wrote;
   if (*pos >= (offt) to_copy)
      return 0;

   size_t avail = to_copy - (size_t)(*pos);
   size_t c = MIN(avail, len);
   memcpy(buf, tbuf + *pos, c);
   *pos += c;
   return (ssize_t) c;
}

static const struct fs_ops static_fsops_procfs = {
   .get_entry = procfs_get_entry,
   .get_inode = procfs_get_inode,
   .open = NULL, /* rely on default open path resolving */
   .on_close = NULL,
   .on_dup_cb = NULL,
   .getdents = procfs_getdents,
   .unlink = NULL,
   .mkdir = NULL,
   .rmdir = NULL,
   .truncate = NULL,
   .stat = procfs_stat,
   .chmod = NULL,
   .rename = NULL,
   .link = NULL,
   .retain_inode = procfs_retain_inode,
   .release_inode = procfs_release_inode,
   .fs_exlock = NULL,
   .fs_exunlock = NULL,
   .fs_shlock = NULL,
   .fs_shunlock = NULL,
};

struct mnt_fs *create_procfs(void)
{
   struct procfs_data *d;
   struct mnt_fs *fs;

   if (!(d = kzalloc_obj(struct procfs_data)))
      return NULL;

   d->root.type = VFS_DIR;
   d->root.ino = 1;
   d->next_inode = 2;
   d->wrt_time = (time_t)get_timestamp();
   rwlock_wp_init(&d->rwlock, false);

   fs = create_fs_obj("procfs", &static_fsops_procfs, d, VFS_FS_RW);

   if (!fs) {
      kfree_obj(d, struct procfs_data);
      return NULL;
   }

   /* ensure root refcount is non-zero so root is never freed */
   d->root.ref_count = 1;
   return fs;
}

void init_procfs(void)
{
   // int rc;

   if (vfs_mkdir("/proc", 0777))
      panic("vfs_mkdir(\"/proc\") failed");

   procfs = create_procfs();


   // if (!procfs)
   //    panic("Unable to create procfs");

   // if ((rc = mp_add(procfs, "/proc/")))
   //    panic("mp_add() failed with error: %d", rc);
}
