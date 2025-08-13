#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "util.h"
#include "vfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dimankarp");
MODULE_DESCRIPTION("Simple network FS kernel module");

static struct file_system_type snfs_fs_type = {
    .name = MODULE_NAME,
    .mount = snfs_mount,
    .kill_sb = snfs_kill_vfs_sb,
};

static int __init snfs_init(void) {
  LOG("SNFS joined the kernel\n");
  register_filesystem(&snfs_fs_type);
  LOG("Registered fs\n");
  return 0;
}

static void __exit snfs_exit(void) {
  unregister_filesystem(&snfs_fs_type);
  LOG("Unregistered fs\n");
  LOG("SNFS left the kernel\n");
}

module_init(snfs_init);
module_exit(snfs_exit);
