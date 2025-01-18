#ifndef __FSMOD_SOURCE_IMPL_H_
#define __FSMOD_SOURCE_IMPL_H_

#include <linux/fs.h>
#include <linux/list.h>

#define VTFS_ROOT_NO 0
#define VTFS_NAME_SZ 16

struct vtfs_inode {
  struct list_head node; /* list of vtfs_inode */
  _Atomic size_t refs;
  ino_t no;
  int type;
  struct list_head children; /* list of vtfs_dentry */
  spinlock_t lock;
};

struct vtfs_dentry {
  struct list_head node;
  char name[VTFS_NAME_SZ];
  struct vtfs_inode* inode;
};

struct vtfs_superblock {
  struct list_head inodes;
  struct list_head dentries;
  spinlock_t lock;
  _Atomic ino_t next_ino;
};

int vtfs_init_sb(void);
int vtfs_create_file(struct vtfs_dentry* dentry, int type);
struct vtfs_inode* vtfs_inode_by_ino(ino_t ino);
struct vtfs_dentry* vtfs_find_child(struct vtfs_inode* inode, const char* name);
void vtfs_add_child(struct vtfs_inode* dir, struct vtfs_dentry* entry);
int vtfs_remove_file(struct vtfs_dentry* file, struct vtfs_inode* from);
#endif  // __FSMOD_SOURCE_IMPL_H_s