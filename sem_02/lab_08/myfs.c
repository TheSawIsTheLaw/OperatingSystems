#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/init.h>
#include<linux/fs.h>
#include<linux/time.h>

#define MYFS_MAGIC_NUMBER 0xFEE1DEAD
#define SLAB_NAME "threehudredbucks"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.");

struct myFSInode
{
    int i_mode;
    unsigned long i_ino;
};

static int cacheCounter = 0;
static int cacheSize = 64;
static struct kmem_cache *cache = NULL;
static struct myFSInode **inodes = NULL;

static struct myFSInode *getCacheInode(void)
{
    if (cacheCounter == cacheSize)
        return NULL;

    inodes[cacheCounter] = kmem_cache_alloc(cache, GFP_KERNEL);

    return inodes[cacheCounter++];
}


static struct inode *myFSMakeInode(struct super_block *sb, int mode)
{
    struct inode *ret = new_inode(sb);
    struct myFSInode *inode = NULL;

    if (ret)
    {
        inode_init_owner(ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        
        inode = getCacheInode();

        if (inode != NULL)
        {
            inode->i_mode = ret->i_mode;
            inode->i_ino = ret->i_ino;
        }

        ret->i_private = inode;
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
        printk(KERN_INFO ">>>FS was successfully mounted.");
    return entry;
}


static struct file_system_type myFSType = {
    .owner = THIS_MODULE,
    .name = "myFS",
    .mount = myFSMount,
    .kill_sb = kill_anon_super,
};

static int __init myFSInit(void)
{
    int ret = register_filesystem(&myFSType);
    if (ret != 0)
    {
        printk(KERN_ERR ">>>CANNOT REGISTER OWN FILESYSTEM!\n");
        return ret;
    }

    inodes = kmalloc(sizeof(struct myFSInode *) * cacheSize, GFP_KERNEL);
    if (inodes == NULL)
    {
        printk(KERN_ERR ">>>kmalloc for inodes error!");
        return -ENOMEM;
    }

    cache = kmem_cache_create(SLAB_NAME, sizeof(struct myFSInode), 0, 0, NULL);
    if (cache == NULL)
    {
        kfree(inodes);
        printk(KERN_ERR ">>>kmem_cache_create problem!");
        return -ENOMEM;
    }

    printk(KERN_DEBUG ">>>SUCCESS, YOUR FILESYSTEM IS READY TO GO!\n");
    return 0;
}

static void __exit myFSExit(void)
{
    int i = 0;
    while (i < cacheCounter)
    {
        kmem_cache_free(cache, inodes[i]);
        i++;
    }
    kmem_cache_destroy(cache);
    kfree(inodes);

    int ret = unregister_filesystem(&myFSType);
    if (ret != 0)
        printk(KERN_ERR ">>>CANNOT UNREGISTER FILESYSTEM!\n");

    printk(KERN_DEBUG ">>>MODULE FS UNLOADED!\n");
}

module_init(myFSInit);
module_exit(myFSExit);

// sudo mount -o loop -t myfs ./image ./dir