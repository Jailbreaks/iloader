#define INSN2_BX_R12__NOP		0xBF004760
#define INSN2_LDR_R1_PC__B_PLUS4	0xE0014900
#define INSN2_MOV_R0_0__STR_R0_R3	0x60182000
#define INSN2_MOV_R0_1__MOV_R0_1	0x20012001
#define INSN2_NOP__NOP			0xBF00BF00
#define INSN2_RETURN_0			0x47702000
#define INSN2_RETURN_21			0x47702021
#define INSNT_BLX_R(d)			(INSNT_BX_R(d) | 0x80)
#define INSNT_BX_R(d)			(0x4700 | (((d) & 15) << 3))
#define INSNT_B_PC4			0xE002
#define INSNT_LDR_R_PC(d, n)		(0x4800 | (((d) & 7) << 8) | ((n) / 4))
#define INSNT_MOV_R_I(d, i)		(0x2000 | (((d) & 7) << 8) | ((i) & 0xFF))
#define INSNT_MOV_R_R(d, s)		(0x4600 | (((s) & 15) << 3) | ((d) & 7) | (((d) & 8) << 4))
#define INSNT_NOP			0xBF00
#define INSNT_POP_PC			0xBD00
#define INSNT_POP_R0_PC			0xBD01
#define INSNT_PUSH_R0			0xB401
#define INSNT_PUSH_R0_R1		0xB403
#define INSNT_RETURN			0x4770
#define INSNT_STR_R0_SP4		0x9001
#define INSNT_STR_R1_R0_68		0x6441
#define INSNT_STR_R1_R4_R0		0x5021
#define INSNW_LDR_R12_PC4		0xC004F8DF
#define INSNW_LDR_SP_PC72		0xD048F8DF
#define INSNW_LDR_SP_PC80		0xD050F8DF
#define INSNW_MOV_R1_2400		0x5110F44F
#define INSNW_MOV_R1_40000000		0x4180F04F
#define INSNW_MOV_R1_80000000		0x4100F04F
#define INSNW_STRH_R1_R4_E2C		0x1E2CF8A4
#define INSNW_STRH_R1_R4_E54		0x1E54F8A4

#define INSNA_BX_R0			0xE12FFF10
#define INSNA_LDR_PC_NEXT		0xE51FF004
#define INSNA_MOV_R1_0			0xE3A01000
#define INSNA_MOV_R2_0			0xE3A02000
#define INSNA_MOV_R2_40000000		0xE3A02101
#define INSNA_NOP			0xE1A00000
#define INSNA_RETURN			0xE12FFF1E

#define INSNT_ILLEGAL			0xdef0
#define INSNA_ILLEGAL			0xe7ffdef0

#define ILLNO SIGILL


#if 1 /* use as little stack as possible */
#define BCALL(fun) \
    "push {r0-r3};" \
    "push {lr};" \
    "blx _getstak;" \
    "pop {r2};" \
    "mov r1, sp;" \
    "adds r1, #16;" \
    "mov sp, r0;" \
    "push {r1,r2};" \
    "subs r1, #16;" \
    "ldmia r1, {r0-r3};" \
    "blx _" #fun ";" \
    "pop {r0, lr};" \
    "subs r0, #16;" \
    "mov sp, r0;" \
    "pop {r0-r3};"
#else
#define BCALL(fun) \
    "push {r0-r3};" \
    "mov r0, sp;" \
    "adds r0, #16;" \
    "push {r0,lr};" \
    "ldr r0, [sp,#8];" \
    "blx _" #fun ";" \
    "pop {r0,lr};" \
    "pop {r0-r3};"
#endif


#if USE_SIGNAL > 1

#define HOOK(what, func) \
do { \
    if (what##_ADDR & 1) *(uint16_t *)(image + what##_ADDR - 1) = INSNT_ILLEGAL; \
    else                 *(uint32_t *)(image + what##_ADDR) = INSNA_ILLEGAL; \
} while (0)

#define BKPT(what, func, where) HOOK(what, func)

#define STACKTRACE()

#else  /* USE_SIGNAL <= 1 */

#define HOOK(what, func) \
do { \
    if (what##_ADDR & 1) { \
        ((uint32_t *)(image + what##_ADDR - 1))[0] = INSNW_LDR_R12_PC4; \
        ((uint32_t *)(image + what##_ADDR - 1))[1] = INSN2_BX_R12__NOP; \
        ((what##_t *)(image + what##_ADDR - 1))[2] = func; \
    } else { \
        ((uint32_t *)(image + what##_ADDR))[0] = INSNA_LDR_PC_NEXT; \
        ((what##_t *)(image + what##_ADDR))[1] = func; \
    } \
} while (0)

#define BKPT(what, func, where) \
do { \
    ((uint16_t *)(image + where))[0] = INSNT_PUSH_R0_R1; \
    ((uint16_t *)(image + where))[1] = INSNT_LDR_R_PC(0, 4); \
    ((uint16_t *)(image + where))[2] = INSNT_STR_R0_SP4; \
    ((uint16_t *)(image + where))[3] = INSNT_POP_R0_PC; \
    *(void (**)(void))(image + where + 8) = func; \
    *(uint32_t *)(image + (what##_ADDR & ~1)) = make_bl((what##_ADDR & 1) ^ 1, what##_ADDR & ~1, where); \
} while (0)

#define STACKTRACE() \
do { \
    unsigned char **fp = __builtin_frame_address(0); \
    assert(fp[1] == __builtin_return_address(0)); \
    while (*fp) { \
	eprintf("++ called from 0x%x\n", (fp[1] - image) & ~1); \
	fp = (unsigned char **)*fp; \
    } \
} while (0)

#endif /* USE_SIGNAL <= 1 */


unsigned int
make_bl(int blx, int pos, int tgt)
{
    int delta;
    unsigned short pfx;
    unsigned short sfx;

    unsigned int omask = 0xF800;
    unsigned int amask = 0x7FF;

    if (blx) { /* XXX untested */
        omask = 0xE800;
        amask = 0x7FE;
        pos &= ~3;
    }

    delta = tgt - pos - 4; /* range: 0x400000 */
    pfx = 0xF000 | ((delta >> 12) & 0x7FF);
    sfx =  omask | ((delta >>  1) & amask);

    return (unsigned int)pfx | ((unsigned int)sfx << 16);
}
