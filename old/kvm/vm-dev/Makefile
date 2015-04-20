obj-m := vm-dev.o
vm-dev-objs += main.o

all:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) modules
	arm-none-linux-gnueabi-gcc user.c -o user

clean:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) clean
	rm -rf user
