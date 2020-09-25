
#export ARCH ?= arm
#export CROSS_COMPILE ?= /opt/armv7/bin/arm-buildroot-linux-gnueabihf-

export ARCH ?= x86
#export CROSS_COMPILE ?= /opt/armv7/bin/arm-buildroot-linux-gnueabihf-

all:
	make -C kernel
	make -C userspace
	make -C examples
	#scp target/* pi@192.168.1.3:/home/pi/test

clean:
	@rm -f target/*
	@make -C kernel clean
	@make -C userspace clean
	@make -C examples clean
