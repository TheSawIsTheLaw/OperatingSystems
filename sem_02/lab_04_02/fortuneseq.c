#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba Dmitry");

#define MAX_LEN PAGE_SIZE

#define DIRNAME "fortSeqDir"
#define FILENAME "fortSeqFile"
#define SYMLINK "fortSeq"
#define FORTUNE_FILEPATH DIRNAME "/" FILENAME

static struct proc_dir_entry *fortDir = NULL;
static struct proc_dir_entry *fortFile = NULL;
static struct proc_dir_entry *fortSymlink = NULL;

static char *fortunesArr;
static int writeIndex;
static int readIndex;

static char copy[MAX_LEN];

int fortShow(struct seq_file *filep, void *v)
{
    printk(KERN_INFO "SEQ--***-- FORT: called show\n");
    int len;
    if (!writeIndex)
        return 0;
    len = snprintf(copy, MAX_LEN, "%s", &fortunesArr[readIndex]);
    seq_printf(filep, "%s", copy);
    readIndex += len;
    return 0;
}

ssize_t fortWrite(struct file *filp, const char __user *buf, size_t len, loff_t *offp)
{
    printk(KERN_INFO "SEQ--***-- FORT: called цкшеу\n");
    if (len > MAX_LEN - writeIndex + 1)
    {
        printk(KERN_ERR "SEQ--***-- FORT: fortunesArr overflow error\n");
        return -ENOSPC; // No enough space
    }
    if (copy_from_user(&fortunesArr[writeIndex], buf, len))
    {
        printk(KERN_ERR "SEQ--***-- FORT: copy_to_user error\n");
        return -EFAULT; // Bad address
    }

    writeIndex += len - 1;
    fortunesArr[writeIndex] = '\0';
    return len;
}

int fortOpen(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "SEQ--***-- FORT: called open\n");
    return single_open(file, fortShow, NULL);
}

int fortRelease(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "SEQ--***-- FORT: called release\n");
    return single_release(inode, file);
}

static struct proc_ops fops =
    {proc_read : seq_read, proc_write : fortWrite, proc_open : fortOpen, proc_release : fortRelease};

static void cleanup(void)
{
    if (fortSymlink)
        remove_proc_entry(SYMLINK, NULL);
    if (fortFile)
        remove_proc_entry(FILENAME, fortDir);
    if (fortDir)
        remove_proc_entry(DIRNAME, NULL);
    if (fortunesArr)
        vfree(fortunesArr);
}

static int noMemoryErrorExit(const char *s)
{
    cleanup();
    printk(KERN_ERR "SEQ--***-- FORT: %s error\n", s);
    return -ENOMEM; // No memory
}

static int __init fortInit(void)
{
    if (!(fortunesArr = vmalloc(MAX_LEN)))
        return noMemoryErrorExit("vmalloc");
    memset(fortunesArr, 0, MAX_LEN);

    if (!(fortDir = proc_mkdir(DIRNAME, NULL)))
        return noMemoryErrorExit("proc_mkdir");
    if (!(fortFile = proc_create(FILENAME, 0666, fortDir, &fops)))
        return noMemoryErrorExit("proc_create");
    if (!(fortSymlink = proc_symlink(SYMLINK, NULL, FORTUNE_FILEPATH)))
        return noMemoryErrorExit("proc_symlink");

    writeIndex = 0;
    readIndex = 0;

    printk(KERN_INFO "SEQ--***-- FORT: module loaded\n");
    return 0;
}

static void __exit fortExit(void)
{
    cleanup();
    printk(KERN_INFO "SEQ--***-- FORT: called read\n");
}

module_init(fortInit) module_exit(fortExit)
