/*
 * This program converts an elf executable to a BSD a.out executable.
 * Using a demand paged format.
 *
 * Copyright (c) 2011-2014 Serge Vakulenko
 * Copyright (c) 1995 Ted Lemon (hereinafter referred to as the author)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>

/*
 * Assume 4k target page size.
 */
#define PAGESIZE 4096

/*
 * Header prepended to each a.out file.
 */
struct exec {
    uint32_t a_magic;           /* magic number */
#define OMAGIC  0407            /* old impure format */
#define ZMAGIC  0413            /* demand load format */

    uint32_t a_text;            /* text segment size */
    uint32_t a_data;            /* initialized data size */
    uint32_t a_bss;             /* uninitialized data size */
    uint32_t a_syms;            /* symbol table size */
    uint32_t a_entry;           /* entry point */
    uint32_t a_trsize;          /* text relocation size */
    uint32_t a_drsize;          /* data relocation size */
};

/*
 * Simple values for n_type.
 */
#define N_UNDF  0x00            /* undefined */
#define N_ABS   0x01            /* absolute */
#define N_TEXT  0x02            /* text segment */
#define N_DATA  0x03            /* data segment */
#define N_BSS   0x04            /* bss segment */
#define N_REG   0x14            /* register symbol */
#define N_FN    0x1f            /* file name */

#define N_EXT   0x20            /* external (global) bit, OR'ed in */
#define N_TYPE  0x1f            /* mask for all the type bits */

struct sect {
    /* should be unsigned long, but assume no a.out binaries on LP64 */
    uint32_t vaddr;
    uint32_t len;
    uint32_t offset;
    uint32_t filesz;
};

/*
 * Standard ELF types, structures, and macros.
 */

/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct {
     unsigned char  e_ident[EI_NIDENT]; /* Magic number and other info */
     uint16_t       e_type;             /* Object file type */
     uint16_t       e_machine;          /* Architecture */
     uint32_t       e_version;          /* Object file version */
     uint32_t       e_entry;            /* Entry point virtual address */
     uint32_t       e_phoff;            /* Program header table file offset */
     uint32_t       e_shoff;            /* Section header table file offset */
     uint32_t       e_flags;            /* Processor-specific flags */
     uint16_t       e_ehsize;           /* ELF header size in bytes */
     uint16_t       e_phentsize;        /* Program header table entry size */
     uint16_t       e_phnum;            /* Program header table entry count */
     uint16_t       e_shentsize;        /* Section header table entry size */
     uint16_t       e_shnum;            /* Section header table entry count */
     uint16_t       e_shstrndx;         /* Section header string table index */
} Elf32_Ehdr;

/* Program segment header.  */

typedef struct {
     uint32_t       p_type;             /* Segment type */
     uint32_t       p_offset;           /* Segment file offset */
     uint32_t       p_vaddr;            /* Segment virtual address */
     uint32_t       p_paddr;            /* Segment physical address */
     uint32_t       p_filesz;           /* Segment size in file */
     uint32_t       p_memsz;            /* Segment size in memory */
     uint32_t       p_flags;            /* Segment flags */
     uint32_t       p_align;            /* Segment alignment */
} Elf32_Phdr;

/* Legal values for p_type (segment type).  */

#define PT_NULL         0               /* Program header table entry unused */
#define PT_LOAD         1               /* Loadable program segment */
#define PT_DYNAMIC      2               /* Dynamic linking information */
#define PT_INTERP       3               /* Program interpreter */
#define PT_NOTE         4               /* Auxiliary information */
#define PT_SHLIB        5               /* Reserved */
#define PT_PHDR         6               /* Entry for header table itself */
#define PT_TLS          7               /* Thread-local storage segment */
#define PT_NUM          8               /* Number of defined types */
#define PT_LOOS         0x60000000      /* Start of OS-specific */
#define PT_GNU_EH_FRAME 0x6474e550      /* GCC .eh_frame_hdr segment */
#define PT_GNU_STACK    0x6474e551      /* Indicates stack executability */
#define PT_GNU_RELRO    0x6474e552      /* Read-only after relocation */
#define PT_LOSUNW       0x6ffffffa
#define PT_SUNWBSS      0x6ffffffa      /* Sun Specific segment */
#define PT_SUNWSTACK    0x6ffffffb      /* Stack segment */
#define PT_HISUNW       0x6fffffff
#define PT_HIOS         0x6fffffff      /* End of OS-specific */
#define PT_LOPROC       0x70000000      /* Start of processor-specific */
#define PT_HIPROC       0x7fffffff      /* End of processor-specific */

/* Legal values for p_type field of Elf32_Phdr.  */

#define PT_MIPS_REGINFO 0x70000000      /* Register usage information */
#define PT_MIPS_RTPROC  0x70000001      /* Runtime procedure table. */
#define PT_MIPS_OPTIONS 0x70000002

/* Legal values for p_flags (segment flags).  */

#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
#define PF_MASKOS       0x0ff00000      /* OS-specific */
#define PF_MASKPROC     0xf0000000      /* Processor-specific */

/*
 * Align the length on page boundary.
 */
uint32_t
page_align(uint32_t len)
{
    return (len + PAGESIZE - 1) & ~(PAGESIZE - 1);
}

/*
 * Read data from file into a freshly allocated buffer.
 */
char *
read_alloc(int file, off_t offset, off_t len)
{
    char   *tmp;
    int     count;
    off_t   off;

    off = lseek(file, offset, SEEK_SET);
    if (off < 0) {
        perror("lseek");
        exit(1);
    }
    tmp = malloc(len);
    if (! tmp) {
        fprintf(stderr, "Can't allocate %ld bytes.\n", (long)len);
        exit(1);
    }
    count = read(file, tmp, len);
    if (count != len) {
        fprintf(stderr, "read: %s.\n",
            count ? strerror(errno) : "End of file reached");
        exit(1);
    }
    return tmp;
}

/*
 * Compare two program segment headers, for qsort().
 */
int
phcmp(const void *vh1, const void *vh2)
{
    const Elf32_Phdr *h1 = (const Elf32_Phdr*) vh1;
    const Elf32_Phdr *h2 = (const Elf32_Phdr*) vh2;

    if (h1->p_vaddr > h2->p_vaddr)
        return 1;
    if (h1->p_vaddr < h2->p_vaddr)
        return -1;
    return 0;
}



/*
 * Copy a chunk of data from `in' file to `out' file.
 */
void
copy(int out, int in, off_t offset, off_t size)
{
    char    ibuf[4096];
    int     remaining, cur, count;

    /* Go to the start of the ELF symbol table... */
    if (lseek(in, offset, SEEK_SET) < 0) {
        perror("copy: lseek");
        exit(1);
    }
    remaining = size;
    while (remaining) {
        cur = remaining;
        if (cur > (int)sizeof ibuf)
            cur = sizeof ibuf;
        remaining -= cur;
        count = read(in, ibuf, cur);
        if (count != cur) {
            fprintf(stderr, "copy: read: %s\n",
                count ? strerror(errno) : "premature end of file");
            exit(1);
        }
        count = write(out, ibuf, cur);
        if (count != cur) {
            perror("copy: write");
            exit(1);
        }
    }
}

int
main(int argc, char **argv)
{
    Elf32_Ehdr ex;
    Elf32_Phdr *ph;
    int i;
    struct sect text = {0}, data = {0}, bss = {0};
    struct exec aex;
    int infile, outfile;
    uint32_t cur_vma = UINT32_MAX;
    int verbose = 0;
    static char zeroes[PAGESIZE];

    /* Check args... */
    for (;;) {
        switch (getopt (argc, argv, "v")) {
        case EOF:
            break;
        case 'v':
            ++verbose;
            continue;
        default:
usage:      fprintf(stderr,
                "usage: elf2aout [-v] input.elf output.aout\n");
            exit(1);
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (argc != 2)
        goto usage;

    /* Try the input file... */
    infile = open(argv[0], O_RDONLY);
    if (infile < 0) {
        fprintf(stderr, "Can't open %s for read: %s\n",
            argv[0], strerror(errno));
        exit(1);
    }

    /* Read the header, which is at the beginning of the file... */
    i = read(infile, &ex, sizeof ex);
    if (i != sizeof ex) {
        fprintf(stderr, "ex: %s: %s.\n",
            argv[0], i ? strerror(errno) : "End of file reached");
        exit(1);
    }

    /* Read the program headers... */
    ph = (Elf32_Phdr *) read_alloc(infile, ex.e_phoff,
        ex.e_phnum * sizeof(Elf32_Phdr));

    /* Figure out if we can cram the program header into an a.out
     * header... Basically, we can't handle anything but loadable
     * segments, but we can ignore some kinds of segments.   We can't
     * handle holes in the address space, and we handle non-standard
     * start addresses by hoping that the loader will know where to load
     * - a.out doesn't have an explicit load address. */
    qsort(ph, ex.e_phnum, sizeof(Elf32_Phdr), phcmp);
    for (i = 0; i < ex.e_phnum; i++) {
        /* Search for LOAD segments. */
        if (ph[i].p_type != PT_LOAD)
            continue;

        if (verbose)
            printf ("Segment type=%x flags=%x vaddr=%x filesz=%x\n",
                ph[i].p_type, ph[i].p_flags, ph[i].p_vaddr, ph[i].p_filesz);

        if (ph[i].p_flags & PF_W) {
            /* Writable segment: data and bss. */
            if (data.len != 0) {
                fprintf(stderr, "Can't handle more than one .data segment: %s\n",
                    argv[0]);
                exit(1);
            }
            data.vaddr  = ph[i].p_vaddr;
            data.len    = page_align (ph[i].p_filesz);
            data.offset = ph[i].p_offset;
            data.filesz = ph[i].p_filesz;

            bss.vaddr = ph[i].p_vaddr + data.len;
            bss.len   = ph[i].p_memsz - data.len;
        } else {
            /* Read-only segment: text. */
            if (text.len != 0) {
                fprintf(stderr, "Can't handle more than one .text segment: %s\n",
                    argv[0]);
                exit(1);
            }
            text.vaddr  = ph[i].p_vaddr;
            text.len    = page_align (ph[i].p_filesz);
            text.offset = ph[i].p_offset;
        }
        /* Remember the lowest segment start address. */
        if (ph[i].p_vaddr < cur_vma)
            cur_vma = ph[i].p_vaddr;
    }

    if (verbose)
        printf ("Text %x-%x, data %x-%x, bss %x-%x\n",
            text.vaddr, text.vaddr + text.len - 1,
            data.vaddr, data.vaddr + data.len - 1,
            bss.vaddr,  bss.vaddr  + bss.len -  1);

    /* Check start of .text segment. */
    if (text.vaddr != 0x400000) {
        fprintf(stderr, "Invalid start of .text segment: %s\n",
            argv[0]);
        exit(1);
    }

    /* Sections must be aligned on a page boundary. */
    if (text.vaddr % PAGESIZE || data.vaddr % PAGESIZE ||
        text.len % PAGESIZE || data.len % PAGESIZE) {
        fprintf(stderr, "Sections must be page aligned.\n");
        exit(1);
    }

    /* Sections must be in order to be converted... */
    if (text.vaddr > data.vaddr || data.vaddr > bss.vaddr ||
        text.vaddr + text.len > data.vaddr || data.vaddr + data.len > bss.vaddr) {
        fprintf(stderr, "Sections ordering prevents a.out conversion.\n");
        exit(1);
    }

    /* There should be no gap between text and data. */
    if (text.vaddr + text.len != data.vaddr) {
        fprintf(stderr, "Unexpected gap between .text and .data segments: %s\n",
            argv[0]);
        exit(1);
    }

    /* We now have enough information to cons up an a.out header... */
    aex.a_magic = ZMAGIC;
    aex.a_text = text.len;
    aex.a_data = data.len;
    aex.a_bss = bss.len;
    aex.a_entry = ex.e_entry;
    aex.a_syms = 0;
    aex.a_trsize = 0;
    aex.a_drsize = 0;
    if (verbose) {
        printf ("  magic: %#o\n", aex.a_magic);
        printf ("   text: %#x\n", aex.a_text);
        printf ("   data: %#x\n", aex.a_data);
        printf ("    bss: %#x\n", aex.a_bss);
        printf ("  entry: %#x\n", aex.a_entry);
        printf ("   syms: %u\n", aex.a_syms);
        printf (" trsize: %#x\n", aex.a_trsize);
        printf (" drsize: %#x\n", aex.a_drsize);
    }

    /* Make the output file... */
    outfile = open(argv[1], O_WRONLY | O_CREAT, 0777);
    if (outfile < 0) {
        fprintf(stderr, "Unable to create %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    /* Truncate file... */
    if (ftruncate(outfile, 0)) {
        fprintf(stderr, "Warning: cannot truncate %s\n", argv[1]);
    }

    /* Write the header and alignment. */
    i = write(outfile, &aex, sizeof aex);
    if (i != sizeof aex) {
        perror("aex: write");
        exit(1);
    }
    i = write(outfile, zeroes, PAGESIZE - sizeof aex);
    if (i != PAGESIZE - sizeof aex) {
        perror("aex alignment: write");
        exit(1);
    }

    /* Copy text section. */
    copy(outfile, infile, text.offset, text.len);

    /* Copy data section. */
    copy(outfile, infile, data.offset, data.filesz);
    if (data.len != data.filesz) {
        i = write(outfile, zeroes, data.len - data.filesz);
        if (i != data.len - data.filesz) {
            perror("data alignment: write");
            exit(1);
        }
    }

    return 0;
}
