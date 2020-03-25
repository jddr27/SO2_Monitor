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
 
 
void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

static int cpuinfo_proc_show(struct seq_file *m, void *v)
{
        struct cpustats c;
        si_stat(&c);
#define K(x) ((x) << (PAGE_SHIFT - 10))
        seq_printf(m,
		"User:       %8lu por\n"
                "Nice:        %8lu por\n"
                "System:     %8lu por\n"
                "total:     %8lu por\n"
                ,
                K(c.user),
                K(c.nice),
                K(c.system),
                K(c.total)
                );

        arch_report_meminfo(m);
        return 0;
#undef K
}

static int cpuinfo_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, cpuinfo_proc_show, NULL);
}

static const struct file_operations cpuinfo_proc_fops = {
        .open           = cpuinfo_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int __init cpuinfo_init(void)   
{
    printk(KERN_INFO "Inicia el modulo de CPU\n");   
    proc_create("201503393_cpu", 0, NULL, &cpuinfo_proc_fops); 
    return 0;
}
     
static void __exit cpuinfo_exit(void)
{
    printk(KERN_INFO "Termina el modulo de CPU\n");
}
 
module_init(cpuinfo_init); 
module_exit(cpuinfo_exit); 