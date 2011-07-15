#ifndef __HELLO_H
#define __HELLO_H

struct context
{
	unsigned long irq_counter;
	unsigned long tasklet_counter;

	struct tasklet_struct tasklet;
	char registered;
};

#endif //__HELLO_H
