char *ckusigv = "Signal support, 9.0.100, 16 Oct 2009";

/* C K U S I G  --  Kermit signal handling for Unix and OS/2 systems */

/*
  Author: Jeffrey Altman (jaltman@secure-endpoints.com),
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#include "ckcsym.h"
#include "ckcasc.h"			/* ASCII character symbols */
#include "ckcdeb.h"			/* Debug & other symbols */
#include "ckcker.h"			/* Kermit symbols */
#include "ckcnet.h"			/* Network symbols */
#ifndef NOSPL
#include "ckuusr.h"
#endif /* NOSPL */

#include <signal.h>
#ifdef NT
#include <setjmpex.h>
#include <excpt.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include "ckcsig.h"

#ifdef NOCCTRAP
extern ckjmpbuf cmjbuf;
#endif /* NOCCTRAP */

#ifdef MAC
#define signal msignal
#define SIGTYP long
#define alarm malarm
#define SIG_IGN 0
#define SIGALRM 1
#define SIGINT  2
SIGTYP (*msignal(int type, SIGTYP (*func)(int)))(int);
#endif /* MAC */

#ifdef STRATUS
/* We know these are set here.  MUST unset them before the definitions. */
#define signal vsignal
#define alarm valarm
SIGTYP (*vsignal(int type, SIGTYP (*func)(int)))(int);
int valarm(int interval);
#endif /* STRATUS */

#ifdef AMIGA
#define signal asignal
#define alarm aalarm
#define SIGALRM (_NUMSIG+1)
#define SIGTYP void
SIGTYP (*asignal(int type, SIGTYP (*func)(int)))(int);
unsigned aalarm(unsigned);
#endif /* AMIGA */

#ifdef NTASM
DWORD
ckgetIP(void)
{
   __asm
   {
      mov eax, dword ptr [esp+0x10]
      jmp ckgetIP + 0x18
   }
   return 1;

}
#endif /* NTASM */

#ifdef NT
DWORD
exception_filter( void )
{
   GetExceptionInformation ;
   return( EXCEPTION_EXECUTE_HANDLER ) ;
}
void
crash( void )
{
   int x = 0, y = 0 ;
    x / y ;
}
#endif /* NT */

#ifndef NOCCTRAP
int
#ifdef CK_ANSIC
cc_execute( ckjptr(sj_buf), ck_sigfunc dofunc, ck_sigfunc failfunc )
#else
cc_execute( sj_buf, dofunc, failfunc)
    ckjptr(sj_buf);
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */
/* cc_execute */ {
    int rc = 0 ;
#ifdef NTASM
   DWORD Eip, Esp ;
    isinterrupted = 0;
    sj_buf->retcode = 0 ;
    sj_buf->Id = GetCurrentThreadId() ;
    memset( &sj_buf->context, 0, sizeof(CONTEXT) );
    sj_buf->context.ContextFlags = CONTEXT_FULL ;
#ifndef COMMENT
    GetThreadContext(GetCurrentThread(), &(sj_buf->context) ) ;
    __asm
    {
          mov       ecx,dword ptr [sj_buf]
          mov       dword ptr [ecx+0xc4],esp
    }
   sj_buf->context.EFlags = 530 ;
   sj_buf->context.Eip = ckgetIP()+0x0C ;
#else /* COMMENT */
   __asm
   {
      mov eax, dword ptr [sj_buf]
      push eax
      mov eax, 0xfffffffe
      push eax
      mov eax, 0x00000039
      mov edx,esp
      int 0x2e
      pop eax
      pop eax
   }
#endif /* COMMENT */
#endif /* NTASM */
    if (
#ifdef NTASM
         isinterrupted
#else
		 cksetjmp(ckjdref(sj_buf))
#endif /* NTASM */
		 ) {
#ifdef NTASM
          __asm
            {
                mov esp, ESPToRestore
            }
            isinterrupted = 0 ;
#endif /* NTASM */
            (*failfunc)(NULL) ;
#ifdef NTASM
             rc = sj_buf->retcode ;
#else /* NTASM */
             rc = -1 ;
#endif  /* NTASM */
         } else {
#ifdef NT
            __try {
               (*dofunc)(NULL);
            }
            __except(exception_filter())
            {
               debug(F100,"cc_execute __except","",0);
               debug(F111,
		     "exception_filter",
		     "_exception_code",
		     etExceptionCode()
		     );
               longjmp(ckjdref(sj_buf),SIGINT);
            }
#else /* NT */
            (*dofunc)(NULL);
#endif /* NT */
         }
   return rc ;
}
#endif /* NOCCTRAP */

int
#ifdef CK_ANSIC				/* ANSIC C declaration... */
alrm_execute(ckjptr(sj_buf),
	     int timo,
	     ck_sighand handler,
	     ck_sigfunc dofunc,
	     ck_sigfunc failfunc
	     )

#else /* Not ANSIC C ... */

alrm_execute(sj_buf,
	     timo,
	     handler,
	     dofunc,
	     failfunc
	     )
    ckjptr(sj_buf);
    int timo;
    ck_sighand handler;
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */

/* alrm_execute */ {

    int rc = 0;
    int savalrm = 0;
_PROTOTYP(SIGTYP (*savhandler), (int));

    savalrm = alarm(timo);
    savhandler = signal(SIGALRM, handler);

#ifdef NTASM
    sj_buf->retcode = 0 ;
    sj_buf->Id = GetCurrentThreadId();
    memset(&sj_buf->context, 0, sizeof(CONTEXT));
    sj_buf->context.ContextFlags = CONTEXT_FULL;
#ifndef COMMENT
    GetThreadContext(GetCurrentThread(), &(sj_buf->context));
#else
   __asm
   {
      mov eax, dword ptr [sj_buf]
      push eax
      mov eax, 0xfffffffe
      push eax
      mov eax, 0x00000039
      mov edx,esp
      int 0x2e
      pop eax
      pop eax
   }
#endif
    isinterrupted = 0;
#endif /* NTASM */
    if (
#ifdef NTASM
		 sj_buf->retcode
#else
		 cksetjmp(ckjdref(sj_buf))
#endif /* NTASM */
		) {
	(*failfunc)(NULL) ;
	rc = -1 ;
    } else {
#ifdef NT
       __try {
          (*dofunc)(NULL) ;
       }
       __except( exception_filter() )
       {
          debug(F100,"alrm_execute __except","",0);
          debug(F111,"exception_filter",
		"_exception_code",
		GetExceptionCode()
		);
          longjmp(ckjdref(sj_buf),SIGINT);
       }
#else /* NT */
       (*dofunc)(NULL) ;
#endif /* NT */
    }
    alarm(savalrm) ;
    if ( savhandler )
        signal( SIGALRM, savhandler ) ;
    return rc ;
}

int
#ifdef CK_ANSIC				/* ANSIC C declaration... */
cc_alrm_execute(ckjptr(sj_buf),
		int timo,
		ck_sighand handler,
		ck_sigfunc dofunc,
		ck_sigfunc failfunc
		)

#else /* Not ANSIC C ... */

cc_alrm_execute(sj_buf,
	     timo,
	     handler,
	     dofunc,
	     failfunc
	     )
    ckjptr(sj_buf);
    int timo;
    ck_sighand handler;
    ck_sigfunc dofunc;
    ck_sigfunc failfunc;
#endif /* CK_ANSIC */

/* cc_alrm_execute */ {

    int rc = 0;
    int savalrm = 0;
_PROTOTYP(SIGTYP (*savhandler), (int));
    savalrm = alarm(timo);
    savhandler = signal( SIGALRM, handler );

#ifdef NTASM
    sj_buf->retcode = 0 ;
    sj_buf->Id = GetCurrentThreadId() ;
    memset( &sj_buf->context, 0, sizeof(CONTEXT) );
    sj_buf->context.ContextFlags = CONTEXT_FULL ;
#ifndef COMMENT
    GetThreadContext( GetCurrentThread(), &(sj_buf->context) ) ;
#else
   __asm
   {
      mov eax, dword ptr [sj_buf]
      push eax
      mov eax, 0xfffffffe
      push eax
      mov eax, 0x00000039
      mov edx,esp
      int 0x2e
      pop eax
      pop eax
   }
#endif
    isinterrupted = 0;
#endif /* NTASM */
    if (
#ifdef NTASM
		 sj_buf->retcode
#else
		 cksetjmp(ckjdref(sj_buf))
#endif /* NTASM */
		) {
	(*failfunc)(NULL) ;
	rc = -1 ;
    } else {
#ifdef NT
       __try {
          (*dofunc)(NULL) ;
       }
       __except( exception_filter() )
       {
	   debug(F100,"cc_alrm_execute __except","",0);
	   debug(F111,
		 "exception_filter",
		 "_exception_code",
		 GetExceptionCode()
		 );
	   longjmp(ckjdref(sj_buf),SIGINT) ;
       }
#else /* NT */
       (*dofunc)(NULL) ;
#endif /* NT */
    }
    alarm(savalrm);
    if (savhandler)
      signal(SIGALRM,savhandler);
    return(rc);
}
