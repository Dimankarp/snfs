#include "ops.h"

#include "impl.h"
#include "util.h"
#include "vfs.h"

struct dentry* snfs_lookup(
    struct inode* parent_inode, struct dentry* child_dentry, unsigned int flag
);
int snfs_iterate_shared(struct file* filp, struct dir_context* ctx);
int snfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
);
int snfs_link(struct dentry* old_dentry, struct inode* parent_dir, struct dentry* new_dentry);
int snfs_unlink(struct inode* parent_inode, struct dentry* child_dentry);
int snfs_rmdir(struct inode* parent_inode, struct dentry* child_dentry);
int snfs_mkdir(
    struct mnt_idmap* map, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode
);

ssize_t snfs_read(struct file* filp, char* __user buffer, size_t len, loff_t* offset);
ssize_t snfs_write(struct file* filp, const char* __user buffer, size_t len, loff_t* offset);
int snfs_fsync(struct file*, loff_t, loff_t, int);

const struct inode_operations snfs_inode_ops = {
    .lookup = snfs_lookup,
    .create = snfs_create,
    .unlink = snfs_unlink,
    .mkdir = snfs_mkdir,
    .rmdir = snfs_rmdir,
    .link = snfs_link
};

const struct file_operations snfs_file_ops = {
    .iterate_shared = snfs_iterate_shared,
    .write = snfs_write,
    .read = snfs_read,
    .fsync = snfs_fsync
};

/* Inode ops */
struct dentry* snfs_lookup(
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

static int vtfs_create_dentry(struct inode* parent_inode, struct dentry* child_dentry, int ftype) {
  ino_t dirino = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;

  if (strlen(name) > VTFS_NAME_SZ) {
    return -ENOANO;
  }
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
  int status = vtfs_create_file(vtfsentry, ftype);
  if (status < 0) {
    kfree(vtfsentry);
    return status;
  }
  LOG("Created file with entry %s\n", vtfsentry->name);
  vtfs_add_child(diri, vtfsentry);
  LOG("Added entry %s as child\n", vtfsentry->name);
  struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, ftype, vtfsentry->inode->no);
  d_instantiate(child_dentry, inode);
  LOG("Added inode to childentry \n");
  return 0;
}

int vtfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
) {
  LOG("[vtfs_create]");
  return vtfs_create_dentry(parent_inode, child_dentry, S_IFREG);
}

int vtfs_mkdir(
    struct mnt_idmap* map, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode
) {
  LOG("[vtfs_mkdir]");
  return vtfs_create_dentry(parent_inode, child_dentry, S_IFDIR);
}

int vtfs_unlink(struct inode* parent_inode, struct dentry* child_dentry) {
  const char* name = child_dentry->d_name.name;
  ino_t dirino = parent_inode->i_ino;
  LOG("[vtfs_unlink]");
  LOG("Searching for inode %lu\n", dirino);
  struct vtfs_inode* diri = vtfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  LOG("Searching %s \n", name);
  struct vtfs_dentry* vtfsd = vtfs_find_child(diri, name);
  if (vtfsd == NULL) {
    return -ENODATA;
  }
  return vtfs_remove_file(vtfsd, diri);
}

int vtfs_rmdir(struct inode* parent_inode, struct dentry* child_dentry) {
  const char* name = child_dentry->d_name.name;
  ino_t dirino = parent_inode->i_ino;
  LOG("[vtfs_rmdir]");
  LOG("Searching for inode %lu\n", dirino);
  struct vtfs_inode* diri = vtfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  LOG("Searching %s \n", name);
  struct vtfs_dentry* vtfsd = vtfs_find_child(diri, name);
  if (vtfsd == NULL) {
    return -ENODATA;
  }
  return vtfs_remove_dir(vtfsd, diri);
}

int vtfs_iterate(struct file* filp, struct dir_context* ctx) {
  vtfs_dump();

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
  mutex_lock(&diri->lock);
  list_for_each_entry(vtfsd, &diri->children, node) {
    LOG("Cur: %d Pos: %lld", cur, ctx->pos);
    if (cur == ctx->pos) {
      ctx->pos++;
      LOG("Emitting %s", vtfsd->name);
      dir_emit(ctx, vtfsd->name, VTFS_NAME_SZ, vtfsd->inode->no, vtfsd->inode->type);
    }
    cur++;
  }
  mutex_unlock(&diri->lock);
  return 0;
}

ssize_t vtfs_read(struct file* filp, char* buffer, size_t len, loff_t* offset) {
  ino_t fino = filp->f_inode->i_ino;
  LOG("[vtfs_read]");
  LOG("Searching for inode %lu\n", fino);
  struct vtfs_inode* filei = vtfs_inode_by_ino(fino);
  if (filei == NULL) {
    return -ENODATA;
  }
  LOG("It's not null type is %d", filei->type);
  if (filei->type == S_IFDIR) {
    return -EISDIR;
  }

  if (offset == NULL) {
    return -1;
  }
  mutex_lock(&filei->lock);
  size_t toread = min(filei->bufsz - *offset, len);
  if (copy_to_user((void __user*)buffer, filei->buf + *offset, toread)) {
    mutex_unlock(&filei->lock);
    return -EFAULT;
  }
  *offset += toread;
  mutex_unlock(&filei->lock);
  return toread;
}

ssize_t vtfs_write(struct file* filp, const char* buffer, size_t len, loff_t* offset) {
  ino_t dirino = filp->f_inode->i_ino;
  LOG("[vtfs_write]");
  LOG("Searching for inode %lu\n", dirino);
  struct vtfs_inode* filei = vtfs_inode_by_ino(dirino);
  if (filei == NULL) {
    return -ENODATA;
  }
  LOG("Checking for dir %lu\n", dirino);
  if (S_ISDIR(filei->type))
    return -EISDIR;
  LOG("Not dir %lu\n", dirino);
  mutex_lock(&filei->lock);

  size_t newsz = *offset + len;
  vtfs_set_buf_sz(filei, newsz);
  int status = copy_from_user(filei->buf + *offset, buffer, len);
  LOG("Copied from user offset: %d len %d status %d\n", *offset, len, status);
  mutex_unlock(&filei->lock);
  filp->f_inode->i_size = newsz;
  filp->f_inode->i_blkbits = 8;
  filp->f_inode->i_blocks = newsz;
  *offset += len;
  return len;
}

int vtfs_fsync(struct file* file, loff_t start, loff_t end, int datasync) {
  return 0;
}

int vtfs_link(struct dentry* old_dentry, struct inode* parent_dir, struct dentry* new_dentry) {
  struct vtfs_inode* parenti = vtfs_inode_by_ino(parent_dir->i_ino);
  if (parenti == NULL) {
    return -ENODATA;
  }
  struct vtfs_inode* oldi = vtfs_inode_by_ino(old_dentry->d_inode->i_ino);
  if (oldi == NULL) {
    return -ENODATA;
  }

  vtfs_hard_link(struct vtfs_inode * inode, struct vtfs_dentry * new)
      vtfs_add_child(diri, vtfsentry);
  LOG("Added entry %s as child\n", vtfsentry->name);
  struct inode* inode = vtfs_get_inode(parent_inode->i_sb, NULL, ftype, vtfsentry->inode->no);
  d_instantiate(child_dentry, inode);
}