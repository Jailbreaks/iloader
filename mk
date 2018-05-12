#!/bin/sh

#C_THUMB="                    "
C_THUMB="-march=armv7 -mthumb"

gcc -o iloader -Wall -W -pedantic -O2 -DUSE_SIGNAL=2 $C_THUMB -Wno-long-long iloader.c lzss.c && ldid -Sent.xml iloader
