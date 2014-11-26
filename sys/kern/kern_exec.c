/*-
 * Execve system call implementation ported from FreeBSD 1.0.
 *
 * Copyright (c) 1989, 1990, 1991, 1992 William F. Jolitz, TeleMuse
 * All rights reserved.
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/exec.h>
#include <sys/mount.h>
#include <sys/resourcevar.h>
#include <sys/mman.h>
#include <sys/acct.h>
#include <sys/wait.h>
#include <sys/filedesc.h>
#include <sys/malloc.h>
#include <sys/syscallargs.h>
#include <sys/signalvar.h>
#include <sys/exec_elf.h>
#include <vm/vm.h>
#include <machine/reg.h>

extern char sigcode[], esigcode[];
#define SZSIGCODE (esigcode - sigcode)

/*
 * Close any files on exec?
 */
static void
fdcloseexec(p)
    struct proc *p;
{
    struct filedesc *fdp = p->p_fd;
    struct file **fpp;
    char *fdfp;
    register int i;

    fpp = fdp->fd_ofiles;
    fdfp = fdp->fd_ofileflags;
    for (i = 0; i <= fdp->fd_lastfile; i++, fpp++, fdfp++)
        if (*fpp != NULL && (*fdfp & UF_EXCLOSE)) {
            if (*fdfp & UF_MAPPED)
                (void) munmapfd(p, i);
            (void) closef(*fpp, p);
            *fpp = NULL;
            *fdfp = 0;
            if (i < fdp->fd_freefile)
                fdp->fd_freefile = i;
        }
    while (fdp->fd_lastfile > 0 && fdp->fd_ofiles[fdp->fd_lastfile] == NULL)
        fdp->fd_lastfile--;
}

/*
 * Copy string and count the length.
 */
static int
countstr(str, buf, argc, arglen)
    char *str, *buf;
    int *argc, *arglen;
{
    u_int stringlen;
    int rv;

    if (*arglen >= ARG_MAX)
        return E2BIG;

    rv = copystr(str, buf, ARG_MAX - *arglen, &stringlen);
    if (rv)
        return (rv == ENAMETOOLONG) ? E2BIG : rv;
    *argc += 1;
    *arglen += stringlen;
    return 0;
}

/*
 * Copy string from user space and count the length.
 */
static int
countinstr(str, buf, argc, arglen)
    char *str, *buf;
    int *argc, *arglen;
{
    u_int stringlen;
    int rv;

    if (*arglen >= ARG_MAX)
        return E2BIG;

    rv = copyinstr(str, buf, ARG_MAX - *arglen, &stringlen);
    if (rv)
        return (rv == ENAMETOOLONG) ? E2BIG : rv;
    *argc += 1;
    *arglen += stringlen;
    return 0;
}

/*
 * Structure holding information about the executable file.
 */
struct exec_hdr {
    int indir;
    char shellname[MAXINTERP];
    char *shellargs;
    unsigned file_offset;
    unsigned virtual_offset;
    unsigned text;
    unsigned data;
    unsigned bss;
    unsigned entry;
};

/*
 * Copy arguments and environment.
 */
static int
copyargs (uap, hdr, framebuf, framesz)
    struct execve_args *uap;
    struct exec_hdr *hdr;
    char *framebuf;
    unsigned *framesz;
{
    char **vectp, *stringp;
    int argc, envc, rv, *argbuf, *argp, user_stringp;
    struct ps_strings *arginfo;

    /*----------------------------------------------------------------
     * Pass 1: compute arg count and size.
     */
    u_int arglen;

    argc = 0;
    arglen = 0;
    vectp = SCARG(uap, argp);
    if (hdr->indir) {
        /* Count shell parameters. */
        rv = countstr(hdr->shellname, framebuf, &argc, &arglen);
        if (rv)
            return rv;

        if (hdr->shellargs) {
            rv = countstr(hdr->shellargs, framebuf, &argc, &arglen);
            if (rv)
                return rv;
        }

        rv = countstr(SCARG(uap, path), framebuf, &argc, &arglen);
        if (rv)
            return rv;

        if (vectp)
            vectp++;
    }

    if (vectp) {
        /* Count arguments. */
        for (;;) {
            char *ptr = (char*) fuword(vectp++);
            if (ptr == 0) {
                argc++;
                break;
            }
            rv = countinstr(ptr, framebuf, &argc, &arglen);
            if (rv)
                return rv;
        }
    }

    vectp = SCARG(uap, envp);
    envc = 0;
    if (vectp) {
        /* Count environment strings. */
        for (;;) {
            char *ptr = (char*) fuword(vectp++);
            if (ptr == 0)
                break;

            rv = countinstr(ptr, framebuf, &envc, &arglen);
            if (rv)
                return rv;
        }
    }
    envc++;

    /*----------------------------------------------------------------
     * Pass 2: copy arguments to user stack.
     */

    /* Now we have argc, argv and arglen: compute the frame size */
    arglen += (argc + envc + 1) * sizeof(int);
    arglen += sizeof(*arginfo) + SZSIGCODE;
    arglen = ALIGN(arglen);
    if (arglen > ARG_MAX)
        return E2BIG;
    *framesz = arglen;

    /* allocate string buffer and arg buffer */
    argbuf = (int*) framebuf;
    argp = argbuf + 1;
    stringp = (char*) (argbuf + argc + envc + 1);
    user_stringp = USRSTACK - arglen + (argc + envc + 1) * sizeof(int);
    arginfo = (struct ps_strings*) (framebuf + arglen - sizeof(*arginfo));

    argc = 0;
    arglen = 0;
    vectp = SCARG(uap, argp);
    arginfo->ps_argvstr = (char*)user_stringp; /* remember location of argv */
    if (hdr->indir) {
        /* Copy shell parameters. */
        *argp++ = user_stringp;
        rv = countstr(hdr->shellname, stringp, &argc, &arglen);
        if (rv)
            return rv;

        if (hdr->shellargs) {
            *argp++ = user_stringp + arglen;
            rv = countstr(hdr->shellargs, stringp + arglen, &argc, &arglen);
            if (rv)
                return rv;
        }

        *argp++ = user_stringp + arglen;
        rv = countstr(SCARG(uap, path), stringp + arglen, &argc, &arglen);
        if (rv)
            return rv;

        if (vectp)
            vectp++;
    }

    if (vectp) {
        /* Copy arguments. */
        for (;;) {
            char *ptr = (char*) fuword(vectp++);
            if (ptr == 0) {
                *argp++ = 0;
                break;
            }
            *argp++ = user_stringp + arglen;
            rv = countinstr(ptr, stringp + arglen, &argc, &arglen);
            if (rv)
                return rv;
        }
    }
    argbuf[0] = argc;
    arginfo->ps_nargvstr = argc;

    vectp = SCARG(uap, envp);
    envc = 0;
    arginfo->ps_envstr = (char*)user_stringp + arglen; /* remember location of env */
    if (vectp) {
        /* Copy environment strings. */
        for (;;) {
            char *ptr = (char*) fuword(vectp++);
            if (ptr == 0)
                break;

            *argp++ = user_stringp + arglen;
            rv = countinstr(ptr, stringp + arglen, &envc, &arglen);
            if (rv)
                return rv;
        }
    }
    *argp = 0;
    arginfo->ps_nenvstr = envc;

    /* copy the process's signal trapoline code */
    bcopy((char*)sigcode, (char*)arginfo - SZSIGCODE, SZSIGCODE);
//printf("%s: framesz=%u, sigcode at %08x\n", __func__, *framesz, (char*)arginfo - SZSIGCODE);
    return 0;
}

/*
 * Read in first few bytes of file for segment sizes, magic number:
 *      ZMAGIC = demand paged RO text
 * Also an ASCII line beginning with #! is
 * the file name of a ``shell'' and arguments may be prepended
 * to the argument list if given here.
 */
static int
getheader(p, ndp, file_size, hdr)
    struct proc *p;
    struct nameidata *ndp;
    unsigned file_size;
    struct exec_hdr *hdr;
{
    int rv, amt;
    union {
        char    ex_shell[MAXINTERP];    /* #! and interpreter name */
        int     magic;                  /* first word: magic tag */
        struct  exec aout_hdr;          /* a.out header */
        struct  elf_ehdr elf_hdr;       /* ELF header */
    } exdata;
    struct elf_phdr segment[2];

    exdata.ex_shell[0] = '\0';  /* for zero length files */

    rv = vn_rdwr(UIO_READ, ndp->ni_vp, (caddr_t)&exdata, sizeof(exdata),
        0, UIO_SYSSPACE, IO_NODELOCKED, p->p_ucred, &amt, p);

    /* big enough to hold a header? */
    if (rv)
        return rv;

    switch (exdata.magic) {
    case ZMAGIC:
        /* A.out demand paged format. */
        hdr->text  = exdata.aout_hdr.a_text;
        hdr->data  = exdata.aout_hdr.a_data;
        hdr->bss   = exdata.aout_hdr.a_bss;
        hdr->entry = exdata.aout_hdr.a_entry;

        hdr->virtual_offset = 0x400000;
        hdr->file_offset    = NBPG;
        break;

    case ELFMAG0 | ELFMAG1<<8 | ELFMAG2 << 16 | ELFMAG3 << 24:
        /* ELF format. */
        if (exdata.elf_hdr.e_ident[4] != ELFCLASS32 ||
            exdata.elf_hdr.e_ident[5] != ELFDATA2LSB ||
            exdata.elf_hdr.e_ident[6] != EV_CURRENT ||
            exdata.elf_hdr.e_ident[7] != ELFOSABI_SYSV)
            return ENOEXEC;
        if (exdata.elf_hdr.e_type != ET_EXEC)
            return ENOEXEC;
        if (exdata.elf_hdr.e_machine != EM_MIPS ||
            exdata.elf_hdr.e_version != EV_CURRENT)
            return ENOEXEC;
        if (exdata.elf_hdr.e_phentsize != sizeof(struct elf_phdr) ||
            exdata.elf_hdr.e_phoff == 0 ||
            exdata.elf_hdr.e_phnum == 0)
            return ENOEXEC;
        if (exdata.elf_hdr.e_shnum == 0 ||
            exdata.elf_hdr.e_shentsize != sizeof(struct elf_shdr))
            return ENOEXEC;

        if (exdata.elf_hdr.e_phnum != 2) {
            /* At the current moment we handle only 2-segment ELF binaries.
             * The rest of the code is implemented as need arise. */
            return ENOEXEC;
        }

        /* Read headers of program segments. */
        rv = vn_rdwr(UIO_READ, ndp->ni_vp, (caddr_t)segment, sizeof(segment),
            exdata.elf_hdr.e_phoff, UIO_SYSSPACE, IO_NODELOCKED, p->p_ucred, &amt, p);
        if (rv)
            return ENOEXEC;
//printf("%s: elf segment 0: type=%x flags=%x, segment 1: type=%x flags=%x\n", __func__, segment[0].p_type, segment[0].p_flags, segment[1].p_type, segment[1].p_flags);

        /* Two loadable segments expected:
         * segment 0: read/execute,
         * segment 1: read/write. */
        if (segment[0].p_type != PT_LOAD ||
            segment[0].p_flags != (PF_R | PF_X) ||
            segment[1].p_type != PT_LOAD ||
            segment[1].p_flags != (PF_R | PF_W))
            return ENOEXEC;

        hdr->text  = roundup(segment[0].p_filesz, NBPG);
        hdr->data  = roundup(segment[1].p_filesz, NBPG);
        hdr->bss   = segment[1].p_memsz - hdr->data;
        hdr->entry = exdata.elf_hdr.e_entry;

        hdr->virtual_offset = segment[0].p_vaddr;
        hdr->file_offset    = segment[0].p_offset;
//printf("%s: virtual_offset=%08x, file_offset=%u\n", __func__, hdr->virtual_offset, hdr->file_offset);
        break;

    default:
        /* Check for shell bang '#!'. */
        if (exdata.ex_shell[0] != '#' || exdata.ex_shell[1] != '!')
            return ENOEXEC;

        /* Only one level of indirection is allowed. */
        if (hdr->indir)
            return ENOEXEC;

        char *cp, *sp;
        for (cp = &exdata.ex_shell[2];; ++cp) {
            if (cp >= &exdata.ex_shell[MAXINTERP])
                return ENOEXEC;

            if (*cp == '\n') {
                *cp = '\0';
                break;
            }
            if (*cp == '\t')
                *cp = ' ';
        }
        cp = &exdata.ex_shell[2]; /* get shell interpreter name */
        while (*cp == ' ')
            cp++;

        sp = hdr->shellname;
        while (*cp && *cp != ' ')
            *sp++ = *cp++;
        *sp = '\0';

        /* copy the args in the #! line */
        while (*cp == ' ')
            cp++;
        if (*cp) {
            sp++;
            hdr->shellargs = sp;
            while (*cp)
                *sp++ = *cp++;
            *sp = '\0';
        } else {
            hdr->shellargs = 0;
        }

        hdr->indir = 1;                 /* indicate this is a script file */
        vput(ndp->ni_vp);

        ndp->ni_dirp = hdr->shellname;  /* find shell interpreter */
        ndp->ni_segflg = UIO_SYSSPACE;
        return -1;
    }
//printf("%s: text=%u, data=%u, bss=%u, entry=%08x\n", __func__, hdr->text, hdr->data, hdr->bss, hdr->entry);

    if (hdr->text != 0 && (ndp->ni_vp->v_flag & VTEXT) == 0 &&
        ndp->ni_vp->v_writecount != 0)
        return ETXTBSY;

    /* sanity check  "ain't not such thing as a sanity clause" -groucho */
    if (hdr->text == 0 ||
        hdr->text > MAXTSIZ ||
        hdr->text % NBPG ||
        hdr->text > file_size)
        return ENOMEM;

    if (hdr->data == 0 ||
        hdr->data > DFLDSIZ ||
        hdr->data > file_size ||
        hdr->data + hdr->text > file_size)
        return ENOMEM;

    if (hdr->bss > MAXDSIZ)
        return ENOMEM;

    if (hdr->text + hdr->data + hdr->bss > MAXTSIZ + MAXDSIZ)
        return ENOMEM;

    if (hdr->data + hdr->bss > p->p_rlimit[RLIMIT_DATA].rlim_cur)
        return ENOMEM;

    if (hdr->entry > hdr->text + hdr->data + hdr->virtual_offset)
        return ENOMEM;

    return 0;
}

/* ARGSUSED */
int
execve(p, uap, retval)
    struct proc *p;
    register struct execve_args *uap;
    int *retval;
{
    register struct nameidata *ndp;
    struct nameidata nd;
    int rv, tsize, dsize, bsize, len;
    struct vattr attr;
    struct exec_hdr hdr;
    struct vmspace *vs;
    vm_offset_t addr;
    char *argbuf = 0;
    unsigned framesz;

    /*----------------------------------------------------------------
     * Step 1. Lookup filename to see if we have something to execute.
     */
    ndp = &nd;
    hdr.indir = 0;
again:
    NDINIT(ndp, LOOKUP, LOCKLEAF | FOLLOW | SAVENAME, UIO_USERSPACE,
        SCARG(uap, path), p);

    /* is it there? */
    rv = namei(ndp);
    if (rv)
        return rv;

    if (ndp->ni_vp->v_writecount) { /* don't exec if file is busy */
        rv = EBUSY;
        goto exec_fail;
    }
    /* does it have any attributes? */
    rv = VOP_GETATTR(ndp->ni_vp, &attr, p->p_ucred, p);
    if (rv)
        goto exec_fail;

    if (ndp->ni_vp->v_mount->mnt_flag & MNT_NOEXEC) { /* no exec on fs ?*/
        rv = EACCES;
        goto exec_fail;
    }

    /* is it executable, and a regular file? */
    if ((ndp->ni_vp->v_mount->mnt_flag & MNT_NOEXEC) || /* 29 Jul 92*/
        (VOP_ACCESS(ndp->ni_vp, VEXEC, p->p_ucred, p)) ||
        ((attr.va_mode & 0111) == 0) ||
        (attr.va_type != VREG)) {
        rv = EACCES;
        goto exec_fail;
    }

    /*----------------------------------------------------------------
     * Step 2. Does the file contain a format we can
     * understand and execute
     */
    rv = getheader(p, ndp, attr.va_size, &hdr);
    if (rv > 0)
        goto exec_fail;
    if (rv < 0) {
        FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
        goto again;
    }

    /*----------------------------------------------------------------
     * Step 3.  File and header are valid. Now, dig out the strings
     * out of the old process image.
     */
    /* get space for argv & environment */
    MALLOC(argbuf, char *, ARG_MAX, M_EXEC, M_WAITOK);
    if (! argbuf) {
        /* can't allocate ARG_MAX */
        rv = ENOMEM;
        goto exec_fail;
    }

    rv = copyargs (uap, &hdr, argbuf, &framesz);
    if (rv)
        goto exec_fail;

    /*----------------------------------------------------------------
     * Step 4. Build the new processes image.
     *
     * At this point, we are committed -- destroy old executable!
     */
    vs = p->p_vmspace;

    /* blow away all address space */
    rv = vm_deallocate(&vs->vm_map, 0, USRSTACK);
    if (rv)
        goto exec_abort;

    /* build a new address space */
    /* treat text, data, and bss in terms of integral page size */
    tsize = roundup(hdr.text, NBPG);
    dsize = roundup(hdr.data, NBPG);
    bsize = roundup(hdr.bss, NBPG);

    addr = hdr.virtual_offset;

    /* map text as being read/execute only and demand paged */
    rv = vm_mmap(&vs->vm_map, &addr, tsize,
        VM_PROT_READ | VM_PROT_EXECUTE,
        VM_PROT_DEFAULT, MAP_FIXED | MAP_FILE | MAP_PRIVATE,
        (caddr_t)ndp->ni_vp, hdr.file_offset);
    if (rv)
        goto exec_abort;

    addr = hdr.virtual_offset + tsize;

    /* map data as being read/write and demand paged */
    rv = vm_mmap(&vs->vm_map, &addr, dsize,
        tsize ? (VM_PROT_READ | VM_PROT_WRITE) : VM_PROT_ALL,
        VM_PROT_DEFAULT, MAP_FIXED | MAP_FILE | MAP_PRIVATE,
        (caddr_t)ndp->ni_vp, hdr.file_offset + tsize);
    if (rv)
        goto exec_abort;

    /* create anonymous memory region for bss */
    addr = hdr.virtual_offset + tsize + dsize;
    rv = vm_allocate(&vs->vm_map, &addr, bsize, FALSE);
    if (rv)
        goto exec_abort;

    /* create anonymous memory region for new stack */
    addr = (vm_offset_t) USRSTACK - MAXSSIZ;
    rv = vm_allocate(&vs->vm_map, &addr, MAXSSIZ, FALSE);
    if (rv)
        goto exec_abort;

    /* copy out the stack contents */
    rv = copyout(argbuf, (char*)USRSTACK - framesz, framesz);
    if (rv)
        goto exec_abort;

    /*----------------------------------------------------------------
     * Step 5. Prepare process for execution.
     */

    /* touchup process information -- vm system is unfinished! */
    vs->vm_tsize = tsize / NBPG;                        /* text size (pages) */
    vs->vm_dsize = (dsize + bsize) / NBPG;              /* data size (pages) */
    vs->vm_taddr = (caddr_t) hdr.virtual_offset;        /* virtual address of text */
    vs->vm_daddr = (caddr_t) hdr.virtual_offset + tsize; /* virtual address of data */
    vs->vm_maxsaddr = (caddr_t) USRSTACK - MAXSSIZ;     /* user VA at max stack growth */
    vs->vm_ssize = (framesz + NBPG - 1) / NBPG;         /* stack size (pages) */

    /* close files on exec, fixup signals */
    fdcloseexec(p);
    execsigs(p);

    /* name this process - nameiexec(p, ndp) */
    len = ndp->ni_cnd.cn_namelen;
    if (len > MAXCOMLEN)
        len = MAXCOMLEN;
    bcopy(ndp->ni_cnd.cn_nameptr, p->p_comm, len);
    p->p_comm[len] = 0;

    /* mark as executable, wakeup any process that was vforked and tell
     * it that it now has it's own resources back */
    p->p_flag |= P_EXEC;
    if (p->p_pptr && (p->p_flag & P_PPWAIT)) {
        p->p_flag &= ~P_PPWAIT;
        wakeup(p->p_pptr);
    }

    /* implement set userid/groupid */
    if ((attr.va_mode&VSUID) && (p->p_flag & P_TRACED) == 0) {
        p->p_ucred = crcopy(p->p_ucred);
        p->p_cred->p_svuid = p->p_ucred->cr_uid = attr.va_uid;
    }
    if ((attr.va_mode&VSGID) && (p->p_flag & P_TRACED) == 0) {
        p->p_ucred = crcopy(p->p_ucred);
        p->p_cred->p_svgid = p->p_ucred->cr_groups[0] = attr.va_gid;
    }
    FREE(argbuf, M_EXEC);

    /* setup initial register state */
    p->p_md.md_regs[SP] = USRSTACK - framesz;
    setregs(p, hdr.entry);

    ndp->ni_vp->v_flag |= VTEXT;        /* mark vnode pure text */
    vput(ndp->ni_vp);
    FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);

    /* if tracing process, pass control back to debugger so breakpoints
       can be set before the program "runs" */
    if (p->p_flag & P_TRACED)
        psignal(p, SIGTRAP);
    p->p_acflag &= ~AFORK;      /* remove fork, but no exec flag */
//printf("%s: succeeded\n", __func__);
    return 0;

exec_fail:
    if (argbuf)
        FREE(argbuf, M_EXEC);
    vput(ndp->ni_vp);
    FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
    return rv;

exec_abort:
    /* sorry, no more process anymore. exit gracefully */
    if (argbuf)
        FREE(argbuf, M_EXEC);
    vput(ndp->ni_vp);
    FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
    exit1(p, W_EXITCODE(0, SIGABRT));

    /* NOTREACHED */
    return 0;
}
