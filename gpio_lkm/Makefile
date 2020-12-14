BUILDROOT_OUTPUT=XXX/buildroot/output
KERNEL_DIR=$(BUILDROOT_OUTPUT)/build/linux-custom/
CROSS_COMPILE=$(BUILDROOT_OUTPUT)/host/bin/arm-linuxobj-m+=key.o

all: key app

hello:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=`pwd` modules
clean:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=`pwd` clean

app: app.c
	$(CROSS_COMPILE)gcc app.c -o app
