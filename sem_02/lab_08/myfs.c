#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/init.h>
#include<linux/fs.h>
#include<linux/time.h>

#define MYFS_MAGIC_NUMBER 0xFEE1DEAD

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.");



static struct inode *myFSMakeInode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);
    struct myFSInode
    {
    int i_mode;
    unsigned long i_ino;
    }myFSInode;

    if (ret)
    {
        inode_init_owner(ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = &myFSInode;
    }

    return ret;
}

static void myFSPutSuper(struct super_block *sb)
{
    printk(KERN_DEBUG ">>>Superblock is destroyed!");
}

static struct super_operations const myFSSuperOps = {
    .put_super = myFSPutSuper,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

static int myFSFillSB(struct super_block *sb, void *data, int silent)
{
    struct inode *root = NULL;

    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYFS_MAGIC_NUMBER;
    sb->s_op = &myFSSuperOps;

    root = myFSMakeInode(sb, S_IFDIR | 0755); // S_IFDIR - маска, говорящая о том, что создаём каталог
    if (!root)
    {
        printk(KERN_ERR ">>>Inode allocation error");
        return -ENOMEM;
    }
    root->i_op = &simple_dir_inode_operations;
    root->i_fop = &simple_dir_operations;
    sb->s_root = d_make_root(root);

    if (!sb->s_root)
    {
        printk(KERN_ERR ">>>Root creation failed!");
        iput(root);
        
        return -ENOMEM;
    }

    return 0;
}

static struct dentry *myFSMount(struct file_system_type *type, int flags, char const *dev, void *data)
{
    struct dentry *const entry = mount_nodev(type, flags, data, myFSFillSB);
    if (IS_ERR(entry))
        printk(KERN_ERR ">>>Mounting failed!");
    else
        printk(KERN_DEBUG ">>>FS was successfully mounted.");
    return entry;
}


static struct file_system_type myFSType = {
    .owner = THIS_MODULE,
    .name = "myFS",
    .mount = myFSMount,
    .kill_sb = kill_block_super,
};

static int __init myFSInit(void)
{
    int ret = register_filesystem(&myFSType);
    if (ret != 0)
    {
        printk(KERN_ERR ">>>CANNOT REGISTER OWN FILESYSTEM!\n");
        return ret;
    }
    printk(KERN_DEBUG ">>>SUCCESS, YOUR FILESYSTEM IS READY TO GO!\n");
    return 0;
}

static void __exit myFSExit(void)
{
    int ret = unregister_filesystem(&myFSType);
    if (ret != 0)
        printk(KERN_ERR ">>>CANNOT UNREGISTER FILESYSTEM!\n");

    printk(KERN_DEBUG ">>>MODULE FS UNLOADED!\n");
}

module_init(myFSInit);
module_exit(myFSExit);