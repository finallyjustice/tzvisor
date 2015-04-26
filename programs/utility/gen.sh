#!/bin/sh

CROSS_COMPILE=arm-none-eabi-
OBJDUMP=${CROSS_COMPILE}objdump
PAL=$1

$OBJDUMP -t ${PAL} | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > ${PAL}.sym
./utility ${PAL}.sym ${PAL}.conf ${PAL}
