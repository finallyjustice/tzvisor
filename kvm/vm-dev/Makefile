obj-m += vm-dev.o

all:
	make -C /home/zhang/kvm/linux-kvm-arm/ M=$(PWD) modules
	arm-none-linux-gnueabi-gcc user.c -o user

clean:
	make -C /home/zhang/kvm/linux-kvm-arm/ M=$(PWD) clean
	rm -rf user
