#include "ops.h"

#include "inode.h"

static unsigned long long mask = 0;

const struct inode_operations vtfs_inode_ops = {
    .lookup = vtfs_lookup, .create = vtfs_create, .unlink = vtfs_unlink
};

const struct file_operations vtfs_file_ops = {.iterate_shared = vtfs_iterate};

struct dentry* vtfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
) {
  ino_t root = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;
  if (root == 1000 && !strcmp(name, "test.txt") && ((mask & 1) != 0)) {
    struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, 101);
    atomic_inc(&inode->i_count);
    d_add(child_dentry, inode);
    return child_dentry;
  } else if (root == 1000 && !strcmp(name, "dir")) {
    struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, S_IFDIR, 200);
    atomic_inc(&inode->i_count);
    d_add(child_dentry, inode);
    return child_dentry;
  }
  d_add(child_dentry, NULL);
  return NULL;
}

int vtfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
) {
  ino_t root = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;
  if (root == 1000 && !strcmp(name, "test.txt")) {
    struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, S_IFREG | S_IRWXUGO, 101);
    inode->i_op = &vtfs_inode_ops;
    inode->i_fop = NULL;
    pr_alert("Create test.txt mask: %llu", mask);
    mask |= 1;
    pr_alert("End test.txt mask: %llu", mask);
    d_instantiate(child_dentry, inode);
  }
  return 0;
}

int vtfs_unlink(struct inode* parent_inode, struct dentry* child_dentry) {
  const char* name = child_dentry->d_name.name;
  ino_t root = parent_inode->i_ino;
  if (root == 1000 && !strcmp(name, "test.txt")) {
    mask &= ~1;
  } else if (root == 1000 && !strcmp(name, "new_file.txt")) {
    mask &= ~2;
  }
  return 0;
}

int vtfs_iterate(struct file* filp, struct dir_context* ctx) {
  char fsname[10];
  struct dentry* dentry = filp->f_path.dentry;
  struct inode* inode = dentry->d_inode;
  unsigned long offset = filp->f_pos;
  int stored = 0;
  ino_t ino = inode->i_ino;

  unsigned char ftype;
  ino_t dino;
  pr_alert("Pos: %lld Mask: %llu", ctx->pos, mask);
  while (ctx->pos < 3) {
    if (ino == 1000) {
      if (ctx->pos == 0) {
        strcpy(fsname, ".");
        ftype = DT_DIR;
        dino = ino;

      } else if (ctx->pos == 1) {
        strcpy(fsname, "..");
        ftype = DT_DIR;
        dino = dentry->d_parent->d_inode->i_ino;
      } else if (ctx->pos == 2 && (mask & 1) != 0) {
        strcpy(fsname, "test.txt");
        ftype = DT_REG;
        dino = 101;
      } else if (ctx->pos == 3 && (mask & 2) != 0) {
        strcpy(fsname, "new_file.txt");
        ftype = DT_REG;
        dino = 102;
      } else {
        return stored;
      }
      stored++;
      ctx->pos++;
      dir_emit(ctx, fsname, 10, dino, ftype);
    }
  }
  return stored;
}
