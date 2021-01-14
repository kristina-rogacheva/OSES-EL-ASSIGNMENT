# OSES-EL-ASSIGNMENT
Heart Rate Monitor - Embedded Linux - Kristina Rogacheva, s280065


In order to integrate this layer in a Yocto-based distribution follow these steps:

1. cd poky
2. source oe-init-build-env build_qemuarm
3. bitbake-layers create-layer ../HeartRateMonitor
4. bitbake-layers add-layer ../HeartRateMonitor

At this point you should add the files of this repo in the created folder. 

In the file local.conf add the following:

5. IMAGE_INSTALL_append = " heartbeat"

   IMAGE_INSTALL_append = " heartbeatmodule"
   
   KERNEL_MODULE_AUTOLOAD += "heartbeatmodule"
   
Now return in the build_qemuarm directory, run

6. bitbake core-image-minimal
7.runqemu qemuarm 

login

8. cat /proc/devices 

look for the major number of "heartbeatmodule_dev". Assume in the following that it is 251:

9. mknod /dev/heartbeatmodule c 251 0

now in the directory /usr/bin you can find the program
