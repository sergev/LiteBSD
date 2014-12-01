/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2014 Serge Vakulenko
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department, The Mach Operating System project at
 * Carnegie-Mellon University and Ralph Campbell.
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
 *      @(#)machdep.c   8.5 (Berkeley) 6/2/95
 */

/* from: Utah $Hdr: machdep.c 1.63 91/04/24$ */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/callout.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>
#include <sys/user.h>
#include <sys/exec.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>
#ifdef SYSVSHM
#include <sys/shm.h>
#endif

#include <vm/vm_kern.h>

#include <machine/cpu.h>
#include <machine/reg.h>
#include <machine/psl.h>
#include <machine/pte.h>
#include <machine/pic32mz.h>

/* the following is used externally (sysctl_hw) */
char    machine[] = "MIPS";     /* cpu "architecture" */
char    cpu_model[30];

vm_map_t buffer_map;

/*
 * Declare these as initialized data so we can patch them.
 */
int     nswbuf = 0;

#ifndef NBUF
#define NBUF 0
#endif
int     nbuf = NBUF;

#ifndef BUFPAGES
#define BUFPAGES 0
#endif
int     bufpages = BUFPAGES;

int     msgbufmapped = 0;       /* set when safe to use msgbuf */
int     maxmem;                 /* max memory per process */
int     physmem;                /* max supported memory, in pages */

/*
 * safepri is a safe priority for sleep to set for a spin-wait
 * during autoconfiguration or after a panic.
 */
int     safepri = PSL_LOWIPL;

struct  user *proc0paddr;
struct  proc nullproc;          /* for use by swtch_exit() */

int     dumpmag = (int)0x8fca0101;      /* magic number for savecore */
int     dumpsize = 0;                   /* also for savecore */
long    dumplo = 0;

/*
 * Check whether button 1 is pressed.
 */
static inline int
button1_pressed()
{
#ifdef BUTTON1_PORT
    if ((BUTTON1_PORT >> BUTTON1_PIN) & 1)
        return 1;
#endif
    return 0;
}

/*
 * Do all the stuff that locore normally does before calling main().
 * Return the first page address following the system.
 */
void
mach_init()
{
    int i;
    unsigned firstaddr;
    caddr_t v, start;
    extern char __data_start[], _edata[], _end[];
    extern void _etext(), _tlb_vector();

    /* Initialize STATUS register: master interrupt disable.
     * Setup interrupt vector base. */
    mtc0_Status(MACH_Status_CU0 | MACH_Status_BEV);
    mtc0_EBase(_tlb_vector);
    mtc0_Status(MACH_Status_CU0);

    /* Set vector spacing: not used really, but must be nonzero. */
    mtc0_IntCtl(32);

    /* Clear CAUSE register: use special interrupt vector 0x200. */
    mtc0_Cause(MACH_Cause_IV);

    /* Copy .data image from flash to RAM.
     * Linker places it at the end of .text segment. */
    bcopy(_etext, __data_start, _edata - __data_start);

    /* Clear .bss segment. */
    v = (caddr_t)mips_round_page(_end);
    bzero(_edata, v - _edata);

    /*
     * Autoboot by default.
     */
    boothowto = 0;
    if (button1_pressed()) {
        /* Boot to single user mode. */
        boothowto |= RB_SINGLE;
    }

    /*
     * Init mapping for u page(s) for proc[0], pm_tlbpid 1.
     */
    start = v;
    proc0paddr = (struct user *)v;
    curproc->p_addr = proc0paddr;
    curproc->p_md.md_regs = proc0paddr->u_pcb.pcb_regs;
    firstaddr = MACH_CACHED_TO_PHYS(v);
    for (i = 0; i < UPAGES; i++) {
        curproc->p_md.md_upte[i] = PG_PFNUM(firstaddr) | PG_UNCACHED | PG_V | PG_D;
        firstaddr += NBPG;
    }
    tlb_write_wired(0, UADDR | 1, curproc->p_md.md_upte[0],
                                  curproc->p_md.md_upte[1]);
    v += UPAGES * NBPG;
    tlb_set_pid(1);

    /*
     * init nullproc for swtch_exit().
     * init mapping for u page(s), pm_tlbpid 0
     * This could be used for an idle process.
     */
    nullproc.p_addr = (struct user *)v;
    nullproc.p_md.md_regs = nullproc.p_addr->u_pcb.pcb_regs;
    bcopy("nullproc", nullproc.p_comm, sizeof("nullproc"));
    for (i = 0; i < UPAGES; i++) {
        nullproc.p_md.md_upte[i] = PG_PFNUM(firstaddr) | PG_UNCACHED | PG_V | PG_D;
        firstaddr += NBPG;
    }
    v += UPAGES * NBPG;

    /* clear pages for u areas */
    bzero(start, v - start);

    switch (DEVID & 0x0fffffff) {
    case 0x05104053: strcpy(cpu_model, "PIC32MZ2048ECG064"); break;
    case 0x0510E053: strcpy(cpu_model, "PIC32MZ2048ECG100"); break;
    case 0x05118053: strcpy(cpu_model, "PIC32MZ2048ECG124"); break;
    case 0x05122053: strcpy(cpu_model, "PIC32MZ2048ECG144"); break;
    case 0x05109053: strcpy(cpu_model, "PIC32MZ2048ECH064"); break;
    case 0x05113053: strcpy(cpu_model, "PIC32MZ2048ECH100"); break;
    case 0x0511D053: strcpy(cpu_model, "PIC32MZ2048ECH124"); break;
    case 0x05127053: strcpy(cpu_model, "PIC32MZ2048ECH144"); break;
    case 0x05131053: strcpy(cpu_model, "PIC32MZ2048ECM064"); break;
    case 0x0513B053: strcpy(cpu_model, "PIC32MZ2048ECM100"); break;
    case 0x05145053: strcpy(cpu_model, "PIC32MZ2048ECM124"); break;
    case 0x0514F053: strcpy(cpu_model, "PIC32MZ2048ECM144"); break;
    default:         sprintf(cpu_model, "PIC32MZ DevID %08x", DEVID);
    }

    /*
     * Find out how much memory is available.
     */
    physmem = btoc(512 * 1024);
    maxmem = physmem;

    /*
     * Initialize error message buffer (at end of core).
     */
    maxmem -= btoc(sizeof (struct msgbuf));
    msgbufp = (struct msgbuf *)(MACH_PHYS_TO_UNCACHED(maxmem << PGSHIFT));
    msgbufmapped = 1;

    /*
     * Allocate space for system data structures.
     * The first available kernel virtual address is in "v".
     * As pages of kernel virtual memory are allocated, "v" is incremented.
     *
     * These data structures are allocated here instead of cpu_startup()
     * because physical memory is directly addressable. We don't have
     * to map these into virtual address space.
     */
    start = v;

#define valloc(name, type, num) \
        (name) = (type *)v; v = (caddr_t)((name)+(num))
#define valloclim(name, type, num, lim) \
        (name) = (type *)v; v = (caddr_t)((lim) = ((name)+(num)))
    valloc(callout, struct callout, ncallout);
    valloc(swapmap, struct map, nswapmap = maxproc * 2);
#ifdef SYSVSHM
    valloc(shmsegs, struct shmid_ds, shminfo.shmmni);
#endif

    /*
     * Determine how many buffers to allocate.
     * We allocate more buffer space than the BSD standard of
     * using 10% of memory for the first 2 Meg, 5% of remaining.
     * We just allocate a flat 10%.  Ensure a minimum of 16 buffers.
     * We allocate 1/2 as many swap buffer headers as file i/o buffers.
     */
    if (bufpages == 0)
        bufpages = physmem / 10 / CLSIZE;
    if (nbuf == 0)
        nbuf = 16;
    if (bufpages < nbuf)
        bufpages = nbuf;
    if (nswbuf == 0) {
        nswbuf = (nbuf / 2) &~ 1;       /* force even */
        if (nswbuf > 256)
            nswbuf = 256;               /* sanity */
    }
    valloc(swbuf, struct buf, nswbuf);
    valloc(buf, struct buf, nbuf);

    /*
     * Clear allocated memory.
     */
    bzero(start, v - start);

    /*
     * Initialize the virtual memory system.
     */
    pmap_bootstrap((vm_offset_t)v);

    /*
     * Setup interrupt controller.
     *
     * Priority IRQ                     Devices
     * --------------------------------------------------------------
     *  1       Software interrupt 0    low-priority clock processing
     *  2       Software interrupt 1    network protocol processing
     *  3       SPI ports               disk controllers
     *  4       Ethernet                network device controllers
     *  5       UART ports              terminal multiplexers
     *  6       Core timer              high-priority clock processing
     */
    INTCON = 0;				/* Interrupt Control */
    IPTMR = 0;				/* Temporal Proximity Timer */

    /* Interrupt Flag Status */
    IFS(0) = 0;
    IFS(1) = 0;
    IFS(2) = 0;
    IFS(3) = 0;
    IFS(4) = 0;
    IFS(5) = 0;

    /* Interrupt Enable Control */
    IEC(0) = 0;
    IEC(1) = 0;
    IEC(2) = 0;
    IEC(3) = 0;
    IEC(4) = 0;
    IEC(5) = 0;

    /* Interrupt Priority Control */
#define LSPI    3       /* level for SPI interrupts */
#define LETH    4       /* level for Ethernet interrupts */
#define LUART   5       /* level for UART interrupts */
#define LTMR    6       /* level for timer interrupts */

    /* 0 - Core Timer Interrupt
     * 1 - Core Software Interrupt 0
     * 2 - Core Software Interrupt 1
     * 3 - External Interrupt 0 */
    IPC(0) = PIC32_IPC_IP0(LTMR) | PIC32_IPC_IP1(1) | PIC32_IPC_IP2(2) | PIC32_IPC_IP3(0);
    IPC(1) = 0;
    IPC(2) = 0;
    IPC(3) = 0;
    IPC(4) = 0;
    IPC(5) = 0;
    IPC(6) = 0;
    IPC(7) = 0;
    IPC(8) = 0;
    IPC(9) = 0;
    IPC(10) = 0;
    IPC(11) = 0;
    IPC(12) = 0;
    IPC(13) = 0;
    IPC(14) = 0;
    IPC(15) = 0;
    IPC(16) = 0;
    IPC(17) = 0;
    IPC(18) = 0;
    IPC(19) = 0;
    IPC(20) = 0;
    IPC(21) = 0;
    IPC(22) = 0;
    IPC(23) = 0;
    IPC(24) = 0;
    IPC(25) = 0;
    IPC(26) = 0;

    /* 108 - Reserved
     * 109 - SPI1 Fault
     * 110 - SPI1 Receive Done
     * 111 - SPI1 Transfer Done */
    IPC(27) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(LSPI) | PIC32_IPC_IP2(LSPI) | PIC32_IPC_IP3(LSPI);

    /* 112 - UART1 Fault
     * 113 - UART1 Receive Done
     * 114 - UART1 Transfer Done
     * 115 - I2C1 Bus Collision Event */
    IPC(28) = PIC32_IPC_IP0(LUART) | PIC32_IPC_IP1(LUART) | PIC32_IPC_IP2(LUART) | PIC32_IPC_IP3(0);
    IPC(29) = 0;
    IPC(30) = 0;
    IPC(31) = 0;
    IPC(32) = 0;
    IPC(33) = 0;
    IPC(34) = 0;

    /* 140 - DMA Channel 6
     * 141 - DMA Channel 7
     * 142 - SPI2 Fault
     * 143 - SPI2 Receive Done */
    IPC(35) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(0) | PIC32_IPC_IP2(LSPI) | PIC32_IPC_IP3(LSPI);

    /* 144 - SPI2 Transfer Done
     * 145 - UART2 Fault
     * 146 - UART2 Receive Done
     * 147 - UART2 Transfer Done */
    IPC(36) = PIC32_IPC_IP0(LSPI) | PIC32_IPC_IP1(LUART) | PIC32_IPC_IP2(LUART) | PIC32_IPC_IP3(LUART);

    IPC(37) = 0;

    /* 152 - Control Area Network 2
     * 153 - Ethernet Interrupt
     * 154 - SPI3 Fault
     * 155 - SPI3 Receive Done */
    IPC(38) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(LETH) | PIC32_IPC_IP2(LSPI) | PIC32_IPC_IP3(LSPI);

    /* 156 - SPI3 Transfer Done
     * 157 - UART3 Fault
     * 158 - UART3 Receive Done
     * 159 - UART3 Transfer Done */
    IPC(39) = PIC32_IPC_IP0(LSPI) | PIC32_IPC_IP1(LUART) | PIC32_IPC_IP2(LUART) | PIC32_IPC_IP3(LUART);

    /* 160 - I2C3 Bus Collision Event
     * 161 - I2C3 Slave Event
     * 162 - I2C3 Master Event
     * 163 - SPI4 Fault */
    IPC(40) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(0) | PIC32_IPC_IP2(0) | PIC32_IPC_IP3(LSPI);

    /* 164 - SPI4 Receive Done
     * 165 - SPI4 Transfer Done
     * 166 - Real Time Clock
     * 167 - Flash Control Event */
    IPC(41) = PIC32_IPC_IP0(LSPI) | PIC32_IPC_IP1(LSPI) | PIC32_IPC_IP2(LTMR) | PIC32_IPC_IP3(0);

    /* 168 - Prefetch Module SEC Event
     * 169 - SQI1 Event
     * 170 - UART4 Fault
     * 171 - UART4 Receive Done */
    IPC(42) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(0) | PIC32_IPC_IP2(LUART) | PIC32_IPC_IP3(LUART);

    /* 172 - UART4 Transfer Done
     * 173 - I2C4 Bus Collision Event
     * 174 - I2C4 Slave Event
     * 175 - I2C4 Master Event */
    IPC(43) = PIC32_IPC_IP0(LUART) | PIC32_IPC_IP1(0) | PIC32_IPC_IP2(0) | PIC32_IPC_IP3(0);

    /* 176 - SPI5 Fault
     * 177 - SPI5 Receive Done
     * 178 - SPI5 Transfer Done
     * 179 - UART5 Fault */
    IPC(44) = PIC32_IPC_IP0(LSPI) | PIC32_IPC_IP1(LSPI) | PIC32_IPC_IP2(LSPI) | PIC32_IPC_IP3(LUART);

    /* 180 - UART5 Receive Done
     * 181 - UART5 Transfer Done
     * 182 - I2C5 Bus Collision Event
     * 183 - I2C5 Slave Event */
    IPC(45) = PIC32_IPC_IP0(LUART) | PIC32_IPC_IP1(LUART) | PIC32_IPC_IP2(0) | PIC32_IPC_IP3(0);

    /* 184 - I2C5 Master Event
     * 185 - SPI6 Fault
     * 186 - SPI6 Receive Done
     * 187 - SPI6 Transfer Done */
    IPC(46) = PIC32_IPC_IP0(0) | PIC32_IPC_IP1(LSPI) | PIC32_IPC_IP2(LSPI) | PIC32_IPC_IP3(LSPI);

    /* 188 - UART6 Fault
     * 189 - UART6 Receive Done
     * 190 - UART6 Transfer Done
     * 191 - Reserved */
    IPC(47) = PIC32_IPC_IP0(LUART) | PIC32_IPC_IP1(LUART) | PIC32_IPC_IP2(LUART) | PIC32_IPC_IP3(0);

    /* Read processor ID register. */
    cpu.cpuprid = mfc0_PRId();
}

void
initcpu()
{
    /* disable clock interrupts (until startrtclock()) */
#if 0
    // TODO: pic32
    register volatile struct chiptime *c = Mach_clock_addr;
    c->regb = REGB_DATA_MODE | REGB_HOURS_FORMAT;
    (void) c->regc;
#endif

    /* Enable software interrupts. */
    IECSET(0) = 1 << PIC32_IRQ_CS0;     /* softclock */
    IECSET(0) = 1 << PIC32_IRQ_CS1;     /* softnet */

    /* Safe to turn interrupts on now. */
    spl0();
}

static void
identify_cpu()
{
    unsigned osccon = OSCCON, spllcon = SPLLCON;
    unsigned pllmult = (spllcon >> 16 & 127) + 1;
    unsigned pllidiv = (spllcon >> 8 & 7) + 1;
    unsigned pllodiv = "\2\2\4\10\20\40\40\40"[spllcon >> 24 & 7];
    static const char *poscmod[] = { "external clock", "(reserved)",
                                     "crystal", "(disabled)" };

    printf("cpu: %s rev A%u, %u MHz\n",
        cpu_model, DEVID >> 28, CPU_KHZ/1000);

    /* COSC: current oscillator selection bits */
    printf("oscillator: ");
    switch (osccon >> 12 & 7) {
    case 0:
    case 7:
        printf("internal Fast RC, divided\n");
        break;
    case 1:
        printf("system PLL div 1:%d mult x%d\n",
                pllidiv * pllodiv, pllmult);
        break;
    case 2:
        printf("%s\n", poscmod [DEVCFG1 >> 8 & 3]);
        break;
    case 3:
        printf("reserved\n");
        break;
    case 4:
        printf("secondary\n");
        break;
    case 5:
        printf("internal Low-Power RC\n");
        break;
    case 6:
        printf("back-up Fast RC\n");
        break;
    }

    /*
     * Compute cache sizes.
     */
    unsigned config1 = mfc0_Config1();
    unsigned il = config1 >> 19 & 7;
    unsigned dl = config1 >> 10 & 7;

    machInstCacheSize = 0;
    machDataCacheSize = 0;
    if (il > 0) {
        unsigned is = config1 >> 22 & 7;
        unsigned ia = config1 >> 16 & 7;
        machInstCacheSize = 64 * (1<<is) * 2 * (1<<il) * (ia+1);
    }
    if (dl > 0) {
        unsigned ds = config1 >> 13 & 7;
        unsigned da = config1 >> 7 & 7;
        machDataCacheSize = 64 * (1<<ds) * 2 * (1<<dl) * (da+1);
    }
    if (machInstCacheSize > 0 || machDataCacheSize > 0) {
        printf("cache: %u/%u kbytes\n",
            machInstCacheSize >> 10, machDataCacheSize >> 10);
    }
}

/*
 * cpu_startup: allocate memory for variable-sized tables,
 * initialize cpu, and do autoconfiguration.
 */
void
cpu_startup()
{
    register unsigned i;
    int base, residual;
    vm_offset_t minaddr, maxaddr;
    vm_size_t size;

    /*
     * Good {morning,afternoon,evening,night}.
     */
    printf(version);
    identify_cpu();
    printf("real mem = %d kbytes\n", ctob(physmem) >> 10);

    /*
     * Allocate virtual address space for file I/O buffers.
     * Note they are different than the array of headers, 'buf',
     * and usually occupy more virtual memory than physical.
     */
    size = MAXBSIZE * nbuf;
    buffer_map = kmem_suballoc(kernel_map, (vm_offset_t *)&buffers,
                   &maxaddr, size, TRUE);
    minaddr = (vm_offset_t)buffers;
    if (vm_map_find(buffer_map, vm_object_allocate(size), (vm_offset_t)0,
            &minaddr, size, FALSE) != KERN_SUCCESS)
        panic("startup: cannot allocate buffers");
    base = bufpages / nbuf;
    residual = bufpages % nbuf;
    for (i = 0; i < nbuf; i++) {
        vm_size_t curbufsize;
        vm_offset_t curbuf;

        /*
         * First <residual> buffers get (base+1) physical pages
         * allocated for them.  The rest get (base) physical pages.
         *
         * The rest of each buffer occupies virtual space,
         * but has no physical memory allocated for it.
         */
        curbuf = (vm_offset_t)buffers + i * MAXBSIZE;
        curbufsize = CLBYTES * (i < residual ? base+1 : base);
        vm_map_pageable(buffer_map, curbuf, curbuf+curbufsize, FALSE);
        vm_map_simplify(buffer_map, curbuf);
    }
    /*
     * Allocate a submap for exec arguments.  This map effectively
     * limits the number of processes exec'ing at any time.
     */
    exec_map = kmem_suballoc(kernel_map, &minaddr, &maxaddr,
                 16 * NCARGS, TRUE);
    /*
     * Allocate a submap for physio
     */
    phys_map = kmem_suballoc(kernel_map, &minaddr, &maxaddr,
                 VM_PHYS_SIZE, TRUE);

    /*
     * Finally, allocate mbuf pool.  Since mclrefcnt is an off-size
     * we use the more space efficient malloc in place of kmem_alloc.
     */
    mclrefcnt = (char *)malloc(NMBCLUSTERS+CLBYTES/MCLBYTES,
                   M_MBUF, M_NOWAIT);
    bzero(mclrefcnt, NMBCLUSTERS+CLBYTES/MCLBYTES);
    mb_map = kmem_suballoc(kernel_map, (vm_offset_t *)&mbutl, &maxaddr,
                   VM_MBUF_SIZE, FALSE);
    /*
     * Initialize callouts
     */
    callfree = callout;
    for (i = 1; i < ncallout; i++)
        callout[i-1].c_next = &callout[i];
    callout[i-1].c_next = NULL;

    printf("avail mem = %d kbytes\n", ptoa(cnt.v_free_count) >> 10);
    printf("using %d buffers containing %d bytes of memory\n",
        nbuf, bufpages * CLBYTES);
    /*
     * Set up CPU-specific registers, cache, etc.
     */
    initcpu();

    /*
     * Set up buffers, so they can be used to read disk labels.
     */
    bufinit();

    /*
     * Configure the system.
     */
    configure();
}

/*
 * Set registers on exec.
 * Clear all registers except sp, pc.
 */
void
setregs(p, entry)
    register struct proc *p;
    u_long entry;
{
    int sp = p->p_md.md_regs[SP];

    bzero((caddr_t)p->p_md.md_regs, (FSR + 1) * sizeof(int));
    p->p_md.md_regs[SP] = sp;
    p->p_md.md_regs[PC] = entry & ~3;
    p->p_md.md_regs[SR] = PSL_USERSET | MACH_Status_EXL;
    p->p_md.md_flags &= ~MDP_FPUSED;
}

/*
 * WARNING: code in locore.s assumes the layout shown for sf_signum
 * thru sf_handler so... don't screw with them!
 */
struct sigframe {
    int     sf_signum;              /* signo for handler */
    int     sf_code;                /* additional info for handler */
    struct  sigcontext *sf_scp;     /* context ptr for handler */
    sig_t   sf_handler;             /* handler addr for u_sigc */
    struct  sigcontext sf_sc;       /* actual context */
};

/*
 * Send an interrupt to process.
 */
void
sendsig(catcher, sig, mask, code)
    sig_t catcher;
    int sig, mask;
    u_long code;
{
    register struct proc *p = curproc;
    register struct sigframe *fp;
    register int *regs;
    register struct sigacts *psp = p->p_sigacts;
    int oonstack, fsize;
    struct sigcontext ksc;
    extern char sigcode[], esigcode[];

    regs = p->p_md.md_regs;
    oonstack = psp->ps_sigstk.ss_flags & SA_ONSTACK;
    /*
     * Allocate and validate space for the signal handler
     * context. Note that if the stack is in data space, the
     * call to grow() is a nop, and the copyout()
     * will fail if the process has not already allocated
     * the space with a `brk'.
     */
    fsize = sizeof(struct sigframe);
    if ((psp->ps_flags & SAS_ALTSTACK) &&
        (psp->ps_sigstk.ss_flags & SA_ONSTACK) == 0 &&
        (psp->ps_sigonstack & sigmask(sig))) {
        fp = (struct sigframe *)(psp->ps_sigstk.ss_base +
            psp->ps_sigstk.ss_size - fsize);
        psp->ps_sigstk.ss_flags |= SA_ONSTACK;
    } else
        fp = (struct sigframe *)(regs[SP] - fsize);
    if ((unsigned)fp <= USRSTACK - ctob(p->p_vmspace->vm_ssize))
        (void)grow(p, (unsigned)fp);

    /*
     * Build the signal context to be used by sigreturn.
     */
    ksc.sc_onstack = oonstack;
    ksc.sc_mask = mask;
    ksc.sc_pc = regs[PC];
    ksc.sc_regs[ZERO] = 0xACEDBADE;         /* magic number */
    bcopy((caddr_t)&regs[1], (caddr_t)&ksc.sc_regs[1],
        sizeof(ksc.sc_regs) - sizeof(int));
    ksc.sc_fpused = p->p_md.md_flags & MDP_FPUSED;
    if (copyout((caddr_t)&ksc, (caddr_t)&fp->sf_sc, sizeof(ksc))) {
        /*
         * Process has trashed its stack; give it an illegal
         * instruction to halt it in its tracks.
         */
        SIGACTION(p, SIGILL) = SIG_DFL;
        sig = sigmask(SIGILL);
        p->p_sigignore &= ~sig;
        p->p_sigcatch &= ~sig;
        p->p_sigmask &= ~sig;
        psignal(p, SIGILL);
        return;
    }

    /*
     * Build the argument list for the signal handler.
     */
    regs[A0] = sig;
    regs[A1] = code;
    regs[A2] = (int)&fp->sf_sc;
    regs[A3] = (int)catcher;
    regs[PC] = (int)catcher;
    regs[SP] = (int)fp;

    /*
     * Signal trampoline code is at base of user stack.
     */
    regs[RA] = (int)PS_STRINGS - (esigcode - sigcode);
}

/*
 * System call to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 * Return to previous pc and psl as specified by
 * context left by sendsig. Check carefully to
 * make sure that the user has not modified the
 * psl to gain improper priviledges or to cause
 * a machine fault.
 */
/* ARGSUSED */
int
sigreturn(p, uap, retval)
    struct proc *p;
    struct sigreturn_args /* {
        syscallarg(struct sigcontext *) sigcntxp;
    } */ *uap;
    register_t *retval;
{
    register struct sigcontext *scp;
    register int *regs;
    struct sigcontext ksc;
    int error;

    scp = SCARG(uap, sigcntxp);
    regs = p->p_md.md_regs;
    /*
     * Test and fetch the context structure.
     * We grab it all at once for speed.
     */
    error = copyin((caddr_t)scp, (caddr_t)&ksc, sizeof(ksc));
    if (error || ksc.sc_regs[ZERO] != 0xACEDBADE) {
        return (EINVAL);
    }
    scp = &ksc;
    /*
     * Restore the user supplied information
     */
    if (scp->sc_onstack & 01)
        p->p_sigacts->ps_sigstk.ss_flags |= SA_ONSTACK;
    else
        p->p_sigacts->ps_sigstk.ss_flags &= ~SA_ONSTACK;
    p->p_sigmask = scp->sc_mask &~ sigcantmask;
    regs[PC] = scp->sc_pc;
    bcopy((caddr_t)&scp->sc_regs[1], (caddr_t)&regs[1],
        sizeof(scp->sc_regs) - sizeof(int));
    if (scp->sc_fpused)
        bcopy((caddr_t)scp->sc_fpregs, (caddr_t)&p->p_md.md_regs[F0],
            sizeof(scp->sc_fpregs));
    return (EJUSTRETURN);
}

/*
 * Boot comes here after turning off memory management and
 * getting on the dump stack, either when called above, or by
 * the auto-restart code.
 */
static void
dumpsys()
{
    int error;

    msgbufmapped = 0;
    if (dumpdev == NODEV)
        return;
    /*
     * For dumps during autoconfiguration,
     * if dump device has already configured...
     */
    if (dumpsize == 0)
        dumpconf();
    if (dumplo < 0)
        return;
    printf("\ndumping to dev %x, offset %d\n", dumpdev, dumplo);
    printf("dump ");
    error = (*bdevsw[major(dumpdev)].d_dump)(dumpdev);
    switch (error) {

    case ENXIO:
        printf("device bad\n");
        break;

    case EFAULT:
        printf("device not ready\n");
        break;

    case EINVAL:
        printf("area improper\n");
        break;

    case EIO:
        printf("i/o error\n");
        break;

    default:
        printf("error %d\n", error);
        break;

    case 0:
        printf("succeeded\n");
    }
}

int     waittime = -1;

void
boot(howto)
    register int howto;
{
    struct proc *p = curproc;       /* XXX */

    /* take a snap shot before clobbering any registers */
    if (curproc)
        savectx(curproc->p_addr);

    boothowto = howto;
    if ((howto & RB_NOSYNC) == 0 && waittime < 0) {
        register struct buf *bp;
        int iter, nbusy;

        waittime = 0;
        (void) spl0();
        printf("syncing disks... ");
        /*
         * Release vnodes held by texts before sync.
         */
        if (panicstr == 0)
            vnode_pager_umount(NULL);
        sync(p, (void *)NULL, (int *)NULL);

        /*
         * Unmount filesystems
         */
        if (panicstr == 0)
            vfs_unmountall();

        for (iter = 0; iter < 20; iter++) {
            nbusy = 0;
            for (bp = &buf[nbuf]; --bp >= buf; )
                if ((bp->b_flags & (B_BUSY|B_INVAL)) == B_BUSY)
                    nbusy++;
            if (nbusy == 0)
                break;
            printf("%d ", nbusy);
            udelay(40000 * iter);
        }
        if (nbusy)
            printf("giving up\n");
        else
            printf("done\n");
        /*
         * If we've been adjusting the clock, the todr
         * will be out of synch; adjust it now.
         */
        resettodr();
    }

    /* Disable interrupts. */
    mips_di();

    if (! (howto & RB_HALT)) {
        if (howto & RB_DUMP)
            dumpsys();

        /* Unlock access to reset register */
        SYSKEY = 0;
        SYSKEY = 0xaa996655;
        SYSKEY = 0x556699aa;

        /* Reset microcontroller */
        RSWRSTSET = 1;
        (void) RSWRST;
    }
    printf ("halted\n");

    for (;;) {
        asm volatile ("wait");
    }
    /*NOTREACHED*/
}

void
dumpconf()
{
    int nblks;

    dumpsize = physmem;
    if (dumpdev != NODEV && bdevsw[major(dumpdev)].d_psize) {
        nblks = (*bdevsw[major(dumpdev)].d_psize)(dumpdev);
        if (dumpsize > btoc(dbtob(nblks - dumplo)))
            dumpsize = btoc(dbtob(nblks - dumplo));
        else if (dumplo == 0)
            dumplo = nblks - btodb(ctob(physmem));
    }

    /*
     * Don't dump on the first CLBYTES (why CLBYTES?)
     * in case the dump device includes a disk label.
     */
    if (dumplo < btodb(CLBYTES))
        dumplo = btodb(CLBYTES);
}

/*
 * Return the best possible estimate of the time in the timeval
 * to which tvp points.  Unfortunately, we can't read the hardware registers.
 * We guarantee that the time will be greater than the value obtained by a
 * previous call.
 */
void
microtime(tvp)
    struct timeval *tvp;
{
    int s = splclock();
    static struct timeval lasttime;

    *tvp = time;
#ifdef notdef
    tvp->tv_usec += clkread();
    while (tvp->tv_usec > 1000000) {
        tvp->tv_sec++;
        tvp->tv_usec -= 1000000;
    }
#endif
    if (tvp->tv_sec == lasttime.tv_sec &&
        tvp->tv_usec <= lasttime.tv_usec &&
        (tvp->tv_usec = lasttime.tv_usec + 1) > 1000000) {
        tvp->tv_sec++;
        tvp->tv_usec -= 1000000;
    }
    lasttime = *tvp;
    splx(s);
}

/*
 * Delay for a given number of microseconds.
 * The processor has a 32-bit hardware Count register,
 * which increments at half CPU rate.
 * We use it to get a precise delay.
 */
void
udelay(unsigned usec)
{
    unsigned now = mfc0_Count();
    unsigned final = now + usec * CPU_KHZ / 2000;

    do {
        now = mfc0_Count();

        /* This comparison is valid only when using a signed type. */
    } while ((int) (now - final) < 0);
}

/*
 * Write the given pid into the TLB pid reg.
 */
void
tlb_set_pid(unsigned pid)
{
    mtc0_EntryHi(pid);                  /* Set up entry high */
    asm volatile ("ehb");               /* Hazard barrier */
}

/*
 * Write the entry into the wired part of TLB at the given index.
 */
void
tlb_write_wired(unsigned index, unsigned hi, unsigned lo0, unsigned lo1)
{
    int x = mips_di();                  /* Disable interrupts */
    int pid = mfc0_EntryHi();           /* Save the current PID */

    mtc0_Index(index);                  /* Set the index */
    mtc0_EntryHi(hi);                   /* Set up entry high */
    mtc0_EntryLo0(lo0);                 /* Set up entry low 0 */
    mtc0_EntryLo1(lo1);                 /* Set up entry low 1 */
    asm volatile ("tlbwi");             /* Write the TLB entry */

    mtc0_Wired(index + 1);              /* Set the number of wired entries */

    mtc0_EntryHi(pid);                  /* Restore the PID */
    mtc0_Status(x);                     /* Restore interrupts */
}

/*
 * Flush the "random" entries from the TLB.
 */
void
tlb_flush()
{
    int x = mips_di();                  /* Disable interrupts */
    int pid = mfc0_EntryHi();           /* Save the current PID */
    int index = VMMACH_NUM_TLB_ENTRIES;
    int addr = MACH_CACHED_MEMORY_ADDR;

    mtc0_EntryLo0(0);                   /* Zero out low entry 0 */
    mtc0_EntryLo1(0);                   /* Zero out low entry 1 */
    while (--index >= VMMACH_FIRST_RAND_ENTRY) {
        mtc0_Index(index);              /* Set the index */
        mtc0_EntryHi(addr);             /* Use invalid memory address */
        asm volatile ("tlbwi");         /* Write the TLB entry */
        addr += 2 << 13;                /* Increment address to avoid MCheck */
    }
    mtc0_EntryHi(pid);                  /* Restore the PID */
    mtc0_Status(x);                     /* Restore interrupts */
}

/*
 * Flush any TLB entries for the given address and TLB PID.
 */
void
tlb_flush_addr(unsigned hi, unsigned lo)
{
    int x, pid, index;

//printf("%s: hi=%08x, lo=%08x\n", __func__, hi, lo);
    x = mips_di();                      /* Disable interrupts */
    pid = mfc0_EntryHi();               /* Save the current PID */

    mtc0_EntryHi(hi);                   /* Look for addr & PID */
    asm volatile ("tlbp");              /* Probe for the entry */
    index = mfc0_Index();               /* See what we got */

    if (index >= 0) {
        /* Entry found. */
        asm volatile ("tlbr");          /* Read existing entry */
        mtc0_EntryHi(hi);               /* Restore addr & PID */
        if (hi & (1 << PGSHIFT)) {
            mtc0_EntryLo1(lo);          /* Clear low entry 1 */
        } else {
            mtc0_EntryLo0(lo);          /* Clear low entry 0 */
        }
        asm volatile ("tlbwi");         /* Update TLB entry */
    }
    mtc0_EntryHi(pid);                  /* Restore the PID */
    mtc0_Status(x);                     /* Restore interrupts */
}

/*
 * Update the TLB entry if highreg is found; otherwise, enter the data.
 */
void
tlb_update (unsigned hi, pt_entry_t *pte)
{
    unsigned lo0, lo1;
    int x, pid, index;

    /* Replicate G bit to paired even/odd entry. */
    if (hi & (1 << PGSHIFT)) {
        lo1 = pte[0].pt_entry;
        lo0 = pte[-1].pt_entry | (lo1 & PG_G);
    } else {
        lo0 = pte[0].pt_entry;
        lo1 = pte[1].pt_entry | (lo0 & PG_G);
    }
//printf("%s: hi=%08x, lo0=%08x, lo1=%08x\n", __func__, hi, lo0, lo1);
    x = mips_di();                      /* Disable interrupts */
    pid = mfc0_EntryHi();               /* Save the current PID */

    mtc0_EntryHi(hi);                   /* Look for addr & PID */
    asm volatile ("tlbp");              /* Probe for the entry */
    index = mfc0_Index();               /* See what we got */

    mtc0_EntryLo0(lo0);                 /* Setup low entry 0 */
    mtc0_EntryLo1(lo1);                 /* Setup low entry 1 */

    if (index >= 0) {
        /* Entry found. */
        asm volatile ("tlbwi");         /* Overwrite existing TLB entry */
    } else {
        asm volatile ("tlbwr");         /* Enter into a random slot */
    }
    mtc0_EntryHi(pid);                  /* Restore the PID */
    mtc0_Status(x);                     /* Restore interrupts */
}

/*
 * Flush instruction cache for range of addr to addr + len - 1.
 * The address can be any valid address so long as no TLB misses occur.
 */
void
mips_flush_icache(vm_offset_t addr, vm_offset_t len)
{
    int x = mips_di();                  /* Disable interrupts */

    // TODO
    mtc0_Status(x);                     /* Restore interrupts */
}

/*
 * Flush data cache for range of addr to addr + len - 1.
 * The address can be any valid address so long as no TLB misses occur.
 * (Be sure to use cached K0SEG kernel addresses)
 */
void
mips_flush_dcache(vm_offset_t addr, vm_offset_t len)
{
    int x = mips_di();                  /* Disable interrupts */

    // TODO
    mtc0_Status(x);                     /* Restore interrupts */
}
