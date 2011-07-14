#ifndef __HELLO_H
#define __HELLO_H

struct context
{
	unsigned long timer_counter;
	unsigned long tasklet_counter;
	unsigned long workqueue_counter;
	unsigned long kthread_counter;


	struct task_struct *task;
	spinlock_t timer_lock;
	volatile int should_update_timer;
	struct work_struct work;
	struct tasklet_struct tasklet;
	struct timer_list timer;
	struct semaphore semaphore;
};

#endif //__HELLO_H
