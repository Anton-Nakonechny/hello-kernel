#ifndef __HELLO_H
#define __HELLO_H

struct context
{
	unsigned long irq_counter;
	unsigned long tasklet_counter;

	struct tasklet_struct tasklet;
	char registered;
};

#define MY_IRQ_LINE 17
//#define MY_MAGIC_ID 12345

#endif //__HELLO_H
