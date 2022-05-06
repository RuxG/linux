/*
 * SO2 Lab - Filesystem drivers
 * Exercise #1 (no-dev filesystem)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>

MODULE_DESCRIPTION("Simple no-dev filesystem");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define MYFS_BLOCKSIZE		4096
#define MYFS_BLOCKSIZE_BITS	12
#define MYFS_MAGIC		0xbeefcafe
#define LOG_LEVEL		KERN_ALERT

/* declarations of functions that are part of operation structures */

static int myfs_mknod(struct inode *dir,
		struct dentry *dentry, umode_t mode, dev_t dev);
static int myfs_create(struct inode *dir, struct dentry *dentry,
		umode_t mode, bool excl);
static int myfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

/* TODO 2: define super_operations structure */
static const struct super_operations myfs_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_drop_inode,
};

static const struct inode_operations myfs_dir_inode_operations = {
	/* TODO 5: Fill dir inode operations structure. */
	.create		= myfs_create,
	.mkdir		= myfs_mkdir,
	.mknod		= myfs_mknod,
	.lookup = simple_lookup,
	.link = simple_link,
	.unlink = simple_unlink,
	.rename		= simple_rename,
	.rmdir		= simple_rmdir,
      //...
};

static const struct file_operations myfs_file_operations = {
	.llseek         = generic_file_llseek,
	.read_iter      = generic_file_read_iter,
	.write_iter     = generic_file_write_iter,
	.mmap           = generic_file_mmap,
};

static const struct inode_operations myfs_file_inode_operations = {
	.setattr	= simple_setattr,
	.getattr	= simple_getattr,
};

static const struct address_space_operations myfs_aops = {
	.readpage		= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end		= simple_write_end,
};

struct inode *myfs_get_inode(struct super_block *sb, const struct inode *dir,
		int mode)
{
	struct inode *inode = new_inode(sb);

	if (!inode)
		return NULL;

	inode_init_owner(inode, dir, mode);
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_ino = get_next_ino();

	inode->i_mapping->a_ops = &myfs_aops;

	if (S_ISDIR(mode)) {
		
		//inode->i_op = &simple_dir_inode_operations;
		inode->i_fop = &simple_dir_operations;

		inode->i_op = &myfs_dir_inode_operations;
		
		inc_nlink(inode);
	} else if (S_ISREG(mode)) {
		inode->i_fop = &myfs_file_operations;
		inode->i_op = &myfs_file_inode_operations;
	}

	

	return inode;
}

static int
myfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct inode * inode = myfs_get_inode(dir->i_sb, dir, mode);
	int error = -ENOSPC;

	if (inode) {
		d_instantiate(dentry, inode);
		dget(dentry);	/* Extra count - pin the dentry in core */
		error = 0;
		dir->i_mtime = dir->i_ctime = current_time(dir);
	}
	return error;
}

static int myfs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
	int retval = myfs_mknod(dir, dentry, mode | S_IFDIR, 0);
	if (!retval)
		inc_nlink(dir);
	return retval;
}

static int myfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return myfs_mknod(dir, dentry, mode | S_IFREG, 0);
}
static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root_inode;
	struct dentry *root_dentry;

	/* TODO 2: fill super_block
	 *   - blocksize, blocksize_bits
	 *   - magic
	 *   - super operations
	 *   - maxbytes
	 */
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = MYFS_BLOCKSIZE;
	sb->s_blocksize_bits = MYFS_BLOCKSIZE_BITS;
	sb->s_magic = MYFS_MAGIC;
	sb->s_op = &myfs_ops;

	/* mode = directory & access rights (755) */
	root_inode = myfs_get_inode(sb, NULL,
			S_IFDIR | S_IRWXU | S_IRGRP |
			S_IXGRP | S_IROTH | S_IXOTH);

	printk(LOG_LEVEL "root inode has %d link(s)\n", root_inode->i_nlink);

	if (!root_inode)
		return -ENOMEM;

	root_dentry = d_make_root(root_inode);
	if (!root_dentry)
		goto out_no_root;
	sb->s_root = root_dentry;

	return 0;

out_no_root:
	iput(root_inode);
	return -ENOMEM;
}

static struct dentry *myfs_mount(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	/* TODO 1: call superblock mount function */
	return mount_nodev(fs_type, flags, data, myfs_fill_super);
}

/* TODO 1: define file_system_type structure */
static struct file_system_type myfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "myfs",
	.mount		= myfs_mount,
	.kill_sb	= kill_litter_super,
};

static int __init myfs_init(void)
{
	int err;

	/* TODO 1: register */
	err = register_filesystem(&myfs_fs_type);
	if (err) {
		printk(LOG_LEVEL "register_filesystem failed\n");
		return err;
	}

	return 0;
}

static void __exit myfs_exit(void)
{
	/* TODO 1: unregister */
	unregister_filesystem(&myfs_fs_type);
}

module_init(myfs_init);
module_exit(myfs_exit);
