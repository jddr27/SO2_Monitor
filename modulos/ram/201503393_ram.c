#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

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
        seq_printf(m,"{\n\"Total\": %8lu,\n\"Usado\": %8lu,\n\"Per\": %8lu\n}\n", i.totalram,(i.totalram - i.freeram),(((i.totalram - i.freeram) * 100) / i.totalram));
        return 0;
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
	printk(KERN_INFO "Inicia el modulo de RAM\n");       
	proc_create("201503393_ram", 0, NULL, &meminfo_proc_fops);
        return 0;
}

static void __exit memo_exit(void)
{
   printk(KERN_INFO "Termina el modulo de RAM\n"); 
}

module_init(memo_init);
module_exit(memo_exit);