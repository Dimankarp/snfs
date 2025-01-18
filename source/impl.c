#include "impl.h"

#include <linux/slab.h>

static struct vtfs_superblock sb;

int vtfs_init_sb(void) {
  sb = (struct vtfs_superblock){
      .inodes = LIST_HEAD_INIT(sb.inodes),
      .dentries = LIST_HEAD_INIT(sb.dentries),
      .next_ino = 1,
  };
  spin_lock_init(&sb.lock);
  struct vtfs_inode* inode = kzalloc(sizeof(*inode), GFP_KERNEL);
  if (inode == NULL) {
    return -ENOMEM;
  }
  inode->refs = 1;
  inode->no = VTFS_ROOT_NO;
  inode->type = S_IFREG;
  spin_lock_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  list_add(&inode->node, &sb.inodes);
  return 0;
}

int vtfs_create_file(struct vtfs_dentry* dentry, int type) {
  struct vtfs_inode* inode = kzalloc(sizeof(*inode), GFP_KERNEL);
  if (inode == NULL) {
    return -ENOMEM;
  }
  inode->refs = 1;
  inode->no = sb.next_ino++;
  inode->type = type;
  spin_lock_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  spin_lock(&sb.lock);
  list_add(&inode->node, &sb.inodes);
  spin_unlock(&sb.lock);
  dentry->inode = inode;
  return 0;
}

int vtfs_remove_file(struct vtfs_dentry* file, struct vtfs_inode* from) {
  if (S_ISDIR(file->inode->type)) {
    return -EISDIR;
  }
  struct vtfs_inode* vtfsi = file->inode;
  vtfsi->refs--;
  if (vtfsi == 0) {
    spin_lock(&sb.lock);
    list_del(&vtfsi->node);
    spin_unlock(&sb.lock);
    kfree(vtfsi);
  }
  spin_lock(&from->lock);
  list_del(&file->node);
  spin_unlock(&from->lock);
  kfree(file);
  return 0;
}

int vtfs_remove_dir(struct vtfs_dentry* dir, struct vtfs_inode* from) {
  if (!S_ISDIR(dir->inode->type)) {
    return -ENOTDIR;
  }
  struct vtfs_inode* vtfsi = dir->inode;
  spin_lock(&vtfsi->lock);
  if (!list_empty(&vtfsi->children)) {
    spin_unlock(&vtfsi->lock);
    return -ENOTEMPTY;
  }
  spin_unlock(&vtfsi->lock);

  vtfsi->refs--;
  if (vtfsi == 0) {
    spin_lock(&sb.lock);
    list_del(&vtfsi->node);
    spin_unlock(&sb.lock);
    kfree(vtfsi);
  }
  spin_lock(&from->lock);
  list_del(&dir->node);
  spin_unlock(&from->lock);
  kfree(dir);
  return 0;
}

struct vtfs_inode* vtfs_inode_by_ino(ino_t ino) {
  struct vtfs_inode* inode;
  spin_lock(&sb.lock);
  list_for_each_entry(inode, &sb.inodes, node) {
    if (inode->no == ino) {
      spin_unlock(&sb.lock);
      return inode;
    }
  }
  spin_unlock(&sb.lock);
  return NULL;
}

struct vtfs_dentry* vtfs_find_child(struct vtfs_inode* inode, const char* name) {
  struct vtfs_dentry* dentry;
  spin_lock(&inode->lock);
  list_for_each_entry(dentry, &inode->children, node) {
    if (strcmp(dentry->name, name) == 0) {
      spin_unlock(&inode->lock);
      return dentry;
    }
  }
  spin_unlock(&inode->lock);
  return NULL;
}

void vtfs_add_child(struct vtfs_inode* dir, struct vtfs_dentry* entry) {
  spin_lock(&dir->lock);
  list_add(&entry->node, &dir->children);
  spin_unlock(&dir->lock);
}