#ifndef __FSMOD_SOURCE_IMPL_H_
#define __FSMOD_SOURCE_IMPL_H_

#include <linux/fs.h>
#include <linux/list.h>

#define SNFS_ROOT_NO 0
#define SNFS_NAME_SZ 16

struct snfs_inode {
  struct list_head node; /* list of snfs_inode */
  _Atomic size_t refs;
  ino_t no;
  int type;
  struct list_head children; /* list of snfs_dentry */
  char* buf;
  size_t bufsz;
  struct mutex lock;
};

struct snfs_dentry {
  struct list_head node;
  char name[SNFS_NAME_SZ];
  struct snfs_inode* inode;
};

struct snfs_superblock {
  struct list_head inodes;
  struct list_head dentries;
  struct snfs_inode* root;
  spinlock_t lock;
  _Atomic ino_t next_ino;
};

int snfs_init_sb(void);
int snfs_create_file(struct snfs_dentry* dentry, int type);
struct snfs_inode* snfs_inode_by_ino(ino_t ino);
struct snfs_dentry* snfs_find_child(struct snfs_inode* inode, const char* name);
void snfs_add_child(struct snfs_inode* dir, struct snfs_dentry* entry);
int snfs_remove_file(struct snfs_dentry* file, struct snfs_inode* from);
int snfs_remove_dir(struct snfs_dentry* dir, struct snfs_inode* from);
int snfs_set_buf_sz(struct snfs_inode* file, size_t newsz);
void snfs_dump(void);
int snfs_hard_link(struct snfs_inode* inode, struct snfs_dentry* new);
#endif  // __FSMOD_SOURCE_IMPL_H_s