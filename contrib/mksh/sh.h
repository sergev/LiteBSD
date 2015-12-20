/*	$OpenBSD: sh.h,v 1.35 2015/09/10 22:48:58 nicm Exp $	*/
/*	$OpenBSD: shf.h,v 1.6 2005/12/11 18:53:51 deraadt Exp $	*/
/*	$OpenBSD: table.h,v 1.8 2012/02/19 07:52:30 otto Exp $	*/
/*	$OpenBSD: tree.h,v 1.10 2005/03/28 21:28:22 deraadt Exp $	*/
/*	$OpenBSD: expand.h,v 1.7 2015/09/01 13:12:31 tedu Exp $	*/
/*	$OpenBSD: lex.h,v 1.13 2013/03/03 19:11:34 guenther Exp $	*/
/*	$OpenBSD: proto.h,v 1.35 2013/09/04 15:49:19 millert Exp $	*/
/*	$OpenBSD: c_test.h,v 1.4 2004/12/20 11:34:26 otto Exp $	*/
/*	$OpenBSD: tty.h,v 1.5 2004/12/20 11:34:26 otto Exp $	*/

/*-
 * Copyright © 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *	       2011, 2012, 2013, 2014, 2015
 *	mirabilos <m@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un‐
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided “AS IS” and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person’s immediate fault when using the work as intended.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#undef __attribute__
#if HAVE_ATTRIBUTE_BOUNDED
#define MKSH_A_BOUNDED(x,y,z)	__attribute__((__bounded__(x, y, z)))
#else
#define MKSH_A_BOUNDED(x,y,z)	/* nothing */
#endif
#if HAVE_ATTRIBUTE_FORMAT
#define MKSH_A_FORMAT(x,y,z)	__attribute__((__format__(x, y, z)))
#else
#define MKSH_A_FORMAT(x,y,z)	/* nothing */
#endif
#if HAVE_ATTRIBUTE_NORETURN
#define MKSH_A_NORETURN		__attribute__((__noreturn__))
#else
#define MKSH_A_NORETURN		/* nothing */
#endif
#if HAVE_ATTRIBUTE_PURE
#define MKSH_A_PURE		__attribute__((__pure__))
#else
#define MKSH_A_PURE		/* nothing */
#endif
#if HAVE_ATTRIBUTE_UNUSED
#define MKSH_A_UNUSED		__attribute__((__unused__))
#else
#define MKSH_A_UNUSED		/* nothing */
#endif
#if HAVE_ATTRIBUTE_USED
#define MKSH_A_USED		__attribute__((__used__))
#else
#define MKSH_A_USED		/* nothing */
#endif

#if defined(MirBSD) && (MirBSD >= 0x09A1) && \
    defined(__ELF__) && defined(__GNUC__) && \
    !defined(__llvm__) && !defined(__NWCC__)
/*
 * We got usable __IDSTRING __COPYRIGHT __RCSID __SCCSID macros
 * which work for all cases; no need to redefine them using the
 * "portable" macros from below when we might have the "better"
 * gcc+ELF specific macros or other system dependent ones.
 */
#else
#undef __IDSTRING
#undef __IDSTRING_CONCAT
#undef __IDSTRING_EXPAND
#undef __COPYRIGHT
#undef __RCSID
#undef __SCCSID
#define __IDSTRING_CONCAT(l,p)		__LINTED__ ## l ## _ ## p
#define __IDSTRING_EXPAND(l,p)		__IDSTRING_CONCAT(l,p)
#ifdef MKSH_DONT_EMIT_IDSTRING
#define __IDSTRING(prefix, string)	/* nothing */
#else
#define __IDSTRING(prefix, string)				\
	static const char __IDSTRING_EXPAND(__LINE__,prefix) []	\
	    MKSH_A_USED = "@(""#)" #prefix ": " string
#endif
#define __COPYRIGHT(x)		__IDSTRING(copyright,x)
#define __RCSID(x)		__IDSTRING(rcsid,x)
#define __SCCSID(x)		__IDSTRING(sccsid,x)
#endif

#ifdef EXTERN
__RCSID("$MirOS: src/bin/mksh/sh.h,v 1.751 2015/12/12 22:25:15 tg Exp $");
#endif
#define MKSH_VERSION "R52 2015/12/12"

/* arithmetic types: C implementation */
#if !HAVE_CAN_INTTYPES
#if !HAVE_CAN_UCBINTS
typedef signed int int32_t;
typedef unsigned int uint32_t;
#else
typedef u_int32_t uint32_t;
#endif
#endif

/* arithmetic types: shell arithmetics */
#ifdef MKSH_LEGACY_MODE
/*
 * POSIX demands these to be the C environment's long type
 */
typedef long mksh_ari_t;
typedef unsigned long mksh_uari_t;
#else
/*
 * These types are exactly 32 bit wide; signed and unsigned
 * integer wraparound, even across division and modulo, for
 * any shell code using them, is guaranteed.
 */
typedef int32_t mksh_ari_t;
typedef uint32_t mksh_uari_t;
#endif

/* boolean type (no <stdbool.h> deliberately) */
typedef unsigned char mksh_bool;
#undef bool
/* false MUST equal the same 0 as written by static storage initialisation */
#undef false
#undef true
/* access macros for boolean type */
#define bool		mksh_bool
/* values must have identity mapping between mksh_bool and short */
#define false		0
#define true		1
/* make any-type into bool or short */
#define tobool(cond)	((cond) ? true : false)

/* char (octet) type: C implementation */
#if !HAVE_CAN_INT8TYPE
#if !HAVE_CAN_UCBINT8
typedef unsigned char uint8_t;
#else
typedef u_int8_t uint8_t;
#endif
#endif

/* other standard types */

#if !HAVE_RLIM_T
typedef unsigned long rlim_t;
#endif

#if !HAVE_SIG_T
#undef sig_t
typedef void (*sig_t)(int);
#endif

#ifdef MKSH_TYPEDEF_SIG_ATOMIC_T
typedef MKSH_TYPEDEF_SIG_ATOMIC_T sig_atomic_t;
#endif

#ifdef MKSH_TYPEDEF_SSIZE_T
typedef MKSH_TYPEDEF_SSIZE_T ssize_t;
#endif

/* un-do vendor damage */

#undef BAD		/* AIX defines that somewhere */
#undef PRINT		/* LynxOS defines that somewhere */
#undef flock		/* SCO UnixWare defines that to flock64 but ENOENT */


#ifndef MKSH_INCLUDES_ONLY

/* extra types */

/* getrusage does not exist on OS/2 kLIBC */
#if !HAVE_GETRUSAGE && !defined(__OS2__)
#undef rusage
#undef RUSAGE_SELF
#undef RUSAGE_CHILDREN
#define rusage mksh_rusage
#define RUSAGE_SELF		0
#define RUSAGE_CHILDREN		-1

struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
};
#endif

/* extra macros */

#ifndef timerclear
#define timerclear(tvp)							\
	do {								\
		(tvp)->tv_sec = (tvp)->tv_usec = 0;			\
	} while (/* CONSTCOND */ 0)
#endif
#ifndef timeradd
#define timeradd(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;	\
		if ((vvp)->tv_usec >= 1000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_usec -= 1000000;			\
		}							\
	} while (/* CONSTCOND */ 0)
#endif
#ifndef timersub
#define timersub(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
		if ((vvp)->tv_usec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_usec += 1000000;			\
		}							\
	} while (/* CONSTCOND */ 0)
#endif

#ifdef MKSH__NO_PATH_MAX
#undef PATH_MAX
#else
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else
#define PATH_MAX	1024
#endif
#endif
#endif
#ifndef SIZE_MAX
#ifdef SIZE_T_MAX
#define SIZE_MAX	SIZE_T_MAX
#else
#define SIZE_MAX	((size_t)-1)
#endif
#endif
#ifndef S_ISLNK
#define S_ISLNK(m)	((m & 0170000) == 0120000)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)	((m & 0170000) == 0140000)
#endif
#if !defined(S_ISCDF) && defined(S_CDF)
#define S_ISCDF(m)	(S_ISDIR(m) && ((m) & S_CDF))
#endif
#ifndef DEFFILEMODE
#define DEFFILEMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif


/* determine ksh_NSIG: first, use the traditional definitions */
#undef ksh_NSIG
#if defined(NSIG)
#define ksh_NSIG (NSIG)
#elif defined(_NSIG)
#define ksh_NSIG (_NSIG)
#elif defined(SIGMAX)
#define ksh_NSIG (SIGMAX + 1)
#elif defined(_SIGMAX)
#define ksh_NSIG (_SIGMAX + 1)
#elif defined(NSIG_MAX)
#define ksh_NSIG (NSIG_MAX)
#else
# error Please have your platform define NSIG.
#endif
/* range-check them */
#if (ksh_NSIG < 1)
# error Your NSIG value is not positive.
#undef ksh_NSIG
#endif
/* second, see if the new POSIX definition is available */
#ifdef NSIG_MAX
#if (NSIG_MAX < 2)
/* and usable */
# error Your NSIG_MAX value is too small.
#undef NSIG_MAX
#elif (ksh_NSIG > NSIG_MAX)
/* and realistic */
# error Your NSIG value is larger than your NSIG_MAX value.
#undef NSIG_MAX
#else
/* since it’s usable, prefer it */
#undef ksh_NSIG
#define ksh_NSIG (NSIG_MAX)
#endif
/* if NSIG_MAX is now still defined, use sysconf(_SC_NSIG) at runtime */
#endif
/* third, for cpp without the error directive, default */
#ifndef ksh_NSIG
#define ksh_NSIG 64
#endif


/* OS-dependent additions (functions, variables, by OS) */

#ifdef MKSH_EXE_EXT
#undef MKSH_EXE_EXT
#define MKSH_EXE_EXT	".exe"
#else
#define MKSH_EXE_EXT	""
#endif

#ifdef __OS2__
#define MKSH_PATHSEPS	";"
#define MKSH_PATHSEPC	';'
#define MKSH_UNIXROOT	"/@unixroot"
#else
#define MKSH_PATHSEPS	":"
#define MKSH_PATHSEPC	':'
#define MKSH_UNIXROOT	""
#endif

#if !HAVE_FLOCK_DECL
extern int flock(int, int);
#endif

#if !HAVE_GETTIMEOFDAY
#define mksh_TIME(tv) do {		\
	(tv).tv_usec = 0;		\
	(tv).tv_sec = time(NULL);	\
} while (/* CONSTCOND */ 0)
#else
#define mksh_TIME(tv) gettimeofday(&(tv), NULL)
#endif

#if !HAVE_GETRUSAGE
extern int getrusage(int, struct rusage *);
#endif

#if !HAVE_MEMMOVE
/* we assume either memmove or bcopy exist, at the moment */
#define memmove(dst, src, len)	bcopy((src), (dst), (len))
#endif

#if !HAVE_REVOKE_DECL
extern int revoke(const char *);
#endif

#if defined(DEBUG) || !HAVE_STRERROR
#undef strerror
#define strerror		/* poisoned */ dontuse_strerror
#define cstrerror		/* replaced */ cstrerror
extern const char *cstrerror(int);
#else
#define cstrerror(errnum)	((const char *)strerror(errnum))
#endif

#if !HAVE_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif

#ifdef __INTERIX
/* XXX imake style */
#define makedev mkdev
extern int __cdecl seteuid(uid_t);
extern int __cdecl setegid(gid_t);
#endif

#if defined(__COHERENT__)
#ifndef O_ACCMODE
/* this need not work everywhere, take care */
#define O_ACCMODE	(O_RDONLY | O_WRONLY | O_RDWR)
#endif
#endif

#ifndef O_BINARY
#define O_BINARY	0
#endif

#ifdef MKSH__NO_SYMLINK
#undef S_ISLNK
#define S_ISLNK(m)	(/* CONSTCOND */ 0)
#define mksh_lstat	stat
#else
#define mksh_lstat	lstat
#endif

#define mksh_ttyst	struct termios
#define mksh_tcget(fd,st) tcgetattr((fd), (st))
#define mksh_tcset(fd,st) tcsetattr((fd), TCSADRAIN, (st))

#ifndef ISTRIP
#define ISTRIP		0
#endif


/* some useful #defines */
#ifdef EXTERN
# define E_INIT(i) = i
#else
# define E_INIT(i)
# define EXTERN extern
# define EXTERN_DEFINED
#endif

/* define bit in flag */
#define BIT(i)		(1 << (i))
#define NELEM(a)	(sizeof(a) / sizeof((a)[0]))

/*
 * Make MAGIC a char that might be printed to make bugs more obvious, but
 * not a char that is used often. Also, can't use the high bit as it causes
 * portability problems (calling strchr(x, 0x80 | 'x') is error prone).
 */
#define MAGIC		(7)	/* prefix for *?[!{,} during expand */
#define ISMAGIC(c)	((unsigned char)(c) == MAGIC)

EXTERN const char *safe_prompt; /* safe prompt if PS1 substitution fails */

#ifdef MKSH_LEGACY_MODE
#define KSH_VERSIONNAME	"LEGACY"
#else
#define KSH_VERSIONNAME	"MIRBSD"
#endif
EXTERN const char initvsn[] E_INIT("KSH_VERSION=@(#)" KSH_VERSIONNAME \
    " KSH " MKSH_VERSION);
#define KSH_VERSION	(initvsn + /* "KSH_VERSION=@(#)" */ 16)

EXTERN const char digits_uc[] E_INIT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
EXTERN const char digits_lc[] E_INIT("0123456789abcdefghijklmnopqrstuvwxyz");
#define letters_uc (digits_uc + 10)
#define letters_lc (digits_lc + 10)

/*
 * Evil hack for const correctness due to API brokenness
 */
union mksh_cchack {
	char *rw;
	const char *ro;
};
union mksh_ccphack {
	char **rw;
	const char **ro;
};

/*
 * Evil hack since casting uint to sint is implementation-defined
 */
typedef union {
	mksh_ari_t i;
	mksh_uari_t u;
} mksh_ari_u;

/* for const debugging */
#if defined(DEBUG) && defined(__GNUC__) && !defined(__ICC) && \
    !defined(__INTEL_COMPILER) && !defined(__SUNPRO_C)
char *ucstrchr(char *, int);
char *ucstrstr(char *, const char *);
#undef strchr
#define strchr ucstrchr
#define strstr ucstrstr
#define cstrchr(s,c) ({			\
	union mksh_cchack in, out;	\
					\
	in.ro = (s);			\
	out.rw = ucstrchr(in.rw, (c));	\
	(out.ro);			\
})
#define cstrstr(b,l) ({			\
	union mksh_cchack in, out;	\
					\
	in.ro = (b);			\
	out.rw = ucstrstr(in.rw, (l));	\
	(out.ro);			\
})
#define vstrchr(s,c)	(cstrchr((s), (c)) != NULL)
#define vstrstr(b,l)	(cstrstr((b), (l)) != NULL)
#else /* !DEBUG, !gcc */
#define cstrchr(s,c)	((const char *)strchr((s), (c)))
#define cstrstr(s,c)	((const char *)strstr((s), (c)))
#define vstrchr(s,c)	(strchr((s), (c)) != NULL)
#define vstrstr(b,l)	(strstr((b), (l)) != NULL)
#endif

#if defined(DEBUG) || defined(__COVERITY__)
#define mkssert(e)	do { if (!(e)) exit(255); } while (/* CONSTCOND */ 0)
#ifndef DEBUG_LEAKS
#define DEBUG_LEAKS
#endif
#else
#define mkssert(e)	do { } while (/* CONSTCOND */ 0)
#endif

/* use this ipv strchr(s, 0) but no side effects in s! */
#define strnul(s)	((s) + strlen(s))

#define utf_ptradjx(src, dst) do {					\
	(dst) = (src) + utf_ptradj(src);				\
} while (/* CONSTCOND */ 0)

#if defined(MKSH_SMALL) && !defined(MKSH_SMALL_BUT_FAST)
#define strdupx(d, s, ap) do {						\
	(d) = strdup_i((s), (ap));					\
} while (/* CONSTCOND */ 0)
#define strndupx(d, s, n, ap) do {					\
	(d) = strndup_i((s), (n), (ap));				\
} while (/* CONSTCOND */ 0)
#else
/* be careful to evaluate arguments only once! */
#define strdupx(d, s, ap) do {						\
	const char *strdup_src = (s);					\
	char *strdup_dst = NULL;					\
									\
	if (strdup_src != NULL) {					\
		size_t strdup_len = strlen(strdup_src) + 1;		\
		strdup_dst = alloc(strdup_len, (ap));			\
		memcpy(strdup_dst, strdup_src, strdup_len);		\
	}								\
	(d) = strdup_dst;						\
} while (/* CONSTCOND */ 0)
#define strndupx(d, s, n, ap) do {					\
	const char *strdup_src = (s);					\
	char *strdup_dst = NULL;					\
									\
	if (strdup_src != NULL) {					\
		size_t strndup_len = (n);				\
		strdup_dst = alloc(strndup_len + 1, (ap));		\
		memcpy(strdup_dst, strdup_src, strndup_len);		\
		strdup_dst[strndup_len] = '\0';				\
	}								\
	(d) = strdup_dst;						\
} while (/* CONSTCOND */ 0)
#endif

#ifdef MKSH_LEGACY_MODE
#ifndef MKSH_NO_CMDLINE_EDITING
#define MKSH_NO_CMDLINE_EDITING	/* defined */
#endif
#ifndef MKSH_CONSERVATIVE_FDS
#define MKSH_CONSERVATIVE_FDS	/* defined */
#endif
#undef MKSH_S_NOVI
#define MKSH_S_NOVI		1
#endif

#ifdef MKSH_SMALL
#ifndef MKSH_CONSERVATIVE_FDS
#define MKSH_CONSERVATIVE_FDS	/* defined */
#endif
#ifndef MKSH_NOPWNAM
#define MKSH_NOPWNAM		/* defined */
#endif
#ifndef MKSH_S_NOVI
#define MKSH_S_NOVI		1
#endif
#endif

#ifndef MKSH_S_NOVI
#define MKSH_S_NOVI		0
#endif

#if defined(MKSH_NOPROSPECTOFWORK) && !defined(MKSH_UNEMPLOYED)
#define MKSH_UNEMPLOYED		1
#endif

/* these shall be smaller than 100 */
#ifdef MKSH_CONSERVATIVE_FDS
#define NUFILE		32	/* Number of user-accessible files */
#define FDBASE		10	/* First file usable by Shell */
#else
#define NUFILE		56	/* Number of user-accessible files */
#define FDBASE		24	/* First file usable by Shell */
#endif

/*
 * simple grouping allocator
 */


/* 0. OS API: where to get memory from and how to free it (grouped) */

/* malloc(3)/realloc(3) -> free(3) for use by the memory allocator */
#define malloc_osi(sz)		malloc(sz)
#define realloc_osi(p,sz)	realloc((p), (sz))
#define free_osimalloc(p)	free(p)

/* malloc(3)/realloc(3) -> free(3) for use by mksh code */
#define malloc_osfunc(sz)	malloc(sz)
#define realloc_osfunc(p,sz)	realloc((p), (sz))
#define free_osfunc(p)		free(p)

#if HAVE_MKNOD
/* setmode(3) -> free(3) */
#define free_ossetmode(p)	free(p)
#endif

#ifdef MKSH__NO_PATH_MAX
/* GNU libc: get_current_dir_name(3) -> free(3) */
#define free_gnu_gcdn(p)	free(p)
#endif


/* 1. internal structure */
struct lalloc {
	struct lalloc *next;
};

/* 2. sizes */
#define ALLOC_ITEM	struct lalloc
#define ALLOC_SIZE	(sizeof(ALLOC_ITEM))

/* 3. group structure (only the same for lalloc.c) */
typedef struct lalloc Area;


EXTERN Area aperm;		/* permanent object space */
#define APERM	&aperm
#define ATEMP	&e->area

/*
 * flags (the order of these enums MUST match the order in misc.c(options[]))
 */
enum sh_flag {
#define SHFLAGS_ENUMS
#include "sh_flags.gen"
	FNFLAGS		/* (place holder: how many flags are there) */
};

#define Flag(f)	(shell_flags[(int)(f)])
#define UTFMODE	Flag(FUNICODE)

/*
 * parsing & execution environment
 *
 * note that kshlongjmp MUST NOT be passed 0 as second argument!
 */
#ifdef MKSH_NO_SIGSETJMP
#define kshjmp_buf	jmp_buf
#define kshsetjmp(jbuf)	_setjmp(jbuf)
#define kshlongjmp	_longjmp
#else
#define kshjmp_buf	sigjmp_buf
#define kshsetjmp(jbuf)	sigsetjmp((jbuf), 0)
#define kshlongjmp	siglongjmp
#endif

struct sretrace_info;
struct yyrecursive_state;

EXTERN struct sretrace_info *retrace_info E_INIT(NULL);
EXTERN int subshell_nesting_type E_INIT(0);

extern struct env {
	ALLOC_ITEM alloc_INT;	/* internal, do not touch */
	Area area;		/* temporary allocation area */
	struct env *oenv;	/* link to previous environment */
	struct block *loc;	/* local variables and functions */
	short *savefd;		/* original redirected fds */
	struct temp *temps;	/* temp files */
	/* saved parser recursion state */
	struct yyrecursive_state *yyrecursive_statep;
	kshjmp_buf jbuf;	/* long jump back to env creator */
	uint8_t type;		/* environment type - see below */
	uint8_t flags;		/* EF_* */
} *e;

/* struct env.type values */
#define E_NONE	0	/* dummy environment */
#define E_PARSE	1	/* parsing command # */
#define E_FUNC	2	/* executing function # */
#define E_INCL	3	/* including a file via . # */
#define E_EXEC	4	/* executing command tree */
#define E_LOOP	5	/* executing for/while # */
#define E_ERRH	6	/* general error handler # */
#define E_GONE	7	/* hidden in child */
/* # indicates env has valid jbuf (see unwind()) */

/* struct env.flag values */
#define EF_BRKCONT_PASS	BIT(1)	/* set if E_LOOP must pass break/continue on */
#define EF_FAKE_SIGDIE	BIT(2)	/* hack to get info from unwind to quitenv */

/* Do breaks/continues stop at env type e? */
#define STOP_BRKCONT(t)	((t) == E_NONE || (t) == E_PARSE || \
			    (t) == E_FUNC || (t) == E_INCL)
/* Do returns stop at env type e? */
#define STOP_RETURN(t)	((t) == E_FUNC || (t) == E_INCL)

/* values for kshlongjmp(e->jbuf, i) */
/* note that i MUST NOT be zero */
#define LRETURN	1	/* return statement */
#define LEXIT	2	/* exit statement */
#define LERROR	3	/* errorf() called */
#define LLEAVE	4	/* untrappable exit/error */
#define LINTR	5	/* ^C noticed */
#define LBREAK	6	/* break statement */
#define LCONTIN	7	/* continue statement */
#define LSHELL	8	/* return to interactive shell() */
#define LAEXPR	9	/* error in arithmetic expression */

/* sort of shell global state */
EXTERN pid_t procpid;		/* PID of executing process */
EXTERN int exstat;		/* exit status */
EXTERN int subst_exstat;	/* exit status of last $(..)/`..` */
EXTERN struct tbl *vp_pipest;	/* global PIPESTATUS array */
EXTERN short trap_exstat;	/* exit status before running a trap */
EXTERN uint8_t trap_nested;	/* running nested traps */
EXTERN uint8_t shell_flags[FNFLAGS];
EXTERN const char *kshname;	/* $0 */
EXTERN struct {
	uid_t kshuid_v;		/* real UID of shell */
	uid_t ksheuid_v;	/* effective UID of shell */
	gid_t kshgid_v;		/* real GID of shell */
	gid_t kshegid_v;	/* effective GID of shell */
	pid_t kshpgrp_v;	/* process group of shell */
	pid_t kshppid_v;	/* PID of parent of shell */
	pid_t kshpid_v;		/* $$, shell PID */
} rndsetupstate;

#define kshpid		rndsetupstate.kshpid_v
#define kshpgrp		rndsetupstate.kshpgrp_v
#define kshuid		rndsetupstate.kshuid_v
#define ksheuid		rndsetupstate.ksheuid_v
#define kshgid		rndsetupstate.kshgid_v
#define kshegid		rndsetupstate.kshegid_v
#define kshppid		rndsetupstate.kshppid_v


/* option processing */
#define OF_CMDLINE	0x01	/* command line */
#define OF_SET		0x02	/* set builtin */
#define OF_SPECIAL	0x04	/* a special variable changing */
#define OF_INTERNAL	0x08	/* set internally by shell */
#define OF_FIRSTTIME	0x10	/* as early as possible, once */
#define OF_ANY		(OF_CMDLINE | OF_SET | OF_SPECIAL | OF_INTERNAL)

/* null value for variable; comparison pointer for unset */
EXTERN char null[] E_INIT("");
/* helpers for string pooling */
EXTERN const char Tintovfl[] E_INIT("integer overflow %zu %c %zu prevented");
EXTERN const char Toomem[] E_INIT("can't allocate %zu data bytes");
#if defined(__GNUC__)
/* trust this to have string pooling; -Wformat bitches otherwise */
#define Tsynerr		"syntax error"
#else
EXTERN const char Tsynerr[] E_INIT("syntax error");
#endif
EXTERN const char Tselect[] E_INIT("select");
EXTERN const char T_typeset[] E_INIT("=typeset");
#define Ttypeset	(T_typeset + 1)		/* "typeset" */
EXTERN const char Talias[] E_INIT("alias");
EXTERN const char Tunalias[] E_INIT("unalias");
EXTERN const char Tcat[] E_INIT("cat");
#ifdef __OS2__
EXTERN const char Textproc[] E_INIT("extproc");
#endif
#ifdef MKSH_PRINTF_BUILTIN
EXTERN const char Tprintf[] E_INIT("printf");
#endif
EXTERN const char Tsgset[] E_INIT("*=set");
#define Tset		(Tsgset + 2)		/* "set" */
EXTERN const char Tsgexport[] E_INIT("*=export");
#define Texport		(Tsgexport + 2)		/* "export" */
EXTERN const char Tsgreadonly[] E_INIT("*=readonly");
#define Treadonly	(Tsgreadonly + 2)	/* "readonly" */
EXTERN const char Tgbuiltin[] E_INIT("=builtin");
#define Tbuiltin	(Tgbuiltin + 1)		/* "builtin" */
EXTERN const char T_function[] E_INIT(" function");
#define Tfunction	(T_function + 1)	/* "function" */
EXTERN const char TC_LEX1[] E_INIT("|&;<>() \t\n");
#define TC_IFSWS	(TC_LEX1 + 7)		/* space tab newline */

typedef uint8_t Temp_type;
/* expanded heredoc */
#define TT_HEREDOC_EXP	0
/* temporary file used for history editing (fc -e) */
#define TT_HIST_EDIT	1
/* temporary file used during in-situ command substitution */
#define TT_FUNSUB	2

/* temp/heredoc files. The file is removed when the struct is freed. */
struct temp {
	struct temp *next;
	struct shf *shf;
	/* pid of process parsed here-doc */
	pid_t pid;
	Temp_type type;
	/* actually longer: name (variable length) */
	char tffn[3];
};

/*
 * stdio and our IO routines
 */

#define shl_xtrace	(&shf_iob[0])	/* for set -x */
#define shl_stdout	(&shf_iob[1])
#define shl_out		(&shf_iob[2])
#ifdef DF
#define shl_dbg		(&shf_iob[3])	/* for DF() */
#endif
EXTERN bool shl_stdout_ok;

/*
 * trap handlers
 */
typedef struct trap {
	const char *name;	/* short name */
	const char *mess;	/* descriptive name */
	char *trap;		/* trap command */
	sig_t cursig;		/* current handler (valid if TF_ORIG_* set) */
	sig_t shtrap;		/* shell signal handler */
	int signal;		/* signal number */
	int flags;		/* TF_* */
	volatile sig_atomic_t set; /* trap pending */
} Trap;

/* values for Trap.flags */
#define TF_SHELL_USES	BIT(0)	/* shell uses signal, user can't change */
#define TF_USER_SET	BIT(1)	/* user has (tried to) set trap */
#define TF_ORIG_IGN	BIT(2)	/* original action was SIG_IGN */
#define TF_ORIG_DFL	BIT(3)	/* original action was SIG_DFL */
#define TF_EXEC_IGN	BIT(4)	/* restore SIG_IGN just before exec */
#define TF_EXEC_DFL	BIT(5)	/* restore SIG_DFL just before exec */
#define TF_DFL_INTR	BIT(6)	/* when received, default action is LINTR */
#define TF_TTY_INTR	BIT(7)	/* tty generated signal (see j_waitj) */
#define TF_CHANGED	BIT(8)	/* used by runtrap() to detect trap changes */
#define TF_FATAL	BIT(9)	/* causes termination if not trapped */

/* values for setsig()/setexecsig() flags argument */
#define SS_RESTORE_MASK	0x3	/* how to restore a signal before an exec() */
#define SS_RESTORE_CURR	0	/* leave current handler in place */
#define SS_RESTORE_ORIG	1	/* restore original handler */
#define SS_RESTORE_DFL	2	/* restore to SIG_DFL */
#define SS_RESTORE_IGN	3	/* restore to SIG_IGN */
#define SS_FORCE	BIT(3)	/* set signal even if original signal ignored */
#define SS_USER		BIT(4)	/* user is doing the set (ie, trap command) */
#define SS_SHTRAP	BIT(5)	/* trap for internal use (ALRM, CHLD, WINCH) */

#define ksh_SIGEXIT 0		/* for trap EXIT */
#define ksh_SIGERR  ksh_NSIG	/* for trap ERR */

EXTERN volatile sig_atomic_t trap;	/* traps pending? */
EXTERN volatile sig_atomic_t intrsig;	/* pending trap interrupts command */
EXTERN volatile sig_atomic_t fatal_trap; /* received a fatal signal */
extern Trap sigtraps[ksh_NSIG + 1];

/* got_winch = 1 when we need to re-adjust the window size */
#ifdef SIGWINCH
EXTERN volatile sig_atomic_t got_winch E_INIT(1);
#else
#define got_winch	true
#endif

/*
 * TMOUT support
 */
/* values for ksh_tmout_state */
enum tmout_enum {
	TMOUT_EXECUTING = 0,	/* executing commands */
	TMOUT_READING,		/* waiting for input */
	TMOUT_LEAVING		/* have timed out */
};
EXTERN unsigned int ksh_tmout;
EXTERN enum tmout_enum ksh_tmout_state E_INIT(TMOUT_EXECUTING);

/* For "You have stopped jobs" message */
EXTERN bool really_exit;

/*
 * fast character classes
 */
#define C_ALPHA	 BIT(0)		/* a-z_A-Z */
#define C_DIGIT	 BIT(1)		/* 0-9 */
#define C_LEX1	 BIT(2)		/* \t \n\0|&;<>() */
#define C_VAR1	 BIT(3)		/* *@#!$-? */
#define C_IFSWS	 BIT(4)		/* \t \n (IFS white space) */
#define C_SUBOP1 BIT(5)		/* "=-+?" */
#define C_QUOTE	 BIT(6)		/* \t\n "#$&'()*;<=>?[\]`| (needing quoting) */
#define C_IFS	 BIT(7)		/* $IFS */
#define C_SUBOP2 BIT(8)		/* "#%" (magic, see below) */

extern unsigned char chtypes[];

#define ctype(c, t)	tobool( ((t) == C_SUBOP2) ?			\
			    (((c) == '#' || (c) == '%') ? 1 : 0) :	\
			    (chtypes[(unsigned char)(c)] & (t)) )
#define ord(c)		((int)(unsigned char)(c))
#define ksh_isalphx(c)	ctype((c), C_ALPHA)
#define ksh_isalnux(c)	ctype((c), C_ALPHA | C_DIGIT)
#define ksh_isdigit(c)	(((c) >= '0') && ((c) <= '9'))
#define ksh_islower(c)	(((c) >= 'a') && ((c) <= 'z'))
#define ksh_isupper(c)	(((c) >= 'A') && ((c) <= 'Z'))
#define ksh_tolower(c)	(ksh_isupper(c) ? (c) - 'A' + 'a' : (c))
#define ksh_toupper(c)	(ksh_islower(c) ? (c) - 'a' + 'A' : (c))
#define ksh_isdash(s)	(((s)[0] == '-') && ((s)[1] == '\0'))
#define ksh_isspace(c)	((((c) >= 0x09) && ((c) <= 0x0D)) || ((c) == 0x20))
#define ksh_eq(c,u,l)	(((c) | 0x20) == (l))
#define ksh_numdig(c)	((c) - ord('0'))
#define ksh_numuc(c)	((c) - ord('A'))
#define ksh_numlc(c)	((c) - ord('a'))

EXTERN int ifs0 E_INIT(' ');	/* for "$*" */

/* Argument parsing for built-in commands and getopts command */

/* Values for Getopt.flags */
#define GF_ERROR	BIT(0)	/* call errorf() if there is an error */
#define GF_PLUSOPT	BIT(1)	/* allow +c as an option */
#define GF_NONAME	BIT(2)	/* don't print argv[0] in errors */

/* Values for Getopt.info */
#define GI_MINUS	BIT(0)	/* an option started with -... */
#define GI_PLUS		BIT(1)	/* an option started with +... */
#define GI_MINUSMINUS	BIT(2)	/* arguments were ended with -- */

/* in case some OS defines these */
#undef optarg
#undef optind

typedef struct {
	const char *optarg;
	int optind;
	int uoptind;		/* what user sees in $OPTIND */
	int flags;		/* see GF_* */
	int info;		/* see GI_* */
	unsigned int p;		/* 0 or index into argv[optind - 1] */
	char buf[2];		/* for bad option OPTARG value */
} Getopt;

EXTERN Getopt builtin_opt;	/* for shell builtin commands */
EXTERN Getopt user_opt;		/* parsing state for getopts builtin command */

/* This for co-processes */

/* something that won't (realisticly) wrap */
typedef int Coproc_id;

struct coproc {
	void *job;	/* 0 or job of co-process using input pipe */
	int read;	/* pipe from co-process's stdout */
	int readw;	/* other side of read (saved temporarily) */
	int write;	/* pipe to co-process's stdin */
	int njobs;	/* number of live jobs using output pipe */
	Coproc_id id;	/* id of current output pipe */
};
EXTERN struct coproc coproc;

#ifndef MKSH_NOPROSPECTOFWORK
/* used in jobs.c and by coprocess stuff in exec.c and select() calls */
EXTERN sigset_t		sm_default, sm_sigchld;
#endif

/* name of called builtin function (used by error functions) */
EXTERN const char *builtin_argv0;
/* is called builtin SPEC_BI? (also KEEPASN, odd use though) */
EXTERN bool builtin_spec;

/* current working directory */
EXTERN char	*current_wd;

/* input line size */
#define LINE		(4096 - ALLOC_SIZE)
/*
 * Minimum required space to work with on a line - if the prompt leaves
 * less space than this on a line, the prompt is truncated.
 */
#define MIN_EDIT_SPACE	7
/*
 * Minimum allowed value for x_cols: 2 for prompt, 3 for " < " at end of line
 */
#define MIN_COLS	(2 + MIN_EDIT_SPACE + 3)
#define MIN_LINS	3
EXTERN mksh_ari_t x_cols E_INIT(80);	/* tty columns */
EXTERN mksh_ari_t x_lins E_INIT(24);	/* tty lines */


/* Determine the location of the system (common) profile */

#ifndef MKSH_DEFAULT_PROFILEDIR
#define MKSH_DEFAULT_PROFILEDIR	"/etc"
#endif

#define MKSH_SYSTEM_PROFILE	MKSH_DEFAULT_PROFILEDIR "/profile"
#define MKSH_SUID_PROFILE	MKSH_DEFAULT_PROFILEDIR "/suid_profile"


/* Used by v_evaluate() and setstr() to control action when error occurs */
#define KSH_UNWIND_ERROR	0	/* unwind the stack (kshlongjmp) */
#define KSH_RETURN_ERROR	1	/* return 1/0 for success/failure */

/*
 * Shell file I/O routines
 */

#define SHF_BSIZE		512

#define shf_fileno(shf)		((shf)->fd)
#define shf_setfileno(shf,nfd)	((shf)->fd = (nfd))
#define shf_getc_i(shf)		((shf)->rnleft > 0 ? \
				    (shf)->rnleft--, *(shf)->rp++ : \
				    shf_getchar(shf))
#define shf_putc_i(c, shf)	((shf)->wnleft == 0 ? \
				    shf_putchar((c), (shf)) : \
				    ((shf)->wnleft--, *(shf)->wp++ = (c)))
#define shf_eof(shf)		((shf)->flags & SHF_EOF)
#define shf_error(shf)		((shf)->flags & SHF_ERROR)
#define shf_errno(shf)		((shf)->errnosv)
#define shf_clearerr(shf)	((shf)->flags &= ~(SHF_EOF | SHF_ERROR))

/* Flags passed to shf_*open() */
#define SHF_RD		0x0001
#define SHF_WR		0x0002
#define SHF_RDWR	(SHF_RD|SHF_WR)
#define SHF_ACCMODE	0x0003		/* mask */
#define SHF_GETFL	0x0004		/* use fcntl() to figure RD/WR flags */
#define SHF_UNBUF	0x0008		/* unbuffered I/O */
#define SHF_CLEXEC	0x0010		/* set close on exec flag */
#define SHF_MAPHI	0x0020		/* make fd > FDBASE (and close orig)
					 * (shf_open() only) */
#define SHF_DYNAMIC	0x0040		/* string: increase buffer as needed */
#define SHF_INTERRUPT	0x0080		/* EINTR in read/write causes error */
/* Flags used internally */
#define SHF_STRING	0x0100		/* a string, not a file */
#define SHF_ALLOCS	0x0200		/* shf and shf->buf were alloc()ed */
#define SHF_ALLOCB	0x0400		/* shf->buf was alloc()ed */
#define SHF_ERROR	0x0800		/* read()/write() error */
#define SHF_EOF		0x1000		/* read eof (sticky) */
#define SHF_READING	0x2000		/* currently reading: rnleft,rp valid */
#define SHF_WRITING	0x4000		/* currently writing: wnleft,wp valid */


struct shf {
	Area *areap;		/* area shf/buf were allocated in */
	unsigned char *rp;	/* read: current position in buffer */
	unsigned char *wp;	/* write: current position in buffer */
	unsigned char *buf;	/* buffer */
	ssize_t bsize;		/* actual size of buf */
	ssize_t rbsize;		/* size of buffer (1 if SHF_UNBUF) */
	ssize_t rnleft;		/* read: how much data left in buffer */
	ssize_t wbsize;		/* size of buffer (0 if SHF_UNBUF) */
	ssize_t wnleft;		/* write: how much space left in buffer */
	int flags;		/* see SHF_* */
	int fd;			/* file descriptor */
	int errnosv;		/* saved value of errno after error */
};

extern struct shf shf_iob[];

struct table {
	Area *areap;		/* area to allocate entries */
	struct tbl **tbls;	/* hashed table items */
	size_t nfree;		/* free table entries */
	uint8_t tshift;		/* table size (2^tshift) */
};

/* table item */
struct tbl {
	/* Area to allocate from */
	Area *areap;
	/* value */
	union {
		char *s;			/* string */
		mksh_ari_t i;			/* integer */
		mksh_uari_t u;			/* unsigned integer */
		int (*f)(const char **);	/* built-in command */
		struct op *t;			/* "function" tree */
	} val;
	union {
		struct tbl *array;	/* array values */
		const char *fpath;	/* temporary path to undef function */
	} u;
	union {
		int field;		/* field with for -L/-R/-Z */
		int errnov;		/* CEXEC/CTALIAS */
	} u2;
	union {
		uint32_t hval;		/* hash(name) */
		uint32_t index;		/* index for an array */
	} ua;
	/*
	 * command type (see below), base (if INTEGER),
	 * offset from val.s of value (if EXPORT)
	 */
	int type;
	/* flags (see below) */
	uint32_t flag;

	/* actually longer: name (variable length) */
	char name[4];
};

EXTERN struct tbl vtemp;

/* common flag bits */
#define ALLOC		BIT(0)	/* val.s has been allocated */
#define DEFINED		BIT(1)	/* is defined in block */
#define ISSET		BIT(2)	/* has value, vp->val.[si] */
#define EXPORT		BIT(3)	/* exported variable/function */
#define TRACE		BIT(4)	/* var: user flagged, func: execution tracing */
/* (start non-common flags at 8) */
/* flag bits used for variables */
#define SPECIAL		BIT(8)	/* PATH, IFS, SECONDS, etc */
#define INTEGER		BIT(9)	/* val.i contains integer value */
#define RDONLY		BIT(10)	/* read-only variable */
#define LOCAL		BIT(11)	/* for local typeset() */
#define ARRAY		BIT(13)	/* array */
#define LJUST		BIT(14)	/* left justify */
#define RJUST		BIT(15)	/* right justify */
#define ZEROFIL		BIT(16)	/* 0 filled if RJUSTIFY, strip 0s if LJUSTIFY */
#define LCASEV		BIT(17)	/* convert to lower case */
#define UCASEV_AL	BIT(18) /* convert to upper case / autoload function */
#define INT_U		BIT(19)	/* unsigned integer */
#define INT_L		BIT(20)	/* long integer (no-op but used as magic) */
#define IMPORT		BIT(21)	/* flag to typeset(): no arrays, must have = */
#define LOCAL_COPY	BIT(22)	/* with LOCAL - copy attrs from existing var */
#define EXPRINEVAL	BIT(23)	/* contents currently being evaluated */
#define EXPRLVALUE	BIT(24)	/* useable as lvalue (temp flag) */
#define AINDEX		BIT(25) /* array index >0 = ua.index filled in */
#define ASSOC		BIT(26) /* ARRAY ? associative : reference */
/* flag bits used for taliases/builtins/aliases/keywords/functions */
#define KEEPASN		BIT(8)	/* keep command assignments (eg, var=x cmd) */
#define FINUSE		BIT(9)	/* function being executed */
#define FDELETE		BIT(10)	/* function deleted while it was executing */
#define FKSH		BIT(11)	/* function defined with function x (vs x()) */
#define SPEC_BI		BIT(12)	/* a POSIX special builtin */
/*
 * Attributes that can be set by the user (used to decide if an unset
 * param should be repoted by set/typeset). Does not include ARRAY or
 * LOCAL.
 */
#define USERATTRIB	(EXPORT|INTEGER|RDONLY|LJUST|RJUST|ZEROFIL|\
			    LCASEV|UCASEV_AL|INT_U|INT_L)

#define arrayindex(vp)	((unsigned long)((vp)->flag & AINDEX ? \
			    (vp)->ua.index : 0))

enum namerefflag {
	SRF_NOP,
	SRF_ENABLE,
	SRF_DISABLE
};

/* command types */
#define CNONE		0	/* undefined */
#define CSHELL		1	/* built-in */
#define CFUNC		2	/* function */
#define CEXEC		4	/* executable command */
#define CALIAS		5	/* alias */
#define CKEYWD		6	/* keyword */
#define CTALIAS		7	/* tracked alias */

/* Flags for findcom()/comexec() */
#define FC_SPECBI	BIT(0)	/* special builtin */
#define FC_FUNC		BIT(1)	/* function */
#define FC_NORMBI	BIT(2)	/* not special builtin */
#define FC_BI		(FC_SPECBI | FC_NORMBI)
#define FC_PATH		BIT(3)	/* do path search */
#define FC_DEFPATH	BIT(4)	/* use default path in path search */


#define AF_ARGV_ALLOC	0x1	/* argv[] array allocated */
#define AF_ARGS_ALLOCED	0x2	/* argument strings allocated */
#define AI_ARGV(a, i)	((i) == 0 ? (a).argv[0] : (a).argv[(i) - (a).skip])
#define AI_ARGC(a)	((a).ai_argc - (a).skip)

/* Argument info. Used for $#, $* for shell, functions, includes, etc. */
struct arg_info {
	const char **argv;
	int flags;	/* AF_* */
	int ai_argc;
	int skip;	/* first arg is argv[0], second is argv[1 + skip] */
};

/*
 * activation record for function blocks
 */
struct block {
	Area area;		/* area to allocate things */
	const char **argv;
	char *error;		/* error handler */
	char *exit;		/* exit handler */
	struct block *next;	/* enclosing block */
	struct table vars;	/* local variables */
	struct table funs;	/* local functions */
	Getopt getopts_state;
	int argc;
	int flags;		/* see BF_* */
};

/* Values for struct block.flags */
#define BF_DOGETOPTS	BIT(0)	/* save/restore getopts state */
#define BF_STOPENV	BIT(1)	/* do not export further */

/*
 * Used by ktwalk() and ktnext() routines.
 */
struct tstate {
	struct tbl **next;
	ssize_t left;
};

EXTERN struct table taliases;	/* tracked aliases */
EXTERN struct table builtins;	/* built-in commands */
EXTERN struct table aliases;	/* aliases */
EXTERN struct table keywords;	/* keywords */
#ifndef MKSH_NOPWNAM
EXTERN struct table homedirs;	/* homedir() cache */
#endif

struct builtin {
	const char *name;
	int (*func)(const char **);
};

extern const struct builtin mkshbuiltins[];

/* values for set_prompt() */
#define PS1	0	/* command */
#define PS2	1	/* command continuation */

EXTERN char *path;		/* copy of either PATH or def_path */
EXTERN const char *def_path;	/* path to use if PATH not set */
EXTERN char *tmpdir;		/* TMPDIR value */
EXTERN const char *prompt;
EXTERN uint8_t cur_prompt;	/* PS1 or PS2 */
EXTERN int current_lineno;	/* LINENO value */

/*
 * Description of a command or an operation on commands.
 */
struct op {
	const char **args;		/* arguments to a command */
	char **vars;			/* variable assignments */
	struct ioword **ioact;		/* IO actions (eg, < > >>) */
	struct op *left, *right;	/* descendents */
	char *str;			/* word for case; identifier for for,
					 * select, and functions;
					 * path to execute for TEXEC;
					 * time hook for TCOM.
					 */
	int lineno;			/* TCOM/TFUNC: LINENO for this */
	short type;			/* operation type, see below */
	/* WARNING: newtp(), tcopy() use evalflags = 0 to clear union */
	union {
		/* TCOM: arg expansion eval() flags */
		short evalflags;
		/* TFUNC: function x (vs x()) */
		short ksh_func;
		/* TPAT: termination character */
		char charflag;
	} u;
};

/* Tree.type values */
#define TEOF		0
#define TCOM		1	/* command */
#define TPAREN		2	/* (c-list) */
#define TPIPE		3	/* a | b */
#define TLIST		4	/* a ; b */
#define TOR		5	/* || */
#define TAND		6	/* && */
#define TBANG		7	/* ! */
#define TDBRACKET	8	/* [[ .. ]] */
#define TFOR		9
#define TSELECT		10
#define TCASE		11
#define TIF		12
#define TWHILE		13
#define TUNTIL		14
#define TELIF		15
#define TPAT		16	/* pattern in case */
#define TBRACE		17	/* {c-list} */
#define TASYNC		18	/* c & */
#define TFUNCT		19	/* function name { command; } */
#define TTIME		20	/* time pipeline */
#define TEXEC		21	/* fork/exec eval'd TCOM */
#define TCOPROC		22	/* coprocess |& */

/*
 * prefix codes for words in command tree
 */
#define EOS	0	/* end of string */
#define CHAR	1	/* unquoted character */
#define QCHAR	2	/* quoted character */
#define COMSUB	3	/* $() substitution (0 terminated) */
#define EXPRSUB	4	/* $(()) substitution (0 terminated) */
#define OQUOTE	5	/* opening " or ' */
#define CQUOTE	6	/* closing " or ' */
#define OSUBST	7	/* opening ${ subst (followed by { or X) */
#define CSUBST	8	/* closing } of above (followed by } or X) */
#define OPAT	9	/* open pattern: *(, @(, etc. */
#define SPAT	10	/* separate pattern: | */
#define CPAT	11	/* close pattern: ) */
#define ADELIM	12	/* arbitrary delimiter: ${foo:2:3} ${foo/bar/baz} */
#define FUNSUB	14	/* ${ foo;} substitution (NUL terminated) */
#define VALSUB	15	/* ${|foo;} substitution (NUL terminated) */

/*
 * IO redirection
 */
struct ioword {
	char *ioname;		/* filename (unused if heredoc) */
	char *delim;		/* delimiter for <<, <<- */
	char *heredoc;		/* content of heredoc */
	unsigned short ioflag;	/* action (below) */
	short unit;		/* unit (fd) affected */
};

/* ioword.flag - type of redirection */
#define IOTYPE		0xF	/* type: bits 0:3 */
#define IOREAD		0x1	/* < */
#define IOWRITE		0x2	/* > */
#define IORDWR		0x3	/* <>: todo */
#define IOHERE		0x4	/* << (here file) */
#define IOCAT		0x5	/* >> */
#define IODUP		0x6	/* <&/>& */
#define IOEVAL		BIT(4)	/* expand in << */
#define IOSKIP		BIT(5)	/* <<-, skip ^\t* */
#define IOCLOB		BIT(6)	/* >|, override -o noclobber */
#define IORDUP		BIT(7)	/* x<&y (as opposed to x>&y) */
#define IONAMEXP	BIT(8)	/* name has been expanded */
#define IOBASH		BIT(9)	/* &> etc. */
#define IOHERESTR	BIT(10)	/* <<< (here string) */
#define IONDELIM	BIT(11)	/* null delimiter (<<) */

/* execute/exchild flags */
#define XEXEC	BIT(0)		/* execute without forking */
#define XFORK	BIT(1)		/* fork before executing */
#define XBGND	BIT(2)		/* command & */
#define XPIPEI	BIT(3)		/* input is pipe */
#define XPIPEO	BIT(4)		/* output is pipe */
#define XXCOM	BIT(5)		/* `...` command */
#define XPCLOSE	BIT(6)		/* exchild: close close_fd in parent */
#define XCCLOSE	BIT(7)		/* exchild: close close_fd in child */
#define XERROK	BIT(8)		/* non-zero exit ok (for set -e) */
#define XCOPROC BIT(9)		/* starting a co-process */
#define XTIME	BIT(10)		/* timing TCOM command */
#define XPIPEST	BIT(11)		/* want PIPESTATUS */

/*
 * flags to control expansion of words (assumed by t->evalflags to fit
 * in a short)
 */
#define DOBLANK	BIT(0)		/* perform blank interpretation */
#define DOGLOB	BIT(1)		/* expand [?* */
#define DOPAT	BIT(2)		/* quote *?[ */
#define DOTILDE	BIT(3)		/* normal ~ expansion (first char) */
#define DONTRUNCOMMAND BIT(4)	/* do not run $(command) things */
#define DOASNTILDE BIT(5)	/* assignment ~ expansion (after =, :) */
#define DOBRACE BIT(6)		/* used by expand(): do brace expansion */
#define DOMAGIC BIT(7)		/* used by expand(): string contains MAGIC */
#define DOTEMP	BIT(8)		/* dito: in word part of ${..[%#=?]..} */
#define DOVACHECK BIT(9)	/* var assign check (for typeset, set, etc) */
#define DOMARKDIRS BIT(10)	/* force markdirs behaviour */
#define DOTCOMEXEC BIT(11)	/* not an eval flag, used by sh -c hack */
#define DOSCALAR BIT(12)	/* change field handling to non-list context */
#define DOHEREDOC BIT(13)	/* change scalar handling to heredoc body */
#define DOHERESTR BIT(14)	/* append a newline char */

#define X_EXTRA	20	/* this many extra bytes in X string */

typedef struct XString {
	char *end, *beg;	/* end, begin of string */
	size_t len;		/* length */
	Area *areap;		/* area to allocate/free from */
} XString;

typedef char *XStringP;

/* initialise expandable string */
#define XinitN(xs, length, area) do {				\
	(xs).len = (length);					\
	(xs).areap = (area);					\
	(xs).beg = alloc((xs).len + X_EXTRA, (xs).areap);	\
	(xs).end = (xs).beg + (xs).len;				\
} while (/* CONSTCOND */ 0)
#define Xinit(xs, xp, length, area) do {			\
	XinitN((xs), (length), (area));				\
	(xp) = (xs).beg;					\
} while (/* CONSTCOND */ 0)

/* stuff char into string */
#define Xput(xs, xp, c)	(*xp++ = (c))

/* check if there are at least n bytes left */
#define XcheckN(xs, xp, n) do {					\
	ssize_t more = ((xp) + (n)) - (xs).end;			\
	if (more > 0)						\
		(xp) = Xcheck_grow(&(xs), (xp), (size_t)more);	\
} while (/* CONSTCOND */ 0)

/* check for overflow, expand string */
#define Xcheck(xs, xp)	XcheckN((xs), (xp), 1)

/* free string */
#define Xfree(xs, xp)	afree((xs).beg, (xs).areap)

/* close, return string */
#define Xclose(xs, xp)	aresize((xs).beg, (xp) - (xs).beg, (xs).areap)

/* begin of string */
#define Xstring(xs, xp)	((xs).beg)

#define Xnleft(xs, xp)	((xs).end - (xp))	/* may be less than 0 */
#define Xlength(xs, xp)	((xp) - (xs).beg)
#define Xsize(xs, xp)	((xs).end - (xs).beg)
#define Xsavepos(xs, xp)	((xp) - (xs).beg)
#define Xrestpos(xs, xp, n)	((xs).beg + (n))

char *Xcheck_grow(XString *, const char *, size_t);

/*
 * expandable vector of generic pointers
 */

typedef struct {
	/* begin of allocated area */
	void **beg;
	/* currently used number of entries */
	size_t len;
	/* allocated number of entries */
	size_t siz;
} XPtrV;

#define XPinit(x, n)	do {					\
	(x).siz = (n);						\
	(x).len = 0;						\
	(x).beg = alloc2((x).siz, sizeof(void *), ATEMP);	\
} while (/* CONSTCOND */ 0)					\

#define XPput(x, p)	do {					\
	if ((x).len == (x).siz) {				\
		(x).beg = aresize2((x).beg, (x).siz,		\
		    2 * sizeof(void *), ATEMP);			\
		(x).siz <<= 1;					\
	}							\
	(x).beg[(x).len++] = (p);				\
} while (/* CONSTCOND */ 0)

#define XPptrv(x)	((x).beg)
#define XPsize(x)	((x).len)
#define XPclose(x)	aresize2((x).beg, XPsize(x), sizeof(void *), ATEMP)
#define XPfree(x)	afree((x).beg, ATEMP)

/*
 * Lexer internals
 */

typedef struct source Source;
struct source {
	const char *str;	/* input pointer */
	const char *start;	/* start of current buffer */
	union {
		const char **strv;	/* string [] */
		struct shf *shf;	/* shell file */
		struct tbl *tblp;	/* alias (SF_HASALIAS) */
		char *freeme;		/* also for SREREAD */
	} u;
	const char *file;	/* input file name */
	int	type;		/* input type */
	int	line;		/* line number */
	int	errline;	/* line the error occurred on (0 if not set) */
	int	flags;		/* SF_* */
	Area	*areap;
	Source *next;		/* stacked source */
	XString	xs;		/* input buffer */
	char	ugbuf[2];	/* buffer for ungetsc() (SREREAD) and
				 * alias (SALIAS) */
};

/* Source.type values */
#define SEOF		0	/* input EOF */
#define SFILE		1	/* file input */
#define SSTDIN		2	/* read stdin */
#define SSTRING		3	/* string */
#define SWSTR		4	/* string without \n */
#define SWORDS		5	/* string[] */
#define SWORDSEP	6	/* string[] separator */
#define SALIAS		7	/* alias expansion */
#define SREREAD		8	/* read ahead to be re-scanned */
#define SSTRINGCMDLINE	9	/* string from "mksh -c ..." */

/* Source.flags values */
#define SF_ECHO		BIT(0)	/* echo input to shlout */
#define SF_ALIAS	BIT(1)	/* faking space at end of alias */
#define SF_ALIASEND	BIT(2)	/* faking space at end of alias */
#define SF_TTY		BIT(3)	/* type == SSTDIN & it is a tty */
#define SF_HASALIAS	BIT(4)	/* u.tblp valid (SALIAS, SEOF) */
#define SF_MAYEXEC	BIT(5)	/* special sh -c optimisation hack */

typedef union {
	int i;
	char *cp;
	char **wp;
	struct op *o;
	struct ioword *iop;
} YYSTYPE;

/* If something is added here, add it to tokentab[] in syn.c as well */
#define LWORD		256
#define LOGAND		257	/* && */
#define LOGOR		258	/* || */
#define BREAK		259	/* ;; */
#define IF		260
#define THEN		261
#define ELSE		262
#define ELIF		263
#define FI		264
#define CASE		265
#define ESAC		266
#define FOR		267
#define SELECT		268
#define WHILE		269
#define UNTIL		270
#define DO		271
#define DONE		272
#define IN		273
#define FUNCTION	274
#define TIME		275
#define REDIR		276
#define MDPAREN		277	/* (( )) */
#define BANG		278	/* ! */
#define DBRACKET	279	/* [[ .. ]] */
#define COPROC		280	/* |& */
#define BRKEV		281	/* ;| */
#define BRKFT		282	/* ;& */
#define YYERRCODE	300

/* flags to yylex */
#define CONTIN		BIT(0)	/* skip new lines to complete command */
#define ONEWORD		BIT(1)	/* single word for substitute() */
#define ALIAS		BIT(2)	/* recognise alias */
#define KEYWORD		BIT(3)	/* recognise keywords */
#define LETEXPR		BIT(4)	/* get expression inside (( )) */
#define VARASN		BIT(5)	/* check for var=word */
#define ARRAYVAR	BIT(6)	/* parse x[1 & 2] as one word */
#define ESACONLY	BIT(7)	/* only accept esac keyword */
#define CMDWORD		BIT(8)	/* parsing simple command (alias related) */
#define HEREDELIM	BIT(9)	/* parsing <<,<<- delimiter */
#define LQCHAR		BIT(10)	/* source string contains QCHAR */
#define HEREDOC 	BIT(11)	/* parsing a here document body */

#define HERES		10	/* max number of << in line */

#undef CTRL
#define	CTRL(x)		((x) == '?' ? 0x7F : (x) & 0x1F)	/* ASCII */
#define	UNCTRL(x)	((x) ^ 0x40)				/* ASCII */
#define	ISCTRL(x)	(((signed char)((uint8_t)(x) + 1)) < 33)

#define IDENT		64

EXTERN Source *source;		/* yyparse/yylex source */
EXTERN YYSTYPE yylval;		/* result from yylex */
EXTERN struct ioword *heres[HERES], **herep;
EXTERN char ident[IDENT + 1];

EXTERN char **history;		/* saved commands */
EXTERN char **histptr;		/* last history item */
EXTERN mksh_ari_t histsize;	/* history size */

/* flags to histsave */
#define HIST_FLUSH	0
#define HIST_QUEUE	1
#define HIST_APPEND	2
#define HIST_STORE	3
#define HIST_NOTE	4

/* user and system time of last j_waitjed job */
EXTERN struct timeval j_usrtime, j_systime;

#define notok2mul(max, val, c)	(((val) != 0) && ((c) != 0) && \
				    (((max) / (c)) < (val)))
#define notok2add(max, val, c)	((val) > ((max) - (c)))
#define notoktomul(val, cnst)	notok2mul(SIZE_MAX, (val), (cnst))
#define notoktoadd(val, cnst)	notok2add(SIZE_MAX, (val), (cnst))
#define checkoktoadd(val, cnst) do {					\
	if (notoktoadd((val), (cnst)))					\
		internal_errorf(Tintovfl, (size_t)(val),		\
		    '+', (size_t)(cnst));				\
} while (/* CONSTCOND */ 0)


/* lalloc.c */
void ainit(Area *);
void afreeall(Area *);
/* these cannot fail and can take NULL (not for ap) */
#define alloc(n, ap)		aresize(NULL, (n), (ap))
#define alloc2(m, n, ap)	aresize2(NULL, (m), (n), (ap))
void *aresize(void *, size_t, Area *);
void *aresize2(void *, size_t, size_t, Area *);
void afree(void *, Area *);	/* can take NULL */
/* edit.c */
#ifndef MKSH_NO_CMDLINE_EDITING
#ifndef MKSH_SMALL
int x_bind(const char *, const char *, bool, bool);
#else
int x_bind(const char *, const char *, bool);
#endif
void x_init(void);
#ifdef DEBUG_LEAKS
void x_done(void);
#endif
int x_read(char *);
#endif
void x_mkraw(int, mksh_ttyst *, bool);
/* eval.c */
char *substitute(const char *, int);
char **eval(const char **, int);
char *evalstr(const char *cp, int);
char *evalonestr(const char *cp, int);
char *debunk(char *, const char *, size_t);
void expand(const char *, XPtrV *, int);
int glob_str(char *, XPtrV *, bool);
char *do_tilde(char *);
/* exec.c */
int execute(struct op * volatile, volatile int, volatile int * volatile);
int shcomexec(const char **);
struct tbl *findfunc(const char *, uint32_t, bool);
int define(const char *, struct op *);
const char *builtin(const char *, int (*)(const char **));
struct tbl *findcom(const char *, int);
void flushcom(bool);
int search_access(const char *, int);
const char *search_path(const char *, const char *, int, int *);
void pr_menu(const char * const *);
void pr_list(char * const *);
/* expr.c */
int evaluate(const char *, mksh_ari_t *, int, bool);
int v_evaluate(struct tbl *, const char *, volatile int, bool);
/* UTF-8 stuff */
size_t utf_mbtowc(unsigned int *, const char *);
size_t utf_wctomb(char *, unsigned int);
int utf_widthadj(const char *, const char **);
size_t utf_mbswidth(const char *) MKSH_A_PURE;
const char *utf_skipcols(const char *, int) MKSH_A_PURE;
size_t utf_ptradj(const char *) MKSH_A_PURE;
#ifdef MIRBSD_BOOTFLOPPY
#define utf_wcwidth(i) wcwidth((wchar_t)(i))
#else
int utf_wcwidth(unsigned int) MKSH_A_PURE;
#endif
int ksh_access(const char *, int);
struct tbl *tempvar(void);
/* funcs.c */
int c_hash(const char **);
int c_pwd(const char **);
int c_print(const char **);
#ifdef MKSH_PRINTF_BUILTIN
int c_printf(const char **);
#endif
int c_whence(const char **);
int c_command(const char **);
int c_typeset(const char **);
int c_alias(const char **);
int c_unalias(const char **);
int c_let(const char **);
int c_jobs(const char **);
#ifndef MKSH_UNEMPLOYED
int c_fgbg(const char **);
#endif
int c_kill(const char **);
void getopts_reset(int);
int c_getopts(const char **);
#ifndef MKSH_NO_CMDLINE_EDITING
int c_bind(const char **);
#endif
int c_shift(const char **);
int c_umask(const char **);
int c_dot(const char **);
int c_wait(const char **);
int c_read(const char **);
int c_eval(const char **);
int c_trap(const char **);
int c_brkcont(const char **);
int c_exitreturn(const char **);
int c_set(const char **);
int c_unset(const char **);
int c_ulimit(const char **);
int c_times(const char **);
int timex(struct op *, int, volatile int *);
void timex_hook(struct op *, char ** volatile *);
int c_exec(const char **);
/* dummy function (just need pointer value), special case in comexec() */
#define c_builtin shcomexec
int c_test(const char **);
#if HAVE_MKNOD
int c_mknod(const char **);
#endif
int c_realpath(const char **);
int c_rename(const char **);
int c_cat(const char **);
int c_sleep(const char **);
/* histrap.c */
void init_histvec(void);
void hist_init(Source *);
#if HAVE_PERSISTENT_HISTORY
void hist_finish(void);
#endif
void histsave(int *, const char *, int, bool);
#if !defined(MKSH_SMALL) && HAVE_PERSISTENT_HISTORY
bool histsync(void);
#endif
int c_fc(const char **);
void sethistsize(mksh_ari_t);
#if HAVE_PERSISTENT_HISTORY
void sethistfile(const char *);
#endif
#if !defined(MKSH_NO_CMDLINE_EDITING) && !MKSH_S_NOVI
char **histpos(void) MKSH_A_PURE;
int histnum(int);
#endif
int findhist(int, int, const char *, bool) MKSH_A_PURE;
char **hist_get_newest(bool);
void inittraps(void);
void alarm_init(void);
Trap *gettrap(const char *, bool, bool);
void trapsig(int);
void intrcheck(void);
int fatal_trap_check(void);
int trap_pending(void);
void runtraps(int intr);
void runtrap(Trap *, bool);
void cleartraps(void);
void restoresigs(void);
void settrap(Trap *, const char *);
int block_pipe(void);
void restore_pipe(int);
int setsig(Trap *, sig_t, int);
void setexecsig(Trap *, int);
#if HAVE_FLOCK || HAVE_LOCK_FCNTL
void mksh_lockfd(int);
void mksh_unlkfd(int);
#endif
/* jobs.c */
void j_init(void);
void j_exit(void);
#ifndef MKSH_UNEMPLOYED
void j_change(void);
#endif
int exchild(struct op *, int, volatile int *, int);
void startlast(void);
int waitlast(void);
int waitfor(const char *, int *);
int j_kill(const char *, int);
#ifndef MKSH_UNEMPLOYED
int j_resume(const char *, int);
#endif
#if !defined(MKSH_UNEMPLOYED) && HAVE_GETSID
void j_suspend(void);
#endif
int j_jobs(const char *, int, int);
void j_notify(void);
pid_t j_async(void);
int j_stopped_running(void);
/* lex.c */
int yylex(int);
void yyskiputf8bom(void);
void yyerror(const char *, ...)
    MKSH_A_NORETURN
    MKSH_A_FORMAT(__printf__, 1, 2);
Source *pushs(int, Area *);
void set_prompt(int, Source *);
int pprompt(const char *, int);
/* main.c */
int include(const char *, int, const char **, bool);
int command(const char *, int);
int shell(Source * volatile, volatile bool);
/* argument MUST NOT be 0 */
void unwind(int) MKSH_A_NORETURN;
void newenv(int);
void quitenv(struct shf *);
void cleanup_parents_env(void);
void cleanup_proc_env(void);
void errorf(const char *, ...)
    MKSH_A_NORETURN
    MKSH_A_FORMAT(__printf__, 1, 2);
void errorfx(int, const char *, ...)
    MKSH_A_NORETURN
    MKSH_A_FORMAT(__printf__, 2, 3);
void warningf(bool, const char *, ...)
    MKSH_A_FORMAT(__printf__, 2, 3);
void bi_errorf(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
#define errorfz()	errorf(NULL)
#define errorfxz(rc)	errorfx((rc), NULL)
#define bi_errorfz()	bi_errorf(NULL)
void internal_errorf(const char *, ...)
    MKSH_A_NORETURN
    MKSH_A_FORMAT(__printf__, 1, 2);
void internal_warningf(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
void error_prefix(bool);
void shellf(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
void shprintf(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
int can_seek(int);
void initio(void);
int ksh_dup2(int, int, bool);
short savefd(int);
void restfd(int, int);
void openpipe(int *);
void closepipe(int *);
int check_fd(const char *, int, const char **);
void coproc_init(void);
void coproc_read_close(int);
void coproc_readw_close(int);
void coproc_write_close(int);
int coproc_getfd(int, const char **);
void coproc_cleanup(int);
struct temp *maketemp(Area *, Temp_type, struct temp **);
void ktinit(Area *, struct table *, uint8_t);
struct tbl *ktscan(struct table *, const char *, uint32_t, struct tbl ***);
/* table, name (key) to search for, hash(n) */
#define ktsearch(tp, s, h) ktscan((tp), (s), (h), NULL)
struct tbl *ktenter(struct table *, const char *, uint32_t);
#define ktdelete(p)	do { p->flag = 0; } while (/* CONSTCOND */ 0)
void ktwalk(struct tstate *, struct table *);
struct tbl *ktnext(struct tstate *);
struct tbl **ktsort(struct table *);
#ifdef DF
void DF(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
#endif
/* misc.c */
void setctypes(const char *, int);
void initctypes(void);
size_t option(const char *) MKSH_A_PURE;
char *getoptions(void);
void change_flag(enum sh_flag, int, bool);
void change_xtrace(unsigned char, bool);
int parse_args(const char **, int, bool *);
int getn(const char *, int *);
int gmatchx(const char *, const char *, bool);
int has_globbing(const char *, const char *) MKSH_A_PURE;
int xstrcmp(const void *, const void *) MKSH_A_PURE;
void ksh_getopt_reset(Getopt *, int);
int ksh_getopt(const char **, Getopt *, const char *);
void print_value_quoted(struct shf *, const char *);
char *quote_value(const char *);
void print_columns(struct shf *, unsigned int,
    void (*)(char *, size_t, unsigned int, const void *),
    const void *, size_t, size_t, bool);
void strip_nuls(char *, size_t)
    MKSH_A_BOUNDED(__string__, 1, 2);
ssize_t blocking_read(int, char *, size_t)
    MKSH_A_BOUNDED(__buffer__, 2, 3);
int reset_nonblock(int);
char *ksh_get_wd(void);
char *do_realpath(const char *);
void simplify_path(char *);
void set_current_wd(const char *);
int c_cd(const char **);
#if defined(MKSH_SMALL) && !defined(MKSH_SMALL_BUT_FAST)
char *strdup_i(const char *, Area *);
char *strndup_i(const char *, size_t, Area *);
#endif
int unbksl(bool, int (*)(void), void (*)(int));
/* shf.c */
struct shf *shf_open(const char *, int, int, int);
struct shf *shf_fdopen(int, int, struct shf *);
struct shf *shf_reopen(int, int, struct shf *);
struct shf *shf_sopen(char *, ssize_t, int, struct shf *);
int shf_close(struct shf *);
int shf_fdclose(struct shf *);
char *shf_sclose(struct shf *);
int shf_flush(struct shf *);
ssize_t shf_read(char *, ssize_t, struct shf *);
char *shf_getse(char *, ssize_t, struct shf *);
int shf_getchar(struct shf *s);
int shf_ungetc(int, struct shf *);
#if defined(MKSH_SMALL) && !defined(MKSH_SMALL_BUT_FAST)
int shf_getc(struct shf *);
int shf_putc(int, struct shf *);
#else
#define shf_getc shf_getc_i
#define shf_putc shf_putc_i
#endif
int shf_putchar(int, struct shf *);
ssize_t shf_puts(const char *, struct shf *);
ssize_t shf_write(const char *, ssize_t, struct shf *);
ssize_t shf_fprintf(struct shf *, const char *, ...)
    MKSH_A_FORMAT(__printf__, 2, 3);
ssize_t shf_snprintf(char *, ssize_t, const char *, ...)
    MKSH_A_FORMAT(__printf__, 3, 4)
    MKSH_A_BOUNDED(__string__, 1, 2);
char *shf_smprintf(const char *, ...)
    MKSH_A_FORMAT(__printf__, 1, 2);
ssize_t shf_vfprintf(struct shf *, const char *, va_list)
    MKSH_A_FORMAT(__printf__, 2, 0);
/* syn.c */
int assign_command(const char *);
void initkeywords(void);
struct op *compile(Source *, bool);
bool parse_usec(const char *, struct timeval *);
char *yyrecursive(int);
void yyrecursive_pop(bool);
/* tree.c */
void fptreef(struct shf *, int, const char *, ...);
char *snptreef(char *, ssize_t, const char *, ...);
struct op *tcopy(struct op *, Area *);
char *wdcopy(const char *, Area *);
const char *wdscan(const char *, int);
#define WDS_TPUTS	BIT(0)		/* tputS (dumpwdvar) mode */
char *wdstrip(const char *, int);
void tfree(struct op *, Area *);
void dumpchar(struct shf *, int);
void dumptree(struct shf *, struct op *);
void dumpwdvar(struct shf *, const char *);
void dumpioact(struct shf *shf, struct op *t);
void vistree(char *, size_t, struct op *)
    MKSH_A_BOUNDED(__string__, 1, 2);
void fpFUNCTf(struct shf *, int, bool, const char *, struct op *);
/* var.c */
void newblock(void);
void popblock(void);
void initvar(void);
struct block *varsearch(struct block *, struct tbl **, const char *, uint32_t);
struct tbl *global(const char *);
struct tbl *local(const char *, bool);
char *str_val(struct tbl *);
int setstr(struct tbl *, const char *, int);
struct tbl *setint_v(struct tbl *, struct tbl *, bool);
void setint(struct tbl *, mksh_ari_t);
void setint_n(struct tbl *, mksh_ari_t, int);
struct tbl *typeset(const char *, uint32_t, uint32_t, int, int);
void unset(struct tbl *, int);
const char *skip_varname(const char *, bool) MKSH_A_PURE;
const char *skip_wdvarname(const char *, bool) MKSH_A_PURE;
int is_wdvarname(const char *, bool) MKSH_A_PURE;
int is_wdvarassign(const char *) MKSH_A_PURE;
struct tbl *arraysearch(struct tbl *, uint32_t);
char **makenv(void);
void change_winsz(void);
size_t array_ref_len(const char *) MKSH_A_PURE;
char *arrayname(const char *);
mksh_uari_t set_array(const char *, bool, const char **);
uint32_t hash(const void *) MKSH_A_PURE;
uint32_t chvt_rndsetup(const void *, size_t) MKSH_A_PURE;
mksh_ari_t rndget(void);
void rndset(unsigned long);
void rndpush(const void *);

enum Test_op {
	/* non-operator */
	TO_NONOP = 0,
	/* unary operators */
	TO_STNZE, TO_STZER, TO_OPTION,
	TO_FILAXST,
	TO_FILEXST,
	TO_FILREG, TO_FILBDEV, TO_FILCDEV, TO_FILSYM, TO_FILFIFO, TO_FILSOCK,
	TO_FILCDF, TO_FILID, TO_FILGID, TO_FILSETG, TO_FILSTCK, TO_FILUID,
	TO_FILRD, TO_FILGZ, TO_FILTT, TO_FILSETU, TO_FILWR, TO_FILEX,
	/* binary operators */
	TO_STEQL, TO_STNEQ, TO_STLT, TO_STGT, TO_INTEQ, TO_INTNE, TO_INTGT,
	TO_INTGE, TO_INTLT, TO_INTLE, TO_FILEQ, TO_FILNT, TO_FILOT,
	/* not an operator */
	TO_NONNULL	/* !TO_NONOP */
};
typedef enum Test_op Test_op;

/* Used by Test_env.isa() (order important - used to index *_tokens[] arrays) */
enum Test_meta {
	TM_OR,		/* -o or || */
	TM_AND,		/* -a or && */
	TM_NOT,		/* ! */
	TM_OPAREN,	/* ( */
	TM_CPAREN,	/* ) */
	TM_UNOP,	/* unary operator */
	TM_BINOP,	/* binary operator */
	TM_END		/* end of input */
};
typedef enum Test_meta Test_meta;

#define TEF_ERROR	BIT(0)		/* set if we've hit an error */
#define TEF_DBRACKET	BIT(1)		/* set if [[ .. ]] test */

typedef struct test_env {
	union {
		const char **wp;	/* used by ptest_* */
		XPtrV *av;		/* used by dbtestp_* */
	} pos;
	const char **wp_end;		/* used by ptest_* */
	Test_op (*isa)(struct test_env *, Test_meta);
	const char *(*getopnd) (struct test_env *, Test_op, bool);
	int (*eval)(struct test_env *, Test_op, const char *, const char *, bool);
	void (*error)(struct test_env *, int, const char *);
	int flags;			/* TEF_* */
} Test_env;

extern const char * const dbtest_tokens[];

Test_op	test_isop(Test_meta, const char *) MKSH_A_PURE;
int test_eval(Test_env *, Test_op, const char *, const char *, bool);
int test_parse(Test_env *);

/* tty_fd is not opened O_BINARY, it's thus never read/written */
EXTERN int tty_fd E_INIT(-1);	/* dup'd tty file descriptor */
EXTERN bool tty_devtty;		/* true if tty_fd is from /dev/tty */
EXTERN mksh_ttyst tty_state;	/* saved tty state */
EXTERN bool tty_hasstate;	/* true if tty_state is valid */

extern int tty_init_fd(void);	/* initialise tty_fd, tty_devtty */

#ifdef __OS2__
#ifndef __GNUC__
# error oops?
#endif
#define binopen2(path,flags)		__extension__({			\
	int binopen2_fd = open((path), (flags) | O_BINARY);		\
	if (binopen2_fd >= 0)						\
		setmode(binopen2_fd, O_BINARY);				\
	(binopen2_fd);							\
})
#define binopen3(path,flags,mode)	__extension__({			\
	int binopen3_fd = open((path), (flags) | O_BINARY, (mode));	\
	if (binopen3_fd >= 0)						\
		setmode(binopen3_fd, O_BINARY);				\
	(binopen3_fd);							\
})
#define mksh_abspath(s)			__extension__({			\
	const char *mksh_abspath_s = (s);				\
	(mksh_abspath_s[0] == '/' || (ksh_isalphx(mksh_abspath_s[0]) &&	\
	    mksh_abspath_s[1] == ':'));					\
})
#else
#define binopen2(path,flags)		open((path), (flags) | O_BINARY)
#define binopen3(path,flags,mode)	open((path), (flags) | O_BINARY, (mode))
#define mksh_abspath(s)			((s)[0] == '/')
#endif

/* be sure not to interfere with anyone else's idea about EXTERN */
#ifdef EXTERN_DEFINED
# undef EXTERN_DEFINED
# undef EXTERN
#endif
#undef E_INIT

#endif /* !MKSH_INCLUDES_ONLY */
