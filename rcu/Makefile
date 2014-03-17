obj-m=tmain.o
tmain-y=main.o

CURRENT_PATH=`pwd`
LINUX_KERNEL=`uname -r`
LINUX_KERNEL_PATH=/usr/src/kernels/$(LINUX_KERNEL)

all:
	make -C $(LINUX_KERNEL_PATH) EXTRA_CFLAGS=-I$(CURRENT_PATH)/include M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean 

