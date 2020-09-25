*************************
*** WORK IN PROGRESS  ***
*** UNDER DEVELOPMENT ***
*************************

/home/osingla/rpi-kernel/linux
CROSS_COMPILE=arm-linux-gnu-

---

/home/osingla/rpi-kernel/kern-module
export CROSS_COMPILE=/opt/armv7/bin/arm-buildroot-linux-gnueabihf-

===

ssh pi@192.168.1.3
loggerma "*****************" ; insmod mpik.ko ; chmod a+rw /dev/mpik ; chmod 0777 /dev/mpik
rmmod mpik.ko

---

ssh pi@192.168.1.3
LD_LIBRARY_PATH=/home/pi/test ./example_1 


========

https://nullprogram.com/blog/2016/09/03/

process_vm_readv
https://man7.org/linux/man-pages/man2/process_vm_readv.2.html

process_vm_writev
https://man7.org/linux/man-pages/man2/process_vm_writev.2.html
