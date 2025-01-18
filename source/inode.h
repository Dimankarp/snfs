#ifndef __FSMOD_SOURCE_INODE_H_
#define __FSMOD_SOURCE_INODE_H_

#include <linux/fs.h>
#include <linux/kobject.h>

extern const umode_t ALL_OWNRSHP;

struct inode* vtfs_get_inode(
    struct super_block* sb, const struct inode* dir, umode_t mode, ino_t i_ino
);

#endif  // __FSMOD_SOURCE_INODE_H_