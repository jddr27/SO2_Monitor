#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>


u64 nsec_to_clock_t(u64 x)
{
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	/*
         * max relative error 5.7e-8 (1.8s per year) for USER_HZ <= 1024,
         * overflow after 64.99 years.
         * exact for HZ=60, 72, 90, 120, 144, 180, 300, 600, 900, ...
         */
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}


static int cpuinfo_proc_show(struct seq_file *p, void *v)
{
	int i;
	u64 user, nice, system, idle, iowait, irq, softirq, steal;
	u64 guest, guest_nice;
	u64 total;

	user = nice = system = idle = iowait = irq = softirq = steal = 0;
	guest = guest_nice = 0;
	total = 0;

	for_each_possible_cpu(i) {
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
		nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		idle += kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
		iowait += kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
		irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		guest_nice += kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
	}

	total = user + nice + system + idle + iowait + irq + 
            softirq + steal + guest + guest_nice;

	/*seq_put_decimal_ull(p, "cpu  ", nsec_to_clock_t(user));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(nice));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(system));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(idle));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(iowait));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(irq));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(softirq));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(steal));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(guest));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(guest_nice));
	seq_put_decimal_ull(p, " = ", nsec_to_clock_t(total));
	seq_put_decimal_ull(p, " = ", nsec_to_clock_t(total2));
	seq_putc(p, '\n');*/
	
	seq_put_decimal_ull(p, "{\n\"Total\":  ", nsec_to_clock_t(total));
	seq_put_decimal_ull(p, ",\n\"Idle\": ", nsec_to_clock_t(idle));
	seq_printf(p, "\n}\n");

	return 0;
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