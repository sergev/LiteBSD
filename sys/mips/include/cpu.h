/*-
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)cpu.h       8.5 (Berkeley) 5/17/95
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <machine/machConst.h>

/*
 * Exported definitions unique to MIPS cpu support.
 */

/*
 * definitions of cpu-dependent requirements
 * referenced in generic code
 */
#define COPY_SIGCODE            /* copy sigcode above user stack in exec */

#define cpu_exec(p)             (p->p_md.md_ss_addr = 0) /* init single step */
#define cpu_wait(p)             /* nothing */
#define cpu_setstack(p, ap)     (p)->p_md.md_regs[SP] = ap
#define cpu_set_init_frame(p, fp) /* nothing */
#define BACKTRACE(p)            /* not implemented */

/*
 * Arguments to hardclock and gatherstats encapsulate the previous
 * machine state in an opaque clockframe.
 */
struct clockframe {
    int pc;     /* program counter at time of interrupt */
    int sr;     /* status register at time of interrupt */
};

#define CLKF_USERMODE(framep)   ((framep)->sr & MACH_Status_UM)
#define CLKF_BASEPRI(framep)    (((framep)->sr & \
            (MACH_Status_IPL_MASK | MACH_Status_IE)) == MACH_Status_IE)
#define CLKF_PC(framep)         ((framep)->pc)
#define CLKF_INTR(framep)       (0)

/*
 * Preempt the current process if in interrupt from user mode,
 * or after the current trap/syscall if in system mode.
 */
#define need_resched()      { want_resched = 1; aston(); }

/*
 * Give a profiling tick to the current process when the user profiling
 * buffer pages are invalid.  On MIPS, request an ast to send us
 * through trap, marking the proc as needing a profiling tick.
 */
#define need_proftick(p)    { (p)->p_flag |= P_OWEUPC; aston(); }

/*
 * Notify the current process (p) that it has a signal pending,
 * process as soon as possible.
 */
#define signotify(p)        aston()

#define aston()             (astpending = 1)

int astpending;             /* need to trap before returning to user mode */
int want_resched;           /* resched() was called */

/*
 * CPU identification, from PRID register.
 */
union cpuprid {
    int     cpuprid;
    struct {
#if BYTE_ORDER == BIG_ENDIAN
        u_int   pad1:16;        /* reserved */
        u_int   cp_imp:8;       /* implementation identifier */
        u_int   cp_majrev:4;    /* major revision identifier */
        u_int   cp_minrev:4;    /* minor revision identifier */
#else
        u_int   cp_minrev:4;    /* minor revision identifier */
        u_int   cp_majrev:4;    /* major revision identifier */
        u_int   cp_imp:8;       /* implementation identifier */
        u_int   pad1:16;        /* reserved */
#endif
    } cpu;
};

/*
 * CTL_MACHDEP definitions.
 */
#define CPU_CONSDEV     1       /* dev_t: console terminal device */
#define CPU_NLIST       2       /* int: address of kernel symbol */
#define CPU_WIFI_SCAN   3       /* int: start scanning for Wi-Fi networks */
#define CPU_MAXID       4       /* number of valid machdep ids */

#define CTL_MACHDEP_NAMES { \
    { 0, 0 }, \
    { "console_device", CTLTYPE_STRUCT }, \
    { "nlist", CTLTYPE_STRUCT }, \
    { "wifi_scan", CTLTYPE_INT }, \
}

#ifdef KERNEL
union   cpuprid cpu;
union   cpuprid fpu;
u_int   machDataCacheSize;
u_int   machInstCacheSize;

struct intrcnt {
    u_int clock;
    u_int softclock;
    u_int softnet;
    u_int uart1;
    u_int uart2;
    u_int uart3;
    u_int uart4;
    u_int uart5;
    u_int uart6;
    u_int ether;
} intrcnt;

struct user;
struct proc;

void dumpconf __P((void));
void configure __P((void));
void mips_flush_icache __P((unsigned addr, unsigned len));
void switch_exit __P((void));
int savectx __P((struct user *));
int copykstack __P((struct user *));
int cpu_singlestep __P((struct proc *));

/*
 * Enable all interrupts.
 * Return previous value of interrupt mask.
 *      Status.IE = 1
 *      Status.IPL = 0
 */
static __inline int
spl0()
{
    int prev = mips_di();               /* read Status and disable interrupts */
    int status = prev | MACH_Status_IE; /* set Status.IE bit */
    mips_clear_bits(status, 10, 9);     /* clear Status.IPL field */
    mtc0_Status(status);                /* write Status: enable all interrupts */
    return prev & (MACH_Status_IPL_MASK | MACH_Status_IE);
}

/*
 * Disable all interrupts.
 * Return previous value of interrupt mask.
 *      Status.IE = 0
 *      Status.IPL = unchanged
 */
static __inline int
splhigh()
{
    int status = mips_di();             /* read Status and disable interrupts */

    return status & (MACH_Status_IPL_MASK | MACH_Status_IE);
}

/*
 * Set interrupt level 1...6.
 * Return previous value of interrupt mask.
 *      Status.IE = unchanged
 *      Status.IPL = 1...6
 */
#define __splN__(n) \
    int status = mips_di();         /* read Status and disable interrupts */ \
    int prev = status; \
    mips_ins(status, n, 10, 9);     /* set Status.IPL field */ \
    mtc0_Status(status);            /* write Status */ \
    return prev & (MACH_Status_IPL_MASK | MACH_Status_IE)

static __inline int spl1() { __splN__(1); }
static __inline int spl2() { __splN__(2); }
static __inline int spl3() { __splN__(3); }
static __inline int spl4() { __splN__(4); }
static __inline int spl5() { __splN__(5); }
static __inline int spl6() { __splN__(6); }

/*
 * Restore saved interrupt mask.
 *      Status.IE = restored
 *      Status.IPL = restored
 */
static __inline void
splx(x)
    int x;
{
    int status = mips_di();             /* read Status and disable interrupts */

    /* Use XOR to save one instruction. */
    status |= MACH_Status_IPL_MASK | MACH_Status_IE;
    x      ^= MACH_Status_IPL_MASK | MACH_Status_IE;
    mtc0_Status(status ^ x);
}

#define splsoftclock()  spl1()          /* low-priority clock processing */
#define splnet()        spl2()          /* network protocol processing */
#define splbio()        spl3()          /* disk controllers */
#define splimp()        spl4()          /* network device controllers */
#define spltty()        spl5()          /* uarts and terminal multiplexers */
#define splclock()      spl6()          /* high-priority clock processing */
#define splstatclock()  splhigh()       /* blocks all interrupt activity */

/*
 * Set/clear software interrupt routines.
 */
static __inline void
setsoftclock()
{
    int status = mips_di();             /* read Status and disable interrupts */
    int cause = mfc0_Cause();           /* read Cause */
    cause |= MACH_Cause_IP0;            /* set Cause.IP0 bit */
    mtc0_Cause(cause);                  /* write Cause */
    mtc0_Status(status);                /* restore Status, re-enable interrrupts */
}

static __inline void
clearsoftclock()
{
    int status = mips_di();             /* read Status and disable interrupts */
    int cause = mfc0_Cause();           /* read Cause */
    cause &= ~MACH_Cause_IP0;           /* clear Cause.IP0 bit */
    mtc0_Cause(cause);                  /* write Cause */
    mtc0_Status(status);                /* restore Status, re-enable interrrupts */
}

static __inline void
setsoftnet()
{
    int status = mips_di();             /* read Status and disable interrupts */
    int cause = mfc0_Cause();           /* read Cause */
    cause |= MACH_Cause_IP1;            /* set Cause.IP1 bit */
    mtc0_Cause(cause);                  /* write Cause */
    mtc0_Status(status);                /* restore Status, re-enable interrrupts */
}

static __inline void
clearsoftnet()
{
    int status = mips_di();             /* read Status and disable interrupts */
    int cause = mfc0_Cause();           /* read Cause */
    cause &= ~MACH_Cause_IP1;           /* clear Cause.IP1 bit */
    mtc0_Cause(cause);                  /* write Cause */
    mtc0_Status(status);                /* restore Status, re-enable interrrupts */
}

/*
 * Spin loop for a given number of microseconds.
 */
void udelay(unsigned);

#endif

#endif /* _CPU_H_ */
