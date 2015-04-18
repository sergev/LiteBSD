/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2014 Serge Vakulenko
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
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
 * from: Utah $Hdr: trap.c 1.32 91/04/06$
 *
 *      @(#)trap.c      8.7 (Berkeley) 6/2/95
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/signalvar.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/wait.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif
#include <net/netisr.h>

#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include <machine/trap.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/pte.h>
#include <machine/pic32mz.h>
#include <mips/dev/device.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>

static void
syscall(p, causeReg, pc)
    struct proc *p;
    unsigned causeReg;      /* cause register at time of exception */
    unsigned pc;            /* program counter where to continue */
{
    int *locr0 = p->p_md.md_regs;
    int *rval = p->p_md.md_rval;
    struct sysent *callp;
    unsigned int code = locr0[V0];
    struct args {
        int i[8];
    } args;
    int error;
//printf ("--- %s(pid=%u) syscall code=%u, RA=%08x \n", __func__, p->p_pid, code, locr0[RA]);

    if ((int)causeReg < 0) {
        /* Don't use syscall in branch delay slot. */
        exit1(p, W_EXITCODE(0, SIGABRT));
        error = EINVAL;
        goto done;
    }
    /* Compute next PC after syscall instruction. */
    locr0[PC] += 4;

    switch (code) {
    case SYS_syscall:
        /*
         * Code is first argument, followed by actual args.
         */
        code = locr0[A0];
        if (code >= nsysent)
            callp = &sysent[SYS_syscall]; /* (illegal) */
        else
            callp = &sysent[code];
        args.i[0] = locr0[A1];
        args.i[1] = locr0[A2];
        args.i[2] = locr0[A3];
        if (callp->sy_argsize > 3 * sizeof(register_t)) {
            error = copyin((caddr_t)(locr0[SP] +
                    4 * sizeof(register_t)),
                (caddr_t)&args.i[3],
                (u_int)(callp->sy_argsize - 3 * sizeof(register_t)));
            if (error) {
                locr0[V0] = error;
                locr0[A3] = 1;
#ifdef KTRACE
                if (KTRPOINT(p, KTR_SYSCALL))
                    ktrsyscall(p->p_tracep, code,
                        callp->sy_argsize,
                        args.i);
#endif
                goto done;
            }
        }
        break;

    case SYS___syscall:
        /*
         * Like syscall, but code is a quad, so as to maintain
         * quad alignment for the rest of the arguments.
         */
        code = locr0[A0 + _QUAD_LOWWORD];
        if (code >= nsysent)
            callp = &sysent[SYS_syscall]; /* (illegal) */
        else
            callp = &sysent[code];
        args.i[0] = locr0[A2];
        args.i[1] = locr0[A3];
        if (callp->sy_argsize > 2 * sizeof(register_t)) {
            error = copyin((caddr_t)(locr0[SP] +
                    4 * sizeof(register_t)),
                (caddr_t)&args.i[2],
                (u_int)(callp->sy_argsize - 2 * sizeof(register_t)));
            if (error) {
                locr0[V0] = error;
                locr0[A3] = 1;
#ifdef KTRACE
                if (KTRPOINT(p, KTR_SYSCALL))
                    ktrsyscall(p->p_tracep, code,
                        callp->sy_argsize,
                        args.i);
#endif
                goto done;
            }
        }
        break;

    default:
        if (code >= nsysent)
            callp = &sysent[SYS_syscall]; /* (illegal) */
        else
            callp = &sysent[code];
        args.i[0] = locr0[A0];
        args.i[1] = locr0[A1];
        args.i[2] = locr0[A2];
        args.i[3] = locr0[A3];
        if (callp->sy_argsize > 4 * sizeof(register_t)) {
            error = copyin((caddr_t)(locr0[SP] +
                    4 * sizeof(register_t)),
                (caddr_t)&args.i[4],
                (u_int)(callp->sy_argsize - 4 * sizeof(register_t)));
            if (error) {
                locr0[V0] = error;
                locr0[A3] = 1;
#ifdef KTRACE
                if (KTRPOINT(p, KTR_SYSCALL))
                    ktrsyscall(p->p_tracep, code,
                        callp->sy_argsize,
                        args.i);
#endif
                goto done;
            }
        }
    }
#ifdef KTRACE
    if (KTRPOINT(p, KTR_SYSCALL))
        ktrsyscall(p->p_tracep, code, callp->sy_argsize, args.i);
#endif
    rval[0] = 0;
    rval[1] = locr0[V1];

    error = (*callp->sy_call)(p, &args, rval);
    /*
     * Reinitialize proc pointer `p' as it may be different
     * if this is a child returning from fork syscall.
     */
    p = curproc;
    rval = p->p_md.md_rval;
    locr0 = p->p_md.md_regs;

    switch (error) {
    case 0:
//printf ("--- (%u) syscall succeded, return %u \n", p->p_pid, rval[0]);
        locr0[V0] = rval[0];
        locr0[V1] = rval[1];
        locr0[A3] = 0;
        break;

    case ERESTART:
//printf ("--- (%u) syscall restarted \n", p->p_pid);
        locr0[PC] = pc;
        break;

    case EJUSTRETURN:
//printf ("--- (%u) syscall just return \n", p->p_pid);
        break;  /* nothing to do */

    default:
//printf ("--- (%u) syscall failed, error %u \n", p->p_pid, error);
        locr0[V0] = error;
        locr0[A3] = 1;
    }
done:;
#ifdef KTRACE
    if (KTRPOINT(p, KTR_SYSRET))
        ktrsysret(p->p_tracep, code, error, rval[0]);
#endif
}

/*
 * Handle an exception.
 * Called from kern_exception() or user_exception()
 * when a processor exception occurs.
 * In the case of a kernel exception, we return the pc where to resume if
 * ((struct pcb *)UADDR)->pcb_onfault is set, otherwise, return old pc.
 */
unsigned
exception(statusReg, causeReg, vadr, pc, args)
    unsigned statusReg;     /* status register at time of the exception */
    unsigned causeReg;      /* cause register at time of exception */
    unsigned vadr;          /* address (if any) the fault occured on */
    unsigned pc;            /* program counter where to continue */
{
    int type, i;
    unsigned ucode = 0;
    struct proc *p = curproc;
    u_quad_t sticks = 0;
    vm_prot_t ftype;
    extern unsigned onfault_table[];

    cnt.v_trap++;

    /*
     * Enable hardware interrupts if they were on before.
     * We only respond to software interrupts when returning to user mode.
     */
    if (statusReg & MACH_Status_IE) {
        int ipl = CURPRI(statusReg);
        if (ipl < 2)
            spl2();
        else
            splx(statusReg);
    }

    type = (causeReg & MACH_Cause_ExcCode) >> MACH_Cause_ExcCode_SHIFT;
    if (USERMODE(statusReg)) {
        type |= TRAP_USER;
        sticks = p->p_sticks;
    }
    switch (type) {
    case TRAP_MOD:                      /* Kernel: TLB modify */
        /* check for kernel address */
        if ((int)vadr < 0) {
#ifdef DIAGNOSTIC
            pt_entry_t *pte = kvtopte(vadr);
            unsigned entry = pte->pt_entry;

            if (!(entry & PG_V) || (entry & PG_D))
                panic("trap: ktlbmod: invalid pte");
#endif
            /* write to read only page in the kernel */
            ftype = VM_PROT_WRITE;
            goto kernel_fault;
        }
        /* FALLTHROUGH */

    case TRAP_MOD + TRAP_USER:          /* User: TLB modify */
        /* write to read only page */
        ftype = VM_PROT_WRITE;
        goto dofault;

    case TRAP_TLBL:                     /* Kernel: TLB refill */
    case TRAP_TLBS:
        ftype = (type == TRAP_TLBS) ? VM_PROT_WRITE : VM_PROT_READ;
        /* check for kernel address */
        if ((int)vadr < 0) {
            vm_offset_t va;
            int rv;

kernel_fault:
            va = trunc_page((vm_offset_t)vadr);
            rv = vm_fault(kernel_map, va, ftype, FALSE);
            if (rv == KERN_SUCCESS)
                return (pc);
            i = ((struct pcb *)UADDR)->pcb_onfault;
            if (i) {
                ((struct pcb *)UADDR)->pcb_onfault = 0;
                return (onfault_table[i]);
            }
            goto err;
        }
        /*
         * It is an error for the kernel to access user space except
         * through the copyin/copyout routines.
         */
        i = ((struct pcb *)UADDR)->pcb_onfault;
        if (i == 0)
            goto err;
        /* check for fuswintr() or suswintr() getting a page fault */
        if (i == 4)
            return (onfault_table[i]);
        goto dofault;

    case TRAP_TLBL + TRAP_USER:         /* User: TLB refill load/fetch */
        ftype = VM_PROT_READ;
        goto dofault;

    case TRAP_TLBS + TRAP_USER:         /* User: TLB refill store */
        ftype = VM_PROT_WRITE;
dofault:
        {
        vm_offset_t va;
        struct vmspace *vm;
        vm_map_t map;
        int rv;

        vm = p->p_vmspace;
        map = &vm->vm_map;
        va = trunc_page((vm_offset_t)vadr);
        rv = vm_fault(map, va, ftype, FALSE);
        /*
         * If this was a stack access we keep track of the maximum
         * accessed stack size.  Also, if vm_fault gets a protection
         * failure it is due to accessing the stack region outside
         * the current limit and we need to reflect that as an access
         * error.
         */
        if ((caddr_t)va >= vm->vm_maxsaddr) {
            if (rv == KERN_SUCCESS) {
                unsigned nss;

                nss = clrnd(btoc(USRSTACK-(unsigned)va));
                if (nss > vm->vm_ssize)
                    vm->vm_ssize = nss;
            } else if (rv == KERN_PROTECTION_FAILURE)
                rv = KERN_INVALID_ADDRESS;
        }
        if (rv == KERN_SUCCESS) {
            if (type == TRAP_MOD || type == TRAP_MOD + TRAP_USER) {
                /*
                 * Mark page as dirty.
                 */
                pt_entry_t *pte = pmap_segmap(&p->p_vmspace->vm_pmap, vadr);
                if (!pte)
                    panic("trap: utlbmod: invalid segmap");
                pte += (vadr >> PGSHIFT) & (NPTEPG - 1);

                vm_offset_t pa = PG_FRAME(pte->pt_entry);
#ifdef ATTR
                pmap_attributes[atop(pa)] |= PMAP_ATTR_MOD;
#else
                if (!IS_VM_PHYSADDR(pa))
                    panic("trap: utlbmod: unmanaged page");
                PHYS_TO_VM_PAGE(pa)->flags &= ~PG_CLEAN;
#endif
            }
            if (!USERMODE(statusReg))
                return (pc);
            goto out;
        }
        if (! USERMODE(statusReg)) {
            i = ((struct pcb *)UADDR)->pcb_onfault;
            if (i) {
                ((struct pcb *)UADDR)->pcb_onfault = 0;
                return (onfault_table[i]);
            }
            goto err;
        }
        ucode = vadr;
        i = (rv == KERN_PROTECTION_FAILURE) ? SIGBUS : SIGSEGV;
#if 1
        // Terminate the process.
        printf("PID %d (%s) protection violation at %x: BadVAddr = %08x \n",
            p->p_pid, p->p_comm, pc, vadr);
        exit1(p, W_EXITCODE(0, i));
#endif
        break;
        }

    case TRAP_TLBRI + TRAP_USER:        /* TLB read-inhibit */
        ftype = VM_PROT_READ;
        goto dofault;

    case TRAP_TLBEI + TRAP_USER:        /* TLB execute-inhibit */
        ftype = VM_PROT_EXECUTE;
        goto dofault;

    case TRAP_AdEL + TRAP_USER:     /* User: address error on load/fetch */
    case TRAP_AdES + TRAP_USER:     /* User: address error on store */
    case TRAP_IBE + TRAP_USER:      /* User: bus error on fetch */
    case TRAP_DBE + TRAP_USER:      /* User: bus error on load/store */
        i = SIGSEGV;
#if 1
        // Terminate the process.
        printf("PID %d (%s) address error at %x: BadVAddr = %08x \n",
            p->p_comm, p->p_pid, pc, vadr);
        exit1(p, W_EXITCODE(0, i));
#endif
        break;

    case TRAP_Sys + TRAP_USER:      /* User: syscall */
        cnt.v_syscall++;
        syscall(p, causeReg, pc);

        /* Reinitialize proc pointer `p' as it may be different
         * if this is a child returning from fork syscall. */
        p = curproc;
        goto out;

    case TRAP_Bp + TRAP_USER:           /* User: breakpoint */
        {
        unsigned va, instr;

        /* compute address of break instruction */
        va = pc;
        if ((int)causeReg < 0)
            va += 4;

        /* read break instruction */
        instr = fuiword((caddr_t)va);
#if 0
        printf("trap: %s (%d) breakpoint %x at %x: (adr %x ins %x)\n",
            p->p_comm, p->p_pid, instr, pc,
            p->p_md.md_ss_addr, p->p_md.md_ss_instr); /* XXX */
#endif
        if (p->p_md.md_ss_addr != va || instr != MACH_BREAK_SSTEP) {
            i = SIGTRAP;
            break;
        }

        /* restore original instruction and clear BP  */
        i = suiword((caddr_t)va, p->p_md.md_ss_instr);
        if (i < 0) {
            vm_offset_t sa, ea;
            int rv;

            sa = trunc_page((vm_offset_t)va);
            ea = round_page((vm_offset_t)va+sizeof(int)-1);
            rv = vm_map_protect(&p->p_vmspace->vm_map, sa, ea,
                VM_PROT_DEFAULT, FALSE);
            if (rv == KERN_SUCCESS) {
                i = suiword((caddr_t)va, p->p_md.md_ss_instr);
                (void) vm_map_protect(&p->p_vmspace->vm_map,
                    sa, ea, VM_PROT_READ|VM_PROT_EXECUTE,
                    FALSE);
            }
        }
        if (i < 0)
            printf("Warning: can't restore instruction at %x: %x\n",
                p->p_md.md_ss_addr, p->p_md.md_ss_instr);
        p->p_md.md_ss_addr = 0;
        i = SIGTRAP;
        break;
        }

    case TRAP_RI + TRAP_USER:           /* User: reserved instruction */
        i = SIGILL;
        break;

    case TRAP_CPU + TRAP_USER:          /* User: coprocessor unusable */
printf ("--- (%u) coprocessor unusable at pc=%08x\n", p->p_pid, pc);
        i = SIGFPE;
        break;

    case TRAP_Ov + TRAP_USER:           /* User: overflow */
        i = SIGFPE;
        break;

    case TRAP_Tr + TRAP_USER:           /* User: trap */
        i = SIGTRAP;
#if 1
        // Terminate the process.
        printf("PID %d (%s) trap at %x \n", p->p_pid, p->p_comm, pc);
        exit1(p, W_EXITCODE(0, i));
#endif
        break;

    case TRAP_DSPDis + TRAP_USER:       /* User: DSP disabled */
printf ("--- (%u) DSP disabled pc=%08x\n", p->p_pid, pc);
        i = SIGFPE;
        break;

    case TRAP_WATCH + TRAP_USER:        /* User: access to WatchHi/Lo address */
        i = SIGTRAP;
#if 1
        printf("PID %d (%s) watch exception at %x \n", p->p_pid, p->p_comm, pc);
#endif
        break;

    case TRAP_AdEL:                     /* Kernel: address error on load/fetch */
    case TRAP_AdES:                     /* Kernel: address error on store */
    case TRAP_DBE:                      /* Kernel: bus error on load/store */
        i = ((struct pcb *)UADDR)->pcb_onfault;
        if (i) {
            ((struct pcb *)UADDR)->pcb_onfault = 0;
            return (onfault_table[i]);
        }
        /* FALLTHROUGH */

    default:
err:
        printf("kernel fault 0x%x at pc=%08x, badvaddr=%08x\n", type, pc, vadr);
        panic("trap");
    }
    trapsignal(p, i, ucode);
out:
    /*
     * Note: we should only get here if returning to user mode.
     */
    /* take pending signals */
    while ((i = CURSIG(p)) != 0)
        postsig(i);
    p->p_priority = p->p_usrpri;
    astpending = 0;
    if (want_resched) {
        int s;

        /*
         * Since we are curproc, clock will normally just change
         * our priority without moving us from one queue to another
         * (since the running process is not on a queue.)
         * If that happened after we put ourselves on the run queue
         * but before we switched, we might not be on the queue
         * indicated by our priority.
         */
        s = splstatclock();
        setrunqueue(p);
        p->p_stats->p_ru.ru_nivcsw++;
        mi_switch();
        splx(s);
        while ((i = CURSIG(p)) != 0)
            postsig(i);
    }

    /*
     * If profiling, charge system time to the trapped pc.
     */
    if (p->p_flag & P_PROFIL) {
        extern int psratio;

        addupc_task(p, pc, (int)(p->p_sticks - sticks) * psratio);
    }

    curpriority = p->p_priority;
    return p->p_md.md_regs[PC];
}

/*
 * Bump a timeval by a small number of usec's.
 */
#define BUMPTIME(t, usec) { \
    register volatile struct timeval *tp = (t); \
    register long us; \
 \
    tp->tv_usec = us = tp->tv_usec + (usec); \
    if (us >= 1000000) { \
        tp->tv_usec = us - 1000000; \
        tp->tv_sec++; \
    } \
}

/*
 * Handle an interrupt.
 * Called from kern_interrupt() or user_interrupt()
 * Note: curproc might be NULL.
 */
void
interrupt(statusReg, pc)
    unsigned statusReg;     /* status register at time of the exception */
    unsigned pc;            /* program counter where to continue */
{
    struct clockframe cf;
    int clk;
    unsigned count;

    cnt.v_intr++;

    /* Get the current irq number */
    int intstat = INTSTAT;
    int irq = PIC32_INTSTAT_VEC (intstat);
//printf("%s: irq %u\n", __func__, irq);

    /* Handle the interrupt. */
    switch (irq) {
    case PIC32_IRQ_CT:                  /* Core Timer */
        /* Increment COMPARE register. */
        IFSCLR(0) = 1 << PIC32_IRQ_CT;
        clk = mfc0_Compare();
again:
        count = mfc0_Count();
        if ((int) (clk - count) > 0) {
            /* Spurious interrupt, ignore. */
            break;
        }
        clk += (CPU_KHZ * 1000 / HZ + 1) / 2;
        mtc0_Compare (clk);
        if ((int) (clk - count) < 0) {
            /* Lost one tick. */
            BUMPTIME(&time, tick);
            goto again;
        }
        intrcnt.clock++;
        cf.pc = pc;
        cf.sr = statusReg;
        hardclock(&cf);
        break;

    case PIC32_IRQ_CS0:                 /* Core software interrupt 0 */
        IFSCLR(0) = 1 << PIC32_IRQ_CS0;
        clearsoftclock();
        cnt.v_soft++;
        intrcnt.softclock++;
        softclock();
        break;

    case PIC32_IRQ_CS1:                 /* Core software interrupt 1 */
        IFSCLR(0) = 1 << PIC32_IRQ_CS1;
        clearsoftnet();
        cnt.v_soft++;
        intrcnt.softnet++;
#ifdef INET
        if (netisr & (1 << NETISR_ARP)) {
            netisr &= ~(1 << NETISR_ARP);
            arpintr();
        }
        if (netisr & (1 << NETISR_IP)) {
            netisr &= ~(1 << NETISR_IP);
            ipintr();
        }
#endif
        break;

    case PIC32_IRQ_U1E: case PIC32_IRQ_U1RX: case PIC32_IRQ_U1TX: /* UART1 */
        intrcnt.uart1++;
        uartintr(0);
        break;
    case PIC32_IRQ_U2E: case PIC32_IRQ_U2RX: case PIC32_IRQ_U2TX: /* UART2 */
        intrcnt.uart2++;
        uartintr(1);
        break;
    case PIC32_IRQ_U3E: case PIC32_IRQ_U3RX: case PIC32_IRQ_U3TX: /* UART3 */
        intrcnt.uart3++;
        uartintr(2);
        break;
    case PIC32_IRQ_U4E: case PIC32_IRQ_U4RX: case PIC32_IRQ_U4TX: /* UART4 */
        intrcnt.uart4++;
        uartintr(3);
        break;
    case PIC32_IRQ_U5E: case PIC32_IRQ_U5RX: case PIC32_IRQ_U5TX: /* UART5 */
        intrcnt.uart5++;
        uartintr(4);
        break;
    case PIC32_IRQ_U6E: case PIC32_IRQ_U6RX: case PIC32_IRQ_U6TX: /* UART6 */
        intrcnt.uart6++;
        uartintr(5);
        break;

#include "en.h"
#if NEN > 0
    case PIC32_IRQ_ETH:                 /* Ethernet interrupt */
        intrcnt.ether++;
        enintr(0);
        break;
#endif

#include "mrf.h"
#if NMRF > 0
#if WF_INT == 0
    case PIC32_IRQ_INT0:                /* Wi-Fi interrupt at INT0 */
        intrcnt.ether++;
        mrfintr(0);
        break;
#endif
#if WF_INT == 1
    case PIC32_IRQ_INT1:                /* Wi-Fi interrupt at INT1 */
        intrcnt.ether++;
        mrfintr(0);
        break;
#endif
#if WF_INT == 2
    case PIC32_IRQ_INT2:                /* Wi-Fi interrupt at INT2 */
        intrcnt.ether++;
        mrfintr(0);
        break;
#endif
#if WF_INT == 3
    case PIC32_IRQ_INT3:                /* Wi-Fi interrupt at INT3 */
        intrcnt.ether++;
        mrfintr(0);
        break;
#endif
#if WF_INT == 4
    case PIC32_IRQ_INT4:                /* Wi-Fi interrupt at INT4 */
        intrcnt.ether++;
        mrfintr(0);
        break;
#endif
#endif
    }
}

/*
 * This is called from user_interrupt() if astpending is set.
 * This is very similar to the tail of exception().
 */
void
softintr()
{
    struct proc *p = curproc;
    int sig;

    cnt.v_soft++;

    /* take pending signals */
    while ((sig = CURSIG(p)) != 0)
        postsig(sig);
    p->p_priority = p->p_usrpri;
    astpending = 0;
    if (p->p_flag & P_OWEUPC) {
        p->p_flag &= ~P_OWEUPC;
        ADDUPROF(p);
    }
    if (want_resched) {
        int s;

        /*
         * Since we are curproc, clock will normally just change
         * our priority without moving us from one queue to another
         * (since the running process is not on a queue.)
         * If that happened after we put ourselves on the run queue
         * but before we switched, we might not be on the queue
         * indicated by our priority.
         */
        s = splstatclock();
        setrunqueue(p);
        p->p_stats->p_ru.ru_nivcsw++;
        mi_switch();
        splx(s);
        while ((sig = CURSIG(p)) != 0)
            postsig(sig);
    }
    curpriority = p->p_priority;
}

/*
 * This routine is called by procxmt() to single step one instruction.
 */
int
cpu_singlestep(p)
    struct proc *p;
{
    // TODO: use Debug.SST bit.
    return (0);
}
