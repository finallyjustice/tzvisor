obj-m += kvm-dev.o

all:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) modules

clean:
	make -C /home/zhang/kvm/tzvisor/linux-kvm-arm/ M=$(PWD) clean
