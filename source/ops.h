#ifndef __FSMOD_SOURCE_OPS_H_
#define __FSMOD_SOURCE_OPS_H_

#include <linux/module.h>
#include <linux/types.h>

extern const struct inode_operations vtfs_inode_ops;
extern const struct file_operations vtfs_file_ops;

struct dentry* vtfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
);

int vtfs_iterate(struct file* filp, struct dir_context* ctx);
int vtfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
);
int vtfs_unlink(struct inode* parent_inode, struct dentry* child_dentry);
#endif  // __FSMOD_SOURCE_OPS_H_