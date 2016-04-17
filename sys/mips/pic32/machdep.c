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
#include <machine/pic32_gpio.h>
#include <machine/assym.h>

/* the following is used externally (sysctl_hw) */
char    machine[] = "MIPS";     /* cpu "architecture" */
char    machine_arch[] = "mipsel";     /* used in "uname -p" */
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

#if defined(MEBII) || defined(SNADPIC) || defined(EMZ64)
/*
 * Chip configuration.
 */
PIC32_DEVCFG (
    DEVCFG0_JTAG_DISABLE |      /* Disable JTAG port */
    DEVCFG0_TRC_DISABLE,        /* Disable trace port */

    /* Using primary oscillator with external crystal 24 MHz.
     * PLL multiplies it to 200 MHz. */
    DEVCFG1_FNOSC_SPLL |        /* System clock supplied by SPLL */
    DEVCFG1_POSCMOD_EXT |       /* External generator */
    DEVCFG1_FCKS_ENABLE |       /* Enable clock switching */
    DEVCFG1_FCKM_ENABLE |       /* Enable fail-safe clock monitoring */
    DEVCFG1_IESO |              /* Internal-external switch over enable */
    DEVCFG1_CLKO_DISABLE,       /* CLKO output disable */

    DEVCFG2_FPLLIDIV_3 |        /* PLL input divider = 3 */
    DEVCFG2_FPLLRNG_5_10 |      /* PLL input range is 5-10 MHz */
    DEVCFG2_FPLLMULT(50) |      /* PLL multiplier = 50x */
    DEVCFG2_FPLLODIV_2,         /* PLL postscaler = 1/2 */

    DEVCFG3_FETHIO |            /* Default Ethernet pins */
    DEVCFG3_USERID(0xffff));    /* User-defined ID */
#endif

#if defined(HMZ144)
PIC32_DEVCFG (
    DEVCFG0_JTAG_DISABLE |      /* Disable JTAG port */
    DEVCFG0_TRC_DISABLE,        /* Disable trace port */

    /* Using primary oscillator with crystal 12 MHz.
     * PLL multiplies it to 200 MHz. */
    DEVCFG1_FNOSC_SPLL |        /* use system PLL */
    DEVCFG1_POSCMOD_HS |        /* primary oscillator HS mode */
    DEVCFG1_FSOSCEN |           /* Enable secondary oscillator */
    DEVCFG1_FCKM_ENABLE |       /* Enable fail-safe clock monitoring */
    DEVCFG1_IESO |              /* Internal-external switch over enable */
    DEVCFG1_CLKO_DISABLE,       /* CLKO output disable */

    DEVCFG2_FPLLIDIV_3 |        /* PLL input divider = 2 */
    DEVCFG2_FPLLRNG_5_10 |      /* PLL input range is 5-10 MHz */
    DEVCFG2_FPLLMULT(100) |     /* PLL multiplier = 100x */
    DEVCFG2_FPLLODIV_2,         /* PLL postscaler = 1/3 */

    DEVCFG3_FETHIO |            /* Default Ethernet pins */
    DEVCFG3_USERID(0xffff));    /* User-defined ID */
#endif

#if defined(MEBII) || defined(HMZ144) || defined(SNADPIC) || defined(EMZ64)
/*
 * Boot code at bfc00000.
 * Jump to Flash memory.
 */
asm ("          .section .startup,\"ax\",@progbits");
asm ("          .globl  _boot");
asm ("          .type   _boot, function");
asm ("_boot:    la      $ra, start");
asm ("          jr      $ra");
asm ("          .text");
#endif

/*
 * Check whether button 1 is pressed.
 */
static inline int
button1_pressed()
{
#ifndef BUTTON1_PORT
    return 0;
#else
    int val;

    val = PORT_VAL(BUTTON1_PORT);
#ifdef BUTTON1_INVERT
    val = ~val;
#endif
    return (val >> BUTTON1_PIN) & 1;
#endif
}

/*
 * Do all the stuff that locore normally does before calling main().
 */
void
mach_init()
{
    int i;
    unsigned firstaddr;
    caddr_t v, start;
    const char *model = 0;
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
     * Assign console pins, board specific.
     */
#if defined(MEBII)
    /* Microchip MEB-II board: use UART1 for console.
     * Map signals rx=RA14, tx=RA15 to pins 4,6 at PICtail connector. */
    U1RXR = 13;             /* Group 1: 1101 = RA14 */
    RPA15R = 1;             /* Group 2: 0001 = U1TX */
#endif

#if defined(HMZ144) || defined(SNADPIC)
    /* Olimex HMZ144 board, SnadPIC board: use UART2 for console.
     * Map signals rx=RE9, tx=RE8. */
    ANSELECLR = (1 << 8) |
                (1 << 9);   /* Set digital mode for RE8 and RE9 */
    U2RXR = 13;             /* Group 3: 1101 = RE9 */
    RPE8R = 2;              /* Group 4: 0010 = U2TX */
#endif

#if defined(EMZ64)
    /* Olimex EMZ64 board: use UART4 for console.
     * Map signals rx=RD0, tx=RD4. */
    U4RXR = 3;              /* Group 4: 0011 = RD0 */
    RPD4R = 2;              /* Group 3: 0010 = U4TX */

    /* Enable the Ethernet PHY chip. */
    LATBSET = 1 << 11;      /* set RB11 high for EPHY-RST# */
    TRISBCLR = 1 << 11;     /* set RB11 as output */
#endif

    /*
     * Enable buttons.
     */
#ifdef BUTTON1_PORT
    ANSEL_CLR(BUTTON1_PORT) = 1 << BUTTON1_PIN;
    TRIS_SET(BUTTON1_PORT) = 1 << BUTTON1_PIN;
#ifdef BUTTON1_INVERT
    /* Active low - enable Pull Up resistor. */
    CNPU_SET(BUTTON1_PORT) = 1 << BUTTON1_PIN;
#else
    /* Active high - enable Pull Down resistor. */
    CNPD_SET(BUTTON1_PORT) = 1 << BUTTON1_PIN;
#endif
#endif
#ifdef BUTTON2_PORT
    ANSEL_CLR(BUTTON2_PORT) = 1 << BUTTON2_PIN;
    TRIS_SET(BUTTON2_PORT) = 1 << BUTTON2_PIN;
#ifdef BUTTON2_INVERT
    /* Active low - enable Pull Up resistor. */
    CNPU_SET(BUTTON2_PORT) = 1 << BUTTON2_PIN;
#else
    /* Active high - enable Pull Down resistor. */
    CNPD_SET(BUTTON2_PORT) = 1 << BUTTON2_PIN;
#endif
#endif
#ifdef BUTTON3_PORT
    ANSEL_CLR(BUTTON3_PORT) = 1 << BUTTON3_PIN;
    TRIS_SET(BUTTON3_PORT) = 1 << BUTTON3_PIN;
#ifdef BUTTON3_INVERT
    /* Active low - enable Pull Up resistor. */
    CNPU_SET(BUTTON3_PORT) = 1 << BUTTON3_PIN;
#else
    /* Active high - enable Pull Down resistor. */
    CNPD_SET(BUTTON3_PORT) = 1 << BUTTON3_PIN;
#endif
#endif

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
    firstaddr = MACH_VIRT_TO_PHYS(v);
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
    case 0x05104053: model = "PIC32MZ2048ECG064"; break;
    case 0x0510E053: model = "PIC32MZ2048ECG100"; break;
    case 0x05118053: model = "PIC32MZ2048ECG124"; break;
    case 0x05122053: model = "PIC32MZ2048ECG144"; break;
    case 0x05109053: model = "PIC32MZ2048ECH064"; break;
    case 0x05113053: model = "PIC32MZ2048ECH100"; break;
    case 0x0511D053: model = "PIC32MZ2048ECH124"; break;
    case 0x05127053: model = "PIC32MZ2048ECH144"; break;
    case 0x05131053: model = "PIC32MZ2048ECM064"; break;
    case 0x0513B053: model = "PIC32MZ2048ECM100"; break;
    case 0x05145053: model = "PIC32MZ2048ECM124"; break;
    case 0x0514F053: model = "PIC32MZ2048ECM144"; break;
    case 0x07203053: model = "PIC32MZ1024EFG064"; break;
    case 0x07208053: model = "PIC32MZ1024EFH064"; break;
    case 0x07230053: model = "PIC32MZ1024EFM064"; break;
    case 0x07204053: model = "PIC32MZ2048EFG064"; break;
    case 0x07209053: model = "PIC32MZ2048EFH064"; break;
    case 0x07231053: model = "PIC32MZ2048EFM064"; break;
    case 0x0720D053: model = "PIC32MZ1024EFG100"; break;
    case 0x07212053: model = "PIC32MZ1024EFH100"; break;
    case 0x0723A053: model = "PIC32MZ1024EFM100"; break;
    case 0x0720E053: model = "PIC32MZ2048EFG100"; break;
    case 0x07213053: model = "PIC32MZ2048EFH100"; break;
    case 0x0723B053: model = "PIC32MZ2048EFM100"; break;
    case 0x07217053: model = "PIC32MZ1024EFG124"; break;
    case 0x0721C053: model = "PIC32MZ1024EFH124"; break;
    case 0x07244053: model = "PIC32MZ1024EFM124"; break;
    case 0x07218053: model = "PIC32MZ2048EFG124"; break;
    case 0x0721D053: model = "PIC32MZ2048EFH124"; break;
    case 0x07245053: model = "PIC32MZ2048EFM124"; break;
    case 0x07221053: model = "PIC32MZ1024EFG144"; break;
    case 0x07226053: model = "PIC32MZ1024EFH144"; break;
    case 0x0724E053: model = "PIC32MZ1024EFM144"; break;
    case 0x07222053: model = "PIC32MZ2048EFG144"; break;
    case 0x07227053: model = "PIC32MZ2048EFH144"; break;
    case 0x0724F053: model = "PIC32MZ2048EFM144"; break;
    }
    if (model)
        strcpy(cpu_model, model);
    else
        sprintf(cpu_model, "PIC32MZ DevID %x", DEVID);

    /*
     * Find out how much memory is available.
     */
    physmem = btoc(512 * 1024);

    /*
     * Initialize error message buffer (at end of core).
     */
    msgbufp = (struct msgbuf *)MACH_PHYS_TO_UNCACHED(mips_ptob(physmem)) - 1;
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

    nswapmap = maxproc * 2;
    valloc(swapmap, struct map, nswapmap);

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
    if (nswbuf == 0)
        nswbuf = 4;                     /* even */
    valloc(swbuf, struct buf, nswbuf);
    valloc(buf, struct buf, nbuf);

    /*
     * Clear allocated memory.
     */
    bzero(start, v - start);

    /*
     * Initialize the virtual memory system.
     * As argument, pass the first page address following the system data.
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
#define LSW0    1       /* level for soft timer interrupts */
#define LSW1    2       /* level for soft network interrupts */
#define LSPI    3       /* level for SPI interrupts */
#define LETH    4       /* level for Ethernet interrupts */
#define LUART   5       /* level for UART interrupts */
#define LTMR    6       /* level for timer interrupts */

    /* 0 - Core Timer Interrupt
     * 1 - Core Software Interrupt 0
     * 2 - Core Software Interrupt 1
     * 3 - External Interrupt 0 */
    IPC(0) = PIC32_IPC_IP(LTMR, LSW0, LSW1, 0);
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
    IPC(27) = PIC32_IPC_IP(0, LSPI, LSPI, LSPI);

    /* 112 - UART1 Fault
     * 113 - UART1 Receive Done
     * 114 - UART1 Transfer Done
     * 115 - I2C1 Bus Collision Event */
    IPC(28) = PIC32_IPC_IP(LUART, LUART, LUART, 0);
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
    IPC(35) = PIC32_IPC_IP(0, 0, LSPI, LSPI);

    /* 144 - SPI2 Transfer Done
     * 145 - UART2 Fault
     * 146 - UART2 Receive Done
     * 147 - UART2 Transfer Done */
    IPC(36) = PIC32_IPC_IP(LSPI, LUART, LUART, LUART);

    IPC(37) = 0;

    /* 152 - Control Area Network 2
     * 153 - Ethernet Interrupt
     * 154 - SPI3 Fault
     * 155 - SPI3 Receive Done */
    IPC(38) = PIC32_IPC_IP(0, LETH, LSPI, LSPI);

    /* 156 - SPI3 Transfer Done
     * 157 - UART3 Fault
     * 158 - UART3 Receive Done
     * 159 - UART3 Transfer Done */
    IPC(39) = PIC32_IPC_IP(LSPI, LUART, LUART, LUART);

    /* 160 - I2C3 Bus Collision Event
     * 161 - I2C3 Slave Event
     * 162 - I2C3 Master Event
     * 163 - SPI4 Fault */
    IPC(40) = PIC32_IPC_IP(0, 0, 0, LSPI);

    /* 164 - SPI4 Receive Done
     * 165 - SPI4 Transfer Done
     * 166 - Real Time Clock
     * 167 - Flash Control Event */
    IPC(41) = PIC32_IPC_IP(LSPI, LSPI, LTMR, 0);

    /* 168 - Prefetch Module SEC Event
     * 169 - SQI1 Event
     * 170 - UART4 Fault
     * 171 - UART4 Receive Done */
    IPC(42) = PIC32_IPC_IP(0, 0, LUART, LUART);

    /* 172 - UART4 Transfer Done
     * 173 - I2C4 Bus Collision Event
     * 174 - I2C4 Slave Event
     * 175 - I2C4 Master Event */
    IPC(43) = PIC32_IPC_IP(LUART, 0, 0, 0);

    /* 176 - SPI5 Fault
     * 177 - SPI5 Receive Done
     * 178 - SPI5 Transfer Done
     * 179 - UART5 Fault */
    IPC(44) = PIC32_IPC_IP(LSPI, LSPI, LSPI, LUART);

    /* 180 - UART5 Receive Done
     * 181 - UART5 Transfer Done
     * 182 - I2C5 Bus Collision Event
     * 183 - I2C5 Slave Event */
    IPC(45) = PIC32_IPC_IP(LUART, LUART, 0, 0);

    /* 184 - I2C5 Master Event
     * 185 - SPI6 Fault
     * 186 - SPI6 Receive Done
     * 187 - SPI6 Transfer Done */
    IPC(46) = PIC32_IPC_IP(0, LSPI, LSPI, LSPI);

    /* 188 - UART6 Fault
     * 189 - UART6 Receive Done
     * 190 - UART6 Transfer Done
     * 191 - Reserved */
    IPC(47) = PIC32_IPC_IP(LUART, LUART, LUART, 0);

    /* Read processor ID register. */
    cpu.cpuprid = mfc0_PRId();

    /*
     * Enable instruction prefetch.
     */
    PRECON = 2;                 /* Two wait states. */
    PRECONSET = 0x30;           /* Enable predictive prefetch for any address */
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
 * File assym.h is incorrect - print the message and panic.
 */
static void
bad_assym(int *error, const char *name, unsigned value)
{
    if (! *error) {
        printf("Fatal error: file sys/mips/include/assym.h is incorrect.\n");
        printf("Please, update the following values and rebuild the kernel:\n");
        *error = 1;
    }
    printf("    #define %s %d\n", name, value);
}

/*
 * Verify assembler symbols.
 * File sys/mips/include/assym.h contains declarations of field offsets
 * of struct proc, user and vmmeter for assembler routines.
 * Here we verify that these values do match our expectations.
 */
static void
verify_assym()
{
    register struct proc *p = 0;
    register struct user *up = 0;
    register struct vmmeter *vm = 0;
    int error = 0;

    /* Offsets for struct proc */
    if (P_FORW != (unsigned)&p->p_forw)
        bad_assym(&error, "P_FORW", (unsigned)&p->p_forw);
    if (P_BACK != (unsigned)&p->p_back)
        bad_assym(&error, "P_BACK", (unsigned)&p->p_back);
    if (P_PRIORITY != (unsigned)&p->p_priority)
        bad_assym(&error, "P_PRIORITY", (unsigned)&p->p_priority);
    if (P_UPTE != (unsigned)p->p_md.md_upte)
        bad_assym(&error, "P_UPTE", (unsigned)p->p_md.md_upte);

    /* Offsets for struct user */
    if (U_PCB_REGS != (unsigned)up->u_pcb.pcb_regs)
        bad_assym(&error, "U_PCB_REGS", (unsigned)up->u_pcb.pcb_regs);
    if (U_PCB_FPREGS != (unsigned)&up->u_pcb.pcb_regs[F0])
        bad_assym(&error, "U_PCB_FPREGS", (unsigned)&up->u_pcb.pcb_regs[F0]);
    if (U_PCB_CONTEXT != (unsigned)&up->u_pcb.pcb_context)
        bad_assym(&error, "U_PCB_CONTEXT", (unsigned)&up->u_pcb.pcb_context);
    if (U_PCB_ONFAULT != (unsigned)&up->u_pcb.pcb_onfault)
        bad_assym(&error, "U_PCB_ONFAULT", (unsigned)&up->u_pcb.pcb_onfault);
    if (U_PCB_SEGTAB != (unsigned)&up->u_pcb.pcb_segtab)
        bad_assym(&error, "U_PCB_SEGTAB", (unsigned)&up->u_pcb.pcb_segtab);

    /* Offsets for struct vmmeter */
    if (V_SWTCH != (unsigned)&vm->v_swtch)
        bad_assym(&error, "V_SWTCH", (unsigned)&vm->v_swtch);

    if (error)
        panic("assym");
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
    verify_assym();
    printf("real mem = %d kbytes\n", ctob(physmem) >> 10);
#if 0
printf("callout  = %08x, %u entries, %u bytes\n", callout, ncallout, ncallout * sizeof(struct callout));
printf("swapmap  = %08x, %u entries, %u bytes\n", swapmap, nswapmap, nswapmap * sizeof(struct map));
printf("shmsegs  = %08x, %u entries, %u bytes\n", shmsegs, shminfo.shmmni, shminfo.shmmni * sizeof(struct shmid_ds));
printf("swbuf    = %08x, %u entries, %u bytes\n", swbuf, nswbuf, nswbuf * sizeof(struct buf));
printf("buf      = %08x, %u entries, %u bytes\n", buf, nbuf, nbuf * sizeof(struct buf));
printf("Sysmap   = %08x, %u entries, %u bytes\n", Sysmap, Sysmapsize, Sysmapsize * sizeof(pt_entry_t));
printf("pmap_attributes = %08x, %u entries, %u bytes\n", pmap_attributes, physmem, physmem);
printf("pv_table = %08x, %u entries, %u bytes\n", pv_table, pv_tabsz, pv_tabsz * 12);

printf("avail_start = %08x, avail_end = %08x\n", avail_start, avail_end);
#endif

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
        if (howto & RB_DUMP) {
            dumpsys();
            udelay(1000000);
        }

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
 * to which tvp points.
 */
void
microtime(tvp)
    struct timeval *tvp;
{
    int s = splhigh();
    int delta = mfc0_Count() - mfc0_Compare();
    *tvp = time;
    splx(s);

    delta += (CPU_KHZ * 1000 / HZ + 1) / 2;
    delta /= CPU_KHZ / 2000;
    if (delta < 0) {
        /* Cannot happen. */
        delta = 0;
    } else if (delta >= 1000000) {
        /* The kernel missed the timer interrupt for a whole second.
         * Something definitely went wrong. */
        panic("microtime watchdog");
    }
    tvp->tv_usec += delta;
    while (tvp->tv_usec > 1000000) {
        tvp->tv_sec++;
        tvp->tv_usec -= 1000000;
    }
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
    unsigned final = now + usec * (CPU_KHZ / 2000);

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
mips_flush_icache(unsigned addr, unsigned len)
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

/*
 * Routines for access to general purpose I/O pins.
 */
static const char pin_name[16] = "?ABCDEFGHJK?????";

int gpio_input_map1(int pin)
{
    switch (pin) {
    case RP('D',2):  return 0;
    case RP('G',8):  return 1;
    case RP('F',4):  return 2;
    case RP('D',10): return 3;
    case RP('F',1):  return 4;
    case RP('B',9):  return 5;
    case RP('B',10): return 6;
    case RP('C',14): return 7;
    case RP('B',5):  return 8;
    case RP('C',1):  return 10;
    case RP('D',14): return 11;
    case RP('G',1):  return 12;
    case RP('A',14): return 13;
    case RP('D',6):  return 14;
    }
    printf("gpio: cannot map peripheral input pin %c%d, group 1\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map2(int pin)
{
    switch (pin) {
    case RP('D',3):  return 0;
    case RP('G',7):  return 1;
    case RP('F',5):  return 2;
    case RP('D',11): return 3;
    case RP('F',0):  return 4;
    case RP('B',1):  return 5;
    case RP('E',5):  return 6;
    case RP('C',13): return 7;
    case RP('B',3):  return 8;
    case RP('C',4):  return 10;
    case RP('D',15): return 11;
    case RP('G',0):  return 12;
    case RP('A',15): return 13;
    case RP('D',7):  return 14;
    }
    printf("gpio: cannot map peripheral input pin %c%d, group 2\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map3(int pin)
{
    switch (pin) {
    case RP('D',9):  return 0;
    case RP('G',6):  return 1;
    case RP('B',8):  return 2;
    case RP('B',15): return 3;
    case RP('D',4):  return 4;
    case RP('B',0):  return 5;
    case RP('E',3):  return 6;
    case RP('B',7):  return 7;
    case RP('F',12): return 9;
    case RP('D',12): return 10;
    case RP('F',8):  return 11;
    case RP('C',3):  return 12;
    case RP('E',9):  return 13;
    }
    printf("gpio: cannot map peripheral input pin %c%d, group 3\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map4(int pin)
{
    switch (pin) {
    case RP('D',1):  return 0;
    case RP('G',9):  return 1;
    case RP('B',14): return 2;
    case RP('D',0):  return 3;
    case RP('B',6):  return 5;
    case RP('D',5):  return 6;
    case RP('B',2):  return 7;
    case RP('F',3):  return 8;
    case RP('F',13): return 9;
    case RP('F',2):  return 11;
    case RP('C',2):  return 12;
    case RP('E',8):  return 13;
    }
    printf("gpio: cannot map peripheral input pin %c%d, group 4\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

void gpio_set_input(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    port->trisset = (1 << (pin & 15));
    port->anselclr = (1 << (pin & 15));
}

void gpio_set_output(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    port->trisclr = (1 << (pin & 15));
    port->anselclr = (1 << (pin & 15));
}

void gpio_set_analog(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    port->trisset = (1 << (pin & 15));
    port->anselset = (1 << (pin & 15));
}

void gpio_set(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    port->latset = (1 << (pin & 15));
}

void gpio_clr(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    port->latclr = (1 << (pin & 15));
}

int gpio_get(int pin)
{
    struct gpioreg *port = (struct gpioreg*) &ANSELA;

    port += (pin >> 4 & 15) - 1;
    return ((port->port & (1 << (pin & 15))) ? 1 : 0);
}

char gpio_portname(int pin)
{
    return pin_name[(pin >> 4) & 15];
}

int gpio_pinno(int pin)
{
    return pin & 15;
}
