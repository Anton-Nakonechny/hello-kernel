#ifndef __HELLO_H
#define __HELLO_H

struct context
{
	unsigned long timer_counter = 0;
	unsigned long tasklet_counter = 0;
	unsigned long workqueue_counter = 0;
	unsigned long kthread_counter = 0;


	struct task_struct *task = NULL;
	spinlock_t timer_lock;
	volatile int should_update_timer = 0;
	struct work_struct work;
	struct tasklet_struct tasklet;
	struct timer_list timer;
}

#endif //__HELLO_H
