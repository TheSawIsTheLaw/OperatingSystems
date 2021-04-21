#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
static char kernel_buffer[256];
#define TEXT_GERMAN  "Hallo Welt"
#define TEXT_ENGLISH "Hello World"
#define PROC_FILE_NAME  "seq"
static struct proc_dir_entry *proc_file;
static char *output_string;

static int prochello_show( struct seq_file *m, void *v )
{
    seq_printf( m, "%s\n", output_string);
    return 0;
}

static int prochello_open(struct inode *inode, struct file *file)
{
    return single_open(file, prochello_show, NULL);
}

static ssize_t prochello_write( struct file *instanz, const char __user \
  *buffer, size_t max_bytes_to_write, loff_t *offset )
{
    ssize_t to_copy, not_copied;
    to_copy = min( max_bytes_to_write, sizeof(kernel_buffer) );
    not_copied = copy_from_user(kernel_buffer,buffer,to_copy);
    if (not_copied==0) 
    {
        printk("kernel_buffer: \"%s\"\n", kernel_buffer);
    
        if (strncmp("deutsch", kernel_buffer, 7)==0) {
            output_string = TEXT_GERMAN;
        }
    
        if (strncmp("english", kernel_buffer, 7)==0) {
            output_string = TEXT_ENGLISH;
        }
    }
    return to_copy - not_copied;
}

static const struct proc_ops prochello_fops = {
    proc_open: prochello_open,
    proc_write: prochello_write,
    proc_release: single_release,
    proc_read: seq_read
};

static int __init prochello_init(void)
{
    output_string = "Hello World";
    proc_file= proc_create_data( PROC_FILE_NAME, S_IRUGO | S_IWUGO, NULL, &prochello_fops, NULL);
    if (!proc_file)
        return -ENOMEM;
    return 0;
}
static void __exit prochello_exit(void)
{
    if( proc_file )
        remove_proc_entry( PROC_FILE_NAME, NULL );
}
module_init( prochello_init );
module_exit( prochello_exit );
MODULE_LICENSE("GPL");