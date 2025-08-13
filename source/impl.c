#include "impl.h"

#include <linux/slab.h>

#include "util.h"

static struct snfs_superblock sb;

int snfs_init_sb(void) {
  sb = (struct snfs_superblock){
      .inodes = LIST_HEAD_INIT(sb.inodes),
      .dentries = LIST_HEAD_INIT(sb.dentries),
      .next_ino = 1,
  };
  spin_lock_init(&sb.lock);
  struct snfs_inode* inode = kzalloc(sizeof(*inode), GFP_KERNEL);
  if (inode == NULL) {
    return -ENOMEM;
  }
  inode->refs = 1;
  inode->no = SNFS_ROOT_NO;
  inode->type = S_IFDIR;
  mutex_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  list_add(&inode->node, &sb.inodes);
  sb.root = inode;
  return 0;
}

int snfs_create_file(struct snfs_dentry* dentry, int type) {
  struct snfs_inode* inode = kzalloc(sizeof(*inode), GFP_KERNEL);
  if (inode == NULL) {
    return -ENOMEM;
  }
  inode->refs = 1;
  inode->no = sb.next_ino++;
  inode->type = type;
  mutex_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  spin_lock(&sb.lock);
  list_add(&inode->node, &sb.inodes);
  spin_unlock(&sb.lock);
  dentry->inode = inode;
  return 0;
}

int snfs_hard_link(struct snfs_inode* inode, struct snfs_dentry* new) {
  if (S_ISDIR(inode->type)) {
    return -EISDIR;
  }
  inode->refs++;
  new->inode = inode;
  return 0;
}

int snfs_remove_file(struct snfs_dentry* file, struct snfs_inode* from) {
  if (S_ISDIR(file->inode->type)) {
    return -EISDIR;
  }
  struct snfs_inode* snfsi = file->inode;
  snfsi->refs--;
  if (snfsi == 0) {
    spin_lock(&sb.lock);
    list_del(&snfsi->node);
    spin_unlock(&sb.lock);
    kfree(snfsi);
  }
  mutex_lock(&from->lock);
  list_del(&file->node);
  mutex_unlock(&from->lock);
  kfree(file);
  return 0;
}

int snfs_remove_dir(struct snfs_dentry* dir, struct snfs_inode* from) {
  if (!S_ISDIR(dir->inode->type)) {
    return -ENOTDIR;
  }
  struct snfs_inode* snfsi = dir->inode;
  mutex_lock(&snfsi->lock);
  if (!list_empty(&snfsi->children)) {
    mutex_unlock(&snfsi->lock);
    return -ENOTEMPTY;
  }
  mutex_unlock(&snfsi->lock);

  snfsi->refs--;
  if (snfsi == 0) {
    spin_lock(&sb.lock);
    list_del(&snfsi->node);
    spin_unlock(&sb.lock);
    kfree(snfsi);
  }
  mutex_lock(&from->lock);
  list_del(&dir->node);
  mutex_unlock(&from->lock);
  kfree(dir);
  return 0;
}

struct snfs_inode* snfs_inode_by_ino(ino_t ino) {
  struct snfs_inode* inode;
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

struct snfs_dentry* snfs_find_child(struct snfs_inode* inode, const char* name) {
  struct snfs_dentry* dentry;
  mutex_lock(&inode->lock);
  list_for_each_entry(dentry, &inode->children, node) {
    if (strcmp(dentry->name, name) == 0) {
      mutex_unlock(&inode->lock);
      return dentry;
    }
  }
  mutex_unlock(&inode->lock);
  return NULL;
}

void snfs_add_child(struct snfs_inode* dir, struct snfs_dentry* entry) {
  mutex_lock(&dir->lock);
  list_add(&entry->node, &dir->children);
  mutex_unlock(&dir->lock);
}

int snfs_set_buf_sz(struct snfs_inode* file, size_t newsz) {
  if (S_ISDIR(file->type)) {
    return -EISDIR;
  }
  char* new_buf = kzalloc(newsz, GFP_KERNEL);  // GFP_KERNEL isn't the right choice here probably
  if (new_buf == NULL) {
    return -ENOMEM;
  }

  if (file->buf != NULL) {
    memcpy(new_buf, file->buf, min(newsz, file->bufsz));
  }
  kfree(file->buf);
  file->buf = new_buf;
  file->bufsz = newsz;
  return 0;
}

static void snfs_dump_recursion(struct snfs_inode* f) {
  struct snfs_dentry* dentry;
  list_for_each_entry(dentry, &f->children, node) {
    LOG("Inode %d name: %s isdir %d", dentry->inode->no, dentry->name, S_ISDIR(dentry->inode->type)
    );
    if (dentry->inode->type == S_IFDIR)
      snfs_dump_recursion(dentry->inode);
  }
}

void snfs_dump(void) {
  snfs_dump_recursion(sb.root);
}
