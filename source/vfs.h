#ifndef __FSMOD_SOURCE_VFS_H_
#define __FSMOD_SOURCE_VFS_H_

#include <linux/fs.h>
#include <linux/kobject.h>

void snfs_kill_vfs_sb(struct super_block* sb);

int snfs_fill_vfs_sb(struct super_block* sb, void* data, int silent);

struct dentry* snfs_mount(
    struct file_system_type* fs_type, int flags, const char* token, void* data
);

struct inode* snfs_get_vfs_inode(
    struct super_block* sb, const struct inode* dir, umode_t mode, ino_t i_ino
);

#endif  // __FSMOD_SOURCE_VFS_H_