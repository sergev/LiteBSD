/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
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
#ifdef KTRACE
#include <sys/ktrace.h>
#endif
#include <net/netisr.h>

#include <machine/trap.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/pte.h>
#include <machine/mips_opcode.h>
#include <machine/pic32mz.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>

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
    register int type, i;
    unsigned ucode = 0;
    register struct proc *p = curproc;
    u_quad_t sticks;
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
        type |= T_USER;
        sticks = p->p_sticks;
    }
    switch (type) {
    case T_TLB_MOD:
        /* check for kernel address */
        if ((int)vadr < 0) {
            register pt_entry_t *pte;
            register unsigned entry;
            register vm_offset_t pa;

            pte = kvtopte(vadr);
            entry = pte->pt_entry;
#ifdef DIAGNOSTIC
            if (!(entry & PG_V) || (entry & PG_D))
                panic("trap: ktlbmod: invalid pte");
#endif
            if (! (entry & PG_D)) {
                /* write to read only page in the kernel */
                ftype = VM_PROT_WRITE;
                goto kernel_fault;
            }
            entry |= PG_D;
            pte->pt_entry = entry;
            vadr &= ~PGOFSET;
            tlb_update(vadr, entry);
            pa = PG_FRAME(entry);
#ifdef ATTR
            pmap_attributes[atop(pa)] |= PMAP_ATTR_MOD;
#else
            if (!IS_VM_PHYSADDR(pa))
                panic("trap: ktlbmod: unmanaged page");
            PHYS_TO_VM_PAGE(pa)->flags &= ~PG_CLEAN;
#endif
            return (pc);
        }
        /* FALLTHROUGH */

    case T_TLB_MOD+T_USER:
        {
        register pt_entry_t *pte;
        register unsigned entry;
        register vm_offset_t pa;
        pmap_t pmap = &p->p_vmspace->vm_pmap;

        if (!(pte = pmap_segmap(pmap, vadr)))
            panic("trap: utlbmod: invalid segmap");
        pte += (vadr >> PGSHIFT) & (NPTEPG - 1);
        entry = pte->pt_entry;
#ifdef DIAGNOSTIC
        if (!(entry & PG_V) || (entry & PG_D))
            panic("trap: utlbmod: invalid pte");
#endif
        if (! (entry & PG_D)) {
            /* write to read only page */
            ftype = VM_PROT_WRITE;
            goto dofault;
        }
        entry |= PG_D;
        pte->pt_entry = entry;
        vadr = (vadr & ~PGOFSET) | pmap->pm_tlbpid;
        tlb_update(vadr, entry);
        pa = PG_FRAME(entry);
#ifdef ATTR
        pmap_attributes[atop(pa)] |= PMAP_ATTR_MOD;
#else
        if (!IS_VM_PHYSADDR(pa))
            panic("trap: utlbmod: unmanaged page");
        PHYS_TO_VM_PAGE(pa)->flags &= ~PG_CLEAN;
#endif
        if (!USERMODE(statusReg))
            return (pc);
        goto out;
        }

    case T_TLB_LD_MISS:
    case T_TLB_ST_MISS:
        ftype = (type == T_TLB_ST_MISS) ? VM_PROT_WRITE : VM_PROT_READ;
        /* check for kernel address */
        if ((int)vadr < 0) {
            register vm_offset_t va;
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

    case T_TLB_LD_MISS+T_USER:
        ftype = VM_PROT_READ;
        goto dofault;

    case T_TLB_ST_MISS+T_USER:
        ftype = VM_PROT_WRITE;
    dofault:
        {
        register vm_offset_t va;
        register struct vmspace *vm;
        register vm_map_t map;
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
        break;
        }

    case T_ADDR_ERR_LD+T_USER:      /* misaligned or kseg access */
    case T_ADDR_ERR_ST+T_USER:      /* misaligned or kseg access */
    case T_BUS_ERR_IFETCH+T_USER:   /* BERR asserted to cpu */
    case T_BUS_ERR_LD_ST+T_USER:    /* BERR asserted to cpu */
        i = SIGSEGV;
        break;

    case T_SYSCALL+T_USER:
        {
        register int *locr0 = p->p_md.md_regs;
        register struct sysent *callp;
        unsigned int code;
        int numsys;
        struct args {
            int i[8];
        } args;
        int rval[2];
        struct sysent *systab;
        extern int nsysent;

        cnt.v_syscall++;
        /* Compute next PC after syscall instruction.
         * Don't use syscall in branch delay slot. */
        locr0[PC] += 4;
        systab = sysent;
        numsys = nsysent;
        code = locr0[V0];
        switch (code) {
        case SYS_syscall:
            /*
             * Code is first argument, followed by actual args.
             */
            code = locr0[A0];
            if (code >= numsys)
                callp = &systab[SYS_syscall]; /* (illegal) */
            else
                callp = &systab[code];
            i = callp->sy_argsize;
            args.i[0] = locr0[A1];
            args.i[1] = locr0[A2];
            args.i[2] = locr0[A3];
            if (i > 3 * sizeof(register_t)) {
                i = copyin((caddr_t)(locr0[SP] +
                        4 * sizeof(register_t)),
                    (caddr_t)&args.i[3],
                    (u_int)(i - 3 * sizeof(register_t)));
                if (i) {
                    locr0[V0] = i;
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
            if (code >= numsys)
                callp = &systab[SYS_syscall]; /* (illegal) */
            else
                callp = &systab[code];
            i = callp->sy_argsize;
            args.i[0] = locr0[A2];
            args.i[1] = locr0[A3];
            if (i > 2 * sizeof(register_t)) {
                i = copyin((caddr_t)(locr0[SP] +
                        4 * sizeof(register_t)),
                    (caddr_t)&args.i[2],
                    (u_int)(i - 2 * sizeof(register_t)));
                if (i) {
                    locr0[V0] = i;
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
            if (code >= numsys)
                callp = &systab[SYS_syscall]; /* (illegal) */
            else
                callp = &systab[code];
            i = callp->sy_argsize;
            args.i[0] = locr0[A0];
            args.i[1] = locr0[A1];
            args.i[2] = locr0[A2];
            args.i[3] = locr0[A3];
            if (i > 4 * sizeof(register_t)) {
                i = copyin((caddr_t)(locr0[SP] +
                        4 * sizeof(register_t)),
                    (caddr_t)&args.i[4],
                    (u_int)(i - 4 * sizeof(register_t)));
                if (i) {
                    locr0[V0] = i;
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

        i = (*callp->sy_call)(p, &args, rval);
        /*
         * Reinitialize proc pointer `p' as it may be different
         * if this is a child returning from fork syscall.
         */
        p = curproc;
        locr0 = p->p_md.md_regs;

        switch (i) {
        case 0:
            locr0[V0] = rval[0];
            locr0[V1] = rval[1];
            locr0[A3] = 0;
            break;

        case ERESTART:
            locr0[PC] = pc;
            break;

        case EJUSTRETURN:
            break;  /* nothing to do */

        default:
            locr0[V0] = i;
            locr0[A3] = 1;
        }
    done:
#ifdef KTRACE
        if (KTRPOINT(p, KTR_SYSRET))
            ktrsysret(p->p_tracep, code, i, rval[0]);
#endif
        goto out;
        }

    case T_BREAK+T_USER:
        {
        register unsigned va, instr;

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

    case T_RES_INST+T_USER:
        i = SIGILL;
        break;

    case T_COP_UNUSABLE+T_USER:
        i = SIGILL;
        break;

    case T_OVFLOW+T_USER:
        i = SIGFPE;
        break;

    case T_ADDR_ERR_LD:     /* misaligned access */
    case T_ADDR_ERR_ST:     /* misaligned access */
    case T_BUS_ERR_LD_ST:   /* BERR asserted to cpu */
        i = ((struct pcb *)UADDR)->pcb_onfault;
        if (i) {
            ((struct pcb *)UADDR)->pcb_onfault = 0;
            return (onfault_table[i]);
        }
        /* FALLTHROUGH */

    default:
    err:
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
    return (pc);
}

/*
 * Handle an interrupt.
 * Called from kern_interrupt() or user_interrupt()
 * Note: curproc might be NULL.
 */
interrupt(statusReg, pc)
    unsigned statusReg;     /* status register at time of the exception */
    unsigned pc;            /* program counter where to continue */
{
    struct clockframe cf;

    cnt.v_intr++;

    /* Get the current irq number */
    int intstat = INTSTAT;
    int irq = PIC32_INTSTAT_VEC (intstat);

    /* Handle the interrupt. */
    switch (irq) {
    case PIC32_IRQ_CT:                  /* Core Timer */
        /* Increment COMPARE register. */
        IFSCLR(0) = 1 << PIC32_IRQ_CT;
        int c = mfc0_Compare();
        c += (CPU_KHZ * 1000 / HZ + 1) / 2;
        mtc0_Compare (c);

        cf.pc = pc;
        cf.sr = statusReg;
        hardclock(&cf);
        break;

    case PIC32_IRQ_CS0:                 /* Core software interrupt 0 */
        clearsoftclock();
        cnt.v_soft++;
        softclock();
        break;

    case PIC32_IRQ_CS1:                 /* Core software interrupt 1 */
        clearsoftnet();
        cnt.v_soft++;
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

#ifdef UART1_ENABLED
    case PIC32_IRQ_U1E:                 /* UART1 */
    case PIC32_IRQ_U1RX:                /* UART1 */
    case PIC32_IRQ_U1TX:                /* UART1 */
        uartintr(makedev(UART_MAJOR,0));
        break;
#endif
    }
}

/*
 * This is called from user_interrupt() if astpending is set.
 * This is very similar to the tail of exception().
 */
softintr()
{
    register struct proc *p = curproc;
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
cpu_singlestep(p)
    register struct proc *p;
{
    // TODO: use Debug.SST bit.
    return (0);
}
