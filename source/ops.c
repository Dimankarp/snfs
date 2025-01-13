#include "ops.h"

const struct inode_operations vtfs_inode_ops = {.lookup = vtfs_lookup};

struct dentry* vtfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
) {
  return NULL;
}