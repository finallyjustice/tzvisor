rmmod vm-dev
rm /dev/tzvisor-vm
insmod vm-dev.ko
mknod /dev/tzvisor-vm c 50 0
