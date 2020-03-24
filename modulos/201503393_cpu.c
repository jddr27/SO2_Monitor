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

struct task_struct *task;        
struct task_struct *task_child;       
struct list_head *list;  
 
static int procinfo_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m,"201503393\nJose Daniel De Leon Ruiz\nDebian 10\n");
	seq_printf(m,"\nPARENT PID\t\t\tPROCESS\t\t\tSTATE\n");

	for_each_process( task ){
	        seq_printf(m, "%d\t\t\t%s\t\t\t%ld\n",task->pid, task->comm, task->state);
	} 
        return 0;
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

static int __init cpuinfo_init(void)   
{
    printk(KERN_INFO "Jose Daniel De Leon Ruiz\n");   
    proc_create("cpu_201503393", 0, NULL, &procinfo_proc_fops); 
    return 0;
}
     
static void __exit cpuinfo_exit(void)
{
    printk(KERN_INFO "Sistemas Operativos 1\n");
}
 
module_init(cpuinfo_init); 
module_exit(cpuinfo_exit); 