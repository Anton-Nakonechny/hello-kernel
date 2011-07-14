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
#include "hello.h"

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
#define PROC_KTHREAD_NAME "kthread"
#define PROC_RECIPIENT_NAME "recipient"
#define PROC_RECIPIENT_MAX_SIZE 20
#define PROC_DO_BUG_MAX_SIZE 8
#define PROC_KEYWORD_CALL_INIT "init"

int read_proc_int(char *buf, char **start, off_t offset, int count, int *eof, void *data);
int write_proc_do_bug(struct file *file, const char *buffer, unsigned long count, void *data);

static struct proc_dir_entry *proc_do_bug_entry = NULL, 
							 *proc_tasklet_entry = NULL, 
							 *proc_workqueue_entry = NULL,
							 *proc_kthread_entry = NULL,
							 *proc_root_dir = NULL;

static unsigned char proc_keyword_size;
#endif

static void my_precious_timer_handler(unsigned long);
static void my_precious_tasklet_handler(unsigned long data);
static void my_precious_workqueue_handler(struct work_struct *work);
static int my_precious_kthread(void * data);

static char *recipient = DEFAULT_RECIPIENT;
//static char reipient_strlen = DEFAULT_RECIPIENT_STRLEN;
static int do_bug = 0;
static int (*buggy_func)(void) = NULL;
static int timeout = MY_TICK;
//DECLARE_WORK(my_precious_workqueue, my_precious_workqueue_handler);
//static unsigned long irq_flags;
module_param (recipient, charp, S_IRUGO);
module_param (do_bug, int, S_IRUGO);

static struct context * my_context = NULL;

#define proc_bye(label) do \
	{\
		printk(KERN_ERR PROC_PROBLEM_MSG);\
		goto label;\
	} while(0)

int try_arm_timer(struct timer_list * timer_ptr, 
		spinlock_t * timer_lock, 
		char * name, 
		unsigned long * counter,
		int try_sema)
{
	int atempted = 0;
	
	if(try_sema)
	{
		down_interruptible(&(my_context->semaphore));
	}

	spin_lock_bh(timer_lock);
	if (atempted = ! time_after(timer_ptr->expires, jiffies))
	{
		mod_timer(timer_ptr, jiffies + HZ * timeout);
		(*counter)++;
	}
	spin_unlock_bh(timer_lock);

	if(try_sema)
	{
		up(&(my_context->semaphore));
	}

	if(atempted)
		printk(KERN_INFO "Expires at %lu; jiffies %lu ; %s is winner\n", timer_ptr->expires, jiffies, name);
	
	return atempted;
}

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
/*schedule_timeout_interruptible (HZ / 2);*/
		schedule();
	}
	do_exit(0);
}

static void my_precious_tasklet_handler(unsigned long data)
{
	try_arm_timer(&(my_context->timer),&(my_context->timer_lock),"tasklet", &(my_context->tasklet_counter), 0);
}

static void my_precious_timer_handler(unsigned long data)
{
	printk(KERN_ALERT "Hello-timer expired! Counter is %lu\n", (my_context->timer_counter)++);
//	tasklet_schedule(&(my_context->tasklet));
	my_context->should_update_timer = 1;
	mb();
	wake_up_process(my_context->task);
	schedule_work(&(my_context->work));
}

static int hello_init(void)
{
	if (!( my_context = kmalloc(sizeof(struct context), GFP_KERNEL)))
	{
		printk(KERN_ALERT "Couldn't allocate memmory in hello driver.");
		goto err;
	};

	memset(my_context, 0, sizeof(struct context));
	my_context->timer_lock = SPIN_LOCK_UNLOCKED;
	sema_init(&(my_context->semaphore), 1);

#ifdef CONFIG_PROC_FS
	if (!(proc_root_dir = proc_mkdir(PROC_DIR_NAME, NULL)))
		proc_bye(err_remove_proc_dir);	
	
	if (!(proc_do_bug_entry = create_proc_entry(PROC_DO_BUG_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_do_bug);
	proc_do_bug_entry->data = &do_bug;
	proc_do_bug_entry->read_proc = read_proc_int;
	proc_do_bug_entry->write_proc = write_proc_do_bug;
	
	if (!(proc_tasklet_entry = create_proc_entry(PROC_TASKLET_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_tasklet);
	proc_tasklet_entry->data = &(my_context->tasklet_counter);
	proc_tasklet_entry->read_proc = read_proc_int;
	
	if (!(proc_workqueue_entry = create_proc_entry(PROC_WORKQUEUE_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_workqueue);
	proc_workqueue_entry->data = &(my_context->workqueue_counter);
	proc_workqueue_entry->read_proc = read_proc_int;

	if (!(proc_kthread_entry = create_proc_entry(PROC_KTHREAD_NAME, 0666, proc_root_dir)))
		proc_bye(err_remove_proc_kthread);
	proc_kthread_entry->data = &(my_context->kthread_counter);
	proc_kthread_entry->read_proc = read_proc_int;

	proc_keyword_size = strlen (PROC_KEYWORD_CALL_INIT);

#endif
	
	init_timer(&(my_context->timer));
	
	INIT_WORK (&(my_context->work),my_precious_workqueue_handler);
	
	my_context->task = kthread_create(my_precious_kthread, NULL, PROC_KTHREAD_NAME); 
	
	tasklet_init(&(my_context->tasklet), my_precious_tasklet_handler, 0x0);

	(my_context->timer).data = 0x0;
	(my_context->timer).function = my_precious_timer_handler;
	mod_timer(&(my_context->timer), jiffies + HZ * MY_TICK);
	
	printk(KERN_INFO "Hello, %s!\n", recipient);
	return 0;

#ifdef CONFIG_PROC_FS
err_remove_proc_kthread:
	remove_proc_entry(PROC_KTHREAD_NAME, proc_root_dir);
err_remove_proc_workqueue:
	remove_proc_entry(PROC_WORKQUEUE_NAME, proc_root_dir);
err_remove_proc_tasklet:
	remove_proc_entry(PROC_TASKLET_NAME, proc_root_dir);
err_remove_proc_do_bug:
	remove_proc_entry(PROC_DO_BUG_NAME, proc_root_dir);
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
	
	if (proc_kthread_entry)
		remove_proc_entry(PROC_KTHREAD_NAME, proc_root_dir);
	
	if (proc_workqueue_entry)
		remove_proc_entry(PROC_WORKQUEUE_NAME, proc_root_dir);
	
	if (proc_do_bug_entry)
		remove_proc_entry(PROC_DO_BUG_NAME, proc_root_dir);
	
	if (proc_root_dir)
		remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
	
	cancel_work_sync(&(my_context->work));


	tasklet_kill (&(my_context->tasklet));
	
	del_timer_sync (&(my_context->timer));
	if(my_context->task)
		kthread_stop (my_context->task);
	printk(KERN_ALERT "Goodbye, cruel %s!\n", recipient);
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

int write_proc_do_bug(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char procfs_buffer[PROC_DO_BUG_MAX_SIZE];
	size_t procfs_buffer_size = count;
	
	if (procfs_buffer_size > PROC_DO_BUG_MAX_SIZE ) 
		procfs_buffer_size = PROC_DO_BUG_MAX_SIZE;

	/* write data to the buffer */
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
#endif

module_init(hello_init);
module_exit(hello_exit);

