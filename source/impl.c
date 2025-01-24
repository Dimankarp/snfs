#include "impl.h"

#include <linux/slab.h>

#include "util.h"

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
  inode->type = S_IFDIR;
  mutex_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  list_add(&inode->node, &sb.inodes);
  sb.root = inode;
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
  mutex_init(&inode->lock);
  INIT_LIST_HEAD(&inode->children);
  spin_lock(&sb.lock);
  list_add(&inode->node, &sb.inodes);
  spin_unlock(&sb.lock);
  dentry->inode = inode;
  return 0;
}

int vtfs_hard_link(struct vtfs_inode* inode, struct vtfs_dentry* new) {
  if (S_ISDIR(inode->type)) {
    return -EISDIR;
  }
  inode->refs++;
  new->inode = inode;
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
  mutex_lock(&from->lock);
  list_del(&file->node);
  mutex_unlock(&from->lock);
  kfree(file);
  return 0;
}

int vtfs_remove_dir(struct vtfs_dentry* dir, struct vtfs_inode* from) {
  if (!S_ISDIR(dir->inode->type)) {
    return -ENOTDIR;
  }
  struct vtfs_inode* vtfsi = dir->inode;
  mutex_lock(&vtfsi->lock);
  if (!list_empty(&vtfsi->children)) {
    mutex_unlock(&vtfsi->lock);
    return -ENOTEMPTY;
  }
  mutex_unlock(&vtfsi->lock);

  vtfsi->refs--;
  if (vtfsi == 0) {
    spin_lock(&sb.lock);
    list_del(&vtfsi->node);
    spin_unlock(&sb.lock);
    kfree(vtfsi);
  }
  mutex_lock(&from->lock);
  list_del(&dir->node);
  mutex_unlock(&from->lock);
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

void vtfs_add_child(struct vtfs_inode* dir, struct vtfs_dentry* entry) {
  mutex_lock(&dir->lock);
  list_add(&entry->node, &dir->children);
  mutex_unlock(&dir->lock);
}

int vtfs_set_buf_sz(struct vtfs_inode* file, size_t newsz) {
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

static void vtfs_dump_recursion(struct vtfs_inode* f) {
  struct vtfs_dentry* dentry;
  list_for_each_entry(dentry, &f->children, node) {
    LOG("Inode %d name: %s isdir %d", dentry->inode->no, dentry->name, S_ISDIR(dentry->inode->type)
    );
    if (dentry->inode->type == S_IFDIR)
      vtfs_dump_recursion(dentry->inode);
  }
}

void vtfs_dump(void) {
  vtfs_dump_recursion(sb.root);
}
