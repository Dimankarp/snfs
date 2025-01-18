
#include "fs.h"

#include <linux/dcache.h>

#include "impl.h"
#include "inode.h"

void vtfs_kill_sb(struct super_block* sb) {
  printk(KERN_INFO "vtfs super block is destroyed. Unmount successfully.\n");
}

int vtfs_fill_super(struct super_block* sb, void* data, int silent) {
  int status = vtfs_init_sb();
  if (status < 0) {
    return status;
  }
  struct inode* inode = vtfs_get_inode(sb, NULL, S_IFDIR, VTFS_ROOT_NO);
  sb->s_root = d_make_root(inode);
  if (sb->s_root == NULL) {
    return -ENOMEM;
  }

  printk(KERN_INFO "return 0\n");
  return 0;
}

struct dentry* vtfs_mount(
    struct file_system_type* fs_type, int flags, const char* token, void* data
) {
  struct dentry* ret = mount_nodev(fs_type, flags, data, vtfs_fill_super);
  if (ret == NULL) {
    printk(KERN_ERR "Can't mount file system");
  } else {
    printk(KERN_INFO "Mounted successfuly");
  }
  return ret;
}
