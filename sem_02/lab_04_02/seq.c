#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.V.");

#define TEXT_BOKMAL "Hei Verden!"
#define TEXT_ENGLISH "Hello World"
#define PROC_FILE_NAME "seq"

#define KERNEL_BUF_SIZE 256

static char kernelBuf[KERNEL_BUF_SIZE];
static struct proc_dir_entry *procFile;
static char *outString;

static int procShow(struct seq_file *m, void *v)
{
    seq_printf(m, "%s\n", outString);
    return 0;
}

static int procOpen(struct inode *inode, struct file *file)
{
    return single_open(file, procShow, NULL);
}

static ssize_t procWrite(struct file *instanz, const char __user *buffer, size_t length,
                               loff_t *offset)
{
    ssize_t toCopy, notCopied;
    toCopy = min(length, sizeof(kernelBuf));
    notCopied = copy_from_user(kernelBuf, buffer, toCopy);
    if (!notCopied)
    {
        printk("kernelBuf: \"%s\"\n", kernelBuf);

        if (strncmp("bokmal", kernelBuf, 6) == 0)
            outString = TEXT_BOKMAL;
        else
            outString = TEXT_ENGLISH;
    }
    return toCopy - notCopied;
}

static const struct proc_ops procFOPS =
{
    proc_open : procOpen, 
    proc_write : procWrite, 
    proc_release : single_release, 
    proc_read : seq_read
};

static int __init procInit(void)
{
    outString = "No hello :(";
    procFile = proc_create_data(PROC_FILE_NAME, S_IRUGO | S_IWUGO, NULL, &procFOPS, NULL);
    if (!procFile)
        return -ENOMEM;
    return 0;
}
static void __exit procExit(void)
{
    if (procFile)
        remove_proc_entry(PROC_FILE_NAME, NULL);
}


module_init(procInit);
module_exit(procExit);
