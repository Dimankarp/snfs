#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "fs.h"
#include "util.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mitya-ha-ha");
MODULE_DESCRIPTION("A simple FS kernel module");

static struct file_system_type vtfs_fs_type = {
    .name = MODULE_NAME,
    .mount = vtfs_mount,
    .kill_sb = vtfs_kill_sb,
};

static int __init vtfs_init(void) {
  LOG("VTFS joined the kernel\n");
  register_filesystem(&vtfs_fs_type);
  LOG("Registered fs\n");
  return 0;
}

static void __exit vtfs_exit(void) {
  unregister_filesystem(&vtfs_fs_type);
  LOG("Unregistered fs\n");
  LOG("VTFS left the kernel\n");
}

module_init(vtfs_init);
module_exit(vtfs_exit);
