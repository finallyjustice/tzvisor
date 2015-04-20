# arm-none-eabi-gcc -march=armv7-a -static -c cprog.c -o cprog.o
# arm-none-eabi-gcc -march=armv7-a -static -c usys.S -o usys.o
# arm-none-eabi-ld cprog.o usys.o libgcc.a -N -e main -Ttext 0 -o cprog

CROSS_COMPILE = arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS  = -march=armv7-a -static -nostdlib -fno-builtin
LDFLAGS = -N -e main -Ttext 0

OBJS = pal.o \
	   usys.o \
	   ulib.o

PAL = pal

all: $(OBJS)
	$(LD) $(OBJS) libgcc.a $(LDFLAGS) -o $(PAL)
	$(OBJDUMP) -S $(PAL) > $(PAL).asm
	$(OBJDUMP) -t $(PAL) | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(PAL).sym

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -rf *.o *.asm *.sym $(PAL)
