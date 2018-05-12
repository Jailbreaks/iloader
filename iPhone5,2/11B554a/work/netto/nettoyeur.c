#include <stdio.h>


#define TARGET_JUMPADDR 0x84000000


int
main(void)
{
    unsigned i;

    *(unsigned int *)(TARGET_JUMPADDR + 0x442ac) = 0xffffffff;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442cc) = 0xffffffff;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44380) = 0xffffffff;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44384) = 0xffffffff;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44574) = 0xffffffff;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44660) = 0xffffffff;

    *(unsigned int *)(TARGET_JUMPADDR + 0x442c8) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442d0) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442d4) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442e0) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442e4) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442ec) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442f4) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x442fc) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44300) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44304) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44308) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x4430c) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44310) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44314) = 0x00000000;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44318) = 0x00000000;

    for (i = 0; i <= 0x44530 - 0x443dc; i += 4) {
        *(unsigned int *)(TARGET_JUMPADDR + 0x443dc + i) = 0;
    }

    *(unsigned char *)(TARGET_JUMPADDR + 0x441b0) = 0x00;
    *(unsigned char *)(TARGET_JUMPADDR + 0x441c0) = 0x00;
    *(unsigned char *)(TARGET_JUMPADDR + 0x441d4) = 0x00;
    *(unsigned char *)(TARGET_JUMPADDR + 0x4420c) = 0x01;

    *(unsigned int *)(TARGET_JUMPADDR + 0x44330) = 0x00000001; /* not needed? */
    *(unsigned int *)(TARGET_JUMPADDR + 0x443d4) = 0x00000002;

    *(unsigned int *)(TARGET_JUMPADDR + 0x44334) = 0xbff44334;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44338) = 0xbff44334;
    *(unsigned int *)(TARGET_JUMPADDR + 0x4436c) = 0xbff4436c;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44370) = 0xbff4436c;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44374) = 0xbff44374;
    *(unsigned int *)(TARGET_JUMPADDR + 0x44378) = 0xbff44374;
    *(unsigned int *)(TARGET_JUMPADDR + 0x443a0) = 0xbff443c0;
    *(unsigned int *)(TARGET_JUMPADDR + 0x443b0) = 0xbff443b0;
    *(unsigned int *)(TARGET_JUMPADDR + 0x443b4) = 0xbff443b0;
    *(unsigned int *)(TARGET_JUMPADDR + 0x443b8) = 0xbff443b8;
    *(unsigned int *)(TARGET_JUMPADDR + 0x443bc) = 0xbff443b8;
    *(unsigned int *)(TARGET_JUMPADDR + 0x445f4) = 0xbff445f4;
    *(unsigned int *)(TARGET_JUMPADDR + 0x445f8) = 0xbff445f4;
    return 0;
}
