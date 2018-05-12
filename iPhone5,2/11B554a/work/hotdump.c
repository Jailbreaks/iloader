#define _GNU_SOURCE
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>


#define DKIOCGETBLOCKSIZE                     _IOR('d', 24, uint32_t)
#define DKIOCGETBLOCKCOUNT                    _IOR('d', 25, uint64_t)


#define RB_AUTOBOOT	0/* flags for system auto-booting itself */

#define RB_ASKNAME	0x01/* ask for file name to reboot from */
#define RB_SINGLE	0x02/* reboot to single user only */
#define RB_NOSYNC	0x04/* dont sync before reboot */
#define RB_HALT		0x08/* don't reboot, just halt */
#define RB_INITNAME	0x10/* name given for /etc/init */
#define RB_DFLTROOT	0x20/* use compiled-in rootdev */
#define RB_ALTBOOT	0x40/* use /boot.old vs /boot */
#define RB_UNIPROC	0x80/* don't start slaves */
#define RB_SAFEBOOT	0x100/* booting safe */
#define RB_UPSDELAY	0x200   /* Delays restart by 5 minutes */
#define RB_QUICK	0x400/* quick and ungraceful reboot with file system caches flushed*/
#define RB_PANIC	0/* reboot due to panic */
#define RB_BOOT		1/* reboot due to boot() */


void *reboot2(uint64_t flags);


int
main(int argc, char **argv)
{
    long rv;
    int fd;
    FILE *f;
    struct stat st;
    char *buf, *ptr;
    uint32_t size;
    uint64_t count, j;
#ifdef __APPLE__
    const char *path = "/dev/rdisk0s1s1";
#else
    const char *path = "ramdisk.dmg";
#endif
    if (argc < 3) {
	fprintf(stderr, "usage: %s payload backup\n", argv[0]);
	return 1;
    }

    fd = open(path, O_RDWR);
    if (fd < 0) {
	err(1, "open");
    }

#ifdef __APPLE__
    if (geteuid()) {
	errno = EPERM;
	err(1, "perm");
    }

    rv = ioctl(fd, DKIOCGETBLOCKSIZE, &size);
    if (rv) {
	err(1, "ioctl");
    }
    rv = ioctl(fd, DKIOCGETBLOCKCOUNT, &count);
    if (rv) {
	err(1, "ioctl");
    }
#else
    size = 512;
    count = 1024;
#endif

    if (size > 1024 * 1024 || !size || (size & (size - 1))) {
	errx(1, "bad size");
    }

    rv = stat(argv[1], &st);
    if (rv) {
	err(1, "stat");
    }
    j = st.st_size / size;
    if (j * size - st.st_size || j > count) {
	err(1, "calc");
    }
    if (count > j) {
	count = j;
    }

    while (size < 1024 * 1024 && (count & 1) == 0) {
	size <<= 1;
	count >>= 1;
    }

    printf("+++ size = %u, count = %" PRIu64 "\n", size, count);

    buf = malloc(count * size);
    if (!buf) {
	err(1, "memory");
    }

    printf("+++ reading from device\n");

    for (j = 0, ptr = buf; j < count; j++, ptr += size) {
	ssize_t n = read(fd, ptr, size);
	if (n < 0) {
	    err(1, "read");
	}
	if (n - size) {
	    err(1, "READ");
	}
    }
    rv = lseek(fd, 0, SEEK_SET);
    if (rv) {
	err(1, "seek");
    }

    printf("+++ writing backup '%s'\n", argv[2]);

    f = fopen(argv[2], "wb");
    if (!f) {
	err(1, "%s", argv[2]);
    }
    rv = fwrite(buf, 1, ptr - buf, f);
    fclose(f);
    if (buf + rv != ptr) {
	err(1, "back");
    }

    printf("+++ sync 1\n");
    sync();
    sleep(1);
    printf("+++ sync 2\n");
    sync();
    sleep(1);
    printf("+++ sync 3\n");
    sync();
    sleep(1);

    printf("+++ reading payload '%s'\n", argv[1]);

    f = fopen(argv[1], "rb");
    if (!f) {
	err(1, "%s", argv[1]);
    }
    rv = fread(buf, 1, count * size, f);
    fclose(f);
    if (rv - count * size) {
	err(1, "inread");
    }

    printf("+++ writing to device\n");

    for (j = 0, ptr = buf; j < count; j++, ptr += size) {
	ssize_t n = write(fd, ptr, size);
	if (n < 0) {
	    err(1, "write");
	}
	if (n - size) {
	    err(1, "WRITE");
	}
    }

    free(buf);
    close(fd);
#ifdef __APPLE__
    printf("+++ rebooting\n");
    reboot(RB_NOSYNC);
    exit(reboot2(RB_NOSYNC) == NULL ? EXIT_SUCCESS : EXIT_FAILURE);
#endif
    printf("+++ done\n");
    return 0;
}
