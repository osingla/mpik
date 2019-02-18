obj-m += mpik.o

# Current dir
PWD  := $(shell pwd)

# Kernel sources
RASPBERRY_DIR ?= /home/osingla/rpi-kernel/linux
KDIR := ${RASPBERRY_DIR}

EXTRA_CFLAGS = -Wno-declaration-after-statement -std=gnu99
ccflags-y  += $(EXTRA_CFLAGS)
KBUILD_NOPEDANTIC=1

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
	mv mpik.ko ../target

clean:
	make -C $(KDIR) M=$(PWD) clean
