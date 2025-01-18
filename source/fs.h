#ifndef __FSMOD_SOURCE_FS_H_
#define __FSMOD_SOURCE_FS_H_

#include <linux/fs.h>
#include <linux/kobject.h>

void vtfs_kill_sb(struct super_block* sb);

int vtfs_fill_super(struct super_block* sb, void* data, int silent);

struct dentry* vtfs_mount(
    struct file_system_type* fs_type, int flags, const char* token, void* data
);

#endif  // __FSMOD_SOURCE_FS_H_