#arm-linux-gnueabihf-gcc -march=armv7-a -static -c cprog.c -o cprog.o 
#arm-linux-gnueabihf-gcc -march=armv7-a -static -c usys.S -o usys.o 
#arm-linux-gnueabihf-ld cprog.o usys.o -N -e main -Ttext 0 -o cprog

#arm-none-linux-gnueabi-gcc -march=armv7-a -static -c cprog.c -o cprog.o 
#arm-none-linux-gnueabi-gcc -march=armv7-a -static -c usys.S -o usys.o 
#arm-none-linux-gnueabi-ld cprog.o usys.o libgcc.a -N -e main -Ttext 0 -o cprog

arm-none-eabi-gcc -march=armv7-a -static -c cprog.c -o cprog.o 
arm-none-eabi-gcc -march=armv7-a -static -c usys.S -o usys.o 
arm-none-eabi-ld cprog.o usys.o libgcc.a -N -e main -Ttext 0 -o cprog
