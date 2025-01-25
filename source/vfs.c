
#include "vfs.h"

#include <linux/dcache.h>

#include "impl.h"
#include "util.h"

void snfs_kill_vfs_sb(struct super_block* sb) {
  LOG("Super block is destroyed. Unmount successfully.\n");
}

int snfs_fill_vfs_sb(struct super_block* sb, void* data, int silent) {
  int status = vtfs_init_sb();
  if (status < 0) {
    return status;
  }
  struct inode* inode = snfs_get_vfs_inode(sb, NULL, S_IFDIR, VTFS_ROOT_NO);
  sb->s_root = d_make_root(inode);
  if (sb->s_root == NULL) {
    return -ENOMEM;
  }

  LOGM("snfs_fill_vfs_sb", "Filled block.\n");
  return 0;
}

struct dentry* snfs_mount(
    struct file_system_type* fs_type, int flags, const char* token, void* data
) {
  struct dentry* ret = mount_nodev(fs_type, flags, data, snfs_fill_vfs_sb);
  if (ret == NULL) {
    printk(KERN_ERR "[snfs]: Can't mount file system");
  } else {
    LOGM("snfs_mount", "Mounted successfully.\n");
  }
  return ret;
}

struct inode* snfs_get_vfs_inode(
    struct super_block* sb, const struct inode* dir, umode_t mode, ino_t i_ino
) {
  struct inode* inode = new_inode(sb);
  if (inode != NULL) {
    inode_init_owner(&nop_mnt_idmap, inode, dir, mode | S_IRWXUGO);
    inode->i_op = &snfs_inode_ops;
    inode->i_fop = &snfs_file_ops;
    inode->i_ino = i_ino;
  }
  return inode;
}
