/*-
 * Copyright (c) 2008-2011 Joseph Koshy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Define LIBELF_{ARCH,BYTEORDER,CLASS} based on the machine architecture.
 */
#if	defined(__amd64__)

#define	LIBELF_ARCH		EM_X86_64
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#define	LIBELF_CLASS		ELFCLASS64

#elif	defined(__aarch64__)

#define	LIBELF_ARCH		EM_AARCH64
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#define	LIBELF_CLASS		ELFCLASS64

#elif	defined(__arm__)

#define	LIBELF_ARCH		EM_ARM
#if	defined(__ARMEB__)	/* Big-endian ARM. */
#define	LIBELF_BYTEORDER	ELFDATA2MSB
#else
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#endif
#define	LIBELF_CLASS		ELFCLASS32

#elif	defined(__i386__)

#define	LIBELF_ARCH		EM_386
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#define	LIBELF_CLASS		ELFCLASS32

#elif	defined(__ia64__)

#define	LIBELF_ARCH		EM_IA_64
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#define	LIBELF_CLASS		ELFCLASS64

#elif	defined(__mips__)

#define	LIBELF_ARCH		EM_MIPS
#if	defined(__MIPSEB__)
#define	LIBELF_BYTEORDER	ELFDATA2MSB
#else
#define	LIBELF_BYTEORDER	ELFDATA2LSB
#endif
#define	LIBELF_CLASS		ELFCLASS32

#elif	defined(__powerpc__)

#define	LIBELF_ARCH		EM_PPC
#define	LIBELF_BYTEORDER	ELFDATA2MSB
#define	LIBELF_CLASS		ELFCLASS32

#elif	defined(__sparc__)

#define	LIBELF_ARCH		EM_SPARCV9
#define	LIBELF_BYTEORDER	ELFDATA2MSB
#define	LIBELF_CLASS		ELFCLASS64

#else
#error	Unknown architecture.
#endif
