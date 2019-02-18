/home/osingla/rpi-kernel/linux
CROSS_COMPILE=arm-linux-gnu-

---

/home/osingla/rpi-kernel/kern-module
export CROSS_COMPILE=/opt/armv7/bin/arm-buildroot-linux-gnueabihf-

===

ssh pi@192.168.1.3
logger "*****************" ; insmod mpik.ko ; chmod a+rw /dev/mpik ; chmod 0777 /dev/mpik
rmmod mpik.ko

---

ssh pi@192.168.1.3
LD_LIBRARY_PATH=/home/pi/test ./example_1 
