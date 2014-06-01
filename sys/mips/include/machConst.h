/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)machConst.h	8.1 (Berkeley) 6/10/93
 *
 * machConst.h --
 *
 *	Machine dependent constants.
 *
 *	Copyright (C) 1989 Digital Equipment Corporation.
 *	Permission to use, copy, modify, and distribute this software and
 *	its documentation for any purpose and without fee is hereby granted,
 *	provided that the above copyright notice appears in all copies.
 *	Digital Equipment Corporation makes no representations about the
 *	suitability of this software for any purpose.  It is provided "as is"
 *	without express or implied warranty.
 *
 * from: $Header: /sprite/src/kernel/mach/ds3100.md/RCS/machConst.h,
 *	v 9.2 89/10/21 15:55:22 jhh Exp $ SPRITE (DECWRL)
 * from: $Header: /sprite/src/kernel/mach/ds3100.md/RCS/machAddrs.h,
 *	v 1.2 89/08/15 18:28:21 rab Exp $ SPRITE (DECWRL)
 * from: $Header: /sprite/src/kernel/vm/ds3100.md/RCS/vmPmaxConst.h,
 *	v 9.1 89/09/18 17:33:00 shirriff Exp $ SPRITE (DECWRL)
 */

#ifndef _MACHCONST
#define _MACHCONST

#define MACH_KUSEG_ADDR			0x0
#define MACH_CACHED_MEMORY_ADDR		0x80000000
#define MACH_UNCACHED_MEMORY_ADDR	0xa0000000
#define MACH_KSEG2_ADDR			0xc0000000
#define MACH_MAX_MEM_ADDR		0xbe000000
#define	MACH_RESERVED_ADDR		0xbfc80000

#define	MACH_CACHED_TO_PHYS(x)	((unsigned)(x) & 0x1fffffff)
#define	MACH_PHYS_TO_CACHED(x)	((unsigned)(x) | MACH_CACHED_MEMORY_ADDR)
#define	MACH_UNCACHED_TO_PHYS(x) ((unsigned)(x) & 0x1fffffff)
#define	MACH_PHYS_TO_UNCACHED(x) ((unsigned)(x) | MACH_UNCACHED_MEMORY_ADDR)

#define MACH_CODE_START			0x80030000

/*
 * The bits in the cause register.
 */
#define MACH_Cause_BD           0x80000000      /* Exception occured in delay slot */
#define MACH_Cause_TI           0x40000000      /* Timer interrupt is pending */
#define MACH_Cause_CE           0x30000000      /* Coprocessor exception */
#define MACH_Cause_DC           0x08000000      /* Disable COUNT register */
#define MACH_Cause_PCI          0x04000000      /* Performance counter interrupt */
#define MACH_Cause_IC           0x02000000      /* Interrupt changing since last IRET */
#define MACH_Cause_AP           0x01000000      /* Exception occured in automated prolog */
#define MACH_Cause_IV           0x00800000      /* Use special interrupt vector 0x200 */
#define MACH_Cause_WP           0x00400000      /* Watch exception pending */
#define MACH_Cause_FDCI         0x00200000      /* Fast debug channel interrupt */
#define MACH_Cause_RIPL(r)      ((r)>>10 & 63)  /* Requested interrupt priority level */
#define MACH_Cause_IP1          0x00020000      /* Request software interrupt 1 */
#define MACH_Cause_IP0          0x00010000      /* Request software interrupt 0 */
#define MACH_Cause_ExcCode      0x0000007c      /* Exception code */
#define MACH_Cause_ExcCode_SHIFT	2

/*
 * The bits in the status register.
 */
#define MACH_Status_CU0         0x10000000      /* Access to coprocessor 0 allowed (in user mode) */
#define MACH_Status_RP          0x08000000      /* Enable reduced power mode */
#define MACH_Status_RE          0x02000000      /* Reverse endianness (in user mode) */
#define MACH_Status_MX          0x01000000      /* DSP resource enable */
#define MACH_Status_BEV         0x00400000      /* Exception vectors: bootstrap */
#define MACH_Status_TS          0x00200000      /* TLB shutdown control */
#define MACH_Status_SR          0x00100000      /* Soft reset */
#define MACH_Status_NMI         0x00080000      /* NMI reset */
#define MACH_Status_IPL(x)      ((x) << 10)     /* Current interrupt priority level */
#define MACH_Status_IPL_MASK    0x0000fc00
#define MACH_Status_IPL_SHIFT   10
#define MACH_Status_UM          0x00000010      /* User mode */
#define MACH_Status_ERL         0x00000004      /* Error level */
#define MACH_Status_EXL         0x00000002      /* Exception level */
#define MACH_Status_IE          0x00000001      /* Interrupt enable */

/*
 * The interrupt masks.
 * If a bit in the mask is 1 then the interrupt is enabled (or pending).
 */

/*
 * Location of exception vectors.
 */
#define MACH_RESET_EXC_VEC	0xBFC00000
#define MACH_UTLB_MISS_EXC_VEC	0x80000000
#define MACH_GEN_EXC_VEC	0x80000080

/*
 * Coprocessor 0 registers.
 */
#define MACH_C0_Index		$0      /* TLB index */
#define MACH_C0_Random		$1      /* TLB random */
#define MACH_C0_EntryLo0	$2      /* TLB entry low 0 */
#define MACH_C0_EntryLo1	$3      /* TLB entry low 1 */
#define MACH_C0_Context		$4      /* TLB context */
#define MACH_C0_BadVAddr	$8      /* Bad virtual address */
#define MACH_C0_EntryHi		$10     /* TLB entry high */
#define MACH_C0_Status		$12     /* Status register */
#define MACH_C0_Cause		$13     /* Exception cause register */
#define MACH_C0_EPC		$14     /* Exception PC */
#define MACH_C0_PRId		$15     /* Processor revision identifier */

/*
 * Values for the code field in a break instruction.
 */
#define MACH_BREAK_INSTR	0x0000000d
#define MACH_BREAK_VAL_MASK	0x03ff0000
#define MACH_BREAK_VAL_SHIFT	16
#define MACH_BREAK_KDB_VAL	512
#define MACH_BREAK_SSTEP_VAL	513
#define MACH_BREAK_BRKPT_VAL	514
#define MACH_BREAK_KDB		(MACH_BREAK_INSTR | \
				(MACH_BREAK_KDB_VAL << MACH_BREAK_VAL_SHIFT))
#define MACH_BREAK_SSTEP	(MACH_BREAK_INSTR | \
				(MACH_BREAK_SSTEP_VAL << MACH_BREAK_VAL_SHIFT))
#define MACH_BREAK_BRKPT	(MACH_BREAK_INSTR | \
				(MACH_BREAK_BRKPT_VAL << MACH_BREAK_VAL_SHIFT))

/*
 * Mininum and maximum cache sizes.
 */
#define MACH_MIN_CACHE_SIZE	(16 * 1024)
#define MACH_MAX_CACHE_SIZE	(256 * 1024)

/*
 * The floating point version and status registers.
 */
#define	MACH_FPC_ID	$0
#define	MACH_FPC_CSR	$31

/*
 * The floating point coprocessor status register bits.
 */
#define MACH_FPC_ROUNDING_BITS		0x00000003
#define MACH_FPC_ROUND_RN		0x00000000
#define MACH_FPC_ROUND_RZ		0x00000001
#define MACH_FPC_ROUND_RP		0x00000002
#define MACH_FPC_ROUND_RM		0x00000003
#define MACH_FPC_STICKY_BITS		0x0000007c
#define MACH_FPC_STICKY_INEXACT		0x00000004
#define MACH_FPC_STICKY_UNDERFLOW	0x00000008
#define MACH_FPC_STICKY_OVERFLOW	0x00000010
#define MACH_FPC_STICKY_DIV0		0x00000020
#define MACH_FPC_STICKY_INVALID		0x00000040
#define MACH_FPC_ENABLE_BITS		0x00000f80
#define MACH_FPC_ENABLE_INEXACT		0x00000080
#define MACH_FPC_ENABLE_UNDERFLOW	0x00000100
#define MACH_FPC_ENABLE_OVERFLOW	0x00000200
#define MACH_FPC_ENABLE_DIV0		0x00000400
#define MACH_FPC_ENABLE_INVALID		0x00000800
#define MACH_FPC_EXCEPTION_BITS		0x0003f000
#define MACH_FPC_EXCEPTION_INEXACT	0x00001000
#define MACH_FPC_EXCEPTION_UNDERFLOW	0x00002000
#define MACH_FPC_EXCEPTION_OVERFLOW	0x00004000
#define MACH_FPC_EXCEPTION_DIV0		0x00008000
#define MACH_FPC_EXCEPTION_INVALID	0x00010000
#define MACH_FPC_EXCEPTION_UNIMPL	0x00020000
#define MACH_FPC_COND_BIT		0x00800000
#define MACH_FPC_MBZ_BITS		0xff7c0000

/*
 * Constants to determine if have a floating point instruction.
 */
#define MACH_OPCODE_SHIFT	26
#define MACH_OPCODE_C1		0x11

/*
 * The low part of the TLB entry.
 */
#define VMMACH_TLB_PF_NUM		0xfffff000
#define VMMACH_TLB_NON_CACHEABLE_BIT	0x00000800
#define VMMACH_TLB_MOD_BIT		0x00000400
#define VMMACH_TLB_VALID_BIT		0x00000200
#define VMMACH_TLB_GLOBAL_BIT		0x00000100

#define VMMACH_TLB_PHYS_PAGE_SHIFT	12

/*
 * The high part of the TLB entry.
 */
#define VMMACH_TLB_VIRT_PAGE_NUM	0xfffff000
#define VMMACH_TLB_PID			0x00000fc0
#define VMMACH_TLB_PID_SHIFT		6
#define VMMACH_TLB_VIRT_PAGE_SHIFT	12

/*
 * The shift to put the index in the right spot.
 */
#define VMMACH_TLB_INDEX_SHIFT		8

/*
 * The number of TLB entries and the first one that write random hits.
 */
#define VMMACH_NUM_TLB_ENTRIES		64
#define VMMACH_FIRST_RAND_ENTRY 	8

/*
 * The number of process id entries.
 */
#define	VMMACH_NUM_PIDS			64

/*
 * TLB probe return codes.
 */
#define VMMACH_TLB_NOT_FOUND		0
#define VMMACH_TLB_FOUND		1
#define VMMACH_TLB_FOUND_WITH_PATCH	2
#define VMMACH_TLB_PROBE_ERROR		3

/*
 * Kernel virtual address for user page table entries
 * (i.e., the address for the context register).
 */
#define VMMACH_PTE_BASE		0xFFC00000

#endif /* _MACHCONST */
