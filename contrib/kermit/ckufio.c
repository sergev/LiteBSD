/* C K U F I O  --  Kermit file system support for UNIX, Aegis, and Plan 9 */

#define CK_NONBLOCK                     /* See zoutdump() */

#ifdef aegis
char *ckzv = "Aegis File support, 9.0.216, 20 Aug 2011";
#else
#ifdef Plan9
char *ckzv = "Plan 9 File support, 9.0.216, 20 Aug 2011";
#else
char *ckzv = "UNIX File support, 9.0.216, 20 Aug 2011";
#endif /* Plan9 */
#endif /* aegis */
/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City,
  and others noted in the comments below.  Note: CUCCA = Previous name of
  Columbia University Academic Information Systems.  Note: AcIS = Previous
  of Columbia University Information Technology.

  Copyright (C) 1985, 2011,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  NOTE TO CONTRIBUTORS: This file, and all the other C-Kermit files, must be
  compatible with C preprocessors that support only #ifdef, #else, #endif,
  #define, and #undef.  Please do not use #if, logical operators, or other
  preprocessor features in any of the portable C-Kermit modules.  You can,
  of course, use these constructions in platform-specific modules where you
  know they are supported.
*/
/* Include Files */

#ifdef MINIX2
#define _MINIX
#endif /* MINIX2 */

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcasc.h"

#ifndef NOCSETS
#include "ckcxla.h"
#endif /* NOCSETS */

/* To avoid pulling in all of ckuusr.h so we copy the few needed prototypes */

struct mtab {				/* Macro table, like keyword table */
    char *kwd;				/* But with pointers for vals */
    char *mval;				/* instead of ints. */
    short flgs;
};
_PROTOTYP( int mlook, (struct mtab [], char *, int) );
_PROTOTYP( int dodo, (int, char *, int) );
_PROTOTYP( int parser, ( int ) );

#ifdef COMMENT
/* This causes trouble in C-Kermit 8.0.  I don't remember the original */
/* reason for this being here but it must have been needed at the time... */
#ifdef OSF13
#ifdef CK_ANSIC
#ifdef _NO_PROTO
#undef _NO_PROTO
#endif /* _NO_PROTO */
#endif /* CK_ANSIC */
#endif /* OSF13 */
#endif /* COMMENT */

#ifndef HPUXPRE65
#include <errno.h>			/* Error number symbols */
#else
#ifndef ERRNO_INCLUDED
#include <errno.h>			/* Error number symbols */
#endif	/* ERRNO_INCLUDED */
#endif	/* HPUXPRE65 */

#include <signal.h>

#ifdef MINIX2
#undef MINIX
#undef CKSYSLOG
#include <limits.h>
#include <time.h>
#define NOFILEH
#endif /* MINIX2 */

#ifdef MINIX
#include <limits.h>
#include <sys/types.h>
#include <time.h>
#else
#ifdef POSIX
#include <limits.h>
#else
#ifdef SVR3
#include <limits.h>
#endif /* SVR3 */
#endif /* POSIX */
#endif /* MINIX */
/*
  Directory Separator macros, to allow this module to work with both UNIX and
  OS/2: Because of ambiguity with the command line editor escape \ character,
  the directory separator is currently left as / for OS/2 too, because the
  OS/2 kernel also accepts / as directory separator.  But this is subject to
  change in future versions to conform to the normal OS/2 style.
*/
#ifndef DIRSEP
#define DIRSEP       '/'
#endif /* DIRSEP */
#ifndef ISDIRSEP
#define ISDIRSEP(c)  ((c)=='/')
#endif /* ISDIRSEP */

#ifdef SDIRENT
#define DIRENT
#endif /* SDIRENT */

#ifdef XNDIR
#include <sys/ndir.h>
#else /* !XNDIR */
#ifdef NDIR
#include <ndir.h>
#else /* !NDIR, !XNDIR */
#ifdef RTU
#include "/usr/lib/ndir.h"
#else /* !RTU, !NDIR, !XNDIR */
#ifdef DIRENT
#ifdef SDIRENT
#include <sys/dirent.h>
#else
#include <dirent.h>
#endif /* SDIRENT */
#else
#include <sys/dir.h>
#endif /* DIRENT */
#endif /* RTU */
#endif /* NDIR */
#endif /* XNDIR */

#ifdef UNIX                             /* Pointer arg to wait() allowed */
#define CK_CHILD                        /* Assume this is safe in all UNIX */
#endif /* UNIX */

extern int binary, recursive, stathack;
#ifdef CK_CTRLZ
extern int eofmethod;
#endif /* CK_CTRLZ */

#include <pwd.h>                        /* Password file for shell name */
#ifdef CK_SRP
#include <t_pwd.h>                      /* SRP Password file */
#endif /* CK_SRP */

#ifdef HPUX10_TRUSTED
#include <hpsecurity.h>
#include <prot.h>
#endif /* HPUX10_TRUSTED */

#ifdef COMMENT
/* Moved to ckcdeb.h */
#ifdef POSIX
#define UTIMEH
#else
#ifdef HPUX9
#define UTIMEH
#endif /* HPUX9 */
#endif /* POSIX */
#endif /* COMMENT */

#ifdef SYSUTIMEH                        /* <sys/utime.h> if requested,  */
#include <sys/utime.h>                  /* for extra fields required by */
#else                                   /* 88Open spec. */
#ifdef UTIMEH                           /* or <utime.h> if requested */
#include <utime.h>                      /* (SVR4, POSIX) */
#ifndef BSD44
#ifndef V7
/* Not sure why this is here.  What it implies is that the code bracketed
   by SYSUTIMEH is valid on all platforms on which we support time 
   functionality.  But we know that is not true because the BSD44 and V7
   platforms do not support sys/utime.h and the data structures which
   are defined in them.  Now this worked before because prior to today's
   changes the UTIMEH definition for BSD44 and V7 did not take place
   until after SYSUTIMEH was defined.  It also would not have been a 
   problem if the ordering of all the time blocks was consistent.  All but
   one of the blocks were BSD44, V7, SYSUTIMEH, <OTHER>.  That one case
   is where this problem was triggered.
*/
#define SYSUTIMEH                       /* Use this for both cases. */
#endif /* V7 */
#endif /* BSD44 */
#endif /* UTIMEH */
#endif /* SYSUTIMEH */

#ifndef NOTIMESTAMP
#ifdef POSIX
#ifndef AS400
#define TIMESTAMP
#endif /* AS400 */
#endif /* POSIX */

#ifdef BSD44                            /* BSD 4.4 */
#ifndef TIMESTAMP
#define TIMESTAMP                       /* Can do file dates */
#endif /* TIMESTAMP */
#include <sys/time.h>
#include <sys/timeb.h>

#else  /* Not BSD44 */

#ifdef BSD4                             /* BSD 4.3 and below */
#define TIMESTAMP                       /* Can do file dates */
#include <time.h>                       /* Need this */
#include <sys/timeb.h>                  /* Need this if really BSD */

#else  /* Not BSD 4.3 and below */

#ifdef SVORPOSIX                        /* System V or POSIX */
#ifndef TIMESTAMP
#define TIMESTAMP
#endif /* TIMESTAMP */
#include <time.h>

/* void tzset(); (the "void" type upsets some compilers) */
#ifndef IRIX60
#ifndef ultrix
#ifndef CONVEX9
/* ConvexOS 9.0, supposedly POSIX, has extern char *timezone(int,int) */
#ifndef Plan9
extern long timezone;
#endif /* Plan9 */
#endif /* CONVEX9 */
#endif /* ultrix */
#endif /* IRIX60 */
#endif /* SVORPOSIX */
#endif /* BSD4 */
#endif /* BSD44 */

#ifdef COHERENT
#include <time.h>
#endif /* COHERENT */

/* Is `y' a leap year? */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

#endif /* NOTIMESTAMP */

#ifdef CIE
#include <stat.h>                       /* File status */
#else
#include <sys/stat.h>
#endif /* CIE */



/* Macro to alleviate isdir() calls internal to this module */

static struct stat STATBUF;
#define xisdir(a) ((stat(a,&STATBUF)==-1)?0:(S_ISDIR(STATBUF.st_mode)?1:0))

extern char uidbuf[];
extern int xferlog;
extern char * xferfile;
int iklogopen = 0;
static time_t timenow;

#define IKSDMSGLEN CKMAXPATH+512

static char iksdmsg[IKSDMSGLEN];

extern int local;

extern int server, en_mkd, en_cwd, en_del;

/*
  Functions (n is one of the predefined file numbers from ckcker.h):

   zopeni(n,name)   -- Opens an existing file for input.
   zopeno(n,name,attr,fcb) -- Opens a new file for output.
   zclose(n)        -- Closes a file.
   zchin(n,&c)      -- Gets the next character from an input file.
   zsinl(n,&s,x)    -- Read a line from file n, max len x, into address s.
   zsout(n,s)       -- Write a null-terminated string to output file, buffered.
   zsoutl(n,s)      -- Like zsout, but appends a line terminator.
   zsoutx(n,s,x)    -- Write x characters to output file, unbuffered.
   zchout(n,c)      -- Add a character to an output file, unbuffered.
   zchki(name)      -- Check if named file exists and is readable, return size.
   zchko(name)      -- Check if named file can be created.
   zchkspa(name,n)  -- Check if n bytes available to create new file, name.
   znewn(name,s)    -- Make a new unique file name based on the given name.
   zdelet(name)     -- Delete the named file.
   zxpand(string)   -- Expands the given wildcard string into a list of files.
   znext(string)    -- Returns the next file from the list in "string".
   zxrewind()       -- Rewind zxpand list.
   zxcmd(n,cmd)     -- Execute the command in a lower fork on file number n.
   zclosf()         -- Close input file associated with zxcmd()'s lower fork.
   zrtol(n1,n2)     -- Convert remote filename into local form.
   zltor(n1,n2)     -- Convert local filename into remote form.
   zchdir(dirnam)   -- Change working directory.
   zhome()          -- Return pointer to home directory name string.
   zkself()         -- Kill self, log out own job.
   zsattr(struct zattr *) -- Return attributes for file which is being sent.
   zstime(f, struct zattr *, x) - Set file creation date from attribute packet.
   zrename(old, new) -- Rename a file.
   zcopy(source,destination) -- Copy a file.
   zmkdir(path)       -- Create the directory path if possible
   zfnqfp(fname,len,fullpath) - Determine full path for file name.
   zgetfs(name)     -- return file size regardless of accessibility
   zchkpid(pid)     -- tell if PID is valid and active
*/

/* Kermit-specific includes */
/*
  Definitions here supersede those from system include files.
  ckcdeb.h is included above.
*/
#include "ckcker.h"                     /* Kermit definitions */
#include "ckucmd.h"                     /* For keyword tables */
#include "ckuver.h"                     /* Version herald */

char *ckzsys = HERALD;

/*
  File access checking ...  There are two calls to access() in this module.
  If this program is installed setuid or setgid on a Berkeley-based UNIX
  system that does NOT incorporate the saved-original-effective-uid/gid
  feature, then, when we have swapped the effective and original uid/gid,
  access() fails because it uses what it thinks are the REAL ids, but we have
  swapped them.  This occurs on systems where ANYBSD is defined, NOSETREU
  is NOT defined, and SAVEDUID is NOT defined.  So, in theory, we should take
  care of this situation like so:

    ifdef ANYBSD
    ifndef NOSETREU
    ifndef SAVEDUID
    define SW_ACC_ID
    endif
    endif
    endif

  But we can't test such a general scheme everywhere, so let's only do this
  when we know we have to...
*/
#ifdef NEXT                             /* NeXTSTEP 1.0-3.0 */
#define SW_ACC_ID
#endif /* NEXT */

/* Support for tilde-expansion in file and directory names */

#ifdef POSIX
#define NAMEENV "LOGNAME"
#else
#ifdef BSD4
#define NAMEENV "USER"
#else
#ifdef ATTSV
#define NAMEENV "LOGNAME"
#endif /* ATTSV */
#endif /* BSD4 */
#endif /* POSIX */

/* Berkeley Unix Version 4.x */
/* 4.1bsd support from Charles E Brooks, EDN-VAX */

#ifdef BSD4
#ifdef MAXNAMLEN
#define BSD42
#endif /* MAXNAMLEN */
#endif /* BSD4 */

/* Definitions of some system commands */

char *DELCMD = "rm -f ";                /* For file deletion */
char *CPYCMD = "cp ";                   /* For file copy */
char *RENCMD = "mv ";                   /* For file rename */
char *PWDCMD = "pwd ";                  /* For saying where I am */

#ifdef COMMENT
#ifdef HPUX10
char *DIRCMD = "/usr/bin/ls -l ";       /* For directory listing */
char *DIRCM2 = "/usr/bin/ls -l ";       /* For directory listing, no args */
#else
char *DIRCMD = "/bin/ls -l ";           /* For directory listing */
char *DIRCM2 = "/bin/ls -l ";           /* For directory listing, no args */
#endif /* HPUX10 */
#else
char *DIRCMD = "ls -l ";                /* For directory listing */
char *DIRCM2 = "ls -l ";                /* For directory listing, no args */
#endif /* COMMENT */

char *TYPCMD = "cat ";                  /* For typing a file */

#ifdef HPUX
char *MAILCMD = "mailx";                /* For sending mail */
#else
#ifdef DGUX540
char *MAILCMD = "mailx";
#else
#ifdef UNIX
#ifdef CK_MAILCMD
char *MAILCMD = CK_MAILCMD;		/* CFLAGS override */
#else
char *MAILCMD = "Mail";			/* Default */
#endif /* CK_MAILCMD */
#else
char *MAILCMD = "";
#endif /* UNIX */
#endif /* HPUX */
#endif /* DGUX40 */

#ifdef UNIX
#ifdef ANYBSD                           /* BSD uses lpr to spool */
#ifdef DGUX540                          /* And DG/UX */
char * PRINTCMD = "lp";
#else
char * PRINTCMD = "lpr";
#endif /* DGUX540 */
#else                                   /* Sys V uses lp */
#ifdef TRS16                            /* except for Tandy-16/6000... */
char * PRINTCMD = "lpr";
#else
char * PRINTCMD = "lp";
#endif /* TRS16 */
#endif /* ANYBSD */
#else  /* Not UNIX */
#define PRINTCMD ""
#endif /* UNIX */

#ifdef FT18                             /* Fortune For:Pro 1.8 */
#undef BSD4
#endif /* FT18 */

#ifdef BSD4
char *SPACMD = "pwd ; df .";            /* Space in current directory */
#else
#ifdef FT18
char *SPACMD = "pwd ; du ; df .";
#else
char *SPACMD = "df ";
#endif /* FT18 */
#endif /* BSD4 */

char *SPACM2 = "df ";                   /* For space in specified directory */

#ifdef FT18
#define BSD4
#endif /* FT18 */

#ifdef BSD4
char *WHOCMD = "finger ";
#else
char *WHOCMD = "who ";
#endif /* BSD4 */

/* More system-dependent includes, which depend on symbols defined */
/* in the Kermit-specific includes.  Oh what a tangled web we weave... */

#ifdef COHERENT                         /* <sys/file.h> */
#define NOFILEH
#endif /* COHERENT */

#ifdef MINIX
#define NOFILEH
#endif /* MINIX */

#ifdef aegis
#define NOFILEH
#endif /* aegis */

#ifdef unos
#define NOFILEH
#endif /* unos */

#ifndef NOFILEH
#include <sys/file.h>
#endif /* NOFILEH */

#ifndef is68k                           /* Whether to include <fcntl.h> */
#ifndef BSD41                           /* All but a couple UNIXes have it. */
#ifndef FT18
#ifndef COHERENT
#include <fcntl.h>
#endif /* COHERENT */
#endif /* FT18  */
#endif /* BSD41 */
#endif /* is68k */

#ifdef COHERENT
#ifdef _I386
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif /* _I386 */
#endif /* COHERENT */

extern int inserver;			/* I am IKSD */
int guest = 0;                          /* Anonymous user */

#ifdef IKSD
extern int isguest;
extern char * anonroot;
#endif /* IKSD */

#ifdef CK_LOGIN
#define GUESTPASS 256
static char guestpass[GUESTPASS] = { NUL, NUL }; /* Anonymous "password" */
static int logged_in = 0;               /* Set when user is logged in */
static int askpasswd = 0;               /* Have OK user, must ask for passwd */
#ifdef CK_PAM
extern int gotemptypasswd;
#endif /* CK_PAM */
#endif /* CK_LOGIN */

#ifdef CKROOT
static char ckroot[CKMAXPATH+1] = { NUL, NUL };
static int ckrootset = 0;
int ckrooterr = 0;
#endif /* CKROOT */

_PROTOTYP( VOID ignorsigs, (void) );
_PROTOTYP( VOID restorsigs, (void) );
#ifdef SELECT
_PROTOTYP( int ttwait, (int, int) );	/* ckutio.c */
#endif	/* SELECT */

/*
  Change argument to "(const char *)" if this causes trouble.
  Or... if it causes trouble, then maybe it was already declared
  in a header file after all, so you can remove this prototype.
*/
#ifndef NDGPWNAM /* If not defined No Declare getpwnam... */
#ifndef _POSIX_SOURCE
#ifndef NEXT
#ifndef SVR4
/* POSIX <pwd.h> already gave prototypes for these. */
#ifdef IRIX40
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef IRIX51
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef M_UNIX
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef HPUX9
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef HPUX10
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
#ifdef DCGPWNAM
_PROTOTYP( struct passwd * getpwnam, (const char *) );
#else
_PROTOTYP( struct passwd * getpwnam, (char *) );
#endif /* DCGPWNAM */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* M_UNIX */
#endif /* IRIX51 */
#endif /* IRIX40 */
#ifndef SUNOS4
#ifndef HPUX9
#ifndef HPUX10
#ifndef _SCO_DS
_PROTOTYP( struct passwd * getpwuid, (PWID_T) );
#endif /* _SCO_DS */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* SUNOS4 */
_PROTOTYP( struct passwd * getpwent, (void) );
#endif /* SVR4 */
#endif /* NEXT */
#endif /* _POSIX_SOURCE */
#endif /* NDGPWNAM */

#ifdef CK_SHADOW                        /* Shadow Passwords... */
#include <shadow.h>
#endif /* CK_SHADOW */
#ifdef CK_PAM                           /* PAM... */
#ifdef MACOSX
#include <pam/pam_appl.h>
#else /* MACOSX */
#include <security/pam_appl.h>
#endif /* MACOSX */
#ifndef PAM_SERVICE_TYPE                /* Defines which PAM service we are */
#define PAM_SERVICE_TYPE "kermit"
#endif /* PAM_SERVICE_TYPE */

#ifdef SOLARIS
#define PAM_CONST 
#else /* SOLARIS */
#define PAM_CONST CONST
#endif 

static char * pam_pw = NULL;

int
#ifdef CK_ANSIC
pam_cb(int num_msg,
       PAM_CONST struct pam_message **msg,
       struct pam_response **resp,
       void *appdata_ptr
       )
#else /* CK_ANSIC */
pam_cb(num_msg, msg, resp, appdata_ptr)
    int num_msg;
    PAM_CONST struct pam_message **msg;
    struct pam_response **resp;
    void *appdata_ptr;
#endif /* CK_ANSIC */
{
    int i;

    debug(F111,"pam_cb","num_msg",num_msg);

    for (i = 0; i < num_msg; i++) {
        char message[PAM_MAX_MSG_SIZE];

        /* Issue prompt and get response */
        debug(F111,"pam_cb","Message",i);
        debug(F111,"pam_cb",msg[i]->msg,msg[i]->msg_style);
        if (msg[i]->msg_style == PAM_ERROR_MSG) {
            debug(F111,"pam_cb","PAM ERROR",0);
            fprintf(stdout,"%s\n", msg[i]->msg);
            return(0);
        } else if (msg[i]->msg_style == PAM_TEXT_INFO) {
            debug(F111,"pam_cb","PAM TEXT INFO",0);
            fprintf(stdout,"%s\n", msg[i]->msg);
            return(0);
        } else if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF) {
            debug(F111,"pam_cb","Reading response, no echo",0);
            /* Ugly hack.  We check to see if a password has been pushed */
            /* into zvpasswd().  This would be true if the password was  */
            /* received by REMOTE LOGIN.                                 */
            if (pam_pw) {
                ckstrncpy(message,pam_pw,PAM_MAX_MSG_SIZE);
            } else
                readpass((char *)msg[i]->msg,message,PAM_MAX_MSG_SIZE);
        } else if (msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
            debug(F111,"pam_cb","Reading response, with echo",0);
            readtext((char *)msg[i]->msg,message,PAM_MAX_MSG_SIZE);
        } else {
            debug(F111,"pam_cb","unknown style",0);
            return(0);
        }

        /* Allocate space for this message's response structure */
        resp[i] = (struct pam_response *) malloc(sizeof (struct pam_response));
        if (!resp[i]) {
            int j;
            debug(F110,"pam_cb","malloc failure",0);
            for (j = 0; j < i; j++) {
                free(resp[j]->resp);
                free(resp[j]);
            }
            return(0);
        }

        /* Allocate a buffer for the response */
        resp[i]->resp = (char *) malloc((int)strlen(message) + 1);
        if (!resp[i]->resp) {
            int j;
            debug(F110,"pam_cb","malloc failure",0);
            for (j = 0; j < i; j++) {
                free(resp[j]->resp);
                free(resp[j]);
            }
            free(resp[i]);
            return(0);
        }
        /* Return the results back to PAM */
        strcpy(resp[i]->resp, message);	/* safe (prechecked) */
        resp[i]->resp_retcode = 0;
    }
    debug(F110,"pam_cb","Exiting",0);
    return(0);
}
#endif /* CK_PAM */

/* Define macros for getting file type */

#ifdef OXOS
/*
  Olivetti X/OS 2.3 has S_ISREG and S_ISDIR defined
  incorrectly, so we force their redefinition.
*/
#undef S_ISREG
#undef S_ISDIR
#endif /* OXOS */

#ifdef UTSV                             /* Same deal for Amdahl UTSV */
#undef S_ISREG
#undef S_ISDIR
#endif /* UTSV */

#ifdef UNISYS52                         /* And for UNISYS UTS V 5.2 */
#undef S_ISREG
#undef S_ISDIR
#endif /* UNISYS52 */

#ifdef ICLSVR3                          /* And for old ICL versions */
#undef S_ISREG
#undef S_ISDIR
#endif /* ICLSVR3 */

#ifdef ISDIRBUG                         /* Also allow this from command line */
#ifdef S_ISREG
#undef S_ISREG
#endif /* S_ISREG */
#ifdef S_ISDIR
#undef S_ISDIR
#endif /*  S_ISDIR */
#endif /* ISDIRBUG */

#ifndef _IFMT
#ifdef S_IFMT
#define _IFMT S_IFMT
#else
#define _IFMT 0170000
#endif /* S_IFMT */
#endif /* _IFMT */

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* S_ISREG */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* S_ISDIR */

/* The following mainly for NeXTSTEP... */

#ifndef S_IWUSR
#define S_IWUSR 0000200
#endif /* S_IWUSR */

#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif /* S_IRGRP */

#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif /* S_IWGRP */

#ifndef S_IXGRP
#define S_IXGRP 0000010
#endif /* S_IXGRP */

#ifndef S_IROTH
#define S_IROTH 0000004
#endif /* S_IROTH */

#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif /* S_IWOTH */

#ifndef S_IXOTH
#define S_IXOTH 0000001
#endif /* S_IXOTH */
/*
  Define maximum length for a file name if not already defined.
  NOTE: This applies to a path segment (directory or file name),
  not the entire path string, which can be CKMAXPATH bytes long.
*/
#ifdef QNX
#ifdef _MAX_FNAME
#define MAXNAMLEN _MAX_FNAME
#else
#define MAXNAMLEN 48
#endif /* _MAX_FNAME */
#else
#ifndef MAXNAMLEN
#ifdef sun
#define MAXNAMLEN 255
#else
#ifdef FILENAME_MAX
#define MAXNAMLEN FILENAME_MAX
#else
#ifdef NAME_MAX
#define MAXNAMLEN NAME_MAX
#else
#ifdef _POSIX_NAME_MAX
#define MAXNAMLEN _POSIX_NAME_MAX
#else
#ifdef _D_NAME_MAX
#define MAXNAMLEN _D_NAME_MAX
#else
#ifdef DIRSIZ
#define MAXNAMLEN DIRSIZ
#else
#define MAXNAMLEN 14
#endif /* DIRSIZ */
#endif /* _D_NAME_MAX */
#endif /* _POSIX_NAME_MAX */
#endif /* NAME_MAX */
#endif /* FILENAME_MAX */
#endif /* sun */
#endif /* MAXNAMLEN */
#endif /* QNX */

#ifdef COMMENT
/* As of 2001-11-03 this is handled in ckcdeb.h */
/* Longest pathname ... */
/*
  Beware: MAXPATHLEN is one of UNIX's dirty little secrets.  Where is it
  defined?  Who knows...  <param.h>, <mod.h>, <unistd.h>, <limits.h>, ...
  There is not necessarily even a definition for it anywhere, or it might have
  another name.  If you get it wrong, bad things happen with getcwd() and/or
  getwd().  If you allocate a buffer that is too short, getwd() might write
  over memory and getcwd() will fail with ERANGE.  The definitions of these
  functions (e.g. in SVID or POSIX.1) do not tell you how to determine the
  maximum path length in order to allocate a buffer that is the right size.
*/
#ifdef BSD44
#include <sys/param.h>                  /* For MAXPATHLEN */
#endif /* BSD44 */
#ifdef COHERENT
#include <sys/param.h>  /* for MAXPATHLEN, needed for -DDIRENT */
#endif /* COHERENT */
#endif /* COMMENT */

#ifdef MAXPATHLEN
#ifdef MAXPATH
#undef MAXPATH
#endif /* MAXPATH */
#define MAXPATH MAXPATHLEN
#else
#ifdef PATH_MAX
#define MAXPATH PATH_MAX
#else
#ifdef _POSIX_PATH_MAX
#define MAXPATH _POSIX_PATH_MAX
#else
#ifdef BSD42
#define MAXPATH 1024
#else
#ifdef SVR4
#define MAXPATH 1024
#else
#define MAXPATH 255
#endif /* SVR4 */
#endif /* BSD42 */
#endif /* _POSIX_PATH_MAX */
#endif /* PATH_MAX */
#endif /* MAXPATHLEN */

/* Maximum number of filenames for wildcard expansion */

#ifndef MAXWLD
/* Already defined in ckcdeb.h so the following is superfluous. */
/* Don't expect changing them to have any effect. */
#ifdef CK_SMALL
#define MAXWLD 50
#else
#ifdef BIGBUFOK
#define MAXWLD 102400
#else
#define MAXWLD 8192
#endif /* BIGBUFOK */
#endif /* CK_SMALL */
#endif /* MAXWLD */

static int maxnames = MAXWLD;

/* Define the size of the string space for filename expansion. */

#ifndef DYNAMIC
#ifdef PROVX1
#define SSPACE 500
#else
#ifdef BSD29
#define SSPACE 500
#else
#ifdef pdp11
#define SSPACE 500
#else
#ifdef aegis
#define SSPACE 10000                    /* Size of string-generating buffer */
#else                                   /* Default static buffer size */
#ifdef BIGBUFOK
#define SSPACE 65000                    /* Size of string-generating buffer */
#else
#define SSPACE 2000                     /* size of string-generating buffer */
#endif /* BIGBUFOK */
#endif /* aegis */
#endif /* pdp11 */
#endif /* BSD29 */
#endif /* PROVX1 */
static char sspace[SSPACE];             /* Buffer for generating filenames */
#else /* is DYNAMIC */
#ifdef CK_64BIT
#define SSPACE 2000000000		/* Two billion bytes */
#else
#ifdef BIGBUFOK
#define SSPACE 10000000			/* Ten million */
#else
#define SSPACE 10000			/* Ten thousand */
#endif /* BIGBUFOK */
#endif	/* CK_64BIT */
char *sspace = (char *)0;
#endif /* DYNAMIC */
static int ssplen = SSPACE;		/* Length of string space buffer */

#ifdef DCLFDOPEN
/* fdopen() needs declaring because it's not declared in <stdio.h> */
_PROTOTYP( FILE * fdopen, (int, char *) );
#endif /* DCLFDOPEN */

#ifdef DCLPOPEN
/* popen() needs declaring because it's not declared in <stdio.h> */
_PROTOTYP( FILE * popen, (char *, char *) );
#endif /* DCLPOPEN */

extern int nopush;

/* More internal function prototypes */
/*
 * The path structure is used to represent the name to match.
 * Each slash-separated segment of the name is kept in one
 * such structure, and they are linked together, to make
 * traversing the name easier.
 */
struct path {
    char npart[MAXNAMLEN+4];            /* name part of path segment */
    struct path *fwd;                   /* forward ptr */
};
#ifndef NOPUSH
_PROTOTYP( int shxpand, (char *, char *[], int ) );
#endif /* NOPUSH */
_PROTOTYP( static int fgen, (char *, char *[], int ) );
_PROTOTYP( static VOID traverse, (struct path *, char *, char *) );
_PROTOTYP( static VOID addresult, (char *, int) );
#ifdef COMMENT
/* Replaced by ckmatch() */
_PROTOTYP( static int match, (char *, char *) );
#endif /* COMMENT */
_PROTOTYP( char * whoami, (void) );
_PROTOTYP( UID_T real_uid, (void) );
_PROTOTYP( static struct path *splitpath, (char *p) );
_PROTOTYP( char * zdtstr, (time_t) );
_PROTOTYP( time_t zstrdt, (char *, int) );

/* Some systems define these symbols in include files, others don't... */

#ifndef R_OK
#define R_OK 4                          /* For access */
#endif /* R_OK */

#ifndef W_OK
#define W_OK 2
#endif /* W_OK */

#ifndef X_OK
#define X_OK 1
#endif /* X_OK */

#ifndef O_RDONLY
#define O_RDONLY 000
#endif /* O_RDONLY */

/* syslog and wtmp items for Internet Kermit Service */

extern char * clienthost;               /* From ckcmai.c. */

static char fullname[CKMAXPATH+1];
static char tmp2[CKMAXPATH+1];

extern int ckxlogging;

#ifdef CKXPRINTF                        /* Our printf macro conflicts with */
#undef printf                           /* use of "printf" in syslog.h */
#endif /* CKXPRINTF */
#ifdef CKSYSLOG
#ifdef RTAIX
#include <sys/syslog.h>
#else  /* RTAIX */
#include <syslog.h>
#endif /* RTAIX */
#endif /* CKSYSLOG */
#ifdef CKXPRINTF
#define printf ckxprintf
#endif /* CKXPRINTF */

int ckxanon = 1;                        /* Anonymous login ok */
int ckxperms = 0040;                    /* Anonymous file permissions */
int ckxpriv = 1;			/* Priv'd login ok */

#ifndef XFERFILE
#define XFERFILE "/var/log/iksd.log"
#endif /* XFERFILE */

/* wtmp logging for IKSD... */

#ifndef CKWTMP                          /* wtmp logging not selected */
int ckxwtmp = 0;                        /* Know this at runtime */
#else                                   /* wtmp file details */
int ckxwtmp = 1;
#ifdef UTMPBUG                          /* Unfortunately... */
/*
  Some versions of Linux have a <utmp.h> file that contains
  "enum utlogin { local, telnet, rlogin, screen, ... };"  This clobbers
  any program that uses any of these words as variable names, function
  names, macro names, etc.  (Other versions of Linux have this declaration
  within #if 0 ... #endif.)  There is nothing we can do about this other
  than to not include the stupid file.  But we need stuff from it, so...
*/
#include <features.h>
#include <sys/types.h>
#define UT_LINESIZE     32
#define UT_NAMESIZE     32
#define UT_HOSTSIZE     256

struct timeval {
  time_t tv_sec;
  time_t tv_usec;
};

struct exit_status {
  short int e_termination;      /* Process termination status.  */
  short int e_exit;             /* Process exit status.  */
};

struct utmp {
  short int ut_type;                    /* Type of login */
  pid_t ut_pid;                         /* Pid of login process */
  char ut_line[UT_LINESIZE];            /* NUL-terminated devicename of tty */
  char ut_id[4];                        /* Inittab id */
  char ut_user[UT_NAMESIZE];            /* Username (not NUL terminated) */

  char ut_host[UT_HOSTSIZE];            /* Hostname for remote login */
  struct exit_status ut_exit;           /* Exit status */
  long ut_session;                      /* Session ID, used for windowing */
  struct timeval ut_tv;                 /* Time entry was made */
  int32_t ut_addr_v6[4];                /* Internet address of remote host */
  char pad[20];                         /* Reserved */
};

#define ut_time ut_tv.tv_sec    /* Why should Linux be like anything else? */
#define ut_name ut_user         /* ... */

extern void
logwtmp __P ((__const char *__ut_line, __const char *__ut_name,
                          __const char *__ut_host));

#else  /* Not UTMPBUG */

#ifndef HAVEUTMPX                       /* Who has <utmpx.h> */
#ifdef SOLARIS
#define HAVEUTMPX
#else
#ifdef IRIX60
#define HAVEUTMPX
#else
#ifdef CK_SCOV5
#define HAVEUTMPX
#else
#ifdef HPUX100
#define HAVEUTMPX
#else
#ifdef UNIXWARE
#define HAVEUTMPX
#endif /* UNIXWARE */
#endif /* HPUX100 */
#endif /* CK_SCOV5 */
#endif /* IRIX60 */
#endif /* SOLARIS */
#endif /* HAVEUTMPX */

#ifdef HAVEUTMPX
#include <utmpx.h>
#else
#ifdef OSF50
/* Because the time_t in the utmp struct is 64 bits but time() wants 32 */
#define __V40_OBJ_COMPAT 1
#endif /* OSF50 */
#include <utmp.h>
#ifdef OSF50
#undef __V40_OBJ_COMPAT
#endif /* OSF50 */
#endif /* HAVEUTMPX */
#endif /* UTMPBUG */

#ifdef HAVEUTMPX
#define UTMPSTRUCT utmpx
#else
#define UTMPSTRUCT utmp
#endif	/* HAVEUTMPX */

#ifndef WTMPFILE
#ifdef QNX
#define WTMPFILE "/usr/adm/wtmp.1"
#else
#ifdef LINUX
#define WTMPFILE "/var/log/wtmp"
#else
#define WTMPFILE "/usr/adm/wtmp"
#endif /* QNX */
#endif /* LINUX */
#endif /* WTMPFILE */
char * wtmpfile = NULL;

static int wtmpfd = 0;
static char cksysline[32] = { NUL, NUL };

#ifndef HAVEUTHOST                      /* Does utmp include ut_host[]? */
#ifdef HAVEUTMPX                        /* utmpx always does */
#define HAVEUTHOST
#else
#ifdef LINUX                            /* Linux does */
#define HAVEUTHOST
#else
#ifdef SUNOS4                           /* SunOS does */
#define HAVEUTHOST
#else
#ifdef AIX41                            /* AIX 4.1 and later do */
#define HAVEUTHOST
#endif /* AIX41 */
#endif /* SUNOS4 */
#endif /* LINUX */
#endif /* HAVEUTMPX */
#endif /* HAVEUTHOST */

#ifdef UW200
PID_T _vfork() {                        /* To satisfy a library foulup */
    return(fork());                     /* in Unixware 2.0.x */
}
#endif /* UW200 */

VOID
#ifdef CK_ANSIC
logwtmp(const char * line, const char * name, const char * host)
#else
logwtmp(line, name, host) char *line, *name, *host;
#endif /* CK_ANSIC */
/* logwtmp */ {
    struct UTMPSTRUCT ut;
    struct stat buf;
    /* time_t time(); */

    if (!ckxwtmp)
      return;

    if (!wtmpfile)
      makestr(&wtmpfile,WTMPFILE);

    if (!line) line = "";
    if (!name) name = "";
    if (!host) host = "";

    if (!wtmpfd && (wtmpfd = open(wtmpfile, O_WRONLY|O_APPEND, 0)) < 0) {
        ckxwtmp = 0;
        debug(F110,"WTMP open failed",line,0);
        return;
    }
    if (!fstat(wtmpfd, &buf)) {
        ckstrncpy(ut.ut_line, line, sizeof(ut.ut_line));
#ifdef FREEBSD9
        ckstrncpy(ut.ut_user, name, sizeof(ut.ut_user));
#else
        ckstrncpy(ut.ut_name, name, sizeof(ut.ut_name));
#endif	/* FREEBSD9 */
#ifdef HAVEUTHOST
        /* Not portable */
        ckstrncpy(ut.ut_host, host, sizeof(ut.ut_host));
#endif /* HAVEUTHOST */
#ifdef HAVEUTMPX
        time(&ut.ut_tv.tv_sec);
#else
#ifdef LINUX
/* In light of the following comment perhaps the previous line should */
/* be "#ifndef COMMENT". */
        {
            /*
             * On 64-bit platforms sizeof(time_t) and sizeof(ut.ut_time)
             * are not the same and attempt to use an address of
             * ut.ut_time as an argument to time() call may cause
             * "unaligned access" trap.
             */
            time_t zz;
            time(&zz);
            ut.ut_time = zz;
        }
#else
#ifdef CK_64BIT
        {
	    /* Now (Jan 2006) we can do this for any 64-bit build */
            time_t zz;
            time(&zz);
            ut.ut_time = zz;
        }
#else
        time(&ut.ut_time);
#endif	/* CK_64BIT */
#endif /* LINUX */
#endif /* HAVEUTMPX */
        if (write(wtmpfd, (char *)&ut, sizeof(struct UTMPSTRUCT)) !=
            sizeof(struct UTMPSTRUCT)) {
#ifndef NOFTRUNCATE
#ifndef COHERENT
            ftruncate(wtmpfd, buf.st_size); /* Error, undo any partial write */
#else
            chsize(wtmpfd, buf.st_size); /* Error, undo any partial write */
#endif /* COHERENT */
#endif /* NOFTRUNCATE */
            debug(F110,"WTMP write error",line,0);
        } else {
            debug(F110,"WTMP record OK",line,0);
            return;
        }
    }
}
#endif /* CKWTMP */

#ifdef CKSYSLOG
/*
  C K S Y S L O G  --  C-Kermit system logging function,

  For use by other modules.
  This module can, but doesn't have to, use it.
  Call with:
    n = SYSLG_xx values defined in ckcdeb.h
    s1, s2, s3: strings.
*/
VOID
cksyslog(n, m, s1, s2, s3) int n, m; char * s1, * s2, * s3; {
    int level;

    if (!ckxlogging)                    /* syslogging */
      return;
    if (!s1) s1 = "";                   /* Fix null args */
    if (!s2) s2 = "";
    if (!s3) s3 = "";
    switch (n) {                        /* Translate Kermit level */
      case SYSLG_DB:                    /* to syslog level */
        level = LOG_DEBUG;
        break;
      default:
        level = m ? LOG_INFO : LOG_ERR;
    }
    debug(F110,"cksyslog s1",s1,0);
    debug(F110,"cksyslog s2",s2,0);
    debug(F110,"cksyslog s3",s3,0);
    errno = 0;
    syslog(level, "%s: %s %s", s1, s2, s3); /* Write syslog record */
    debug(F101,"cksyslog errno","",errno);
}
#endif /* CKSYSLOG */


/* Declarations */

int maxnam = MAXNAMLEN;                 /* Available to the outside */
int maxpath = MAXPATH;
int ck_znewn = -1;

#ifdef UNIX
char startupdir[MAXPATH+1];
#endif /* UNIX */

int pexitstat = -2;                     /* Process exit status */

FILE *fp[ZNFILS] = {                    /* File pointers */
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* Flags for each file indicating whether it was opened with popen() */
int ispipe[ZNFILS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Buffers and pointers used in buffered file input and output. */
#ifdef DYNAMIC
extern char *zinbuffer, *zoutbuffer;
#else
extern char zinbuffer[], zoutbuffer[];
#endif /* DYNAMIC */
extern char *zinptr, *zoutptr;
extern int zincnt, zoutcnt;
extern int wildxpand, wildena;		/* Wildcard handling */

static CK_OFF_T iflen = (CK_OFF_T)-1;	/* Input file length */

static PID_T pid = 0;                   /* pid of child fork */
static int fcount = 0;                  /* Number of files in wild group */
static int nxpand = 0;                  /* Copy of fcount */
static char nambuf[CKMAXPATH+4];        /* Buffer for a pathname */

#ifndef NOFRILLS
#define ZMBUFLEN 200
static char zmbuf[ZMBUFLEN];		/* For mail, remote print strings */
#endif /* NOFRILLS */

char **mtchs = NULL;                    /* Matches found for filename */
char **mtchptr = NULL;                  /* Pointer to current match */

/*  Z K S E L F  --  Kill Self: log out own job, if possible.  */

/* Note, should get current pid, but if your system doesn't have */
/* getppid(), then just kill(0,9)...  */

#ifndef SVR3
#ifndef POSIX
#ifndef OSFPC
/* Already declared in unistd.h for SVR3 and POSIX */
#ifdef CK_ANSIC
extern PID_T getppid(void);
#else
#ifndef PS2AIX10
#ifndef COHERENT
extern PID_T getppid();
#endif /* COHERENT */
#endif /* PS2AIX10 */
#endif /* CK_ANSIC */
#endif /* OSFPC */
#endif /* POSIX */
#endif /* SVR3 */

int
zkself() {                              /* For "bye", but no guarantee! */
#ifdef PROVX1
    return(kill(0,9));
#else
#ifdef V7
    return(kill(0,9));
#else
#ifdef TOWER1
    return(kill(0,9));
#else
#ifdef FT18
    return(kill(0,9));
#else
#ifdef aegis
    return(kill(0,9));
#else
#ifdef COHERENT
    return(kill((PID_T)getpid(),1));
#else
#ifdef PID_T
    exit(kill((PID_T)getppid(),1));
    return(0);
#else
    exit(kill(getppid(),1));
    return(0);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
}

static VOID
getfullname(name) char * name; {
    char *p = (char *)fullname;
    int len = 0;
    fullname[0] = '\0';
    /* If necessary we could also chase down symlinks here... */
#ifdef COMMENT
    /* This works but is incompatible with wuftpd */
    if (isguest && anonroot) {
        ckstrncpy(fullname,anonroot,CKMAXPATH);
        len = strlen(fullname);
        if (len > 0)
          if (fullname[len-1] == '/')
            len--;
    }
    p += len;
#endif /* COMMENT */
    zfnqfp(name, CKMAXPATH - len, p);
    while (*p) {
        if (*p < '!') *p = '_';
        p++;
    }
}

/*  D O I K L O G  --  Open Kermit-specific ftp-like transfer log. */

VOID                                    /* Called in ckcmai.c */
doiklog() {
    if (iklogopen)                      /* Already open? */
      return;
    if (xferlog) {                      /* Open iksd log if requested */
        if (!xferfile)                  /* If no pathname given */
          makestr(&xferfile,XFERFILE);	/* use this default */
        if (*xferfile) {
            xferlog = open(xferfile, O_WRONLY | O_APPEND | O_CREAT, 0660);
            debug(F101,"doiklog open","",xferlog);
            if (xferlog < 0) {
#ifdef CKSYSLOG
                syslog(LOG_ERR, "xferlog open failure %s: %m", xferfile);
#endif /* CKSYSLOG */
                debug(F101,"doiklog open errno","",errno);
                xferlog = 0;
            } else
              iklogopen = 1;
        } else
          xferlog = 0;
#ifdef CKSYSLOG
        if (xferlog && ckxlogging)
          syslog(LOG_INFO, "xferlog: %s open ok", xferfile);
#endif /* CKSYSLOG */
    }
}

/*  Z O P E N I  --  Open an existing file for input. */

/* Returns 1 on success, 0 on failure */

int
zopeni(n,name) int n; char *name; {
    int x;

    debug(F111,"zopeni",name,n);
    if ((x = chkfn(n)) != 0) {
	debug(F111,"zopeni chkfn",ckitoa(n),x);
	return(0);
    }
    zincnt = 0;                         /* Reset input buffer */
    if (n == ZSYSFN) {                  /* Input from a system function? */
#ifdef COMMENT
/*** Note, this function should not be called with ZSYSFN ***/
/*** Always call zxcmd() directly, and give it the real file number ***/
/*** you want to use.  ***/
        return(zxcmd(n,name));          /* Try to fork the command */
#else
        debug(F110,"zopeni called with ZSYSFN, failing!",name,0);
        *nambuf = '\0';                 /* No filename. */
        return(0);                      /* fail. */
#endif /* COMMENT */
    }
    if (n == ZSTDIO) {                  /* Standard input? */
        if (is_a_tty(0)) {
            fprintf(stderr,"Terminal input not allowed");
            debug(F110,"zopeni: attempts input from unredirected stdin","",0);
            return(0);
        }
        fp[ZIFILE] = stdin;
        ispipe[ZIFILE] = 0;
        return(1);
    }
#ifdef CKROOT
    debug(F111,"zopeni setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zopeni setroot violation",name,0);
	return(0);
    }
#endif /* CKROOT */
    fp[n] = fopen(name,"r");            /* Real file, open it. */
    /* debug(F111,"zopeni fopen", name, fp[n]); */
#ifdef ZDEBUG
    /* printf("ZOPENI fp[%d]=%ld\n",n,fp[n]); */
#endif /* ZDEBUG */
    ispipe[n] = 0;

    if (xferlog
#ifdef CKSYSLOG
        || ((ckxsyslog >= SYSLG_FA) && ckxlogging)
#endif /* CKSYSLOG */
        ) {
        getfullname(name);
        debug(F110,"zopeni fullname",fullname,0);
    }
    if (fp[n] == NULL) {
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FA && ckxlogging) {
	    syslog(LOG_INFO, "file[%d] %s: open failed (%m)", n, fullname);
	    perror(fullname);
	} else
#endif /* CKSYSLOG */
	  perror(name);
        return(0);
    } else {
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FA && ckxlogging)
          syslog(LOG_INFO, "file[%d] %s: open read ok", n, fullname);
#endif /* CKSYSLOG */
        clearerr(fp[n]);
        return(1);
    }
}

#ifdef QNX
#define DONDELAY
#else
#ifdef O_NDELAY
#define DONDELAY
#endif /* O_NDELAY */
#endif /* QNX */

/*  Z O P E N O  --  Open a new file for output.  */

/*ARGSUSED*/	/* zz not used */
int
zopeno(n,name,zz,fcb)
/* zopeno */  int n; char *name; struct zattr *zz; struct filinfo *fcb; {

    char p[8];
    int append = 0;
    int istty = 0, filefd = 0;

/* As of Version 5A, the attribute structure and the file information */
/* structure are included in the arglist. */

#ifdef DEBUG
    debug(F111,"zopeno",name,n);
    if (fcb) {
        debug(F101,"zopeno fcb disp","",fcb->dsp);
        debug(F101,"zopeno fcb type","",fcb->typ);
        debug(F101,"zopeno fcb char","",fcb->cs);
    } else {
        debug(F100,"zopeno fcb is NULL","",0);
    }
#endif /* DEBUG */

    if (chkfn(n) != 0)                  /* Already open? */
      return(0);                        /* Nothing to do. */

    if ((n == ZCTERM) || (n == ZSTDIO)) { /* Terminal or standard output */
        fp[ZOFILE] = stdout;
        ispipe[ZOFILE] = 0;
#ifdef COMMENT
	/* This seems right but it breaks client server ops */
	fp[n] = stdout;
        ispipe[n] = 0;
#endif /* COMMENT */
#ifdef COMMENT
#ifdef DEBUG
        if (n != ZDFILE)
          debug(F101,"zopeno fp[n]=stdout","",fp[n]);
#endif /* DEBUG */
#endif	/* COMMENT */
        zoutcnt = 0;
        zoutptr = zoutbuffer;
        return(1);
    }

/* A real file.  Open it in desired mode (create or append). */

#ifdef CKROOT
    debug(F111,"zopeno setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zopeno setroot violation",name,0);
	return(0);
    }
#endif /* CKROOT */

    ckstrncpy(p,"w",8);			/* Assume write/create mode */
    if (fcb) {                          /* If called with an FCB... */
        if (fcb->dsp == XYFZ_A) {       /* Does it say Append? */
            ckstrncpy(p,"a",8);		/* Yes. */
            debug(F100,"zopeno append","",0);
            append = 1;
        }
    }
    if (xferlog
#ifdef CKSYSLOG
        || ((ckxsyslog >= SYSLG_FC) && ckxlogging)
#endif /* CKSYSLOG */
        ) {
        getfullname(name);
        debug(F110,"zopeno fullname",fullname,0);
    }
    {
    /* Allow tty devices to opened as output files 2009/10/20 */
	int fd, mode = 0;
	debug(F110,"zopeno attempting to open",name,0);
#ifdef O_NONBLOCK
	mode = O_NONBLOCK;
#else
#ifdef O_NDELAY
	mode = O_NDELAY;
#else
#ifdef FNDELAY
	mode = FNDELAY;
#endif /* FNDELAY */
#endif	/* O_NDELAY */
#endif	/* O_NONBLOCK */
	debug(F111,"zopeno open mode",name,mode);
	fd = open(name,O_WRONLY,mode);
	debug(F111,"zopeno open",name,fd); 
	if (fd > -1) {
	    if (isatty(fd)) {
		filefd = fd;
		istty++;
	    }
	}
    }
    debug(F111,"zopeno istty",name,istty);
    debug(F110,"zopeno fopen arg",p,0);
    if (istty)
      fp[n] = fdopen(filefd,p);
    else
      fp[n] = fopen(name,p);		/* Try to open the file */
    ispipe[ZIFILE] = 0;

#ifdef ZDEBUG
    printf("ZOPENO fp[%d]=%ld\n",n,fp[n]);
#endif /* ZDEBUG */

    if (fp[n] == NULL) {                /* Failed */
        debug(F101,"zopeno failed errno","",errno);
	if (istty) close(filefd);
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FC && ckxlogging)
          syslog(LOG_INFO, "file[%d] %s: %s failed (%m)",
                 n,
                 fullname,
                 append ? "append" : "create"
                 );
#endif /* CKSYSLOG */
#ifdef COMMENT                          /* Let upper levels print message. */
        perror("Can't open output file");
#endif /* COMMENT */
    } else {                            /* Succeeded */
        extern int zofbuffer, zofblock, zobufsize;
        debug(F101, "zopeno zobufsize", "", zobufsize);
        if (n == ZDFILE || n == ZTFILE) { /* If debug or transaction log */
            setbuf(fp[n],NULL);           /* make it unbuffered. */
#ifdef DONDELAY
        } else if (n == ZOFILE && !zofblock) { /* blocking or nonblocking */
            int flags;
            if ((flags = fcntl(fileno(fp[n]),F_GETFL,0)) > -1)
              fcntl(fileno(fp[n]),F_SETFL, flags |
#ifdef QNX
                    O_NONBLOCK
#else
                    O_NDELAY
#endif /* QNX */
                    );
            debug(F100,"zopeno ZOFILE nonblocking","",0);
#endif /* DONDELAY */
        } else if (n == ZOFILE && !zofbuffer) { /* buffered or unbuffered */
            setbuf(fp[n],NULL);
            debug(F100,"zopeno ZOFILE unbuffered","",0);
        }

#ifdef CK_LOGIN
        /* Enforce anonymous file-creation permission */
        if (isguest)
          if (n == ZWFILE || n == ZMFILE ||
              n == ZOFILE || n == ZDFILE ||
              n == ZTFILE || n == ZPFILE ||
              n == ZSFILE)
            chmod(name,ckxperms);
#endif /* CK_LOGIN */
#ifdef CKSYSLOG
        if (ckxsyslog >= SYSLG_FC && ckxlogging)
          syslog(LOG_INFO, "file[%d] %s: %s ok",
                 n,
                 fullname,
                 append ? "append" : "create"
                 );
#endif /* CKSYSLOG */
        debug(F100, "zopeno ok", "", 0);
    }
    zoutcnt = 0;                        /* (PWP) reset output buffer */
    zoutptr = zoutbuffer;
    return((fp[n] != NULL) ? 1 : 0);
}

/*  Z C L O S E  --  Close the given file.  */

/*  Returns 0 if arg out of range, 1 if successful, -1 if close failed.  */

int
zclose(n) int n; {
    int x = 0, x2 = 0;
    extern CK_OFF_T ffc;

    debug(F101,"zclose file number","",n);
    if (chkfn(n) < 1) return(0);        /* Check range of n */
    if ((n == ZOFILE) && (zoutcnt > 0)) /* (PWP) output leftovers */
      x2 = zoutdump();

    if (fp[ZSYSFN] || ispipe[n]) {      /* If file is really pipe */
#ifndef NOPUSH
        x = zclosf(n);                  /* do it specially */
#else
        x = EOF;
#endif /* NOPUSH */
        debug(F101,"zclose zclosf","",x);
        /* debug(F101,"zclose zclosf fp[n]","",fp[n]); */
    } else {
        if ((fp[n] != stdout) && (fp[n] != stdin))
          x = fclose(fp[n]);
        fp[n] = NULL;
#ifdef COMMENT
	if (n == ZCTERM || n == ZSTDIO)	/* See zopeno() */
	  if (fp[ZOFILE] == stdout)
	    fp[ZOFILE] = NULL;
#endif /* COMMENT */
    }
    iflen = -1L;                        /* Invalidate file length */
    if (x == EOF) {                     /* if we got a close error */
        debug(F101,"zclose fclose fails","",x);
        return(-1);
    } else if (x2 < 0) {                /* or error flushing last buffer */
        debug(F101,"zclose error flushing last buffer","",x2);
        return(-1);                     /* then return an error */
    } else {
        /* Print log record compatible with wu-ftpd */
        if (xferlog && (n == ZIFILE || n == ZOFILE)) {
            char * s, *p;
            extern char ttname[];
            if (!iklogopen) (VOID) doiklog(); /* Open log if necessary */
            debug(F101,"zclose iklogopen","",iklogopen);
            if (iklogopen) {
		int len;
		char * fnam;

                timenow = time(NULL);
#ifdef CK_LOGIN
                if (logged_in)
                  s = clienthost;
                else
#endif /* CK_LOGIN */
                  s = (char *)ttname;
                if (!s) s = "";
                if (!*s) s = "*";
#ifdef CK_LOGIN
                if (logged_in) {
                    p = guestpass;
                    if (!*p) p = "*";
                } else
#endif /* CK_LOGIN */
                  p = whoami();

		len = 24 + 12 + (int)strlen(s) + 16
		  + (int)strlen(fullname) + 1 + 1 + 1 + 1
		    + (int)strlen(p) + 6 + 2 + 12;
		fnam = fullname;
		if (!*fnam) fnam = "(pipe)";

		if (len > IKSDMSGLEN)
		  sprintf(iksdmsg,	/* SAFE */
                        "%.24s [BUFFER WOULD OVERFLOW]\n",ctime(&timenow));
		else
		  sprintf(iksdmsg,	/* SAFE */
                        "%.24s %d %s %s %s %c %s %c %c %s %s %d %s\n",
                        ctime(&timenow),        /* date/time */
                        gtimer(),               /* elapsed secs */
                        s,                      /* peer name */
			ckfstoa(ffc),	        /* byte count */
                        fnam,			/* full pathname of file */
                        (binary ? 'b' : 'a'),   /* binary or ascii */
                        "_",                    /* options = none */
                        n == ZIFILE ? 'o' : 'i', /* in/out */
#ifdef CK_LOGIN
                        (isguest ? 'a' : 'r'),  /* User type */
#else
                        'r',
#endif /* CK_LOGIN */
                        p,                      /* Username or guest passwd */
#ifdef CK_LOGIN
                        logged_in ? "iks" : "kermit", /* Record ID */
#else
                        "kermit",
#endif /* CK_LOGIN */
                        0,              /* User ID on client system unknown */
                        "*"             /* Ditto */
                        );
                debug(F110,"zclose iksdmsg",iksdmsg,0);
                write(xferlog, iksdmsg, (int)strlen(iksdmsg));
            }
        }
        debug(F101,"zclose returns","",1);
        return(1);
    }
}

/*  Z C H I N  --  Get a character from the input file.  */

/*  Returns -1 if EOF, 0 otherwise with character returned in argument  */

int
zchin(n,c) int n; int *c; {
    int a;

#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        a = coninc(0);
        if (*c < 0)
          return(-1);
    } else
#endif /* IKSD */
    /* (PWP) Just in case this gets called when it shouldn't. */
    if (n == ZIFILE) {
        a = zminchar();			/* Note: this catches Ctrl-Z */
        if (a < 0)			/* (See zinfill()...) */
	  return(-1);
    } else {
	a = getc(fp[n]);
	if (a == EOF) return(-1);
#ifdef CK_CTRLZ
	/* If SET FILE EOF CTRL-Z, first Ctrl-Z marks EOF */
	if (!binary && a == 0x1A && eofmethod == XYEOF_Z)
	  return(-1);
#endif /* CK_CTRLZ */
    }
    *c = (CHAR) a & 0377;
    return(0);
}

/*  Z S I N L  --  Read a line from a file  */

/*
  Writes the line into the address provided by the caller.
  n is the Kermit "channel number".
  Writing terminates when newline is encountered, newline is not copied.
  Writing also terminates upon EOF or if length x is exhausted.
  Returns 0 on success, -1 on EOF or error.
*/
int
zsinl(n,s,x) int n, x; char *s; {
    int a, z = 0;                       /* z is return code. */
    int count = 0;
    int len = 0;
    char *buf;
    extern CHAR feol;                   /* Line terminator */

    if (!s || chkfn(n) < 1)             /* Make sure file is open, etc */
      return(-1);
    buf = s;
    s[0] = '\0';                        /* Don't return junk */

    a = -1;                             /* Current character, none yet. */
    while (x--) {                       /* Up to given length */
        int old = 0;
        if (feol)                       /* Previous character */
          old = a;
        if (zchin(n,&a) < 0) {          /* Read a character from the file */
            debug(F101,"zsinl zchin fail","",count);
            if (count == 0)
              z = -1;                   /* EOF or other error */
            break;
        } else
          count++;
        if (feol) {                     /* Single-character line terminator */
            if (a == feol)
              break;
        } else {                        /* CRLF line terminator */
            if (a == '\015')            /* CR, get next character */
              continue;
            if (old == '\015') {        /* Previous character was CR */
                if (a == '\012') {      /* This one is LF, so we have a line */
                    break;
                } else {                /* Not LF, deposit CR */
                    *s++ = '\015';
                    x--;
                    len++;
                }
            }
        }
        *s = a;                         /* Deposit character */
        s++;
        len++;
    }
    *s = '\0';                          /* Terminate the string */
    debug(F011,"zsinl",buf,len);
    return(z);
}

/*  Z X I N  --  Read x bytes from a file  */

/*
  Reads x bytes (or less) from channel n and writes them
  to the address provided by the caller.
  Returns number of bytes read on success, 0 on EOF or error.
*/
int
zxin(n,s,x) int n, x; char *s; {
#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
        int a, i;
        a = ttchk();
        if (a < 1) return(0);
        for (i = 0; i < a && i < x; i++)
          s[i] = coninc(0);
        return(i);
    }
#endif /* IKSD */

    return(fread(s, sizeof (char), x, fp[n]));
}

/*
  Z I N F I L L  --  Buffered file input.

  (re)fill the file input buffer with data.  All file input
  should go through this routine, usually by calling the zminchar()
  macro defined in ckcker.h.  Returns:

  Value 0..255 on success, the character that was read.
  -1 on end of file.
  -2 on any kind of error other than end of file.
  -3 timeout when reading from pipe (Kermit packet mode only).
*/
int
zinfill() {
    extern int kactive, srvping;
    errno = 0;

#ifdef ZDEBUG
    printf("ZINFILL fp[%d]=%ld\n",ZIFILE,fp[ZIFILE]);
#endif /* ZDEBUG */

#ifdef IKSD
    if (inserver && !local && fp[ZIFILE] == stdin) {
        int a, i;
        a = ttchk();
        if (a < 0) return(-2);
        for (i = 0; i < a && i < INBUFSIZE; i++) {
            zinbuffer[i] = coninc(0);
        }
        zincnt = i;
        /* set pointer to beginning, (== &zinbuffer[0]) */
        zinptr = zinbuffer;
        if (zincnt == 0) return(-1);
        zincnt--;                       /* One less char in buffer */
        return((int)(*zinptr++) & 0377); /* because we return the first */
    }
#endif /* IKSD */

    debug(F101,"zinfill kactive","",kactive);

    if (!(kactive && ispipe[ZIFILE])) {
        if (feof(fp[ZIFILE])) {
            debug(F100,"ZINFILL feof","",0);
#ifdef ZDEBUG
            printf("ZINFILL EOF\n");
#endif /* ZDEBUG */
            return(-1);
        }
    }
    clearerr(fp[ZIFILE]);

#ifdef SELECT
    /* Here we can call select() to get a timeout... */
    if (kactive && ispipe[ZIFILE]) {
        int secs, z = 0;
#ifndef NOXFER
        if (srvping) {
            secs = 1;
            debug(F101,"zinfill calling ttwait","",secs);
            z = ttwait(fileno(fp[ZIFILE]),secs);
            debug(F101,"zinfill ttwait","",z);
        }
#endif /* NOXFER */
        if (z == 0)
          return(-3);
    }
#endif /* SELECT */

#ifdef DEBUG
    if (deblog) {
        int i;
        debug(F101,"ZINFILL INBUFSIZE","",INBUFSIZE);
#ifdef USE_MEMCPY
        memset(zinbuffer, 0xFF, INBUFSIZE);
#else
        for (i = 0; i < INBUFSIZE; i++) {
            zinbuffer[i] = 0xFF;
#ifdef COMMENT				/* Too much! */
            debug(F101,"ZINFILL zinbuffer[i]","",i);
#endif /* COMMENT */
        }
#endif /* USE_MEMCPY */
	ckstrncpy(zinbuffer,"zinbuffer is a valid buffer",INBUFSIZE);
	/* debug(F111,"ZINFILL about to call fread",zinbuffer,zinbuffer); */
    }
#endif /* DEBUG */

/*
  Note: The following read MUST be nonblocking when reading from a pipe
  and we want timeouts to work.  See zxcmd().
*/
    zincnt = fread(zinbuffer, sizeof (char), INBUFSIZE, fp[ZIFILE]);
    debug(F101,"ZINFILL fread","",zincnt); /* Just the size */
#ifdef ZDEBUG
    printf("FREAD=%d\n",zincnt);
#endif /* ZDEBUG */
#ifdef CK_CTRLZ
    /* If SET FILE EOF CTRL-Z, first Ctrl-Z marks EOF */
    if (zincnt > 0 && !binary && eofmethod == XYEOF_Z) {
	register int i;
	for (i = 0; i < zincnt; i++) {
	    if (zinbuffer[i] == SUB) {
		zincnt = i;		/* Stop at first Ctrl-Z */
		if (i == 0)
		  return(-1);
		break;
	    }
        }
    }
#endif /* CK_CTRLZ */

    if (zincnt == 0) {                  /* Got nothing? */
        if (ferror(fp[ZIFILE])) {
            debug(F100,"ZINFILL ferror","",0);
            debug(F101,"ZINFILL errno","",errno);
#ifdef ZDEBUG
            printf("ZINFILL errno=%d\n",errno);
#endif /* ZDEBUG */
#ifdef EWOULDBLOCK
            return((errno == EWOULDBLOCK) ? -3 : -2);
#else
            return(-2);
#endif /* EWOULDBLOCK */
        }

    /* In case feof() didn't work just above -- sometimes it doesn't... */

        if (feof(fp[ZIFILE]) ) {
            debug(F100,"ZINFILL count 0 EOF return -1","",0);
            return (-1);
        } else {
            debug(F100,"ZINFILL count 0 not EOF return -2","",0);
            return(-2);
        }
    }
    zinptr = zinbuffer;    /* set pointer to beginning, (== &zinbuffer[0]) */
    zincnt--;                           /* One less char in buffer */
    return((int)(*zinptr++) & 0377);    /* because we return the first */
}

/*  Z S O U T  --  Write a string out to the given file, buffered.  */

/*  Returns 0 on success, -1 on failure */

int
zsout(n,s) int n; char *s; {
    int rc = 0;
    rc = chkfn(n);
    if (rc < 1) return(-1);             /* Keep this, prevents memory faults */
    if (!s) return(0);                  /* Null pointer, do nothing, succeed */
    if (!*s) return(0);                 /* empty string, ditto */

#ifdef IKSD
    /*
      This happens with client-side Kermit server when a REMOTE command
      was sent from the server to the client and the server is supposed to
      display the text, but of course there is no place to display it
      since it is in remote mode executing Kermit protocol.
    */
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
#ifdef COMMENT
        return(ttol(s,((int)strlen(s)) < 0) ? -1 : 0);
#else
        return(0);
#endif /* COMMENT */
    }
#endif /* IKSD */

    if (n == ZSFILE) {
	int k;
	k = strlen(s);
	rc = write(fileno(fp[n]),s,k);
	return((rc == k) ? 0 : -1);
    }
    rc = fputs(s,fp[n]) == EOF ? -1 : 0;
    if (n == ZWFILE)
      fflush(fp[n]);
    return(rc);
}

/*  Z S O U T L  --  Write string to file, with line terminator, buffered  */

/*  Returns 0 on success, -1 on failure */

int
zsoutl(n,s) int n; char *s; {
    if (zsout(n,s) < 0)
        return(-1);

#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
#ifdef COMMENT
        return(ttoc(LF));
#else
        return(0);                      /* See comments in zsout() */
#endif /* COMMENT */
    }
#endif /* IKSD */

    if (n == ZSFILE)                    /* Session log is unbuffered */
      return(write(fileno(fp[n]),"\n",1) == 1 ? 0 : -1);
    else if (fputs("\n",fp[n]) == EOF)
      return(-1);
    if (n == ZDIFIL || n == ZWFILE)     /* Flush connection log records */
      fflush(fp[n]);
    return(0);
}

/*  Z S O U T X  --  Write x characters to file, unbuffered.  */

/*  Returns number of characters written on success, -1 on failure */

int
zsoutx(n,s,x) int n, x; char *s; {
    int k;
    if (!s) return(0);
    if (!*s) return(0);

#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
#ifdef COMMENT
        return(ttol(s,x));              /* See comments in zsout() */
#else
        return(x);
#endif /* COMMENT */
    }
#endif /* IKSD */

    if ((k = (int)strlen(s)) > x) x = k; /* Nothing else would make sense */
#ifdef COMMENT
    if (chkfn(n) < 1) return(-1);
    return(write(fp[n]->_file,s,x));
#endif /* COMMENT */
    return(write(fileno(fp[n]),s,x) == x ? x : -1);
}

/*  Z C H O U T  --  Add a character to the given file.  */

/*  Should return 0 or greater on success, -1 on failure (e.g. disk full)  */

int
#ifdef CK_ANSIC
zchout(register int n, char c)
#else
zchout(n,c) register int n; char c;
#endif /* CK_ANSIC */
/* zchout() */ {
    /* if (chkfn(n) < 1) return(-1); */

#ifdef IKSD
    if (inserver && !local && (n == ZCTERM || n == ZSTDIO)) {
#ifdef COMMENT
        return(ttoc(c));
#else
        return(0);                      /* See comments in zsout() */
#endif /* COMMENT */
    }
#endif /* IKSD */

    if (n == ZSFILE)                    /* Use unbuffered for session log */
      return(write(fileno(fp[n]),&c,1) == 1 ? 0 : -1);
                                /* Buffered for everything else */
    if (putc(c,fp[n]) == EOF)   /* If true, maybe there was an error */
      return(ferror(fp[n])?-1:0);       /* Check to make sure */
    else                                /* Otherwise... */
      return(0);                        /* There was no error. */
}

/* (PWP) buffered character output routine to speed up file IO */

int
zoutdump() {
    int x;
    char * zp;
    zoutptr = zoutbuffer;               /* Reset buffer pointer in all cases */
#ifdef DEBUG
    if (deblog)
      debug(F101,"zoutdump zoutcnt","",zoutcnt);
#endif /* DEBUG */
    if (zoutcnt == 0) {                 /* Nothing to output */
        return(0);
    } else if (zoutcnt < 0) {           /* Unexpected negative argument */
        zoutcnt = 0;                    /* Reset output buffer count */
        return(-1);                     /* and fail. */
    }

#ifdef IKSD
    if (inserver && !local && fp[ZOFILE] == stdout) {
#ifdef COMMENT
        x = ttol(zoutbuffer,zoutcnt);
#else
        x = 1;                          /* See comments in zsout() */
#endif /* COMMENT */
        zoutcnt = 0;
        return(x > 0 ? 0 : -1);
    }
#endif /* IKSD */

/*
  Frank Prindle suggested that replacing this fwrite() by an fflush()
  followed by a write() would improve the efficiency, especially when
  writing to stdout.  Subsequent tests showed a 5-fold improvement.
*/
#ifdef COMMENT
    if (x = fwrite(zoutbuffer, 1, zoutcnt, fp[ZOFILE])) ...
#endif /* COMMENT */

#ifndef CK_NONBLOCK
    fflush(fp[ZOFILE]);
#endif /* CK_NONBLOCK */
    zp = zoutbuffer;
    while (zoutcnt > 0) {
        if ((x = write(fileno(fp[ZOFILE]),zp,zoutcnt)) > -1) {
#ifdef DEBUG
            if (deblog)                 /* Save a function call... */
              debug(F101,"zoutdump wrote","",x);
#endif /* DEBUG */
            zoutcnt -= x;               /* Adjust output buffer count */
            zp += x;                    /* and pointer */
        } else {
#ifdef DEBUG
            if (deblog) {
                debug(F101,"zoutdump write error","",errno);
                debug(F101,"zoutdump write returns","",x);
            }
#endif /* DEBUG */
            zoutcnt = 0;                /* Reset output buffer count */
            return(-1);                 /* write() failed */
        }
    }
    return(0);
}

/*  C H K F N  --  Internal function to verify file number is ok  */

/*
 Returns:
  -1: File number n is out of range
   0: n is in range, but file is not open
   1: n in range and file is open
*/
int
chkfn(n) int n; {
    /* if (n != ZDFILE) debug(F101,"chkfn","",n); */
    if (n < 0 || n >= ZNFILS) {
        if (n != ZDFILE) debug(F101,"chkfn out of range","",n);
        return(-1);
    } else {
        /* if (n != ZDFILE) debug(F101,"chkfn fp[n]","",fp[n]); */
        return((fp[n] == NULL) ? 0 : 1);
    }
}

/*  Z G E T F S -- Return file size regardless of accessibility */
/*
  Used for directory listings, etc.
  Returns:
    The size of the file in bytes, 0 or greater, if the size can be learned.
    -1 if the file size can not be obtained.
  Also (and this is a hack just for UNIX):
    If the argument is the name of a symbolic link,
    the global variable issymlink is set to 1,
    and the global buffer linkname[] gets the link value.
    And it sets zgfs_dir to 1 if it's a directory, otherwise 0.
  This lets us avoid numerous redundant calls to stat().
*/
int zgfs_link = 0;
int zgfs_dir = 0;
time_t zgfs_mtime = 0;
unsigned int zgfs_mode = 0;

#ifdef CKSYMLINK
char linkname[CKMAXPATH+1];
#ifndef _IFLNK
#define _IFLNK 0120000
#endif /* _IFLNK */
#endif /* CKSYMLINK */

CK_OFF_T
zgetfs(name) char *name; {
    struct stat buf;
    char fnam[CKMAXPATH+4];
    CK_OFF_T size = (CK_OFF_T)-1;
    int x;
    int needrlink = 0;
    char * s;

    if (!name) name = "";
    if (!*name) return(-1);

#ifdef UNIX
    x = strlen(name);
    if (x == 9 && !strcmp(name,"/dev/null"))
      return(0);
#endif /* UNIX */

    s = name;
#ifdef DTILDE
    if (*s == '~') {
        s = tilde_expand(s);
        if (!s) s = "";
        if (!*s) s = name;
    }
#endif /* DTILDE */
    x = ckstrncpy(fnam,s,CKMAXPATH);
    s = fnam;
    debug(F111,"zgetfs fnam",s,x);
    if (x > 0 && s[x-1] == '/')
      s[x-1] = '\0';

    zgfs_dir = 0;                       /* Assume it's not a directory */
    zgfs_link = 0;                      /* Assume it's not a symlink */
    zgfs_mtime = 0;			/* No time yet */
    zgfs_mode = 0;			/* No permission bits yet */

#ifdef CKSYMLINK                        /* We're doing symlinks? */
#ifdef USE_LSTAT                        /* OK to use lstat()? */
    x = lstat(s,&buf);
    debug(F101,"STAT","",1);
    if (x < 0)                          /* stat() failed */
      return(-1);
    if (                                /* Now see if it's a symlink */
#ifdef S_ISLNK
        S_ISLNK(buf.st_mode)
#else
#ifdef _IFLNK
        ((_IFMT & buf.st_mode) == _IFLNK)
#endif /* _IFLNK */
#endif /* S_ISLNK */
        ) {
        zgfs_link = 1;                  /* It's a symlink */
        linkname[0] = '\0';             /* Get the name */
        x = readlink(s,linkname,CKMAXPATH);
        debug(F101,"zgetfs readlink",s,x);
        if (x > -1 && x < CKMAXPATH) {  /* It's a link */
            linkname[x] = '\0';
            size = buf.st_size;         /* Remember size of link */
            x = stat(s,&buf);           /* Now stat the linked-to file */
	    debug(F101,"STAT","",2);
            if (x < 0)                  /* so we can see if it's a directory */
              return(-1);
        } else {
            ckstrncpy(linkname,"(lookup failed)",CKMAXPATH);
        }
    }
#else  /* !USE_LSTAT */
    x = stat(s,&buf);                   /* No lstat(), use stat() instead */
    debug(F101,"STAT","",3);
    if (x < 0)
      return(-1);
#endif /* USE_LSTAT */

    /* Do we need to call readlink()? */

#ifdef NOLINKBITS
/*
  lstat() does not work in SCO operating systems.  From "man NS lstat":

  lstat obtains information about the file named by path. In the case of a
  symbolic link, lstat returns information about the link, and not the file
  named by the link. It is only used by the NFS automount daemon and should
  not be utilized by users.
*/
    needrlink = 1;
    debug(F101,"zgetfs forced needrlink","",needrlink);
#else
#ifdef S_ISLNK
    needrlink = S_ISLNK(buf.st_mode);
    debug(F101,"zgetfs S_ISLNK needrlink","",needrlink);
#else
#ifdef _IFLNK
    needrlink = (_IFMT & buf.st_mode) == _IFLNK;
    debug(F101,"zgetfs _IFLNK needrlink","",needrlink);
#else
    needrlink = 1;
    debug(F101,"zgetfs default needrlink","",needrlink);
#endif /* _IFLNK */
#endif /* S_ISLNK */
#endif /* NOLINKBITS */

    if (needrlink) {
        linkname[0] = '\0';
        errno = 0;
        x = readlink(s,linkname,CKMAXPATH);
#ifdef DEBUG
        debug(F111,"zgetfs readlink",s,x);
        if (x < 0)
          debug(F101,"zgetfs readlink errno","",errno);
        else
          debug(F110,"zgetfs readlink result",linkname,0);
#endif /* DEBUG */
        if (x > -1 && x < CKMAXPATH) {
            zgfs_link = 1;
            linkname[x] = '\0';
        }
    }
#else  /* !CKSYMLINK */
    x = stat(s,&buf);                   /* Just stat the file */
    debug(F111,"zgetfs stat",s,x);
    if (x < 0)                          /* and get the size */
      return(-1);
#endif /* CKSYMLINK */

    zgfs_mtime = buf.st_mtime;
    zgfs_mode = buf.st_mode;
    zgfs_dir = (S_ISDIR(buf.st_mode)) ? 1 : 0; /* Set "is directory" flag */
    debug(F111,"zgetfs size",s,size);
    debug(F111,"zgetfs st_size",s,buf.st_size);
    return((size < 0L) ? buf.st_size : size); /* Return the size */
}


/*  Z C H K I  --  Check if input file exists and is readable  */

/*
  Returns:
   >= 0 if the file can be read (returns the size).
     -1 if file doesn't exist or can't be accessed,
     -2 if file exists but is not readable (e.g. a directory file).
     -3 if file exists but protected against read access.

  For Berkeley Unix, a file must be of type "regular" to be readable.
  Directory files, special files, and symbolic links are not readable.
*/
CK_OFF_T
zchki(name) char *name; {
    struct stat buf;
    char * s;
    int x, itsadir = 0;
    extern int zchkid, diractive, matchfifo;

    if (!name)
      return(-1);
    x = strlen(name);
    if (x < 1)
      return(-1);
    s = name;

#ifdef UNIX
    if (x == 9 && !strcmp(s,"/dev/null"))
      return(0);
    if (x == 8 && !strcmp(s,"/dev/tty"))
      return(0);
#endif /* UNIX */

#ifdef DTILDE
    if (*s == '~') {
        s = tilde_expand(s);
        if (!s) s = "";
        if (!*s) s = name;
    }
#endif /* DTILDE */

#ifdef CKROOT
    debug(F111,"zchki setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zchki setroot violation",name,0);
	return(-1);
    }
#endif /* CKROOT */

    x = stat(s,&buf);
    debug(F101,"STAT","",5);
    if (x < 0) {
        debug(F111,"zchki stat fails",s,errno);
        return(-1);
    }
    if (S_ISDIR (buf.st_mode))
      itsadir = 1;

    if (!(itsadir && zchkid)) {         /* Unless this... */
        if (!S_ISREG (buf.st_mode)      /* Must be regular file */
#ifdef S_ISFIFO
            && (!matchfifo || !S_ISFIFO (buf.st_mode))  /* or FIFO */
#endif /* S_ISFIFO */
            ) {
            debug(F111,"zchki not regular file (or fifo)",s,matchfifo);
            return(-2);
        }
    }
    debug(F111,"zchki stat ok:",s,x);

    if (diractive) {			/* If listing don't check access */
	x = 1;
    } else {
#ifdef SW_ACC_ID
	debug(F100,"zchki swapping ids for access()","",0);
	priv_on();
#endif /* SW_ACC_ID */
	if ((x = access(s,R_OK)) < 0)
	  x = access(s,X_OK);		/* For RUN-class commands */
#ifdef SW_ACC_ID
	priv_off();
	debug(F100,"zchki swapped ids restored","",0);
#endif /* SW_ACC_ID */
    }
    if (x < 0) {			/* Is the file accessible? */
        debug(F111,"zchki access failed:",s,x); /* No */
        return(-3);
    } else {
        iflen = buf.st_size;            /* Yes, remember size */
        ckstrncpy(nambuf,s,CKMAXPATH);  /* and name globally. */
        debug(F111,"zchki access ok:",s,iflen);
        return((iflen > -1L) ? iflen : 0L);
    }
}

/*  Z C H K O  --  Check if output file can be created  */

/*
  Returns -1 if write permission for the file would be denied, 0 otherwise.

  NOTE: The design is flawed.  There is no distinction among:
   . Can I overwrite an existing file?
   . Can I create a file (or directory) in an existing directory?
   . Can I create a file (or directory) and its parent(s)?
*/
int
zchko(name) char *name; {
    int i, x, itsadir = 0;
    char *s = NULL;
    char * oname;
    extern int zchkod;                  /* Used by IF WRITEABLE */

    debug(F110,"zchko entry",name,0);

    if (!name) return(-1);              /* Watch out for null pointer. */

    oname = name;

#ifdef CKROOT
    debug(F111,"zchko setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zchko setroot violation",name,0);
	errno = EACCES;
	return(-1);
    }
#endif /* CKROOT */

    x = (int)strlen(name);              /* Get length of filename */
    debug(F111,"zchko len",name,x);
    debug(F111,"zchko zchkod",name,zchkod);

#ifdef UNIX
/*
  Writing to null device is OK.
*/
    if (x == 9 && !strcmp(name,"/dev/null"))
      return(0);
    if (x == 8 && !strcmp(name,"/dev/tty"))
      return(0);
#endif /* UNIX */

    s = name;
#ifdef DTILDE
    if (*s == '~') {
        s = tilde_expand(s);
        if (!s) s = "";
        if (!*s) s = name;
	x = strlen(s);
    }
#endif /* DTILDE */
    name = s;
    s = NULL;
/*
  zchkod is a global flag meaning we're checking not to see if the directory
  file is writeable, but if it's OK to create files IN the directory.
*/
    if (!zchkod && isdir(name)) {	/* Directories are not writeable */
	debug(F111,"zchko isdir",name,1);
	return(-1);
    }
    s = malloc(x+3);                    /* Must copy because we can't */
    if (!s) {                           /* write into our argument. */
        fprintf(stderr,"zchko: Malloc error 46\n");
        return(-1);
    }
    ckstrncpy(s,name,x+3);
#ifdef UNIX
#ifdef NOUUCP
    {					/* 2009/10/20 */
    /* Allow tty devices to opened as output files */
	int fd, istty = 0, mode = 0;
	debug(F110,"zchko attempting to open",name,0);
	/* Don't block on lack of Carrier or other modem signals */
#ifdef O_NONBLOCK
	mode = O_NONBLOCK;
#else
#ifdef O_NDELAY
	mode = O_NDELAY;
#else
#ifdef FNDELAY
	mode = FNDELAY;
#endif /* FNDELAY */
#endif	/* O_NDELAY */
#endif	/* O_NONBLOCK */
	debug(F111,"zchko open mode",name,mode);
	fd = open(name,O_WRONLY,mode);	/* Must attempt to open it */
	debug(F111,"zchko open",name,fd); 
	if (fd > -1) {			/* to get a file descriptor */
	    if (isatty(fd))		/* for isatty() */
	      istty++;
	    debug(F111,"zchko isatty",name,istty);
	    fd = close(fd);
	    if (istty) {
		goto doaccess;
	    }
	} else {
	    debug(F101,"zchko open errno","",errno); 
	    x = -1;
	}
    }
#endif	/* NOUUCP */
#endif	/* UNIX */
    for (i = x; i > 0; i--) {           /* Strip filename from right. */
        if (ISDIRSEP(s[i-1])) {
            itsadir = 1;
            break;
        }
    }
    debug(F101,"zchko i","",i);
    debug(F101,"zchko itsadir","",itsadir);

#ifdef COMMENT
/* X/OPEN XPG3-compliant systems fail if argument ends with "/"...  */
    if (i == 0)                         /* If no path, use current directory */
      strcpy(s,"./");
    else                                /* Otherwise, use given one. */
      s[i] = '\0';
#else
#ifdef COMMENT
/*
  The following does not work for "foo/bar" where the foo directory does
  not exist even though we could create it: access("foo/.") fails, but
  access("foo") works OK.
*/
/* So now we use "path/." if path given, or "." if no path given. */
    s[i++] = '.';                       /* Append "." to path. */
    s[i] = '\0';
#else
/* So NOW we strip path segments from the right as long as they don't */
/* exist -- we only call access() for path segments that *do* exist.. */
/* (But this isn't quite right either since now zchko(/foo/bar/baz/xxx) */
/* succeeds when I have write access to foo and bar but baz doesn't exit.) */

    if (itsadir && i > 0) {
        s[i-1] = '\0';
        while (s[0] && !isdir(s)) {
            for (i = (int)strlen(s); i > 0; i--) {
                if (ISDIRSEP(s[i-1])) {
                    s[i-1] = '\0';
                    break;
                }
            }
            if (i == 0)
              s[0] = '\0';
        }
    } else {
        s[i++] = '.';                   /* Append "." to path. */
        s[i] = '\0';
    }
#endif /* COMMENT */
#endif /* COMMENT */

    if (!s[0])
      ckstrncpy(s,".",x+3);

  doaccess:

#ifdef SW_ACC_ID
    debug(F100,"zchko swapping ids for access()","",0);
    priv_on();
#endif /* SW_ACC_ID */

    x = access(s,W_OK);                 /* Check access of path. */

#ifdef SW_ACC_ID
    priv_off();
    debug(F100,"zchko swapped ids restored","",0);
#endif /* SW_ACC_ID */

    if (x < 0)
      debug(F111,"zchko access failed:",s,errno);
    else
      debug(F111,"zchko access ok:",s,x);
    if (s) free(s);			/* Free temporary storage */

    return((x < 0) ? -1 : 0);           /* and return. */
}

/*  Z D E L E T  --  Delete the named file.  */

/* Returns: -1 on error, 0 on success */

int
zdelet(name) char *name; {
    int x;
#ifdef CK_LOGIN
    if (isguest)
      return(-1);
#endif /* CK_LOGIN */

#ifdef CKROOT
    debug(F111,"zdelet setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zdelet setroot violation",name,0);
	return(-1);
    }
#endif /* CKROOT */

    x = unlink(name);
    debug(F111,"zdelet",name,x);
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_FC && ckxlogging) {
        fullname[0] = '\0';
        zfnqfp(name,CKMAXPATH,fullname);
        debug(F110,"zdelet fullname",fullname,0);
        if (x < 0)
          syslog(LOG_INFO, "file[] %s: delete failed (%m)", fullname);
        else
          syslog(LOG_INFO, "file[] %s: delete ok", fullname);
    }
#endif /* CKSYSLOG */
    return(x);
}

/*  Z R T O L  --  Convert remote filename into local form  */

VOID
zrtol(name,name2) char *name, *name2; {
    nzrtol(name,name2,1,0,CKMAXPATH);
}

VOID
nzrtol(name,name2,fncnv,fnrpath,max)
    char *name, *name2; int fncnv, fnrpath, max;
{ /* nzrtol */
    char *s, *p;
    int flag = 0, n = 0;
    char fullname[CKMAXPATH+1];
    int devnull = 0;
    int acase = 0;
    if (!name2) return;
    if (!name) name = "";

    debug(F111,"nzrtol name",name,fncnv);

#ifdef DTILDE
    s = name;
    if (*s == '~') {
        s = tilde_expand(s);
        if (!s) s = "";
        if (*s) name = s;
    }
#endif /* DTILDE */

    /* Handle the path -- we don't have to convert its format, since */
    /* the standard path format and our (UNIX) format are the same. */

    fullname[0] = NUL;
    devnull = !strcmp(name,"/dev/null");

    if (!devnull && fnrpath == PATH_OFF) { /* RECEIVE PATHNAMES OFF */
        zstrip(name,&p);
        strncpy(fullname,p,CKMAXPATH);
    } else if (!devnull && fnrpath == PATH_ABS) { /* REC PATHNAMES ABSOLUTE */
        strncpy(fullname,name,CKMAXPATH);
    } else if (!devnull && isabsolute(name)) { /* RECEIVE PATHNAMES RELATIVE */
	ckmakmsg(fullname,CKMAXPATH,".",name,NULL,NULL);
    } else {                            /* Ditto */
        ckstrncpy(fullname,name,CKMAXPATH);
    }
    fullname[CKMAXPATH] = NUL;
    debug(F110,"nzrtol fullname",fullname,0);

#ifndef NOTRUNCATE
/*
  The maximum length for any segment of a filename is MAXNAMLEN, defined
  above.  On some platforms (at least QNX) if a segment exceeds this limit,
  the open fails with ENAMETOOLONG, so we must prevent it by truncating each
  overlong name segment to the maximum segment length before passing the
  name to open().  This must be done even when file names are literal, so as
  not to halt a file transfer unnecessarily.
*/
    {
        char buf[CKMAXPATH+1];          /* New temporary buffer on stack */
        char *p = fullname;             /* Source and  */
        char *s = buf;                  /* destination pointers */
        int i = 0, n = 0;
        debug(F101,"nzrtol sizing MAXNAMLEN","",MAXNAMLEN);
        while (*p && n < CKMAXPATH) {   /* Copy name to new buffer */
            if (++i > MAXNAMLEN) {      /* If this segment too long */
                while (*p && *p != '/') /* skip past the rest... */
                  p++;
                i = 0;                  /* and reset counter. */
            } else if (*p == '/') {     /* End of this segment. */
                i = 0;                  /* Reset counter. */
            }
            *s++ = *p++;                /* Copy this character. */
            n++;
        }
        *s = NUL;
        ckstrncpy(fullname,buf,CKMAXPATH); /* Copy back to original buffer. */
        debug(F111,"nzrtol sizing",fullname,n);
    }
#endif /* NOTRUNCATE */

    if (!fncnv || devnull) {            /* Not converting */
        ckstrncpy(name2,fullname,max);  /* We're done. */
        return;
    }
    name = fullname;                    /* Converting */

    p = name2;
    for (; *name != '\0' && n < maxnam; name++) {
        if (*name > SP) flag = 1;       /* Strip leading blanks and controls */
        if (flag == 0 && *name < '!')
          continue;
	if (fncnv > 0) {
	    if (*name == SP) {
		*p++ = '_';
		n++;
		continue;
	    }
	    if (isupper(*name))		/* Check for mixed case */
	      acase |= 1;
	    else if (islower(*name))
	      acase |= 2;
	}
        *p++ = *name;
        n++;
    }
    *p-- = '\0';                        /* Terminate */
    while (*p < '!' && p > name2)       /* Strip trailing blanks & controls */
      *p-- = '\0';

    if (*name2 == '\0') {               /* Nothing left? */
        ckstrncpy(name2,"NONAME",max);	/* do this... */
    } else if (acase == 1) {            /* All uppercase? */
        p = name2;                      /* So convert all letters to lower */
        while (*p) {
            if (isupper(*p))
              *p = tolower(*p);
            p++;
        }
    }
    debug(F110,"nzrtol new name",name2,0);
}


/*  Z S T R I P  --  Strip device & directory name from file specification */

/*  Strip pathname from filename "name", return pointer to result in name2 */

static char work[CKMAXPATH+1];

VOID
zstrip(name,name2) char *name, **name2; {
    char *cp, *pp;
    int n = 0;

    debug(F110,"zstrip before",name,0);
    if (!name) { *name2 = ""; return; }
    pp = work;
#ifdef DTILDE
    /* Strip leading tilde */
    if (*name == '~') name++;
    debug(F110,"zstrip after tilde-stripping",name,0);
#endif /* DTILDE */
    for (cp = name; *cp; cp++) {
        if (ISDIRSEP(*cp)) {
            pp = work;
            n = 0;
        } else {
            *pp++ = *cp;
            if (n++ >= CKMAXPATH)
              break;
        }
    }
    *pp = '\0';                         /* Terminate the string */
    *name2 = work;
    debug(F110,"zstrip after",*name2,0);
}

/*  Z L T O R  --  Local TO Remote */

VOID
zltor(name,name2) char *name, *name2; {
    nzltor(name,name2,1,0,CKMAXPATH);
}

/*  N Z L T O R  --  New Local TO Remote */

/*
  fncnv = 0 for no conversion, > 0 for regular conversion, < 0 for minimal.
*/
VOID
nzltor(name,name2,fncnv,fnspath,max)
    char *name, *name2; int fncnv, fnspath, max;
{ /* nzltor */
    char *cp, *pp;
#ifdef COMMENT
    int dc = 0;
#endif /* COMMENT */
    int n = 0;
    char *dotp = NULL;
    char *dirp = NULL;
    char fullname[CKMAXPATH+1];
    char *p;
    CHAR c;

#ifndef NOCSETS
    extern int fcharset, /* tcharset, */ language;
    int langsv;
    _PROTOTYP ( CHAR (*sxo), (CHAR) ) = NULL; /* Translation functions */
#ifdef CK_ANSIC
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])(CHAR);
#else
    extern CHAR (*xls[MAXTCSETS+1][MAXFCSETS+1])();
#endif /* CK_ANSIC */
    langsv = language;
    language = L_USASCII;
#ifdef COMMENT
    /* Proper translation of filenames must be done elsewhere */
    n = tcharset ? tcharset : TC_USASCII;
    sxo = xls[n][fcharset];
#else
    sxo = xls[TC_USASCII][fcharset];
#endif /* COMMENT */
#endif /* NOCSETS */

    debug(F110,"nzltor name",name,0);

    /* Handle pathname */

    fullname[0] = NUL;
    if (fnspath == PATH_OFF) {          /* PATHNAMES OFF */
        zstrip(name,&p);
        ckstrncpy(fullname,p,CKMAXPATH);
    } else {                            /* PATHNAMES RELATIVE or ABSOLUTE */
        char * p = name;
        while (1) {
            if (!strncmp(p,"../",3))
              p += 3;
            else if (!strncmp(p,"./",2))
              p += 2;
            else
              break;
        }
        if (fnspath == PATH_ABS) {      /* ABSOLUTE */
            zfnqfp(p,CKMAXPATH,fullname);
        } else {                        /* RELATIVE */
            ckstrncpy(fullname,p,CKMAXPATH);
        }
    }
    debug(F110,"nzltor fullname",fullname,0);

    if (!fncnv) {                       /* Not converting */
        ckstrncpy(name2,fullname,max);  /* We're done. */
#ifndef NOCSETS
        langsv = language;
#endif /* NOCSETS */
        return;
    }
    name = fullname;                    /* Converting */

#ifdef aegis
    char *namechars;
    int tilde = 0, bslash = 0;

    if ((namechars = getenv("NAMECHARS")) != NULL) {
        if (ckstrchr(namechars, '~' ) != NULL) tilde  = '~';
        if (ckstrchr(namechars, '\\') != NULL) bslash = '\\';
    } else {
        tilde = '~';
        bslash = '\\';
    }
#endif /* aegis */

    pp = work;                          /* Output buffer */
    for (cp = name, n = 0; *cp && n < max; cp++,n++) { /* Convert name chars */
        c = *cp;
#ifndef NOCSETS
        if (sxo) c = (*sxo)(c);         /* Convert to ASCII */
#endif /* NOCSETS */
        if (fncnv > 0 && islower(c))	/* Uppercase letters */
          *pp++ = toupper(c);           /* Change tilde to hyphen */
        else if (c == '~')
          *pp++ = '-';
        else if (fncnv > 0 && c == '#')	/* Change number sign to 'X' */
          *pp++ = 'X';
        else if (c == '*' || c == '?')  /* Change wildcard chars to 'X' */
          *pp++ = 'X';
        else if (c == ' ')              /* Change space to underscore */
          *pp++ = '_';
        else if (c < ' ')               /* Change controls to 'X' */
          *pp++ = 'X';
        else if (fncnv > 0 && c == '.') { /* Change dot to underscore */
            dotp = pp;                  /* Remember where we last did this */
            *pp++ = '_';
        } else {
            if (c == '/')
              dirp = pp;
            *pp++ = c;
        }
    }
    *pp = NUL;                          /* Tie it off. */
#ifdef COMMENT
    if (dotp) *dotp = '.';              /* Restore last dot (if any) */
#else
    if (dotp > dirp) *dotp = '.';       /* Restore last dot in file name */
#endif /* COMMENT */
    cp = name2;                         /* If nothing before dot, */
    if (*work == '.') *cp++ = 'X';      /* insert 'X' */
    ckstrncpy(cp,work,max);
#ifndef NOCSETS
    language = langsv;
#endif /* NOCSETS */
    debug(F110,"nzltor name2",name2,0);
}


/*  Z C H D I R  --  Change directory  */
/*
  Call with:
    dirnam = pointer to name of directory to change to,
      which may be "" or NULL to indicate user's home directory.
  Returns:
    0 on failure
    1 on success
*/
int
zchdir(dirnam) char *dirnam; {
    char *hd, *sp;
#ifdef IKSDB
    _PROTOTYP (int slotdir,(char *,char *));
#endif /* IKSDB */
#ifndef NOSPL
    extern struct mtab *mactab;             /* Main macro table */
    extern int nmac;                        /* Number of macros */
#endif /* NOSPL */

    debug(F110,"zchdir",dirnam,0);
    if (!dirnam) dirnam = "";
    if (!*dirnam)                       /* If argument is null or empty, */
      dirnam = zhome();                 /* use user's home directory. */
    sp = dirnam;
    debug(F110,"zchdir 2",dirnam,0);

#ifdef DTILDE
    hd = tilde_expand(dirnam);          /* Attempt to expand tilde */
    if (!hd) hd = "";
    if (*hd == '\0') hd = dirnam;       /* in directory name. */
#else
    hd = dirnam;
#endif /* DTILDE */
    debug(F110,"zchdir 3",hd,0);

#ifdef CKROOT
    debug(F111,"zchdir setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(hd)) {
	debug(F110,"zchdir setroot violation",hd,0);
	return(0);
    }
#endif /* CKROOT */

#ifdef pdp11
    /* Just to save some space */
    return((chdir(hd) == 0) ? 1 : 0);
#else
    if (chdir(hd) == 0) {                       /* Try to cd */
#ifdef IKSDB
#ifdef CK_LOGIN
        if (inserver && ikdbopen)
          slotdir(isguest ? anonroot : "", zgtdir());
#endif /* CK_LOGIN */
#endif /* IKSDB */

#ifndef NOSPL
        if (nmac) {			/* Any macros defined? */
            int k;			/* Yes */
            static int on_cd = 0;
            if (!on_cd) {
                on_cd = 1;
                k = mlook(mactab,"on_cd",nmac);   /* Look this up */
                if (k >= 0) {                     /* If found, */
                    if (dodo(k,zgtdir(),0) > -1)  /* set it up, */
		      parser(1);                  /* and execute it */
                }
                on_cd = 0;
            }
        }
#endif /* NOSPL */
        return(1);
    }
    return(0);
#endif /* pdp11 */
}

int
#ifdef CK_ANSIC
zchkpid(unsigned long xpid)
#else
zchkpid(xpid) unsigned long xpid;
#endif /* CK_ANSIC */
{
    return((kill((PID_T)xpid,0) < 0) ? 0 : 1);
}


/*  Z H O M E  --  Return pointer to user's home directory  */

static char * zhomdir = NULL;

char *
zhome() {
    char * home;

#ifdef CKROOT
    if (ckrootset)
      return((char *)ckroot);
#endif /* CKROOT */

#ifdef Plan9
    home = getenv("home");
#else
    home = getenv("HOME");
#endif /* Plan9 */
    makestr(&zhomdir,home);
    return(home ? zhomdir : ".");
}

/*  Z G T D I R  --  Returns a pointer to the current directory  */

/*
  The "preferred" interface for getting the current directory in modern UNIX
  is getcwd() [POSIX 1003.1 5.2.2].  However, on certain platforms (such as
  SunOS), it is implemented by forking a shell, feeding it the pwd command,
  and returning the result, which is not only inefficient but also can result
  in stray messages to the terminal.  In such cases -- as well as when
  getcwd() is not available at all -- getwd() can be used instead by defining
  USE_GETWD.  However, note that getwd() provides no buffer-length argument
  and therefore no safeguard against memory leaks.
*/
#ifndef USE_GETWD
#ifdef BSD42
#define USE_GETWD
#else
#ifdef SUNOS4
#define USE_GETWD
#endif /* SUNOS4 */
#endif /* BSD42 */
#endif /* USE_GETWD */

#ifdef pdp11
#define CWDBL 80                        /* Save every byte we can... */
#else
#define CWDBL CKMAXPATH
#endif /* pdp11 */
static char cwdbuf[CWDBL+2];
/*
  NOTE: The getcwd() prototypes are commented out on purpose.  If you get
  compile-time warnings, search through your system's header files to see
  which one has the needed prototype, and #include it.  Usually it is
  <unistd.h>.  See the section for including <unistd.h> in ckcdeb.h and
  make any needed adjustments there (and report them).
*/
char *
zgtdir() {
    char * buf = cwdbuf;
    char * s;

#ifdef USE_GETWD
    extern char *getwd();
    s = getwd(buf);
    debug(F110,"zgtdir BSD4 getwd()",s,0);
    if (!s) s = "./";
    return(s);
#else
#ifdef BSD44
#ifdef DCLGETCWD
_PROTOTYP( char * getcwd, (char *, SIZE_T) );
#endif /* DCLGETCWD */
    debug(F101,"zgtdir BSD44 CWDBL","",CWDBL);
    s = getcwd(buf,CWDBL);
    if (!s) s = "./";
    return(s);
#else
#ifdef MINIX2
#ifdef DCLGETCWD
    _PROTOTYP( char * getcwd, (char *, SIZE_T) );
#endif /* DCLGETCWD */
    debug(F101,"zgtdir MINIX2 CWDBL","",CWDBL);
    s = getcwd(buf,CWDBL);
    if (!s) s = "./";
    return(s);
#else
#ifdef SVORPOSIX
#ifdef COMMENT
/* This non-ANSI prototype can be fatal at runtime! (e.g. in SCO3.2v5.0.5). */
/* Anyway it's already prototyped in some header file that we have included. */
    extern char *getcwd();
#else
#ifdef DCLGETCWD
    _PROTOTYP( char * getcwd, (char *, SIZE_T) );
#endif /* DCLGETCWD */
#endif /* COMMENT */
    debug(F101,"zgtdir SVORPOSIX CWDBL","",CWDBL);
    s = getcwd(buf,CWDBL);
    if (!s) s = "./";
    return(s);
#else
#ifdef COHERENT
#ifdef _I386
#ifdef DCLGETCWD
    extern char *getcwd();
#endif /* DCLGETCWD */
    debug(F101,"zgtdir COHERENT _I386 CWDBL","",CWDBL);
    s = getcwd(buf,CWDBL);
    if (!s) s = "./";
    return(s);
#else
    extern char *getwd();
    debug(F101,"zgtdir COHERENT CWDBL","",CWDBL);
    s = getwd(buf);
    if (!s) s = "./";
    return(s);
#endif /* _I386 */
#else
#ifdef SUNOS4
    debug(F101,"zgtdir SUNOS CWDBL","",CWDBL);
    s = getcwd(buf,CWDBL);
    if (!s) s = "./";
    return(s);
#else
    return("./");
#endif /* SUNOS4 */
#endif /* COHERENT */
#endif /* SYSVORPOSIX */
#endif /* MINIX2 */
#endif /* BSD44 */
#endif /* USE_GETWD */
}

/*  Z X C M D -- Run a system command so its output can be read like a file */

#ifndef NOPUSH
int
zxcmd(filnum,comand) int filnum; char *comand; {
    int out;
    int pipes[2];
    extern int kactive;                 /* From ckcpro.w and ckcmai.c */

    if (nopush) {
        debug(F100,"zxcmd fails: nopush","",0);
        return(-1);
    }
    debug(F111,"zxcmd",comand,filnum);
    if (chkfn(filnum) < 0) return(-1);  /* Need a valid Kermit file number. */
    if (filnum == ZSTDIO || filnum == ZCTERM) /* But not one of these. */
      return(0);

    out = (filnum == ZIFILE || filnum == ZRFILE) ? 0 : 1 ;
    debug(F101,"zxcmd out",comand,out);

/* Output to a command */

    if (out) {                          /* Need popen() to do this. */
	ckstrncpy(fullname,"(pipe)",CKMAXPATH);
#ifdef NOPOPEN
        return(0);                      /* no popen(), fail. */
#else
/* Use popen() to run the command. */

#ifdef _POSIX_SOURCE
/* Strictly speaking, popen() is not available in POSIX.1 */
#define DCLPOPEN
#endif /* _POSIX_SOURCE */

	debug(F110,"zxcmd out",comand,0);

        if (priv_chk()) {
	    debug(F100,"zxcmd priv_chk failed","",0);
            return(0);
	}	
	errno = 0;
        fp[filnum] = popen(comand,"w");
	debug(F111,"zxcmd popen",fp[filnum] ? "OK" : "Failed", errno);
	if (fp[filnum] == NULL)
	  return(0);
#ifdef COMMENT
/* I wonder what this is all about... */
	close(pipes[0]);		/* Don't need the input side */
	fp[filnum] = fdopen(pipes[1],"w"); /* Open output stream. */
	fp[ZSYSFN] = fp[filnum];           /* Remember. */
#endif /* COMMENT */
	ispipe[filnum] = 1;
	zoutcnt = 0;			/* (PWP) reset input buffer */
	zoutptr = zoutbuffer;
	return(1);
#endif /* NOPOPEN */
    }

/* Input from a command */

#ifdef SNI541
    /* SINIX-L 5.41 does not like fdopen() */
    return(0);
#else
    if (pipe(pipes) != 0) {
        debug(F100,"zxcmd pipe failure","",0);
        return(0);                      /* can't make pipe, fail */
    }

/* Create a fork in which to run the named process */

    if ((
#ifdef aegis
         pid = vfork()                  /* child */
#else
         pid = fork()                   /* child */
#endif /* aegis */
         ) == 0) {

/* We're in the fork. */

        char *shpath, *shname, *shptr;  /* Find user's preferred shell */
#ifndef aegis
        struct passwd *p;
        char *defshell;
#ifdef HPUX10                           /* Default shell */
        defshell = "/usr/bin/sh";
#else
#ifdef Plan9
        defshell = "/bin/rc";
#else
        defshell = "/bin/sh";
#endif /* Plan9 */
#endif /* HPUX10 */
#endif /* aegis */
        if (priv_can()) exit(1);        /* Turn off any privileges! */
        debug(F101,"zxcmd pid","",pid);
        close(pipes[0]);                /* close input side of pipe */
        close(0);                       /* close stdin */
        if (open("/dev/null",0) < 0) return(0); /* replace input by null */
#ifndef OXOS
#ifndef SVORPOSIX
        dup2(pipes[1],1);               /* BSD: replace stdout & stderr */
        dup2(pipes[1],2);               /* by the pipe */
#else
        close(1);                       /* AT&T: close stdout */
        if (dup(pipes[1]) != 1)         /* Send stdout to the pipe */
          return(0);
        close(2);                       /* Send stderr to the pipe */
        if (dup(pipes[1]) != 2)
          return(0);
#endif /* SVORPOSIX */
#else /* OXOS */
        dup2(pipes[1],1);
        dup2(pipes[1],2);
#endif /* OXOS */
        close(pipes[1]);                /* Don't need this any more. */

#ifdef aegis
        if ((shpath = getenv("SERVERSHELL")) == NULL)
          shpath = "/bin/sh";
#else
        shpath = getenv("SHELL");       /* What shell? */
        if (shpath == NULL) {
            p = getpwuid( real_uid() ); /* Get login data */
            /* debug(F111,"zxcmd shpath","getpwuid()",p); */
            if (p == (struct passwd *)NULL || !*(p->pw_shell))
              shpath = defshell;
            else shpath = p->pw_shell;
        }
#endif /* aegis */
        shptr = shname = shpath;
        while (*shptr != '\0')
          if (*shptr++ == '/')
            shname = shptr;
        debug(F110,shpath,shname,0);
	restorsigs();			/* Restore ignored signals */
        execl(shpath,shname,"-c",comand,(char *)NULL); /* Execute the cmd */
        exit(0);                        /* just punt if it failed. */
    } else if (pid == (PID_T) -1) {
        debug(F100,"zxcmd fork failure","",0);
        return(0);
    }
    debug(F101,"zxcmd pid","",pid);
    close(pipes[1]);                    /* Don't need the output side */
    ispipe[filnum] = 1;                 /* Remember it's a pipe */
    fp[filnum] = fdopen(pipes[0],"r");	/* Open a stream for input. */

#ifdef DONDELAY
#ifdef SELECT
    if (filnum == ZIFILE && kactive) {  /* Make pipe reads nonblocking */
        int flags, x;
        if ((flags = fcntl(fileno(fp[filnum]),F_GETFL,0)) > -1) {
            debug(F101,"zxcmd fcntl 1 pipe flags","",flags);
            x = fcntl(fileno(fp[filnum]),F_SETFL, flags |
#ifdef QNX
                  O_NONBLOCK
#else
                  O_NDELAY
#endif /* QNX */
                  );
            debug(F101,"zxcmd fcntl 2 result","",x);
        }
    }
#endif /* SELECT */
#endif /* DONDELAY */
#endif /* SNI541 */
    fp[ZSYSFN] = fp[filnum];            /* Remember. */
    zincnt = 0;                         /* (PWP) reset input buffer */
    zinptr = zinbuffer;
    fullname[0] = '\0';
    return(1);
} /* zxcmd */

/*  Z C L O S F  - wait for the child fork to terminate and close the pipe. */

/*  Used internally by zclose - returns -1 on failure, 1 on success. */

int
zclosf(filnum) int filnum; {
    int wstat, out;
    int statusp;

    debug(F101,"zclosf filnum","",filnum);
    out = (filnum == ZIFILE || filnum == ZRFILE) ? 0 : 1 ;
    debug(F101,"zclosf out","",out);

#ifndef NOPOPEN
    if (ispipe[filnum]
        /* In UNIX we use popen() only for output files */
        && out
        ) {
        int x;
        x = pclose(fp[filnum]);
        pexitstat = x >> 8;
        debug(F101,"zclosf pclose","",x);
        debug(F101,"zclosf pexitstat","",pexitstat);
        fp[filnum] = fp[ZSYSFN] = NULL;
        ispipe[filnum] = 0;
        return((x != 0) ? -1 : 1);
    }
#endif /* NOPOPEN */
    /* debug(F101,"zclosf fp[filnum]","", fp[filnum]); */
    /* debug(F101,"zclosf fp[ZSYSFN]","", fp[ZSYSFN]); */

    if (pid != (PID_T) 0) {
        debug(F101,"zclosf killing pid","",pid);
#ifdef Plan9
        kill(pid, SIGKILL);
#else
        kill(pid,9);
#endif /* Plan9 */

#ifndef CK_CHILD
/*
  This is the original code (before 20 April 1997) and has proven totally
  portable.  But it does not give us the process's return code.
*/
        while ((wstat = wait((WAIT_T *)0)) != pid && wstat != -1) ;
#else
/* Here we try to get the return code.  Let's hope this is portable too. */
        while ((wstat = wait(&statusp)) != pid && wstat != -1) ;
        pexitstat = (statusp & 0xff) ? statusp : statusp >> 8;
        debug(F101,"zclosf wait statusp","",statusp);
        debug(F101,"zclosf wait pexitstat","",pexitstat);
#endif /* CK_CHILD */
        pid = 0;
    }
    fclose(fp[filnum]);
    fp[filnum] = fp[ZSYSFN] = NULL;

    ispipe[filnum] = 0;
    /* debug(F101,"zclosf fp[filnum]","",fp[filnum]); */
#ifdef CK_CHILD
    return(pexitstat == 0 ? 1 : -1);
#else
    return(1);
#endif /* CK_CHILD */
}

#else  /* NOPUSH */

int
zxcmd(filnum,comand) int filnum; char *comand; {
    return(0);
}
int
zclosf(filnum) int filnum; {
    return(EOF);
}
#endif /* NOPUSH */


/*  Z X P A N D  --  Expand a wildcard string into an array of strings  */
/*
  As of C-Kermit 7.0, this API is obsolete, replaced by nzxpand(), and this
  function is only used internally.  See nzxpand() below.

  Returns the number of files that match fnarg, with data structures set up
  so that first file (if any) will be returned by the next znext() call.

  Depends on external variable wildxpand: 0 means we expand wildcards
  internally, nonzero means we call the shell to do it.
  
  AND in C-Kermit 8.0.212 and later, on extern wildena: 1 means wildcards
  are enabled, 0 means disabled, the characters are taken literally.
*/
static int xdironly = 0;
static int xfilonly = 0;
static int xmatchdot = 0;
static int xrecursive = 0;
static int xnobackup = 0;
static int xnolinks = 0;

static char *freeptr = NULL, **resptr = NULL; /* Copies of caller's args */
static int remlen;                      /* Remaining space in caller's array */
static int numfnd = 0;			/* Number of matches found */

#define MINSPACE 1024

static int
initspace(resarry,len) char * resarry[]; int len; {
#ifdef DYNAMIC
    if (len < MINSPACE) len = MINSPACE;
    if (!sspace) {                      /* Need to allocate string space? */
        while (len >= MINSPACE) {
            if ((sspace = malloc(len+2))) { /* Got it. */
                debug(F101,"fgen string space","",len);
                break;
            }
            len = (len / 2) + (len / 4); /* Didn't, reduce by 3/4 */
        }
        if (len <= MINSPACE) {		/* Did we get it? */
            fprintf(stderr,"fgen can't malloc string space\n");
            return(-1);
        }
	ssplen = len;
    }
#endif /* DYNAMIC */

    freeptr = sspace;                   /* This is where matches are copied. */
    resptr = resarry;			/* Static copies of these so */
    remlen = len;                       /* recursive calls can alter them. */
    debug(F101,"initspace ssplen","",ssplen);
    return(0);
}

/*
  Z S E T F I L  --  Query or change the size of file list buffers.

  fc = 1: Change current string space to n, return new size.
  fc = 2: Return current string space size.
  fc = 3: Change current maxnames to n, return new maxnames.
  fc = 4: Return current maxnames.
  Returns < 0 on error.
*/
int
zsetfil(n, fc) int n, fc; {
#ifdef DYNAMIC
    switch (fc) {
      case 1:				/* Stringspace */
	if (sspace) {
	    free(sspace);
	    sspace = NULL;
	}
	if (initspace(mtchs,n) < 0)
	  return(-1);
      case 2:				/* Fall thru deliberately */
	return(ssplen);
      case 3:				/* Listsize */
	if (mtchs) {
	    free((char *)mtchs);
	    mtchs = NULL;
	}
	mtchs = (char **)malloc(n * sizeof(char *));
	if (!mtchs)
	  return(-1);
	maxnames = n;
      case 4:				/* Fall thru deliberately */
	return(maxnames);
    }
#endif /* DYNAMIC */
    return(-1);
}



#ifndef NONZXPAND
#ifndef pdp11
static
#endif /* pdp11 */
#endif /* NONZXPAND */
int
zxpand(fnarg) char *fnarg; {
    extern int diractive;
    char fnbuf[CKMAXPATH+8], * fn, * p;

#ifdef DTILDE                           /* Built with tilde-expansion? */
    char *tnam;
#endif /* DTILDE */
    int x;
    int haveonedir = 0;

    if (!fnarg) {                       /* If no argument provided */
	nxpand = fcount = 0;
	return(0);			/* Return zero files found */
    }
    debug(F110,"zxpand entry",fnarg,0);
    debug(F101,"zxpand xdironly","",xdironly);
    debug(F101,"zxpand xfilonly","",xfilonly);

    if (!*fnarg) {			/* If no argument provided */
	nxpand = fcount = 0;
	return(0);			/* Return zero files found */
    }

#ifdef CKROOT
    debug(F111,"zxpand setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(fnarg)) {
	debug(F110,"zxpand setroot violation",fnarg,0);
	nxpand = fcount = 0;
	return(0);
    }
#endif /* CKROOT */

#ifdef COMMENT
/*
  This would have been perfect, except it makes us return fully qualified
  pathnames for all files.
*/
    zfnqfp(fnarg,CKMAXPATH,fnbuf);
    debug(F110,"zxpand zfnqfp",fnbuf,0);
    s = zgtdir();
    debug(F110,"zxpand zgtdir",s,0);
    p = fnbuf;
    while (*p && *s)                    /* Make it relative */
      if (*s++ != *p++)
        break;
    fn = (*s) ? fnbuf : p;
    debug(F110,"zxpand fn 0",fn,0);
    if (!*fn) {
        fn = fnbuf;
        fnbuf[0] = '*';
        fnbuf[1] = '\0';
    }
    debug(F110,"zxpand fn 0.5",fn,0);
#else
#ifdef DTILDE                           /* Built with tilde-expansion? */
    if (*fnarg == '~') {                /* Starts with tilde? */
        tnam = tilde_expand(fnarg);     /* Try to expand it. */
        ckstrncpy(fnbuf,tnam,CKMAXPATH);
    } else
#endif /* DTILDE */
      ckstrncpy(fnbuf,fnarg,CKMAXPATH);
    fn = fnbuf;                         /* Point to what we'll work with */
#endif /* COMMENT */
    debug(F110,"zxpand fn 1",fn,0);

    if (!*fn)                           /* But make sure something is there */
      return(0);

    p = fn + (int)strlen(fn) - 1;
    if (*p == '/') {                    /* If last char = / it must be a dir */
	if (!xfilonly && !iswild(p)) haveonedir++;
        ckstrncat(fn, "*", CKMAXPATH+8); /* so append '*' */
    } else if (p > fn) {                /* If ends in "/." */
        if (*(p-1) == '/' && *p == '.') /* change '.' to '*' */
          *p = '*';
    } else if (p == fn) {               /* If it's '.' alone */
        if (*p == '.')                  /* change '.' to '*' */
          *p = '*';
    }
    debug(F110,"zxpand fn 2",fn,0);
    x = isdir(fn);                      /* Is it a directory? */
    debug(F111,"zxpand isdir 1",fn,x);
    if (x) {                            /* If so, make it into a wildcard */
	if (!xfilonly && !iswild(p))
	  haveonedir++;
        if ((x = strlen(fn)) > 0) {
            if (!ISDIRSEP(fn[x-1]))
              fn[x++] = DIRSEP;
            fn[x++] = '*';
            fn[x] = '\0';
        }
    }
    debug(F111,"zxpand fn 3 haveonedir",fn,haveonedir);
/*
  The following allows us to parse a single directory name without opening
  the directory and looking at its contents.  The diractive flag is a horrible
  hack (especially since DIR /NORECURSIVE turns it off), but otherwise we'd
  have to change the API.
*/
    debug(F111,"zxpand fn 3 diractive",fn,diractive);
    if (!diractive && haveonedir) {
	fcount = 0;
	if (!mtchs) {
	    mtchs = (char **)malloc(maxnames * sizeof(*mtchs));
	    if (!mtchs)
	      return(nxpand = fcount);
	}
	fcount = 1;
	debug(F110,"zxpand haveonedir A1",fnarg,0);
	initspace(mtchs,ssplen);
	addresult(fnarg,1);
	if (numfnd < 0) return(-1);
	mtchptr = mtchs;		/* Save pointer for next. */
	debug(F111,"zxpand haveonedir A2",*mtchptr,numfnd);
	return(nxpand = fcount);
    }

#ifndef NOPUSH
    if (!nopush && wildxpand)           /* Who is expanding wildcards? */
      fcount = (mtchs == NULL &&        /* Shell */
                (mtchs = (char **)malloc(maxnames * sizeof(*mtchs))) == NULL)
        ? 0
          :  shxpand(fn,mtchs,maxnames);
    else
#endif /* NOPUSH */
      fcount = (mtchs == NULL &&        /* Kermit */
                (mtchs = (char **)malloc(maxnames * sizeof(*mtchs))) == NULL)
        ? 0
          : fgen(fn,mtchs,maxnames);      /* Look up the file. */

    if (fcount == 0 && haveonedir) {
	fcount = 1;
	debug(F110,"zxpand haveonedir B",fnarg,0);
	addresult(fnarg,1);
	if (numfnd < 0) return(-1);
    }
    mtchptr = mtchs;                    /* Save pointer for next. */
    nxpand = fcount;

#ifdef DEBUG
    if (deblog) {
        if (fcount > 1)
          debug(F111,"zxpand ok",mtchs[0],fcount);
        else
          debug(F101,"zxpand fcount","",fcount);
    }
#endif /* DEBUG */
    return(fcount);
}

#ifndef NONZXPAND
/*  N Z X P A N D  --  Expand a file list, with options.  */
/*
  Call with:
   s = pointer to filename or pattern.
   flags = option bits:

     flags & ZX_FILONLY   Match regular files
     flags & ZX_DIRONLY   Match directories
     flags & ZX_RECURSE   Descend through directory tree
     flags & ZX_MATCHDOT  Match "dot files"
     flags & ZX_NOBACKUP  Don't match "backup files"
     flags & ZX_NOLINKS   Don't follow symlinks.

   Returns the number of files that match s, with data structures set up
   so that first file (if any) will be returned by the next znext() call.
*/
int
nzxpand(s,flags) char * s; int flags; {
    char * p;
    int x;

    debug(F111,"nzxpand",s,flags);
    x = flags & (ZX_DIRONLY|ZX_FILONLY);
    xdironly = (x == ZX_DIRONLY);
    xfilonly = (x == ZX_FILONLY);
    if (xdironly && xfilonly) {
        xdironly = 0;
        xfilonly = 0;
    }
    xmatchdot  = (flags & ZX_MATCHDOT);
    debug(F111,"nzxpand xmatchdot 1",s,xmatchdot);
    /* If xmatchdot not set by caller but pattern implies it, set it anyway */
    if (!xmatchdot && ((p = ckstrchr(s,'.')))) {
	if (p == s && p[1] != '/') {
	    xmatchdot = 1;
	    debug(F111,"nzxpand xmatchdot 2",s,xmatchdot);
	} else if (p > s) {
	    xmatchdot = (*(p-1) == ',') || (*(p-1) == '{') || (*(p-1) == '/');
	    debug(F111,"nzxpand xmatchdot 3",s,xmatchdot);
	}
    }
    xrecursive = (flags & ZX_RECURSE);
    xnobackup  = (flags & ZX_NOBACKUP);
    xnolinks   = (flags & ZX_NOLINKS);

#ifdef DEBUG
    if (deblog) {
	debug(F101,"nzxpand xdironly","",xdironly);
	debug(F101,"nzxpand xfilonly","",xfilonly);
	debug(F101,"nzxpand xmatchdot","",xmatchdot);
	debug(F101,"nzxpand xrecursive","",xrecursive);
	debug(F101,"nzxpand xnobackup","",xnobackup);
	debug(F101,"nzxpand xnolinks","",xnolinks);
    }
#endif /* DEBUG */

    x = zxpand(s);
    if (x > 1)
      sh_sort(mtchs,NULL,x,0,0,1);	/* Alphabetize the list */
    xdironly = 0;
    xfilonly = 0;
    xmatchdot = 0;
    xrecursive = 0;
    xnobackup = 0;
    xnolinks = 0;
    return(x);
}
#endif /* NONZXPAND */

#ifndef NOZXREWIND
/*  Z X R E W I N D  --  Rewinds the zxpand() list */

int
zxrewind() {
    /* if (!mtchs) return(-1); */
    fcount = nxpand;
    mtchptr = mtchs;
    return(nxpand);
}
#endif /* NOZXREWIND */

/*  Z N E X T  --  Get name of next file from list created by zxpand(). */
/*
  Returns >0 if there's another file, with its name copied into the arg string,
  or 0 if no more files in list.
*/
int
znext(fn) char *fn; {
    if (fcount-- > 0) {
        ckstrncpy(fn,*mtchptr++,CKMAXPATH);
    } else {
        fn[0] = '\0';
    }
#ifndef COMMENT
    debug(F111,"znext",fn,fcount+1);
    return(fcount+1);
#else
    debug(F111,"znext",fn,fcount);      /* Return 0 if no filename to return */
    return(fcount);
#endif /* COMMENT */
}

/*  Z C H K S P A  --  Check if there is enough space to store the file  */

/*
 Call with file specification f, size n in bytes.
 Returns -1 on error, 0 if not enough space, 1 if enough space.
*/
/*ARGSUSED*/
int
#ifdef CK_ANSIC
zchkspa(char *f, CK_OFF_T n)
#else
zchkspa(f,n) char *f; CK_OFF_T n;
#endif /* CK_ANSIC */
/* zchkspa() */ {
    /* In UNIX there is no good (and portable) way. */
    return(1);                          /* Always say OK. */
}

#ifdef COMMENT				/* (not used) */

/*  I S B A C K U P  --  Tells if given file has a backup suffix  */
/*
   Returns:
   -1: Invalid argument
    0: File does not have a backup suffix
   >0: Backup suffix number
*/
int
isbackup(fn) char * fn; {		/* Get backup suffix number */
    int i, j, k, x, state, flag;

    if (!fn)				/* Watch out for null pointers. */
      return(-1);
    if (!*fn)				/* And empty names. */
      return(-1);

    flag = state = 0;
    for (i = (int)strlen(fn) - 1; (!flag && (i > 0)); i--) {
	switch (state) {
	  case 0:			/* State 0 - final char */
	    if (fn[i] == '~')		/* Is tilde */
	      state = 1;		/* Switch to next state */
	    else			/* Otherwise */
	      flag = 1;			/* Quit - no backup suffix. */
	    break;
	  case 1:			/* State 1 - digits */
	    if (fn[i] == '~'  && fn[i-1] == '.') { /* Have suffix */
		return(atoi(&fn[i+1]));
	    } else if (fn[i] >= '0' && fn[i] <= '9') { /* In number part */
		continue;		/* Keep going */
	    } else {			/* Something else */
		flag = 1;		/* Not a backup suffix - quit. */
	    }
	    break;
	}
    }
    return(0);
}
#endif /* COMMENT */


/*  Z N E W N  --  Make a new name for the given file  */

/*
  Given the name, fn, of a file that already exists, this function builds a
  new name of the form "<oldname>.~<n>~", where <oldname> is argument name
  (fn), and <n> is a version number, one higher than any existing version
  number for that file, up to 99999.  This format is consistent with that used
  by GNU EMACS.  If the constructed name is too long for the system's maximum,
  enough characters are truncated from the end of <fn> to allow the version
  number to fit.  If no free version numbers exist between 1 and 99999, a
  version number of "xxxx" is used.  Returns a pointer to the new name in
  argument s.
*/
#ifdef pdp11
#define ZNEWNBL 63                      /* Name buffer length */
#define ZNEWNMD 3                       /* Max digits for version number */
#else
#define ZNEWNBL CKMAXPATH
#define ZNEWNMD 4
#endif /* pdp11 */

#define MAXBUDIGITS 5

static char znewbuf[ZNEWNBL+12];

VOID
znewn(fn,s) char *fn, **s; {
    char * buf;				/* Pointer to buffer for new name */
    char * xp, * namepart = NULL;       /* Pointer to filename part */
    struct zfnfp * fnfp;                /* znfqfp() result struct pointer */
    int d = 0, t, fnlen, buflen;
    int n, i, k, flag, state;
    int max = MAXNAMLEN;                /* Maximum name length */
    char * dname = NULL;

    buf = znewbuf;
    *s = NULL;                          /* Initialize return value */
    if (!fn) fn = "";                   /* Check filename argument */
    i = strlen(fn);

/* If incoming file already has a backup suffix, remove it. */
/* Then we'll tack a new on later, which will be the highest for this file. */

    if (i <= max && i > 0 && fn[i-1] == '~') {
	char * p;
	i--;
	debug(F111,"znewn suffix removal",fn,i);
	if ((dname = (char *)malloc(i+1))) {
	    ckstrncpy(dname,fn,i+1);
	    p = dname;
	    for (flag = state = 0; (!flag && (i > 0)); i--) {
		switch (state) {
		  case 0:		/* State 0 - final char */
		    if (p[i] == '~')	/* Is tilde */
		      state = 1;	/* Switch to next state */
		    else		/* Otherwise */
		      flag = 1;		/* Quit - no backup suffix. */
		    break;
		  case 1:		/* State 1 - digits */
		    if (p[i] == '~'  && p[i-1] == '.') { /* Have suffix */
			p[i-1] = NUL;	/* Trim it */
			fn = dname;
			debug(F111,"znewn suffix removal 2",fn,i);
			flag = 1;	/* done */
		    } else if (p[i] >= '0' && p[i] <= '9') { /* Number part */
			continue;	/* Keep going */
		    } else {		/* Something else */
			flag = 1;	/* Not a backup suffix - quit. */
		    }
		    break;
		}
	    }
	}
    }
    if ((fnlen = strlen(fn)) < 1) {	/* Get length */
	if (dname) free(dname);
	return;
    }
    debug(F111,"znewn",fn,fnlen);

    debug(F101,"znewn max 1","",max);
    if (max < 14) max = 14;             /* Make max reasonable for any UNIX */
    if (max > ZNEWNBL) max = ZNEWNBL;
    debug(F101,"znewn max 2","",max);

    if ((fnfp = zfnqfp(fn, ZNEWNBL, buf))) { /* Get fully qualified name */
        namepart = fnfp->fname;         /* Isolate the filename */
        k = strlen(fn);                 /* Length of name part */
        debug(F111,"znewn namepart",namepart,k);
    } else {
	if (dname) free(dname);
	return;
    }
    buflen = fnfp->len;                 /* Length of fully qualified name */
    debug(F111,"znewn len",buf,buflen);

    if (k + MAXBUDIGITS + 3 < max) {    /* Backup name fits - no overflow */
	/* Make pattern for backup names */
        ckstrncpy(buf+buflen,".~*~",ZNEWNBL+12-buflen);
        n = nzxpand(buf,ZX_FILONLY);    /* Expand the pattern */
        debug(F111,"znewn A matches",buf,n);
        while (n-- > 0) {               /* Find any existing name.~n~ files */
            xp = *mtchptr++;            /* Point at matching name */
            t = atoi(xp+buflen+2);      /* Get number */
            if (t > d) d = t;           /* Save d = highest version number */
        }
        sprintf(buf+buflen,".~%d~",d+1); /* Yes, make "name.~<d+1>~" */
        debug(F110,"znewn A newname",buf,0);
    } else {                            /* Backup name would be too long */
        int xlen;                       /* So we have to eat back into it */
        int delta;
        char buf2[ZNEWNBL+12];

        delta = max - k;
        debug(F101,"znewn B delta","",delta);

        for (i = MAXBUDIGITS; i > 0; i--) { /* In this case the format of */
            ckstrncpy(buf2,buf,ZNEWNBL+12); /* the backup name depends on */
            xlen = buflen - i - 3 + delta;  /* how many digits are in the */
            ckstrncpy(buf2+xlen,".~*~",ZNEWNBL+12-xlen); /* backup number */
            n = nzxpand(buf2,ZX_FILONLY);
            debug(F111,"znewn B matches",buf2,n);
            if (n > 0)
              break;
        }
        while (n-- > 0) {               /* Find any existing name.~n~ files */
            xp = *mtchptr++;            /* Point at matching name */
            t = atoi(xp+xlen+2);        /* Get number */
            if (t > d) d = t;           /* Save d = highest version number */
        }
        if (d > 0)                      /* If the odometer turned over... */
          if ((d % 10) == 9)            /* back up one space. */
            xlen--;
        sprintf(buf2+xlen,".~%d~",d+1); /* This just fits */
        ckstrncpy(buf,buf2,ZNEWNBL+12);	/* (we could be more clever here...) */
        debug(F110,"znewn B new name",buf,0);
    }
    *s = buf;                           /* Point to new name */
    ck_znewn = d+1;                     /* Also make it available globally */
    if (dname) free(dname);
    return;
}

/*  Z R E N A M E  --  Rename a file  */
/*
   Call with old and new names.
   If new name is the name of a directory, the 'old' file is moved to
   that directory.
   Returns 0 on success, -1 on failure.
*/
int
zrename(old,new) char *old, *new; {
    char *p, *s;
    int x;

    if (!old) old = "";
    if (!new) new = "";
    debug(F110,"zrename old",old,0);
    debug(F110,"zrename new",new,0);
    if (!*old) return(-1);
    if (!*new) return(-1);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest)
      return(-1);
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef CKROOT
    debug(F111,"zrename setroot",ckroot,ckrootset);
    if (ckrootset) {
	if (!zinroot(old)) {
	    debug(F110,"zrename old: setroot violation",old,0);
	    return(-1);
	}
	if (!zinroot(new)) {
	    debug(F110,"zrename new: setroot violation",new,0);
	    return(-1);
	}
    }
#endif /* CKROOT */

    p = NULL;
    s = new;

    if (isdir(new)) {
        char *q = NULL;
        x = strlen(new);
        if (!(p = malloc(strlen(new) + strlen(old) + 2)))
          return(-1);
        strcpy(p,new);                  /* (safe) Directory part */
        if (!ISDIRSEP(*(new+x-1)))      /* Separator, if needed */
          strcat(p,"/");		/* (safe) */
        zstrip(old,&q);                 /* Strip path part from old name */
        strcat(p,q);                    /* cat to new directory (safe) */
        s = p;
        debug(F110,"zrename dir",s,0);
    }
#ifdef DEBUG
    else debug(F110,"zrename no dir",s,0);
#endif /* DEBUG */

#ifdef IKSD
    if (inserver && (!ENABLED(en_del))) {
	if (zchki(s) > -1)		/* Destination file exists? */
	  return(-1);
    }
#endif /* IKSD */

    x = -1;                             /* Return code. */
#ifdef RENAME
/* Atomic, preferred, uses a single system call, rename(), if available. */
    x = rename(old,s);
    debug(F111,"zrename rename()",old,x);
    if (x) x = -1;
#endif /* RENAME */

    /* If rename() failed or not available try link()/unlink() */

    if (x < 0) {
	if (zchko(old) > -1) {		/* Requires write access to orignal */
	    x = link(old,s);
	    debug(F111,"zrename link()",old,x);
	    if (x > -1) {		/* Make a link with the new name. */
		x = unlink(old);
		debug(F111,"zrename unlink()",old,x);
	    }
	    /* If link/unlink failed copy and delete */
	    if (x < 0) {
		x = zcopy(old,s);
		debug(F111,"zrename zcopy()",old,x);
		if (x > -1) {
		    x = zdelet(old);
		    debug(F111,"zrename zdelet()",old,x);
		}
	    }
	}
    }
    fullname[0] = '\0';			/* Clear this out for next time. */

#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_FC && ckxlogging) {
        zfnqfp(old,CKMAXPATH,fullname);
        tmp2[0] = '\0';
        zfnqfp(s,CKMAXPATH,tmp2);
        if (x > -1)
          syslog(LOG_INFO,"file[] %s: renamed to %s ok", fullname, tmp2);
        else
          syslog(LOG_INFO,"file[] %s: rename to %s failed (%m)",fullname,tmp2);
    }
#endif /* CKSYSLOG */

    if (p) free(p);
    return(x);
}

/*  Z C O P Y  --  Copy a single file. */
/*
  Call with source and destination names.
  If destination is a directory, the source file is
  copied to that directory with its original name.
  Returns:
   0 on success.
  <0 on failure:
  -2 = source file is not a regular file.
  -3 = source file not found.
  -4 = permission denied.
  -5 = source and destination are the same file.
  -6 = i/o error.
  -1 = other error.
*/
int
zcopy(source,destination) char *source, *destination; {
    char *src, *dst;			/* Local pointers to filenames */
    int x, y, rc;                       /* Workers */
    int in = -1, out = -1;              /* i/o file descriptors */
    struct stat srcbuf;                 /* Source file info buffer */
    int perms;                          /* Output file permissions */
    char buf[1024];                     /* File copying buffer */

    if (!source) source = "";
    if (!destination) destination = "";

    debug(F110,"zcopy src arg",source,0);
    debug(F110,"zcopy dst arg",destination,0);

    if (!*source) return(-1);
    if (!*destination) return(-1);

#ifdef IKSD
#ifdef CK_LOGIN
    if (inserver && isguest)
      return(-4);
#endif /* CK_LOGIN */
#endif /* IKSD */

#ifdef CKROOT
    debug(F111,"zcopy setroot",ckroot,ckrootset);
    if (ckrootset) {
	if (!zinroot(source)) {
	    debug(F110,"zcopy source: setroot violation",source,0);
	    return(-1);
	}
	if (!zinroot(destination)) {
	    debug(F110,"zcopy destination: setroot violation",destination,0);
	    return(-1);
	}
    }
#endif /* CKROOT */

    src = source;
    dst = destination;

    if (stat(src,&srcbuf) == 0) {       /* Get source file info */
        struct stat dstbuf;             /* Destination file info buffer */
	debug(F101,"STAT","",6);
        if (stat(dst,&dstbuf) == 0) {
	    debug(F101,"STAT","",7);
            if (srcbuf.st_dev == dstbuf.st_dev)
              if (srcbuf.st_ino == dstbuf.st_ino) {
                  debug(F100,"zcopy files identical: stat()","",0);
                  return(-5);
              }
        }
    } else {                            /* stat() failed... */
	debug(F101,"STAT","",8);
        debug(F111,"source file not found",src,errno);
        return(-3);
    }
    fullname[0] = '\0';                 /* Get full pathnames */
    if (zfnqfp(source,CKMAXPATH,fullname))
      src = fullname;
    debug(F110,"zcopy src",src,0);
    tmp2[0] = '\0';
    if (zfnqfp(destination,CKMAXPATH,tmp2))
      dst = tmp2;
    debug(F110,"zcopy dst 1",dst,0);
    if (!strcmp(src,dst)) {             /* Src and dst are same file? */
        debug(F100,"zcopy files identical: strcmp()","",0); /* This... */
        return(-5);                     /* should not happen. */
    }
    if (isdir(src)) {                   /* Source file is a directory? */
        debug(F110,"zcopy source is directory",src,0);
        return(-2);                     /* Fail */
    }
    if (isdir(dst)) {                   /* Destination is a directory? */
        char *q = NULL;                 /* Yes, add filename to it. */
        x = strlen(dst);
	if (x < 1) return(-1);
        if (!ISDIRSEP(*(dst+x-1))) {    /* Add separator if needed */
            tmp2[x++] = '/';
            tmp2[x] = '\0';
        }
	debug(F111,"zcopy dst 2",dst,x);
        zstrip(src,&q);                 /* Strip path part from old name */
        ckstrncpy(tmp2+x,q,CKMAXPATH-x); /* Concatenate it to new name */
    }
    debug(F110,"zcopy dst 3",dst,0);

#ifdef IKSD
    if (inserver && (!ENABLED(en_del))) {
	if (zchki(dst) > -1)		/* Destination file exists? */
	  return(-4);
    }
#endif /* IKSD */

    perms = umask(0);                   /* Get user's umask */
    umask(perms);			/* Put it back! */
    perms ^= 0777;                      /* Flip the bits */
    perms &= 0666;                      /* Zero execute bits from umask */
    perms |= (srcbuf.st_mode & 0111);   /* OR in source file's execute bits */
    rc = -1;                            /* Default return code */
    errno = 0;                          /* Reset errno */
    in = open(src, O_RDONLY, 0);        /* Open source file */
    debug(F111,"zcopy open source",src,in);
    if (in > -1) {                      /* If open... */
	/* Open destination file */
#ifdef O_TRUNC
        out = open(dst, O_WRONLY|O_CREAT|O_TRUNC, perms);
#else
        out = open(dst, O_WRONLY|O_CREAT, perms);
#endif /* O_TRUNC */
        debug(F111,"zcopy open dest",dst,out);
        if (out > -1) {                 /* If open... */
            while ((x = read(in,buf,1024)) > 0) { /* Copy in 1K blocks */
                y = write(out,buf,x);
                if (y < 0) {            /* On write failure */
                    x = -1;
                    rc = -6;            /* Indicate i/o error */
                    break;
                }
            }
            debug(F101,"zcopy final read","",x);
            debug(F101,"zcopy errno","",errno);
            rc = (x == 0) ? 0 : -6;     /* In case of read failure */
        }
    }
    if (in > -1) close(in);             /* Close files */
    if (out > -1) close(out);
    if (rc == -1) {                     /* Set return code */
        switch (errno) {
          case ENOENT: rc = -3; break;
          case EACCES: rc = -4; break;
          case EIO:    rc = -6;
        }
    }

#ifdef CKSYSLOG
    if (rc > -1 && ckxsyslog >= SYSLG_FC && ckxlogging) {
        if (rc)
          syslog(LOG_INFO,"file[] %s: copy to %s failed (%m)", fullname, tmp2);
        else
          syslog(LOG_INFO,"file[] %s: copy to %s ok", fullname, tmp2);
    }
#endif /* CKSYSLOG */

    return(rc);
}

/*  Z S A T T R */
/*
 Fills in a Kermit file attribute structure for the file which is to be sent.
 Returns 0 on success with the structure filled in, or -1 on failure.
 If any string member is null, then it should be ignored.
 If any numeric member is -1, then it should be ignored.
*/
#ifdef CK_PERMS

#ifdef CK_GPERMS
#undef CK_GPERMS
#endif /* CK_GPERMS */

#ifdef UNIX
#ifndef S_IRUSR
#define S_IRUSR 0400
#endif /* S_IRUSR */
#ifndef S_IWUSR
#define S_IXUSR 0200
#endif /* S_IWUSR */
#ifndef S_IXUSR
#define S_IXUSR 0100
#endif /* S_IXUSR */
#endif /* UNIX */

#ifdef S_IRUSR
#ifdef S_IWUSR
#ifdef S_IXUSR
#define CK_GPERMS
#endif /* S_IXUSR */
#endif /* S_IWUSR */
#endif /* S_IRUSR */

static char gperms[2];

#endif /* CK_GPERMS */

static char lperms[24];

#ifdef CK_PERMS
static char xlperms[24];

/*  Z S E T P E R M  --  Set permissions of a file  */

int
zsetperm(f,code) char * f; int code; {
    int x;
#ifdef CK_SCO32V4
    mode_t mask;
#else
    int mask;
#endif /* CK_SCO32V4 */
    mask = code;
    if (inserver && guest) {
	debug(F110,"zsetperm guest",f,0);
	return(0);
    }
    x = chmod(f,mask);
    if (x < 0) {
	debug(F111,"zsetperm error",f,errno);
	return(0);
    }
    debug(F111,"zsetperm ok",f,mask);
    return(1);
}

/*  Z G P E R M  --  Get permissions of a file as an octal string  */

char *
zgperm(f) char *f; {
    extern int diractive;
    int x; char *s = (char *)xlperms;
    struct stat buf;
    debug(F110,"zgperm",f,0);
    if (!f) return("----------");
    if (!*f) return("----------");

#ifdef CKROOT
    debug(F111,"zgperm setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(f)) {
	debug(F110,"zgperm setroot violation",f,0);
	return("----------");
    }
#endif /* CKROOT */

#ifdef USE_LSTAT
    if (diractive)
      x = lstat(f,&buf);
    else
#endif /* USE_LSTAT */
      x = stat(f,&buf);
    debug(F101,"STAT","",9);
    if (x < 0)
      return("----------");
    sprintf(s,"%o",buf.st_mode);
    debug(F110,"zgperm",s,0);
    return(s);
}

/* Like zgperm() but returns permissions in "ls -l" string format */

static char xsperms[24];

char *
ziperm(f) char * f; {
    extern int diractive;
    int x; char *s = (char *)xsperms;
    struct stat buf;
    unsigned int perms = 0;

    debug(F110,"ziperm",f,0);

    if (!f) return(NULL);
    if (!*f) return(NULL);

    if (diractive && zgfs_mode != 0) {
	perms = zgfs_mode;		/* zgetfs() already got them */
    } else {
#ifdef USE_LSTAT
	if (diractive)
	  x = lstat(f,&buf);
	else
#endif /* USE_LSTAT */
	  x = stat(f,&buf);
	debug(F101,"STAT","",10);
	if (x < 0)
	  return("----------");
	perms = buf.st_mode;
    }
    switch (perms & S_IFMT) {
      case S_IFDIR:
        *s++ = 'd';
        break;
      case S_IFCHR:                     /* Character special */
        *s++ = 'c';
        break;
      case S_IFBLK:                     /* Block special */
        *s++ = 'b';
        break;
      case S_IFREG:                     /* Regular */
        *s++ = '-';
        break;
#ifdef S_IFLNK
      case S_IFLNK:                     /* Symbolic link */
        *s++ = 'l';
        break;
#endif /* S_IFLNK */
#ifdef S_IFSOCK
      case S_IFSOCK:                    /* Socket */
        *s++ = 's';
        break;
#endif /* S_IFSOCK */
#ifdef S_IFIFO
#ifndef Plan9
#ifndef COHERENT
      case S_IFIFO:                     /* FIFO */
        *s++ = 'p';
        break;
#endif /* COHERENT */
#endif /* Plan9 */
#endif /* S_IFIFO */
#ifdef S_IFWHT
      case S_IFWHT:                     /* Whiteout */
        *s++ = 'w';
        break;
#endif /* S_IFWHT */
      default:                          /* Unknown */
        *s++ = '?';
        break;
    }
    if (perms & S_IRUSR)          /* Owner's permissions */
      *s++ = 'r';
    else
      *s++ = '-';
    if (perms & S_IWUSR)
      *s++ = 'w';
    else
      *s++ = '-';
    switch (perms & (S_IXUSR | S_ISUID)) {
      case 0:
        *s++ = '-';
        break;
      case S_IXUSR:
        *s++ = 'x';
        break;
      case S_ISUID:
        *s++ = 'S';
        break;
      case S_IXUSR | S_ISUID:
        *s++ = 's';
        break;
    }
    if (perms & S_IRGRP)          /* Group permissions */
      *s++ = 'r';
    else
      *s++ = '-';
    if (perms & S_IWGRP)
      *s++ = 'w';
    else
      *s++ = '-';
    switch (perms & (S_IXGRP | S_ISGID)) {
      case 0:
        *s++ = '-';
        break;
      case S_IXGRP:
        *s++ = 'x';
        break;
      case S_ISGID:
        *s++ = 'S';
        break;
      case S_IXGRP | S_ISGID:
        *s++ = 's';
        break;
    }
    if (perms & S_IROTH)          /* World permissions */
      *s++ = 'r';
    else
      *s++ = '-';
    if (perms & S_IWOTH)
      *s++ = 'w';
    else
      *s++ = '-';
    switch (
#ifdef Plan9
            perms & (S_IXOTH)
#else
            perms & (S_IXOTH | S_ISVTX)
#endif
            ) {
      case 0:
        *s++ = '-';
        break;
      case S_IXOTH:
        *s++ = 'x';
        break;
#ifndef Plan9
      case S_ISVTX:
        *s++ = 'T';
        break;
      case S_IXOTH | S_ISVTX:
        *s++ = 't';
        break;
#endif /* Plan9 */
    }
    *s = '\0';
    debug(F110,"ziperm",xsperms,0);
    return((char *)xsperms);
}

#else

char *
zgperm(f) char *f; {
    return("----------");
}
char *
ziperms(f) char *f; {
    return("----------");
}
#endif /* CK_PERMS */

int
zsattr(xx) struct zattr *xx; {
    CK_OFF_T k; int x;
    struct stat buf;

    k = iflen % 1024;			/* File length in K */
    if (k) k = 1L;
    xx->lengthk = (iflen / 1024) + k;
    xx->type.len = 0;                   /* File type can't be filled in here */
    xx->type.val = "";
    if (*nambuf) {
        xx->date.val = zfcdat(nambuf);  /* File creation date */
        xx->date.len = (int)strlen(xx->date.val);
    } else {
        xx->date.len = 0;
        xx->date.val = "";
    }
    xx->creator.len = 0;                /* File creator */
    xx->creator.val = "";
    xx->account.len = 0;                /* File account */
    xx->account.val = "";
    xx->area.len = 0;                   /* File area */
    xx->area.val = "";
    xx->password.len = 0;               /* Area password */
    xx->password.val = "";
    xx->blksize = -1L;                  /* File blocksize */
    xx->xaccess.len = 0;                /* File access */
    xx->xaccess.val = "";
    xx->encoding.len = 0;               /* Transfer syntax */
    xx->encoding.val = 0;
    xx->disp.len = 0;                   /* Disposition upon arrival */
    xx->disp.val = "";
    xx->lprotect.len = 0;               /* Local protection */
    xx->lprotect.val = "";
    xx->gprotect.len = 0;               /* Generic protection */
    xx->gprotect.val = "";
    x = -1;
    if (*nambuf) x = stat(nambuf,&buf);
    debug(F101,"STAT","",11);
    if (x >= 0) {
        debug(F111,"zsattr buf.st_mode & 0777",nambuf,buf.st_mode & 0777);
        /* UNIX filemode as an octal string without filetype bits */
        sprintf(lperms,"%o",buf.st_mode & 0777);
        xx->lprotect.len = (int)strlen(lperms);
        xx->lprotect.val = (char *)lperms;
        x = 0;
#ifdef CK_GPERMS
        /* Generic permissions only if we have stat.h symbols defined */
        if (buf.st_mode & S_IRUSR) x |= 1;      /* Read */
        if (buf.st_mode & S_IWUSR) x |= (2+16); /* Write and Delete */
        if (buf.st_mode & S_IXUSR) x |= 4;      /* Execute */
        gperms[0] = tochar(x);
        gperms[1] = NUL;
        xx->gprotect.len = 1;
        xx->gprotect.val = (char *)gperms;
#endif /* CK_GPERMS */
    }
    debug(F111,"zsattr lperms",xx->lprotect.val,xx->lprotect.len);
    debug(F111,"zsattr gperms",xx->gprotect.val,xx->gprotect.len);
    xx->systemid.val = "U1";            /* U1 = UNIX */
    xx->systemid.len = 2;               /* System ID */
    xx->recfm.len = 0;                  /* Record format */
    xx->recfm.val = "";
    xx->sysparam.len = 0;               /* System-dependent parameters */
    xx->sysparam.val = "";
    xx->length = iflen;                 /* Length */
    return(0);
}

/* Z F C D A T  --  Get file creation date */
/*
  Call with pointer to filename.
  On success, returns pointer to modification date in yyyymmdd hh:mm:ss format.
  On failure, returns pointer to null string.
*/
static char datbuf[40];

char *
#ifdef CK_ANSIC
zdtstr(time_t timearg)
#else
zdtstr(timearg) time_t timearg;
#endif /* CK_ANSIC */
/* zdtstr */ {
#ifndef TIMESTAMP
    return("");
#else
    struct tm * time_stamp;
    struct tm * localtime();
    int yy, ss;

    debug(F101,"zdtstr timearg","",timearg);
    if (timearg < 0)
      return("");
    time_stamp = localtime(&(timearg));
    if (!time_stamp) {
        debug(F100,"localtime returns null","",0);
        return("");
    }
/*
  We assume that tm_year is ALWAYS years since 1900.
  Any platform where this is not the case will have problems
  starting in 2000.
*/
    yy = time_stamp->tm_year;           /* Year - 1900 */
    debug(F101,"zdtstr tm_year","",time_stamp->tm_year);
    if (yy > 1000) {
        debug(F101,"zstrdt YEAR-2000 ALERT 1: localtime year","",yy);
    }
    yy += 1900;
    debug(F101,"zdatstr year","",yy);

    if (time_stamp->tm_mon  < 0 || time_stamp->tm_mon  > 11)
      return("");
    if (time_stamp->tm_mday < 0 || time_stamp->tm_mday > 31)
      return("");
    if (time_stamp->tm_hour < 0 || time_stamp->tm_hour > 23)
      return("");
    if (time_stamp->tm_min  < 0 || time_stamp->tm_min  > 59)
      return("");
    ss = time_stamp->tm_sec;            /* Seconds */
    if (ss < 0 || ss  > 59)             /* Some systems give a BIG number */
      ss = 0;
    sprintf(datbuf,
#ifdef pdp11
/* For some reason, 2.1x BSD sprintf gets the last field wrong. */
            "%04d%02d%02d %02d:%02d:00",
#else
            "%04d%02d%02d %02d:%02d:%02d",
#endif /* pdp11 */
            yy,
            time_stamp->tm_mon + 1,
            time_stamp->tm_mday,
            time_stamp->tm_hour,
            time_stamp->tm_min
#ifndef pdp11
            , ss
#endif /* pdp11 */
            );
    yy = (int)strlen(datbuf);
    debug(F111,"zdatstr",datbuf,yy);
    if (yy > 17) datbuf[17] = '\0';
    return(datbuf);
#endif /* TIMESTAMP */
}

char *
zfcdat(name) char *name; {
#ifdef TIMESTAMP
    struct stat buffer;
    extern int diractive;
    unsigned int mtime;
    int x;
    char * s;

    if (!name)
      return("");
    s = name;
    if (!*s)
      return("");

#ifdef CKROOT
    debug(F111,"zfcdat setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(name)) {
	debug(F110,"zfcdat setroot violation",name,0);
	return("");
    }
#endif /* CKROOT */

#ifdef DTILDE
    if (*s == '~') {
        s = tilde_expand(s);
        if (!s) s = "";
        if (!*s) s = name;
    }
#endif /* DTILDE */

    datbuf[0] = '\0';
    x = 0;
    debug(F111,"zfcdat",s,diractive);

    if (diractive && zgfs_mtime) {
	mtime = zgfs_mtime;
    } else {
#ifdef USE_LSTAT
	if (diractive) {
	    x = lstat(s,&buffer);
	    debug(F101,"STAT","",12);
	    debug(F101,"zfcdat lstat","",x);
	} else {
#endif /* USE_LSTAT */
	    x = stat(s,&buffer);
	    debug(F101,"STAT","",13);
	    debug(F101,"zfcdat stat","",x);
#ifdef USE_LSTAT
	}
#endif /* USE_LSTAT */
	if (x != 0) {
#ifdef USE_LSTAT
	    debug(F111,"zfcdat stat failed",s,errno);
#else
	    debug(F111,"zfcdat lstat failed",s,errno);
#endif /* USE_LSTAT */
	    return("");
	}
	debug(F101,"zfcdat buffer.st_mtime","",buffer.st_mtime);
	mtime = buffer.st_mtime;
    }
    return(zdtstr(mtime));
#else
    return("");
#endif /* TIMESTAMP */
}

#ifndef NOTIMESTAMP

/* Z S T R D T  --  Converts local date string to internal representation */
/*
  In our case (UNIX) this is seconds since midnite 1 Jan 1970 UTC,
  suitable for comparison with UNIX file dates.  As far as I know, there is
  no library or system call -- at least nothing reasonably portable -- to
  convert local time to UTC.
*/
time_t
zstrdt(date,len) char * date; int len; {
#ifdef M_UNIX
/*
  SCO UNIX 3.2v2.0 and ODT 2.0 lack prototypes for ftime().
  ODT 3.0 (3.2v4.2 OS) has a prototype, which may vary in
  dependence on the XPG4 supplement presence.  So always use
  what the system header file supplies in ODT 3.0...
*/
#ifndef ODT30
#ifndef _SCO_DS
    extern void ftime();  /* extern void ftime(struct timeb *) */
#endif /* _SCO_DS */
#endif /* ODT30 */
#else
#ifndef M_XENIX
    extern int ftime();
#endif /* M_XENIX */
#endif /* M_UNIX */
    extern struct tm * localtime();

    /* And this should have been declared always through a header file */
#ifdef HPUX10
    time_t tmx;
    long days;
#else
#ifdef BSD44
    time_t tmx;
    long days;
#else
    long tmx, days;
#endif /* BSD44 */
#endif /* HPUX10 */
    int i, n, isleapyear;
                   /*       J  F  M  A   M   J   J   A   S   O   N   D   */
                   /*      31 28 31 30  31  30  31  31  30  31  30  31   */
    static
    int monthdays [13] = {  0,0,31,59,90,120,151,181,212,243,273,304,334 };
    char s[5];
    struct tm *time_stamp;

#ifdef BSD44
    struct timeval tp[2];
    long xtimezone = 0L;
#else
#ifdef V7
    struct utimbuf {
      time_t timep[2];          /* New access and modificaton time */
    } tp;
    char *tz;
    long timezone;              /* In case timezone not defined in .h file */
#else
#ifdef SYSUTIMEH
    struct utimbuf tp;
#else
    struct utimbuf {
        time_t atime;
        time_t mtime;
    } tp;
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* BSD44 */

#ifdef ANYBSD
    long timezone = 0L;
    static struct timeb tbp;
#endif /* ANYBSD */

#ifdef BEBOX
    long timezone = 0L;
#endif /* BEBOX */

    debug(F111,"zstrdt",date,len);

    if ((len == 0)
        || (len != 17)
        || (date[8] != ' ')
        || (date[11] != ':')
        || (date[14] != ':') ) {
        debug(F111,"Bad creation date ",date,len);
        return(-1);
    }
    debug(F111,"zstrdt date check 1",date,len);
    for(i = 0; i < 8; i++) {
        if (!isdigit(date[i])) {
            debug(F111,"Bad creation date ",date,len);
            return(-1);
        }
    }
    debug(F111,"zstrdt date check 2",date,len);
    i++;

    for (; i < 16; i += 3) {
        if ((!isdigit(date[i])) || (!isdigit(date[i + 1]))) {
            debug(F111,"Bad creation date ",date,len);
            return(-1);
        }
    }
    debug(F111,"zstrdt date check 3",date,len);


#ifdef COMMENT /* was BSD44 */
/*
   man gettimeofday on BSDI 3.1 says:
   "The timezone field is no longer used; timezone information is stored out-
     side the kernel.  See ctime(3) for more information."  So this chunk of
   code is effectively a no-op, at least in BSDI 3.x.
*/
    {
        int x;
        struct timezone tzp;
        x = gettimeofday(NULL, &tzp);
        debug(F101,"zstrdt BSD44 gettimeofday","",x);
        if (x > -1)
          xtimezone = tzp.tz_minuteswest * 60L;
        else
          xtimezone = 0L;
        debug(F101,"zstrdt BSD44 timezone","",xtimezone);
    }
#else
#ifdef ANYBSD
    debug(F100,"zstrdt BSD calling ftime","",0);
    ftime(&tbp);
    debug(F100,"zstrdt BSD back from ftime","",0);
    timezone = tbp.timezone * 60L;
    debug(F101,"zstrdt BSD timezone","",timezone);
#else
#ifdef SVORPOSIX
    tzset();                            /* Set timezone */
#else
#ifdef V7
    if ((tz = getenv("TZ")) == NULL)
      timezone = 0;                     /* UTC/GMT */
    else
      timezone = atoi(&tz[3]);          /* Set 'timezone'. */
    timezone *= 60L;
#endif /* V7 */
#endif /* SVORPOSIX */
#endif /* ANYBSD */
#endif /* COMMENT (was BSD44) */

    debug(F100,"zstrdt so far so good","",0);

    s[4] = '\0';
    for (i = 0; i < 4; i++)             /* Fix the year */
      s[i] = date[i];

    n = atoi(s);
    debug(F111,"zstrdt year",s,n);
    if (n < 1970) {
        debug(F100,"zstrdt fails - year","",n);
        return(-1);
    }

/*  Previous year's leap days.  This won't work after year 2100. */

    isleapyear = (( n % 4 == 0 && n % 100 !=0) || n % 400 == 0);
    days = (long) (n - 1970) * 365;
    days += (n - 1968 - 1) / 4 - (n - 1900 - 1) / 100 + (n - 1600 - 1) / 400;

    s[2] = '\0';

    for (i = 4; i < 16; i += 2) {
        s[0] = date[i];
        s[1] = date[i + 1];
        n = atoi(s);
        switch (i) {
          case 4:                       /* MM: month */
            if ((n < 1 ) || ( n > 12)) {
                debug(F111,"zstrdt 4 bad date ",date,len);
                return(-1);
            }
            days += monthdays [n];
            if (isleapyear && n > 2)
              ++days;
            continue;

          case 6:                       /* DD: day */
            if ((n < 1 ) || ( n > 31)) {
                debug(F111,"zstrdt 6 bad date ",date,len);
                return(-1);
            }
            tmx = (days + n - 1) * 24L * 60L * 60L;
            i++;                        /* Skip the space */
            continue;

          case 9:                       /* hh: hour */
            if ((n < 0 ) || ( n > 23)) {
                debug(F111,"zstrdt 9 bad date ",date,len);
                return(-1);
            }
            tmx += n * 60L * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 12:                      /* mm: minute */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zstrdt 12 bad date ",date,len);
                return(-1);
            }
#ifdef COMMENT /* (was BSD44) */        /* Correct for time zone */
            tmx += xtimezone;
            debug(F101,"zstrdt BSD44 tmx","",tmx);
#else
#ifdef ANYBSD
            tmx += timezone;
#else
#ifndef CONVEX9 /* Don't yet know how to do this here */
#ifdef ultrix
            tmx += (long) timezone;
#else
#ifdef Plan9
            {
                extern time_t tzoffset;
                tmx += tzoffset;
            }
#else
#ifndef BSD44
#ifndef NOTIMEZONE
            tmx += timezone;
#endif	/* NOTIMEZONE */
#endif /* BSD44 */
#endif /* Plan9 */
#endif /* ultrix */
#endif /* CONVEX9 */
#endif /* ANYBSD */
#endif /* COMMENT (was BSD44) */
            tmx += n * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 15:                      /* ss: second */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zstrdt 15 bad date ",date,len);
                return(-1);
            }
            tmx += n;
        }
        time_stamp = localtime(&tmx);
        debug(F101,"zstrdt tmx 1","",tmx);
        if (!time_stamp)
          return(-1);
#ifdef COMMENT
        /* Why was this here? */
        time_stamp = localtime(&tmx);
        debug(F101,"zstrdt tmx 2","",tmx);
#endif /* COMMENT */
#ifdef BSD44
        {   /* New to 7.0 - Works in at at least BSDI 3.1 and FreeBSD 2.2.7 */
            long zz;
            zz = time_stamp->tm_gmtoff; /* Seconds away from Zero Meridian */
            debug(F101,"zstrdt BSD44 tm_gmtoff","",zz);
            tmx -= zz;
            debug(F101,"zstrdt BSD44 tmx 3 (GMT)","",tmx);
        }
#else
        /*
           Daylight Savings Time adjustment.
           Do this everywhere BUT in BSD44 because in BSD44,
           tm_gmtoff also includes the DST adjustment.
        */
        if (time_stamp->tm_isdst) {
            tmx -= 60L * 60L;
            debug(F101,"zstrdt tmx 3 (DST)","",tmx);
        }
#endif /* BSD44 */
        n = time_stamp->tm_year;
        if (n < 300) {
            n += 1900;
        }
    }
    return(tmx);
}


#ifdef ZLOCALTIME
/* Z L O C A L T I M E  --  GMT/UTC time string to local time string */

/*
   Call with: "yyyymmdd hh:mm:ss" GMT/UTC date-time.
   Returns:   "yyyymmdd hh:mm:ss" local date-time on success, NULL on failure.
*/
static char zltimbuf[64];

char *
zlocaltime(gmtstring) char * gmtstring; {
#ifdef M_UNIX
/*
  SCO UNIX 3.2v2.0 and ODT 2.0 lack prototypes for ftime().
  ODT 3.0 (3.2v4.2 OS) has a prototype, which may vary in
  dependence on the XPG4 supplement presence.  So always use
  what the system header file supplies in ODT 3.0...
*/
#ifndef ODT30
#ifndef _SCO_DS
    extern void ftime();  /* extern void ftime(struct timeb *) */
#endif /* _SCO_DS */
#endif /* ODT30 */
#else
#ifndef M_XENIX
    extern int ftime();
#endif /* M_XENIX */
#endif /* M_UNIX */
    extern struct tm * localtime();

    /* And this should have been declared always through a header file */
#ifdef HPUX10
    time_t tmx;
    long days;
#else
#ifdef BSD44
    time_t tmx;
    long days;
#else
    long tmx, days;
#endif /* BSD44 */
#endif /* HPUX10 */
    int i, n, x, isleapyear;
                   /*       J  F  M  A   M   J   J   A   S   O   N   D   */
                   /*      31 28 31 30  31  30  31  31  30  31  30  31   */
    static
    int monthdays [13] = {  0,0,31,59,90,120,151,181,212,243,273,304,334 };
    char s[5];
    struct tm *time_stamp;

#ifdef BSD44
    struct timeval tp[2];
#else
#ifdef V7
    struct utimbuf {
      time_t timep[2];          /* New access and modificaton time */
    } tp;
#else
#ifdef SYSUTIMEH
    struct utimbuf tp;
#else
    struct utimbuf {
        time_t atime;
        time_t mtime;
    } tp;
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* BSD44 */

#ifdef ANYBSD
    static struct timeb tbp;
#endif /* ANYBSD */

    char * date = gmtstring;
    int len;

    len = strlen(date);
    debug(F111,"zlocaltime",date,len);

    if ((len == 0)
        || (len != 17)
        || (date[8] != ' ')
        || (date[11] != ':')
        || (date[14] != ':') ) {
        debug(F111,"Bad creation date ",date,len);
        return(NULL);
    }
    debug(F111,"zlocaltime date check 1",date,len);
    for(i = 0; i < 8; i++) {
        if (!isdigit(date[i])) {
            debug(F111,"Bad creation date ",date,len);
            return(NULL);
        }
    }
    debug(F111,"zlocaltime date check 2",date,len);
    i++;

    for (; i < 16; i += 3) {
        if ((!isdigit(date[i])) || (!isdigit(date[i + 1]))) {
            debug(F111,"Bad creation date ",date,len);
	    return(NULL);
        }
    }
    debug(F111,"zlocaltime date check 3",date,len);

    debug(F100,"zlocaltime so far so good","",0);

    s[4] = '\0';
    for (i = 0; i < 4; i++)             /* Fix the year */
      s[i] = date[i];

    n = atoi(s);
    debug(F111,"zlocaltime year",s,n);
    if (n < 1970) {
        debug(F100,"zlocaltime fails - year","",n);
        return(NULL);
    }

/*  Previous year's leap days.  This won't work after year 2100. */

    isleapyear = (( n % 4 == 0 && n % 100 !=0) || n % 400 == 0);
    days = (long) (n - 1970) * 365;
    days += (n - 1968 - 1) / 4 - (n - 1900 - 1) / 100 + (n - 1600 - 1) / 400;

    s[2] = '\0';

    for (i = 4; i < 16; i += 2) {
        s[0] = date[i];
        s[1] = date[i + 1];
        n = atoi(s);
        switch (i) {
          case 4:                       /* MM: month */
            if ((n < 1 ) || ( n > 12)) {
                debug(F111,"zlocaltime 4 bad date ",date,len);
                return(NULL);
            }
            days += monthdays [n];
            if (isleapyear && n > 2)
              ++days;
            continue;

          case 6:                       /* DD: day */
            if ((n < 1 ) || ( n > 31)) {
                debug(F111,"zlocaltime 6 bad date ",date,len);
                return(NULL);
            }
            tmx = (days + n - 1) * 24L * 60L * 60L;
            i++;                        /* Skip the space */
            continue;

          case 9:                       /* hh: hour */
            if ((n < 0 ) || ( n > 23)) {
                debug(F111,"zlocaltime 9 bad date ",date,len);
                return(NULL);
            }
            tmx += n * 60L * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 12:                      /* mm: minute */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zlocaltime 12 bad date ",date,len);
                return(NULL);
            }
            tmx += n * 60L;
            i++;                        /* Skip the colon */
            continue;

          case 15:                      /* ss: second */
            if ((n < 0 ) || ( n > 59)) {
                debug(F111,"zlocaltime 15 bad date ",date,len);
                return(NULL);
            }
            tmx += n;
        }

/*
  At this point tmx is the time_t representation of the argument date-time
  string without any timezone or DST adjustments.  Therefore it should be
  the same as the time_t representation of the GMT/UTC time.  Now we should
  be able to feed it to localtime() and have it converted to a struct tm
  representing the local time equivalent of the given UTC time.
*/
        time_stamp = localtime(&tmx);
        if (!time_stamp)
          return(NULL);
    }

/* Now we simply reformat the struct tm to a string */

    x = time_stamp->tm_year;
    if (time_stamp->tm_year < 70 || time_stamp->tm_year > 8099)
      return(NULL);
    if (time_stamp->tm_mon < 0 || time_stamp->tm_mon > 11)
      return(NULL);
    if (time_stamp->tm_mday < 1 || time_stamp->tm_mday > 31)
      return(NULL);
    if (time_stamp->tm_hour < 0 || time_stamp->tm_hour > 24)
      return(NULL);
    if (time_stamp->tm_min < 0 || time_stamp->tm_min > 60)
      return(NULL);
    if (time_stamp->tm_sec < 0 || time_stamp->tm_sec > 60)
      return(NULL);
    sprintf(zltimbuf,"%04d%02d%02d %02d:%02d:%02d",
	    time_stamp->tm_year + 1900,
	    time_stamp->tm_mon + 1,
	    time_stamp->tm_mday,
	    time_stamp->tm_hour,
	    time_stamp->tm_min,
	    time_stamp->tm_sec
	    );
    return((char *)zltimbuf);
}
#endif /* ZLOCALTIME */
#endif /* NOTIMESTAMP */

/* Z S T I M E  --  Set modification date/time+permissions for incoming file */
/*
 Call with:
 f  = pointer to name of existing file.
 yy = pointer to a Kermit file attribute structure in which yy->date.val
      is a date of the form yyyymmdd hh:mm:ss, e.g. 19900208 13:00:00.
      yy->lprotect.val & yy->gprotect.val are permission/protection values.
 x  = is a function code: 0 means to set the file's attributes as given.
      1 means compare the date in struct yy with the file creation date.
 Returns:
 -1 on any kind of error.
  0 if x is 0 and the attributes were set successfully.
  0 if x is 1 and date from attribute structure <= file creation date.
  1 if x is 1 and date from attribute structure > file creation date.
*/
int
zstime(f,yy,x)
    char *f; struct zattr *yy; int x;
/* zstime */ {
    int r = -1;                         /* Return code */
#ifdef CK_PERMS
    int setperms = 0;
#endif /* CK_PERMS */
    int setdate = 0;

/* It is ifdef'd TIMESTAMP because it might not work on V7. bk@kullmar.se.  */

#ifdef TIMESTAMP
#ifdef BSD44
    extern int utimes();
#else
    extern int utime();
#endif /* BSD44 */

    struct stat sb;

/* At least, the declarations for int functions are not needed anyway */

#ifdef BSD44
    struct timeval tp[2];
    long xtimezone;
#else
#ifdef V7
    struct utimbuf {
	time_t timep[2];		/* New access and modificaton time */
    } tp;
    char *tz;
    long timezone;                      /* In case not defined in .h file */
#else
#ifdef SYSUTIMEH
    struct utimbuf tp;
#else
    struct utimbuf {
        time_t atime;
        time_t mtime;
    } tp;
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* BSD44 */

    long tm = 0L;

    if (!f) f = "";
    if (!*f) return(-1);
    if (!yy) return(-1);

#ifdef CKROOT
    debug(F111,"zstime setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(f)) {
	debug(F110,"zstime setroot violation",f,0);
	return(0);
    }
#endif /* CKROOT */

    if (yy->date.len == 0) {            /* No date in struct */
        if (yy->lprotect.len != 0) {    /* So go do permissions */
            goto zsperms;
        } else {
            debug(F100,"zstime: nothing to do","",0);
            return(0);
        }
    }
    if ((tm = zstrdt(yy->date.val,yy->date.len)) < 0) {
        debug(F101,"zstime: zstrdt fails","",0);
        return(-1);
    }
    debug(F101,"zstime: tm","",tm);
    debug(F111,"zstime: A-pkt date ok ",yy->date.val,yy->date.len);

    if (stat(f,&sb)) {                  /* Get the time for the file */
	debug(F101,"STAT","",14);
        debug(F111,"zstime: Can't stat file:",f,errno);
        return(-1);
    }
    debug(F101,"STAT","",15);
    setdate = 1;

  zsperms:
#ifdef CK_PERMS
    {
        int i, x = 0, xx, flag = 0;
        char * s;
#ifdef DEBUG
        char obuf[24];
        if (deblog) {
            debug(F111,"zstime lperms",yy->lprotect.val,yy->lprotect.len);
            debug(F111,"zstime gperms",yy->gprotect.val,yy->gprotect.len);
            debug(F110,"zstime system id",yy->systemid.val,0);
            sprintf(obuf,"%o",sb.st_mode);
            debug(F110,"zstime file perms before",obuf,0);
        }
#endif /* DEBUG */

#ifdef CK_LOGIN
        debug(F101,"zstime isguest","",isguest);
        debug(F101,"zstime ckxperms","",ckxperms);
        if (isguest) {
#ifdef COMMENT
            /* Clear owner permissions */
            sb.st_mode &= (unsigned) 0177077; /* (16 bits) */
#else
            /* Set permissions from ckxperms variable */
            sb.st_mode = ckxperms;
#endif /* COMMENT */
            debug(F101,"zstime isguest sb.st_mode","",sb.st_mode);
#ifdef COMMENT
            /* We already set them in zopeno() */
            setperms = 1;
#endif /* COMMENT */
            flag = 0;
        } else
#endif /* CK_LOGIN */
          if ((yy->lprotect.len > 0 &&  /* Have local-format permissions */
            yy->systemid.len > 0 &&     /* from A-packet... */
#ifdef UNIX
            !strcmp(yy->systemid.val,"U1") /* AND you are same as me */
#else
            0
#endif /* UNIX */
             ) || (yy->lprotect.len < 0) /* OR by inheritance from old file */
            ) {
            flag = 1;
            s = yy->lprotect.val;       /* UNIX filemode */
            xx = yy->lprotect.len;
            if (xx < 0)                 /* len < 0 means inheritance */
              xx = 0 - xx;
            for (i = 0; i < xx; i++) {  /* Decode octal string */
                if (*s <= '7' && *s >= '0') {
                    x = 8 * x + (int)(*s) - '0';
                } else {
                    flag = 0;
                    break;
                }
                s++;
            }
#ifdef DEBUG
            sprintf(obuf,"%o",x);
            debug(F110,"zstime octal lperm",obuf,0);
#endif /* DEBUG */
        } else if (!flag && yy->gprotect.len > 0) {
            int g;
#ifdef CK_SCO32V4
            mode_t mask;
#else
            int mask;
#endif /* CK_SCO32V4 */
            mask = umask(0);            /* Get umask */
            debug(F101,"zstime mask 1","",mask);
            umask(mask);                /* Put it back */
            mask ^= 0777;               /* Flip the bits */
            debug(F101,"zstime mask 2","",mask);
            g = xunchar(*(yy->gprotect.val)); /* Decode generic protection */
            debug(F101,"zstime gprotect","",g);
#ifdef S_IRUSR
            debug(F100,"zstime S_IRUSR","",0);
            if (g & 1) x |= S_IRUSR;    /* Read permission */
            flag = 1;
#endif /* S_IRUSR */
#ifdef S_IWUSR
            debug(F100,"zstime S_IWUSR","",0);
            if (g & 2) x |= S_IWUSR;    /* Write permission */
            if (g & 16) x |= S_IWUSR;   /* Delete permission */
            flag = 1;
#endif /* S_IWUSR */
#ifdef S_IXUSR
            debug(F100,"zstime S_IXUSR","",0);
            if (g & 4)                  /* Has execute permission bit */
              x |= S_IXUSR;
            else                        /* Doesn't have it */
              mask &= 0666;             /* so also clear it out of mask */
            flag = 1;
#endif /* S_IXUSR */
            debug(F101,"zstime mask x","",x);
            x |= mask;
            debug(F101,"zstime mask x|mask","",x);
        }
        debug(F101,"zstime flag","",flag);
        if (flag) {
#ifdef S_IFMT
            debug(F101,"zstime S_IFMT x","",x);
            sb.st_mode = (sb.st_mode & S_IFMT) | x;
            setperms = 1;
#else
#ifdef _IFMT
            debug(F101,"zstime _IFMT x","",x);
            sb.st_mode = (sb.st_mode & _IFMT) | x;
            setperms = 1;
#endif /* _IFMT */
#endif /* S_IFMT */
        }
#ifdef DEBUG
        sprintf(obuf,"%04o",sb.st_mode);
        debug(F111,"zstime file perms after",obuf,setperms);
#endif /* DEBUG */
    }
#endif /* CK_PERMS */

    debug(F101,"zstime: sb.st_atime","",sb.st_atime);

#ifdef BSD44
    tp[0].tv_sec = sb.st_atime;         /* Access time first */
    tp[1].tv_sec = tm;                  /* Update time second */
    debug(F100,"zstime: BSD44 modtime","",0);
#else
#ifdef V7
    tp.timep[0] = tm;                   /* Set modif. time to creation date */
    tp.timep[1] = sb.st_atime;          /* Don't change the access time */
    debug(F100,"zstime: V7 modtime","",0);
#else
#ifdef SYSUTIMEH
    tp.modtime = tm;                    /* Set modif. time to creation date */
    tp.actime = sb.st_atime;            /* Don't change the access time */
    debug(F100,"zstime: SYSUTIMEH modtime","",0);
#else
    tp.mtime = tm;                      /* Set modif. time to creation date */
    tp.atime = sb.st_atime;             /* Don't change the access time */
    debug(F100,"zstime: default modtime","",0);
#endif /* SYSUTIMEH */
#endif /* V7 */
#endif /* BSD44 */

    switch (x) {                        /* Execute desired function */
      case 0:                           /* Set the creation date of the file */
#ifdef CK_PERMS                         /* And permissions */
/*
  NOTE: If we are inheriting permissions from a previous file, and the
  previous file was a directory, this would turn the new file into a directory
  too, but it's not, so we try to unset the right bit.  Luckily, this code
  will probably never be executed since the upper level modules do not allow
  reception of a file that has the same name as a directory.

  NOTE 2: We change the permissions *before* we change the modification time,
  otherwise changing the permissions would set the mod time to the present
  time.
*/
        {
            int x;
            debug(F101,"zstime setperms","",setperms);
            if (S_ISDIR(sb.st_mode)) {
                debug(F101,"zstime DIRECTORY bit on","",sb.st_mode);
                sb.st_mode ^= 0040000;
                debug(F101,"zstime DIRECTORY bit off","",sb.st_mode);
            }
            if (setperms) {
                x = chmod(f,sb.st_mode);
                debug(F101,"zstime chmod","",x);
            }
        }
        if (x < 0) return(-1);
#endif /* CK_PERMS */

        if (!setdate)                   /* We don't have a date */
          return(0);                    /* so skip the following... */

        if (
#ifdef BSD44
            utimes(f,tp)
#else
            utime(f,&tp)
#endif /* BSD44 */
            ) {                         /* Fix modification time */
            debug(F111,"zstime 0: can't set modtime for file",f,errno);
            r = -1;
        } else  {
	    /* Including the modtime here is not portable */
            debug(F110,"zstime 0: modtime set for file",f,0);
            r = 0;
        }
        break;

      case 1:                           /* Compare the dates */
/*
  This was st_atime, which was wrong.  We want the file-data modification
  time, st_mtime.
*/
        debug(F111,"zstime 1: compare",f,sb.st_mtime);
        debug(F111,"zstime 1: compare","packet",tm);

        r = (sb.st_mtime < tm) ? 0 : 1;
        break;

      default:                          /* Error */
        r = -1;
    }
#endif /* TIMESTAMP */
    return(r);
}

/* Find initialization file. */

#ifdef NOTUSED
int
zkermini() {
/*  nothing here for Unix.  This function added for benefit of VMS Kermit.  */
    return(0);
}
#endif /* NOTUSED */

#ifndef UNIX
/* Historical -- not used in Unix any more (2001-11-03) */
#ifndef NOFRILLS
int
zmail(p,f) char *p; char *f; {          /* Send file f as mail to address p */
/*
  Returns 0 on success
   2 if mail delivered but temp file can't be deleted
  -2 if mail can't be delivered
  -1 on file access error
  The UNIX version always returns 0 because it can't get a good return
  code from zsyscmd.
*/
    int n;

#ifdef CK_LOGIN
    if (isguest)
      return(-2);
#endif /* CK_LOGIN */

    if (!f) f = "";
    if (!*f) return(-1);

#ifdef CKROOT
    debug(F111,"zmail setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(f)) {
	debug(F110,"zmail setroot violation",f,0);
	return(-1);
    }
#endif /* CKROOT */

#ifdef BSD4
/* The idea is to use /usr/ucb/mail, rather than regular mail, so that   */
/* a subject line can be included with -s.  Since we can't depend on the */
/* user's path, we use the convention that /usr/ucb/Mail = /usr/ucb/mail */
/* and even if Mail has been moved to somewhere else, this should still  */
/* find it...  The search could be made more reliable by actually using  */
/* access() to see if /usr/ucb/Mail exists. */

    n = strlen(f);
    n = n + n + 15 + (int)strlen(p);

    if (n > ZMBUFLEN)
      return(-2);

#ifdef DGUX540
    sprintf(zmbuf,"mailx -s %c%s%c %s < %s", '"', f, '"', p, f);
#else
    sprintf(zmbuf,"Mail -s %c%s%c %s < %s", '"', f, '"', p, f);
#endif /* DGUX540 */
    zsyscmd(zmbuf);
#else
#ifdef SVORPOSIX
#ifndef OXOS
    sprintf(zmbuf,"mail %s < %s", p, f);
#else /* OXOS */
    sprintf(zmbuf,"mailx -s %c%s%c %s < %s", '"', f, '"', p, f);
#endif /* OXOS */
    zsyscmd(zmbuf);
#else
    *zmbuf = '\0';
#endif
#endif
    return(0);
}
#endif /* NOFRILLS */
#endif /* UNIX */

#ifndef NOFRILLS
int
zprint(p,f) char *p; char *f; {         /* Print file f with options p */
    extern char * printername;          /* From ckuus3.c */
    extern int printpipe;
    int n;

#ifdef CK_LOGIN
    if (isguest)
      return(-2);
#endif /* CK_LOGIN */

    if (!f) f = "";
    if (!*f) return(-1);

#ifdef CKROOT
    debug(F111,"zprint setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(f)) {
	debug(F110,"zprint setroot violation",f,0);
	return(-1);
    }
#endif /* CKROOT */

    debug(F110,"zprint file",f,0);
    debug(F110,"zprint flags",p,0);
    debug(F110,"zprint printername",printername,0);
    debug(F101,"zprint printpipe","",printpipe);

#ifdef UNIX
/*
  Note use of standard input redirection.  In some systems, lp[r] runs
  setuid to lp (or ...?), so if user has sent a file into a directory
  that lp does not have read access to, it can't be printed unless it is
  fed to lp[r] as standard input.
*/
    if (printpipe && printername) {
	n = 8 + (int)strlen(f) + (int)strlen(printername);
	if (n > ZMBUFLEN)
	  return(-2);
        sprintf(zmbuf,"cat %s | %s", f, printername);
    } else if (printername) {
	n = 8 + (int)strlen(f) + (int)strlen(printername);
	if (n > ZMBUFLEN)
	  return(-2);
        sprintf(zmbuf,"cat %s >> %s", f, printername);
    } else {
	n = 4 + (int)strlen(PRINTCMD) + (int)strlen(p) + (int)strlen(f);
	if (n > ZMBUFLEN)
	  return(-2);
        sprintf(zmbuf,"%s %s < %s", PRINTCMD, p, f);
    }
    debug(F110,"zprint command",zmbuf,0);
    zsyscmd(zmbuf);
#else /* Not UNIX */
    *zmbuf = '\0';
#endif /* UNIX */
    return(0);
}
#endif /* NOFRILLS */

/*  Wildcard expansion functions...  */

static char scratch[MAXPATH+4];         /* Used by both methods */

static int oldmtchs = 0;                /* Let shell (ls) expand them. */
#ifdef COMMENT
static char *lscmd = "/bin/ls -d";      /* Command to use. */
#else
static char *lscmd = "echo";            /* Command to use. */
#endif /* COMMENT */

#ifndef NOPUSH
int
shxpand(pat,namlst,len) char *pat, *namlst[]; int len; {
    char *fgbuf = NULL;                 /* Buffer for forming ls command */
    char *p, *q;                        /* Workers */

    int i, x, retcode, itsadir;
    char c;

    x = (int)strlen(pat) + (int)strlen(lscmd) + 3; /* Length of ls command */
    for (i = 0; i < oldmtchs; i++) {    /* Free previous file list */
        if (namlst[i] ) {               /* If memory is allocated  */
            free(namlst[i]);            /* Free the memory         */
            namlst[i] = NULL ;          /* Remember no memory is allocated */
        }
    }
    oldmtchs = 0 ;                      /* Remember there are no matches */
    fgbuf = malloc(x);                  /* Get buffer for command */
    if (!fgbuf) return(-1);             /* Fail if cannot */
    ckmakmsg(fgbuf,x,lscmd," ",pat,NULL); /* Form the command */
    zxcmd(ZIFILE,fgbuf);                /* Start the command */
    i = 0;                              /* File counter */
    p = scratch;                        /* Point to scratch area */
    retcode = -1;                       /* Assume failure */
    while ((x = zminchar()) != -1) {    /* Read characters from command */
        c = (char) x;
        if (c == ' ' || c == '\n') {    /* Got newline or space? */
            *p = '\0';                  /* Yes, terminate string */
            p = scratch;                /* Point back to beginning */
            if (zchki(p) == -1)         /* Does file exist? */
              continue;                 /* No, continue */
            itsadir = isdir(p);         /* Yes, is it a directory? */
            if (xdironly && !itsadir)   /* Want only dirs but this isn't */
              continue;                 /* so skip. */
            if (xfilonly && itsadir)    /* It's a dir but want only files */
              continue;                 /* so skip. */
            x = (int)strlen(p);         /* Keep - get length of name */
            q = malloc(x+1);            /* Allocate space for it */
            if (!q) goto shxfin;        /* Fail if space can't be obtained */
            strcpy(q,scratch);          /* (safe) Copy name to space */
            namlst[i++] = q;            /* Copy pointer to name into array */
            if (i >= len) goto shxfin;  /* Fail if too many */
        } else {                        /* Regular character */
            *p++ = c;                   /* Copy it into scratch area */
        }
    }
    retcode = i;                        /* Return number of matching files */
shxfin:                                 /* Common exit point */
    free(fgbuf);                        /* Free command buffer */
    fgbuf = NULL;
    zclosf(ZIFILE);                     /* Delete the command fork. */
    oldmtchs = i;                       /* Remember how many files */
    return(retcode);
}
#endif /* NOPUSH */

/*
  Directory-reading functions for UNIX originally written for C-Kermit 4.0
  by Jeff Damens, CUCCA, 1984.
*/
static char * xpat = NULL;              /* Global copy of fgen() pattern */
static char * xpatlast = NULL;          /* Rightmost segment of pattern*/
static int xpatslash = 0;               /* Slash count in pattern */
static int xpatwild = 0;                /* Original pattern is wild */
static int xleafwild = 0;               /* Last segment of pattern is wild */
static int xpatabsolute = 0;

#ifdef aegis
static char bslash;
#endif /* aegis */


/*  S P L I T P A T H  */

/*
  Splits the slash-separated portions of the argument string into
  a list of path structures.  Returns the head of the list.  The
  structures are allocated by malloc, so they must be freed.
  Splitpath is used internally by the filename generator.

  Input:
    A path string.

  Returns:
    A linked list of the slash-separated segments of the input.
*/
static struct path *
splitpath(p) char *p; {
    struct path *head,*cur,*prv;
    int i;

    debug(F111,"splitpath",p,xrecursive);
    head = prv = NULL;

    if (!p) return(NULL);
    if (!*p) return(NULL);

    if (!strcmp(p,"**")) {              /* Fix this */
        p = "*";
    }
    if (ISDIRSEP(*p)) p++;              /* Skip leading slash if any */

    /* Make linked list of path segments from pattern */

    while (*p) {
        cur = (struct path *) malloc(sizeof (struct path));
        /* debug(F101,"splitpath malloc","",cur); */
        if (cur == NULL) {
            debug(F100,"splitpath malloc failure","",0);
            prv -> fwd = NULL;
            return((struct path *)NULL);
        }
        cur -> fwd = NULL;
        if (head == NULL)               /* First, make list head */
          head = cur;
        else                            /* Not first, link into chain */
          prv -> fwd = cur;
        prv = cur;                      /* Link from previous to this one */

#ifdef aegis
        /* treat backslash as "../" */
        if (bslash && *p == bslash) {
            strcpy(cur->npart, "..");	/* safe */
            ++p;
        } else {
            for (i=0; i < MAXNAMLEN && *p && *p != '/' && *p != bslash; i++)
              cur -> npart[i] = *p++;
            cur -> npart[i] = '\0';     /* end this segment */
            if (i >= MAXNAMLEN)
              while (*p && *p != '/' && *p != bslash)
                p++;
        }
        if (*p == '/') p++;
#else
        /* General case (UNIX) */
        for (i = 0; i < MAXNAMLEN && !ISDIRSEP(*p) && *p != '\0'; i++) {
            cur -> npart[i] = *p++;
        }

        cur -> npart[i] = '\0';         /* End this path segment */
        if (i >= MAXNAMLEN)
          while (!ISDIRSEP(*p) && *p != '\0') p++;
        if (ISDIRSEP(*p))
          p++;

#endif /* aegis */
    }
    if (prv) {
        makestr(&xpatlast,prv -> npart);
        debug(F110,"splitpath xpatlast",xpatlast,0);
    }
#ifdef DEBUG
    /* Show original path list */
    if (deblog) {
        for (i = 0, cur = head; cur; i++) {
            debug(F111,"SPLITPATH",cur -> npart, i);
            cur = cur -> fwd;
        }
    }
#endif /* DEBUG */
    return(head);
}

/*  F G E N  --  Generate File List  */

/*
  File name generator.  It is passed a string, possibly containing wildcards,
  and an array of character pointers.  It finds all the matching filenames and
  stores pointers to them in the array.  The returned strings are allocated
  from a static buffer local to this module (so the caller doesn't have to
  worry about deallocating them); this means that successive calls to fgen
  will wipe out the results of previous calls.

  Input:
    A wildcard string, an array to write names to, the length of the array.

  Returns:
    The number of matches.
    The array is filled with filenames that matched the pattern.
    If there wasn't enough room in the array, -1 is returned.

  Originally by: Jeff Damens, CUCCA, 1984.  Many changes since then.
*/
static int
fgen(pat,resarry,len) char *pat,*resarry[]; int len; {
    struct path *head;
    char *sptr, *s;
    int n;

#ifdef aegis
    char *namechars;
    int tilde = 0, bquote = 0;

    if ((namechars = getenv("NAMECHARS")) != NULL) {
        if (ckstrchr(namechars, '~' ) != NULL) tilde  = '~';
        if (ckstrchr(namechars, '\\') != NULL) bslash = '\\';
        if (ckstrchr(namechars, '`' ) != NULL) bquote = '`';
    } else {
        tilde = '~'; bslash = '\\'; bquote = '`';
    }
    sptr = scratch;

    /* copy "`node_data", etc. anchors */
    if (bquote && *pat == bquote)
      while (*pat && *pat != '/' && *pat != bslash)
        *sptr++ = *pat++;
    else if (tilde && *pat == tilde)
      *sptr++ = *pat++;
    while (*pat == '/')
      *sptr++ = *pat++;
    if (sptr == scratch) {
        strcpy(scratch,"./");		/* safe */
        sptr = scratch+2;
    }
    if (!(head = splitpath(pat))) return(-1);

#else /* not aegis */

    debug(F111,"fgen pat",pat,len);
    debug(F110,"fgen current directory",zgtdir(),0);
    debug(F101,"fgen stathack","",stathack);

    scratch[0] = '\0';
    xpatwild = 0;
    xleafwild = 0;
    xpatabsolute = 0;

    if (!(head = splitpath(pat)))       /* Make the path segment list */
	return(-1);

    sptr = scratch;

#ifdef COMMENT
    if (strncmp(pat,"./",2) && strncmp(pat,"../",3)) {
#endif /* COMMENT */
	if (!ISDIRSEP(*pat))		/* If name is not absolute */
	  *sptr++ = '.';		/* put "./" in front. */
	*sptr++ = DIRSEP;
#ifdef COMMENT
    }
#endif /* COMMENT */
    *sptr = '\0';
#endif /* aegis */

    makestr(&xpat,pat);                 /* Save copy of original pattern */
    debug(F110,"fgen scratch",scratch,0);

    for (n = 0, s = xpat; *s; s++)      /* How many slashes in the pattern */
      if (*s == DIRSEP)                 /* since these are fences for */
        n++;                            /* pattern matching */
    xpatslash = n;
    debug(F101,"fgen xpatslash","",xpatslash);

    numfnd = 0;                         /* None found yet */

    if (initspace(resarry,ssplen) < 0)
      return(-1);

    xpatwild = iswild(xpat);		/* Original pattern is wild? */
    xpatabsolute = isabsolute(xpat);
    xleafwild = iswild(xpatlast);

    debug(F111,"fgen xpat",xpat,xpatwild);
    debug(F111,"fgen xpatlast",xpatlast,xleafwild);
    debug(F101,"fgen xpatabsolute","",xpatabsolute);

    traverse(head,scratch,sptr);        /* Go walk the directory tree. */
    while (head != NULL) {              /* Done - free path segment list. */
        struct path *next = head -> fwd;
        free((char *)head);
        head = next;
    }
    debug(F101,"fgen","",numfnd);
    return(numfnd);                     /* Return the number of matches */
}

/* Define LONGFN (long file names) automatically for BSD 2.9 and 4.2 */
/* LONGFN can also be defined on the cc command line. */

#ifdef BSD29
#ifndef LONGFN
#define LONGFN
#endif
#endif

#ifdef BSD42
#ifndef LONGFN
#define LONGFN
#endif
#endif

/*
   T R A V E R S E  --  Traverse a directory tree.

   Walks the directory tree looking for matches to its arguments.
   The algorithm is, briefly:

    If the current pattern segment contains no wildcards, that
    segment is added to what we already have.  If the name so far
    exists, we call ourselves recursively with the next segment
    in the pattern string; otherwise, we just return.

    If the current pattern segment contains wildcards, we open the name
    we've accumulated so far (assuming it is really a directory), then read
    each filename in it, and, if it matches the wildcard pattern segment, add
    that filename to what we have so far and call ourselves recursively on
    the next segment.

    Finally, when no more pattern segments remain, we add what's accumulated
    so far to the result array and increment the number of matches.

  Inputs:
    A pattern path list (as generated by splitpath), a string pointer that
    points to what we've traversed so far (this can be initialized to "/"
    to start the search at the root directory, or to "./" to start the
    search at the current directory), and a string pointer to the end of
    the string in the previous argument, plus the global "recursive",
    "xmatchdot", and "xdironly" flags.

  Returns: void, with:
    mtchs[] containing the array of filename string pointers, and:
    numfnd containing the number of filenames.

  Although it might be poor practice, the mtchs[] array is revealed to the
  outside in case it needs it; for example, to be sorted prior to use.
  (It is poor practice because not all platforms implement file lists the
  same way; some don't use an array at all.)

  Note that addresult() acts as a second-level filter; due to selection
  criteria outside of the pattern, it might decline to add files that
  this routine asks it to, e.g. because we are collecting only directory
  names but not the names of regular files.

  WARNING: In the course of C-Kermit 7.0 development, this routine became
  ridiculously complex, in order to meet approximately sixty specific
  requirements.  DON'T EVEN THINK ABOUT MODIFYING THIS ROUTINE!  Trust me;
  it is not possible to fix anything in it without breaking something else.
  This routine badly needs a total redesign and rewrite.  Note: There may
  be some good applications for realpath() and/or scandir() and/or fts_blah()
  here, on platforms where they are available.
*/
static VOID
traverse(pl,sofar,endcur) struct path *pl; char *sofar, *endcur; {

/* Appropriate declarations for directory routines and structures */
/* #define OPENDIR means to use opendir(), readdir(), closedir()  */
/* If OPENDIR not defined, we use open(), read(), close() */

#ifdef DIRENT                           /* New way, <dirent.h> */
#define OPENDIR
    DIR *fd, *opendir();
    struct dirent *dirbuf;
    struct dirent *readdir();
#else /* !DIRENT */
#ifdef LONGFN                           /* Old way, <dir.h> with opendir() */
#define OPENDIR
    DIR *fd, *opendir();
    struct direct *dirbuf;
#else /* !LONGFN */
    int fd;                             /* Old way, <dir.h> with open() */
    struct direct dir_entry;
    struct direct *dirbuf = &dir_entry;
#endif /* LONGFN */
#endif /* DIRENT */
    int mopts = 0;			/* ckmatch() opts */
    int depth = 0;			/* Directory tree depth */

    char nambuf[MAXNAMLEN+4];           /* Buffer for a filename */
    int itsadir = 0, segisdir = 0, itswild = 0, mresult, n, x /* , y */ ;
    struct stat statbuf;                /* For file info. */

    debug(F101,"STAT","",16);
    if (pl == NULL) {                   /* End of path-segment list */
        *--endcur = '\0'; /* Terminate string, overwrite trailing slash */
        debug(F110,"traverse add: end of path segment",sofar,0);
        addresult(sofar,-1);
        return;
    }
    if (stathack) {
	/* This speeds up the search a lot and we still get good results */
	/* but it breaks the tagging of directory names done in addresult */
	if (xrecursive || xfilonly || xdironly || xpatslash) {
	    itsadir = xisdir(sofar);
	    debug(F101,"STAT","",17);
	} else
	  itsadir = (strncmp(sofar,"./",2) == 0);
    } else {
	itsadir = xisdir(sofar);
	debug(F101,"STAT","",18);
    }
    debug(F111,"traverse entry sofar",sofar,itsadir);

#ifdef CKSYMLINK                        /* We're doing symlinks? */
#ifdef USE_LSTAT                        /* OK to use lstat()? */
    if (itsadir && xnolinks) {		/* If not following symlinks */
	int x;
	struct stat buf;
	x = lstat(sofar,&buf);
	debug(F111,"traverse lstat 1",sofar,x);
	if (x > -1 &&
#ifdef S_ISLNK
	    S_ISLNK(buf.st_mode)
#else
#ifdef _IFLNK
	    ((_IFMT & buf.st_mode) == _IFLNK)
#endif /* _IFLNK */
#endif /* S_ISLNK */
	    )
	  itsadir = 0;
    }
#endif /* USE_LSTAT */
#endif /* CKSYMLINK */

    if (!xmatchdot && xpatlast[0] == '.')
      xmatchdot = 1;
    if (!xmatchdot && xpat[0] == '.' && xpat[1] != '/' && xpat[1] != '.')
      xmatchdot = 1;

    /* ckmatch() options */

    if (xmatchdot)   mopts |= 1;	/* Match dot */
    if (!xrecursive) mopts |= 2;	/* Dirsep is fence */

    debug(F111,"traverse entry xpat",xpat,xpatslash);
    debug(F111,"traverse entry xpatlast",xpatlast,xmatchdot);
    debug(F110,"traverse entry pl -> npart",pl -> npart,0);

#ifdef RECURSIVE
    if (xrecursive > 0 && !itsadir) {
        char * s;         /* Recursive descent and this is a regular file */
        *--endcur = '\0'; /* Terminate string, overwrite trailing slash */

        /* Find the nth slash from the right and match from there... */
        /* (n == the number of slashes in the original pattern - see fgen) */
        if (*sofar == '/') {
            debug(F110,"traverse xpatslash absolute",sofar,0);
            s = sofar;
        } else {
            debug(F111,"traverse xpatslash relative",sofar,xpatslash);
            for (s = endcur - 1, n = 0; s >= sofar; s--) {
                if (*s == '/') {
                    if (++n >= xpatslash) {
                        s++;
                        break;
                    }
                }
            }
        }
#ifndef NOSKIPMATCH
	/* This speeds things up a bit. */
	/* If it causes trouble define NOSKIPMATCH and rebuild. */
	if (xpat[0] == '*' && !xpat[1])
	  x = xmatchdot ? 1 : (s[0] != '.');
	else
#endif /* NOSKIPMATCH */
	  x = ckmatch(xpat, s, 1, mopts); /* Match with original pattern */
        debug(F111,"traverse xpatslash ckmatch",s,x);
        if (x > 0) {
            debug(F110,"traverse add: recursive, match, && !isdir",sofar,0);
            addresult(sofar,itsadir);
        }
        return;
    }
#endif /* RECURSIVE */

    debug(F111,"traverse sofar 2",sofar,0);

    segisdir = ((pl -> fwd) == NULL) ? 0 : 1;
    itswild = wildena ? (iswild(pl -> npart)) : 0; /* 15 Jun 2005 */

    debug(F111,"traverse segisdir",sofar,segisdir);
    debug(F111,"traverse itswild ",pl -> npart,itswild);

#ifdef RECURSIVE
    if (xrecursive > 0) {               /* If recursing and... */
        if (segisdir && itswild)        /* this is a dir and npart is wild */
          goto blah;                    /* or... */
        else if (!xpatabsolute && !xpatwild) /* search object is nonwild */
          goto blah;                    /* then go recurse */
    }
#endif /* RECURSIVE */

    if (!itswild) {                     /* This path segment not wild? */
#ifdef COMMENT
        strcpy(endcur,pl -> npart);     /* (safe) Append next part. */
        endcur += (int)strlen(pl -> npart); /* Advance end pointer */
#else
/*
  strcpy() does not account for quoted metacharacters.
  We must remove the quotes before doing the stat().
*/
	{
	    int quote = 0;
	    char c, * s;
	    s = pl -> npart;
	    while ((c = *s++)) {
		if (!quote) {
		    if (c == CMDQ) {
			quote = 1;
			continue;
		    }
		}
		*endcur++ = c;
		quote = 0;
	    }
	}
#endif /* COMMENT */
        *endcur = '\0';                 /* End new current string. */

        if (stat(sofar,&statbuf) == 0) { /* If this piece exists... */
            debug(F110,"traverse exists",sofar,0);
            *endcur++ = DIRSEP;         /* add slash to end */
            *endcur = '\0';             /* and end the string again. */
            traverse(pl -> fwd, sofar, endcur);
        }
#ifdef DEBUG
        else debug(F110,"traverse not found", sofar, 0);
#endif /* DEBUG */
        return;
    }

    *endcur = '\0';                     /* End current string */
    debug(F111,"traverse sofar 3",sofar,0);

    if (!itsadir)
      return;

    /* Search is recursive or ... */
    /* path segment contains wildcards, have to open and search directory. */

  blah:

    debug(F110,"traverse opening directory", sofar, 0);

#ifdef OPENDIR
    debug(F110,"traverse opendir()",sofar,0);
    if ((fd = opendir(sofar)) == NULL) {        /* Can't open, fail. */
        debug(F101,"traverse opendir() failed","",errno);
        return;
    }
    while ((dirbuf = readdir(fd)))
#else /* !OPENDIR */
    debug(F110,"traverse directory open()",sofar,0);
    if ((fd = open(sofar,O_RDONLY)) < 0) {
        debug(F101,"traverse directory open() failed","",errno);
        return;
    }
    while (read(fd, (char *)dirbuf, sizeof dir_entry) > 0)
#endif /* OPENDIR */
      {                         /* Read each entry in this directory */
          int exists;
          char *eos, *s;
          exists = 0;

          /* On some platforms, the read[dir]() can return deleted files, */
          /* e.g. HP-UX 5.00.  There is no point in grinding through this */
          /* routine when the file doesn't exist... */

          if (          /* There  actually is an inode... */
#ifdef BSD42
                         dirbuf->d_ino != -1
#else
#ifdef unos
                         dirbuf->d_ino != -1
#else
#ifdef QNX
                         dirbuf->d_stat.st_ino != 0
#else
#ifdef SOLARIS
                         dirbuf->d_ino != 0
#else
#ifdef sun
                         dirbuf->d_fileno != 0
#else
#ifdef bsdi
                         dirbuf->d_fileno != 0
#else
#ifdef __386BSD__
                         dirbuf->d_fileno != 0
#else
#ifdef __FreeBSD__
                         dirbuf->d_fileno != 0
#else
#ifdef ultrix
                         dirbuf->gd_ino != 0
#else
#ifdef Plan9
                         1
#else
                         dirbuf->d_ino != 0
#endif /* Plan9 */
#endif /* ultrix */
#endif /* __FreeBSD__ */
#endif /* __386BSD__ */
#endif /* bsdi */
#endif /* sun */
#endif /* SOLARIS */
#endif /* QNX */
#endif /* unos */
#endif /* BSD42 */
              )
            exists = 1;
          if (!exists)
            continue;

          ckstrncpy(nambuf,             /* Copy the name */
                  dirbuf->d_name,
                  MAXNAMLEN
                  );
          if (nambuf[0] == '.') {
              if (!nambuf[1] || (nambuf[1] == '.' && !nambuf[2])) {
                  debug(F110,"traverse skipping",nambuf,0);
                  continue;             /* skip "." and ".." */
              }
          }
          s = nambuf;                   /* Copy name to end of sofar */
          eos = endcur;
          while ((*eos = *s)) {
              s++;
              eos++;
          }
/*
  Now we check the file for (a) whether it is a directory, and (b) whether
  its name matches our pattern.  If it is a directory, and if we have been
  told to build a recursive list, then we must descend regardless of whether
  it matches the pattern.  If it is not a directory and it does not match
  our pattern, we skip it.  Note: sofar is the full pathname, nambuf is
  the name only.
*/
          /* Do this first to save pointless function calls */
          if (nambuf[0] == '.' && !xmatchdot) /* Dir name starts with '.' */
            continue;
	  if (stathack) {
	      if (xrecursive || xfilonly || xdironly || xpatslash) {
		  itsadir = xisdir(sofar); /* See if it's a directory */
		  debug(F101,"STAT","",19);
	      } else {
		  itsadir = 0;
	      }
	  } else {
	      itsadir = xisdir(sofar);
	      debug(F101,"STAT","",20);
	  }

#ifdef CKSYMLINK
#ifdef USE_LSTAT
	  if (itsadir && xnolinks) {		/* If not following symlinks */
	      int x;
	      struct stat buf;
	      x = lstat(sofar,&buf);
	      debug(F111,"traverse lstat 2",sofar,x);
	      if (x > -1 &&
#ifdef S_ISLNK
		  S_ISLNK(buf.st_mode)
#else
#ifdef _IFLNK
		  ((_IFMT & buf.st_mode) == _IFLNK)
#endif /* _IFLNK */
#endif /* S_ISLNK */
		  )
		itsadir = 0;
	  }
#endif /* USE_LSTAT */
#endif /* CKSYMLINK */

#ifdef RECURSIVE
          if (xrecursive > 0 && itsadir &&
              (xpatlast[0] == '*') && !xpatlast[1]
              ) {
              debug(F110,
                    "traverse add: recursive && isdir && segisdir or match",
                    sofar,
                    segisdir
                    );
	      addresult(sofar,itsadir);
	      if (numfnd < 0) return;
          }
#endif /* RECURSIVE */

          debug(F111,"traverse mresult xpat",xpat,xrecursive);
          debug(F111,"traverse mresult pl -> npart",
                pl -> npart,
                ((pl -> fwd) ? 9999 : 0)
                );
          debug(F111,"traverse mresult sofar segisdir",sofar,segisdir);
          debug(F111,"traverse mresult sofar itsadir",sofar,itsadir);
          debug(F101,"traverse mresult xmatchdot","",xmatchdot);
/*
  Match the path so far with the pattern after stripping any leading "./"
  from either or both.  The pattern chosen is the full original pattern if
  the match candidate (sofar) is not a directory, or else just the name part
  (pl->npart) if it is.
*/
	  {
	      char * s1;		/* The pattern */
	      char * s2 = sofar;	/* The path so far */
	      char * s3;		/* Worker */
	      int opts;			/* Match options */

	      s1 = itsadir ? pl->npart : xpat;

#ifndef COMMENT
	      /* I can't explain this but it unbreaks "cd blah/sub<Esc>" */
	      if (itsadir && !xrecursive && xpatslash > 0 &&
		  segisdir == 0 && itswild) {
		  s1 = xpat;
		  debug(F110,"traverse mresult s1 kludge",s1,0);
	      }
#endif /* COMMENT */

	      if (xrecursive && xpatslash == 0)
		s2 = nambuf;
	      while ((s1[0] == '.') && (s1[1] == '/')) /* Strip "./" */
		s1 += 2;
	      while ((s2[0] == '.') && (s2[1] == '/')) /* Ditto */
		s2 += 2;
	      opts = mopts;		/* Match options */
	      if (itsadir) 		/* Current segment is a directory */
		opts = mopts & 1;	/* No fences */
	      s3 = s2;			/* Get segment depth */
	      depth = 0;
	      while (*s3) { if (*s3++ == '/') depth++; }
#ifndef NOSKIPMATCH
	      /* This speeds things up a bit. */
	      /* If it causes trouble define NOSKIPMATCH and rebuild. */
	      if (depth == 0 && (s1[0] == '*') && !s1[1])
		mresult = xmatchdot ? 1 : (s2[0] != '.');
	      else
#endif /* NOSKIPMATCH */
		mresult = ckmatch(s1,s2,1,opts); /* Match */
	  }
#ifdef DEBUG
	  if (deblog) {
	      debug(F111,"traverse mresult depth",sofar,depth);
	      debug(F101,"traverse mresult xpatslash","",xpatslash);
	      debug(F111,"traverse mresult nambuf",nambuf,mresult);
	      debug(F111,"traverse mresult itswild",pl -> npart,itswild);
	      debug(F111,"traverse mresult segisdir",pl -> npart,segisdir);
	  }
#endif /* DEBUG */
          if (mresult ||		/* If match succeeded */
	      xrecursive ||		/* Or search is recursive */
	      depth < xpatslash		/* Or not deep enough to match... */
	      ) {
              if (                      /* If it's not a directory... */
/*
  The problem here is that segisdir is apparently not set appropriately.
  If I leave in the !segisdir test, then "dir /recursive blah" (where blah is
  a directory name) misses some regular files because sometimes segisdir
  is set and sometimes it's not.  But if I comment it out, then
  "dir <star>/<star>.txt lists every file in * and does not even open up the
  subdirectories.  However, "dir /rec <star>/<star>.txt" works right.
*/
#ifdef COMMENT
                  mresult && (!itsadir && !segisdir)
#else
                  mresult &&		/* Matched */
                  !itsadir &&		/* sofar is not a directory */
                  ((!xrecursive && !segisdir) || xrecursive)
#endif /* COMMENT */
                  ) {
		  debug(F110,
			"traverse add: match && !itsadir",sofar,0);
		  addresult(sofar,itsadir);
		  if (numfnd < 0) return;
              } else if (itsadir && (xrecursive || mresult)) {
                  struct path * xx = NULL;
                  *eos++ = DIRSEP;      /* Add directory separator */
                  *eos = '\0';          /* to end of segment */
#ifdef RECURSIVE
                  /* Copy previous pattern segment to this new directory */

                  if (xrecursive > 0 && !(pl -> fwd)) {
                      xx = (struct path *) malloc(sizeof (struct path));
                      pl -> fwd = xx;
                      if (xx) {
                          xx -> fwd = NULL;
                          strcpy(xx -> npart, pl -> npart); /* safe */
                      }
                  }
#endif /* RECURSIVE */
                  traverse(pl -> fwd, sofar, eos); /* Traverse new directory */
              }
          }
      }
#ifdef OPENDIR
    closedir(fd);
#else /* !OPENDIR */
    close(fd);
#endif /* OPENDIR */
}

/*
 * addresult:
 *  Adds a result string to the result array.  Increments the number
 *  of matches found, copies the found string into our string
 *  buffer, and puts a pointer to the buffer into the caller's result
 *  array.  Our free buffer pointer is updated.  If there is no
 *  more room in the caller's array, the number of matches is set to -1.
 * Input: a result string.
 * Returns: nothing.
 */
static VOID
addresult(str,itsadir) char *str; int itsadir; {
    int len;

    if (!freeptr) {
	debug(F100,"addresult string space not init'd","",0);
	initspace(mtchs,ssplen);
    }
    if (!str) str = "";
    debug(F111,"addresult",str,itsadir);
    if (!*str)
      return;

    if (itsadir < 0) {
	itsadir = xisdir(str);
    }
    if ((xdironly && !itsadir) || (xfilonly && itsadir)) {
        debug(F111,"addresult skip",str,itsadir);
        return;
    }
    while (str[0] == '.' && ISDIRSEP(str[1])) /* Strip all "./" from front */
      str += 2;
    if (--remlen < 0) {                 /* Elements left in array of names */
        debug(F111,"addresult ARRAY FULL",str,numfnd);
        numfnd = -1;
        return;
    }
    len = (int)strlen(str);		/* Space this will use */
    debug(F111,"addresult len",str,len);

    if (len < 1)
      return;

    if ((freeptr + len + itsadir + 1) > (sspace + ssplen)) {
        debug(F111,"addresult OUT OF SPACE",str,numfnd);
#ifdef DYNAMIC
	printf(
"?String space %d exhausted - use SET FILE STRINGSPACE to increase\n",ssplen);
#else
	printf("?String space %d exhausted\n",ssplen);
#endif /* DYNAMIC */
        numfnd = -1;                    /* Do not record if not enough space */
        return;
    }
    strcpy(freeptr,str);		/* safe */

    /* Tag directory names by putting '/' at the end */

    if (itsadir && (freeptr[len-1] == '/')) {
        freeptr[len++] = DIRSEP;
        freeptr[len] = '\0';
    }
    if (numfnd >= maxnames) {
#ifdef DYNAMIC
	printf(
"?Too many files (%d max) - use SET FILE LISTSIZE to increase\n",maxnames);
#else
	printf("?Too many files - %d max\n",maxnames);
#endif /* DYNAMIC */
        numfnd = -1;
        return;
    }
    str = freeptr;
    *resptr++ = freeptr;
    freeptr += (len + 1);
    numfnd++;
    debug(F111,"addresult ADD",str,numfnd);
}

#ifdef COMMENT
/*
 * match(pattern,string):
 *  pattern matcher.  Takes a string and a pattern possibly containing
 *  the wildcard characters '*' and '?'.  Returns true if the pattern
 *  matches the string, false otherwise.
 * Orignally by: Jeff Damens, CUCCA, 1984
 * No longer used as of C-Kermit 7.0, now we use ckmatch() instead (ckclib.c).
 *
 * Input: a string and a wildcard pattern.
 * Returns: 1 if match, 0 if no match.
 */
static int
match(pattern, string) char *pattern, *string; {
    char *psave = NULL, *ssave = NULL;  /* Backup pointers for failure */
    int q = 0;                          /* Quote flag */

    if (*string == '.' && *pattern != '.' && !xmatchdot) {
        debug(F110,"match skip",string,0);
        return(0);
    }
    while (1) {
        for (; *pattern == *string; pattern++,string++) /* Skip first */
          if (*string == '\0') return(1); /* End of strings, succeed */

        if (*pattern == '\\' && q == 0) { /* Watch out for quoted */
            q = 1;                      /* metacharacters */
            pattern++;                  /* advance past quote */
            if (*pattern != *string) return(0);
            continue;
        } else q = 0;

        if (q) {
            return(0);
        } else {
            if (*string != '\0' && *pattern == '?') {
                pattern++;              /* '?', let it match */
                string++;
            } else if (*pattern == '*') { /* '*' ... */
                psave = ++pattern;      /* remember where we saw it */
                ssave = string;         /* let it match 0 chars */
            } else if (ssave != NULL && *ssave != '\0') { /* if not at end  */
                                        /* ...have seen a star */
                string = ++ssave;       /* skip 1 char from string */
                pattern = psave;        /* and back up pattern */
            } else return(0);           /* otherwise just fail */
        }
    }
}
#endif /* COMMENT */

/*
  The following two functions are for expanding tilde in filenames
  Contributed by Howie Kaye, CUCCA, developed for CCMD package.
*/

/*  W H O A M I  --  Get user's username.  */

/*
  1) Get real uid
  2) See if the $USER environment variable is set ($LOGNAME on AT&T)
  3) If $USER's uid is the same as ruid, realname is $USER
  4) Otherwise get logged in user's name
  5) If that name has the same uid as the real uid realname is loginname
  6) Otherwise, get a name for ruid from /etc/passwd
*/
char *
whoami() {
#ifdef DTILDE
#ifdef pdp11
#define WHOLEN 100
#else
#define WHOLEN 257
#endif /* pdp11 */
    static char realname[UIDBUFLEN+1];  /* user's name */
    static int ruid = -1;               /* user's real uid */
    char loginname[UIDBUFLEN+1], envname[256]; /* temp storage */
    char *c;
    struct passwd *p;
    _PROTOTYP(extern char * getlogin, (void) );

    debug(F111,"whoami ruid A",realname,ruid);

    if (ruid != -1)
      return(realname);

    ruid = real_uid();                  /* get our uid */
    debug(F101,"whoami ruid B","",ruid);
    if (ruid < 0) ruid = getuid();
    debug(F101,"whoami ruid C","",ruid);

  /* how about $USER or $LOGNAME? */
    if ((c = getenv(NAMEENV)) != NULL) { /* check the env variable */
        ckstrncpy(envname, c, 255);
	debug(F110,"whoami envname",envname,0);
        if ((p = getpwnam(envname)) != NULL) {
            if (p->pw_uid == ruid) {    /* get passwd entry for envname */
                ckstrncpy(realname, envname, UIDBUFLEN); /* uid's are same */
		debug(F110,"whoami realname",realname,0);
                return(realname);
            }
        }
    }

  /* can we use loginname() ? */

    if ((c =  getlogin()) != NULL) {    /* name from utmp file */
        ckstrncpy (loginname, c, UIDBUFLEN);
	debug(F110,"whoami loginname",loginname,0); 
        if ((p = getpwnam(loginname)) != NULL) /* get passwd entry */
          if (p->pw_uid == ruid)        /* for loginname */
            ckstrncpy(realname, envname, UIDBUFLEN); /* if uid's are same */
    }

  /* Use first name we get for ruid */

    if ((p = getpwuid(ruid)) == NULL) { /* name for uid */
	debug(F101,"whoami no username for ruid","",ruid); 
        realname[0] = '\0';             /* no user name */
        ruid = -1;
        return(NULL);
    }
    ckstrncpy(realname, p->pw_name, UIDBUFLEN);
    debug(F110,"whoami realname from getpwuid",realname,0);
    return(realname);
#else
    return(NULL);
#endif /* DTILDE */
}

/*  T I L D E _ E X P A N D  --  expand ~user to the user's home directory. */

char *
tilde_expand(dirname) char *dirname; {
#ifdef DTILDE
#ifdef pdp11
#define BUFLEN 100
#else
#define BUFLEN 257
#endif /* pdp11 */
    struct passwd *user;
    static char olddir[BUFLEN+1];
    static char oldrealdir[BUFLEN+1];
    static char temp[BUFLEN+1];
    int i, j;

    debug(F111,"tilde_expand",dirname,dirname[0]);

    if (dirname[0] != '~') {              /* Not a tilde...return param */
	debug(F000,"tilde_expand NOT TILDE","",dirname[0]);
	return(dirname);
    }
    if (!strcmp(olddir,dirname)) {      /* Same as last time */
	debug(F110,"tilde_expand same as previous",oldrealdir,0);
	return(oldrealdir);               /* so return old answer. */
    } else {
	debug(F110,"tilde_expand working...","",0);
        j = (int)strlen(dirname);
        for (i = 0; i < j; i++)         /* find username part of string */
          if (!ISDIRSEP(dirname[i]))
            temp[i] = dirname[i];
          else break;
        temp[i] = '\0';                 /* tie off with a NULL */
	debug(F111,"tilde_expand first part",temp,i);
        if (i == 1) {                   /* if just a "~" */
#ifdef IKSD
            if (inserver)
              user = getpwnam(uidbuf);  /* Get info on current user */
            else
#endif /* IKSD */
            {
                char * p = whoami();
		debug(F110,"tilde_expand p",p,0);
                if (p) {
		    user = getpwnam(p);
		    debug(F110,"tilde_expand getpwpam ~",user,0);
		} else {
		    user = NULL;
		}
            }
        } else {
	    debug(F110,"tilde_expand ~user",&temp[1],0);
            user = getpwnam(&temp[1]);  /* otherwise on the specified user */
	    debug(F110,"tilde_expand getpwpam user",user,0);
        }

    }
    if (user != NULL) {                 /* valid user? */
        ckstrncpy(olddir, dirname, BUFLEN); /* remember the directory */
        ckstrncpy(oldrealdir,user->pw_dir, BUFLEN); /* and home directory */
        ckstrncat(oldrealdir,&dirname[i], BUFLEN);
        oldrealdir[BUFLEN] = '\0';
        return(oldrealdir);
    } else {                            /* invalid? */
        ckstrncpy(olddir, dirname, BUFLEN); /* remember for next time */
        ckstrncpy(oldrealdir, dirname, BUFLEN);
        return(oldrealdir);
    }
#else
    return(NULL);
#endif /* DTILDE */
}

/*
  Functions for executing system commands.
  zsyscmd() executes the system command in the normal, default way for
  the system.  In UNIX, it does what system() does.  Thus, its results
  are always predictable.
  zshcmd() executes the command using the user's preferred shell.
*/
int
zsyscmd(s) char *s; {
#ifdef aegis
    if (nopush) return(-1);
    if (!priv_chk()) return(system(s));
#else
    PID_T shpid;
#ifdef COMMENT
/* This doesn't work... */
    WAIT_T status;
#else
    int status;
#endif /* COMMENT */

    if (nopush) return(-1);
    if ((shpid = fork())) {
        if (shpid < (PID_T)0) return(-1); /* Parent */
        while (shpid != (PID_T) wait(&status))
         ;
        return(status);
    }
    if (priv_can()) {                   /* Child: cancel any priv's */
        printf("?Privilege cancellation failure\n");
        _exit(255);
    }
    restorsigs();			/* Restore ignored signals */
#ifdef HPUX10
    execl("/usr/bin/sh","sh","-c",s,NULL);
    perror("/usr/bin/sh");
#else
#ifdef Plan9
    execl("/bin/rc", "rc", "-c", s, NULL);
    perror("/bin/rc");
#else
    execl("/bin/sh","sh","-c",s,NULL);
    perror("/bin/sh");
#endif /* Plan9 */
#endif /* HPUX10 */
    _exit(255);
    return(0);                          /* Shut up ANSI compilers. */
#endif /* aegis */
}


/*  Z _ E X E C  --  Overlay ourselves with another program  */

#ifndef NOZEXEC
#ifdef HPUX5
#define NOZEXEC
#else
#ifdef ATT7300
#define NOZEXEC
#endif /* ATT7300 */
#endif /* HPUX5 */
#endif /* NOZEXEC */

VOID
z_exec(p,s,t) char * p, ** s; int t; {  /* Overlay ourselves with "p s..." */
#ifdef NOZEXEC
    printf("EXEC /REDIRECT NOT IMPLEMENTED IN THIS VERSION OF C-KERMIT\n");
    debug(F110,"z_exec NOT IMPLEMENTED",p,0);
#else
    int x;
    extern int ttyfd;
    debug(F110,"z_exec command",p,0);
    debug(F110,"z_exec arg 0",s[0],0);
    debug(F110,"z_exec arg 1",s[1],0);
    debug(F101,"z_exec t","",t);
    errno = 0;
    if (t) {
        if (ttyfd > 2) {
            dup2(ttyfd, 0);
            dup2(ttyfd, 1);
            /* dup2(ttyfd, 2); */
            close(ttyfd);
        }
    }
    restorsigs();			/* Restore ignored signals */
    x = execvp(p,s);
    if (x < 0) debug(F101,"z_exec errno","",errno);
#endif /* NOZEXEC */
}

/*
  Z S H C M D  --  Execute a shell command (or program thru the shell).

  Original UNIX code by H. Fischer; copyright rights assigned to Columbia U.
  Adapted to use getpwuid to find login shell because many systems do not
  have SHELL in environment, and to use direct calling of shell rather
  than intermediate system() call. -- H. Fischer (1985); many changes since
  then.  Call with s pointing to command to execute.  Returns:
   -1 on failure to start the command (can't find, can't fork, can't run).
    1 if command ran and gave an exit status of 0.
    0 if command ran and gave a nonzero exit status.
  with pexitstatus containing the command's exit status.
*/
int
zshcmd(s) char *s; {
    PID_T pid;

#ifdef NOPUSH
    return(0);
#else
    if (nopush) return(-1);
    if (!s) return(-1);
    while (*s == ' ') s++;

    debug(F110,"zshcmd command",s,0);

#ifdef aegis
    if ((pid = vfork()) == 0) {         /* Make child quickly */
        char *shpath, *shname, *shptr;  /* For finding desired shell */

        if (priv_can()) exit(1);        /* Turn off privs. */
        if ((shpath = getenv("SHELL")) == NULL) shpath = "/com/sh";

#else                                   /* All Unix systems */
    if ((pid = fork()) == 0) {          /* Make child */
        char *shpath, *shname, *shptr;  /* For finding desired shell */
        struct passwd *p;
#ifdef HPUX10                           /* Default */
        char *defshell = "/usr/bin/sh";
#else
#ifdef Plan9
        char *defshell = "/bin/rc";
#else
        char *defshell = "/bin/sh";
#endif /* Plan9 */
#endif /* HPUX10 */
        if (priv_can()) exit(1);        /* Turn off privs. */
#ifdef COMMENT
/* Old way always used /etc/passwd shell */
        p = getpwuid(real_uid());       /* Get login data */
        if (p == (struct passwd *) NULL || !*(p->pw_shell))
          shpath = defshell;
        else
          shpath = p->pw_shell;
#else
/* New way lets user override with SHELL variable, but does not rely on it. */
/* This allows user to specify a different shell. */
        shpath = getenv("SHELL");       /* What shell? */
	debug(F110,"zshcmd SHELL",shpath,0);
	{
	    int x = 0;
	    if (!shpath) {
		x++;
	    } else if (!*shpath) {
		x++;
	    }
	    if (x) {
		debug(F100,"zshcmd SHELL not defined","",0);
		p = getpwuid( real_uid() ); /* Get login data */
		if (p == (struct passwd *)NULL || !*(p->pw_shell)) {
		    shpath = defshell;
		} else {
		    shpath = p->pw_shell;
		}
		debug(F110,"zshcmd shpath from getpwuid",shpath,0);
	    }
        }
#endif /* COMMENT */
#endif /* aegis */
        shptr = shname = shpath;
        while (*shptr != '\0')
          if (*shptr++ == DIRSEP)
            shname = shptr;
	restorsigs();			/* Restore ignored signals */
	debug(F110,"zshcmd execl shpath",shpath,0);
	debug(F110,"zshcmd execl shname",shname,0);
        if (s == NULL || *s == '\0') {  /* Interactive shell requested? */
	    debug(F100,"zshcmd execl interactive","",0);
            execl(shpath,shname,"-i",NULL); /* Yes, do that */
        } else {                        /* Otherwise, */
	    debug(F110,"zshcmd execl command",s,0);
            execl(shpath,shname,"-c",s,NULL); /* exec the given command */
        }                               /* If execl() failed, */
        debug(F101,"zshcmd errno","",errno);
	perror(shpath);			/* print reason and */
        exit(BAD_EXIT);                 /* return bad return code. */

    } else {                            /* Parent */

        int wstat;                      /* ... must wait for child */
#ifdef CK_CHILD
        int child;                      /* Child's exit status */
#endif /* CK_CHILD */
        SIGTYP (*istat)(), (*qstat)();

        if (pid == (PID_T) -1) return(-1); /* fork() failed? */

        istat = signal(SIGINT,SIG_IGN); /* Let the fork handle keyboard */
        qstat = signal(SIGQUIT,SIG_IGN); /* interrupts itself... */

	debug(F110,"zshcmd parent waiting for child",s,0);
#ifdef CK_CHILD
        while (((wstat = wait(&child)) != pid) && (wstat != -1))
#else
        while (((wstat = wait((WAIT_T *)0)) != pid) && (wstat != -1))
#endif /* CK_CHILD */
          ;                             /* Wait for fork */
        signal(SIGINT,istat);           /* Restore interrupts */
        signal(SIGQUIT,qstat);
#ifdef CK_CHILD
        pexitstat = (child & 0xff) ? child : child >> 8;
	debug(F101,"zshcmd exit status","",pexitstat);
        return(child == 0 ? 1 : 0);     /* Return child's status */
#endif /* CK_CHILD */
    }
    return(1);
#endif /* NOPUSH */
}

/*  I S W I L D  --  Check if filespec is "wild"  */

/*
  Returns:
    0 wildcards disabled or argument is empty or is the name of a single file;
    1 if it contains wildcard characters.
  Note: must match the algorithm used by match(), hence no [a-z], etc.
*/
int
iswild(filespec) char *filespec; {
    char c, *p, *f; int x;
    int quo = 0;
    if (!filespec)			/* Safety */
      return(0);
    if (!wildena)			/* Wildcards disabled - 12 Jun 2005 */
      return(0);
    f = filespec;
    if (wildxpand) {			/* Shell handles wildcarding */
        if ((x = nzxpand(filespec,0)) > 1)
          return(1);
        if (x == 0) return(0);          /* File does not exist */
        p = malloc(MAXNAMLEN + 20);
        znext(p);
        x = (strcmp(filespec,p) != 0);
        free(p);
        p = NULL;
        return(x);
    } else {				/* We do it ourselves */
        while ((c = *filespec++) != '\0') {
            if (c == '\\' && quo == 0) {
                quo = 1;
                continue;
            }
            if (!quo && (c == '*' || c == '?'
#ifdef CKREGEX
#ifndef VMS
                         || c == '['
#endif /* VMS */
			 || c == '{'
#endif /* CKREGEX */
                         )) {
		debug(F111,"iswild",f,1);
		return(1);
	    }
            quo = 0;
        }
	debug(F111,"iswild",f,0);
        return(0);
    }
}

/*
  I S D I R  --  Is a Directory.

  Tell if string pointer s is the name of an existing directory.  Returns 1 if
  directory, 0 if not a directory.

  The following no longer applies:

  If the file is a symlink, we return 1 if
  it is a directory OR if it is a link to a directory and the "xrecursive" flag
  is NOT set.  This is to allow parsing a link to a directory as if it were a
  directory (e.g. in the CD or IF DIRECTORY command) but still prevent
  recursive traversal from visiting the same directory twice.
*/

#ifdef ISDIRCACHE
/* This turns out to be unsafe and gives little benefit anyway. */
/* See notes 28 Sep 2003.  Thus ISDIRCACHE is not defined. */

static char prevpath[CKMAXPATH+4] = { '\0', '\0' };
static int prevstat = -1;
int
clrdircache() {
    debug(F100,"CLEAR ISDIR CACHE","",0);
    prevstat = -1;
    prevpath[0] = NUL;
}
#endif /* ISDIRCACHE */

int
isalink(s) char *s; {
#ifndef CKSYMLINK
    return(0);
#else
    int r = 0;
    char filbuf[CKMAXPATH+4];
    if (readlink(s,filbuf,CKMAXPATH) > -1)
      r = 1;
    debug(F110,"isalink readlink",s,r);
    return(r);
#endif	/* CKSYMLINK */
}

int
isdir(s) char *s; {
    int x, needrlink = 0, islink = 0;
    struct stat statbuf;
    char fnam[CKMAXPATH+4];

    if (!s) return(0);
    debug(F110,"isdir entry",s,0);
#ifdef DTILDE				/* 2005-08-13 */
    if (*s == '~') {			/* Starts with tilde? */
        s = tilde_expand(s);		/* Attempt to expand tilde */
        if (!s) s = "";
	debug(F110,"isdir tilde_expand",s,0);
    }
#endif /* DTILDE */
    if (!*s) return(0);

#ifdef ISDIRCACHE
    if (prevstat > -1) {
	if (s[0] == prevpath[0]) {
	    if (!strcmp(s,prevpath)) {
		debug(F111,"isdir cache hit",s,prevstat);
		return(prevstat);
	    }
	}
    }
#endif /* ISDIRCACHE */

#ifdef CKSYMLINK
#ifdef COMMENT
/*
  The following over-clever bit has been commented out because it presumes
  to know when a symlink might be redundant, which it can't possibly know.
  Using plain old stat() gives Kermit the same results as ls and ls -R, which
  is just fine: no surprises.
*/
#ifdef USE_LSTAT
    if (xrecursive) {
        x = lstat(s,&statbuf);
        debug(F111,"isdir lstat",s,x);
    } else {
#endif /* USE_LSTAT */
        x = stat(s,&statbuf);
        debug(F111,"isdir stat",s,x);
#ifdef USE_LSTAT
    }
#endif /* USE_LSTAT */
#else
    x = stat(s,&statbuf);
    debug(F111,"isdir stat",s,x);
#endif /* COMMENT */
    if (x == -1) {
        debug(F101,"isdir errno","",errno);
        return(0);
    }
    islink = 0;
    if (xrecursive) {
#ifdef NOLINKBITS
        needrlink = 1;
#else
#ifdef S_ISLNK
        islink = S_ISLNK(statbuf.st_mode);
        debug(F101,"isdir S_ISLNK islink","",islink);
#else
#ifdef _IFLNK
        islink = (_IFMT & statbuf.st_mode) == _IFLNK;
        debug(F101,"isdir _IFLNK islink","",islink);
#endif /* _IFLNK */
#endif /* S_ISLNK */
#endif /* NOLINKBITS */
        if (needrlink) {
            if (readlink(s,fnam,CKMAXPATH) > -1)
              islink = 1;
        }
    }
#else
    x = stat(s,&statbuf);
    if (x == -1) {
        debug(F101,"isdir errno","",errno);
        return(0);
    }
    debug(F111,"isdir stat",s,x);
#endif /* CKSYMLINK */
    debug(F101,"isdir islink","",islink);
    debug(F101,"isdir statbuf.st_mode","",statbuf.st_mode);
    x = islink ? 0 : (S_ISDIR (statbuf.st_mode) ? 1 : 0);
#ifdef ISDIRCACHE
    prevstat = x;
    ckstrncpy(prevpath,s,CKMAXPATH+1);
#endif /* ISDIRCACHE */
    return(x);
}

#ifdef CK_MKDIR
/* Some systems don't have mkdir(), e.g. Tandy Xenix 3.2.. */

/* Z M K D I R  --  Create directory(s) if necessary */
/*
   Call with:
    A pointer to a file specification that might contain directory
    information.  The filename is expected to be included.
    If the file specification does not include any directory separators,
    then it is assumed to be a plain file.
    If one or more directories are included in the file specification,
    this routine tries to create them if they don't already exist.
   Returns:
    0 or greater on success, i.e. the number of directories created.
   -1 on failure to create the directory
*/
int
zmkdir(path) char *path; {
    char *xp, *tp, c;
    int x, count = 0;

    if (!path) path = "";
    if (!*path) return(-1);

#ifdef CKROOT
    debug(F111,"zmkdir setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(path)) {
	debug(F110,"zmkdir setroot violation",path,0);
	return(-1);
    }
#endif /* CKROOT */

    x = strlen(path);
    debug(F111,"zmkdir",path,x);
    if (x < 1 || x > MAXPATH)           /* Check length */
      return(-1);
    if (!(tp = malloc(x+1)))            /* Make a temporary copy */
      return(-1);
    strcpy(tp,path);			/* safe (prechecked) */
#ifdef DTILDE
    if (*tp == '~') {                   /* Starts with tilde? */
        xp = tilde_expand(tp);          /* Attempt to expand tilde */
        if (!xp) xp = "";
        if (*xp) {
            char *zp;
            debug(F110,"zmkdir tilde_expand",xp,0);
            if (!(zp = malloc(strlen(xp) + 1))) { /* Make a place for it */
                free(tp);
                tp = NULL;
                return(-1);
            }
            free(tp);                   /* Free previous buffer */
            tp = zp;                    /* Point to new one */
            strcpy(tp,xp);              /* Copy expanded name to new buffer */
        }
    }
#endif /* DTILDE */
    debug(F110,"zmkdir tp after tilde_expansion",tp,0);
    xp = tp;
    if (ISDIRSEP(*xp))                  /* Don't create root directory! */
      xp++;

    /* Go thru filespec from left to right... */

    for (; *xp; xp++) {                 /* Create parts that don't exist */
        if (!ISDIRSEP(*xp))             /* Find next directory separator */
          continue;
        c = *xp;                        /* Got one. */
        *xp = NUL;                      /* Make this the end of the string. */
        if (!isdir(tp)) {               /* This directory exists already? */
#ifdef CK_LOGIN
            if (isguest)                    /* Not allowed for guests */
	      return(-1);
#ifndef NOXFER
            /* Nor if MKDIR and/or CD are disabled */
            else
#endif /* CK_LOGIN */
	      if ((server
#ifdef IKSD
		   || inserver
#endif /* IKSD */
		   ) && (!ENABLED(en_mkd) || !ENABLED(en_cwd)))
		return(-1);
#endif /* IKSD */

            debug(F110,"zmkdir making",tp,0);
            x =                         /* No, try to create it */
#ifdef NOMKDIR
               -1                       /* Systems without mkdir() */
#else
               mkdir(tp,0777)           /* UNIX */
#endif /* NOMKDIR */
                 ;
            if (x < 0) {
                debug(F101,"zmkdir failed, errno","",errno);
                free(tp);               /* Free temporary buffer. */
                tp = NULL;
                return(-1);             /* Return failure code. */
            } else
              count++;
        }
        *xp = c;                        /* Replace the separator. */
    }
    free(tp);                           /* Free temporary buffer. */
    return(count);                      /* Return success code. */
}
#endif /* CK_MKDIR */

int
zrmdir(path) char *path; {
#ifdef CK_LOGIN
    if (isguest)
      return(-1);
#endif /* CK_LOGIN */

    if (!path) path = "";
    if (!*path) return(-1);

#ifdef CKROOT
    debug(F111,"zrmdir setroot",ckroot,ckrootset);
    if (ckrootset) if (!zinroot(path)) {
	debug(F110,"zrmdir setroot violation",path,0);
	return(-1);
    }
#endif /* CKROOT */

#ifndef NOMKDIR
    return(rmdir(path));
#else
    return(-1);
#endif /* NOMKDIR */
}

/* Z F S E E K  --  Position input file pointer */
/*
   Call with:
    CK_OFF_T (32 or 64 bits), 0-based, indicating desired position.
   Returns:
    0 on success.
   -1 on failure.
*/
#ifndef NORESEND
int
#ifdef CK_ANSIC
zfseek(CK_OFF_T pos)
#else
zfseek(pos) CK_OFF_T pos;
#endif /* CK_ANSIC */
/* zfseek */ {
    zincnt = -1;                        /* Must empty the input buffer */
    debug(F101,"zfseek","",pos);
    return(CKFSEEK(fp[ZIFILE], pos, 0)?-1:0);
}
#endif /* NORESEND */

/*  Z F N Q F P  --  Convert filename to fully qualified absolute pathname */

/*
  Given a possibly unqualified or relative file specification fn, zfnqfp()
  returns the fully qualified filespec for the same file, returning a struct
  that contains the length (len) of the result, a pointer (fpath) to the
  whole result, and a pointer (fname) to where the filename starts.
*/
static struct zfnfp fnfp = { 0, NULL, NULL };

struct zfnfp *
zfnqfp(fname, buflen, buf)  char * fname; int buflen; char * buf; {
    char * s;
    int len;
#ifdef MAXPATHLEN
    char zfntmp[MAXPATHLEN+4];
#else
    char zfntmp[CKMAXPATH+4];
#endif /* MAXPATHLEN */

    char sb[32], * tmp;
    int i = 0, j = 0, k = 0, x = 0, y = 0;
    int itsadir = 0;

    s = fname;
    if (!s)
      return(NULL);
    if (!*s)
      return(NULL);
    if (!buf)
      return(NULL);

    /* Initialize the data structure */

    fnfp.len = ckstrncpy(buf,fname,buflen);
    fnfp.fpath = buf;
    fnfp.fname = NULL;
    len = buflen;
    debug(F111,"zfnqfp fname",fname,len);

#ifdef DTILDE
    if (*s == '~') {                    /* Starts with tilde? */
        char * xp;
        xp = tilde_expand(s);           /* Attempt to expand tilde */
        debug(F110,"zfnqfp xp",xp,0);   /* (realpath() doesn't do this) */
        if (!xp) xp = "";
        if (*xp)
          s = xp;
    }
#endif /* DTILDE */

#ifdef CKREALPATH

/* N.B.: The realpath() result buffer MUST be MAXPATHLEN bytes long */
/* otherwise we write over memory. */

    if (!realpath(s,zfntmp)) {
        debug(F111,"zfnqfp realpath fails",s,errno);
#ifdef COMMENT
	if (errno != ENOENT)
	  return(NULL);
#else
	/* If realpath() fails use the do-it-yourself method */
	/* 16 Jan 2002 */
	goto norealpath;
#endif /* COMMENT */
    }
    len = strlen(zfntmp);
    if (len > buflen) {
	debug(F111,"zfnqfp result too long",ckitoa(buflen),len);
	return(NULL);
    } else {
	ckstrncpy(buf,zfntmp,buflen);
    }
    if (buf[len-1] != '/') {
	if ((itsadir = isdir(buf)) && len < (buflen - 1)) {
	    buf[len++] = '/';
	    buf[len] = NUL;
	}
    }
    fnfp.len = len;
    fnfp.fpath = buf;
    debug(F110,"zfnqfp realpath path",fnfp.fpath,0);
    tmp = buf + fnfp.len - 1;
    if (!itsadir) {
	while (tmp >= buf) {
	    if (*tmp == '/') {
		fnfp.fname = tmp + 1;
		debug(F110,"zfnqfp realpath name",fnfp.fname,0);
		break;
	    }
	    tmp--;
	}
    }
    return(&fnfp);

#endif /* CKREALPATH */

  norealpath:

    tmp = zfntmp;
    while (*s) {                        /* Remove leading "./" (0 or more) */
        debug(F110,"zfnqfp while *s",s,0);
        if (*s == '.' && *(s+1) == '/') {
            s += 2;
            while (*s == '/') s++;
        } else
          break;
    }
    if (!*s) return(NULL);
    if (*s == '/') {                    /* Pathname is absolute */
        ckstrncpy(buf,s,len);
        x = strlen(buf);
        y = 0;
    } else {                            /* Pathname is relative */
        char * p;
        if (p = zgtdir()) {             /* So get current directory */
            debug(F110,"zfnqfp zgtdir",p,0);
            x = ckstrncpy(buf,p,len);
            buf[x++] = '/';
            debug(F110,"zfnqfp buf 1",buf,0);
            len -= x;                   /* How much room left in buffer */
            if ((y = (int)strlen(s)) > len) /* If enough room... */
              return(NULL);
            ckstrncpy(buf+x,s,len);     /* ... append the filename */
            debug(F110,"zfnqfp buf 2",buf,0);
        } else {
            return(NULL);
        }
    }

    /* Buf now holds full path but maybe containing some . or .. tricks */

    j = x + y;                          /* Length of what's in buf */
    len = j;
    debug(F101,"zfnqfp len","",len);

    /* Catch dangling "/." or "/.." */
    if ((j > 1 && buf[j-1] == '.' && buf[j-2] == '/') ||
        (j > 2 && buf[j-1] == '.' && buf[j-2] == '.' && buf[j-3] == '/')) {
        if (j < buflen - 2) {
            buf[j] = '/';
            buf[j+1] = NUL;
        }
    }
    j = -1;                             /* j = position of rightmost "/" */
    i = 0;                              /* i = destination index */
    tmp[i] = NUL;                       /* destination is temporary buffer  */

    for (x = 0; x < len; x++) {         /* x = source index */
        if (buf[x] == '/') {
            for (k = 0; k < 4; k++) {
                sb[k] = buf[x+k];
                sb[k+1] = '\0';
                if (!sb[k]) break;
            }
            if (!strncmp(sb,"/./",3)) { /* Eliminate "./" in "/./" */
                x += 1;
                continue;
            } else if (!strncmp(sb,"//",2)) { /* Change "//" to "/" */
                continue;
            } else if (!strncmp(sb,"/../",4)) { /* ".." in path */
                for (k = i - 1; k >= 0; k--) { /* Back up one level */
                    if (tmp[k] == '/') {
                        i = k;
                        tmp[i] = NUL;
                        break;
                    }
                }
                x += 2;
                continue;
            }
        }
        if (i >= (buflen - 1)) {
            debug(F111,"zfnqfp overflow",tmp,i);
            return(NULL);
        }
        tmp[i++] = buf[x];              /* Regular character, copy */
        tmp[i] = NUL;
        if (buf[x] == '/')              /* Remember rightmost "/" */
          j = i;
    }
    ckstrncpy(buf,tmp,buflen-1);        /* Copy the result back */

    buf[buflen-1] = NUL;
    if (!buf[0]) {                      /* If empty, say root */
        buf[0] = '/';
        buf[2] = NUL;
        j = 0;
        i = 1;
    }
    if ((itsadir = isdir(buf))) {
	if (buf[i-1] != '/' && i < (buflen - 1)) {
	    buf[i++] = '/';
	    buf[i] = NUL;
	}
    }
    if (!itsadir && (j > -1)) {		/* Set pointer to basename */
        fnfp.fname = (char *)(buf + j);
        fnfp.fpath = (char *)buf;
        fnfp.len = i;
        debug(F111,"zfnqfp path",fnfp.fpath,i);
        debug(F110,"zfnqfp name",fnfp.fname,0);
        return(&fnfp);
    }
    return(NULL);
}

/*  Z C M P F N  --  Compare two filenames  */

/*  Returns 1 if the two names refer to the same existing file, 0 otherwise. */

int
zcmpfn(s1,s2) char * s1, * s2; {
    char buf1[CKMAXPATH+1];
    char buf2[CKMAXPATH+1];

#ifdef USE_LSTAT
    char linkname[CKMAXPATH+1];
    struct stat buf;
#endif /* USE_LSTAT */
    int x, rc = 0;

    if (!s1) s1 = "";
    if (!s2) s2 = "";
    if (!*s1 || !*s2) return(0);

#ifdef CKSYMLINK                        /* We're doing symlinks? */
#ifdef USE_LSTAT                        /* OK to use lstat()? */
    x = lstat(s1,&buf);
    if (x > -1 &&			/* Now see if it's a symlink */
#ifdef S_ISLNK
        S_ISLNK(buf.st_mode)
#else
#ifdef _IFLNK
        ((_IFMT & buf.st_mode) == _IFLNK)
#endif /* _IFLNK */
#endif /* S_ISLNK */
        ) {
        linkname[0] = '\0';             /* Get the name */
        x = readlink(s1,linkname,CKMAXPATH);
        if (x > -1 && x < CKMAXPATH) {  /* It's a link */
            linkname[x] = '\0';
	    s1 = linkname;
        }
    }
#endif /* USE_LSTAT */
#endif /* CKSYMLINK */

    if (zfnqfp(s1,CKMAXPATH,buf1)) {	/* Convert to full pathname */

#ifdef CKSYMLINK			/* Same deal for second name... */
#ifdef USE_LSTAT
	x = lstat(s2,&buf);
	if (x > -1 &&
#ifdef S_ISLNK
	    S_ISLNK(buf.st_mode)
#else
#ifdef _IFLNK
	    ((_IFMT & buf.st_mode) == _IFLNK)
#endif /* _IFLNK */
#endif /* S_ISLNK */
	    ) {
	    linkname[0] = '\0';
	    x = readlink(s2,linkname,CKMAXPATH);
	    if (x > -1 && x < CKMAXPATH) {
		linkname[x] = '\0';
		s2 = linkname;
	    }
	}
#endif /* USE_LSTAT */
#endif /* CKSYMLINK */
	if (zfnqfp(s2,CKMAXPATH,buf2)) {
	    debug(F110,"zcmpfn s1",buf1,0);
	    debug(F110,"zcmpfn s2",buf2,0);
	    if (!strncmp(buf1,buf2,CKMAXPATH))
	      rc = 1;
	}
    }
    debug(F101,"zcmpfn result","",rc);
    return(rc);
}

#ifdef CKROOT

/* User-mode chroot() implementation */

int
zsetroot(s) char * s; {			/* Sets the root */
    char buf[CKMAXPATH+1];
    if (!s) return(-1);
    if (!*s) return(-1);
    debug(F110,"zsetroot",s,0);
    if (!isdir(s)) return(-2);
    if (!zfnqfp(s,CKMAXPATH,buf))	/* Get full, real path */
      return(-3);
    if (access(buf,R_OK) < 0) {		/* Check access */
	debug(F110,"zsetroot access denied",buf,0);
	return(-4);
    }
    s = buf;
    if (ckrootset) {			/* If root already set */
	if (!zinroot(s)) {		/* make sure new root is in it */
	    debug(F110,"zsetroot new root not in root",ckroot,0);
	    return(-5);
	}
    }
    if (zchdir(buf) < 1) return(-4);	/* Change directory to new root */
    ckrootset = ckstrncpy(ckroot,buf,CKMAXPATH); /* Now set the new root */
    if (ckroot[ckrootset-1] != '/') {
	ckroot[ckrootset++] = '/';
	ckroot[ckrootset] = '\0';
    }
    debug(F111,"zsetroot rootset",ckroot,ckrootset);
    ckrooterr = 0;			/* Reset error flag */
    return(1);
}

char *
zgetroot() {				/* Returns the root */
    if (!ckrootset)
      return(NULL);
    return((char *)ckroot);
}

int
zinroot(s) char * s; {			/* Checks if file s is in the root */
    int x, n;
    struct zfnfp * f = NULL;
    char buf[CKMAXPATH+2];

    debug(F111,"zinroot setroot",ckroot,ckrootset);
    ckrooterr = 0;			/* Reset global error flag */
    if (!ckrootset)			/* Root not set */
      return(1);			/* so it's ok - no need to check */
    if (!(f = zfnqfp(s,CKMAXPATH,buf)))	/* Get full and real pathname */
      return(0);			/* Fail if we can't  */
    n = f->len;				/* Length of full pathname */
    debug(F111,"zinroot n",buf,n);
    if (n < (ckrootset - 1) || n > CKMAXPATH) {	/* Bad length */
	ckrooterr = 1;			        /* Fail */
	return(0);
    }
    if (isdir(buf) && buf[n-1] != '/') {  /* If it's a directory name */
	buf[n++] = '/';			  /* make sure it ends with '/' */
	buf[n] = '\0';
    }
    x = strncmp(buf,ckroot,ckrootset);	/* Compare, case-sensitive */
    debug(F111,"zinroot checked",buf,x);
    if (x == 0)				/* OK */
      return(1);
    ckrooterr = 1;			/* Not OK */
    return(0);
}
#endif /* CKROOT */

#ifdef CK_LOGIN
/*
  The following code provides support for user login and logout
  including anonymous accounts.  If this feature is to be supported
  outside of UNIX, it should be spread out among the ck?fio.c modules...
*/
#ifndef _PATH_BSHELL
#define _PATH_BSHELL    "/usr/bin/bash"
#endif /* _PATH_BSHELL */
#ifndef _PATH_FTPUSERS
#define _PATH_FTPUSERS  "/etc/ftpusers"
#endif /* _PATH_FTPUSERS */

/*
 * Helper function for sgetpwnam().
 */
char *
sgetsave(s) char *s; {
    char *new = malloc((unsigned) strlen(s) + 1);
    if (new == NULL) {
        printf("?Local resource failure: malloc\n");
        exit(1);
        /* NOTREACHED */
    }
    (void) strcpy(new, s);		/* safe */
    return (new);
}

/*
 * Save the result of getpwnam().  Used for USER command, since
 * the data returned must not be clobbered by any other command
 * (e.g., globbing).
 */
struct passwd *
sgetpwnam(name) char *name; {
    static struct passwd save;
    register struct passwd *p;
#ifdef CK_SHADOW
    register struct spwd *sp;
#endif /* CK_SHADOW */
    char *sgetsave();

#ifdef HPUX10_TRUSTED
    struct pr_passwd *pr;
#endif /* HPUX10_TRUSTED */

#ifdef CK_SHADOW
    sp = getspnam(name);
    if (sp == NULL) {
        debug(F110,"sgetpwnam","getspnam() fails",0);
	return (NULL);
    }
#endif /* CK_SHADOW */

#ifdef HPUX10_TRUSTED
    if ((pr = getprpwnam(name)) == NULL)
      return(NULL);
#endif /* HPUX10_TRUSTED */

    p = getpwnam(name);
    /* debug(F111,"sgetpwnam","getpwnam()",p); */
    if (p == NULL)
      return(NULL);
    if (save.pw_name) {
        free(save.pw_name);
        free(save.pw_passwd);
        free(save.pw_gecos);
        free(save.pw_dir);
        free(save.pw_shell);
    }
    save = *p;
    save.pw_name = sgetsave(p->pw_name);
#ifdef CK_SHADOW
    save.pw_passwd = sgetsave(sp->sp_pwdp);
#else /* CK_SHADOW */
#ifdef HPUX10_TRUSTED
    if (pr->uflg.fg_encrypt && pr->ufld.fd_encrypt && *pr->ufld.fd_encrypt)
      save.pw_passwd = sgetsave(pr->ufld.fd_encrypt);
    else
      save.pw_passwd = sgetsave("");
#else /* HPUX10_TRUSTED */
    save.pw_passwd = sgetsave(p->pw_passwd);
#endif /* HPUX10_TRUSTED */
#endif /* CK_SHADOW */
    save.pw_gecos = sgetsave(p->pw_gecos);
    save.pw_dir = sgetsave(p->pw_dir);
    save.pw_shell = sgetsave(p->pw_shell);
    return(&save);
}

#define CKXLOGBSIZ 256

struct passwd * pw = NULL;
char * home = NULL;                     /* Home directory pointer for glob */
#ifdef CMASK
#undef CMASK
#endif /* CMASK */

#define CMASK 027

int defumask = CMASK;                   /* Default umask value */

/*  Z V U S E R  --  Verify user, Returns 1 if user OK, 0 otherwise.  */

/* Sets global passwd pointer pw if named account exists and is acceptable;
 * sets askpasswd if a PASS command is expected.  If logged in previously,
 * need to reset state.  If name is "ftp" or "anonymous", the name is not in
 * _PATH_FTPUSERS, and ftp account exists, set guest and pw, then just return.
 * If account doesn't exist, ask for passwd anyway.  Otherwise, check user
 * requesting login privileges.  Disallow anyone who does not have a standard
 * shell as returned by getusershell().  Disallow anyone mentioned in the file
 * _PATH_FTPUSERS to allow people such as root and uucp to be avoided.
 */
_PROTOTYP(static int checkuser, (char *) );

char zvuname[64] = { NUL, NUL };
char zvhome[CKMAXPATH+1] = { NUL, NUL };
#define ZENVUSER 70
#define ZENVHOME CKMAXPATH+12
#define ZENVLOGNAME 74
static char zenvuser[ZENVUSER];
static char zenvhome[ZENVHOME];
static char zenvlogname[ZENVLOGNAME];

#ifdef CK_PAM
static char pam_data[500];
struct pam_conv pam_conv = {pam_cb, pam_data}; /* PAM structure */
struct pam_handle * pamh = NULL;               /* PAM reference handle */
#endif /* CK_PAM */

int
zvuser(name) char *name; {
    register char *cp = NULL;
    int x;
    char *shell;
#ifdef GETUSERSHELL
_PROTOTYP(char * getusershell, (void) );
#endif /* GETUSERSHELL */
#ifndef NODCLENDUSERSHELL
_PROTOTYP(VOID endusershell, (void) );
#endif	/* NODCLENDUSERSHELL */

#ifdef CK_PAM
    int pam_status;
    const char * reply = NULL;
#endif /* CK_PAM */

    debug(F111,"user",name,logged_in);

    if (!name) name = "";
    zvuname[0] = NUL;

    debug(F101,"zvuser ckxsyslog","",ckxsyslog);

#ifdef CKSYSLOG
    debug(F100,"zvuser CKSYSLOG defined","",0);
#endif /* CKSYSLOG */

    if (logged_in)                      /* Should not be called if logged in */
      return(0);

#ifdef CKSYSLOG
    if (ckxsyslog && ckxlogging) {
        syslog(LOG_INFO,
                "login: user %s",name
                );
    }
#endif /* CKSYSLOG */

    guest = 0;                          /* Assume not guest */
    askpasswd = 0;

    if (strcmp(name, "ftp") == 0 || strcmp(name, "anonymous") == 0) {
        debug(F101,"zvuser anonymous ckxanon","",ckxanon);
        if (!ckxanon) {                 /* Anonymous login not allowed */
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                syslog(LOG_INFO,
                       "login: anonymous login not allowed: %s",
                       clienthost ? clienthost : "(unknown host)"
                       );
            }
#endif /* CKSYSLOG */
            return(0);
        }
        if (checkuser("ftp") || checkuser("anonymous")) {
            debug(F100,"zvuser anon forbidden by ftpusers file","",0);
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                syslog(LOG_INFO,
                       "login: anonymous login forbidden by ftpusers file: %s",
                       clienthost ? clienthost : "(unknown host)"
                       );
            }
#endif /* CKSYSLOG */
            return(0);
	} else if ((pw = sgetpwnam("ftp")) != NULL) {
            debug(F100,"zvuser anon sgetpwnam(ftp) OK","",0);
            guest = 1;
            askpasswd = 1;
            ckstrncpy(zvuname,"anonymous",64);
            return(1);
        } else {
            debug(F100,"zvuser anon sgetpwnam(ftp) FAILED","",0);
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                syslog(LOG_INFO,
                       "login: anonymous getpwnam(ftp) failed: %s",
                       clienthost ? clienthost : "(unknown host)"
                       );
            }
#endif /* CKSYSLOG */
            return(0);
        }
    }
    pw = sgetpwnam(name);
    if (pw) {
/*
  Of course some UNIX platforms (like AIX) don't have getusershell().
  In that case we can't check if the user's account has been "turned off"
  or somesuch, e.g. by setting their shell to "/etc/nologin" or somesuch,
  which runs (usually just printing a message and exiting), but which is
  not listed in /etc/shells.  For that matter, if getusershell() is not
  available, then probably neither is /etc/shells.
*/
        debug(F100,"zvuser sgetpwnam ok","",0);
        shell = pw->pw_shell;
        if (!shell) shell = "";
        if (!*shell)
          shell = _PATH_BSHELL;
        debug(F110,"zvuser shell",shell,0);
#ifdef GETUSERSHELL
        while ((cp = getusershell()) != NULL) {
            debug(F110,"zvuser getusershell",cp,0);
            if ((int)strcmp(cp, shell) == 0)
              break;
        }
        debug(F100,"zvuser endusershell 1","",0);
#ifndef NODCLENDUSERSHELL
        (VOID) endusershell();
#else
        endusershell();
#endif	/* NODCLENDUSERSHELL */
        debug(F100,"zvuser endusershell 2","",0);
#else /* GETUSERSHELL */
        cp = "";                        /* Do not refuse if we cannot check */
#endif /* GETUSERSHELL */
        x = checkuser(name);
        debug(F101,"zvuser checkuser","",x);
        if (cp == NULL) {
            debug(F100,"zvuser refused 1","",0);
            pw = (struct passwd *) NULL;
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                syslog(LOG_INFO,
                       "login: invalid shell %s for %s %s",shell, name,
                       clienthost ? clienthost : "(unknown host)"
                       );
            }
#endif /* CKSYSLOG */
            return(0);
        } else if (x) {
            debug(F100,"zvuser refused 2","",0);
            pw = (struct passwd *) NULL;
#ifdef CKSYSLOG
            if (ckxsyslog && ckxlogging) {
                syslog(LOG_INFO,
                       "login: %s login forbidden by ftpusers file: %s",
                       name, clienthost ? clienthost : "(unknown host)"
                       );
            }
#endif /* CKSYSLOG */
            return(0);
        } else {
            x = 0;
#ifdef CK_PAM
            /* Get PAM authentication details */
            debug(F110,"zvuser","calling pam_start",0);
            if ((pam_status =
                 pam_start(PAM_SERVICE_TYPE,name,&pam_conv,&pamh))
                != PAM_SUCCESS) {
                reply = pam_strerror(NULL, pam_status);
                debug(F110,"zvuser PAM failure",reply,0);
                printf("%s\n",reply);
#ifdef CKSYSLOG
                if (ckxsyslog && ckxlogging) {
                    syslog(LOG_INFO,
                           "login: %s refused by PAM \"%s\": %s",
                           name,reply,
                           clienthost ? clienthost : "(unknown host)"
                           );
                }
#endif /* CKSYSLOG */
                return(0);
            }
#endif /* CK_PAM */
            askpasswd = 1;
            ckstrncpy(zvuname,name,64);
            return(1);
        }
    } else {
        x = 0;
        debug(F100,"zvuser sgetpwnam NULL","",0);
#ifdef CKSYSLOG
        if (ckxsyslog && ckxlogging) {
            syslog(LOG_INFO,
                   "login: getpwnam(%s) failed: %s",name,
                   clienthost ? clienthost : "(unknown host)"
                   );
        }
#endif /* CKSYSLOG */
        return(0);
    }

#ifdef FTP_KERBEROS
    if (auth_type && strcmp(auth_type, "KERBEROS_V4") == 0) {
#ifdef COMMENT
	/* Why sprintf and then printf? */
	/* Also, what is kerb_ok?  And is the test on it right? */
        char buf[CKXLOGBSIZ];
        sprintf(buf, "Kerberos user %s%s%s@%s is%s authorized as %s%s",
                 kdata.pname, *kdata.pinst ? "." : "",
                 kdata.pinst, kdata.prealm,
                 (kerb_ok = kuserok(&kdata,name) == 0) ? "" : " not",
                 name, kerb_ok ? "" : "; Password required.");
        printf("%s", buf);
#else
        printf("Kerberos user %s%s%s@%s is%s authorized as %s%s",
                 kdata.pname, *kdata.pinst ? "." : "",
                 kdata.pinst, kdata.prealm,
                 (kerb_ok = kuserok(&kdata,name) == 0) ? "" : " not",
                 name, kerb_ok ? "" : "; Password required.");
#endif /* COMMENT */
        if (kerb_ok) return(1);
    } else
      return(0);
#endif /* FTP_KERBEROS */
}

/* Check if the given user is in the forbidden-user file */

static int
checkuser(name) char *name; {
    extern char * userfile;
    FILE *fd;
    int i;
    char line[CKXLOGBSIZ+1];

    if (!name)
      name = "";
    i = strlen(name);
    debug(F111,"checkuser name",name,i);
    if (!*name)
      return(1);

    fd = fopen(userfile ? userfile : _PATH_FTPUSERS, "r");
    /* debug(F111,"checkuser userfile",userfile,fd); */
    if (fd) {
        line[0] = '\0';
        while (fgets(line, sizeof(line), fd)) {
            debug(F110,"checkuser line",line,0);
            if (line[0] <= '#')
              continue;
            if (strncmp(line, name, i) == 0) {
                debug(F110,"checkuser REFUSED",name,0);
                return(1);
            }
            line[0] = '\0';
        }
        (VOID) fclose(fd);
    }
    debug(F110,"checkuser OK",name,0);
    return(0);
}

/*  Z V L O G O U T  --  Log out from Internet Kermit Service  */

VOID
zvlogout() {
#ifdef COMMENT
    /* This could be dangerous */
    if (setuid((UID_T)0) < 0) {
        debug(F100,"zvlogout setuid FAILED","",0);
        goto bad;
    }
    debug(F100,"zvlogout setuid OK","",0);
#endif /* COMMENT */
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_LI && ckxlogging) {
        cksyslog(SYSLG_LI, 1, "logout",(char *) uidbuf, clienthost);
    }
#endif /* CKSYSLOG */
#ifdef CKWTMP
    debug(F110,"WTMP logout",cksysline,logged_in);
    if (logged_in)
      logwtmp(cksysline, "", "");
#endif /* CKWTMP */
    pw = NULL;
    logged_in = 0;
    guest = 0;
    isguest = 0;
}

#ifdef FTP_KERBEROS
kpass(name, p) char *name, *p; {
    char instance[INST_SZ];
    char realm[REALM_SZ];
    char tkt_file[20];
    KTEXT_ST ticket;
    AUTH_DAT authdata;
    unsigned long faddr;
    struct hostent *hp;

    if (krb_get_lrealm(realm, 1) != KSUCCESS)
      return(0);

    ckstrncpy(tkt_file, TKT_ROOT, 20);
    ckstrncat(tkt_file, "_ftpdXXXXXX", 20);
    krb_set_tkt_string(mktemp(tkt_file));

    (VOID) ckstrncpy(instance, krb_get_phost(hostname), sizeof(instance));

    if ((hp = gethostbyname(instance)) == NULL)
      return(0);

#ifdef HADDRLIST
    hp = ck_copyhostent(hp);		/* safe copy that won't change */
#endif /* HADDRLIST */
    bcopy((char *)hp->h_addr, (char *) &faddr, sizeof(faddr));

    if (krb_get_pw_in_tkt(name, "", realm, "krbtgt", realm, 1, p) ||
        krb_mk_req(&ticket, "rcmd", instance, realm, 33) ||
        krb_rd_req(&ticket, "rcmd", instance, faddr, &authdata, "") ||
        kuserok(&authdata, name)) {
        dest_tkt();
        return(0);
    }
    dest_tkt();
    return(1);
}
#endif /* FTP_KERBEROS */

VOID
zsyslog() {
#ifdef CKSYSLOG
    if (ckxsyslog && !ckxlogging) {
#ifdef LOG_DAEMON
        openlog(inserver ? "iksd" : "ckermit", LOG_PID, LOG_DAEMON);
#else
        openlog(inserver ? "iksd" : "ckermit", LOG_PID);
#endif /* LOG_DAEMON */
        ckxlogging = 1;
        debug(F100,"zsyslog syslog opened","",0);
    }
#endif /* CKSYSLOG */
}

/*  Z V P A S S  --  Verify password; returns 1 if OK, 0 otherwise  */

#ifndef AUTH_USER
#define AUTH_USER 3
#endif /* AUTH_USER */
#ifndef AUTH_VALID
#define AUTH_VALID 4
#endif /* AUTH_VALID */

#ifdef __FreeBSD__			/* 299 This was necessary in */
#ifndef NODCLINITGROUPS			/* FreeBSD 4.4, don't know */
#define NODCLINITGROUPS			/* about other versions... */
#endif	/* NODCLINITGROUPS */            
#endif	/*  __FreeBSD__ */

int
zvpass(p) char *p; {
#ifndef NODCLINITGROUPS
_PROTOTYP(int initgroups, (const char *, gid_t) );
#endif	/* NODCLINITGROUPS */

    char *xpasswd, *salt;
    char * dir = NULL;
#ifdef CK_PAM
    int pam_status;
    const char * reply = NULL;
#endif /* CK_PAM */

    if (logged_in || askpasswd == 0) {
        return(0);
    }
    debug(F111,"zvpass",p ? (guest ? p : "xxxxxx") : "(null)",guest);
    if (!p) p = "";
    askpasswd = 0;
    if (guest && !*p) {                 /* Guests must specify a password */
#ifdef CKSYSLOG
        if (ckxsyslog && ckxlogging) {
            syslog(LOG_INFO,
                   "login: anonymous guests must specify a password"
                   );
        }
#endif /* CKSYSLOG */
        return(0);
    }
    if (!guest
#ifdef CK_AUTHENTICATION
        && ck_tn_auth_valid() != AUTH_VALID
#endif /* CK_AUTHENTICATION */
        ) {                     /* "ftp" is only account allowed no password */
#ifdef CK_PAM
        debug(F110,"zvpass","calling pam_set_item(AUTHTOK)",0);
        if ((pam_status = pam_set_item(pamh,PAM_AUTHTOK,p)) != PAM_SUCCESS) {
            reply = pam_strerror(pamh, pam_status);
            debug(F110,"zvpass PAM failure",reply,0);
            /* if no password given treat as non-fatal error */
            /* pam will prompt for password in pam_authenticate() */
            if (!p) {
                printf("%s\n",reply);
                pam_end(pamh, 0);
                debug(F100,"zvpass denied","",0);
                pw = NULL;
                zvuname[0] = NUL;
                return(0);
            }
        }
        debug(F110,"zvpass","calling pam_authenticate",0);
#ifdef COMMENT
        if (*p)
	  pam_pw = p;
#else
/*
  Make IKSD authentication (using PAM) ask for a password when an
  invalid username has been given, to avoid disclosing which account
  names are valid. See #417247 (Debian).
*/
        if (*p
#ifdef CK_LOGIN
	    || gotemptypasswd
#endif /* CK_LOGIN */
	    )
	    pam_pw = p;
#endif	/* COMMENT */
        if ((pam_status = pam_authenticate(pamh, 0)) != PAM_SUCCESS) {
            reply = pam_strerror(pamh, pam_status);
            debug(F110,"zvpass PAM failure",reply,0);
            printf("%s\n",reply);
            pam_end(pamh, 0);
            debug(F100,"zvpass denied","",0);
            pam_pw = NULL;
            pw = NULL;
            zvuname[0] = NUL;
            return(0);
        }
        pam_pw = NULL;
        debug(F110,"zvpass","calling pam_acct_mgmt",0);
        if ((pam_status = pam_acct_mgmt(pamh, 0)) != PAM_SUCCESS) {
            reply = pam_strerror(pamh, pam_status);
            debug(F110,"zvpass PAM failure",reply,0);
            printf("%s\n",reply);
            pam_end(pamh, 0);
            debug(F100,"zvpass denied","",0);
            pw = NULL;
            zvuname[0] = NUL;
            return(0);
        }
        debug(F110,"zvpass","PAM validates OK",0);
        pam_end(pamh,0);
#else /* CK_PAM */
        if (pw == NULL)
          salt = "xx";
        else
          salt = pw->pw_passwd;

#ifdef HPUX10_TRUSTED
        xpasswd = bigcrypt(p, salt);
#else
/*
  On 64-bit platforms this can give "cast to pointer from integer of
  different size" warning, but I'm not sure what the effect is at runtime,
  or what to do about it.
 */
        xpasswd = (char *)crypt(p, salt);
#endif /* HPUX10_TRUSTED */

        if (
#ifdef FTP_KERBEROS
            /* null pw_passwd ok if Kerberos password ok */
            pw == NULL ||
            ((*pw->pw_passwd != '\0' ||
              strcmp(xpasswd, pw->pw_passwd))
             && !kpass(pw->pw_name, p))
#else
#ifdef CK_SRP
            /* check with tpasswd first if there */
            pw == NULL || *pw->pw_passwd == '\0' ||
            t_verifypw (pw->pw_name, p) == 0 ||
            (t_verifypw (pw->pw_name, p) < 0 &&
            strcmp (xpasswd, pw->pw_passwd))
#else /* CK_SRP */
            /* The strcmp does not catch null passwords! */
            (pw == NULL) || (*pw->pw_passwd == '\0') ||
            strcmp(xpasswd, pw->pw_passwd)
#endif /* CK_SRP */
#endif /* FTP_KERBEROS */
            ) {
            debug(F100,"zvpass denied","",0);
            pw = NULL;
            zvuname[0] = NUL;
            return(0);
        }
#endif /* CK_PAM */
    }

    (VOID) setgid((GID_T)pw->pw_gid);   /* Set group ID */

#ifndef NOINITGROUPS
    (VOID) initgroups(pw->pw_name, pw->pw_gid);
#endif /* NOINITGROUPS */

    logged_in = 1;
    dir = pw->pw_dir;

#ifdef CKWTMP
    /* Open wtmp before chroot */
    if (ckxwtmp) {
        sprintf(cksysline,"iks_%04x", getpid()); /* safe */
        logwtmp(cksysline, pw->pw_name,
                 clienthost ? clienthost : "(unknown host)"
                );
        debug(F110,"WTMP login",cksysline,logged_in);
    }
#endif /* CKWTMP */
/*
  For anonymous users, we chroot to user ftp's home directory unless
  started with --anonroot:xxx, in which case we chroot to xxx.  We must
  immediately chdir() to the same directory we chroot() to or else the
  old current directory remains accessible as "." outside the new root.
*/
    if (guest) {
        if (anonroot)                   /* Non-default anonymous root */
          dir = anonroot;
        else
          makestr(&anonroot,dir);
        errno = 0;
        debug(F110,"zvpass anon chroot",dir,0);
        if (chroot(dir) < 0) {
            debug(F111,"zvpass anon chroot FAILED",dir,errno);
            goto bad;
        }
        errno = 0;
        if (chdir("/") < 0) {
            debug(F111,"zvpass anon chdir FAILED",dir,errno);
            goto bad;
        }
        debug(F110,"zvpass anon chroot/chdir OK",dir,0);
    } else if (chdir(dir) < 0) {        /* Not guest */
#ifdef COMMENT
        if (chdir("/") < 0) {
            debug(F110,"Non-guest chdir FAILED",dir,0);
            goto bad;
        } else
          printf("?No directory! Logging in with home=/\n");
#else
        debug(F110,"zvpass non-guest chdir FAILED",dir,0);
        goto bad;                       /* Be conservative at first */
#endif /* COMMENT */
    }
    debug(F110,"zvpass non-guest chdir OK",dir,0);
    if (setuid((UID_T)pw->pw_uid) < 0) {
        debug(F101,"zvpass setuid FAILED","",pw->pw_uid);
        goto bad;
    }
    debug(F101,"zvpass setuid OK","",pw->pw_uid);

    guestpass[0] = '\0';
    if (guest) {
        extern int fncact;
        isguest = 1;
        fncact = XYFX_R;                /* FILE COLLISION = RENAME */
        debug(F110,"GUEST fncact=R",p,0);
        lset(guestpass,"anonymous:",10,32);
        ckstrncpy(&guestpass[10],p,GUESTPASS-10);
        home = "/";
        printf("Anonymous login.\r\n");

#ifdef SETPROCTITLE
	/* proctitle declared where?  Obviously this code is never compiled. */
        sprintf(proctitle, "%s: anonymous/%.*s",
                clienthost ? clienthost : "(unk)",
                sizeof(proctitle) - sizeof(clienthost) -
                sizeof(": anonymous/"), p);
        setproctitle(proctitle);
#endif /* SETPROCTITLE */

#ifdef CKSYSLOG
        if (ckxsyslog && ckxlogging) {
            syslog(LOG_INFO,
                   "login: anonymous %s %s",
                   clienthost ? clienthost : "(unknown host)",
                   p
                   );
        }
#endif /* CKSYSLOG */

    } else {                            /* Real user */
        isguest = 0;
        home = dir;
        ckstrncpy(guestpass,zvuname,GUESTPASS);

        printf("User %s logged in.\r\n", pw->pw_name);
#ifdef SETPROCTITLE
	/* not used */
        sprintf(proctitle, "%s: %s",
                clienthost ? clienthost : "(unk)",
                pw->pw_name
                );
        setproctitle(proctitle);
#endif /* SETPROCTITLE */

#ifdef CKSYSLOG
        if (ckxsyslog && ckxlogging)
          syslog(LOG_INFO, "login: %s %s",
                 pw->pw_name,
                 clienthost ? clienthost : "(unknown host)"
                 );
#endif /* CKSYSLOG */
    }
    ckstrncpy(zvhome,home,CKMAXPATH);   /* Set environment variables */
#ifndef NOPUTENV

    ckmakmsg(zenvuser,ZENVUSER,"USER=",zvuname,NULL,NULL);
    putenv((char *)zenvuser);
    ckmakmsg(zenvlogname,ZENVLOGNAME,"LOGNAME=",zvuname,NULL,NULL);
    putenv((char *)zenvlogname);
    ckmakmsg(zenvhome,ZENVHOME,"HOME=",zvhome,NULL,NULL);
    putenv((char *)zenvhome);
#endif /* NOPUTENV */
    /* homdir = (char *)zvhome; */
    ckstrncpy((char *)uidbuf,(char *)zvuname,64);
    (VOID) umask(defumask);
#ifdef IKSDB
    if (ikdbopen) {
        char * p2;
        int k;
        extern char dbrec[];
        extern unsigned long myflags;
        extern unsigned int mydbslot;
        extern struct iksdbfld dbfld[];
#ifdef CK_AUTHENTICATION
        extern unsigned long myamode, myatype;
#endif /* CK_AUTHENTICATION */
        myflags |= DBF_LOGGED;
#ifdef DEBUG
	if (deblog) {
	    debug(F101,"zvpass guest","",guest);
	    debug(F111,"zvpass zvuname",zvuname,0);
	    debug(F110,"zvpass guestpass",guestpass,0);
	    debug(F110,"zvpass dir",dir,0);
	    debug(F110,"zvpass home",home,0);
	    debug(F110,"zvpass anonroot",anonroot,0);
	}
#endif /* DEBUG */
        p2 = guest ? guestpass : zvuname;
        if (guest) {
            p2 = (char *)guestpass;
            myflags &= ~DBF_USER;
        } else {
            p2 = (char *)zvuname;
            myflags |= DBF_USER;
        }
        k = strlen(p2);
        strncpy(&dbrec[DB_ULEN],ulongtohex((unsigned long)k,4),4);
        lset(&dbrec[dbfld[db_USER].off],p2,1024,32);
        strncpy(&dbrec[DB_FLAGS],ulongtohex(myflags,4),4);
#ifdef CK_AUTHENTICATION
        myamode = ck_tn_auth_valid();
        strncpy(&dbrec[DB_AMODE],ulongtohex(myamode,4),4);
        myatype = ck_tn_authenticated();
        strncpy(&dbrec[DB_ATYPE],ulongtohex(myatype,4),4);
#endif /* CK_AUTHENTICATION */
        if (guest) {
            p2 = dir;
        } else {
            p2 = zgtdir();
            if (!p2) p2 = "";
            if (!*p2) p2 = home;
        }
        strncpy(&dbrec[DB_DLEN],
                ulongtohex((unsigned long)strlen(p2),4),
                4
                );
        lset(&dbrec[dbfld[db_DIR].off],p2,1024,32);
        updslot(mydbslot);
    }
#endif /* IKSDB */
    return(1);

bad:                                    /* Common failure exit */
    zvuname[0] = NUL;
    zvlogout();
    return(0);
}
#endif /* CK_LOGIN */

/* Buggy Xenix 2.3.4 cc needs this line after the endif */
