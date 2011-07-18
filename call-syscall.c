#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <linux/kernel.h>
#include <stdlib.h>

#define __NR_mysyscall 341

//__syscall1(long, mysyscall, int, i);

long mysyscall(int i)
{
	return syscall(__NR_mysyscall, i);
};

int main(void)
{
	printf("15+10 = %i", mysyscall(15));
}
