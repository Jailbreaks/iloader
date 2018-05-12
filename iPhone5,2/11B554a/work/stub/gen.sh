#!/bin/sh

/opt/gnuarm/bin/arm-none-eabi-gcc -c iboot_p1.S
/opt/gnuarm/bin/arm-none-eabi-objdump -D iboot_p1.o
