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
int vtfs_rmdir(struct inode* parent_inode, struct dentry* child_dentry);
int vtfs_mkdir(
    struct mnt_idmap* map, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode
);
ssize_t vtfs_read(struct file* filp, char* __user buffer, size_t len, loff_t* offset);

ssize_t vtfs_write(struct file* filp, const char* __user buffer, size_t len, loff_t* offset);
int vtfs_fsync (struct file*, loff_t, loff_t, int);
#endif  // __FSMOD_SOURCE_OPS_H_