#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yakuba D.");


static int __init md_init(void) 
{ 
   printk(KERN_INFO "Module init:\n");

   struct task_struct *task = &init_task; // Инициализация списка декрипторов процессов
   do
   {
       printk(KERN_INFO "~~~~~~ Proccess: %s (comm) - %d (ID), Parent: %s (comm) - %d (ID)\n", 
            task->comm, task->pid, task->parent->comm, task->parent->pid);
   } while ((task = next_task(task)) != &init_task); // Прохождения списка

   printk(KERN_INFO "~~~~~~ Proccess: %s (comm) - %d (ID), Parent: %s (comm) - %d (ID)\n",
		current->comm, current->pid, current->parent->comm, current->parent->pid); // получение информации о текущем процессе

   return 0; 
}

static void __exit md_exit(void) 
{ 
   printk(KERN_INFO "Module goes away... It's his final message. If you won't replace the battery...\n"); 
} 

module_init(md_init); 
module_exit(md_exit); 