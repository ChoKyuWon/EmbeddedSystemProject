obj-m += gpio_kernelmodule.o
DIR = $(shell pwd)
#KERNEL_DIR = /lib/modules/$(shell uname -r)/build
KERNEL_DIR = ./linux-4.9.248
all:
	make -C $(KERNEL_DIR) M=$(DIR) modules
clean:
	make -C $(KERNEL_DIR) M=$(DIR) clean
