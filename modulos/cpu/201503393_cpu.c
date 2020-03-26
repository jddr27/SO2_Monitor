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

#ifndef arch_irq_stat_cpu
#define arch_irq_stat_cpu(cpu) 0
#endif
#ifndef arch_irq_stat
#define arch_irq_stat() 0
#endif

#ifdef arch_idle_time

static u64 get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle;

	idle = kcs->cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

static u64 get_iowait_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 iowait;

	iowait = kcs->cpustat[CPUTIME_IOWAIT];
	if (cpu_online(cpu) && nr_iowait_cpu(cpu))
		iowait += arch_idle_time(cpu);
	return iowait;
}

#else

static u64 get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle, idle_usecs = -1ULL;

	if (cpu_online(cpu))
		idle_usecs = get_cpu_idle_time_us(cpu, NULL);

	if (idle_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcs->cpustat[CPUTIME_IDLE];
	else
		idle = idle_usecs * NSEC_PER_USEC;

	return idle;
}

static u64 get_iowait_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 iowait, iowait_usecs = -1ULL;

	if (cpu_online(cpu))
		iowait_usecs = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcs->cpustat[CPUTIME_IOWAIT];
	else
		iowait = iowait_usecs * NSEC_PER_USEC;

	return iowait;
}

#endif

static int cpuinfo_proc_show(struct seq_file *p, void *v)
{
	int i, j;
	u64 user, nice, system, idle, iowait, irq, softirq, steal;
	u64 guest, guest_nice;
	u64 sum = 0;
	u64 sum_softirq = 0;
	u64 total;
	long double divi, tmp, porce;
	unsigned int per_softirq_sums[NR_SOFTIRQS] = {0};
	struct timespec64 boottime;

	user = nice = system = idle = iowait =
		irq = softirq = steal = 0;
	guest = guest_nice = 0;
	getboottime64(&boottime);

	struct kernel_cpustat kcpustat;
	u64 *cpustat = kcpustat.cpustat;

	//kcpustat_cpu_fetch(&kcpustat, 0);
	kcpustat = kcpustat_cpu(0);

	user		+= cpustat[CPUTIME_USER];
	nice		+= cpustat[CPUTIME_NICE];
	system		+= cpustat[CPUTIME_SYSTEM];
	idle		+= get_idle_time(&kcpustat, i);
	iowait		+= get_iowait_time(&kcpustat, i);
	irq		+= cpustat[CPUTIME_IRQ];
	softirq		+= cpustat[CPUTIME_SOFTIRQ];
	steal		+= cpustat[CPUTIME_STEAL];
	guest		+= cpustat[CPUTIME_GUEST];
	guest_nice	+= cpustat[CPUTIME_GUEST_NICE];
	sum		+= kstat_cpu_irqs_sum(i);
	sum		+= arch_irq_stat_cpu(i);

	for (j = 0; j < NR_SOFTIRQS; j++) {
		unsigned int softirq_stat = kstat_softirqs_cpu(j, i);

		per_softirq_sums[j] += softirq_stat;
		sum_softirq += softirq_stat;
	}
	sum += arch_irq_stat();

	total = nsec_to_clock_t(user) + nsec_to_clock_t(nice) + nsec_to_clock_t(system) + 
                nsec_to_clock_t(idle) + nsec_to_clock_t(iowait) + nsec_to_clock_t(irq) + 
                nsec_to_clock_t(softirq) + nsec_to_clock_t(steal) + nsec_to_clock_t(guest) + 
                nsec_to_clock_t(guest_nice);
	divi = idle / total;
	tmp = 1.0 - divi;
	porce = tmp * 100;

    seq_put_decimal_ull(p, "cpu  ", nsec_to_clock_t(user));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(nice));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(system));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(idle));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(iowait));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(irq));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(softirq));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(steal));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(guest));
	seq_put_decimal_ull(p, " ", nsec_to_clock_t(guest_nice));
	seq_putc(p, '\n');
	seq_printf(m,"\"Total\": %Lf", porce);
	seq_putc(p, '\n');

	return 0;
}

static int cpuinfo_proc_open(struct inode *inode, struct file *file)
{
        unsigned int size = 1024 + 128 * num_online_cpus();
	/* minimum size to display an interrupt count : 2 bytes */
	size += 2 * nr_irqs;
	return single_open_size(file, cpuinfo_proc_show, NULL, size);
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