#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809
#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <libkern/OSCacheControl.h>
#endif
#include "endian.h"
#include "lzss.h"

#ifdef __arm__
#define NAKED __attribute__((naked))
#else
#define NAKED
#endif


#define XLAT(addr)	(image + addr - IMAGE_START)
#define CALL(func)	((func##_t)(uintptr_t)(image + func##_ADDR))


int rfd;
unsigned char *image, *image2;
void *huge;
void *gethuge(void) { return huge; } /* XXX if it doesn't spill r1-r3, we can branch directly to it */
#define STACK_SIZE (1024 * 1024)
unsigned int *altstack;
unsigned int *getstak(void) { return altstack + STACK_SIZE; } /* XXX it probably won't use r2 at any optimization level */
unsigned char nettoyeur[256];
unsigned int nettoyeur_sz;
char c_assert[(sizeof(long) == 4) ? 1 : -1];

void dumpfile(const char *name);
void check_irq_count(void);


#include "asm.h"


typedef void (*func_t)(void);

typedef void (*cache_stuff_t)(void);
typedef void (*wait_for_event_t)(void);
typedef int (*gpio_pin_state_t)(int);
typedef void (*gpio_set_state_t)(int, int);
typedef uint64_t (*get_timer_us_t)(void);
typedef void (*reset_cpu_t)(void);
typedef int (*readp_t)(void *ih, void *buffer, long long offset, int length);
typedef int (*get_mem_size_t)(void);
typedef int (*putchar_t)(int c);
typedef void (*adjust_stack_t)(void);
typedef void (*adjust_environ_t)(void);

typedef void (*iboot_warmup_t)(void);
typedef void (*iboot_start_t)(void);
typedef void (*panic_t)(const char *message, const char *fmt, ...);
typedef void (*system_init_t)(void);
typedef void *(*task_create_t)(const char *name, int (*func)(void *), void *arg, unsigned stack_size);
typedef void (*task_start_t)(void *task);
typedef void (*task_exit_t)(int code);
typedef int (*printf_t)(const char *format, ...);
typedef void *(*malloc_t)(unsigned int size);
typedef void (*free_t)(void *ptr);
typedef void (*create_envvar_t)(const char *var, const char *val, int persistent);
typedef int (*decompress_lzss_t)(void *dst, int dstsize, void *src, int srcsize);


void __attribute__ ((format(printf, 1, 2)))
eprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void __attribute__ ((format(printf, 1, 2)))
xprintf(const char *fmt, ...)
{
    (void)fmt;
}


void __attribute__((noreturn))
my_wait_for_event(void)
{
    eprintf("%s: wfe loop\n", __FUNCTION__);
    STACKTRACE();
    check_irq_count();
    dumpfile("DUMP_wfe");
    exit(3);
}


void NAKED
my_hugechunk(void)
{
#ifdef __arm__
    __asm volatile (
	"push {r1-r3,lr};"
	"blx _gethuge;"
	"pop {r1-r3,pc};"
    );
#endif /* __arm__ */
}


int
my_gpio_pin_state(int wtf)
{
    xprintf("%s(0x%x)\n", __FUNCTION__, wtf);
    return 0xFF;
}


void
my_gpio_set_state(int wtf, int wth)
{
    xprintf("%s(0x%x, 0x%x)\n", __FUNCTION__, wtf, wth);
}


uint64_t
my_get_timer_us(void)
{
#if 0
    /* https://github.com/winocm/uboot-iphone4/blob/master/arch/arm/cpu/armv7/s5l8930/timer.c */
    uint32_t hi, lo, hh;
    do {
	hi = *(uint32_t *)(0xBF100000 + 0x2004);
        lo = *(uint32_t *)(0xBF100000 + 0x2000);
	hh = *(uint32_t *)(0xBF100000 + 0x2004);
    } while (hi != hh);
    return (hi << 32) | lo; /* usec */
#else
    static uint64_t usec = 0xdeadbeef;
    usec += rand() % 500;
    xprintf("%s() => %lu\n", __FUNCTION__, (long)usec);
    return usec;
#endif
}


void __attribute__((noreturn))
my_reset_cpu(void)
{
    eprintf("%s: going for a loop\n", __FUNCTION__);
    dumpfile("DUMP_loop");
    exit(3);
}


int
my_get_mem_size(void)
{
    xprintf("%s()\n", __FUNCTION__);
    return 0x8000000 * 1 * (64 >> 5);
}


void
my_breakpoint1_helper(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3, unsigned int sp, unsigned int lr)
{
#if defined(__thumb__) || (defined(__arm__) && USE_SIGNAL <= 1)
    register unsigned int r11 __asm("r11"); /* XXX these are unreliable if going through ALTHOOK */
    eprintf("breakpoint1: %s\n", (char *)r11);
    (void)(r0 && r1 && r2 && r3 && sp && lr);
#else  /* !__arm__ */
    eprintf("breakpoint1: r0 = 0x%x, r1 = 0x%x, r2 = 0x%x, r3 = 0x%x, sp = 0x%x, lr = 0x%x\n", r0, r1, r2, r3, sp, lr);
#endif /* !__arm__ */
}


#include "target/iBoot.h"


#if USE_SIGNAL
void
handler(int signum, siginfo_t *info, void *ctx)
{
    ucontext_t *uctx = (ucontext_t *)ctx;
#ifdef __arm__
    _STRUCT_MCONTEXT *mctx = uctx->uc_mcontext;
    _STRUCT_ARM_THREAD_STATE *tctx = &mctx->__ss;
    int i;
    if (dispatch(signum, info->si_addr, tctx) == 0) {
	return;
    }
    dumpfile("DUMP_signal");
    for (i = 0; i < 13; i++) {
	eprintf("r%-2d = 0x%08x%s", i, tctx->__r[i], ((i & 3) == 3) ? "\n" : " ");
    }
    eprintf("sp  = 0x%08x lr  = 0x%08x pc  = 0x%08x\ncpsr = 0x%08x\n", tctx->__sp, tctx->__lr, tctx->__pc, tctx->__cpsr);
if (0) {
    unsigned char **fp = (unsigned char **)tctx->__r[7];
    while (*fp) {
	eprintf("** called from 0x%x\n", (fp[1] - image) & ~1);
	fp = (unsigned char **)*fp;
    }
}
#else  /* !__arm__ */
    (void)(uctx);
#endif /* !__arm__ */
    eprintf("handler(%d, {%d, %p}, %p)\n", signum, info->si_signo, info->si_addr, ctx);
    exit(2);
}
#endif /* USE_SIGNAL */


void
dumpfile(const char *name)
{
    int ofd = creat(name, 0644);
    if (ofd >= 0) {
	write(ofd, image, IMAGE_SIZE + IMAGE_HEAP_SIZE);
	close(ofd);
    }
}


int
main(int argc, char **argv)
{
    int rv;
    int fd;
    struct stat st;

    unsigned i;

#if USE_SIGNAL
    struct sigaction act;
    stack_t ss;
#endif /* USE_SIGNAL */

    srand(666);

    assert(((IMAGE_SIZE + IMAGE_HEAP_SIZE) & 0xFFF) == 0);

    altstack = malloc(STACK_SIZE * sizeof(int));
    assert(altstack);

    fd = open(IMAGE_NAME, O_RDONLY);
    assert(fd >= 0);

    rv = fstat(fd, &st);
    assert(rv == 0);
    assert(st.st_size <= IMAGE_SIZE);

#if 0
    image = valloc(IMAGE_SIZE + IMAGE_HEAP_SIZE);
#else
    image = NULL;
    rv = posix_memalign((void **)&image, 0x100000, IMAGE_SIZE + IMAGE_HEAP_SIZE + IMAGE_STACK_SIZE);
    assert(rv == 0);
#endif
    assert(image);
    memset(XLAT(IMAGE_BSS_START), 0, IMAGE_END - IMAGE_BSS_START); /* XXX cleared by iboot_warmup */

    rv = read(fd, image, st.st_size);
    assert(rv == st.st_size);

    close(fd);

    rfd = open((argc > 1) ? argv[1] : "target/ramdisk.dmg", O_RDONLY);
    assert(rfd >= 0);

    eprintf("relocating to %p\n", image);

    huge = NULL;
    rv = posix_memalign((void **)&huge, 0x1000000, IMAGE_HUGECHUNK);
    assert(rv == 0);

    /* relocate all stored offsets */
    for (i = 0; i < IMAGE_SIZE / 4; i++) {
	uint32_t addr = ((uint32_t *)image)[i];
	if (addr >= IMAGE_START && addr <= IMAGE_END) {
	    ((void **)image)[i] = XLAT(addr);
	}
    }

    patch_image(image);

    /* hook some functions */
    HOOK(cache_stuff, my_cache_stuff);
    HOOK(wait_for_event, my_wait_for_event);
    BKPT(hugechunk, my_hugechunk, 0x1A4);
    HOOK(gpio_pin_state, my_gpio_pin_state);
    HOOK(gpio_set_state, my_gpio_set_state);
    HOOK(get_timer_us, my_get_timer_us);
    HOOK(reset_cpu, my_reset_cpu);
    HOOK(readp, my_readp);
    HOOK(get_mem_size, my_get_mem_size);
    HOOK(putchar, putchar);
    HOOK(adjust_stack, my_adjust_stack);
    HOOK(adjust_environ, my_adjust_environ);

    /* add breakpoints */
    BKPT(breakpoint1, my_breakpoint1, 0x1C8);

    /* debug memalign */
    *(uint32_t *)(image + fuck1_ADDR - 1) = make_bl(0, fuck1_ADDR - 1, 0x1B0);
    *(uint16_t *)(image + 0x1B0) = INSNT_LDR_R_PC(6, 0);
    *(uint16_t *)(image + 0x1B2) = INSNT_BX_R(6);
    *(func_t *)(image + 0x1B4) = fuck1;
    *(uint32_t *)(image + fuck2_ADDR - 1) = make_bl(0, fuck2_ADDR - 1, 0x1B8);
    *(uint16_t *)(image + 0x1B8) = INSNT_LDR_R_PC(4, 0);
    *(uint16_t *)(image + 0x1BA) = INSNT_BX_R(4);
    *(func_t *)(image + 0x1BC) = fuck2;
    *(uint32_t *)(image + fuck3_ADDR - 1) = make_bl(0, fuck3_ADDR - 1, 0x1C0);
    *(uint16_t *)(image + 0x1C0) = INSNT_LDR_R_PC(6, 0);
    *(uint16_t *)(image + 0x1C2) = INSNT_BX_R(6);
    *(func_t *)(image + 0x1C4) = fuck3;

    dumpfile("DUMP");
    rv = mprotect(image, IMAGE_TEXT_END - IMAGE_START, PROT_READ | PROT_EXEC);
    assert(rv == 0);
    /* XXX MAP_JIT? */
#if 0 && defined(__arm__) /* this is dangerous, enable only AFTER everything works ok */
    rv = mprotect(image + IMAGE_TEXT_END - IMAGE_START, IMAGE_END + IMAGE_HEAP_SIZE - IMAGE_TEXT_END, PROT_READ | PROT_WRITE | PROT_EXEC);
    assert(rv == 0);
#elif 1 && defined(__arm__) /* this is even more DANGEROUS, enable only AFTER everything works ok */
    rv = mprotect(image, IMAGE_SIZE + IMAGE_HEAP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    assert(rv == 0);
    image2 = malloc(IMAGE_SIZE);
    assert(image2);
    memcpy(image2, image, IMAGE_SIZE);
#endif

#if USE_SIGNAL
    ss.ss_sp = altstack;
    ss.ss_flags = 0;
    ss.ss_size = STACK_SIZE * sizeof(int);
    rv = sigaltstack(&ss, NULL);
    assert(rv == 0);
    act.sa_sigaction = handler;
    act.sa_flags = SA_ONSTACK | SA_SIGINFO /*| SA_RESTART*/ /*| SA_RESETHAND*/;
    sigemptyset(&act.sa_mask);
    rv = sigaction(SIGILL , &act, NULL);
    rv = sigaction(SIGABRT, &act, NULL);
    rv = sigaction(SIGFPE , &act, NULL);
    rv = sigaction(SIGSEGV, &act, NULL);
    rv = sigaction(SIGBUS , &act, NULL);
    rv = sigaction(SIGTRAP, &act, NULL);
    rv = sigaction(SIGSYS, &act, NULL);
#endif /* USE_SIGNAL */

{
    unsigned char *buf, *end;
    fd = open("target/nettoyeur", O_RDONLY);
    assert(fd >= 0);
    rv = fstat(fd, &st);
    assert(rv == 0);
    buf = malloc(st.st_size);
    assert(buf);
    rv = read(fd, buf, st.st_size);
    assert(rv == st.st_size);
    close(fd);
    patch_nettoyeur(buf);
    end = compress_lzss(nettoyeur, sizeof(nettoyeur), buf, rv);
    nettoyeur_sz = end - nettoyeur;
    assert(nettoyeur_sz <= 230);
    free(buf);
}
if (1) {
    CALL(iboot_warmup)();
} else {
    void *task;
    CALL(system_init)();
    task = CALL(task_create)("main", (int (*)(void *))((uintptr_t)image + main_task_ADDR), NULL, 7168);
    CALL(task_start)(task);
    CALL(task_exit)(0);
}

    eprintf("exiting\n");
    free(huge);
    free(image);
    free(image2);
    free(altstack);
    return 0;
}
