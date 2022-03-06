#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* TODO: add missing headers */
#include <linux/sched.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int my_proc_init(void)
{
	struct task_struct *p;

	/* TODO: print current process pid and its name */
	p = current;
	pr_info("Current process name: %s, process pid: %d", p->comm, p->pid);

	pr_info("Process list:\n");
	/* TODO: print the pid and name of all processes */
        for_each_process(p) {
		pr_info("Process name: %s, process pid: %d\n", p->comm, p->pid);
	}
	return 0;
}

static void my_proc_exit(void)
{
	struct task_struct *p;

	/* TODO: print current process pid and its name */
	p = current;
	/* TODO: print current process pid and name */
	pr_info("Process name: %s, process pid: %d", p->comm, p->pid);
}

module_init(my_proc_init);
module_exit(my_proc_exit);
