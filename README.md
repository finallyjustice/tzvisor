# tzvisor

TZVisor is the project to decouple the Trusted Execution Environment from KVM in IaaS cloud.

Installation Guide:

Please refer to http://www.virtualopensystems.com/en/solutions/guides/kvm-on-arm/ for the installation of KVM/ARM in Fast Models. Our testing environment is Fast Models 9.1.

To compile linux-kvm-arm, run test.sh in ./linux-kvm-arm to get the uImage, which will be used as host image.

Generate kernel image for guest VM with:
    dd if=uImage of=zImage skip=64 bs=1
And copy the zImage to guest VM disk.


Compile STK (Secure Tiny Kernel) with test.sh in ./stk folder.

Compile tzvmods with ./compile

Compile user with make

Run ginstall.sh to copy all necessary files to host and guest disk.

For any question, please contact dongli.zhang0129@gmail.com
