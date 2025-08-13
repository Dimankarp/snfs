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

  struct snfs_inode* snfsi = snfs_inode_by_ino(dirno);
  if (snfsi == NULL) {
    d_add(child_dentry, NULL);
    return NULL;
  }
  struct snfs_dentry* snfsd = snfs_find_child(snfsi, name);
  if (snfsd == NULL) {
    d_add(child_dentry, NULL);
    return NULL;
  }
  struct snfs_inode* found = snfsd->inode;
  struct inode* inode = snfs_get_vfs_inode(parent_inode->i_sb, NULL, found->type, found->no);
  atomic_inc(&inode->i_count);
  d_add(child_dentry, inode);
  return NULL;
}

static int snfs_create_dentry(struct inode* parent_inode, struct dentry* child_dentry, int ftype) {
  ino_t dirino = parent_inode->i_ino;
  const char* name = child_dentry->d_name.name;

  if (strlen(name) > SNFS_NAME_SZ) {
    return -ENOANO;
  }
  LOG("Searching for inode %lu\n", dirino);
  struct snfs_inode* diri = snfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  struct snfs_dentry* snfsentry = kzalloc(sizeof(*snfsentry), GFP_KERNEL);
  strscpy(snfsentry->name, name, SNFS_NAME_SZ);
  if (snfsentry == NULL) {
    return -ENOMEM;
  }
  LOG("Allocated entry %s\n", snfsentry->name);
  int status = snfs_create_file(snfsentry, ftype);
  if (status < 0) {
    kfree(snfsentry);
    return status;
  }
  LOG("Created file with entry %s\n", snfsentry->name);
  snfs_add_child(diri, snfsentry);
  LOG("Added entry %s as child\n", snfsentry->name);
  struct inode* inode = snfs_get_vfs_inode(parent_inode->i_sb, NULL, ftype, snfsentry->inode->no);
  d_instantiate(child_dentry, inode);
  LOG("Added inode to childentry \n");
  return 0;
}

int snfs_create(
    struct mnt_idmap* map,
    struct inode* parent_inode,
    struct dentry* child_dentry,
    umode_t mode,
    bool b
) {
  LOG("[snfs_create]");
  return snfs_create_dentry(parent_inode, child_dentry, S_IFREG);
}

int snfs_mkdir(
    struct mnt_idmap* map, struct inode* parent_inode, struct dentry* child_dentry, umode_t mode
) {
  LOG("[snfs_mkdir]");
  return snfs_create_dentry(parent_inode, child_dentry, S_IFDIR);
}

int snfs_unlink(struct inode* parent_inode, struct dentry* child_dentry) {
  const char* name = child_dentry->d_name.name;
  ino_t dirino = parent_inode->i_ino;
  LOG("[snfs_unlink]");
  LOG("Searching for inode %lu\n", dirino);
  struct snfs_inode* diri = snfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  LOG("Searching %s \n", name);
  struct snfs_dentry* snfsd = snfs_find_child(diri, name);
  if (snfsd == NULL) {
    return -ENODATA;
  }
  return snfs_remove_file(snfsd, diri);
}

int snfs_rmdir(struct inode* parent_inode, struct dentry* child_dentry) {
  const char* name = child_dentry->d_name.name;
  ino_t dirino = parent_inode->i_ino;
  LOG("[snfs_rmdir]");
  LOG("Searching for inode %lu\n", dirino);
  struct snfs_inode* diri = snfs_inode_by_ino(dirino);
  if (diri == NULL) {
    return -ENODATA;
  }
  LOG("Found inode %lu\n", dirino);
  LOG("Searching %s \n", name);
  struct snfs_dentry* snfsd = snfs_find_child(diri, name);
  if (snfsd == NULL) {
    return -ENODATA;
  }
  return snfs_remove_dir(snfsd, diri);
}

int snfs_iterate_shared(struct file* filp, struct dir_context* ctx) {
  snfs_dump();

  struct dentry* dentry = filp->f_path.dentry;
  struct inode* inode = dentry->d_inode;
  ino_t dirino = inode->i_ino;
  int cur = 0;

  LOG("[snfs_iterate]");
  LOG("Searching for inode %lu\n", dirino);
  struct snfs_inode* diri = snfs_inode_by_ino(dirino);
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
  struct snfs_dentry* snfsd;
  mutex_lock(&diri->lock);
  list_for_each_entry(snfsd, &diri->children, node) {
    LOG("Cur: %d Pos: %lld", cur, ctx->pos);
    if (cur == ctx->pos) {
      ctx->pos++;
      LOG("Emitting %s", snfsd->name);
      dir_emit(ctx, snfsd->name, SNFS_NAME_SZ, snfsd->inode->no, snfsd->inode->type);
    }
    cur++;
  }
  mutex_unlock(&diri->lock);
  return 0;
}

ssize_t snfs_read(struct file* filp, char* buffer, size_t len, loff_t* offset) {
  ino_t fino = filp->f_inode->i_ino;
  LOG("[snfs_read]");
  LOG("Searching for inode %lu\n", fino);
  struct snfs_inode* filei = snfs_inode_by_ino(fino);
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

ssize_t snfs_write(struct file* filp, const char* buffer, size_t len, loff_t* offset) {
  ino_t dirino = filp->f_inode->i_ino;
  LOG("[snfs_write]");
  LOG("Searching for inode %lu\n", dirino);
  struct snfs_inode* filei = snfs_inode_by_ino(dirino);
  if (filei == NULL) {
    return -ENODATA;
  }
  LOG("Checking for dir %lu\n", dirino);
  if (S_ISDIR(filei->type))
    return -EISDIR;
  LOG("Not dir %lu\n", dirino);
  mutex_lock(&filei->lock);

  size_t newsz = *offset + len;
  snfs_set_buf_sz(filei, newsz);
  int status = copy_from_user(filei->buf + *offset, buffer, len);
  LOG("Copied from user offset: %d len %d status %d\n", *offset, len, status);
  mutex_unlock(&filei->lock);
  filp->f_inode->i_size = newsz;
  filp->f_inode->i_blkbits = 8;
  filp->f_inode->i_blocks = newsz;
  *offset += len;
  return len;
}

int snfs_fsync(struct file* file, loff_t start, loff_t end, int datasync) {
  return 0;
}
