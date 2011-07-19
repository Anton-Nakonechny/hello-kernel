# If KERNELRELEASE is defined, we've been invoked from the
# # kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
		obj-m := hello.o hello-keyboard.o
# # Otherwise we were called directly from the command
# # line; invoke the kernel build system.
else
		KERNELDIR ?= /lib/modules/$(shell uname -r)/build
		PWD := $(shell pwd)
default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
		rm -f call-syscall
		$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

call-syscall: % : %.c
		gcc $< -o $@
endif

