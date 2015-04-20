# dd if=uImage of=zImage skip=64 bs=1

#LOADADDR=0x80008000 CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm make uImage
LOADADDR=0x80008000 CROSS_COMPILE=arm-none-eabi- ARCH=arm make uImage
