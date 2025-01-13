#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "fs.h"

#define MODULE_NAME "vtfs"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mitya-ha-ha");
MODULE_DESCRIPTION("A simple FS kernel module");

#define LOG(fmt, ...) pr_info("[" MODULE_NAME "]: " fmt, ##__VA_ARGS__)

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
