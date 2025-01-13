#include "inode.h"
#include "ops.h"

const umode_t ALL_OWNRSHP = S_IRWXU | S_IRWXG | S_IRWXO;

struct inode* vtfs_get_inode(
    struct super_block* sb, const struct inode* dir, umode_t mode, int i_ino
) {
  struct inode* inode = new_inode(sb);
  if (inode != NULL) {
    inode_init_owner(&nop_mnt_idmap, inode, dir, mode | ALL_OWNRSHP);
  }

  inode->i_op = &vtfs_inode_ops;
  inode->i_ino = i_ino;
  return inode;
}
