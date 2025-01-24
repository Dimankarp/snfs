#ifndef __FSMOD_SOURCE_OPS_H_
#define __FSMOD_SOURCE_OPS_H_

#include <linux/module.h>
#include <linux/types.h>

extern const struct inode_operations snfs_inode_ops;
extern const struct file_operations snfs_file_ops;

#endif  // __FSMOD_SOURCE_OPS_H_