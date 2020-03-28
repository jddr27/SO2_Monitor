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
 
 
struct task_struct *task;  /* Structure defined in sched.h for tasks/processes */
struct task_struct *task_child;  /* Structure needed to iterate through task children */
struct list_head *list;  /* Structure needed to iterate through the list in each task->children struct */
 
int todos, corr, durm, para, zomb;

char * get_task_state(long state)
{
    switch (state) {
        case TASK_RUNNING:
            corr++;
            return "R";
        case TASK_INTERRUPTIBLE:
            durm++;
            return "S";
        case TASK_ZOMBIE:
            zomb++;
            return "Z";
        case __TASK_STOPPED:
            para++;
            return "T";
        default:
        {
            return "?";
        }
    }
}

static int procinfo_proc_show(struct seq_file *m, void *v)
{
    todos = corr = durm = para = zomb = 0;
    seq_printf(m,"{\nProcs:[");
    for_each_process( task ){
        seq_printf(m,"{\n\"pid\": %d,\n\"nombre\": \"%s\",\n\"estado\": \"%s\",\n\"hijos\":[",task->pid, task->comm, get_task_state(task->state));
        list_for_each(list, &task->children){ 
            task_child = list_entry( list, struct task_struct, sibling );
            seq_printf(m, "\n\t{\n\t\t\"pid\": %d,\n\t\t\"nombre\": \"%s\",\n\t\t\"estado\": \"%s\"",task_child->pid, task_child->comm, get_task_state(task_child->state));
            seq_printf(m,"\n\t}\n");
        }
        todos++;
        seq_printf(m,"\n]},\n"); 
    }
    seq_printf(m,"],");
    seq_printf(m,"\n\"Todos\": %d,\n\"Corr\": %d,\n\"Durm\": %d,\n\"Para\": %d,\n\"Zomb\": %d\n}\n", todos, corr, durm, para, zomb);   
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

static int __init procsinfo_init(void)   
{
    printk(KERN_INFO "Inicia el modulo de Procs\n");   
    proc_create("201503393_procs", 0, NULL, &procinfo_proc_fops); 
    return 0;
}
     
static void __exit procsinfo_exit(void)
{
    printk(KERN_INFO "Termina el modulo de Procs\n");
}
 
module_init(procsinfo_init); 
module_exit(procsinfo_exit); 