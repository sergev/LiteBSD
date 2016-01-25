/*-
 * Copyright (c) 1982, 1988, 1991, 1993
 *  The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
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
 *  @(#)systm.h 8.7 (Berkeley) 3/29/95
 */

/*
 * The `securelevel' variable controls the security level of the system.
 * It can only be decreased by process 1 (/sbin/init).
 *
 * Security levels are as follows:
 *   -1 permannently insecure mode - always run system in level 0 mode.
 *    0 insecure mode - immutable and append-only flags make be turned off.
 *  All devices may be read or written subject to permission modes.
 *    1 secure mode - immutable and append-only flags may not be changed;
 *  raw disks of mounted filesystems, /dev/mem, and /dev/kmem are
 *  read-only.
 *    2 highly secure mode - same as (1) plus raw disks are always
 *  read-only whether mounted or not. This level precludes tampering
 *  with filesystems by unmounting them, but also inhibits running
 *  newfs while the system is secured.
 *
 * In normal operation, the system runs in level 0 mode while single user
 * and in level 1 mode while multiuser. If level 2 mode is desired while
 * running multiuser, it can be set in the multiuser startup script
 * (/etc/rc.local) using sysctl(1). If it is desired to run the system
 * in level 0 mode while multiuser, initialize the variable securelevel
 * in /sys/kern/kern_sysctl.c to -1. Note that it is NOT initialized to
 * zero as that would allow the vmunix binary to be patched to -1.
 * Without initialization, securelevel loads in the BSS area which only
 * comes into existence when the kernel is loaded and hence cannot be
 * patched by a stalking hacker.
 */
extern int securelevel;         /* system security level */
extern const char *panicstr;    /* panic message */
extern char version[];          /* system version */
extern char copyright[];        /* system copyright */

extern int nblkdev;             /* number of entries in bdevsw */
extern int nchrdev;             /* number of entries in cdevsw */
extern int nswdev;              /* number of swap devices */
extern int nswap;               /* size of swap space */

extern int selwait;             /* select timeout address */

extern u_char curpriority;      /* priority of current process */

extern int physmem;             /* physical memory */

extern dev_t dumpdev;           /* dump device */
extern long dumplo;             /* offset into dumpdev */

extern dev_t rootdev;           /* root device */
extern struct vnode *rootvp;    /* vnode equivalent to above */

extern dev_t swapdev;           /* swapping device */
extern struct vnode *swapdev_vp;/* vnode equivalent to above */

extern struct sysent {          /* system call table */
    short   sy_narg;            /* number of args */
    short   sy_argsize;         /* total size of arguments */
    int     (*sy_call)();       /* implementing function */
} sysent[];
extern int nsysent;
#define SCARG(p,k)  ((p)->k.datum)  /* get arg from args pointer */

extern int boothowto;           /* reboot flags, from console subsystem */

#ifndef CBSIZE
#define CBSIZE 1024
#endif

/*
 * General function declarations.
 */
int     nullop __P((void));
int     enodev __P((void));
int     enoioctl __P((void));
int     enxio __P((void));
int     eopnotsupp __P((void));
int     einval __P((void));
int     seltrue __P((dev_t dev, int which, struct proc *p));
void    *hashinit __P((int count, int type, u_long *hashmask));
int     nosys __P((struct proc *, void *, register_t *));

#ifdef __GNUC__
void    panic __P((const char *, ...)) __attribute__((noreturn));
#else
void    panic __P((const char *, ...));
#endif
void    tablefull __P((const char *));
void    addlog __P((const char *, ...));
void    log __P((int, const char *, ...));
void    printf __P((const char *, ...));
int     sprintf __P((char *buf, const char *, ...));
void    ttyprintf __P((struct tty *, const char *, ...));
void    uprintf __P((const char *, ...));

void    bcopy __P((const void *from, void *to, u_int len));
void    ovbcopy __P((const void *from, void *to, u_int len));
void    bzero __P((void *buf, u_int len));

int     copystr __P((void *kfaddr, void *kdaddr, u_int len, u_int *done));
int     copyinstr __P((void *udaddr, void *kaddr, u_int len, u_int *done));
int     copyoutstr __P((void *kaddr, void *udaddr, u_int len, u_int *done));
int     copyin __P((void *udaddr, void *kaddr, u_int len));
int     copyout __P((void *kaddr, void *udaddr, u_int len));

int     fubyte __P((void *base));
#ifdef notdef
int     fuibyte __P((void *base));
#endif
int     subyte __P((void *base, int byte));
int     suibyte __P((void *base, int byte));
int     fuword __P((void *base));
int     fuiword __P((void *base));
int     suword __P((void *base, int word));
int     suiword __P((void *base, int word));

int     hzto __P((struct timeval *tv));
void    timeout __P((void (*func)(void *), void *arg, int ticks));
void    untimeout __P((void (*func)(void *), void *arg));
void    realitexpire __P((void *));

struct clockframe;
void    hardclock __P((struct clockframe *frame));
void    softclock __P((void));
void    statclock __P((struct clockframe *frame));

void    initclocks __P((void));

void    startprofclock __P((struct proc *));
void    stopprofclock __P((struct proc *));
void    setstatclockrate __P((int hzrate));

void    consinit __P((void));
void    kmeminit __P((void));
void    cpu_startup __P((void));
void    rqinit __P((void));
void    vfsinit __P((void));
void    mbinit __P((void));
void    clist_init __P((void));
void    ifinit __P((void));
void    domaininit __P((void));
void    vntblinit __P((void));
void    nchinit __P((void));
void    scheduler __P((void));
void    cpu_initclocks __P((void));
void    resettodr __P((void));

struct execve_args;
struct dup2_args;
struct close_args;
struct socket;
struct stat;
struct mount;
struct filedesc;
struct componentname;
struct vop_generic_args;
struct mbuf;
enum vtype;
int     fork __P((struct proc *, void *, register_t *));
int     execve __P((struct proc *, struct execve_args *, register_t *));
int     sync __P((struct proc *, void *, register_t *));
int     dup2 __P((struct proc *, struct dup2_args *, register_t *));
int     closef __P((struct file *, struct proc *));
int     close __P((struct proc *, struct close_args *, register_t *));
int     soo_stat __P((struct socket *, struct stat *));
int     trace_req __P((struct proc *));
int     cpu_coredump __P((struct proc *, struct vnode *, struct ucred *));
int     sysctl_clockrate __P((char *, size_t *));
int     sysctl_vnode __P((char *, size_t *, struct proc *));
int     uiomove __P((caddr_t, int, struct uio *));
int     fuswintr __P((caddr_t));
int     suswintr __P((caddr_t, int));
int     ureadc __P((int c, struct uio *));
int     strcmp __P((const char *, const char *));
int     dounmount(struct mount *, int, struct proc *);
int     dupfdopen(struct filedesc *, int, int, int, int);
int     vn_writechk(struct vnode *);
int     cttyopen(dev_t, int, int, struct proc *);
int     cttyread(dev_t, struct uio *, int);
int     cttywrite(dev_t, struct uio *, int);
int     cttyioctl(dev_t, u_long, caddr_t, int, struct proc *);
int     cttyselect(dev_t, int, struct proc *);
int     fifo_printinfo(struct vnode *);
int     vfinddev(dev_t, enum vtype, struct vnode **);
int     iskmemdev(dev_t);
int     cache_lookup(struct vnode *, struct vnode **, struct componentname *);
int     groupmember(gid_t, struct ucred *);
int     cpu_fork(struct proc *, struct proc *);
int     iszerodev(dev_t);
int     badaddr(char *, int);
int     physio(void(*)(struct buf *), struct buf *, dev_t, int, u_int(*)(), struct uio *);
int     getsock(struct filedesc *, int, struct file **);
int     null_bypass(struct vop_generic_args *);
int     sockargs(struct mbuf **, caddr_t, int, int);

struct sockaddr;
struct lock;
struct iovec;
enum uio_rw;
void    timevaladd __P((struct timeval *, struct timeval *));
void    timevalsub __P((struct timeval *, struct timeval *));
void    setregs __P((struct proc *, u_long));
void    exit1 __P((struct proc *, int));
void    brelvp __P((struct buf *));
void    remrq __P((struct proc *));
void    cpu_switch __P((struct proc *));
void    cnputc __P((int));
void    boot __P((int));
void    logwakeup __P((void));
void    ffree __P((struct file *));
void    reassignbuf __P((struct buf *, struct vnode *));
void    bgetvp __P((struct vnode *, struct buf *));
void    pagemove __P((caddr_t, caddr_t, int));
void    vwakeup __P((struct buf *));
void    cache_purge(struct vnode *);
void    cache_purgevfs(struct mount *);
void    bremfree(struct buf *);
void    vfs_bufstats(void);
void    pfctlinput(int, struct sockaddr *);
void    inittodr(time_t);
void    cache_enter(struct vnode *, struct vnode *, struct componentname *);
void    lockmgr_printinfo(struct lock *);
void    cpu_swapin(struct proc *);
void    ktrpsig(struct vnode *, int, void(*)(int), int, int);
void    ktrcsw(struct vnode *, int, int);
void    ktrgenio(struct vnode *, int, enum uio_rw, struct iovec *, int, int);
void    ktrnamei(struct vnode *, char *);
void    ktrsyscall(struct vnode *, int, int, register_t []);
void    ktrsysret(struct vnode *, int, int, int);

dev_t   chrtoblk(dev_t);

/*
 * Routines to manipulate queues built from doubly linked lists.
 * These routines expect to be passed pointers to structures which
 * have as their first members a forward pointer and a back pointer.
 */

/*
 * Insert `elem' in the queue immediately `pred'.
 */
static __inline void
insque (e, p)
    caddr_t e, p;
{
    struct qelem {
        struct qelem *forw;
        struct qelem *back;
    };
    register struct qelem *elem = (struct qelem *) e;
    register struct qelem *pred = (struct qelem *) p;

    elem->forw       = pred->forw;
    pred->forw->back = elem;
    elem->back       = pred;
    pred->forw       = elem;
}

/*
 * Remove `elem' from its containing queue.
 */
static __inline void
remque (e)
    caddr_t e;
{
    struct qelem {
        struct qelem *forw;
        struct qelem *back;
    };
    register struct qelem *elem = (struct qelem *) e;

    elem->forw->back = elem->back;
    elem->back->forw = elem->forw;
}

#include <libkern/libkern.h>
