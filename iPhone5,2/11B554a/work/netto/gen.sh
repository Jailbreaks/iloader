#!/bin/sh

function do_netto() {
cat <<EOF
#include <stdio.h>


#define TARGET_JUMPADDR 0x84000000


int
main(void)
{
EOF

sed 's|^0xbff|0x|;s|: | |;s| != | |' $1 | awk '{ print "    *(unsigned int *)(TARGET_JUMPADDR + " $1 ") = " $2 ";" }'

cat <<EOF
    return 0;
}
EOF
}

do_netto $1 > nettoyeur.c
/opt/gnuarm/bin/arm-none-eabi-gcc -c -Os nettoyeur.c
/opt/gnuarm/bin/arm-none-eabi-objcopy -O binary nettoyeur.o nettoyeur.bin
./lzss_main nettoyeur.bin nettoyeur.cmp
