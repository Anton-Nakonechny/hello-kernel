#include <linux/init.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include "hello-keyboard.h"

MODULE_LICENSE("Dual BSD/GPL");

#define PROC_DIR_NAME "hello"
#define PROC_PROBLEM_MSG "You are having problem with proc."
#define	DEFAULT_RECIPIENT "World"
#define	DEFAULT_RECIPIENT_STRLEN 5
#define MY_TICK 2

#ifdef CONFIG_PROC_FS

#define PROC_DO_BUG_NAME "do_bug"
#define PROC_TASKLET_NAME "tasklet"
#define PROC_WORKQUEUE_NAME "workqueue"
#define PROC_IRQ_NAME "kthread"
#define PROC_RECIPIENT_NAME "recipient"
#define PROC_RECIPIENT_MAX_SIZE 20
#define PROC_DO_BUG_MAX_SIZE 8
#define PROC_KEYWORD_CALL_INIT "init"

#define my_irq 17

int read_proc_int(char *buf, char **start, off_t offset, int count, int *eof, void *data);
int write_proc_do_bug(struct file *file, const char *buffer, unsigned long count, void *data);

static struct proc_dir_entry *proc_do_bug_entry = NULL, 
							 *proc_tasklet_entry = NULL, 
							 *proc_workqueue_entry = NULL,
							 *proc_irq_entry = NULL,
							 *proc_root_dir = NULL;

static unsigned char proc_keyword_size;
#endif

static void my_precious_timer_handler(unsigned long);
static void my_precious_tasklet_handler(unsigned long data);

static char *recipient = DEFAULT_RECIPIENT;
static int irq_num = 0;

module_param (irq_num, int, S_IRUGO);

static int dev_id = 323;
static struct context * my_context = NULL;

#define proc_bye(label) do \
	{\
		printk(KERN_ERR PROC_PROBLEM_MSG);\
		goto label;\
	} while(0)
/*
static void my_precious_workqueue_handler(struct work_struct *work)
{
	try_arm_timer(&(my_context->timer),&(my_context->timer_lock),"workqueue", &(my_context->workqueue_counter), 1);
}

static int my_precious_kthread(void * data)
{
	while(1)
	{
		if (kthread_should_stop())
			break;
		if(my_context->should_update_timer)
		{
			try_arm_timer(&(my_context->timer),&(my_context->timer_lock),"kthread", &(my_context->kthread_counter), 1);
			my_context->should_update_timer = 0;
		}
		set_current_state (TASK_INTERRUPTIBLE);
//schedule_timeout_interruptible (HZ / 2);
		schedule();
	}
	do_exit(0);
}
*/

irqreturn_t hard_handler(int irq, void *dev_id)
{
	if(&dev_id == 323 && printk_ratelimit())
		printk(KERN_NOTICE "You can't hide yourself!\n");
	return IRQ_HANDLED;
}

static void my_precious_tasklet_handler(unsigned long data)
{
//	try_arm_timer(&(my_context->timer),&(my_context->timer_lock),"tasklet", &(my_context->tasklet_counter), 0);
}

/*
static void my_precious_timer_handler(unsigned long data)
{
	printk(KERN_ALERT "Hello-timer expired! Counter is %lu\n", (my_context->timer_counter)++);
//	tasklet_schedule(&(my_context->tasklet));
	mb();
	wake_up_process(my_context->task);
	schedule_work(&(my_context->work));
}
*/
static int hello_init(void)
{
	int register_result;

	printk(KERN_INFO "Hello, %s!\n", recipient);
	
	if (!( my_context = kmalloc(sizeof(struct context), GFP_KERNEL)))
	{
		printk(KERN_ALERT "Couldn't allocate memmory in hello driver.");
		goto err;
	};

	memset(my_context, 0, sizeof(struct context));

#ifdef CONFIG_PROC_FS
	if (!(proc_root_dir = proc_mkdir(PROC_DIR_NAME, NULL)))
		proc_bye(err_remove_proc_dir);	
	
	if (!(proc_tasklet_entry = create_proc_entry(PROC_TASKLET_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_tasklet);
	proc_tasklet_entry->data = &(my_context->tasklet_counter);
	proc_tasklet_entry->read_proc = read_proc_int;
	

	if (!(proc_irq_entry = create_proc_entry(PROC_IRQ_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_irq);
	proc_irq_entry->data = &(my_context->irq_counter);
	proc_irq_entry->read_proc = read_proc_int;

	proc_keyword_size = strlen (PROC_KEYWORD_CALL_INIT);

#endif
	printk(KERN_INFO "REGISTER TRY RES IS %i", request_any_context_irq(irq_num = irq_num ? irq_num : my_irq, hard_handler, IRQF_SHARED | IRQF_TRIGGER_MASK, "bazinga", 153));
	if (register_result = request_irq(irq_num = irq_num ? irq_num : my_irq, hard_handler, IRQF_SHARED | IRQF_TRIGGER_MASK, "bazinga", NULL)) 
	{
		printk(KERN_ERR "Couldn't register ! res = %i EBUSY=%i", register_result, EBUSY);
		my_context->registered = 0;
	}
	else
		my_context->registered = 1;

//	tasklet_init(&(my_context->tasklet), my_precious_tasklet_handler, 0x0);

	return 0;

#ifdef CONFIG_PROC_FS
err_remove_proc_irq:
	remove_proc_entry(PROC_IRQ_NAME, proc_root_dir);
err_remove_proc_workqueue:
	remove_proc_entry(PROC_WORKQUEUE_NAME, proc_root_dir);
err_remove_proc_tasklet:
	remove_proc_entry(PROC_TASKLET_NAME, proc_root_dir);
err_remove_proc_dir:
	remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
err_free:
	kfree(my_context);
err:
	
	return -1;
}

static void hello_exit(void)
{
#ifdef CONFIG_PROC_FS
	if (proc_tasklet_entry)
		remove_proc_entry(PROC_TASKLET_NAME, proc_root_dir);
	
	if (proc_irq_entry)
		remove_proc_entry(PROC_IRQ_NAME, proc_root_dir);
	
	if (proc_workqueue_entry)
		remove_proc_entry(PROC_WORKQUEUE_NAME, proc_root_dir);
	
	if (proc_root_dir)
		remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
	
	//tasklet_kill (&(my_context->tasklet));
	
	printk(KERN_ALERT "Goodbye, cruel %s!\n", recipient);
	if(my_context->registered)
		free_irq(my_irq, NULL);	

	kfree(my_context);
}

#ifdef CONFIG_PROC_FS
int read_proc_int(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int length;

/*	int j = jiffies + HZ * 5;

//	set_current_state (TASK_INTERRUPTIBLE);
//	schedule_timeout (5*HZ);
//	while (time_before(jiffies,j))
//		schedule();
//		cpu_relax();*/
	length = sprintf(buf, "%i\n", * (int *) data);

	*eof = 1;
	return length;
}
/*
int write_proc_do_bug(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char procfs_buffer[PROC_DO_BUG_MAX_SIZE];
	size_t procfs_buffer_size = count;
	
	if (procfs_buffer_size > PROC_DO_BUG_MAX_SIZE ) 
		procfs_buffer_size = PROC_DO_BUG_MAX_SIZE;

	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
	if ((procfs_buffer_size == strlen(procfs_buffer)) && ! strncmp(procfs_buffer, PROC_KEYWORD_CALL_INIT, proc_keyword_size) )
		printk(KERN_ALERT "Have tried to call module_init again\n reult=%d", hello_init());

	do_bug = simple_strtoul (procfs_buffer, NULL, 10);

	if (do_bug) 
		buggy_func();
	return procfs_buffer_size;
}
*/
#endif

module_init(hello_init);
module_exit(hello_exit);

