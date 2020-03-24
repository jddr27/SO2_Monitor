#include <linux/module.h>    // Incluido para todos los modulos de kernel
#include <linux/kernel.h>    // Incluido para KERN_INFO
#include <linux/init.h>        // Incluido para __init y __exit macros

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

static int meminfo_proc_show(struct seq_file *m, void *v)
{
        struct sysinfo i;
        si_meminfo(&i);
#define K(x) ((x) << (PAGE_SHIFT - 10))
        seq_printf(m,
		"201503393\nJose Daniel De Leon Ruiz\nDebian 10\n"
                "MemTotal:       %8lu kB\n"
                "MemFree:        %8lu kB\n"
                "Porcentaje:     %8lu kB\n"
                ,
                K(i.totalram),
                K(i.freeram),
                K(i.freeram * 100 / i.totalram)
                );

        arch_report_meminfo(m);
        return 0;
#undef K
}

static int meminfo_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, meminfo_proc_show, NULL);
}

static const struct file_operations meminfo_proc_fops = {
        .open           = meminfo_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int __init memo_init(void)
{
	printk(KERN_INFO "201503393\n");       
	proc_create("memo_201503393", 0, NULL, &meminfo_proc_fops);
        return 0;
}

static void __exit memo_exit(void)
{
   printk(KERN_INFO "Sistemas Operativos 1\n"); 
}

module_init(memo_init);
module_exit(memo_exit);