#include "ops.h"

#include "impl.h"
#include "inode.h"

const struct inode_operations vtfs_inode_ops = {
    .lookup = vtfs_lookup, .create = vtfs_create,// .unlink = vtfs_unlink
};

const struct file_operations vtfs_file_ops = {.iterate_shared = vtfs_iterate};

struct dentry* vtfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
) {
  ino_t dirno = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;
  struct vtfs_inode* vtfsi = vtfs_inode_by_ino(dirno);
  if (vtfsi == NULL) {
    d_add(child_dentry, NULL);
    return ERR_PTR(-ENODATA);
  }
  struct vtfs_dentry* vtfsd = vtfs_find_child(vtfsi, name);
  if (vtfsi == NULL) {
    d_add(child_dentry, NULL);
    return ERR_PTR(-ENODATA);
  }
  struct vtfs_inode* found = vtfsd->inode;
  struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, found->type, found->no);
  atomic_inc(&inode->i_count);
  d_add(child_dentry, inode);
  return NULL;
}

int vtfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
) {
  ino_t dirino = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;

  if (strlen(name) > VTFS_NAME_SZ) {
    return -ENOANO;
  }

  struct vtfs_inode* diri = vtfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  struct vtfs_dentry* vtfsentry = kzalloc(sizeof(*vtfsentry), GFP_KERNEL);
  strcpy(vtfsentry->name, name);
  if (vtfsentry == NULL) {
    return -ENOMEM;
  }
  int status = vtfs_create_file(vtfsentry, S_IFREG);
  if (status < 0) {
    kfree(vtfsentry);
    return status;
  }
  vtfs_add_child(diri, vtfsentry);
  struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, vtfsentry->inode->no);
  inode->i_op = &vtfs_inode_ops;
  inode->i_fop = NULL;
  d_instantiate(child_dentry, inode);
  return 0;
}

// int vtfs_unlink(struct inode* parent_inode, struct dentry* child_dentry) {
//   const char* name = child_dentry->d_name.name;
//   ino_t root = parent_inode->i_ino;
//   if (root == 1000 && !strcmp(name, "test.txt")) {
//     mask &= ~1;
//   } else if (root == 1000 && !strcmp(name, "new_file.txt")) {
//     mask &= ~2;
//   }
//   return 0;
// }

int vtfs_iterate(struct file* filp, struct dir_context* ctx) {
  struct dentry* dentry = filp->f_path.dentry;
  struct inode* inode = dentry->d_inode;
  int cur = 0;
  ino_t ino = inode->i_ino;

  struct vtfs_inode* diri = vtfs_inode_by_ino(ino);
  if (diri == NULL) {
    return -ENODATA;
  }

  if (!S_ISDIR(inode->i_mode)) {
    return -ENOTDIR;
  }

  if (!dir_emit_dots(filp, ctx)) {
    return 0;
  }
  struct vtfs_dentry* vtfsd;
  list_for_each_entry(vtfsd, &diri->children, node) {
    if (cur == ctx->pos) {
      ctx->pos++;
      dir_emit(ctx, vtfsd->name, VTFS_NAME_SZ, vtfsd->inode->no, vtfsd->inode->type);
    }
    cur++;
  }
  return 0;
}
