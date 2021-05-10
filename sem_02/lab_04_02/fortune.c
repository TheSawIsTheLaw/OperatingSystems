#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.V.");

#define MAX_LEN PAGE_SIZE

#define FORTUNE_DIRNAME "fortDir"
#define FORTUNE_FILENAME "fortFile"
#define FORTUNE_SYMLINK "fortune"
#define FORTUNE_FILEPATH FORTUNE_DIRNAME "/" FORTUNE_FILENAME

static struct proc_dir_entry *fortDir = NULL;
static struct proc_dir_entry *fortFile = NULL;
static struct proc_dir_entry *fortSymlink = NULL;

static char *fortunesArr; // Введённые фортунки
static int writeIndex;    // Индекс для добавления
static int readIndex;     // Индекс для чтения

static char copy[MAX_LEN];

ssize_t fortRead(struct file *filep, char __user *buf, size_t count, loff_t *offp)
{
    printk(KERN_INFO "--***-- FORT: called read\n");

    if (*offp > 0 || !writeIndex)
        return 0;

    if (readIndex >= writeIndex)
        readIndex = 0;

    int len = snprintf(copy, MAX_LEN, "%s\n", &fortunesArr[readIndex]);
    if (copy_to_user(buf, copy, len))
    {
        printk(KERN_ERR "--***-- FORT: copy_to_user error\n");
        return -EFAULT;
    }

    readIndex += len;
    *offp += len;

    return len;
}

ssize_t fortWrite(struct file *file, const char __user *buf, size_t len, loff_t *offp)
{
    printk(KERN_INFO "--***-- FORT: called write\n");

    if (len > MAX_LEN - writeIndex + 1)
    {
        printk(KERN_ERR "--***-- FORT: fortunesArr overflow error\n");
        return -ENOSPC; // No enough space
    }

    if (copy_from_user(&fortunesArr[writeIndex], buf, len))
    {
        printk(KERN_ERR "--***-- FORT: copy_to_user error\n");
        return -EFAULT; // Bad address
    }

    writeIndex += len;
    fortunesArr[writeIndex - 1] = '\0';

    return len;
}

// cat /proc/fortune -> result
int fortOpen(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "--***-- FORT: called open\n");

    return 0;
}

int fortRelease(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "--***-- FORT: called release\n");

    return 0;
}

static struct proc_ops fops =
    {proc_read : fortRead, proc_write : fortWrite, proc_open : fortOpen, proc_release : fortRelease};

static void cleanup(void)
{
    if (fortSymlink)
        remove_proc_entry(FORTUNE_SYMLINK, NULL);
    if (fortFile)
        remove_proc_entry(FORTUNE_FILENAME, fortDir);
    if (fortDir)
        remove_proc_entry(FORTUNE_DIRNAME, NULL);
    if (fortunesArr)
        vfree(fortunesArr);
}

static int noMemoryErrorExit(const char *s)
{
    cleanup();
    printk(KERN_ERR "--***-- FORT: %s error\n", s);
    return -ENOMEM; // No memory
}

static int __init fortInit(void)
{
    if (!(fortunesArr = vmalloc(MAX_LEN)))
        return noMemoryErrorExit("vmalloc");
    memset(fortunesArr, 0, MAX_LEN);

    if (!(fortDir = proc_mkdir(FORTUNE_DIRNAME, NULL)))
        return noMemoryErrorExit("proc_mkdir");
    if (!(fortFile = proc_create(FORTUNE_FILENAME, 0666, fortDir, &fops))) // 666 - !!!!!!!!!!!
        return noMemoryErrorExit("proc_create");
    if (!(fortSymlink = proc_symlink(FORTUNE_SYMLINK, NULL, FORTUNE_FILEPATH)))
        return noMemoryErrorExit("proc_symlink");

    writeIndex = 0;
    readIndex = 0;

    printk(KERN_INFO "--***-- FORT: module loaded\n");
    return 0;
}

static void __exit fortExit(void)
{
    cleanup();
    printk(KERN_INFO "--***-- FORT: module unloaded\n");
}

module_init(fortInit) module_exit(fortExit)
