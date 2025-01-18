#include "inode.h"

#include "ops.h"

struct inode* vtfs_get_inode(
    struct super_block* sb, const struct inode* dir, umode_t mode, ino_t i_ino
) {
  struct inode* inode = new_inode(sb);
  if (inode != NULL) {
    inode_init_owner(&nop_mnt_idmap, inode, dir, mode|S_IRWXUGO);
    inode->i_op = &vtfs_inode_ops;
    inode->i_fop = &vtfs_file_ops;
    inode->i_ino = i_ino;
  }
  return inode;
}
