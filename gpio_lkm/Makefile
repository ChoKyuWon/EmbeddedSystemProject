obj-m += gpio_kernelmodule.o
DIR = $(shell pwd)
KERNEL_DIR = /lib/modules/$(shell uname -r)/build

all:
        make -C $(KERNEL_DIR) M=$(DIR) modules
clean:
        make -C $(KERNEL_DIR) M=$(DIR) clean