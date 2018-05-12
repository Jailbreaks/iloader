#!/bin/sh

/opt/gnuarm/bin/arm-none-eabi-gcc -c -Os nettoyeur.c
/opt/gnuarm/bin/arm-none-eabi-objcopy -O binary nettoyeur.o nettoyeur.bin
./lzss_main nettoyeur.bin nettoyeur.cmp
