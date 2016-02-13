/*  C K U S I G . H  */

/*  Definitions and prototypes for signal handling  */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

#ifdef CK_ANSIC
typedef void (*ck_sigfunc)(void *);
typedef void (*ck_sighand)(int);
#else
typedef VOID (*ck_sigfunc)();
typedef VOID (*ck_sighand)();
#endif /* CK_ANSIC */

/* Macros for POSIX vs old-style signal handling. */

#ifdef CK_POSIX_SIG
typedef sigjmp_buf ckjmpbuf;
#else
typedef jmp_buf ckjmpbuf;
#endif /* CK_POSIX_SIG */
/*
  Suppose you want to pass the address of a jmp_buf bar to a function foo.
  Since jmp_buf is normally defined (typedef'd) as an array, you would do
  it like this:  foo(bar), where foo = foo(jmp_buf bar).  But suppose a
  jmp_buf is (say) a struct rather than an array.  Then you must do
  foo(&bar) where foo is foo(jmp_buf * bar).  This is controlled here in
  the traditional fashion, by ifdefs.  By default, we assume that jmp_buf
  is an array.  Define the symbol JBNOTARRAY if jmp_buf is not an array.
*/
#ifndef JBNOTARRAY
#ifdef NT
#define JBNOTARRAY
#endif /* NT */
#endif /* JBNOTARRAY */

#ifdef JBNOTARRAY
typedef ckjmpbuf * ckjptr;
#define ckjaddr(x) & x
#define ckjdref(x) * x
#ifdef CK_POSIX_SIG
#define cksetjmp(x) sigsetjmp(x,1)
#else
#define cksetjmp(x) setjmp(x,1)
#endif /* CK_POSIX_SIG */
#else  /* jmp_buf is an array */
typedef ckjmpbuf ckjptr;
#define ckjaddr(x) x
#define ckjdref(x) x
#ifdef CK_POSIX_SIG
#define cksetjmp sigsetjmp
#else
#define cksetjmp setjmp
#endif /* CK_POSIX_SIG */
#endif /* JBNOTARRAY */

_PROTOTYP( int cc_execute, (ckjptr, ck_sigfunc, ck_sigfunc) );
_PROTOTYP( int alrm_execute,
	  (ckjptr,
	   int timo,
	   ck_sighand handler,
	   ck_sigfunc, ck_sigfunc) );
_PROTOTYP( int cc_alrm_execute,
	  (ckjptr,
	   int timo,
	   ck_sighand handler,
	   ck_sigfunc,
	   ck_sigfunc) );

/* End of ckusig.h */

