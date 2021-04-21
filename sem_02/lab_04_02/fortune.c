#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.V.");

#define MAX_COOKIE_LENGTH PAGE_SIZE

#define FORTUNE_DIRNAME "fortDir"
#define FORTUNE_FILENAME "fortFile"
#define FORTUNE_SYMLINK "fortune"
#define FORTUNE_FILEPATH FORTUNE_DIRNAME "/" FORTUNE_FILENAME

static struct proc_dir_entry *fortDir = NULL;
static struct proc_dir_entry *fortFile = NULL;
static struct proc_dir_entry *fortSymlink = NULL;

static char *cookiePot; // Хранилище наших фортунок
static int cookieIndex; // Индекс для добавления печеньки
static int nextFortune; // Индекс для чтения печеньки

static char tmp[MAX_COOKIE_LENGTH];

ssize_t fortune_read(struct file *filep, char __user *buf, size_t count, loff_t *offp)
{
    if (*offp > 0 || !cookieIndex)
        return 0;

    if (nextFortune >= cookieIndex)
        nextFortune = 0;

    int len = snprintf(tmp, MAX_COOKIE_LENGTH, "%s\n", &cookiePot[nextFortune]);
    if (copy_to_user(buf, tmp, len))
    {
        printk(KERN_ERR "--***-- FORT: copy_to_user error\n");
        return -EFAULT;
    }

    nextFortune += len;
    *offp += len;

    return len;
}

ssize_t fortune_write(struct file *file, const char __user *buf, size_t len, loff_t *offp)
{
    if (len > MAX_COOKIE_LENGTH - cookieIndex + 1)
    {
        printk(KERN_ERR "--***-- FORT: cookiePot overflow error\n");
        return -ENOSPC; // No enough space
    }

    if (copy_from_user(&cookiePot[cookieIndex], buf, len))
    {
        printk(KERN_ERR "--***-- FORT: copy_to_user error\n");
        return -EFAULT; // Bad address
    }

    cookieIndex += len;
    cookiePot[cookieIndex - 1] = '\0';

    return len;
}

// cat /proc/fortune -> result
int fortune_open(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "--***-- FORT: called open\n");

    return 0;
}

int fortune_release(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "--***-- FORT: called release\n");

    return 0;
}

static struct proc_ops fops =
{
    proc_read : fortune_read, 
    proc_write : fortune_write, 
    proc_open : fortune_open, 
    proc_release : fortune_release
};

static void cleanup(void)
{
    if (fortSymlink)
        remove_proc_entry(FORTUNE_SYMLINK, NULL);
    if (fortFile)
        remove_proc_entry(FORTUNE_FILENAME, fortDir);
    if (fortDir)
        remove_proc_entry(FORTUNE_DIRNAME, NULL);
    if (cookiePot)
        vfree(cookiePot);
}

static int noMemoryErrorExit(const char *s)
{
    cleanup();
    printk(KERN_ERR "--***-- FORT: %s error\n", s);
    return -ENOMEM; // No memory
}

static int __init fortune_init(void)
{
    if (!(cookiePot = vmalloc(MAX_COOKIE_LENGTH)))
        return noMemoryErrorExit("vmalloc");
    memset(cookiePot, 0, MAX_COOKIE_LENGTH);

    if (!(fortDir = proc_mkdir(FORTUNE_DIRNAME, NULL)))
        return noMemoryErrorExit("proc_mkdir");
    if (!(fortFile = proc_create(FORTUNE_FILENAME, 0666, fortDir, &fops))) // 666 - !!!!!!!!!!!
        return noMemoryErrorExit("proc_create");
    if (!(fortSymlink = proc_symlink(FORTUNE_SYMLINK, NULL, FORTUNE_FILEPATH)))
        return noMemoryErrorExit("proc_symlink");

    cookieIndex = 0;
    nextFortune = 0;

    printk(KERN_INFO "--***-- FORT: module loaded\n");
    return 0;
}

static void __exit fortune_exit(void)
{
    cleanup();
    printk(KERN_INFO "--***-- FORT: module unloaded\n");
}

module_init(fortune_init) 
module_exit(fortune_exit)
