/*
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2014 Serge Vakulenko
 *
 * This code is derived from software contributed to Berkeley by
 * Digital Equipment Corporation and Ralph Campbell.
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
 * Copyright (C) 1989 Digital Equipment Corporation.
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies.
 * Digital Equipment Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * from: $Header: /sprite/src/kernel/mach/ds3100.md/RCS/loMem.s,
 *      v 1.1 89/07/11 17:55:04 nelson Exp $ SPRITE (DECWRL)
 * from: $Header: /sprite/src/kernel/mach/ds3100.md/RCS/machAsm.s,
 *      v 9.2 90/01/29 18:00:39 shirriff Exp $ SPRITE (DECWRL)
 * from: $Header: /sprite/src/kernel/vm/ds3100.md/vmPmaxAsm.s,
 *      v 1.1 89/07/10 14:27:41 nelson Exp $ SPRITE (DECWRL)
 *
 *      @(#)locore.s    8.7 (Berkeley) 6/2/95
 */

/*
 *      Contains code that is the first executed at boot time plus
 *      assembly language support routines.
 */

#include <sys/errno.h>
#include <sys/syscall.h>

#include <machine/param.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/machAsmDefs.h>
#include <machine/pte.h>
#include <machine/assym.h>

        .set    noreorder               # Don't allow the assembler to reorder instructions.
        .set    noat

/*-----------------------------------
 * Reset/NMI exception handler.
 */
        .globl  start
        .type   start, @function
        //
        // CPU initialization code copied from MIPS MD00901 Application Note
        // "Boot-MIPS: Example Boot Code for MIPS Cores"
        // http://www.imgtec.com/downloads/app-notes/MD00901-2B-CPS-APP-01.03.zip
        //
start:
        mtc0    zero, MACH_C0_Count     # Clear cp0 Count (Used to measure boot time.)
#if 0
check_nmi:                              # Check whether we are here due to a reset or NMI.
        mfc0    s1, MACH_C0_Status      # Read Status
        ext     s1, s1, 19, 1           # extract NMI
        beqz    s1, init_gpr            # Branch if this is NOT an NMI exception.
        nop

        # Call nmi_exception().
        la      sp, _eram - 16          # Set up stack base.
        la      gp, _gp                 # GP register value defined by linker script.
        la      s1, nmi_exception       # Call user-defined NMI handler.
        jalr    s1
        nop
#endif
        //
        // Set all GPRs of all register sets to predefined state.
        //
init_gpr:
        li      $1, 0xdeadbeef          # 0xdeadbeef stands out, kseg2 mapped, odd.

        # Determine how many shadow sets are implemented (in addition to the base register set.)
        # the first time thru the loop it will initialize using $1 set above.
        # At the bottom og the loop, 1 is  subtract from $30
        # and loop back to next_shadow_set to start the next loop and the next lowest set number.
        mfc0    $29, MACH_C0_SRSCtl     # read SRSCtl
        ext     $30, $29, 26, 4         # extract HSS

next_shadow_set:                        # set PSS to shadow set to be initialized
        ins     $29, $30, 6, 4          # insert PSS
        mtc0    $29, MACH_C0_SRSCtl     # write SRSCtl

        wrpgpr  $1, $1
        wrpgpr  $2, $1
        wrpgpr  $3, $1
        wrpgpr  $4, $1
        wrpgpr  $5, $1
        wrpgpr  $6, $1
        wrpgpr  $7, $1
        wrpgpr  $8, $1
        wrpgpr  $9, $1
        wrpgpr  $10, $1
        wrpgpr  $11, $1
        wrpgpr  $12, $1
        wrpgpr  $13, $1
        wrpgpr  $14, $1
        wrpgpr  $15, $1
        wrpgpr  $16, $1
        wrpgpr  $17, $1
        wrpgpr  $18, $1
        wrpgpr  $19, $1
        wrpgpr  $20, $1
        wrpgpr  $21, $1
        wrpgpr  $22, $1
        wrpgpr  $23, $1
        wrpgpr  $24, $1
        wrpgpr  $25, $1
        wrpgpr  $26, $1
        wrpgpr  $27, $1
        wrpgpr  $28, $1
        beqz    $30, init_cp0
        wrpgpr  $29, $1

        wrpgpr  $30, $1
        wrpgpr  $31, $1
        b       next_shadow_set
        add     $30, -1                 # Decrement to the next lower number

        //
        // Init CP0 Status, Count, Compare, Watch*, and Cause.
        //
init_cp0:
        # Initialize Status
        li      v1, MACH_Status_BEV | MACH_Status_ERL
        mtc0    v1, MACH_C0_Status      # write Status

        # Initialize Watch registers if implemented.
        mfc0    v0, MACH_C0_Config1     # read Config1
        ext     v1, v0, 3, 1            # extract bit 3 WR (Watch registers implemented)
        beq     v1, zero, done_wr
        li      v1, 0x7                 # (M_WatchHiI | M_WatchHiR | M_WatchHiW)

        # Clear Watch Status bits and disable watch exceptions
        mtc0    v1, MACH_C0_WatchHi     # write WatchHi0
        mfc0    v0, MACH_C0_WatchHi     # read WatchHi0
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo   # clear WatchLo0

        mtc0    v1, MACH_C0_WatchHi, 1  # write WatchHi1
        mfc0    v0, MACH_C0_WatchHi, 1  # read WatchHi1
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,1 # clear WatchLo1

        mtc0    v1, MACH_C0_WatchHi, 2  # write WatchHi2
        mfc0    v0, MACH_C0_WatchHi, 2  # read WatchHi2
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,2 # clear WatchLo2

        mtc0    v1, MACH_C0_WatchHi, 3  # write WatchHi3
        mfc0    v0, MACH_C0_WatchHi, 3  # read WatchHi3
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,3 # clear WatchLo3

        mtc0    v1, MACH_C0_WatchHi, 4  # write WatchHi4
        mfc0    v0, MACH_C0_WatchHi, 4  # read WatchHi4
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,4 # clear WatchLo4

        mtc0    v1, MACH_C0_WatchHi, 5  # write WatchHi5
        mfc0    v0, MACH_C0_WatchHi, 5  # read WatchHi5
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,5 # clear WatchLo5

        mtc0    v1, MACH_C0_WatchHi, 6  # write WatchHi6
        mfc0    v0, MACH_C0_WatchHi, 6  # read WatchHi6
        bgez    v0, done_wr             # Check for bit 31 (sign bit) for more Watch registers
        mtc0    zero, MACH_C0_WatchLo,6 # clear WatchLo6

        mtc0    v1, MACH_C0_WatchHi, 7  # write WatchHi7
        mtc0    zero, MACH_C0_WatchLo,7 # clear WatchLo7

done_wr:
        # Clear WP bit to avoid watch exception upon user code entry, IV, and software interrupts.
        mtc0    zero, MACH_C0_Cause     # clear Cause: init AFTER init of WatchHi/Lo registers.

        # Clear timer interrupt. (Count was cleared at the reset vector to allow timing boot.)
        mtc0    zero, MACH_C0_Compare   # clear Compare

/*-----------------------------------
 * Initialization.
 */
        //
        // Clear TLB: generate unique EntryHi contents per entry pair.
        //
init_tlb:
        # Determine if we have a TLB
        mfc0    v1, MACH_C0_Config      # read Config
        ext     v1, v1, 7, 3            # extract MT field
        li      a3, 0x1                 # load a 1 to check against
        bne     v1, a3, init_icache

        # Config1MMUSize == Number of TLB entries - 1
        mfc0    v0, MACH_C0_Config1     # Config1
        ext     v1, v0, 25, 6           # extract MMU Size
        mtc0    zero, MACH_C0_EntryLo0  # clear EntryLo0
        mtc0    zero, MACH_C0_EntryLo1  # clear EntryLo1
        mtc0    zero, MACH_C0_PageMask  # clear PageMask
        mtc0    zero, MACH_C0_Wired     # clear Wired
        li      a0, 0x80000000

next_tlb_entry:
        mtc0    v1, MACH_C0_Index       # write Index
        mtc0    a0, MACH_C0_EntryHi     # write EntryHi
        ehb
        tlbwi
        add     a0, 2<<13               # Add 8K to the address to avoid TLB conflict with previous entry

        bne     v1, zero, next_tlb_entry
        add     v1, -1

        //
        // Clear L1 instruction cache.
        //
init_icache:
        # Determine how big the I-cache is
        mfc0    v0, MACH_C0_Config1     # read Config1
        ext     v1, v0, 19, 3           # extract I-cache line size
        beq     v1, zero, done_icache   # Skip ahead if no I-cache
        nop

        mfc0    s1, MACH_C0_Config7     # Read Config7
        ext     s1, s1, 18, 1           # extract HCI
        bnez    s1, done_icache         # Skip when Hardware Cache Initialization bit set

        li      a2, 2
        sllv    v1, a2, v1              # Now have true I-cache line size in bytes

        ext     a0, v0, 22, 3           # extract IS
        li      a2, 64
        sllv    a0, a2, a0              # I-cache sets per way

        ext     a1, v0, 16, 3           # extract I-cache Assoc - 1
        add     a1, 1
        mul     a0, a0, a1              # Total number of sets
        lui     a2, 0x8000              # Get a KSeg0 address for cacheops

        mtc0    zero, MACH_C0_ITagLo    # Clear ITagLo register
        move    a3, a0

next_icache_tag:
        # Index Store Tag Cache Op
        # Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
        cache   0x8, 0(a2)              # ICIndexStTag
        add     a3, -1                  # Decrement set counter
        bne     a3, zero, next_icache_tag
        add     a2, v1                  # Get next line address
done_icache:

        //
        // Enable cacheability of kseg0 segment.
        // Until this point the code is executed from segment bfc00000,
        // (i.e. kseg1), so I-cache is not used.
        // Here we jump to kseg0 and run with I-cache enabled.
        //
enable_k0_cache:
        # Set CCA for kseg0 to cacheable.
        # NOTE! This code must be executed in KSEG1 (not KSEG0 uncached)
        mfc0    v0, MACH_C0_Config      # read Config
        li      v1, 3                   # CCA for single-core processors
        ins     v0, v1, 0, 3            # insert K0
        mtc0    v0, MACH_C0_Config      # write Config

        la      a2, init_dcache
        jr      a2                      # switch back to KSEG0
        ehb

        //
        // Initialize the L1 data cache
        //
init_dcache:
        mfc0    v0, MACH_C0_Config1     # read Config1
        ext     v1, v0, 10, 3           # extract D-cache line size
        beq     v1, zero, done_dcache   # Skip ahead if no D-cache
        nop

        mfc0    s1, MACH_C0_Config7     # Read Config7
        ext     s1, s1, 18, 1           # extract HCI
        bnez    s1, done_dcache         # Skip when Hardware Cache Initialization bit set

        li      a2, 2
        sllv    v1, a2, v1              # Now have true D-cache line size in bytes

        ext     a0, v0, 13, 3           # extract DS
        li      a2, 64
        sllv    a0, a2, a0              # D-cache sets per way

        ext     a1, v0, 7, 3            # extract D-cache Assoc - 1
        add     a1, 1
        mul     a0, a0, a1              # Get total number of sets
        lui     a2, 0x8000              # Get a KSeg0 address for cacheops

        mtc0    zero, MACH_C0_ITagLo    # Clear ITagLo/DTagLo registers
        mtc0    zero, MACH_C0_DTagLo
        move    a3, a0

next_dcache_tag:
        # Index Store Tag Cache Op
        # Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
        cache   0x9, 0(a2)              # DCIndexStTag
        add     a3, -1                  # Decrement set counter
        bne     a3, zero, next_dcache_tag
        add     a2, v1                  # Get next line address
done_dcache:

/*
 * Amount to take off of the stack for the benefit of the debugger.
 */
#define START_FRAME     ((4 * 4) + 4 + 4)

        .set    at
        la      sp, _eram - START_FRAME
        la      gp, _gp
        sw      zero, START_FRAME - 4(sp)       # Zero out old ra for debugger
        jal     mach_init                       # mach_init()
        sw      zero, START_FRAME - 8(sp)       # Zero out old fp for debugger
init_unix:
        di                                      # Disable interrupts
        li      sp, KERNELSTACK - START_FRAME   # switch to standard stack
        jal     main                            # main(frame)
        move    a0, zero
/*
 * proc[1] == /etc/init now running here.
 * Restore user registers and return.
 */
        .set    noat
enter_user_mode:
        li      v0, PSL_USERSET | MACH_Status_EXL
        mtc0    v0, MACH_C0_Status              # switch to user mode
        lw      v0, UADDR+U_PCB_REGS+(PC * 4)
        mtc0    v0, MACH_C0_EPC                 # entry address
        lw      t0, UADDR+U_PCB_REGS+(MULLO * 4)
        lw      t1, UADDR+U_PCB_REGS+(MULHI * 4)
        mtlo    t0
        mthi    t1
        lw      AT, UADDR+U_PCB_REGS+(AST * 4)
        lw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        lw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        lw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        lw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        lw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        lw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        lw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        lw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        lw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        lw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        lw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        lw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        lw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        lw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        lw      s0, UADDR+U_PCB_REGS+(S0 * 4)
        lw      s1, UADDR+U_PCB_REGS+(S1 * 4)
        lw      s2, UADDR+U_PCB_REGS+(S2 * 4)
        lw      s3, UADDR+U_PCB_REGS+(S3 * 4)
        lw      s4, UADDR+U_PCB_REGS+(S4 * 4)
        lw      s5, UADDR+U_PCB_REGS+(S5 * 4)
        lw      s6, UADDR+U_PCB_REGS+(S6 * 4)
        lw      s7, UADDR+U_PCB_REGS+(S7 * 4)
        lw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        lw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        lw      gp, UADDR+U_PCB_REGS+(GP * 4)
        lw      sp, UADDR+U_PCB_REGS+(SP * 4)   # stack pointer
        lw      s8, UADDR+U_PCB_REGS+(S8 * 4)
        lw      ra, UADDR+U_PCB_REGS+(RA * 4)
        eret
        .set    at

/*
 * GCC2 seems to want to call __main in main() for some reason.
 */
LEAF(__main)
        j       ra
        nop
END(__main)

/*
 * This code is copied the user's stack for returning from signal handlers
 * (see sendsig() and sigreturn()). We have to compute the address
 * of the sigcontext struct for the sigreturn call.
 */
        .globl  sigcode
        .type   sigcode, @function
sigcode:
        addu    a0, sp, 16              # address of sigcontext
        li      v0, SYS_sigreturn       # sigreturn(scp)
        syscall
        break   0                       # just in case sigreturn fails
        .globl  esigcode
esigcode:

/*
 * Primitives
 */

/*
 * This table is indexed by u.u_pcb.pcb_onfault in exception().
 * The reason for using this table rather than storing an address in
 * u.u_pcb.pcb_onfault is simply to make the code faster.
 */
        .globl  onfault_table
        .data
        .align  2
onfault_table:
        .word   0               # invalid index number
#define BADERR          1
        .word   baderr
#define COPYERR         2
        .word   copyerr
#define FSWBERR         3
        .word   fswberr
#define FSWINTRBERR     4
        .word   fswintrberr
        .text

/*
 * See if access to addr with a len type instruction causes a machine check.
 * len is length of access (1=byte, 2=short, 4=long)
 *
 * badaddr(addr, len)
 *      char *addr;
 *      int len;
 */
LEAF(badaddr)
        li      v0, BADERR
        bne     a1, 1, 2f
        sw      v0, UADDR+U_PCB_ONFAULT
        b       5f
        lbu     v0, (a0)
2:
        bne     a1, 2, 4f
        nop
        b       5f
        lhu     v0, (a0)
4:
        lw      v0, (a0)
5:
        sw      zero, UADDR+U_PCB_ONFAULT
        j       ra
        move    v0, zero                # made it w/o errors
baderr:
        j       ra
        li      v0, 1                   # exception sends us here
END(badaddr)

/*
 * strlen(str)
 */
LEAF(strlen)
        addu    v1, a0, 1
1:
        lb      v0, 0(a0)               # get byte from string
        bne     v0, zero, 1b            # continue if not end
        addu    a0, a0, 1               # increment pointer (delay slot)

        j       ra
        subu    v0, a0, v1              # compute length-1 for '\0' char
END(strlen)

/*
 * NOTE: this version assumes unsigned chars in order to be "8 bit clean".
 */
LEAF(strcmp)
1:
        lbu     t0, 0(a0)               # get two bytes and compare them
        lbu     t1, 0(a1)
        beq     t0, zero, LessOrEq      # end of first string?
        nop
        bne     t0, t1, NotEq
        nop
        lbu     t0, 1(a0)               # unroll loop
        lbu     t1, 1(a1)
        beq     t0, zero, LessOrEq      # end of first string?
        addu    a0, a0, 2
        beq     t0, t1, 1b
        addu    a1, a1, 2
NotEq:
        j       ra
        subu    v0, t0, t1
LessOrEq:
        j       ra
        subu    v0, zero, t1
END(strcmp)

/*
 * bzero(s1, n)
 */
LEAF(bzero)
        blt     a1, 12, smallclr        # small amount to clear?
        subu    a3, zero, a0            # compute # bytes to word align address
        and     a3, a3, 3
        beq     a3, zero, 1f            # skip if word aligned
        subu    a1, a1, a3              # subtract from remaining count
        swr     zero, 0(a0)             # clear 1, 2, or 3 bytes to align
        addu    a0, a0, a3
1:
        and     v0, a1, 3               # compute number of words left
        subu    a3, a1, v0
        move    a1, v0
        addu    a3, a3, a0              # compute ending address
2:
        addu    a0, a0, 4               # clear words
        bne     a0, a3, 2b              #  unrolling loop does not help
        sw      zero, -4(a0)            #  since we are limited by memory speed
smallclr:
        ble     a1, zero, 2f
        addu    a3, a1, a0              # compute ending address
1:
        addu    a0, a0, 1               # clear bytes
        bne     a0, a3, 1b
        sb      zero, -1(a0)
2:
        j       ra
        nop
END(bzero)

/*
 * bcmp(s1, s2, n)
 */
LEAF(bcmp)
        blt     a2, 16, smallcmp        # is it worth any trouble?
        xor     v0, a0, a1              # compare low two bits of addresses
        and     v0, v0, 3
        subu    a3, zero, a1            # compute # bytes to word align address
        bne     v0, zero, unalignedcmp  # not possible to align addresses
        and     a3, a3, 3

        beq     a3, zero, 1f
        subu    a2, a2, a3              # subtract from remaining count
        move    v0, v1                  # init v0,v1 so unmodified bytes match
        lwr     v0, 0(a0)               # read 1, 2, or 3 bytes
        lwr     v1, 0(a1)
        addu    a1, a1, a3
        bne     v0, v1, nomatch
        addu    a0, a0, a3
1:
        and     a3, a2, ~3              # compute number of whole words left
        subu    a2, a2, a3              #   which has to be >= (16-3) & ~3
        addu    a3, a3, a0              # compute ending address
2:
        lw      v0, 0(a0)               # compare words
        lw      v1, 0(a1)
        addu    a0, a0, 4
        bne     v0, v1, nomatch
        addu    a1, a1, 4
        bne     a0, a3, 2b
        nop
        b       smallcmp                # finish remainder
        nop
unalignedcmp:
        beq     a3, zero, 2f
        subu    a2, a2, a3              # subtract from remaining count
        addu    a3, a3, a0              # compute ending address
1:
        lbu     v0, 0(a0)               # compare bytes until a1 word aligned
        lbu     v1, 0(a1)
        addu    a0, a0, 1
        bne     v0, v1, nomatch
        addu    a1, a1, 1
        bne     a0, a3, 1b
        nop
2:
        and     a3, a2, ~3              # compute number of whole words left
        subu    a2, a2, a3              #   which has to be >= (16-3) & ~3
        addu    a3, a3, a0              # compute ending address
3:
        lwr     v0, 0(a0)               # compare words a0 unaligned, a1 aligned
        lwl     v0, 3(a0)
        lw      v1, 0(a1)
        addu    a0, a0, 4
        bne     v0, v1, nomatch
        addu    a1, a1, 4
        bne     a0, a3, 3b
        nop
smallcmp:
        ble     a2, zero, match
        addu    a3, a2, a0              # compute ending address
1:
        lbu     v0, 0(a0)
        lbu     v1, 0(a1)
        addu    a0, a0, 1
        bne     v0, v1, nomatch
        addu    a1, a1, 1
        bne     a0, a3, 1b
        nop
match:
        j       ra
        move    v0, zero
nomatch:
        j       ra
        li      v0, 1
END(bcmp)

/*
 * memcpy(to, from, len)
 * {ov}bcopy(from, to, len)
 */
LEAF(memcpy)
        move    v0, a0                  # swap from and to
        move    a0, a1
        move    a1, v0
ALEAF(bcopy)
ALEAF(ovbcopy)
        addu    t0, a0, a2              # t0 = end of s1 region
        sltu    t1, a1, t0
        sltu    t2, a0, a1
        and     t1, t1, t2              # t1 = true if from < to < (from+len)
        beq     t1, zero, forward       # non overlapping, do forward copy
        slt     t2, a2, 12              # check for small copy

        ble     a2, zero, 2f
        addu    t1, a1, a2              # t1 = end of to region
1:
        lb      v1, -1(t0)              # copy bytes backwards,
        subu    t0, t0, 1               #   doesnt happen often so do slow way
        subu    t1, t1, 1
        bne     t0, a0, 1b
        sb      v1, 0(t1)
2:
        j       ra
        nop
forward:
        bne     t2, zero, smallcpy      # do a small bcopy
        xor     v1, a0, a1              # compare low two bits of addresses
        and     v1, v1, 3
        subu    a3, zero, a1            # compute # bytes to word align address
        beq     v1, zero, aligned       # addresses can be word aligned
        and     a3, a3, 3

        beq     a3, zero, 1f
        subu    a2, a2, a3              # subtract from remaining count
        lwr     v1, 0(a0)               # get next 4 bytes (unaligned)
        lwl     v1, 3(a0)
        addu    a0, a0, a3
        swr     v1, 0(a1)               # store 1, 2, or 3 bytes to align a1
        addu    a1, a1, a3
1:
        and     v1, a2, 3               # compute number of words left
        subu    a3, a2, v1
        move    a2, v1
        addu    a3, a3, a0              # compute ending address
2:
        lwr     v1, 0(a0)               # copy words a0 unaligned, a1 aligned
        lwl     v1, 3(a0)
        addu    a0, a0, 4
        addu    a1, a1, 4
        bne     a0, a3, 2b
        sw      v1, -4(a1)
        b       smallcpy
        nop
aligned:
        beq     a3, zero, 1f
        subu    a2, a2, a3              # subtract from remaining count
        lwr     v1, 0(a0)               # copy 1, 2, or 3 bytes to align
        addu    a0, a0, a3
        swr     v1, 0(a1)
        addu    a1, a1, a3
1:
        and     v1, a2, 3               # compute number of whole words left
        subu    a3, a2, v1
        move    a2, v1
        addu    a3, a3, a0              # compute ending address
2:
        lw      v1, 0(a0)               # copy words
        addu    a0, a0, 4
        addu    a1, a1, 4
        bne     a0, a3, 2b
        sw      v1, -4(a1)
smallcpy:
        ble     a2, zero, 2f
        addu    a3, a2, a0              # compute ending address
1:
        lbu     v1, 0(a0)               # copy bytes
        addu    a0, a0, 1
        addu    a1, a1, 1
        bne     a0, a3, 1b
        sb      v1, -1(a1)
2:
        j       ra
        nop
END(memcpy)

/*
 * Copy a null terminated string within the kernel address space.
 * Maxlength may be null if count not wanted.
 *      copystr(fromaddr, toaddr, maxlength, &lencopied)
 *              caddr_t fromaddr;
 *              caddr_t toaddr;
 *              u_int maxlength;
 *              u_int *lencopied;
 */
LEAF(copystr)
        move    t2, a2                  # Save the number of bytes
1:
        lbu     t0, 0(a0)
        subu    a2, a2, 1
        beq     t0, zero, 2f
        sb      t0, 0(a1)
        addu    a0, a0, 1
        bne     a2, zero, 1b
        addu    a1, a1, 1
2:
        beq     a3, zero, 3f
        subu    a2, t2, a2              # compute length copied
        sw      a2, 0(a3)
3:
        j       ra
        move    v0, zero
END(copystr)

/*
 * Copy a null terminated string from the user address space into
 * the kernel address space.
 *
 *      copyinstr(fromaddr, toaddr, maxlength, &lencopied)
 *              caddr_t fromaddr;
 *              caddr_t toaddr;
 *              u_int maxlength;
 *              u_int *lencopied;
 */
NON_LEAF(copyinstr, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        sw      ra, STAND_RA_OFFSET(sp)
        blt     a0, zero, copyerr       # make sure address is in user space
        li      v0, COPYERR
        jal     copystr
        sw      v0, UADDR+U_PCB_ONFAULT
        lw      ra, STAND_RA_OFFSET(sp)
        sw      zero, UADDR+U_PCB_ONFAULT
        addu    sp, sp, STAND_FRAME_SIZE
        j       ra
        move    v0, zero
END(copyinstr)

/*
 * Copy a null terminated string from the kernel address space into
 * the user address space.
 *
 *      copyoutstr(fromaddr, toaddr, maxlength, &lencopied)
 *              caddr_t fromaddr;
 *              caddr_t toaddr;
 *              u_int maxlength;
 *              u_int *lencopied;
 */
NON_LEAF(copyoutstr, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        sw      ra, STAND_RA_OFFSET(sp)
        blt     a1, zero, copyerr       # make sure address is in user space
        li      v0, COPYERR
        jal     copystr
        sw      v0, UADDR+U_PCB_ONFAULT
        lw      ra, STAND_RA_OFFSET(sp)
        sw      zero, UADDR+U_PCB_ONFAULT
        addu    sp, sp, STAND_FRAME_SIZE
        j       ra
        move    v0, zero
END(copyoutstr)

/*
 * Copy specified amount of data from user space into the kernel
 *      copyin(from, to, len)
 *              caddr_t *from;  (user source address)
 *              caddr_t *to;    (kernel destination address)
 *              unsigned len;
 */
NON_LEAF(copyin, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        sw      ra, STAND_RA_OFFSET(sp)
        blt     a0, zero, copyerr       # make sure address is in user space
        li      v0, COPYERR
        jal     bcopy
        sw      v0, UADDR+U_PCB_ONFAULT
        lw      ra, STAND_RA_OFFSET(sp)
        sw      zero, UADDR+U_PCB_ONFAULT
        addu    sp, sp, STAND_FRAME_SIZE
        j       ra
        move    v0, zero
END(copyin)

/*
 * Copy specified amount of data from kernel to the user space
 *      copyout(from, to, len)
 *              caddr_t *from;  (kernel source address)
 *              caddr_t *to;    (user destination address)
 *              unsigned len;
 */
NON_LEAF(copyout, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        sw      ra, STAND_RA_OFFSET(sp)
        blt     a1, zero, copyerr       # make sure address is in user space
        li      v0, COPYERR
        jal     bcopy
        sw      v0, UADDR+U_PCB_ONFAULT
        lw      ra, STAND_RA_OFFSET(sp)
        sw      zero, UADDR+U_PCB_ONFAULT
        addu    sp, sp, STAND_FRAME_SIZE
        j       ra
        move    v0, zero
END(copyout)

LEAF(copyerr)
        lw      ra, STAND_RA_OFFSET(sp)
        sw      zero, UADDR+U_PCB_ONFAULT
        addu    sp, sp, STAND_FRAME_SIZE
        j       ra
        li      v0, EFAULT              # return error
END(copyerr)

/*
 * Copy data to the DMA buffer.
 * The DMA bufffer can only be written one short at a time
 * (and takes ~14 cycles).
 *
 *      CopyToBuffer(src, dst, length)
 *              u_short *src;   NOTE: must be short aligned
 *              u_short *dst;
 *              int length;
 */
LEAF(CopyToBuffer)
        blez    a2, 2f
        nop
1:
        lhu     t0, 0(a0)               # read 2 bytes of data
        subu    a2, a2, 2
        addu    a0, a0, 2
        addu    a1, a1, 4
        bgtz    a2, 1b
        sh      t0, -4(a1)              # write 2 bytes of data to buffer
2:
        j       ra
        nop
END(CopyToBuffer)

/*
 * Copy data from the DMA buffer.
 * The DMA bufffer can only be read one short at a time
 * (and takes ~12 cycles).
 *
 *      CopyFromBuffer(src, dst, length)
 *              u_short *src;
 *              char *dst;
 *              int length;
 */
LEAF(CopyFromBuffer)
        and     t0, a1, 1               # test for aligned dst
        beq     t0, zero, 3f
        nop
        blt     a2, 2, 7f               # at least 2 bytes to copy?
        nop
1:
        lhu     t0, 0(a0)               # read 2 bytes of data from buffer
        addu    a0, a0, 4               # keep buffer pointer word aligned
        addu    a1, a1, 2
        subu    a2, a2, 2
        sb      t0, -2(a1)
        srl     t0, t0, 8
        bge     a2, 2, 1b
        sb      t0, -1(a1)
3:
        blt     a2, 2, 7f               # at least 2 bytes to copy?
        nop
6:
        lhu     t0, 0(a0)               # read 2 bytes of data from buffer
        addu    a0, a0, 4               # keep buffer pointer word aligned
        addu    a1, a1, 2
        subu    a2, a2, 2
        bge     a2, 2, 6b
        sh      t0, -2(a1)
7:
        ble     a2, zero, 9f            # done?
        nop
        lhu     t0, 0(a0)               # copy one more byte
        nop
        sb      t0, 0(a1)
9:
        j       ra
        nop
END(CopyFromBuffer)

/*
 * Copy the kernel stack to the new process and save the current context so
 * the new process will return nonzero when it is resumed by cpu_switch().
 *
 *      copykstack(up)
 *              struct user *up;
 */
LEAF(copykstack)
        subu    v0, sp, UADDR           # compute offset into stack
        addu    v0, v0, a0              # v0 = new stack address
        move    v1, sp                  # v1 = old stack address
        li      t1, KERNELSTACK
1:
        lw      t0, 0(v1)               # copy stack data
        addu    v1, v1, 4
        sw      t0, 0(v0)
        bne     v1, t1, 1b
        addu    v0, v0, 4
        /* FALLTHROUGH */
/*
 * Save registers and state so we can do a longjmp later.
 * Note: this only works if p != curproc since
 * cpu_switch() will copy over pcb_context.
 *
 *      savectx(up)
 *              struct user *up;
 */
ALEAF(savectx)
        sw      s0, U_PCB_CONTEXT+0(a0)
        sw      s1, U_PCB_CONTEXT+4(a0)
        sw      s2, U_PCB_CONTEXT+8(a0)
        sw      s3, U_PCB_CONTEXT+12(a0)
        mfc0    v0, MACH_C0_Status
        sw      s4, U_PCB_CONTEXT+16(a0)
        sw      s5, U_PCB_CONTEXT+20(a0)
        sw      s6, U_PCB_CONTEXT+24(a0)
        sw      s7, U_PCB_CONTEXT+28(a0)
        sw      sp, U_PCB_CONTEXT+32(a0)
        sw      s8, U_PCB_CONTEXT+36(a0)
        sw      ra, U_PCB_CONTEXT+40(a0)
        sw      v0, U_PCB_CONTEXT+44(a0)
        j       ra
        move    v0, zero
END(copykstack)

/*
 * The following primitives manipulate the run queues.  _whichqs tells which
 * of the 32 queues _qs have processes in them.  Setrunqueue puts processes
 * into queues, Remrq removes them from queues.  The running process is on
 * no queue, other processes are on a queue related to p->p_priority, divided
 * by 4 actually to shrink the 0-127 range of priorities into the 32 available
 * queues.
 */
/*
 * setrunqueue(p)
 *      proc *p;
 *
 * Call should be made at splclock(), and p->p_stat should be SRUN.
 */
NON_LEAF(setrunqueue, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        lw      t0, P_BACK(a0)          ## firewall: p->p_back must be 0
        sw      ra, STAND_RA_OFFSET(sp) ##
        beq     t0, zero, 1f            ##
        lbu     t0, P_PRIORITY(a0)      # put on p->p_priority / 4 queue
        PANIC("setrunqueue")            ##
1:
        li      t1, 1                   # compute corresponding bit
        srl     t0, t0, 2               # compute index into 'whichqs'
        sll     t1, t1, t0
        lw      t2, whichqs             # set corresponding bit
        nop
        or      t2, t2, t1
        sw      t2, whichqs
        sll     t0, t0, 3               # compute index into 'qs'
        la      t1, qs
        addu    t0, t0, t1              # t0 = qp = &qs[pri >> 2]
        lw      t1, P_BACK(t0)          # t1 = qp->ph_rlink
        sw      t0, P_FORW(a0)          # p->p_forw = qp
        sw      t1, P_BACK(a0)          # p->p_back = qp->ph_rlink
        sw      a0, P_FORW(t1)          # p->p_back->p_forw = p;
        sw      a0, P_BACK(t0)          # qp->ph_rlink = p
        j       ra
        addu    sp, sp, STAND_FRAME_SIZE
END(setrunqueue)

/*
 * Remrq(p)
 *
 * Call should be made at splclock().
 */
NON_LEAF(remrq, STAND_FRAME_SIZE, ra)
        subu    sp, sp, STAND_FRAME_SIZE
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)
        lbu     t0, P_PRIORITY(a0)      # get from p->p_priority / 4 queue
        li      t1, 1                   # compute corresponding bit
        srl     t0, t0, 2               # compute index into 'whichqs'
        lw      t2, whichqs             # check corresponding bit
        sll     t1, t1, t0
        and     v0, t2, t1
        sw      ra, STAND_RA_OFFSET(sp) ##
        bne     v0, zero, 1f            ##
        lw      v0, P_BACK(a0)          # v0 = p->p_back
        PANIC("remrq")                  ## it wasnt recorded to be on its q
1:
        lw      v1, P_FORW(a0)          # v1 = p->p_forw
        nop
        sw      v1, P_FORW(v0)          # p->p_back->p_forw = p->p_forw;
        sw      v0, P_BACK(v1)          # p->p_forw->p_back = p->r_rlink
        sll     t0, t0, 3               # compute index into 'qs'
        la      v0, qs
        addu    t0, t0, v0              # t0 = qp = &qs[pri >> 2]
        lw      v0, P_FORW(t0)          # check if queue empty
        nop
        bne     v0, t0, 2f              # No. qp->ph_link != qp
        nop
        xor     t2, t2, t1              # clear corresponding bit in 'whichqs'
        sw      t2, whichqs
2:
        sw      zero, P_BACK(a0)        ## for firewall checking
        j       ra
        addu    sp, sp, STAND_FRAME_SIZE
END(remrq)

/*
 * switch_exit()
 *
 * At exit of a process, do a cpu_switch for the last time.
 * The mapping of the pcb at p->p_addr has already been deleted,
 * and the memory for the pcb+stack has been freed.
 * All interrupts should be blocked at this point.
 */
LEAF(switch_exit)
        la      v1, nullproc                    # save state into garbage proc
        lw      t0, P_UPTE+0(v1)                # t0 = first u. pte
        lw      t1, P_UPTE+4(v1)                # t1 = 2nd u. pte
        li      v0, UADDR                       # v0 = first HI entry
        mtc0    zero, MACH_C0_Index             # set the index register
        mtc0    v0, MACH_C0_EntryHi             # init high entry
        mtc0    t0, MACH_C0_EntryLo0            # init low entry 0
        mtc0    t1, MACH_C0_EntryLo1            # init low entry 1
        tlbwi                                   # Write the TLB entry.
        sw      zero, curproc
        b       cpu_switch
        li      sp, KERNELSTACK - START_FRAME   # switch to standard stack
END(switch_exit)

/*
 * When no processes are on the runq, cpu_switch branches to idle
 * to wait for something to come ready.
 * Note: this is really a part of cpu_switch() but defined here for kernel
 * profiling.
 */
LEAF(idle)
        li      t0, MACH_Status_IE
        di      v0                              # read Status and disable interrupts
        or      t0, v0                          # set IE
        ins     t0, zero, 10, 9                 # clear IPL
        mtc0    t0, MACH_C0_Status              # write Status: enable all interrupts
        sw      zero, curproc                   # set curproc NULL for stats
1:
        wait
        lw      t0, whichqs                     # look for non-empty queue
        beq     t0, zero, 1b
        nop
        b       sw1
        mtc0    v0, MACH_C0_Status              # Restore interrupt mask
END(idle)

/*
 * cpu_switch()
 * Find the highest priority process and resume it.
 */
NON_LEAF(cpu_switch, STAND_FRAME_SIZE, ra)
        sw      sp, UADDR+U_PCB_CONTEXT+32      # save old sp
        subu    sp, sp, STAND_FRAME_SIZE
        sw      ra, STAND_RA_OFFSET(sp)
        .mask   0x80000000, (STAND_RA_OFFSET - STAND_FRAME_SIZE)

        lw      t2, cnt+V_SWTCH                 # for statistics
        lw      t1, whichqs                     # look for non-empty queue
        sw      s0, UADDR+U_PCB_CONTEXT+0       # do a 'savectx()'
        sw      s1, UADDR+U_PCB_CONTEXT+4
        sw      s2, UADDR+U_PCB_CONTEXT+8
        sw      s3, UADDR+U_PCB_CONTEXT+12
        mfc0    t0, MACH_C0_Status              # t0 = saved status register
        sw      s4, UADDR+U_PCB_CONTEXT+16
        sw      s5, UADDR+U_PCB_CONTEXT+20
        sw      s6, UADDR+U_PCB_CONTEXT+24
        sw      s7, UADDR+U_PCB_CONTEXT+28
        sw      s8, UADDR+U_PCB_CONTEXT+36
        sw      ra, UADDR+U_PCB_CONTEXT+40      # save return address
        sw      t0, UADDR+U_PCB_CONTEXT+44      # save status register
        addu    t2, t2, 1
        sw      t2, cnt+V_SWTCH
        beq     t1, zero, idle                  # if none, idle
        mtc0    zero, MACH_C0_Status            # Disable all interrupts
sw1:
        lw      t0, whichqs                     # look for non-empty queue
        li      t2, -1                          # t2 = lowest bit set
        beq     t0, zero, idle                  # if none, idle
        move    t3, t0                          # t3 = saved whichqs
1:
        addu    t2, t2, 1
        and     t1, t0, 1                       # bit set?
        beq     t1, zero, 1b
        srl     t0, t0, 1                       # try next bit
/*
 * Remove process from queue.
 */
        sll     t0, t2, 3
        la      t1, qs
        addu    t0, t0, t1                      # t0 = qp = &qs[highbit]
        lw      a0, P_FORW(t0)                  # a0 = p = highest pri process
        nop
        lw      v0, P_FORW(a0)                  # v0 = p->p_forw
        bne     t0, a0, 2f                      # make sure something in queue
        sw      v0, P_FORW(t0)                  # qp->ph_link = p->p_forw;
        PANIC("cpu_switch")                     # nothing in queue
2:
        sw      t0, P_BACK(v0)                  # p->p_forw->p_back = qp
        bne     v0, t0, 3f                      # queue still not empty
        sw      zero, P_BACK(a0)                ## for firewall checking
        li      v1, 1                           # compute bit in 'whichqs'
        sll     v1, v1, t2
        xor     t3, t3, v1                      # clear bit in 'whichqs'
        sw      t3, whichqs
3:
/*
 * Switch to new context.
 */
        sw      zero, want_resched
        jal     pmap_alloc_tlbpid               # v0 = TLB PID
        move    s0, a0                          # BDSLOT: save p
        sw      s0, curproc                     # set curproc
        lw      t0, P_UPTE+0(s0)                # t0 = first u. pte
        lw      t1, P_UPTE+4(s0)                # t1 = 2nd u. pte
        or      v0, v0, UADDR                   # v0 = first HI entry
/*
 * Resume process indicated by the pte's for its u struct
 * NOTE: This is hard coded to UPAGES == 2.
 * Also, there should be no TLB faults at this point.
 */
        mtc0    zero, MACH_C0_Index             # set the index register
        mtc0    v0, MACH_C0_EntryHi             # init high entry
        mtc0    t0, MACH_C0_EntryLo0            # init low entry 0
        mtc0    t1, MACH_C0_EntryLo1            # init low entry 1
        tlbwi                                   # Write the TLB entry.
/*
 * Now running on new u struct.
 * Restore registers and return.
 */
        lw      v0, UADDR+U_PCB_CONTEXT+44      # restore kernel context
        lw      ra, UADDR+U_PCB_CONTEXT+40
        lw      s0, UADDR+U_PCB_CONTEXT+0
        lw      s1, UADDR+U_PCB_CONTEXT+4
        lw      s2, UADDR+U_PCB_CONTEXT+8
        lw      s3, UADDR+U_PCB_CONTEXT+12
        lw      s4, UADDR+U_PCB_CONTEXT+16
        lw      s5, UADDR+U_PCB_CONTEXT+20
        lw      s6, UADDR+U_PCB_CONTEXT+24
        lw      s7, UADDR+U_PCB_CONTEXT+28
        lw      sp, UADDR+U_PCB_CONTEXT+32
        lw      s8, UADDR+U_PCB_CONTEXT+36
        mtc0    v0, MACH_C0_Status
        j       ra
        li      v0, 1                           # possible return to 'savectx()'
END(cpu_switch)

/*
 * {fu,su},{ibyte,isword,iword}, fetch or store a byte, short or word to
 * user text space.
 * {fu,su},{byte,sword,word}, fetch or store a byte, short or word to
 * user data space.
 */
LEAF(fuword)
ALEAF(fuiword)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        lw      v0, 0(a0)               # fetch word
        j       ra
        sw      zero, UADDR+U_PCB_ONFAULT
END(fuword)

LEAF(fusword)
ALEAF(fuisword)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        lhu     v0, 0(a0)               # fetch short
        j       ra
        sw      zero, UADDR+U_PCB_ONFAULT
END(fusword)

LEAF(fubyte)
ALEAF(fuibyte)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        lbu     v0, 0(a0)               # fetch byte
        j       ra
        sw      zero, UADDR+U_PCB_ONFAULT
END(fubyte)

LEAF(suword)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        sw      a1, 0(a0)               # store word
        sw      zero, UADDR+U_PCB_ONFAULT
        j       ra
        move    v0, zero
END(suword)

/*
 * Have to flush instruction cache afterwards.
 */
LEAF(suiword)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        sw      a1, 0(a0)               # store word
        sw      zero, UADDR+U_PCB_ONFAULT
        move    v0, zero
        j       mips_flush_icache       # NOTE: this should not clobber v0!
        li      a1, 4                   # size of word
END(suiword)

/*
 * Will have to flush the instruction cache if byte merging is done in hardware.
 */
LEAF(susword)
ALEAF(suisword)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        sh      a1, 0(a0)               # store short
        sw      zero, UADDR+U_PCB_ONFAULT
        j       ra
        move    v0, zero
END(susword)

LEAF(subyte)
ALEAF(suibyte)
        blt     a0, zero, fswberr       # make sure address is in user space
        li      v0, FSWBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        sb      a1, 0(a0)               # store byte
        sw      zero, UADDR+U_PCB_ONFAULT
        j       ra
        move    v0, zero
END(subyte)

LEAF(fswberr)
        j       ra
        li      v0, -1
END(fswberr)

/*
 * fuswintr and suswintr are just like fusword and susword except that if
 * the page is not in memory or would cause an exception, then we return an error.
 * The important thing is to prevent sleep() and switch().
 */
LEAF(fuswintr)
        blt     a0, zero, fswintrberr   # make sure address is in user space
        li      v0, FSWINTRBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        lhu     v0, 0(a0)               # fetch short
        j       ra
        sw      zero, UADDR+U_PCB_ONFAULT
END(fuswintr)

LEAF(suswintr)
        blt     a0, zero, fswintrberr   # make sure address is in user space
        li      v0, FSWINTRBERR
        sw      v0, UADDR+U_PCB_ONFAULT
        sh      a1, 0(a0)               # store short
        sw      zero, UADDR+U_PCB_ONFAULT
        j       ra
        move    v0, zero
END(suswintr)

LEAF(fswintrberr)
        j       ra
        li      v0, -1
END(fswintrberr)

/*-----------------------------------
 * Exception handlers and data for bootloader.
 */
        .section .exception
        .set    noat
/*
 * TLB exception vector: handle TLB translation misses.
 * The BaddVAddr, Context, and EntryHi registers contain the failed
 * virtual address.
 */
        .org    0
        .globl  _tlb_vector
_tlb_vector:
        .type   _tlb_vector, @function
        mfc0    k0, MACH_C0_Status              # Get the status register
        and     k0, MACH_Status_UM              # test for user mode
        beqz    k0, kern_tlb_refill
        nop

user_tlb_refill:
        mfc0    k0, MACH_C0_BadVAddr            # get the virtual address
        srl     k0, SEGSHIFT                    # compute segment table index
        sll     k0, 2
        lw      k1, UADDR+U_PCB_SEGTAB          # get the current segment table
        addu    k1, k0
        lw      k1, 0(k1)                       # get pointer to segment map
        beqz    k1, user_exception              # invalid segment map
        mfc0    k0, MACH_C0_BadVAddr            # get the virtual address
        srl     k0, PGSHIFT - 2                 # compute segment map index
        andi    k0, (NPTEPG - 2) << 2           # make even
        addu    k1, k0                          # index into segment map
        lw      k0, 0(k1)                       # get even page PTE
        lw      k1, 4(k1)                       # get odd page PTE
        mtc0    k0, MACH_C0_EntryLo0
        mtc0    k1, MACH_C0_EntryLo1
        tlbwr                                   # update TLB
        eret
/*
 * Handle a TLB miss exception from kernel mode.
 */
kern_tlb_refill:
        mfc0    k0, MACH_C0_BadVAddr            # get the fault address
        li      k1, 0xc0000000                  # VM_MIN_KERNEL_ADDRESS
        subu    k0, k1                          # compute index
        srl     k0, PGSHIFT
        lw      k1, Sysmapsize                  # index within range?
        sltu    k1, k0, k1
        beqz    k1, check_stack                 # No. check for valid stack
        andi    k1, k0, 1                       # check for odd page
        bnez    k1, odd_page
        sll     k0, 2                           # compute offset from index
even_page:
        lw      k1, Sysmap
        addu    k1, k0
        lw      k0, 0(k1)                       # get even page PTE
        lw      k1, 4(k1)                       # get odd page PTE
        mtc0    k0, MACH_C0_EntryLo0            # save PTE entry
        and     k0, PG_V                        # check for valid entry
        beqz    k0, kern_exception              # PTE invalid
        mtc0    k1, MACH_C0_EntryLo1
        tlbwr                                   # update TLB
        eret
odd_page:
        lw      k1, Sysmap
        addu    k1, k0
        lw      k0, -4(k1)                      # get even page PTE
        lw      k1, 0(k1)                       # get odd page PTE
        mtc0    k1, MACH_C0_EntryLo1
        and     k1, PG_V                        # check for valid entry
        beqz    k1, kern_exception              # PTE invalid
        mtc0    k0, MACH_C0_EntryLo0            # save PTE entry
        tlbwr                                   # update TLB
        eret

/*
 * Data for bootloader.
 */
        .org    0xf8
        .type   _ebase, @object
_ebase:
        .word   _tlb_vector                     # EBase value

        .type   _imgptr, @object
_imgptr:
        .word   -1                              # Image header pointer

check_stack:
        subu    k0, sp, UADDR + 0x200           # check to see if we have a
        sltiu   k0, UPAGES*NBPG - 0x200         #  valid kernel stack
        bnez    k0, kern_exception              # Go panic
        nop

        la      a0, _eram - START_FRAME - 8     # set sp to a valid place
        sw      sp, 24(a0)
        move    sp, a0
        la      gp, _gp
        la      a0, 9f
        MSG("kern_tlb_refill: EPC %x, RA %x, BadVAddr %x, Status %x, Cause %x, SP %x\n")
        mfc0    a2, MACH_C0_Status
        mfc0    a3, MACH_C0_Cause
        mfc0    a1, MACH_C0_EPC
        sw      a2, 16(sp)
        sw      a3, 20(sp)
        move    a2, ra
        jal     printf
        mfc0    a3, MACH_C0_BadVAddr

        la      sp, _eram - START_FRAME         # set sp to a valid place
        PANIC("kernel stack overflow")

/*
 * General exception vector address:
 * handle all execptions except RESET and TLBMiss.
 * Find out what mode we came from and jump to the proper handler.
 */
        .org    0x180
_exception_vector:
        .type   _exception_vector, @function
        mfc0    k0, MACH_C0_Status              # Get the status register
        and     k0, MACH_Status_UM              # test for user mode
        bnez    k0, user_exception
        nop
        j       kern_exception
        nop

/*
 * General interrupt vector address.
 * Find out what mode we came from and jump to the proper handler.
 */
        .org    0x200
_interrupt_vector:
        .type   _interrupt_vector, @function
        mfc0    k0, MACH_C0_Status              # Get the status register
        and     k0, MACH_Status_UM              # test for user mode
        bnez    k0, user_interrupt
        nop
        j       kern_interrupt
        nop

/*----------------------------------------------------------------------------
 * Handle an exception from kernel mode.
 *
 * The kernel exception stack contains 18 saved general registers,
 * the status register and the multiply lo and high registers.
 * In addition, we set this up for linkage conventions.
 */
#define KERN_REG_SIZE           (18 * 4)
#define KERN_REG_OFFSET         (STAND_FRAME_SIZE)
#define KERN_SR_OFFSET          (STAND_FRAME_SIZE + KERN_REG_SIZE)
#define KERN_MULT_LO_OFFSET     (STAND_FRAME_SIZE + KERN_REG_SIZE + 4)
#define KERN_MULT_HI_OFFSET     (STAND_FRAME_SIZE + KERN_REG_SIZE + 8)
#define KERN_EXC_FRAME_SIZE     (STAND_FRAME_SIZE + KERN_REG_SIZE + 12)

kern_exception:
        subu    sp, sp, KERN_EXC_FRAME_SIZE
        la      gp, _gp                         # switch to kernel GP

        //TODO: in case of MCheck, set up SP to _eram-16.
/*
 * Save the relevant kernel registers onto the stack.
 * We do not need to save s0 - s8, sp and gp because
 * the compiler does it for us.
 */
        sw      AT, KERN_REG_OFFSET + 0(sp)
        sw      v0, KERN_REG_OFFSET + 4(sp)
        sw      v1, KERN_REG_OFFSET + 8(sp)
        sw      a0, KERN_REG_OFFSET + 12(sp)
        mflo    v0
        mfhi    v1
        sw      a1, KERN_REG_OFFSET + 16(sp)
        sw      a2, KERN_REG_OFFSET + 20(sp)
        sw      a3, KERN_REG_OFFSET + 24(sp)
        sw      t0, KERN_REG_OFFSET + 28(sp)
        mfc0    a0, MACH_C0_Status              # First arg is the status reg.
        sw      t1, KERN_REG_OFFSET + 32(sp)
        sw      t2, KERN_REG_OFFSET + 36(sp)
        sw      t3, KERN_REG_OFFSET + 40(sp)
        sw      t4, KERN_REG_OFFSET + 44(sp)
        mfc0    a1, MACH_C0_Cause               # Second arg is the cause reg.
        sw      t5, KERN_REG_OFFSET + 48(sp)
        sw      t6, KERN_REG_OFFSET + 52(sp)
        sw      t7, KERN_REG_OFFSET + 56(sp)
        sw      t8, KERN_REG_OFFSET + 60(sp)
        mfc0    a2, MACH_C0_BadVAddr            # Third arg is the fault addr.
        sw      t9, KERN_REG_OFFSET + 64(sp)
        sw      ra, KERN_REG_OFFSET + 68(sp)
        sw      v0, KERN_MULT_LO_OFFSET(sp)
        sw      v1, KERN_MULT_HI_OFFSET(sp)
        mfc0    a3, MACH_C0_EPC                 # Fourth arg is the pc.
        sw      a0, KERN_SR_OFFSET(sp)

        move    t0, a0
        ins     t0, zero, 0, 5                  # Clear UM, EXL, IE.
        mtc0    t0, MACH_C0_Status              # Set Status, interrupts still disabled.
/*
 * Call the exception handler.
 */
        jal     exception
        sw      a3, STAND_RA_OFFSET(sp)         # for debugging
/*
 * Restore registers and return from the exception.
 * v0 contains the return address.
 */
        lw      t0, KERN_MULT_LO_OFFSET(sp)
        lw      t1, KERN_MULT_HI_OFFSET(sp)
        mtlo    t0
        mthi    t1
        lw      AT, KERN_REG_OFFSET + 0(sp)
        lw      v1, KERN_REG_OFFSET + 8(sp)
        lw      a0, KERN_REG_OFFSET + 12(sp)
        lw      a1, KERN_REG_OFFSET + 16(sp)
        lw      a2, KERN_REG_OFFSET + 20(sp)
        lw      a3, KERN_REG_OFFSET + 24(sp)
        lw      t0, KERN_REG_OFFSET + 28(sp)
        lw      t1, KERN_REG_OFFSET + 32(sp)
        lw      t2, KERN_REG_OFFSET + 36(sp)
        lw      t3, KERN_REG_OFFSET + 40(sp)
        lw      t4, KERN_REG_OFFSET + 44(sp)
        lw      t5, KERN_REG_OFFSET + 48(sp)
        lw      t6, KERN_REG_OFFSET + 52(sp)
        lw      t7, KERN_REG_OFFSET + 56(sp)
        lw      t8, KERN_REG_OFFSET + 60(sp)
        lw      t9, KERN_REG_OFFSET + 64(sp)
        lw      ra, KERN_REG_OFFSET + 68(sp)
        di                                      # Disable interrupts.
        mtc0    v0, MACH_C0_EPC                 # Restore EPC.
        lw      k0, KERN_SR_OFFSET(sp)
        mtc0    k0, MACH_C0_Status              # Restore Status.
        lw      v0, KERN_REG_OFFSET + 4(sp)
        addu    sp, sp, KERN_EXC_FRAME_SIZE
        eret                                    # Return from exception, enable intrs.

/*----------------------------------------------------------------------------
 * Handle an exception from user mode.
 * Save all of the registers except for the kernel temporaries in u.u_pcb.
 */
user_exception:
        sw      AT, UADDR+U_PCB_REGS+(AST * 4)
        sw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        sw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        sw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        mflo    v0
        sw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        sw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        sw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        sw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        mfhi    v1
        sw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        sw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        sw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        sw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        mfc0    a0, MACH_C0_Status              # First arg is the status reg.
        sw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        sw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        sw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        sw      s0, UADDR+U_PCB_REGS+(S0 * 4)
        mfc0    a1, MACH_C0_Cause               # Second arg is the cause reg.
        sw      s1, UADDR+U_PCB_REGS+(S1 * 4)
        sw      s2, UADDR+U_PCB_REGS+(S2 * 4)
        sw      s3, UADDR+U_PCB_REGS+(S3 * 4)
        sw      s4, UADDR+U_PCB_REGS+(S4 * 4)
        mfc0    a2, MACH_C0_BadVAddr            # Third arg is the fault addr
        sw      s5, UADDR+U_PCB_REGS+(S5 * 4)
        sw      s6, UADDR+U_PCB_REGS+(S6 * 4)
        sw      s7, UADDR+U_PCB_REGS+(S7 * 4)
        sw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        mfc0    a3, MACH_C0_EPC                 # Fourth arg is the pc.
        sw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        sw      gp, UADDR+U_PCB_REGS+(GP * 4)
        sw      sp, UADDR+U_PCB_REGS+(SP * 4)
        sw      s8, UADDR+U_PCB_REGS+(S8 * 4)
        li      sp, KERNELSTACK - STAND_FRAME_SIZE      # switch to kernel SP
        sw      ra, UADDR+U_PCB_REGS+(RA * 4)
        sw      v0, UADDR+U_PCB_REGS+(MULLO * 4)
        sw      v1, UADDR+U_PCB_REGS+(MULHI * 4)
        sw      a0, UADDR+U_PCB_REGS+(SR * 4)
        sw      a3, UADDR+U_PCB_REGS+(PC * 4)
        la      gp, _gp                         # switch to kernel GP
        move    t0, a0
        ins     t0, zero, 0, 5                  # Clear UM, EXL, IE.
        mtc0    t0, MACH_C0_Status              # Set Status, interrupts still disabled.
/*
 * Call the exception handler.
 */
        jal     exception
        sw      a3, STAND_RA_OFFSET(sp)         # for debugging
/*
 * Restore user registers and return. NOTE: interrupts are enabled.
 */
        lw      t0, UADDR+U_PCB_REGS+(MULLO * 4)
        lw      t1, UADDR+U_PCB_REGS+(MULHI * 4)
        mtlo    t0
        mthi    t1
        lw      AT, UADDR+U_PCB_REGS+(AST * 4)
        lw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        lw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        lw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        lw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        lw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        lw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        lw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        lw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        lw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        lw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        lw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        lw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        lw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        lw      s0, UADDR+U_PCB_REGS+(S0 * 4)
        lw      s1, UADDR+U_PCB_REGS+(S1 * 4)
        lw      s2, UADDR+U_PCB_REGS+(S2 * 4)
        lw      s3, UADDR+U_PCB_REGS+(S3 * 4)
        lw      s4, UADDR+U_PCB_REGS+(S4 * 4)
        lw      s5, UADDR+U_PCB_REGS+(S5 * 4)
        lw      s6, UADDR+U_PCB_REGS+(S6 * 4)
        lw      s7, UADDR+U_PCB_REGS+(S7 * 4)
        lw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        lw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        lw      s8, UADDR+U_PCB_REGS+(S8 * 4)
        lw      ra, UADDR+U_PCB_REGS+(RA * 4)
        di                                      # Disable interrupts.
        lw      gp, UADDR+U_PCB_REGS+(GP * 4)
        mtc0    v0, MACH_C0_EPC                 # Restore EPC.
        lw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        lw      k0, UADDR+U_PCB_REGS+(SR * 4)
        mtc0    k0, MACH_C0_Status              # Restore Status.
        lw      sp, UADDR+U_PCB_REGS+(SP * 4)
        eret

/*----------------------------------------------------------------------------
 * Handle an interrupt from kernel mode.
 * Interrupts use the standard kernel stack.
 * switch_exit sets up a kernel stack after exit so interrupts would not fail.
 *
 *      kern_interrupt()
 */
#define KINTR_REG_OFFSET        (STAND_FRAME_SIZE)
#define KINTR_SR_OFFSET         (STAND_FRAME_SIZE + KERN_REG_SIZE)
#define KINTR_MULT_LO_OFFSET    (STAND_FRAME_SIZE + KERN_REG_SIZE + 4)
#define KINTR_MULT_HI_OFFSET    (STAND_FRAME_SIZE + KERN_REG_SIZE + 8)
#define KINTR_FRAME_SIZE        (STAND_FRAME_SIZE + KERN_REG_SIZE + 12)

kern_interrupt:
        subu    sp, sp, KINTR_FRAME_SIZE        # allocate stack frame
        la      gp, _gp                         # switch to kernel GP
        sw      a0, KINTR_REG_OFFSET + 12(sp)
        mfc0    a0, MACH_C0_Status              # First arg is the status reg.
        sw      a1, KINTR_REG_OFFSET + 16(sp)
        mfc0    a1, MACH_C0_EPC                 # Second arg is the pc.

        mfc0    k0, MACH_C0_Cause               # Get Cause.
        ext     k1, k0, 10, 6                   # Extract Cause.IPL.
        move    k0, a0
#TODO: something wrong with nested interrupts
//li k1, 7
        ins     k0, k1, 10, 6                   # Raise Status.IPL,
        ins     k0, zero, 1, 1                  # Clear EXL.
        mtc0    k0, MACH_C0_Status              # Set Status, re-enable interrupts.

/*
 * Save the relevant kernel registers onto the stack.
 * We do not need to save s0 - s8, sp and gp because
 * the compiler does it for us.
 */
        sw      a0, KINTR_SR_OFFSET(sp)
        sw      AT, KINTR_REG_OFFSET + 0(sp)
        sw      v0, KINTR_REG_OFFSET + 4(sp)
        sw      v1, KINTR_REG_OFFSET + 8(sp)
        mflo    v0
        mfhi    v1
        sw      a2, KINTR_REG_OFFSET + 20(sp)
        sw      a3, KINTR_REG_OFFSET + 24(sp)
        sw      t0, KINTR_REG_OFFSET + 28(sp)
        sw      t1, KINTR_REG_OFFSET + 32(sp)
        sw      t2, KINTR_REG_OFFSET + 36(sp)
        sw      t3, KINTR_REG_OFFSET + 40(sp)
        sw      t4, KINTR_REG_OFFSET + 44(sp)
        sw      t5, KINTR_REG_OFFSET + 48(sp)
        sw      t6, KINTR_REG_OFFSET + 52(sp)
        sw      t7, KINTR_REG_OFFSET + 56(sp)
        sw      t8, KINTR_REG_OFFSET + 60(sp)
        sw      t9, KINTR_REG_OFFSET + 64(sp)
        sw      ra, KINTR_REG_OFFSET + 68(sp)
        sw      v0, KINTR_MULT_LO_OFFSET(sp)
        sw      v1, KINTR_MULT_HI_OFFSET(sp)
/*
 * Call the interrupt handler.
 */
        jal     interrupt
        sw      a1, STAND_RA_OFFSET(sp)         # for debugging
/*
 * Restore registers and return from the interrupt.
 */
        lw      t0, KINTR_MULT_LO_OFFSET(sp)
        lw      t1, KINTR_MULT_HI_OFFSET(sp)
        mtlo    t0
        mthi    t1
        lw      AT, KINTR_REG_OFFSET + 0(sp)
        lw      v0, KINTR_REG_OFFSET + 4(sp)
        lw      v1, KINTR_REG_OFFSET + 8(sp)
        lw      a0, KINTR_REG_OFFSET + 12(sp)
        lw      a1, KINTR_REG_OFFSET + 16(sp)
        lw      a2, KINTR_REG_OFFSET + 20(sp)
        lw      a3, KINTR_REG_OFFSET + 24(sp)
        lw      t0, KINTR_REG_OFFSET + 28(sp)
        lw      t1, KINTR_REG_OFFSET + 32(sp)
        lw      t2, KINTR_REG_OFFSET + 36(sp)
        lw      t3, KINTR_REG_OFFSET + 40(sp)
        lw      t4, KINTR_REG_OFFSET + 44(sp)
        lw      t5, KINTR_REG_OFFSET + 48(sp)
        lw      t6, KINTR_REG_OFFSET + 52(sp)
        lw      t7, KINTR_REG_OFFSET + 56(sp)
        lw      t8, KINTR_REG_OFFSET + 60(sp)
        lw      t9, KINTR_REG_OFFSET + 64(sp)
        lw      ra, KINTR_REG_OFFSET + 68(sp)
        di                                      # Disable interrupts.
        lw      k0, KINTR_SR_OFFSET(sp)
        mtc0    k0, MACH_C0_Status              # Restore Status
        lw      k1, STAND_RA_OFFSET(sp)
        mtc0    k1, MACH_C0_EPC                 # Restore EPC.
        addu    sp, sp, KINTR_FRAME_SIZE
        eret                                    # Return from interrupt.

/*----------------------------------------------------------------------------
 * Handle an interrupt from user mode.
 * Note: we save minimal state in the u.u_pcb struct and use the standard
 * kernel stack since there has to be a u page if we came from user mode.
 * If there is a pending software interrupt, then save the remaining state
 * and call softintr(). This is all because if we call switch() inside
 * interrupt(), not all the user registers have been saved in u.u_pcb.
 */
user_interrupt:
        sw      sp, UADDR+U_PCB_REGS+(SP * 4)
        li      sp, KERNELSTACK - STAND_FRAME_SIZE      # switch to kernel SP
        sw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        mfc0    a0, MACH_C0_Status              # First arg is the status reg.
        sw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        mfc0    a1, MACH_C0_EPC                 # Second arg is the pc.

        sw      gp, UADDR+U_PCB_REGS+(GP * 4)
        la      gp, _gp                         # switch to kernel GP

        mfc0    k0, MACH_C0_Cause               # Get Cause.
        ext     k1, k0, 10, 6                   # Extract Cause.IPL.
        move    k0, a0
#TODO: something wrong with nested interrupts
//li k1, 7
        ins     k0, k1, 10, 6                   # Raise Status.IPL,
        ins     k0, zero, 1, 4                  # Clear UM and EXL.
        mtc0    k0, MACH_C0_Status              # Set Status, re-enable interrupts.

/*
 * Save the relevant user registers into the u.u_pcb struct.
 * We do not need to save s0 - s8 because
 * the compiler does it for us.
 */
        sw      a0, UADDR+U_PCB_REGS+(SR * 4)
        sw      a1, UADDR+U_PCB_REGS+(PC * 4)
        sw      AT, UADDR+U_PCB_REGS+(AST * 4)
        sw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        sw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        mflo    v0
        mfhi    v1
        sw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        sw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        sw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        sw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        sw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        sw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        sw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        sw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        sw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        sw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        sw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        sw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        sw      ra, UADDR+U_PCB_REGS+(RA * 4)
        sw      v0, UADDR+U_PCB_REGS+(MULLO * 4)
        sw      v1, UADDR+U_PCB_REGS+(MULHI * 4)
/*
 * Call the interrupt handler.
 */
        jal     interrupt
        sw      a1, STAND_RA_OFFSET(sp)         # for debugging
/*
 * Restore registers and return from the interrupt.
 */
        lw      a0, UADDR+U_PCB_REGS+(SR * 4)
        mtc0    a0, MACH_C0_Status              # Restore the SR, disable intrs
        ehb
        lw      v0, astpending                  # any pending interrupts?
        bne     v0, zero, soft_interrupt        # dont restore, call softintr
        lw      t0, UADDR+U_PCB_REGS+(MULLO * 4)
        lw      t1, UADDR+U_PCB_REGS+(MULHI * 4)
        lw      AT, UADDR+U_PCB_REGS+(AST * 4)
        lw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        lw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        lw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        lw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        lw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        lw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        mtlo    t0
        mthi    t1
        lw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        lw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        lw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        lw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        lw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        lw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        lw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        lw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        lw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        lw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        lw      gp, UADDR+U_PCB_REGS+(GP * 4)
        lw      sp, UADDR+U_PCB_REGS+(SP * 4)
        lw      ra, UADDR+U_PCB_REGS+(RA * 4)
        lw      k0, UADDR+U_PCB_REGS+(PC * 4)
        mtc0    k0, MACH_C0_EPC                 # Restore EPC.
        eret                                    # Return from the interrupt.

/*
 * We have pending software interrupts; save remaining user state in u.u_pcb.
 */
soft_interrupt:
        sw      s0, UADDR+U_PCB_REGS+(S0 * 4)
        sw      s1, UADDR+U_PCB_REGS+(S1 * 4)
        sw      s2, UADDR+U_PCB_REGS+(S2 * 4)
        sw      s3, UADDR+U_PCB_REGS+(S3 * 4)
        sw      s4, UADDR+U_PCB_REGS+(S4 * 4)
        sw      s5, UADDR+U_PCB_REGS+(S5 * 4)
        sw      s6, UADDR+U_PCB_REGS+(S6 * 4)
        sw      s7, UADDR+U_PCB_REGS+(S7 * 4)
        sw      s8, UADDR+U_PCB_REGS+(S8 * 4)
/*
 * Call the software interrupt handler.
 */
        li      t1, 2                           # new IPL value
        ins     a0, t1, 10, 9                   # set IPL
        ins     a0, zero, 1, 4                  # clear UM and EXL
        jal     softintr
        mtc0    a0, MACH_C0_Status              # enable interrupts (spl2)
/*
 * Restore user registers and return. NOTE: interrupts are enabled.
 */
        lw      t0, UADDR+U_PCB_REGS+(MULLO * 4)
        lw      t1, UADDR+U_PCB_REGS+(MULHI * 4)
        mtlo    t0
        mthi    t1
        lw      AT, UADDR+U_PCB_REGS+(AST * 4)
        lw      v0, UADDR+U_PCB_REGS+(V0 * 4)
        lw      v1, UADDR+U_PCB_REGS+(V1 * 4)
        lw      a0, UADDR+U_PCB_REGS+(A0 * 4)
        lw      a1, UADDR+U_PCB_REGS+(A1 * 4)
        lw      a2, UADDR+U_PCB_REGS+(A2 * 4)
        lw      a3, UADDR+U_PCB_REGS+(A3 * 4)
        lw      t0, UADDR+U_PCB_REGS+(T0 * 4)
        lw      t1, UADDR+U_PCB_REGS+(T1 * 4)
        lw      t2, UADDR+U_PCB_REGS+(T2 * 4)
        lw      t3, UADDR+U_PCB_REGS+(T3 * 4)
        lw      t4, UADDR+U_PCB_REGS+(T4 * 4)
        lw      t5, UADDR+U_PCB_REGS+(T5 * 4)
        lw      t6, UADDR+U_PCB_REGS+(T6 * 4)
        lw      t7, UADDR+U_PCB_REGS+(T7 * 4)
        lw      s0, UADDR+U_PCB_REGS+(S0 * 4)
        lw      s1, UADDR+U_PCB_REGS+(S1 * 4)
        lw      s2, UADDR+U_PCB_REGS+(S2 * 4)
        lw      s3, UADDR+U_PCB_REGS+(S3 * 4)
        lw      s4, UADDR+U_PCB_REGS+(S4 * 4)
        lw      s5, UADDR+U_PCB_REGS+(S5 * 4)
        lw      s6, UADDR+U_PCB_REGS+(S6 * 4)
        lw      s7, UADDR+U_PCB_REGS+(S7 * 4)
        lw      t8, UADDR+U_PCB_REGS+(T8 * 4)
        lw      t9, UADDR+U_PCB_REGS+(T9 * 4)
        lw      s8, UADDR+U_PCB_REGS+(S8 * 4)
        lw      ra, UADDR+U_PCB_REGS+(RA * 4)
        di                                      # Disable interrupts.
        lw      gp, UADDR+U_PCB_REGS+(GP * 4)
        lw      sp, UADDR+U_PCB_REGS+(SP * 4)
        lw      k0, UADDR+U_PCB_REGS+(SR * 4)
        mtc0    k0, MACH_C0_Status              # Restore Status.
        lw      k1, UADDR+U_PCB_REGS+(PC * 4)
        mtc0    k1, MACH_C0_EPC                 # Restore EPC.
        eret                                    # Return from the interrupt.
