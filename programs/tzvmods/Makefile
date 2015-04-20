obj-m := vm-dev.o kvm-dev.o
vm-dev-objs += vm_dev_main.o
kvm-dev-objs += kvm_dev_main.o

all:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) modules
	arm-none-linux-gnueabi-gcc user.c -o user

clean:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) clean
	rm -rf user
