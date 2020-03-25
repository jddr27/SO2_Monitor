#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
 
 
struct task_struct *task;        /*    Structure defined in sched.h for tasks/processes    */
struct task_struct *task_child;        /*    Structure needed to iterate through task children    */
struct list_head *list;            /*    Structure needed to iterate through the list in each task->children struct    */
 
static int procinfo_proc_show(struct seq_file *m, void *v)
{
    for_each_process( task ){            /*    for_each_process() MACRO for iterating through each task in the os located in linux\sched\signal.h    */
        seq_printf(m,"\nPARENT PID: %d PROCESS: %s STATE: %ld",task->pid, task->comm, task->state);/*    log parent id/executable name/state    */
        list_for_each(list, &task->children){                        /*    list_for_each MACRO to iterate through task->children    */
 
            task_child = list_entry( list, struct task_struct, sibling );    /*    using list_entry to declare all vars in task_child struct    */
     
            seq_printf(m, "\nCHILD OF %s[%d] PID: %d PROCESS: %s STATE: %ld",task->comm, task->pid, /*    log child of and child pid/name/state    */
                task_child->pid, task_child->comm, task_child->state);
        }
        seq_printf(m,"-----------------------------------------------------");    /*for aesthetics*/
    }    
}

static int procinfo_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, procinfo_proc_show, NULL);
}

static const struct file_operations procinfo_proc_fops = {
        .open           = procinfo_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int __init procsinfo_init(void)   
{
    printk(KERN_INFO "Inicia el modulo de probando\n");   
    proc_create("probando", 0, NULL, &procinfo_proc_fops); 
    return 0;
}
     
static void __exit procsinfo_exit(void)
{
    printk(KERN_INFO "Termina el modulo de probando\n");
}
 
module_init(procsinfo_init); 
module_exit(procsinfo_exit); 