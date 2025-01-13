#ifndef __FSMOD_SOURCE_OPS_H_
#define __FSMOD_SOURCE_OPS_H_

#include <linux/module.h>
#include <linux/types.h>

extern const struct inode_operations vtfs_inode_ops;

struct dentry* vtfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
);

#endif  // __FSMOD_SOURCE_OPS_H_