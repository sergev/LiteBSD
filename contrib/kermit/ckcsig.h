/*  C K C S I G . H  */

/*  Definitions and prototypes for signal handling  */

/*
  Author: Jeffrey E Altman (jaltman@secure-endpoints.com),
  Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifdef OS2
#ifndef NT
#ifndef __HEV__                 /* INCL_SEMAPHORE may also define HEV */
#define __HEV__
typedef  ULONG    HEV;                  /* hev */
typedef  HEV      *PHEV;
#endif /* __HEV__ */
#endif /* NT */
struct _threadinfo {
    int inuse;
    int child;
    int sibling;
#ifdef NT
    HANDLE id;
    HANDLE handle;
    HANDLE parent;
    HANDLE CompletionSem ;
    HANDLE DieSem ;
#else /* NT */
    TID id;
    TID parent;
    HEV CompletionSem;
    HEV DieSem;
#endif /* NT */
};
#endif /* OS2 */

#ifdef CK_ANSIC
typedef SIGTYP (*ck_sigfunc)(void *);
typedef SIGTYP (*ck_sighand)(int);
#else
typedef SIGTYP (*ck_sigfunc)();
typedef SIGTYP (*ck_sighand)();
#endif /* CK_ANSIC */

/* Macros for POSIX vs old-style signal handling. */

#ifdef CK_POSIX_SIG
typedef sigjmp_buf ckjmpbuf;
#else
#ifdef NT
#define NOCRYPT
#include <windows.h>
#ifdef NTASM
typedef struct {
    CONTEXT context;
    DWORD retcode;
} ckjmpbuf;
#else /* NTASM */
typedef jmp_buf ckjmpbuf;
#endif /* NTASM */
#else
typedef jmp_buf ckjmpbuf;
#endif
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
#define cklongjmp(x,y) siglongjmp(x,y)
#else
#ifdef NT
__inline int
ck_ih(void) {
    extern int TlsIndex;
#ifdef NTSIG
    struct _threadinfo * threadinfo;
    threadinfo = (struct _threadinfo *) TlsGetValue(TlsIndex);
    if (threadinfo) {
        if (WaitAndResetSem(threadinfo->DieSem,0)) {
            ckThreadDie(threadinfo);
            return 1;                   /* This should never execute */
        }
    }
#ifdef COMMENT
    else debug( F100, "ck_ih() threadinfo is NULL","",0);
#endif /* COMMENT */
#endif /* NTSIG */
    return 0;
}
#ifdef NTSIG
#define cksetjmp(x) setjmp(x)
#define cklongjmp(x,y) longjmp(x,y)
#else /* NTSIG */
#ifdef NTASM
__inline DWORD
cksetjmp( ckjptr jmp ) {
    extern int isinterrupted;
    jmp->retcode = 0;
    memset( &jmp->context, 0, sizeof(CONTEXT) );
    jmp->context.ContextFlags = CONTEXT_FULL ;
    if ( !GetThreadContext( GetCurrentThread(), &jmp->context ) )
      debug( F101, "cksetjmp GetThreadContext failed","",GetLastError());
    debug(F101,"cksetjmp returns","",jmp->retcode);
    isinterrupted = 0;
    return (jmp->retcode);
}

__inline void
cklongjmp( ckjptr jmp, int retval ) {
    extern HANDLE tidCommand;
    extern int ttyfd, mdmtyp ;
    extern DWORD CommandID;
    extern int isinterrupted;

    connoi();
    isinterrupted = 1;
    jmp->retcode = ( retval ? retval : 1 );
    debug(F101,"about to SetThreadContext for thread","", CommandID);
    debug(F101,"from Thread","",GetCurrentThreadId());
    if ( mdmtyp >= 0 ) {
        PurgeComm( (HANDLE) ttyfd, PURGE_TXABORT | PURGE_RXABORT );
    }
    if (SetThreadContext( tidCommand, &jmp->context ))
      debug(F100,"cklongjmp SetThreadContext success","",0);
    else
      debug(F101,"cklongjmp SetThreadContext failed","",GetLastError());
    msleep(50);
    cmini(1);                           /* Reset command parser */
    putkey(13);                         /* Stuff a carriage return */
   /* PostEventAvailSem(); */
}
#else /* NTASM */
void crash( void ) ;
#define cksetjmp(x) setjmp(x)
__inline void
cklongjmp( ckjptr jmp, int retval ) {
    extern HANDLE tidCommand;
    extern int ttyfd, mdmtyp;
    extern DWORD CommandID;
    CONTEXT context;

    if ( mdmtyp >= 0 ) {
        PurgeComm( (HANDLE) ttyfd, PURGE_TXABORT | PURGE_RXABORT ) ;
    }
    memset( &context, 0, sizeof(CONTEXT) );
    context.ContextFlags = CONTEXT_FULL;
    if ( !GetThreadContext( tidCommand, &context ) )
      debug( F101, "cklongjmp GetThreadContext failed","",GetLastError());

    /* Invalidate the instruction pointer */
    context.Eip =  (unsigned long) crash;

    debug(F101,"about to SetThreadContext for thread","", CommandID);
    debug(F101,"from Thread","",GetCurrentThreadId());
    if (SetThreadContext( tidCommand, &context ))
      debug(F100,"cklongjmp SetThreadContext success","",0);
    else
      debug(F101,"cklongjmp SetThreadContext failed","",GetLastError());
}
#endif /* NTASM */
#endif /* NTSIG */
#else /* NT */
#define cksetjmp(x) setjmp(x)
#define cklongjmp(x,y) longjmp(x,y)
#endif /* NT */
#endif /* CK_POSIX_SIG */
#else  /* jmp_buf is an array */
typedef ckjmpbuf ckjptr;
#define ckjaddr(x) x
#define ckjdref(x) x
#ifdef CK_POSIX_SIG
#define cksetjmp(x) sigsetjmp(x,1)
#define cklongjmp(x,y) siglongjmp(x,y)
#else
#define cksetjmp(x) setjmp(x)
#define cklongjmp(x,y) longjmp(x,y)
#endif /* CK_POSIX_SIG */
#endif /* JBNOTARRAY */

_PROTOTYP( int cc_execute, (ckjptr, ck_sigfunc, ck_sigfunc) );
_PROTOTYP( int alrm_execute,
          (ckjptr,
           int /* timo */,
           ck_sighand /* handler */,
           ck_sigfunc, ck_sigfunc) );
_PROTOTYP( int cc_alrm_execute,
          (ckjptr,
           int /* timo */,
           ck_sighand /* handler */,
           ck_sigfunc,
           ck_sigfunc) );

/* End of ckusig.h */
