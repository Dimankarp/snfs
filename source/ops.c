#include "ops.h"

#include "impl.h"
#include "inode.h"
#include "util.h"

const struct inode_operations vtfs_inode_ops = {
    .lookup = vtfs_lookup, .create = vtfs_create,  // .unlink = vtfs_unlink
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
    return NULL;
  }
  struct vtfs_dentry* vtfsd = vtfs_find_child(vtfsi, name);
  if (vtfsd == NULL) {
    d_add(child_dentry, NULL);
    return NULL;
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
  LOG("[vtfs_create]");
  LOG("Searching for inode %lu\n", dirino);
  struct vtfs_inode* diri = vtfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  struct vtfs_dentry* vtfsentry = kzalloc(sizeof(*vtfsentry), GFP_KERNEL);
  strscpy(vtfsentry->name, name, VTFS_NAME_SZ);
  if (vtfsentry == NULL) {
    return -ENOMEM;
  }
  LOG("Allocated entry %s\n", vtfsentry->name);
  int status = vtfs_create_file(vtfsentry, S_IFREG);
  if (status < 0) {
    kfree(vtfsentry);
    return status;
  }
  LOG("Created file with entry %s\n", vtfsentry->name);
  vtfs_add_child(diri, vtfsentry);
  LOG("Added entry %s as child\n", vtfsentry->name);
  struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, S_IFREG, vtfsentry->inode->no);
  d_instantiate(child_dentry, inode);
  LOG("Added inode to childentry \n");
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
  ino_t dirino = inode->i_ino;
  int cur = 0;

  LOG("[vtfs_iterate]");
  LOG("Searching for inode %lu\n", dirino);
  struct vtfs_inode* diri = vtfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  if (!S_ISDIR(inode->i_mode)) {
    return -ENOTDIR;
  }
  LOG("Emitting nodes\n");
  if (!dir_emit_dots(filp, ctx)) {
    return 0;
  }
  cur = 2;
  LOG("Emitted node");
  struct vtfs_dentry* vtfsd;
  spin_lock(&diri->lock);
  list_for_each_entry(vtfsd, &diri->children, node) {
    LOG("Cur: %d Pos: %lld", cur, ctx->pos);
    if (cur == ctx->pos) {
      ctx->pos++;
      LOG("Emitting %s", vtfsd->name);
      dir_emit(ctx, vtfsd->name, VTFS_NAME_SZ, vtfsd->inode->no, vtfsd->inode->type);
    }
    cur++;
  }
  spin_unlock(&diri->lock);
  return 0;
}
