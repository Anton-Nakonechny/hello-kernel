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

#define	DEFAULT_RECIPIENT "World"
#define	DEFAULT_RECIPIENT_STRLEN 5

#ifdef CONFIG_PROC_FS

#define PROC_DIR_NAME "hello"
#define PROC_PROBLEM_MSG "You are having problem with proc."
#define PROC_TASKLET_NAME "tasklet"
#define PROC_IRQ_NAME "irq"

char * name = "bazinga";

int read_proc_int(char *buf, char **start, off_t offset, int count, int *eof, void *data);
int write_proc_do_bug(struct file *file, const char *buffer, unsigned long count, void *data);

static struct proc_dir_entry 
							 *proc_tasklet_entry = NULL, 
							 *proc_irq_entry = NULL,
							 *proc_root_dir = NULL;

#endif

static void my_precious_tasklet_handler(unsigned long data);

static char *recipient = DEFAULT_RECIPIENT;
static int irq_num = 0;

module_param (irq_num, int, S_IRUGO);

static int dev_id = MY_MAGIC_ID;
static struct context * my_context = NULL;

#define proc_bye(label) do \
	{\
		printk(KERN_ERR PROC_PROBLEM_MSG);\
		goto label;\
	} while(0)

irqreturn_t hard_handler(int irq, void *dev_id)
{
	if(*((int *)dev_id) == MY_MAGIC_ID)
	{	
		tasklet_schedule(&(my_context->tasklet));
		return IRQ_HANDLED;
	}
	else
		return IRQ_NONE;
}


static void my_precious_tasklet_handler(unsigned long data)
{
	if (printk_ratelimit())
		printk(KERN_NOTICE "You can't hide yourself! Counter is %lu\n",++(my_context->irq_counter));
}

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

#endif

	// check if module param provided during insmod. Use MY_IRQ_LINE as default
	irq_num = irq_num ? irq_num : MY_IRQ_LINE;
	
	tasklet_init(&(my_context->tasklet), my_precious_tasklet_handler, 0x0);
	
	if (register_result = request_irq(irq_num, hard_handler, IRQF_SHARED, name, &dev_id)) 
	{
		printk(KERN_ERR "Couldn't register ! res = %i EBUSY=%i", register_result, EBUSY);
		my_context->registered = 0;
	}
	else
		my_context->registered = 1;

	return 0;

#ifdef CONFIG_PROC_FS
err_remove_proc_irq:
	remove_proc_entry(PROC_IRQ_NAME, proc_root_dir);
err_remove_proc_tasklet:
	remove_proc_entry(PROC_TASKLET_NAME, proc_root_dir);
err_remove_proc_dir:
	remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
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
	
	if (proc_root_dir)
		remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
	
	tasklet_kill (&(my_context->tasklet));
	
	printk(KERN_ALERT "Goodbye, cruel %s!\n", recipient);
	if(my_context->registered)
		free_irq (irq_num, &dev_id);	

	kfree(my_context);
}

#ifdef CONFIG_PROC_FS
int read_proc_int(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int length;

	length = sprintf(buf, "%i\n", * (int *) data);

	*eof = 1;
	return length;
}
#endif

module_init(hello_init);
module_exit(hello_exit);

//SYSCALL_DEFINE0(mysyscall);
