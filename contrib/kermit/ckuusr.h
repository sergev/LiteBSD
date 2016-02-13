/*  C K U U S R . H  --  Symbol definitions for C-Kermit ckuus*.c modules  */

/*
  Author: Frank da Cruz <fdc@columbia.edu>,
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKUUSR_H
#define CKUUSR_H

#include "ckucmd.h"			/* Get symbols from command package */

#ifndef NOLUCACHE			/* Use lookup() cache */
#ifndef NOSPL				/* To speed up script programs */
#ifndef USE_LUCACHE
#define USE_LUCACHE
#endif /* USE_LUCACHE */
#endif /* NOSPL */
#endif /* NOLUCACHE */

#ifndef NODOUBLEQUOTING			/* New to 8.0 */
#define DOUBLEQUOTING			/* Allow fields to be enclosed in */
#endif /* NODOUBLEQUOTING */		/* doublequotes. */

#ifndef NOLOCUS				/* SET LOCUS */
#define LOCUS
#endif /* NOLOCUS */

/* Sizes of things - FNVALL and MAXARGLEN increased from 8K 20050912 */

#ifdef BIGBUFOK
#define FNVALL CMDBL			/* Function return value length */
#define MAXARGLEN CMDBL			/* Max func arg length after eval */
#define MAXARGLIST 1024			/* Max number of args for a macro */
#define FSPECL CMDBL			/* Max length for MSEND/GET string */
#define MSENDMAX 1024			/* Number of filespecs for MSEND */
#define MAC_MAX 16384			/* Maximum number of macros */

#else  /* Same as above but for smaller builds... */

#define FNVALL 1022
#define MAXARGLEN 1023
#define MAXARGLIST 64
#define FSPECL 300
#define MSENDMAX 128
#define MAC_MAX 1024
#endif /* BIGBUFOK */

#define GVARS 127			/* Highest global var allowed */
#ifdef BIGBUFOK
#define VNAML 4096			/* Max length for variable name */
#define ARRAYREFLEN 1024		/* Max length for array reference */
#define FORDEPTH 32			/* Maximum depth of nested FOR loops */
#define MAXTAKE 54			/* Maximum nesting of TAKE files */
#define MACLEVEL 128			/* Maximum nesting for macros */
#define INPBUFSIZ 4096			/* Size of INPUT buffer */
#define PROMPTL 1024			/* Max length for prompt */
#define LBLSIZ 8192			/* Maximum length for a GOTO label */
#else
#define VNAML 256			/* Max length for variable name */
#define ARRAYREFLEN 128			/* Max length for array reference */
#define FORDEPTH 10			/* Maximum depth of nested FOR loops */
#define MAXTAKE 32			/* Maximum nesting of TAKE files */
#define MACLEVEL 64			/* Maximum nesting for macros */
#define INPBUFSIZ 256
#define PROMPTL 256			/* Max length for prompt */
#define LBLSIZ 128			/* Maximum length for a GOTO label */
#endif /* BIGBUFOK */
#define NARGS 10			/* Max number of macro arguments */
#define LINBUFSIZ (CMDBL + 10)		/* Size of line[] buffer */
#define TMPBUFSIZ (CMDBL + 10)		/* Size of temporary buffer */

#define CMDSTKL ( MACLEVEL + MAXTAKE + 2) /* Command stack depth */

#ifndef NOPURGE				/* PURGE command */
#ifdef UNIX
#define CKPURGE
#endif /* UNIX */
#endif /* NOPURGE */

#ifndef NOMINPUT			/* MINPUT command */
#ifndef NOSPL
#define CK_MINPUT
#ifndef MINPMAX
#ifdef BIGBUFOK
#define MINPMAX 96			/* Max number of MINPUT strings */
#else
#define MINPMAX 16
#endif /* BIGBUFOK */
#endif /* MINPMAX */
#define MINPBUFL 256			/* Size of MINPUT temp buffer */
#endif /* NOSPL */
#endif /* NOMINPUT */

#define ARRAYBASE 95			/* Lowest array-name character */

#ifndef NOKERBANG
#ifndef KERBANG
#define KERBANG
#endif /* KERBANG */
#endif /* NOKERBANG */

/* Bit values (1, 2, 4, 8, ...) for the ccflgs field */

#define CF_APC  1			/* Executing an APC command */
#define CF_KMAC 2			/* Executing a \Kmacro */
#define CF_CMDL 4			/* Macro from -C "blah" command line */
#define CF_REXX 8			/* Macro from REXX interpreter */
#define CF_IMAC 16			/* Internal macro like FOR, WHILE... */

struct cmdptr {				/* Command stack structure */
    int src;				/* Command Source */
    int lvl;				/* Current TAKE or DO level */
    int ccflgs;				/* Flags */
};

struct mtab {				/* Macro table, like keyword table */
    char *kwd;				/* But with pointers for vals */
    char *mval;				/* instead of ints. */
    short flgs;
};

struct localvar {			/* Local variable structure. */
    char * lv_name;
    char * lv_value;
    struct localvar * lv_next;
};

struct stringlist {			/* General purpose string list */
    char * sl_name;
    struct stringlist * sl_next;
};

struct stringint {			/* String and (wide) integer */
    char * sval;			/* used mainly with command switches */
    int ival;
    CK_OFF_T wval;
};

#ifndef NOICP
/*
  C-Kermit Initialization file...

  System-dependent defaults are built into the program, see below.
  These can be overridden in either of two ways:
  1. CK_DSYSINI is defined at compile time, in which case a default
     system-wide initialization file name is chosen from below, or:
  2: CK_SYSINI is defined to be a string, which lets the program
     builder choose the initialization filespec at compile time.
  These can be given on the CC command line, so the source code need not be
  changed.
*/

#ifndef CK_SYSINI			/* If no initialization file given, */
#ifdef CK_DSYSINI			/* but CK_DSYSINI is defined... */

/* Supply system-dependent "default default" initialization file */

#ifdef UNIX				/* For UNIX, system-wide */
/* This allows one copy of the standard init file on the whole system, */
/* rather than a separate copy in each user's home directory. */
#ifdef HPUX10
#define CK_SYSINI "/usr/share/lib/kermit/ckermit.ini"
#else
#ifdef CU_ACIS
#define CK_SYSINI "/usr/share/lib/kermit/ckermit.ini"
#else
#ifdef __linux__
#define CK_SYSINI "/usr/share/kermit/ckermit.ini"
#else
#define CK_SYSINI "/usr/local/bin/ckermit.ini"
#endif /* linux */
#endif /* CU_ACIS */
#endif /* HPUX10 */
/* Fill in #else..#ifdef's here for VMS, OS/2, etc. */
/* Fill in matching #endif's here. */
#endif /* UNIX */

#endif /* CK_DSYSINI */
#endif /* CK_SYSINI */

#ifdef CK_SYSINI			/* Init-file precedence */
#ifndef CK_INI_A			/* A means system-wide file first */
#ifndef CK_INI_B			/* B means user's first */
#define CK_INI_A			/* A is default */
#endif /* CK_INI_B */
#endif /* CK_INI_A */
#else
#ifdef CK_INI_A				/* Otherwise */
#undef CK_INI_A				/* make sure these aren't defined */
#endif /* CK_INI_A */
#ifdef CK_INI_B
#undef CK_INI_B
#endif /* CK_INI_B */
#endif /* CK_SYSINI */

#ifdef CK_SYSINI			/* One more check... */
#ifdef CK_INI_A				/* Make sure they're not both */
#ifdef CK_INI_B				/* defined. */
#undef CK_INI_B
#endif /* CK_INI_B */
#endif /* CK_INI_A */
#endif /* CK_SYSINI */
/*
  If neither CK_DSYSINI nor CK_SYSINI are defined, these are the
  built-in defaults for each platform.  USE_CUSTOM means to execute the
  customization file automatically if the initialization file is not found.
*/
#ifndef NOCUSTOM
#ifndef USE_CUSTOM
#define USE_CUSTOM
#endif /* USE_CUSTOM */
#endif /* NOCUSTOM */

#ifndef  KERMRCL			/* Path length for init file */
#define KERMRCL CKMAXPATH
#endif /*  KERMRCL */

#ifdef vms
#define KERMRC "CKERMIT.INI"		/* Init file name */
#define MYCUSTOM "CKERMOD.INI"		/* Customization file name */
#else
#ifdef OS2
#ifdef NT
#define KERMRC "k95.ini"
#define MYCUSTOM "k95custom.ini"
#else
#define KERMRC "k2.ini"
#define MYCUSTOM "k2custom.ini"
#endif /* NT */
#else
#ifdef UNIXOROSK
#define KERMRC ".kermrc"
#define MYCUSTOM ".mykermrc"
#else
#ifdef STRATUS
#define KERMRC "ckermit.ini"
#define MYCUSTOM "ckermod.ini"
#else
#define KERMRC "CKERMIT.INI"
#define MYCUSTOM "ckermod.ini"
#endif /* STRATUS */
#endif /* UNIXOROSK */
#endif /* OS2 */
#endif /* vms */

#ifndef KERMRCL
#define KERMRCL 256
#endif /* KERMRCL */
#endif /* NOICP */

/* User interface features */

#ifdef CK_CURSES			/* Thermometer */
#ifndef NO_PCT_BAR
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* NO_PCT_BAR */
#endif /* CK_PCT_BAR */
#endif /* CK_CURSES */

#ifdef KUI				/* KUI requires the Thermometer code */
#ifndef NO_PCT_BAR
#ifndef CK_PCT_BAR
#define CK_PCT_BAR
#endif /* NO_PCT_BAR */
#endif /* CK_PCT_BAR */
#endif /* KUI */

/* Includes */

#ifdef MINIX
/* why? */
#include <sys/types.h>
#endif /* MINIX */

/* Symbols for command source */

#define CMD_KB 0			/* KeyBoard or standard input */
#define CMD_TF 1			/* TAKE command File */
#define CMD_MD 2			/* Macro Definition */

/*
  SET TRANSFER CANCELLATION command should be available in all versions.
  But for now...
*/
#ifdef UNIX				/* UNIX has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* UNIX */
#ifdef VMS				/* VMS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* VMS */
#ifdef datageneral			/* DG AOS/VS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* datageneral */
#ifdef STRATUS				/* Stratus VOS has it */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* STRATUS */
#ifdef OSK				/* OS-9 */
#ifndef XFRCAN
#define XFRCAN
#endif /* XFRCAN */
#endif /* OSK */

#ifndef NOCMDL
/* Extended Command-Line Option Codes (keep alphabetical by keyword) */

#define XA_ANON 0			/* --anonymous */
#define XA_BAFI 1			/* --bannerfile */
#define XA_CDFI 2			/* --cdfile */
#define XA_CDMS 3			/* --cdmessage */
#define XA_HELP 4			/* --help */
#define XA_HEFI 5			/* --helpfile */
#define XA_IKFI 6			/* --xferfile */
#define XA_IKLG 7			/* --xferlog */
#define XA_ANFI 8			/* --initfile */
#define XA_PERM 9			/* --permissions */
#define XA_ROOT 10			/* --root */
#define XA_SYSL 11			/* --syslog */
#define XA_USFI 12			/* --userfile */
#define XA_WTFI 13			/* --wtmpfile */
#define XA_WTMP 14			/* --wtmplog */
#define XA_TIMO 15			/* --timeout */
#define XA_NOIN 16			/* --nointerrupts */
#define XA_DBAS 17			/* --database */
#define XA_DBFI 18			/* --dbfile */
#define XA_PRIV 19			/* --privid */
#define XA_VERS 20			/* --version */
#define XA_NPRM 21			/* --noperms */
#define XA_XPOS 22			/* Window position X coordinate */
#define XA_YPOS 23			/* Window position Y coordinate */
#define XA_FNAM 24			/* Font Facename */
#define XA_FSIZ 25			/* Font Size */
#define XA_TERM 26			/* Terminal type */
#define XA_CSET 27			/* Remote Character Set */
#define XA_ROWS 28			/* Screen rows (height) */
#define XA_COLS 29			/* Screen columns (width) */
#define XA_TEL  30			/* Make a Telnet connection */
#define XA_FTP  31			/* Make an FTP connection */
#define XA_SSH  32			/* Make an SSH connection */
#define XA_USER 33			/* Username for login */
#define XA_PASS 34			/* Password for login */
#define XA_TITL 35                      /* Window Title */
#define XA_NOMN 36			/* No GUI Menu Bar */
#define XA_NOTB 37			/* No GUI Tool Bar */
#define XA_NOSB 38                      /* No GUI Status Bar */
#define XA_NOPUSH 39                    /* Disable external commands */
#define XA_NOSCROLL 40                  /* Disable scrollback operations */
#define XA_NOESCAPE 41                  /* Disable escape from connect mode */
#define XA_LOCK 42                      /* All lockdown options */
#define XA_NOBAR 43                     /* No GUI Bars */
#define XA_WMAX  44
#define XA_WMIN  45
#define XA_SCALE 46                     /* GUI Scale Font */
#define XA_CHGD  47                     /* GUI Change Dimensions */
#define XA_NOCLOSE 48                   /* GUI Disable Close Window */
#define XA_UNBUF 49			/* UNIX unbuffered console */
#define XA_MAX  49			/* Highest extended option number */
#endif /* NOCMDL */

#ifndef NOICP
/* Top Level Commands */
/* Values associated with top-level commands must be 0 or greater. */

#define XXBYE   0	/* BYE */
#define XXCLE   1	/* CLEAR */
#define XXCLO   2	/* CLOSE */
#define XXCON   3	/* CONNECT */
#define XXCPY   4	/* COPY */
#define XXCWD   5	/* CWD (Change Working Directory) */
#define XXDEF	6	/* DEFINE (a macro or variable) */
#define XXDEL   7	/* (Local) DELETE */
#define XXDIR   8	/* (Local) DIRECTORY */

/* DIRECTORY Command options... */
#define DIR_BRF 1	/* BRIEF */
#define DIR_VRB 2	/* VERBOSE */
#define DIR_PAG 3	/* PAGE */
#define DIR_NOP 4	/* NOPAGE */
#define DIR_ISO 5	/* ISODATE */
#define DIR_DAT 6	/* ENGLISHDATE */
#define DIR_HDG 7	/* HEADINGS */
#define DIR_NOH 8	/* NOHEADINGS */
#define DIR_SRT 9	/* SORT */
#define DIR_NOS 10      /* NOSORT */
#define DIR_ASC 11	/* ASCENDING */
#define DIR_DSC 12      /* DESCENDING */
#define DIR_REC 13      /* RECURSIVE */
#define DIR_NOR 14	/* NORECURIVE */
#define DIR_DOT 15	/* DOTFILES */
#define DIR_NOD 16	/* NODOTFILES */
#define DIR_DIR 17	/* DIRECTORIES */
#define DIR_FIL 18	/* FILES */
#define DIR_ALL 19	/* ALL */
#define DIR_NAM 20	/* NAMES: */
#define DIR_TYP 21	/* FILETYPES */
#define DIR_NOT 22	/* NOFILETYPES */
#define DIR_BUP 23	/* BACKUP */
#define DIR_NOB 24	/* NOBACKUP */
#define DIR_MSG 25      /* MESSAGE */
#define DIR_NOM 26      /* NOMESSAGE */
#define DIR_ARR 27      /* ARRAY: */
#define DIR_NAR 28      /* NOARRAY */
#define DIR_EXC 29	/* EXCEPT: */
#define DIR_LAR 30	/* LARGER-THAN: */
#define DIR_SMA 31	/* SMALLER-THAN: */
#define DIR_AFT 32	/* AFTER: */
#define DIR_NAF 33	/* NOT-AFTER: */
#define DIR_BEF 34	/* BEFORE: */
#define DIR_NBF 35	/* NOT-BEFORE: */
#define DIR_SUM 36	/* SUMMARY */
#define DIR_BIN 37	/* TYPE (only show binary or text) */
#define DIR_LNK 38	/* follow symlinks */
#define DIR_NLK 39	/* don't follow symlinks */
#define DIR_OUT 40	/* Output file for listing */
#define DIR_TOP 41	/* Top n lines */
#define DIR_COU 42	/* COUNT:var */
#define DIR_NOL 43	/* NOLINKS (don't show symlinks at at all) */

#define DIRS_NM 0       /* Sort directory by NAME */
#define DIRS_DT 1       /* Sort directory by DATE */
#define DIRS_SZ 2       /* Sort directory by SIZE */

#define XXDIS   9	/* DISABLE */
#define XXECH  10	/* ECHO */
#define XXEXI  11	/* EXIT */
#define XXFIN  12	/* FINISH */
#define XXGET  13	/* GET */
#define XXHLP  14	/* HELP */
#define XXINP  15	/* INPUT */
#define XXLOC  16	/* LOCAL */
#define XXLOG  17	/* LOG */
#define XXMAI  18	/* MAIL */
#define XXMOU  19	/* (Local) MOUNT */
#define XXMSG  20	/* (Local) MESSAGE */
#define XXOUT  21	/* OUTPUT */
#define XXPAU  22	/* PAUSE */
#define XXPRI  23	/* (Local) PRINT */
#define XXQUI  24	/* QUIT */
#define XXREC  25	/* RECEIVE */
#define XXREM  26	/* REMOTE */
#define XXREN  27	/* (Local) RENAME */
#define XXSEN  28	/* SEND */

/* SEND switches */

#define SND_BIN  0	/* Binary mode */
#define SND_DEL  1	/* Delete after */
#define SND_EXC  2	/* Except */
#define SND_LAR  3	/* Larger than */
#define SND_MAI  4	/* Mail */
#define SND_BEF  5	/* Before */
#define SND_AFT  6	/* After */
#define SND_PRI  7	/* Print */
#define SND_SHH  8	/* Quiet */
#define SND_REC  9	/* Recursive */
#define SND_SMA 10	/* Smaller than */
#define SND_STA 11	/* Starting-from */
#define SND_TXT 12	/* Text mode */
#define SND_CMD 13	/* From command (pipe)  */
#define SND_RES 14	/* Resend/Recover */
#define SND_PRO 15	/* Protocol */
#define SND_ASN 16	/* As-name */
#define SND_IMG 17	/* Image */
#define SND_LBL 18	/* Labeled */
#define SND_NAF 19	/* Not-After */
#define SND_NBE 20	/* Not-Before */
#define SND_FLT 21	/* Filter */
#define SND_PTH 22	/* Pathnames */
#define SND_NAM 23	/* Filenames */
#define SND_MOV 24      /* MOVE to another directory */
#define SND_REN 25      /* RENAME after sending */
#define SND_CAL 26	/* Calibrate */
#define SND_FIL 27	/* File containing list of files to send */
#define SND_NOB 28	/* Skip backup files  */
#define SND_DOT 29	/* Include dot-files */
#define SND_NOD 30	/* Exclude dot-files */
#define SND_ARR 31	/* Send from array */
#define SND_TYP 32	/* TYPE (only send text (or binary)) */
#define SND_XPA 33	/* TRANSPARENT */
#define SND_PIP 34	/* PIPES */
#define SND_ERR 35	/* ERROR */
#define SND_CSL 36	/* Local character set */
#define SND_CSR 37	/* Remote character set */
#define SND_UPD 38	/* Update */
#define SND_COL 39	/* Collision */
#define SND_NML 40	/* Namelist */
#define SND_SRN 41	/* Server-Rename */
#define SND_LNK 42	/* Follow links */
#define SND_NLK 43	/* Don't follow links */
#define SND_SIM 44	/* Simulate */
#define SND_DIF 45	/* If dates Differ */
#define SND_PAT 46	/* Pattern to use locally when GET'ing */
#define SND_NLS 47	/* (FTP only) MGET forces NLST */
#define SND_MLS 48	/* (FTP only) MGET forces MLSD */

#define SND_MAX 48	/* Highest SEND switch */

#define XXSER  29   	/* SERVER */
#define XXSET  30	/* SET */
#define XXSHE  31	/* Command for SHELL */
#define XXSHO  32	/* SHOW */
#define XXSPA  33	/* (Local) SPACE */
#define XXSTA  34	/* STATISTICS */
#define XXSUB  35	/* (Local) SUBMIT */
#define XXTAK  36	/* TAKE */
#define XXTRA  37	/* TRANSMIT */
#define XXTYP  38	/* (Local) TYPE */
#define XXWHO  39	/* (Local) WHO */
#define XXDIAL 40	/* (Local) DIAL */
#define XXLOGI 41	/* (Local) SCRIPT */
#define XXCOM  42	/* Comment */
#define XXHAN  43       /* HANGUP */
#define XXXLA  44	/* TRANSLATE */
#define XXIF   45	/* IF */
#define XXLBL  46       /* label */
#define XXGOTO 47	/* GOTO */
#define XXEND  48       /* END */
#define XXSTO  49       /* STOP */
#define XXDO   50       /* DO */
#define XXPWD  51       /* PWD */
#define XXTES  52       /* TEST */
#define XXASK  53       /* ASK */
#define XXASKQ 54       /* ASKQ */
#define XXASS  55       /* ASSIGN */
#define XXREI  56       /* REINPUT */
#define XXINC  57       /* INCREMENT */
#define XXDEC  59       /* DECREMENT */
#define XXELS  60       /* ELSE */
#define XXEXE  61	/* EXECUTE */
#define XXWAI  62	/* WAIT */
#define XXVER  63       /* VERSION */
#define XXENA  64       /* ENABLE */
#define XXWRI  65       /* WRITE */
#define XXCLS  66       /* CLS (clear screen) */
#define XXRET  67	/* RETURN */
#define XXOPE  68       /* OPEN */
#define XXREA  69	/* READ */
#define XXON   70       /* ON */
#define XXDCL  71       /* DECLARE */
#define XXBEG  72       /* BEGIN (not used) */
#define XXFOR  72       /* FOR */
#define XXWHI  73       /* WHILE */
#define XXIFX  74       /* Extended IF */
#define XXCMS  75       /* SEND from command output (not yet) */
#define XXCMR  76       /* RECEIVE to a command's input (not yet) */
#define XXCMG  77       /* GET to a command's input (not yet) */
#define XXSUS  78       /* SUSPEND */
#define XXERR  79       /* ERROR */
#define XXMSE  80       /* MSEND */
#define XXBUG  81       /* BUG */
#define XXPAD  82       /* PAD (as in X.25 PAD) ANYX25 */
#define XXRED  83       /* REDIAL */
#define XXGTA  84	/* _getargs (invisible internal) */
#define XXPTA  85	/* _putargs (invisible internal) */
#define XXGOK  86       /* GETOK - Ask for YES/NO */
#define XXTEL  87	/* TELNET */
#define XXASX  88	/* _ASSIGN (evaluates var name) */
#define XXDFX  89	/* _DEFINE (evaluates var name) */
#define XXPNG  90	/* PING (for TCP/IP) */
#define XXINT  91       /* INTRODUCTION */
#define XXCHK  92	/* CHECK (a feature) */
#define XXMSL  93       /* MSLEEP, MPAUSE (millisecond sleep) */
#define XXNEW  94       /* NEWS */
#define XXAPC  95       /* APC */
#define XXFUN  96       /* REDIRECT */
#define XXWRL  97	/* WRITE-LINE */
#define XXREXX 98	/* Rexx */
#define XXMINP 100	/* MINPUT */
#define XXRSEN 101	/* RESEND */
#define XXPSEN 102	/* PSEND */
#define XXGETC 103	/* GETC */
#define XXEVAL 104	/* EVALUATE */
#define XXFWD  105	/* FORWARD */
#define XXUPD  106      /* UPDATES */
#define XXBEEP 107      /* BEEP */
#define XXMOVE 108      /* MOVE */
#define XXMMOVE 109     /* MMOVE */
#define XXREGET 110     /* REGET */
#define XXLOOK  111	/* LOOKUP */
#define XXVIEW  112     /* VIEW (terminal buffer) */
#define XXANSW  113	/* ANSWER (the phone) */
#define XXPDIA  114	/* PDIAL (partial dial) */
#define XXASC   115	/* ASCII / TEXT */
#define XXBIN   116	/* BINARY */
#define XXFTP   117	/* FTP */
#define XXMKDIR 118	/* MKDIR */
#define XXRMDIR 119	/* RMDIR */
#define XXTELOP 120	/* TELOPT */
#define XXRLOG  121	/* RLOGIN */
#define XXUNDEF 122	/* UNDEFINE */
#define XXNPSH  123	/* NOPUSH */
#define XXADD   124	/* ADD */
#define ADD_SND   0     /* ADD SEND-LIST */
#define ADD_BIN   1     /* ADD BINARY-PATTERNS */
#define ADD_TXT   2     /* ADD TEXT-PATTERNS */
#define XXLOCAL 125	/* LOCAL */
#define XXKERMI 126	/* KERMIT */
#define XXDATE  127	/* DATE */
#define XXSWIT  128     /* SWITCH */
#define XXXFWD  129	/* _FORWARD */
#define XXSAVE  130     /* SAVE */
#define XXXECH  131     /* XECHO */
#define XXRDBL  132     /* READBLOCK */
#define XXWRBL  133     /* WRITEBLOCK */
#define XXRETR  134     /* RETRIEVE */
#define XXEIGHT 135     /* EIGHTBIT */
#define XXEDIT  136	/* EDIT */
#define XXCSEN  137	/* CSEND */
#define XXCREC  138	/* CRECEIVE */
#define XXCQ    139	/* CQ */
#define XXTAPI  140     /* TAPI actions such as dialogs */
#define XXRES   141     /* RESET */
#define XXCGET  142     /* CGET */
#define XXFUNC  143     /* Function (help-only) */
#define XXKVRB  144     /* Kverb (help-only) */
#define XXBROWS 145	/* BROWSE */
#define XXMGET  146	/* MGET */
#define XXBACK  147	/* BACK */
#define XXWHERE 148	/* WHERE */
#define XXREDO  149     /* REDO */
#define XXEXCH  150     /* EXCHANGE */
#define XXREMV  151     /* REMOVE */
#define XXCHRT  152	/* CHROOT */
#define XXOPTS  153	/* Options (help-only) */
#define XXAUTH  154	/* AUTHORIZE */
#define XXPIPE  155	/* PIPE */
#define XXSSH   156	/* SSH */
#define XXTERM  157     /* TERMINAL */
#define XXSTATUS 158    /* STATUS */
#define XXCPR   159	/* COPYRIGHT */
#define XXASSER 160	/* ASSERT */
#define XXSUCC  161     /* SUCCEED */
#define XXFAIL  162     /* FAIL */
#define XXLOGIN 163     /* LOGIN */
#define XXLOGOUT 164    /* LOGOUT */
#define XXNLCL  165     /* NOLOCAL */
#define XXWILD  166     /* WILDCARDS (help-only) */

/* One-word synonyms for REMOTE commands */

#define XXRCPY  167	/* REMOTE COPY */
#define XXRCWD  168	/* Change Working Directory */
#define XXRDEL  169	/* Delete */
#define XXRDIR  170	/* Directory */
#define XXRHLP  171	/* Help */
#define XXRHOS  172	/* Host */
#define XXRKER  173	/* Kermit */
#define XXRMSG  174	/* Message */
#define XXRPRI  175	/* Print */
#define XXRREN  176	/* Rename */
#define XXRSET  177	/* Set */
#define XXRSPA  178	/* Space */
#define XXRSUB  179	/* Submit */
#define XXRTYP  180	/* Type */
#define XXRWHO  181	/* Who */
#define XXRPWD  182	/* Print Working Directory */
#define XXRQUE  183	/* Query */
#define XXRASG  184	/* Assign */
#define XXRMKD  185	/* mkdir */
#define XXRRMD  186	/* rmdir */
#define XXRXIT  187	/* Exit */

/* Top-Level commands, cont'd... */

#define XXGETK  188	/* GETKEYCODE */
#define XXMORE  189	/* MORE */
#define XXXOPTS 190	/* Extended-Options (help-only) */
#define XXIKSD  191	/* IKSD */
#define XXRESET 192	/* RESET */
#define XXASSOC 193     /* ASSOCIATE */

#define ASSOC_FC  0     /* ASSOCIATE FILE-CHARACTER-SET */
#define ASSOC_TC  1     /* ASSOCIATE TRANSFER-CHARACTER-SET */

#define XXSHIFT 194	/* SHIFT */
#define XXMAN   195     /* MANUAL */
#define XXLS    196     /* LS */
#define XXSORT  197	/* SORT */
#define XXPURGE 198	/* PURGE */
#define XXFAST  199	/* FAST */
#define XXCAU   200	/* CAUTIOUS */
#define XXROB   201	/* ROBUST */
#define XXMACRO 202     /* Immediate macro */
#define XXSCRN  203	/* SCREEN */
#define XXLNOUT 204     /* LINEOUT */
#define XX_INCR 205	/* _INCREMENT */
#define XX_DECR 206	/* _DECREMENT */
#define XX_EVAL 207	/* _EVALUATE */
#define XXARRAY 208	/* ARRAY */
#define XXPARSE 209	/* PARSE */
#define XXHTTP  210     /* HTTP */

#ifdef CKCHANNELIO
#define XXFILE  211	/* FILE */
#define XXF_CL  212	/* FCLOSE */
#define XXF_FL  213	/* FFLUSH */
#define XXF_LI  214	/* FLIST */
#define XXF_OP  215	/* FOPEN */
#define XXF_RE  216	/* FREAD */
#define XXF_SE  217	/* FSEEK */
#define XXF_ST  218	/* FSTATUS */
#define XXF_WR  219	/* FWRITE */
#define XXF_RW  220	/* FREWIND */
#define XXF_CO  221	/* FCOUNT */
#endif /* CKCHANNELIO */

#define XXEXEC  222	/* exec() */
#define XXTRACE 223	/* TRACE */
#define XXNOTAV 224	/* The "not available" command */
#define XXPTY   225     /* PTY (like PIPE) */
#define XXCHMOD 226     /* CHMOD */
#define XXPROMP 227	/* PROMPT */
#define XXFEACH 228	/* FOREACH */
#define XXGREP  229     /* GREP */
#define XXSEXP  230	/* S-Expression */
#define XXUNDCL 231	/* UNDECLARE */
#define XXVOID  232     /* VOID */
#define XXPUT   233	/* PUT */
#define XXUNDFX 234	/* _UNDEFINE */
#define XXHEAD  235     /* HEAD */
#define XXTAIL  236     /* TAIL */
#define XXDEBUG 237     /* DEBUG */
#define XXLEARN 238     /* LEARN */
#define XXPAT   239     /* PATTERNS (help only) */

#define XXCDUP  240     /* CDUP (Change working directory upwards) */
#define XXRCDUP 241     /* REMOTE CDUP */
#define XXCAT   242	/* CAT (= TYPE /NOPAGE) */
#define XXFIREW 243     /* FIREWALL (help only) */

#define XXLCWD  244	/* Local C(W)D */
#define XXLCDU  245	/* Local CDUP */
#define XXLPWD  246	/* Local PWD */
#define XXLDEL  247	/* Local DELETE */
#define XXLDIR  248	/* Local DIRECTORY */
#define XXLREN  249	/* Local RENAME */
#define XXLMKD  250	/* Local MKDIR */
#define XXLRMD  251	/* Local RMDIR */
#define XXUSER  252	/* (FTP) USER */
#define XXACCT  253	/* (FTP) ACCOUNT */
#define XXLINK  254     /* LINK source destination */
#define XXORIE  255	/* ORIENT(ATION) */
#define XXDIALER 256    /* DIALER */
#define XXKCD   257     /* KCD */
#define XXSITE  258     /* (FTP) SITE */
#define XXPASV  259     /* (FTP) PASSIVE */
#define XXCONT  260	/* CONTINUE */
#define XXNSCR  261	/* NOSCROLL */
#define XXSFTP  262	/* SFTP */
#define XXSKRM  263	/* SKERMIT */
#define XXWDIR  264	/* WDIRECTORY */
#define XXHDIR  265	/* HDIRECTORY */
#define XXTOUC  266	/* TOUCH */
#define XXLOCU  267	/* LOCUS (for HELP) */
#define XXPUTE  268     /* PUTENV */
#define XXXMSG  269     /* XMESSAGE */

/* End of Top-Level Commands */

#define SCN_CLR   0			/* SCREEN CLEAR */
#define SCN_CLE   1			/* SCREEN CLEOL */
#define SCN_MOV   2			/* SCREEN MOVE */

/* ARRAY operations */

#define ARR_DCL   0			/* Declare */
#define ARR_CPY   1			/* Copy */
#define ARR_RSZ   2			/* Resize */
#define ARR_SRT   3			/* Sort */
#define ARR_CLR   4			/* Clear */
#define ARR_SEA   5			/* Search */
#define ARR_DST   6			/* Destroy */
#define ARR_SHO   7			/* Show */
#define ARR_SET   8			/* Set */
#define ARR_EQU   9			/* Equate */

/* SORT options */

#define SRT_CAS   0			/* /CASE */
#define SRT_KEY   1			/* /KEY:n */
#define SRT_REV   2			/* /REVERSE */
#define SRT_RNG   3			/* /RANGE:n:m */
#define SRT_NUM   4			/* /NUMERIC */

/* PURGE command options */

#define PU_KEEP 0			/* /KEEP: */
#define PU_LIST 1			/* /LIST */
#define PU_PAGE 2			/* /PAGE */
#define PU_NOPA 3			/* /NOPAGE */
#define PU_NODE 4			/* /SIMULATE */
#define PU_DELE 5			/* /DELETE */
#define PU_NOLI 6			/* /NOLIST */
#define PU_QUIE 7			/* /QUIET (= NOLIST) */
#define PU_VERB 8			/* /VERBOSE (= LIST) */
#define PU_ASK  9			/* /ASK */
#define PU_NASK 10			/* /NOASK */
#define PU_LAR  11			/* /LARGER-THAN: */
#define PU_SMA  12			/* /SMALLER-THAN: */
#define PU_AFT  13			/* /AFTER: */
#define PU_NAF  14			/* /NOT-AFTER: */
#define PU_BEF  15			/* /BEFORE: */
#define PU_NBF  16			/* /NOT-BEFORE: */
#define PU_EXC  17			/* /EXCEPT: */
#define PU_RECU 18			/* /RECURSIVE */
#define PU_DOT  19			/* /DOTFILES */
#define PU_NODOT 20			/* /NODOTFILES */
#define PU_HDG  21			/* /HEADING */
#define PU_NOH  22			/* /NOHEADING */

/* DELETE command options */

#define DEL_NOL 0			/* /NOLIST */
#define DEL_LIS 1			/* /LIST */
#define DEL_HDG 2			/* /HEADINGS */
#define DEL_NOH 2			/* /NOHEADINGS */
#define DEL_BEF 3			/* /BEFORE: */
#define DEL_NBF 4			/* /NOT-BEFORE: */
#define DEL_AFT 5			/* /AFTER: */
#define DEL_NAF 6			/* /NOT-AFTER: */
#define DEL_DOT 7			/* /DOTFILES */
#define DEL_NOD 8			/* /NODOTFILES */
#define DEL_EXC 9			/* /EXCEPT:*/
#define DEL_PAG 10			/* /PAGE */
#define DEL_NOP 11			/* /NOPAGE */
#define DEL_REC 12			/* /RECURSIVE */
#define DEL_NOR 13			/* /NORECURSIVE */
#define DEL_VRB 14			/* /VERBOSE */
#define DEL_QUI 15			/* /QUIET */
#define DEL_SMA 16			/* /SMALLER-THAN: */
#define DEL_LAR 17			/* /LARGER-THAN: */
#define DEL_SIM 18			/* /SIMULATE */
#define DEL_ASK 19			/* /ASK */
#define DEL_NAS 20			/* /NOASK */
#define DEL_SUM 21			/* /SUMMARY */
#define DEL_DIR 22			/* /DIRECTORY */
#define DEL_ALL 23			/* /ALL */
#define DEL_TYP 24			/* /TYPE: */
#define DEL_LNK 25			/* /FOLLOWLINKS */
#define DEL_NLK 26			/* /NOFOLLOWLINKS */

/* RENAME switches that can be used in the same table as the DEL switches */

#define REN_LOW 100			/* Convert to lowercase */
#define REN_UPP 101			/* Converto to uppercase */
#define REN_RPL 102			/* String replacement */
#define REN_OVW 103			/* Overwrite file with same name */
#define REN_XLA 104			/* Translate character sets */
#define REN_SPA 105			/* Fix spaces */

/* FILE operations */

#define FIL_OPN  0			/* OPEN */
#define FIL_CLS  1			/* CLOSE */
#define FIL_REA  2			/* READ */
#define FIL_GET  3			/* GET */
#define FIL_WRI  4			/* WRITE */
#define FIL_REW  5			/* REWIND */
#define FIL_LIS  6			/* LIST */
#define FIL_FLU  7			/* FLUSH */
#define FIL_SEE  8			/* SEEK */
#define FIL_STA  9			/* STATUS */
#define FIL_COU 10			/* COUNT */

/* OPEN / CLOSE items */

#define OPN_FI_R 1			/* FILE READ */
#define OPN_FI_W 2			/* FILE WRITE */
#define OPN_FI_A 3			/* FILE APPEND */
#define OPN_PI_R 4			/* PIPE READ */
#define OPN_PI_W 5			/* PIPE WRITE */
#define OPN_PT_R 6			/* PTY READ */
#define OPN_PT_W 7			/* PTY WRITE */
#define OPN_SER	 8			/* PORT or LINE */
#define OPN_NET	 9			/* HOST */

/* KERBEROS command switches */

#define KRB_S_VE  0	/* /VERSION */
#define KRB_S_CA  1	/* /CACHE: */
#define KRB_S_MAX 1	/* Highest KERBEROS switch number */

#ifdef CK_KERBEROS

/* KERBEROS actions */

#define KRB_A_IN  0	/* INITIALIZE */
#define KRB_A_DE  1	/* DESTROY */
#define KRB_A_LC  2	/* LIST-CREDENTIALS */

/* KERBEROS INIT switches */

#define KRB_I_FW  0	/* /FORWARDABLE */
#define KRB_I_LF  1	/* /LIFETIME: */
#define KRB_I_PD  2	/* /POSTDATE: */
#define KRB_I_PR  3	/* /PROXIABLE */
#define KRB_I_RB  4	/* /RENEWABLE: */
#define KRB_I_RN  5	/* /RENEW */
#define KRB_I_SR  6	/* /SERVICE: */
#define KRB_I_VA  7	/* /VALIDATE */
#define KRB_I_RL  8     /* /REALM: */
#define KRB_I_IN  9     /* /INSTANCE: */
#define KRB_I_PW  10    /* /PASSWORD: */
#define KRB_I_PA  11    /* /PREAUTH */
#define KRB_I_VB  12    /* /VERBOSE */
#define KRB_I_BR  13    /* /BRIEF */
#define KRB_I_NFW 14	/* /NOT-FORWARDABLE */
#define KRB_I_NPR 15	/* /NOT-PROXIABLE */
#define KRB_I_NPA 16    /* /NOT-PREAUTH */
#define KRB_I_K4  17    /* /KERBEROS4    (should k5 get k4 as well) */
#define KRB_I_NK4 18    /* /NO-KERBEROS4 */
#define KRB_I_POP 19    /* /POPUP */
#define KRB_I_ADR 20    /* /ADDRESSES: */
#define KRB_I_NAD 21    /* /NO-ADDRESSES */
#define KRB_I_MAX 21    /* Highest KERBEROS INIT switch number */

#endif /* CK_KERBEROS */

/* SET parameters */

#define XYBREA  0	/* BREAK simulation */
#define XYCHKT  1	/* Block check type */
#define XYDEBU  2	/* Debugging */
#define XYDELA  3	/* Delay */
#define XYDUPL  4	/* Duplex */
#define XYEOL   5	/* End-Of-Line (packet terminator) */
#define XYESC   6	/* Escape character */
#define XYFILE  7	/* File Parameters (see ckcker.h for values) */
			/* (this space available) */
#define XYFLOW  9	/* Flow Control */
#define XYHAND 10	/* Handshake */
#define XYIFD  11	/* Incomplete File Disposition */
#define XYIMAG 12	/* "Image Mode" */
#define XYINPU 13	/* INPUT command parameters */
#define XYLEN  14	/* Maximum packet length to send */
#define XYLINE 15	/* Communication line to use */

/* SET LINE / SET HOST command switches */

#define SL_CNX   0	/* /CONNECT */
#define SL_SRV   1	/* /SERVER */
#define SL_SHR   2	/* /SHARE */
#define SL_NSH   3	/* /NOSHARE */
#define SL_BEE   4	/* /BEEP */
#define SL_ANS   5	/* /ANSWER */
#define SL_DIA   6	/* /DIAL:xxx */
#define SL_SPD   7	/* /SPEED:xxx */
#define SL_FLO   8	/* /FLOW:xxx */
#define SL_TMO   9	/* /TIMEOUT:xxx */
#define SL_CMD  10	/* /COMMAND */
#define SL_PSW  11	/* /PASSWORD:xxx */
#define SL_IKS  12      /* /KERMIT-SERVICE */
#define SL_NET  13      /* /NETWORK-TYPE:xxx */
#define SL_ENC  14      /* /ENCRYPT:type (telnet) /ENCRYPT (rlogin) */
#define SL_KRB4 15      /* /KERBEROS 4 (rlogin/telnet) */
#define SL_KRB5 16      /* /KERBEROS 5 (rlogin/telnet) */
#define SL_SRP  17      /* /SRP (telnet) */
#define SL_NTLM 18      /* /NTLM (telnet) */
#define SL_SSL  19      /* /SSL (telnet) */
#define SL_UID  20      /* /USERID:xxxx */
#define SL_AUTH 21      /* /AUTH:type */
#define SL_WAIT 22	/* /WAIT */
#define SL_NOWAIT 23	/* /NOWAIT */
#define SL_PTY  24      /* /PTY */

#define XYLOG  16	/* Log file */
#define XYMARK 17	/* Start of Packet mark */
#define XYNPAD 18	/* Amount of padding */
#define XYPADC 19	/* Pad character */
#define XYPARI 20	/* Parity */
#define XYPAUS 21	/* Interpacket pause */
#define XYPROM 22	/* Program prompt string */
#define XYQBIN 23	/* 8th-bit prefix */
#define XYQCTL 24	/* Control character prefix */
#define XYREPT 25	/* Repeat count prefix */
#define XYRETR 26	/* Retry limit */
#define XYSPEE 27	/* Line speed (baud rate) */
#define XYTACH 28	/* Character to be doubled */
#define XYTIMO 29	/* Timeout interval */
#define XYMODM 30	/* Modem - also see XYDIAL */

#define XYSEND 31	/* SET SEND parameters */
#define XYRECV 32   	/* SET RECEIVE parameters */
#define XYTERM 33	/* SET TERMINAL parameters */
#define   XYTBYT 0      /*  Terminal Bytesize (7 or 8) */
#define   XYTTYP 1      /*  Terminal emulation Type */
#define     TT_NONE  0	/*    NONE, no emulation */
#ifdef OS2
/*
  Note, the symbols for VT and VT-like terminals should be in ascending
  numerical order, so that higher ones can be treated as supersets of
  lower ones with respect to capabilities.

  This is no longer the case with the influx of new terminal types.
  Just make sure that the ISXXXXX() macros include the proper family
  groups.
*/
#define     TT_DG200    1 	/*    Data General 200 */
#define     TT_DG210    2  	/*    Data General 210 */
#define     TT_DG217    3   	/*    Data General 217 */
#define     TT_HP2621   4 	/*    Hewlett-Packard 2621A */
#define     TT_HPTERM   5	/*    Hewlett-Packard Console */
#define     TT_HZL1500  6 	/*    Hazeltine 1500 */
#define     TT_VC4404   7 	/*    Volker Craig VC4404/404 */
#define     TT_WY30     8	/*    WYSE-30/30+ */
#define     TT_WY50     9 	/*    WYSE-50/50+ */
#define     TT_WY60    10       /*    WYSE-60	 */
#define     TT_WY160   11       /*    WYSE-160   */
#define     TT_QNX     12       /*    QNX */
#define     TT_QANSI   13       /*    QNX Ansi emulation */
#define     TT_VT52    14	/*    DEC VT-52  */
#define     TT_H19     15	/*    Heath-19 */
#define     TT_IBM31   16       /*    IBM 31xx */
#define     TT_SCOANSI 17	/*    SCOANSI (Unix mode) */
#define     TT_AT386   18 	/*    Unixware AT386 (Unix mode) */
#define     TT_ANSI    19	/*    IBM ANSI.SYS (BBS) */
#define     TT_VIP7809 20	/*    Honeywell VIP7809 */
#define     TT_LINUX   21       /*    Linux Console */
#define     TT_HFT     22       /*    IBM High Function Terminal */
#define     TT_AIXTERM 23       /*    IBM AIXterm */
#define     TT_SUN     24       /*    SUN Console */
#define     TT_BA80    25       /*    Nixdorf BA80 */
#define     TT_BEOS    26       /*    BeOS Ansi */
#define     TT_VT100   27	/*    DEC VT-100 */
#define     TT_VT102   28	/*    DEC VT-102 */
#define     TT_VT220   29	/*    DEC VT-220 */
#define     TT_VT220PC 30       /*    DEC VT-220 with PC keyboard */
#define     TT_VT320   31	/*    DEC VT-320 */
#define     TT_VT320PC 32	/*    DEC VT-320 with PC keyboard */
#define     TT_WY370   33	/*    WYSE 370 ANSI Terminal */
#define     TT_97801   34       /*    Sinix 97801-5xx terminal */
#define     TT_AAA     35       /*    Ann Arbor Ambassador */
#define     TT_TVI910  36	/*    TVI 910+ */
#define     TT_TVI925  37       /*    TVI 925  */
#define     TT_TVI950  38       /*    TVI950   */
#define     TT_ADM3A   39       /*    LSI ADM 3A */
#define     TT_ADM5    40		/*    LSI ADM 5 */
#define     TT_VTNT    41       /*    Microsoft NT Virtual Terminal */
#define     TT_MAX   TT_VTNT
#define     TT_VT420   96	/*    DEC VT-420 */
#define     TT_VT520   97	/*    DEC VT-520/525 */
#define     TT_TEK40 99	/*    Tektronix 401x */
#define     TT_KBM_EMACS   TT_MAX+1
#define     TT_KBM_HEBREW  TT_MAX+2
#define     TT_KBM_RUSSIAN TT_MAX+3
#define     TT_KBM_WP      TT_MAX+4

#define ISAAA(x)   (x == TT_AAA)
#define ISANSI(x)  (x >= TT_SCOANSI && x <= TT_ANSI)
#define ISBA80(x)  (x == TT_BA80)
#define ISBEOS(x)  (x == TT_BEOS)
#define ISQNX(x)   (x == TT_QNX)
#define ISQANSI(x)   (x == TT_QANSI)
#define ISLINUX(x) (x == TT_LINUX)
#define ISSCO(x)   (x == TT_SCOANSI)
#define ISAT386(x) (x == TT_AT386)
#define ISAVATAR(x) (x == TT_ANSI)
#define ISSUN(x)    (x == TT_SUN)
#define ISUNIXCON(x) (x == TT_SCOANSI || x == TT_AT386 || \
                      x == TT_LINUX   || x == TT_SUN)
#define ISDG200(x) (x >= TT_DG200 && x <= TT_DG217)
#define ISHZL(x)   (x == TT_HZL1500)
#define ISH19(x)   (x == TT_H19)
#define ISIBM31(x) (x == TT_IBM31)
#define ISTVI(x)   (x >= TT_TVI910 && x <= TT_TVI950)
#define ISTVI910(x) (x == TT_TVI910)
#define ISTVI925(x) (x == TT_TVI925)
#define ISTVI950(x) (x == TT_TVI950)
#define ISVT52(x)  (x == TT_VT52 || x == TT_H19)
#ifdef COMMENT
#define ISVT520(x) (x == TT_VT520)
#define ISVT420(x) (x >= TT_VT420 && x <= TT_VT520)
#else /* COMMENT */
/* Since we do not yet support 420/520 extend 320 */
#define ISVT520(x) (ISVT320(x))
#define ISVT420(x) (ISVT320(x))
#endif /* COMMENT */
#define ISVT320(x) (x >= TT_VT320 && x <= TT_AAA)
#define ISVT220(x) (x >= TT_VT220 && x <= TT_AAA || \
                    ISBEOS(x) || ISQANSI(x) || \
                    ISLINUX(x) || ISSUN(x))
#define ISVT102(x) (x >= TT_VIP7809 && x <= TT_BA80 || \
		    x == TT_VT102 || ISVT220(x))
#define ISVT100(x) (x == TT_VT100 || ISVT102(x))
#define ISWY30(x)  (x == TT_WY30)
#define ISWYSE(x)  (x >= TT_WY30 && x <= TT_WY160)
#define ISWY50(x)  (x == TT_WY50)
#define ISWY60(x)  (x == TT_WY60 || x == TT_WY160)
#define ISWY160(x) (x == TT_WY160)
#define ISWY370(x) (x == TT_WY370)
#define ISVC(x)    (x == TT_VC4404)
#define ISHP(x)    (x == TT_HPTERM || x == TT_HP2621)
#define ISHPTERM(x) (x == TT_HPTERM)
#define ISVIP(x)   (x == TT_VIP7809)
#define IS97801(x) (x == TT_97801)
#define ISHFT(x)   (x == TT_HFT || x == TT_AIXTERM)
#define ISAIXTERM(x) (x == TT_AIXTERM)
#define ISTEK(x)   (x == TT_TEK40)
#define ISVTNT(x)  (x == TT_VTNT)
#define ISADM3A(x) (x == TT_ADM3A)
#define ISADM5(x)  (x == TT_ADM5)
#endif /* OS2 */

#define   XYTCS  2      /*  Terminal Character Set */
#define   XYTSO  3	/*  Terminal Shift-In/Shift-Out */
#define   XYTNL  4      /*  Terminal newline mode */
#ifdef OS2
#define   XYTCOL 5      /*  Terminal colors */
#endif /* OS2 */
#define   XYTEC  6	/*  Terminal echo = duplex = local-echo */
#ifdef OS2
#define   XYTCUR 7	/*  Terminal cursor */
#define     TTC_ULINE 0
#define     TTC_HALF  1
#define     TTC_BLOCK 2
#define   XYTARR 8	/*  Terminal arrow-key mode */
#define   XYTKPD 9      /*  Terminal keypad mode */
#define    TTK_NORM 0   /*    Normal mode for arrow / keyad keys */
#define    TTK_APPL 1   /*    Application mode for arrow / keyad keys */
#define   XYTWRP 10     /*  Terminal wrap */
#endif /* OS2 */
#define   XYTCRD 11	/*  Terminal CR-display */
#define   XYTANS 12	/*  Terminal answerback */
#ifdef OS2
#define   XYSCRS 13     /*  Terminal scrollback buffer size */
#endif /* OS2 */
#define   XYTAPC 14	/*  Terminal APC */
#ifdef OS2
#define   XYTBEL 15     /*  Terminal Bell */
#endif /* OS2 */
#define   XYTDEB 16	/*  Terminal Debug */
#ifdef OS2
#define   XYTROL 17     /*  Terminal Rollback */
#define     TTR_OVER   0  /*  Rollback Overwrite */
#define     TTR_INSERT 1  /*  Rollback Insert */
#define     TTR_KEYS   2  /*  Keystrokes */
#define       TTRK_IGN 0  /*    Ignore */
#define       TTRK_RST 2  /*    Restore and Send */
#define       TTRK_SND 1  /*    Send */
#define   XYTCTS 18     /*  Terminal Transmit-Timeout */
#define   XYTCPG 19     /*  Terminal Code Page */
#ifdef COMMENT
#define   XYTHCU 20     /*  Terminal Hide-Cursor */
#endif /* COMMENT */
#define   XYTPAC 21	    /*  Terminal Output-Pacing */
#define   XYTMOU 22	    /*  Terminal Mouse */
#endif /* OS2 */
#define   XYTHIG 23     /*  Terminal Width */
#define   XYTWID 24     /*  Terminal Height */
#ifdef OS2
#define   XYTUPD 25     /*  Terminal Screen-update */
#define    TTU_FAST 0   /*     FAST but jerky */
#define    TTU_SMOOTH 1 /*     SMOOTH but slow */
#define   XYTFON 26     /*  Terminal Full screen Font */
#define    TTF_ROM    0 /*     ROM font */
#define    TTF_CY1    1 /*     CYRILL1 font */
#define    TTF_CY2    2 /*     CYRILL2 font */
#define    TTF_CY3    3 /*     CYRILL3 font */
#define    TTF_111  111 /*     CP111 font */
#define    TTF_112  112 /*     CP112 font */
#define    TTF_113  113 /*     CP113 font */
#define    TTF_437  437 /*     CP437 font */
#define    TTF_850  850 /*     CP850 font */
#define    TTF_851  851 /*     CP851 font */
#define    TTF_852  852 /*     CP852 font */
#define    TTF_853  853 /*     CP853 font */
#define    TTF_860  860 /*     CP860 font */
#define    TTF_861  861 /*     CP861 font */
#define    TTF_862  862 /*     CP862 font */
#define    TTF_863  863 /*     CP863 font */
#define    TTF_864  864 /*     CP864 font */
#define    TTF_865  865 /*     CP865 font */
#define    TTF_866  866 /*     CP866 font */
#define    TTF_880  880 /*     CP880 font */
#define    TTF_881  881 /*     CP881 font */
#define    TTF_882  882 /*     CP882 font */
#define    TTF_883  883 /*     CP883 font */
#define    TTF_884  884 /*     CP884 font */
#define    TTF_885  885 /*     CP885 font */
#define   XYTVCH 27     /* SET TERMINAL VIDEO-CHANGE */
#define    TVC_DIS   0  /*     DISABLED */
#define    TVC_ENA   1  /*     ENABLED  */
#define    TVC_W95   2  /*     WIN95-SAFE */
#endif /* OS2 */
#define   XYTAUTODL 28  /* SET TERMINAL AUTODOWNLOAD */
#define    TAD_OFF     0 /*    OFF */
#define    TAD_ON      1 /*    ON  */
#define    TAD_K       2 /*    KERMIT */
#define    TAD_Z       3 /*    ZMODEM */
#define    TAD_X_STR     0 /*    STRING */
#define    TAD_X_DETECT  1 /*    DETECTION ( PACKET, STRING ) */
#define    TAD_X_C0      2 /*    C0 CONFLICTS */
#define    TAD_ERR     4 /*    ERROR { STOP, CONTINUE } */
#define    TAD_ASK     5 /*    ASK (dialog) */
#define   XYTAUTOUL 29  /* SET TERMINAL AUTOUPLOAD   */
#ifdef OS2
#define   XYTATTBUG 30  /* SET TERM ATTR-BUG */
#define   XYTSTAT   31  /* SET TERM STATUSLINE */
#endif /* OS2 */
#define   XYTESC    32  /* SET TERM ESCAPE-CHARACTER */
#define   XYTCTRL   33  /* SET TERM CONTROLS */
#ifdef OS2
#define   XYTATTR   34  /* SET TERM ATTRIBUTE representation */
#define   XYTSGRC   35  /* SET TERM SGRCOLORS */
#endif /* OS2 */
#define   XYTLCS    36  /* SET TERM LOCAL-CHARACTER-SET */
#define   XYTRCS    37  /* SET TERM REMOTE-CHARACTER-SET */
#define   XYTUNI    38  /* SET TERM UNICODE */
#define   XYTKEY    39  /* SET TERM KEY */
#ifdef OS2
#define   XYTSEND   40  /* SET TERM SEND-DATA */
#define   XYTSEOB   41  /* SET TERM SEND-END-OF-BLOCK */
#define   XYTMBEL   42  /* SET TERM MARGIN-BELL */
#endif /* OS2 */
#define   XYTIDLE   43  /* SET TERM IDLE-SEND */
#ifdef OS2
#define   XYTKBMOD  44  /* SET TERM KEYBOARD-MODE */
#define   XYTUNX    45  /* SET TERM UNIX-MODE (DG) */
#define   XYTASCRL  46  /* SET TERM AUTOSCROLL */
#define   XYTAPAGE  47  /* SET TERM AUTOPAGE */
#endif /* OS2 */
#define   XYTRIGGER 48  /* SET TERM TRIGGER */
#ifdef OS2
#define   XYTPCTERM 49  /* SET TERM PCTERM */
#define   XYTOPTI   50  /* SET TERM SCREEN-OPTIMIZE */
#define   XYTSCNM   51  /* SET TERM SCREEN-MODE (DECSCNM) */
#endif /* OS2 */
#define   XYTPRN    52  /* SET TERM PRINT {AUTO, COPY, OFF} */
#ifdef OS2
#define   XYTSAC    53  /* SET TERM SPACING-ATTRIBUTE-CHARACTER (inv) */
#define   XYTSNIPM  54  /* SET TERM SNI-AUTOROLL */
#define   XYTSNISM  55  /* SET TERM SNI-SCROLLMODE */
#define   XYTKBDGL  56  /* SET TERM KBD-FOLLOWS-GL/GR */
#define   XYTVTLNG  57  /* SET TERM VT-LANGUAGE */
#define     VTL_NORTH_AM  1
#define     VTL_BRITISH   2
#define     VTL_BELGIAN   3
#define     VTL_FR_CAN    4
#define     VTL_DANISH    5
#define     VTL_FINNISH   6
#define     VTL_GERMAN    7
#define     VTL_DUTCH     8
#define     VTL_ITALIAN   9
#define     VTL_SW_FR    10
#define     VTL_SW_GR    11
#define     VTL_SWEDISH  12
#define     VTL_NORWEGIA 13
#define     VTL_FRENCH   14
#define     VTL_SPANISH  15
#define     VTL_PORTUGES 16
#define     VTL_HEBREW   19
#define     VTL_GREEK    22
#define     VTL_CANADIAN 28
#define     VTL_TURK_Q   29
#define     VTL_TURK_F   30
#define     VTL_HUNGARIA 31
#define     VTL_SLOVAK   33
#define     VTL_CZECH    34
#define     VTL_POLISH   35
#define     VTL_ROMANIAN 36
#define     VTL_SCS      38
#define     VTL_RUSSIAN  39
#define     VTL_LATIN_AM 40
#define   XYTVTNRC  58  /* SET TERM VT-NRC-MODE */
#define   XYTSNICC  59  /* SET TERM SNI-CH.CODE */
#define   XYTSNIFV  60  /* SET TERM SNI-FIRMWARE-VERSIONS */
#define   XYTURLHI  61  /* SET TERM URL-HIGHLIGHT */
#endif /* OS2 */
#define   XYTITMO   62  /* SET TERM IDLE-TIMEOUT */
#define   XYTIACT   63  /* SET TERM IDLE-ACTION  */
#define   XYTLSP    64  /* SET TERM LINE-SPACING */
#define   XYTLFD    65	/* SET TERM LF-DISPLAY   */

#define XYATTR 34       /* Attribute packets  */
#define XYSERV 35	/* Server parameters  */
#define   XYSERT 0      /*  Server timeout    */
#define   XYSERD 1	/*  Server display    */
#define   XYSERI 2      /*  Server idle       */
#define   XYSERP 3	/*  Server get-path   */
#define   XYSERL 4	/*  Server login      */
#define   XYSERC 5	/*  Server CD-Message */
#define   XYSERK 6	/*  Server keepalive  */
#define XYWIND 36       /* Window size */
#define XYXFER 37       /* Transfer */
#define   XYX_CAN 0	/*   Cancellation  */
#define   XYX_CSE 1	/*   Character-Set */
#define   XYX_LSH 2	/*   Locking-Shift */
#define   XYX_PRO 3	/*   Protocol      */
#define   XYX_MOD 4	/*   Mode          */
#define   XYX_DIS 5	/*   Display       */
#define   XYX_SLO 6	/*   Slow-start    */
#define   XYX_CRC 7	/*   CRC calculation */
#define   XYX_BEL 8	/*   Bell */
#define   XYX_PIP 9	/*   Pipes */
#define   XYX_INT 10    /*   Interruption */
#define   XYX_XLA 11    /*   (character-set) Translation On/Off */
#define   XYX_MSG 12	/*   Message */
#define   XYX_RPT 13	/*   Report */
#define XYLANG 38       /* Language */
#define XYCOUN 39       /* Count */
#define XYTAKE 40       /* Take */
#define XYUNCS 41       /* Unknown-character-set */
#define XYKEY  42       /* Key */
#define XYMACR 43       /* Macro */
#define XYHOST 44       /* Hostname on network */
#define XYNET  45       /* SET NETWORK things */

#define XYNET_D 99	/* NETWORK DIRECTORY */
#define XYNET_T 100	/* NETWORK TYPE */

#define XYCARR 46	/* Carrier */
#define XYXMIT 47       /* Transmit */

#define XYDIAL 48       /* Dial options */

/* And now we interrupt the flow to bring you lots of stuff about dialing */

#ifndef MAXTOLLFREE	/* Maximum number of toll-free area codes */
#define MAXTOLLFREE 8
#endif /* MAXTOLLFREE */

#ifndef MAXTPCC		/* Maximum Tone or Pulse dialing countries */
#define MAXTPCC 160
#endif /* MAXTPCC */

#ifndef MAXPBXEXCH	/* Maximum number of PBX exchanges */
#define MAXPBXEXCH 8
#endif /* MAXPBXEXCH */

#ifndef MAXLOCALAC
#define MAXLOCALAC 32
#endif /* MAXLOCALAC */

#ifndef MAXDNUMS
#ifdef BIGBUFOK
#define MAXDDIR 32	/* Maximum number of dialing directories */
#define MAXDNUMS 4095	/* Max numbers to fetch from dialing directories */
#else
#define MAXDDIR 12
#define MAXDNUMS 1024
#endif /* BIGBUFOK */
#endif /* MAXDNUMS */
/*
  IMPORTANT: In 5A(192), the old SET DIAL command was split into two commands:
  SET MODEM (for modem-related parameters) and SET DIAL (for dialing items).
  To preserve the old formats, etc, invisibly we keep one symbol space for
  both commands.
*/
#define  XYDHUP  0	/*   Dial Hangup */
#define  XYDINI  1      /*   MODEM (dial) Initialization string */
#define  XYDKSP  2      /*   MODEM (dial) Kermit-Spoof */
#define  XYDTMO  3      /*   Dial Timeout */
#define  XYDDPY  4      /*   Dial Display */
#define  XYDSPD  5      /*   Dial Speed matching */
#define  XYDMNP  6	/*   MODEM (dial) MNP negotiation enabled (obsolete) */
#define  XYDEC   7	/*   MODEM (dial) error correction enabled */
#define  XYDDC   8      /*   MODEM (dial) compression enabled */
#define  XYDHCM  9      /*   MODEM (dial) hangup-string (moved elsewhere) */
#define  XYDDIR 10	/*   Dial directory */
#define  XYDDIA 11	/*   MODEM (dial) dial-command */
#define  XYDMHU 12	/*   MODEM HANGUP (dial modem-hangup) */

#ifndef DEFMDHUP	/* Default MODEM HANGUP-METHOD */
#define DEFMDMHUP 1	/* 0 = RS232, 1 = modem command */
#endif /* DEFMDMHUP */

#define  XYDNPR 13      /*   Dial PREFIX */
#define  XYDSTR 14	/*   MODEM COMMAND (dial string) ... */

#define   XYDS_DC 0	/*    Data compression */
#define   XYDS_EC 1	/*    Error correction */
#define   XYDS_HU 2     /*    Hangup command */
#define   XYDS_HW 3     /*    Hardware flow control */
#define   XYDS_IN 4     /*    Init-string */
#define   XYDS_NF 5     /*    No flow control */
#define   XYDS_PX 6     /*    Prefix (no, this goes in SET DIAL) */
#define   XYDS_SW 7     /*    Software flow control */
#define   XYDS_DT 8     /*    Tone dialing command */
#define   XYDS_DP 9     /*    Pulse dialing command */
#define   XYDS_AN 10    /*    Autoanswer */
#define   XYDS_RS 11    /*    Reset */
#define   XYDS_MS 12    /*    Dial mode string */
#define   XYDS_MP 13    /*    Dial mode prompt */
#define   XYDS_SP 14	/*    Modem speaker */
#define   XYDS_VO 15	/*    Modem speaker volume */
#define   XYDS_ID 16	/*    Ignore dialtone */
#define   XYDS_I2 17	/*    Init string #2 */

#define   XYDM_A  9     /*    Method: Auto */
#define   XYDM_D  0     /*      Default */
#define   XYDM_T  2     /*      Tone */
#define   XYDM_P  3     /*      Pulse */

#define  XYDFC   15	/*   MODEM (dial) flow-control */
#define  XYDMTH  16	/*   Dial method */
#define  XYDESC  17     /*   MODEM (dial) escape-character */
#define  XYDMAX  18	/*   MODEM (dial) maximum interface speed */
#define  XYDCAP  19     /*   MODEM (dial) capabilities */
#define  XYDTYP  20	/*   MODEM TYPE */
#define  XYDINT  21	/*   DIAL retries */
#define  XYDRTM  22	/*   DIAL time between retries */
#define  XYDNAM  23	/*   MODEM NAME */
#define  XYDLAC  24	/*   DIAL (LOCAL-)AREA-CODE */
#define  XYDMCD  25	/*   MODEM CARRIER */

#define  XYDCNF  26	/*   DIAL CONFIRMATION */
#define  XYDCVT  27	/*   DIAL CONVERT-DIRECTORY */
#define  XYDIXP  28	/*   DIAL INTERNATIONAL-PREFIX */
#define  XYDIXS  29	/*   DIAL INTERNATIONAL-SUFFIX */
#define  XYDLCC  30	/*   DIAL LOCAL-COUNTRY-CODE */
#define  XYDLDP  31	/*   DIAL LONG-DISTANCE-PREFIX */
#define  XYDLDS  32	/*   DIAL LONG-DISTANCE-SUFFIX */
#define  XYDPXX  33	/*   DIAL PBX-EXCHANGE */
#define  XYDPXI  34	/*   DIAL PBX-INTERNAL-PREFIX */
#define  XYDPXO  35	/*   DIAL PBX-OUTSIDE-PREFIX */
#define  XYDSFX  36	/*   DIAL SUFFIX */
#define  XYDSRT  37	/*   DIAL SORT */
#define  XYDTFC  38	/*   DIAL TOLL-FREE-AREA-CODE */
#define  XYDTFP  39	/*   DIAL TOLL-FREE-PREFIX */
#define  XYDTFS  40	/*   DIAL TOLL-FREE-SUFFIX */
#define  XYDCON  41     /*   DIAL CONNECT */
#define  XYDRSTR 42     /*   DIAL RESTRICT */
#define  XYDRSET 43     /*   MODEM RESET */
#define  XYDLCP  44	/*   DIAL LOCAL-PREFIX */
#define  XYDLCS  45	/*   DIAL LOCAL-SUFFIX */
#define  XYDLLAC 46     /*   DIAL LC-AREA-CODES */
#define  XYDFLD  47	/*   DIAL FORCE LONG-DISTANCE */
#define  XYDSPK  48	/*   MODEM SPEAKER */
#define  XYDVOL  49	/*   MODEM VOLUME */
#define  XYDIDT  50	/*   IGNORE DIALTONE */
#define  XYDPAC  51	/*   PACING */
#define  XYDMAC  52     /*   MACRO */
#define  XYDPUCC 53	/*   PULSE-COUNTRIES */
#define  XYDTOCC 54	/*   TONE-COUNTRIES */
#define  XYDTEST 55	/*   TEST */

#define XYA_CID   1	/* SET ANSWER CALLER-ID */
#define XYA_RNG   2	/* SET ANSWER RINGS */

#define XYSESS 49       /* SET SESSION options */
#define XYBUF  50       /* Buffer length */
#define XYBACK 51	/* Background */
#define XYDFLT 52       /* Default */
#define XYDBL  53	/* Double */
#define XYCMD  54       /* COMMAND */
#define XYCASE 55       /* Case */
#define XYCOMP 56       /* Compression */
#define XYX25  57       /* X.25 parameter (ANYX25) */
#define XYPAD  58       /* X.3 PAD parameter (ANYX25) */
#define XYWILD 59       /* Wildcard expansion method */

#define WILD_OFF  0	/* Wildcard expansion off */
#define WILD_ON   1	/* Wildcard expansion on  */
#define WILD_KER  2	/* Wildcard expansion by Kermit */
#define WILD_SHE  3	/* Wildcard expansion by Shell */

#define XYSUSP 60       /* Suspend */
#define XYMAIL 61	/* Mail-Command */
#define XYPRIN 62	/* Print-Command */
#define XYQUIE 63	/* Quiet */
#define XYLCLE 64	/* Local-echo */
#define XYSCRI 65	/* SCRIPT command parameters */
#define XYMSGS 66       /* MESSAGEs ON/OFF */
#ifdef TNCODE
#define XYTEL  67	/* SET TELNET parameters */
#define  CK_TN_EC 0	/*  TELNET ECHO */
#define  CK_TN_TT 1	/*  TELNET TERMINAL-TYPE */
#define  CK_TN_NL 2     /*  TELNET NEWLINE-MODE */
#define  CK_TN_BM 3     /*  TELNET BINARY-MODE */
#define  CK_TN_BUG 4    /*  TELNET BUG */
#define  CK_TN_ENV 5    /*  TELNET ENVIRONMENT */
#define    TN_ENV_USR  0 /*    VAR USER */
#define    TN_ENV_JOB  1 /*    VAR JOB */
#define    TN_ENV_ACCT 2 /*    VAR ACCT */
#define    TN_ENV_PRNT 3 /*    VAR PRINTER */
#define    TN_ENV_SYS  4 /*    VAR SYSTEMTYPE */
#define    TN_ENV_DISP 5 /*    VAR DISPLAY */
#define    TN_ENV_UVAR 6 /*    USERVAR */
#define    TN_ENV_LOC  7 /*    USERVAR LOCATION */
#define    TN_ENV_ON  98 /*    ON (enabled) */
#define    TN_ENV_OFF 99 /*    OFF (disabled) */
#define  CK_TN_LOC 6    /*  TELNET LOCATION */
#define  CK_TN_AU  7    /*  TELNET AUTHENTICATION */
#define    TN_AU_FWD   4 /*    AUTH FORWARD */
#define    TN_AU_TYP   5 /*    AUTH TYPE */
#define      AUTH_NONE 0 /*      AUTH NONE */
#define      AUTH_KRB4 1 /*      AUTH Kerberos IV */
#define      AUTH_KRB5 2 /*      AUTH Kerberos V */
#define      AUTH_SSL  7 /*      AUTH Secure Sockets Layer */
#define      AUTH_TLS 98 /*      AUTH Transport Layer Security */
#define      AUTH_SRP  5 /*      AUTH Secure Remote Password */
#define      AUTH_NTLM 15 /*      AUTH NT Lan Manager */
#define      AUTH_AUTO 99 /*     AUTH AUTOMATIC */
#define    TN_AU_HOW  8  /*    AUTH HOW FLAG */
#define    TN_AU_ENC  9  /*    AUTH ENCRYPT FLAG */
#define  CK_TN_ENC 8    /*  TELNET ENCRYPTION */
#define    TN_EN_TYP   4 /*      ENCRYPT TYPE */
#define    TN_EN_START 5 /*      ENCRYPT START */
#define    TN_EN_STOP  6 /*      ENCRYPT STOP  */
#define  CK_TN_IKS 9    /*  TELNET KERMIT-SERVER */
#define  CK_TN_RE  10   /*  TELNET REMOTE-ECHO */
#define  CK_TN_TLS 11   /*  TELNET START_TLS */
#define  CK_TN_XD  12   /*  TELNET XDISPLOC */
#define  CK_TN_NAWS 13  /*  TELNET NAWS */
#define  CK_TN_WAIT 14  /*  TELNET WAIT-FOR-NEGOTIATIONS */
#define  CK_TN_SGA  15  /*  TELNET SGA */
#define  CK_TN_CLIENT 16  /* TELNET CLIENT */
#define  CK_TN_SERVER 17  /* TELNET SERVER */
#define  CK_TN_PHR    18  /* TELNET PRAGMA-HEARTBEAT */
#define  CK_TN_PLG    19  /* TELNET PRAGMA-LOGON */
#define  CK_TN_PSP    20  /* TELNET PRAGMA-SSPI */
#define  CK_TN_SAK    21  /* TELNET IBM SAK */
#define  CK_TN_FLW    22  /* TELNET LFLOW */
#define  CK_TN_XF     23  /* TELNET TRANSFER-MODE */
#define  CK_TN_PUID   24  /* TELNET PROMPT-FOR-USERID */
#define  CK_TN_NE     25  /* TELNET NO-ENCRYPT-DURING-XFER */
#define  CK_TN_CPC    26  /* TELNET COM-PORT-CONTROL */
#define  CK_TN_DB     27  /* TELNET DEBUG */
#define  CK_TN_FX     28  /* TELNET FORWARD_X */
#define  CK_TN_DL     29  /* TELNET DELAY-SB */
#define  CK_TN_SFU    30  /* TELNET SFU-COMPATIBILITY */
#define  CK_TN_LOG    31  /* TELNET LOGOUT */
#endif /* TNCODE */
#define XYOUTP 68	/* OUTPUT command parameters */
#define   OUT_PAC 0	/*   OUTPUT PACING */
#define   OUT_ESC 1	/*   OUTPUT SPECIAL-ESCAPES */
#define XYEXIT  69	/* SET EXIT */
#define XYPRTR  70	/* SET PRINTER */
#define XYFPATH 71	/* PATHNAME */

#ifdef OS2
#define XYMOUSE 72	/* MOUSE SUPPORT */
#define  XYM_ON     0   /* Mouse ON/OFF        */
#define  XYM_BUTTON 1   /* Define Mouse Events */
#define  XYM_CLEAR  2   /* Clear Mouse Events  */
#define  XYM_DEBUG  3   /* Debug Mode ON/OFF */
/* These must match similar definitions in ckokey.h */
#define   XYM_B1 0      /* Mouse Button One */
#define   XYM_B2 1      /* Mouse Button Two */
#define   XYM_B3 2      /* Mouse Button Three */
#define   XYM_ALT   1     /* Alt */
#define   XYM_CTRL  2     /* Ctrl */
#define   XYM_SHIFT 4     /* Shift */
#define   XYM_C1    0     /* Single Click */
#define   XYM_C2    8     /* Double Click */
#define   XYM_DRAG  16    /* Drag Event */
#endif /* OS2 */

#define XYBELL 73   /* BELL */

#ifdef OS2
#define XYPRTY     74   /* Thread Priority Level */
#define   XYP_IDLE  1
#define   XYP_REG   2
#define   XYP_SRV   4
#define   XYP_RTP   3
#endif /* OS2 */

#define XYALRM     75	/* SET ALARM */
#define XYPROTO    76	/* SET PROTOCOL */
#define XYPREFIX   77   /* SET PREFIXING */
#define XYLOGIN    78   /* Login info for script programs... */

#define  LOGI_UID   0	/* User ID  */
#define  LOGI_PSW   1	/* Password */
#define  LOGI_PRM   2	/* Prompt   */

#define XYSTARTUP  79    /* Startup file */
#define XYTMPDIR   80    /* Temporary directory */

#ifdef OS2
#define XYTAPI     81    /* Microsoft Telephone API options */
#define   XYTAPI_CFG     1  /* TAPI Configure-Line Dialog */
#define   XYTAPI_DIAL    2  /* TAPI Dialing-Properties Dialog */
#define   XYTAPI_LIN     3  /* TAPI Line */
#define   XYTAPI_LOC     4  /* TAPI Location */
#define   XYTAPI_PASS    5  /* TAPI Passthrough */
#define   XYTAPI_CON     6  /* TAPI Conversions */
#define   XYTAPI_LGHT    7  /* TAPI Modem Lights */
#define   XYTAPI_PRE     8  /* TAPI Pre-dialing Terminal */
#define   XYTAPI_PST     9  /* TAPI Post-dialing Terminal */
#define   XYTAPI_INA    10  /* TAPI Inactivity Timeout */
#define   XYTAPI_BNG    11  /* TAPI Wait for Credit Card Tone */
#define   XYTAPI_MAN    12  /* TAPI Manual Dialing */
#define   XYTAPI_USE    13  /* TAPI Use Line Config settings */
#endif /* OS2 */

#ifdef TCPSOCKET
#define XYTCP  82       /* TCP options */
#define  XYTCP_NODELAY   1  /* No Delay */
#define  XYTCP_SENDBUF   2  /* Send Buffer Size */
#define  XYTCP_LINGER    3  /* Linger */
#define  XYTCP_RECVBUF   4  /* Receive Buffer Size */
#define  XYTCP_KEEPALIVE 5  /* Keep Alive packets */
#define  XYTCP_UCX       6  /* UCX 2.0 port swabbing bug */
#define  XYTCP_NAGLE     7  /* Delay - inverse of 1 */
#define  XYTCP_RDNS      8  /* Reverse DNS lookup */
#define  XYTCP_ADDRESS   9  /* Set preferred IP Address */
#define  XYTCP_DNS_SRV  10  /* Use DNS Service Records */
#define  XYTCP_DONTROUTE 11 /* Dont Route */
#define  XYTCP_SOCKS_SVR 12 /* Name of Socks Server */
#define  XYTCP_HTTP_PROXY 13 /* Name/Port of HTTP Proxy Server */
#define  XYTCP_SOCKS_NS  14 /* Name of Socks Name Server */
#endif /* TCPSOCKET */

#ifdef OS2
#define XYMSK  83       /* MS-DOS Kermit compatibility options */
#define  MSK_COLOR 0    /*  Terminal color handling   */
#define  MSK_KEYS  1    /*  SET KEY uses MSK keycodes */
#define  MSK_REN   2    /*  File renaming uses 8.3 notation always */
#endif /* OS2 */

#define XYDEST  84	/* SET DESTINATION as in MS-DOS Kermit */

#ifdef OS2
#define XYWIN95 85	/* SET WIN95 work arounds  */
#define   XYWKEY 0	/*    Keyboard translation */
#define   XYWAGR 1      /*    Alt-Gr               */
#define   XYWOIO 2      /*    Overlapped I/O       */
#define   XYWLUC 3	/*    Lucida Console substitutions */
#define   XYWSELECT 4   /*    Select on Write Bug */
#define   XYW8_3 5      /*    Use 8.3 filenames? */
#define   XYWPOPUP 6    /*    Use Popups?  */
#define   XYWHSL 7      /*    Horz Scan Line substitutions */
#define XYDLR   86 	/* SET K95 DIALER work arounds */
#define XYTITLE 87	/* SET TITLE of window */
#endif /* OS2 */

#define XYIGN   88	/* SET IGNORE-CHARACTER */
#define XYEDIT  89      /* SET EDITOR */
#define XYFLTR  90      /* SET { SEND, RECEIVE } FILTER */
#define XYBROWSE 91     /* SET BROWSER */
#define XYEOF    92     /* EOF (= FILE EOF) */
#ifdef OS2
#define XYBDCP   93     /* BPRINTER */
#endif /* OS2 */
#define XYFLAG   94	/* FLAG */
#define XYLIMIT  95     /* SESSION-LIMIT */
#define XYINIL   96     /* Protocol negotiation string max length */
#define XYRELY   97     /* RELIABLE */
#define XYSTREAM 98     /* STREAMING */
#define XYTLOG   99     /* TRANSACTION-LOG */
#define XYCLEAR 100     /* CLEARCHANNEL */
#define XYAUTH  101	/* AUTHENTICATION */

#ifdef TNCODE
#define XYKRBPR   0	/* Kerberos Principal */
#define XYKRBRL   1	/* Kerberos Realm */
#define XYKRBCC   2	/* Kerberos 5 Credentials-Cache */
#define XYKRBSRV  3     /* Kerberos Service Name */
#define XYKRBDBG  4     /* Kerberos Debugging */
#define XYKRBLIF  5     /* Kerberos Lifetime */
#define XYKRBPRE  6     /* Kerberos 4 Preauth */
#define XYKRBINS  7     /* Kerberos 4 Instance */
#define XYKRBFWD  8     /* Kerberos 5 Forwardable */
#define XYKRBPRX  9     /* Kerberos 5 Proxiable */
#define XYKRBRNW  10    /* Kerberos 5 Renewable lifetime */
#define XYKRBGET  11    /* Kerberos Auto-Get-TGTs */
#define XYKRBDEL  12    /* Kerberos Auto-Destroy-TGTs */
#define   KRB_DEL_NO  0 /*   Kerberos No Auto Destroy */
#define   KRB_DEL_CL  1 /*   Kerberos Auto Destory on Close */
#define   KRB_DEL_EX  2 /*   Kerberos Auto Destroy on Exit  */
#define XYKRBK5K4 13    /* Kerberos 5 Get K4 Tickets */
#define XYKRBPRM  14    /* Kerberos 4/5 Prompt */
#define XYKRBADR  15    /* Kerberos 4/5 CheckAddrs */
#define XYKRBNAD  16    /* Kerberos 5 No Addresses */
#define XYKRBADD  17    /* Kerberos 5 Address List */
#define XYKRBKTB  18    /* Kerberos 4/5 Key Table */
#define XYSRPPRM   0    /* SRP Prompt */
#define XYSSLRCFL  0    /* SSL/TLS RSA Certs file */
#define XYSSLCOK   1    /* SSL/TLS Certs-Ok flag */
#define XYSSLCRQ   2    /* SSL/TLS Certs-Required flag */
#define XYSSLCL    3    /* SSL/TLS Cipher List */
#define XYSSLDBG   4    /* SSL/TLS Debug flag */
#define XYSSLRKFL  5    /* SSL/TLS RSA Key File */
#define XYSSLLFL   6    /* SSL/TLS Log File */
#define XYSSLON    7    /* SSL/TLS Only flag */
#define XYSSLSEC   8    /* SSL/TLS Secure flag */
#define XYSSLVRB   9    /* SSL/TLS Verbose flag */
#define XYSSLVRF  10    /* SSL/TLS Verify flag */
#define XYSSLDUM  11    /* SSL/TLS Dummy flag */
#define XYSSLDCFL 12    /* SSL/TLS DSA Certs file */
#define XYSSLDKFL 13    /* SSL/TLS DH Certs file */
#define XYSSLDPFL 14    /* SSL/TLS DH Param file */
#define XYSSLCRL  15    /* SSL/TLS CRL file */
#define XYSSLCRLD 16    /* SSL/TLS CRL dir */
#define XYSSLVRFF 17    /* SSL/TLS Verify file */
#define XYSSLVRFD 18    /* SSL/TLS Verify dir */
#define XYSSLRND  19    /* SSL/TLS Random file */
#define XYSSLDCCF 20    /* SSL/TLS DSA Certs Chain File */
#define XYSSLRCCF 21    /* SSL/TLS RSA Certs Chain File */

/* The following must be powers of 2 for a bit mask */

#define  XYKLCEN  1	/* Kerberos List Credentials: Encryption */
#define  XYKLCFL  2	/* Kerberos List Credentials: Flags */
#define  XYKLCAD  4     /* Kerberos List Credentials: Addresses */
#endif /* TNCODE */

#define XYFUNC   102	/* SET FUNCTION */

#define  FUNC_DI   0	/* FUNCTION DIAGNOSTICS */
#define  FUNC_ER   1    /* FUNCTION ERROR */

#define XYFTP    103	/* FTP application */
#define XYSLEEP  104	/* SLEEP / PAUSE options */
#define XYSSH    105	/* SSH options */
#define XYTELOP  106    /* TELNET OPTIONS (TELOPT) */
#define XYCD     107    /* SET CD */
#define XYCSET   108	/* CHARACTER-SET */
#define XYSTOP   109    /* STOP-BITS */
#define XYSERIAL 110	/* SERIAL */
#define XYDISC   111    /* CLOSE-ON-DISCONNECT */
#define XYOPTS   112    /* OPTIONS */
#define XYQ8FLG  113    /* Q8FLAG (invisible) */
#define XYTIMER  114    /* TIMER */
#define XYFACKB  115    /* F-ACK-BUG */
#define XYBUP    116    /* SET SEND/RECEIVE BACKUP */
#define XYMOVE   117	/* SET SEND/RECEIVE MOVE-TO */
#define XYRENAME 118	/* SET SEND/RECEIVE RENAME-TO */
#define XYHINTS  119    /* SET HINTS */
#define XYEVAL   120    /* SET EVALUATE */
#define XYFACKP  121    /* F-ACK-PATH */
#define XYSYSL   122    /* SysLog */
#define XYQNXPL  123	/* QNX Port Lock */
#define XYIKS    124    /* SET IKS ... */
#define XYROOT   125    /* SET ROOT */
#define XYFTPX   126    /* SET FTP */
#define XYSEXP   127	/* SET SEXP */
#define XYGPR    128    /* SET GET-PUT-REMOTE */
#define XYLOCUS  129	/* SET LOCUS */
#define XYGUI    130	/* SET GUI */
#define XYANSWER 131    /* SET ANSWER */
#define XYMATCH  132    /* SET MATCHDOT */
#define XYSFTP   133    /* SET SFTP */
#define XY_REN   134    /* SET RENAME */
#define XYEXTRN  135    /* SET EXTERNAL-PROTOCOL */
#define XYVAREV  136    /* SET VARIABLE-EVALUATION */

/* End of SET commands */

/* S-Expressions -- floating-point support required */

#ifndef CKFLOAT
#ifndef NOSEXP
#define NOSEXP
#endif /* NOSEXP */
#endif /* CKFLOAT */

/* Maximum number of elements in an S-Expression */

#ifndef NOSEXP
#ifndef SEXPMAX
#ifdef BIGBUFOK
#define SEXPMAX 1024
#else
#define SEXPMAX 32
#endif /* BIGBUFOK */
#endif /* SEXPMAX */
#endif /* NOSEXP */

#ifdef ANYX25
/* PAD command parameters */

#define XYPADL 0        /* clear virtual call */
#define XYPADS 1        /* status of virtual call */
#define XYPADR 2        /* reset of virtual call */
#define XYPADI 3        /* send an interrupt packet */

/* Used with XYX25... */
#define XYUDAT 0       /* X.25 call user data */
#define XYCLOS 1       /* X.25 closed user group call */
#define XYREVC 2       /* X.25 reverse charge call */
#endif /* ANYX25 */

#ifdef OS2
/* SET PRINTER switches */

#define PRN_OUT 0			/* Output only */
#define PRN_BID 1			/* Bidirectional */
#define PRN_DOS 2			/* DOS device */
#define PRN_WIN 3			/* Windows queue */
#define PRN_TMO 4			/* Timeout */
#define PRN_TRM 5			/* Terminator */
#define PRN_SEP 6			/* Separator */
#define PRN_SPD 7			/* COM-port speed */
#define PRN_FLO 8			/* COM-port flow control */
#define PRN_PAR 9			/* COM-port parity */
#define PRN_NON 10			/* No printer */
#define PRN_FIL 11			/* Filename */
#define PRN_PIP 12			/* Pipename */
#define PRN_PS  13                      /* Text to PS */
#define PRN_WID 14                      /* PS Width */
#define PRN_LEN 15                      /* PS Length */
#define PRN_RAW 16                      /* Non-PS */
#define PRN_CS  17                      /* Character Set */
#define PRN_MAX 17			/* Number of switches defined */

/* Printer types */

#define PRT_DOS 0			/* DOS */
#define PRT_WIN 1			/* Windows Queue */
#define PRT_FIL 2			/* File */
#define PRT_PIP 3			/* Pipe */
#define PRT_NON 4			/* None */

#define PRINTSWI
#endif /* OS2 */
#endif /* NOICP */

#ifndef NODIAL
/*
  Symbols for modem types, moved here from ckudia.c, May 1997, because now
  they are also used in some other modules.  The numbers MUST correspond to
  the ordering of entries within the modemp[] array.
*/
#ifdef MINIDIAL				/* Minimum dialer support */

#define         n_DIRECT         0	/* Direct connection -- no modem */
#define		n_CCITT		 1	/* CCITT/ITU-T V.25bis */
#define		n_HAYES		 2	/* Hayes 2400 */
#define		n_UNKNOWN	 3	/* Unknown */
#define         n_UDEF           4	/* User-Defined */
#define         n_GENERIC        5	/* Generic High Speed */
#define         n_ITUTV250       6	/* ITU-T V.250 */
#define		MAX_MDM		 6	/* Number of modem types */

#else					/* Full-blown dialer support */

#define         n_DIRECT         0	/* Direct connection -- no modem */
#define		n_ATTDTDM	 1
#define         n_ATTISN         2
#define		n_ATTMODEM	 3
#define		n_CCITT		 4
#define		n_CERMETEK	 5
#define		n_DF03		 6
#define		n_DF100		 7
#define		n_DF200		 8
#define		n_GDC		 9
#define		n_HAYES		10
#define		n_PENRIL	11
#define		n_RACAL		12
#define		n_UNKNOWN       13
#define		n_VENTEL	14
#define		n_CONCORD	15
#define		n_ATTUPC	16	/* aka UNIX PC and ATT7300 */
#define		n_ROLM          17      /* Rolm CBX DCM */
#define		n_MICROCOM	18	/* Microcoms in SX command mode */
#define         n_USR           19	/* Modern USRs */
#define         n_TELEBIT       20      /* Telebits of all kinds */
#define         n_DIGITEL       21	/* Digitel DT-22 (CCITT variant) */
#define         n_H_1200        22	/* Hayes 1200 */
#define		n_H_ULTRA       23	/* Hayes Ultra and maybe Optima */
#define		n_H_ACCURA      24	/* Hayes Accura and maybe Optima */
#define         n_PPI           25	/* Practical Peripherals */
#define         n_DATAPORT      26	/* AT&T Dataport */
#define         n_BOCA          27	/* Boca */
#define		n_MOTOROLA      28	/* Motorola Fastalk or Lifestyle */
#define		n_DIGICOMM	29	/* Digicomm Connection */
#define		n_DYNALINK      30	/* Dynalink 1414VE */
#define		n_INTEL		31	/* Intel 14400 Faxmodem */
#define		n_UCOM_AT	32	/* Microcoms in AT mode */
#define		n_MULTI		33	/* Multitech MT1432 */
#define		n_SUPRA		34	/* SupraFAXmodem */
#define	        n_ZOLTRIX	35	/* Zoltrix */
#define		n_ZOOM		36	/* Zoom */
#define		n_ZYXEL		37	/* ZyXEL */
#define         n_TAPI          38	/* TAPI Line modem - whatever it is */
#define         n_TBNEW         39	/* Newer Telebit models */
#define		n_MAXTECH       40	/* MaxTech XM288EA */
#define         n_UDEF          41	/* User-Defined */
#define         n_RWV32         42	/* Generic Rockwell V.32 */
#define         n_RWV32B        43	/* Generic Rockwell V.32bis */
#define         n_RWV34         44	/* Generic Rockwell V.34 */
#define		n_MWAVE		45	/* IBM Mwave Adapter */
#define         n_TELEPATH      46	/* Gateway Telepath */
#define         n_MICROLINK     47	/* MicroLink modems */
#define         n_CARDINAL      48	/* Cardinal modems */
#define         n_GENERIC       49      /* Generic high-speed */
#define         n_XJACK         50	/* Megahertz X-Jack */
#define         n_SPIRITII      51	/* Quickcomm Spirit II */
#define         n_MONTANA       52	/* Motorola Montana */
#define         n_COMPAQ        53	/* Compaq Data+Fax Modem */
#define         n_FUJITSU       54	/* Fujitsu Fax/Modem Adpater */
#define         n_MHZATT        55	/* Megahertz AT&T V.34 */
#define         n_SUPRASON      56	/* SupraSonic */
#define         n_BESTDATA      57	/* Best Data */
#define         n_ATT1900       58      /* AT&T STU III Model 1900 */
#define         n_ATT1910       59      /* AT&T STU III Model 1910 */
#define         n_KEEPINTOUCH   60	/* AT&T KeepinTouch */
#define         n_USRX2         61	/* USR XJ-1560 X2 56K */
#define         n_ROLMAT        62	/* Rolm with AT command set */
#define		n_ATLAS         63      /* Atlas / Newcom ixfC 33.6 */
#define         n_CODEX         64	/* Motorola Codex 326X Series */
#define         n_MT5634ZPX     65	/* Multitech MT5634ZPX */
#define         n_ULINKV250     66	/* Microlink ITU-T V.250 56K */
#define         n_ITUTV250      67	/* Generic ITU-T V.250 */
#define         n_RWV90         68	/* Generic Rockwell V.90 */
#define         n_SUPRAX        69      /* Diamond Supra Express V.90 */
#define         n_LUCENT        70      /* Lucent Venus chipset */
#define         n_PCTEL         71      /* PCTel chipset */
#define         n_CONEXANT      72      /* Conexant modem family */
#define		n_ZOOMV34	73	/* Zoom */
#define		n_ZOOMV90	74	/* Zoom */
#define         n_ZOOMV92       75      /* ZOOM V.92 */
#define         n_MOTSM56       76	/* Motorola SM56 chipset */
#define		MAX_MDM		76	/* Number of modem types */

#endif /* MINIDIAL */
#endif /* NODIAL */

#ifndef NOICP
/* SHOW command symbols */

#define SHPAR 0				/* Parameters */
#define SHVER 1				/* Versions */
#define SHCOM 2				/* Communications */
#define SHPRO 3				/* Protocol */
#define SHFIL 4				/* File */
#define SHLNG 5				/* Language */
#define SHCOU 6				/* Count */
#define SHMAC 7				/* Macros */
#define SHKEY 8				/* Key */
#define SHSCR 9				/* Scripts */
#define SHSPD 10			/* Speed */
#define SHSTA 11			/* Status */
#define SHSER 12			/* Server */
#define SHXMI 13			/* Transmit */
#define SHATT 14			/* Attributes */
#define SHMOD 15			/* Modem */
#define SHDFLT 16			/* Default (as in VMS) */
#define SHVAR 17			/* Show global variables */
#define SHARG 18			/* Show macro arguments */
#define SHARR 19			/* Show arrays */
#define SHBUI 20			/* Show builtin variables */
#define SHFUN 21			/* Show functions */
#define SHPAD 22			/* Show (X.25) PAD */
#define SHTER 23			/* Show terminal settings */
#define SHESC 24			/* Show escape character */
#define SHDIA 25			/* Show DIAL parameters */
#define SHNET 26			/* Show network parameters */
#define SHLBL 27			/* Show VMS labeled file parameters */
#define SHSTK 28			/* Show stack, MAC debugging */
#define SHCSE 29			/* Show character sets */
#define SHFEA 30			/* Show features */
#define SHCTL 31			/* Show control-prefix table */
#define SHEXI 32			/* Show EXIT items */
#define SHPRT 33			/* Show printer */
#define SHCMD 34			/* Show command parameters */
#define SHKVB 35			/* Show \Kverbs */
#define SHMOU 36			/* Show Mouse (like Show Key) */
#define SHTAB 37			/* Show Tabs (OS/2) */
#define SHVSCRN 38			/* Show Virtual Screen (OS/2) */
#define SHALRM  39			/* ALARM */
#define SHSFL   40			/* SEND-LIST */
#define SHUDK   41                      /* DEC VT UDKs (OS/2) */
#define SHDBL   42			/* DOUBLE/IGNORE characters */
#define SHEDIT    43			/* EDITOR */
#define SHBROWSE  44			/* BROWSER */
#define SHTAPI    45			/* TAPI */
#define SHTAPI_L  46			/* TAPI Location */
#define SHTAPI_M  47			/* TAPI Modem Properties */
#define SHTAPI_C  48			/* TAPI Comm Properties  */
#define SHTEL     49			/* SHOW TELNET */
#define SHINP     50			/* SHOW INPUT */
#define SHTRIG    51			/* SHOW TRIGGER */
#define SHLOG     52			/* SHOW LOGS */
#define SHOUTP    53			/* SHOW OUTPUT */
#define SHOPAT    54			/* SHOW PATTERNS */
#define SHOSTR    55			/* SHOW STREAMING */
#define SHOAUTH   56			/* SHOW AUTHENTICATION */
#define SHOFTP    57			/* SHOW FTP */
#define SHTOPT    58                    /* SHOW TELOPT */
#define SHXOPT    59			/* SHOW EXTENDED-OPTIONS */
#define SHCD      60			/* SHOW CD */
#define SHASSOC   61			/* SHOW ASSOCIATIONS */
#define SHCONNX   62			/* SHOW CONNECTION */
#define SHOPTS    63			/* SHOW OPTIONS */
#define SHOFLO    64			/* SHOW FLOW-CONTROL */
#define SHOXFER   65			/* SHOW TRANSFER */
#define SHTCP     66                    /* SHOW TCP */
#define SHHISTORY 67			/* SHOW (command) HISTORY */
#define SHSEXP    68			/* SHOW SEXPRESSIONS */
#define SHOSSH    69			/* SHOW SSH */
#define SHOIKS    70                    /* SHOW IKS */
#define SHOGUI    71			/* SHOW GUI (K95) */
#define SHOREN    72			/* SHOW RENAME */

/* REMOTE command symbols */

#define XZCPY  0	/* Copy */
#define XZCWD  1	/* Change Working Directory */
#define XZDEL  2	/* Delete */
#define XZDIR  3	/* Directory */
#define XZHLP  4	/* Help */
#define XZHOS  5	/* Host */
#define XZKER  6	/* Kermit */
#define XZLGI  7	/* Login */
#define XZLGO  8	/* Logout */
#define XZMAI  9	/* Mail <-- wrong, this should be top-level */
#define XZMOU 10	/* Mount */
#define XZMSG 11	/* Message */
#define XZPRI 12	/* Print */
#define XZREN 13	/* Rename */
#define XZSET 14	/* Set */
#define XZSPA 15	/* Space */
#define XZSUB 16	/* Submit */
#define XZTYP 17	/* Type */
#define XZWHO 18	/* Who */
#define XZPWD 19	/* Print Working Directory */
#define XZQUE 20	/* Query */
#define XZASG 21	/* Assign */
#define XZMKD 22	/* mkdir */
#define XZRMD 23	/* rmdir */
#define XZXIT 24	/* Exit */
#define XZCDU 25        /* CDUP */

/* SET INPUT command parameters */

#define IN_DEF  0			/* Default timeout */
#define IN_TIM  1			/* Timeout action */
#define IN_CAS  2			/* Case (matching) */
#define IN_ECH  3			/* Echo */
#define IN_SIL  4			/* Silence */
#define IN_BUF  5			/* Buffer size */
#define IN_PAC  6                       /* Input Pacing (debug) */
#define IN_TRM  7			/* Input Terminal Display */
#define IN_ADL  8			/* Input autodownload */
#define IN_PAT  9			/* Pattern to match */
#define IN_ASG 10 			/* Assign matching text to variable */
#define IN_CAN 11			/* Keyboard cancellation of INPUT */
#define IN_SCA 12			/* Timeout scaling */

/* ENABLE/DISABLE command parameters */

#define EN_ALL  0			/* ALL */
#define EN_CWD  1			/* CD/CWD */
#define EN_DIR  2			/* DIRECTORY */
#define EN_FIN  3			/* FINISH */
#define EN_GET  4			/* GET */
#define EN_HOS  5			/* HOST command */
#define EN_KER  6			/* KERMIT command */
#define EN_LOG  7			/* LOGIN */
#define EN_SEN  8			/* SEND */
#define EN_SET  9			/* SET */
#define EN_SPA 10			/* SPACE */
#define EN_TYP 11			/* TYPE */
#define EN_WHO 12			/* WHO, finger */
#define EN_DEL 13			/* Delete */
#define EN_BYE 14			/* BYE (as opposed to FINISH) */
#define EN_QUE 15			/* QUERY */
#define EN_ASG 16			/* ASSIGN */
#define EN_CPY 17			/* COPY */
#define EN_REN 18			/* RENAME */
#define EN_RET 19			/* RETRIEVE */
#define EN_MAI 20			/* MAIL */
#define EN_PRI 21			/* PRINT */
#define EN_MKD 22			/* MKDIR */
#define EN_RMD 23			/* RMDIR */
#define EN_XIT 24			/* EXIT */
#define EN_ENA 25			/* ENABLE */
#endif /* NOICP */

#ifndef NOICP
/* CLEAR command symbols */
#define CLR_DEV    1			/* Clear Device Buffers */
#define CLR_INP    2			/* Clear Input Buffers */
#define CLR_BTH    CLR_DEV|CLR_INP	/* Clear Device and Input */
#define CLR_SCL    4			/* Clear Scrollback buffer */
#define CLR_CMD    8			/* Clear Command Screen */
#define CLR_TRM   16			/* Clear Terminal Screen */
#define CLR_DIA   32			/* Clear Dial Status */
#define CLR_SFL   64			/* Clear Send-File-List */
#define CLR_APC  128			/* Clear APC */
#define CLR_ALR  256			/* Clear Alarm */
#define CLR_TXT  512			/* Clear text-patterns */
#define CLR_BIN 1024			/* Clear binary-patterns */
#define CLR_SCR 2048			/* Clear screen */
#define CLR_KBD 4096			/* Clear keyboard buffer */
#endif /* NOICP */

/* Symbols for logs */

#define LOGD 0				/* Debugging */
#define LOGP 1				/* Packets */
#define LOGS 2				/* Session */
#define LOGT 3				/* Transaction */
#define LOGX 4				/* Screen */
#define LOGR 5				/* The "open read" file */
#define LOGW 6				/* The "open write/append" file */
#define LOGE 7				/* Error (e.g. stderr) */
#define LOGM 8				/* The dialing log */

#ifndef NOSPL
/* Symbols for builtin variables */

#define VN_ARGC 0			/* ARGC */
#define VN_COUN 1			/* COUNT */
#define VN_DATE 2			/* DATE */
#define VN_DIRE 3			/* DIRECTORY */
#define VN_ERRO 4			/* ERRORLEVEL */
#define VN_TIME 5			/* TIME */
#define VN_VERS 6			/* VERSION */
#define VN_IBUF 7			/* INPUT buffer */
#define VN_SUCC 8			/* SUCCESS flag */
#define VN_LINE 9			/* LINE */
#define VN_ARGS 10			/* Program command-line arg count */
#define VN_SYST 11			/* System type */
#define VN_SYSV 12			/* System version */
#define VN_RET  13			/* RETURN value */
#define VN_FILE 14			/* Most recent filespec */
#define VN_NDAT 15			/* Numeric date yyyy/mm/dd */
#define VN_HOME 16			/* Home directory */
#define VN_SPEE 17			/* Transmission speed */
#define VN_HOST 18			/* Host name */
#define VN_TTYF 19			/* TTY file descriptor (UNIX only) */
#define VN_PROG 20			/* Program name */
#define VN_NTIM 21			/* NTIME */
#define VN_FFC  22			/* Characters in last file xferred */
#define VN_TFC  23			/* Chars in last file group xferred */
#define VN_CPU  24			/* CPU type */
#define VN_CMDL 25			/* Command level */
#define VN_DAY  26                      /* Day of week, string */
#define VN_NDAY 27                      /* Day of week, numeric */
#define VN_LCL  28			/* Local (vs) remote mode */
#define VN_CMDS 29			/* Command source */
#define VN_CMDF 30			/* Command file name */
#define VN_MAC  31			/* Macro name */
#define VN_EXIT 32			/* Exit status */
#define VN_ICHR 33			/* INPUT character */
#define VN_ICNT 34			/* INPUT count */
#define VN_PRTY 35			/* Current parity */
#define VN_DIAL 36			/* DIAL status */
#define VN_KEYB 37			/* Keyboard type */
#define VN_CPS  38			/* Chars per second, last transfer */
#define VN_RPL  39			/* Receive packet length */
#define VN_SPL  40			/* Send packet length */
#define VN_MODE 41			/* Transfer mode (text, binary) */
#define VN_REXX 42			/* Rexx return value */
#define VN_NEWL 43			/* Newline character or sequence */
#define VN_COLS 44			/* Columns on console screen */
#define VN_ROWS 45			/* Rows on console screen */
#define VN_TTYP 46			/* Terminal type */
#define VN_MINP 47			/* MINPUT result */
#define VN_CONN 48			/* Connection type */
#define VN_SYSI 49			/* System ID */
#define VN_TZ   50			/* Timezone */
#define VN_SPA  51			/* Space */
#define VN_QUE  52			/* Query */
#define VN_STAR 53			/* Startup directory */
#define VN_CSET 54			/* Local character set */
#define VN_MDM  55			/* Modem type */
#define VN_EVAL 56			/* Most recent EVALUATE result */

#define VN_D_CC 57			/* DIAL COUNTRY-CODE */
#define VN_D_AC 58			/* DIAL AREA-CODE */
#define VN_D_IP 59			/* DIAL INTERNATIONAL-PREFIX */
#define VN_D_LP 60			/* DIAL LD-PREFIX */

#define VN_UID  61
#define VN_PWD  62
#define VN_PRM  63

#define VN_PROTO 64			/* Protocol */
#define VN_DLDIR 65			/* Download directory */

#define VN_M_AAA 66			/* First MODEM one */
#define VN_M_INI 66			/* Modem init string */
#define VN_M_DCM 67			/* Modem dial command */
#define VN_M_DCO 68			/* Modem data compression on */
#define VN_M_DCX 69			/* Modem data compression off */
#define VN_M_ECO 70			/* Modem error correction on */
#define VN_M_ECX 71			/* Modem error correction off */
#define VN_M_AAO 72			/* Modem autoanswer on */
#define VN_M_AAX 73			/* Modem autoanswer off */
#define VN_M_HUP 74			/* Modem hangup command */
#define VN_M_HWF 75			/* Modem hardware flow command */
#define VN_M_SWF 76			/* Modem software flow command */
#define VN_M_NFC 77			/* Modem no flow-control command */
#define VN_M_PDM 78			/* Modem pulse dialing mode */
#define VN_M_TDM 79			/* Modem tone dialing mode */
#define VN_M_ZZZ 79			/* Last MODEM one */

#define VN_SELCT 80			/* Selected Text from Mark Mode */
#define VN_TEMP  81			/* Temporary directory */
#define VN_ISTAT 82			/* INPUT command status */
#define VN_INI   83			/* INI (kermrc) directory */
#define VN_EXEDIR 84			/* EXE directory */
#define VN_ERRNO  85			/* Value of errno */
#define VN_ERSTR  86			/* Corresponding error message */
#define VN_TFLN   87			/* TAKE file line number */
#define VN_XVNUM  88			/* Product-specific version number */
#define VN_RPSIZ  89			/* Receive packet length */
#define VN_WINDO  90			/* Window size */
#define VN_MDMSG  91			/* Modem message */
#define VN_DNUM   92			/* Dial number */
#define VN_APC    93			/* APC active */
#define VN_IPADDR 94			/* My IP address */
#define VN_CRC16  95			/* CRC-16 of most recent file group */
#define VN_TRMK   96                    /* Macro executed from Terminal Mode */
#define VN_PID    97			/* Process ID */
#define VN_FNAM   98			/* Name of file being transferred */
#define VN_FNUM   99			/* Number of file being transferred */
#define VN_PEXIT  100			/* Process exit status */
#define VN_P_CTL  101 			/* Control Prefix */
#define VN_P_8BIT 102			/* 8-bit prefix */
#define VN_P_RPT  103			/* Repeat count prefix */
#define VN_D_LCP  104			/* DIAL LOCAL-PREFIX */
#define VN_URL    105			/* Last URL selected */
#define VN_REGN   106			/* Registration Name */
#define VN_REGO   107			/* Registration Organization */
#define VN_REGS   108			/* Registration Serial number */
#define VN_XPROG  109			/* xprogram (like xversion) */
#define VN_EDITOR 110			/* Editor */
#define VN_EDOPT  111			/* Editor options */
#define VN_EDFILE 112			/* Editor file */
#define VN_BROWSR 113			/* Browser */
#define VN_BROPT  114			/* Browser options */
#define VN_HERALD 115			/* Program herald */
#define VN_TEST   116			/* Program test level or "0" */
#define VN_XFSTAT 117			/* File-Transfer status */
#define VN_XFMSG  119			/* File-Transfer message */
#define VN_SNDL   120			/* Send-list status */
#define VN_TRIG   121			/* Trigger value */
#define VN_MOU_X  122                   /* OS/2 Mouse Cursor X */
#define VN_MOU_Y  123                   /* OS/2 Mouse Cursor Y */
#define VN_PRINT  124			/* Printer */
#define VN_ESC    125			/* Escape character */
#define VN_INTIME 126			/* INPUT elapsed time */
#define VN_K4RLM  127			/* Kerberos 4 Realm */
#define VN_K5RLM  128			/* Kerberos 5 Realm */
#define VN_K4PRN  129			/* Kerberos 4 Principal */
#define VN_K5PRN  130			/* Kerberos 5 Principal */
#define VN_K4CC   131			/* Kerberos 4 Credentials Cache */
#define VN_K5CC   132			/* Kerberos 5 Credentials Cache */
#define VN_OSNAM  133			/* OS name */
#define VN_OSVER  134			/* OS version */
#define VN_OSREL  135			/* OS release */
#define VN_NAME   136			/* Name I was called by */
#define VN_MODL   137			/* CPU model */
#define VN_X25LA  138			/* X.25 local address */
#define VN_X25RA  139			/* X.25 remote address */
#define VN_K4SRV  140                   /* Kerberos 4 Service Name */
#define VN_K5SRV  141                   /* Kerberos 5 Service Name */
#define VN_PDSFX  142			/* PDIAL suffix */
#define VN_DTYPE  143			/* DIAL type */
#define VN_LCKPID 144			/* Lockfile PID (UNIX) */
#define VN_BLK    145			/* Block check */
#define VN_TFTIM  146			/* File transfer elapsed time */
#define VN_D_PXX  147 			/* DIAL PBX-EXCHANGE */
#define VN_HWPAR  148 			/* Hardware Parity */
#define VN_SERIAL 149 			/* SET SERIAL value */
#define VN_LCKDIR 150			/* Lockfile directory (UNIX) */

#define VN_K4ENO  151                   /* Kerberos 4 Last Errno */
#define VN_K4EMSG 152                   /* Kerberos 4 Last Err Msg */
#define VN_K5ENO  153                   /* Kerberos 5 Last Errno */
#define VN_K5EMSG 154                   /* Kerberos 5 Last Err Msg */

#define VN_INTMO  155			/* Input timeout */
#define VN_AUTHS  156                   /* Authentication State */

#define VN_DM_LP  157			/* Dial Modifier: Long Pause */
#define VN_DM_SP  158			/* Dial Modifier: Short Pause */
#define VN_DM_PD  159			/* Dial Modifier: Pulse Dial */
#define VN_DM_TD  160			/* Dial Modifier: Tone Dial */
#define VN_DM_WA  161			/* Dial Modifier: Wait for Answer */
#define VN_DM_WD  162			/* Dial Modifier: Wait for Dialtone */
#define VN_DM_RC  163			/* Dial Modifier: Return to Command */

/* (more below...) */

#define VN_TY_LN  164			/* TYPE command line number */
#define VN_TY_LC  165			/* TYPE command line count */
#define VN_TY_LM  166			/* TYPE command match count */

#define VN_MACLVL 167			/* \v(maclevel) */

#define VN_XF_BC  168			/* Transfer blockcheck errors */
#define VN_XF_TM  169			/* Transfer timeouts */
#define VN_XF_RX  170			/* Transfer retransmissions */

#define VN_M_NAM  171			/* Modem full name  */
#define VN_MS_CD  172			/* Modem signal CD  */
#define VN_MS_CTS 173			/* Modem signal CTS */
#define VN_MS_DSR 174			/* Modem signal DSR */
#define VN_MS_DTR 175			/* Modem signal DTR */
#define VN_MS_RI  176			/* Modem signal RI  */
#define VN_MS_RTS 177			/* Modem signal RTS */

#define VN_MATCH  178			/* Most recent pattern match */
#define VN_SLMSG  179			/* SET LINE (error) message */
#define VN_TXTDIR 180			/* Kermit text-file directory */
#define VN_MA_PI  181			/* Math constant Pi */
#define VN_MA_E   182			/* Math constant e */
#define VN_MA_PR  183			/* Math precision (digits) */
#define VN_CMDBL  184			/* Command buffer length */

#define VN_AUTHT  185                   /* Authentication Type */

#ifdef CKCHANNELIO
#define VN_FERR   186			/* FILE error */
#define VN_FMAX   187			/* FILE max */
#define VN_FCOU   188			/* Result of last FILE COUNT */
#endif /* CKCHANNELIO */

#define VN_DRTR   189			/* DIAL retry counter */
#define VN_CXTIME 190			/* Elapsed time in session */
#define VN_BYTE   191			/* Byte order */
#define VN_AUTHN  192                   /* Authentication Name */
#define VN_KBCHAR 193			/* kbchar */
#define VN_TTYNAM 194			/* Name of controlling terminal */

#define VN_X509_S 195                   /* X.509 Certificate Subject */
#define VN_X509_I 196                   /* X.509 Certificate Issuer  */

#define VN_PROMPT 197			/* C-Kermit's prompt */
#define VN_BUILD  198			/* Build ID string */

#define VN_SEXP   199			/* Last S-Expression */
#define VN_VSEXP  200			/* Value of last S-Expression */
#define VN_LSEXP  201			/* SEXP depth */

#define VN_FTIME  202			/* Time as floating-poing number */

#define VN_FTP_C  203                   /* FTP Reply Code */
#define VN_FTP_M  204                   /* FTP Reply Message */
#define VN_FTP_S  205			/* FTP Server type */
#define VN_FTP_H  206			/* FTP Host */
#define VN_FTP_X  207			/* FTP Connected */
#define VN_FTP_L  208			/* FTP Logged in */
#define VN_FTP_G  209			/* FTP GET-PUT-REMOTE setting */

#define VN_SECURE 210                   /* Encrypted connection 0 or 1 */

#define VN_DM_HF  211			/* Dial Modifier: Hook Flash */
#define VN_DM_WB  212			/* Dial Modifier: Wait for Bong */
#define VN_CX_STA 213			/* CX_STATUS */

#define VN_FTP_B  214                   /* FTP CPL */
#define VN_FTP_D  215                   /* FTP DPL */
#define VN_FTP_Z  216                   /* FTP SECURITY */
#define VN_HTTP_C 217                   /* HTTP Code */
#define VN_HTTP_N 218                   /* HTTP Connected */
#define VN_HTTP_H 219                   /* HTTP Host */
#define VN_HTTP_M 220                   /* HTTP Message */
#define VN_HTTP_S 221                   /* HTTP Security */

#define VN_NOW    222			/* Timestamp yyyymmdd hh:mm:ss */
#define VN_HOUR   223			/* Current hour of the day 0-23 */

#define VN_CI_DA  224			/* Caller ID date */
#define VN_CI_TI  225			/* Caller ID time */
#define VN_CI_NA  226			/* Caller ID name */
#define VN_CI_NU  227			/* Caller ID number */
#define VN_CI_ME  228			/* Caller ID message */
#define VN_PERSONAL 229                 /* Personal Directory on Windows */
#define VN_APPDATA  230                 /* User AppData directory */
#define VN_COMMON   231                 /* Common AppData directory */
#define VN_DESKTOP  232                 /* User Desktop directory */
#define VN_TNC_SIG  233                 /* RFC 2717 Signature */

#ifdef KUI
#define VN_GUI_XP   234                 /* GUI Window X position */
#define VN_GUI_YP   235                 /* GUI Window Y position */
#define VN_GUI_XR   236                 /* GUI Window X resolution */
#define VN_GUI_YR   237                 /* GUI Window Y resolution */
#define VN_GUI_RUN  238                 /* GUI Window Run mode */
#define VN_GUI_FNM  239                 /* GUI Window Font Name */
#define VN_GUI_FSZ  240                 /* GUI Window Font Size */
#endif /* KUI */

#define VN_LOG_PKT  241                 /* Packet Log Filename */
#define VN_LOG_TRA  242                 /* Transaction Log Filename */
#define VN_LOG_SES  243                 /* Session Log Filename */
#define VN_LOG_DEB  244                 /* Debug Log Filename */
#define VN_LOG_CON  245                 /* Connection Log Filename */

#define VN_ISCALE   246			/* INPUT scale factor */
#define VN_BITS     247			/* Bits of this build (16, 32, 64) */
#define VN_LASTFIL  248			/* Last input filespec */
#define VN_LASTKWV  249			/* Last \fkeywordvalue() keyword */
#define VN_DMSG     250			/* Msg corresponding to dialstatus */
#define VN_HOSTIP   251			/* IP address of remote host */
#define VN_INPMSG   252			/* Msg corresponding to instatus */
#define VN_VAREVAL  253			/* SET VARIABLE-EVALUATION setting */
#define VN_PREVCMD  254			/* Previous command */
#endif /* NOSPL */

/* INPUT status values */

#define INP_OK  0			/* Succeeded */
#define INP_TO  1			/* Timed out */
#define INP_UI  2			/* User interrupted */
#define INP_IE  3			/* Internal error */
#define INP_IO  4			/* I/O error or connection lost */
#define INP_IKS 5                       /* Kermit Server Active */
#define INP_BF  6			/* Buffer full */

/* INPUT switch values */

#define INPSW_NOM 1			/* /NOMATCH */
#define INPSW_CLR 2			/* /CLEAR */
#define INPSW_NOW 4			/* /NOWRAP */
#define INPSW_COU 8			/* /COUNT */

#ifndef NOSPL
/* Symbols for builtin functions */

#define FNARGS 6			/* Maximum number of function args */

#define FN_IND      0			/* Index (of string 1 in string 2) */
#define FN_LEN      1			/* Length (of string) */
#define FN_LIT      2			/* Literal (don't expand the string) */
#define FN_LOW      3			/* Lower (convert to lowercase) */
#define FN_MAX      4			/* Max (maximum) */
#define FN_MIN      5			/* Min (minimum) */
#define FN_MOD      6			/* Mod (modulus) */
#define FN_EVA      7			/* Eval (evaluate arith expression) */
#define FN_SUB      8			/* Substr (substring) */
#define FN_UPP      9			/* Upper (convert to uppercase) */
#define FN_REV      10			/* Reverse (a string) */
#define FN_REP      11			/* Repeat (a string) */
#define FN_EXE      12			/* Execute (a macro) */
#define FN_VAL      13			/* Return value (of a macro) */
#define FN_LPA      14			/* LPAD (left pad) */
#define FN_RPA      15			/* RPAD (right pad) */
#define FN_DEF      16			/* Definition of a macro, unexpanded */
#define FN_CON      17			/* Contents of a variable, ditto */
#define FN_FIL      18			/* File list */
#define FN_FC       19			/* File count */
#define FN_CHR      20			/* Character (like BASIC CHR$()) */
#define FN_RIG      21			/* Right (like BASIC RIGHT$()) */
#define FN_COD      22			/* Code value of character */
#define FN_RPL      23			/* Replace */
#define FN_FD       24			/* File date */
#define FN_FS       25			/* File size */
#define FN_RIX      26			/* Rindex (index from right) */
#define FN_VER      27			/* Verify */
#define FN_IPA      28			/* Find and return IP address */
#define FN_CRY      39			/* ... */
#define FN_OOX      40			/* ... */
#define FN_HEX      41			/* Hexify */
#define FN_UNH      42			/* Unhexify */
#define FN_BRK      43			/* Break */
#define FN_SPN      44			/* Span */
#define FN_TRM      45			/* Trim */
#define FN_LTR      46			/* Left-Trim */
#define FN_CAP      47			/* Capitalize */
#define FN_TOD      48			/* Time-of-day-to-secs-since-midnite */
#define FN_SEC      49			/* Secs-since-midnite-to-time-of-day */
#define FN_FFN      50			/* Full file name */
#define FN_CHK      51			/* Checksum of text */
#define FN_CRC      52			/* CRC-16 of text */
#define FN_BSN      53			/* Basename of file */
#define FN_CMD      54			/* Output of a command (cooked) */
#define FN_RAW      55			/* Output of a command (raw) */
#define FN_STX      56			/* Strip from right */
#define FN_STL      57			/* Strip from left */
#define FN_STN      58			/* Strip n chars */
#define FN_SCRN_CX  59			/* Screen Cursor X Pos */
#define FN_SCRN_CY  60			/* Screen Cursor Y Pos */
#define FN_SCRN_STR 61			/* Screen String */
#define FN_2HEX     62			/* Number (not string) to hex */
#define FN_2OCT     63			/* Number (not string) to octal */
#define FN_RFIL     64			/* Recursive file list */
#define FN_DIR      65			/* Directory list */
#define FN_RDIR     66			/* Recursive directory list */
#define FN_DNAM     67			/* Directory part of filename */
#define FN_RAND     68			/* Random number */
#define FN_WORD     69			/* Word extraction */
#define FN_SPLIT    70			/* Split string into words */
#define FN_KRB_TK   71			/* Kerberos tickets */
#define FN_KRB_NX   72			/* Kerberos next ticket */
#define FN_KRB_IV   73			/* Kerberos ticket is valid */
#define FN_KRB_TT   74			/* Kerberos ticket time */
#define FN_ERRMSG   75			/* Error code to message */

#ifndef UNIX
#ifndef VMS
#undef FN_ERRMSG
#endif /* VMS */
#endif /* UNIX */

#define FN_DIM      76			/* Dimension of array */
#define FN_DTIM     77			/* Convert to standard date/time */
#define FN_JDATE    78			/* Regular date to day of year */
#define FN_PNCVT    79			/* Convert phone number for dialing */
#define FN_DATEJ    80			/* Day of year to date */
#define FN_MJD      81			/* Date to modified Julian date */
#define FN_MJD2     82			/* Modified Julian date to date */
#define FN_DAY      83			/* Day of week of given date */
#define FN_NDAY     84			/* Numeric day of week of given date */
#define FN_TIME     85			/* Convert to hh:mm:ss */
#define FN_NTIM     86			/* Convert to seconds since midnite */
#define FN_N2TIM    87			/* Sec since midnite to hh:mm:ss */
#define FN_PERM     88			/* Permissions of file */
#define FN_KRB_FG   89			/* Kerberos Ticket Flags */
#define FN_SEARCH   90			/* Search for pattern in string */
#define FN_RSEARCH  91			/* Ditto, but right to left */
#define FN_XLATE    92			/* Translate string charset */
#define FN_ALOOK    93			/* Array lookup */
#define FN_TLOOK    94			/* Table lookup */
#define FN_TOB64    95			/* Encode into Base64 */
#define FN_FMB64    96			/* Decode from Base64 */

#define FN_ABS      97			/* Absolute value */

#ifdef CKFLOAT
#define FN_FPADD    98			/* Floating-point add */
#define FN_FPSUB    99			/* Floating-point substract */
#define FN_FPMUL   100			/* Floating-point multiply */
#define FN_FPDIV   101			/* Floating-point divide */
#define FN_FPEXP   102			/* Floating-point e to the x */
#define FN_FPLN    103			/* Floating-point natural log */
#define FN_FPLOG   104			/* Floating-point base-10 log */
#define FN_FPPOW   105			/* Floating-point raise to power */
#define FN_FPSQR   106			/* Floating-point square root */
#define FN_FPABS   107			/* Floating-point absolute value */
#define FN_FPMOD   108			/* Floating-point modulus */
#define FN_FPMAX   109			/* Floating-point maximum */
#define FN_FPMIN   110			/* Floating-point minimum*/
#define FN_FPINT   111			/* Floating-point to integer */
#define FN_FPROU   112			/* Floating-point round */
#define FN_FPSIN   113			/* FP sine */
#define FN_FPCOS   114			/* FP cosine */
#define FN_FPTAN   115			/* FP tangent */
#endif /* CKFLOAT */

#ifdef CKCHANNELIO
#define FN_FSTAT   116			/* File status */
#define FN_FPOS    117			/* File position */
#define FN_FEOF    118			/* File eof */
#define FN_FILNO   119			/* File number / handle */
#define FN_FGCHAR  120			/* File getchar */
#define FN_FGLINE  121			/* File getline */
#define FN_FGBLK   122			/* File getblock */
#define FN_FPCHAR  123			/* File putchar */
#define FN_FPLINE  124			/* File putline */
#define FN_FPBLK   125			/* File putblock */
#define FN_NLINE   126			/* File get current line number */
#define FN_FERMSG  127			/* File error message */
#endif /* CKCHANNELIO */

#define FN_LEF     128			/* Left (= substr starting on left) */
#define FN_AADUMP  129			/* Associative Array Dump */
#define FN_STB     130			/* \fstripb()  */
#define FN_PATTERN 131			/* \fpattern() */
#define FN_HEX2N   132			/* \fhexton()  */
#define FN_OCT2N   133			/* \foctton()  */
#define FN_HEX2IP  134			/* \fhextoip() */
#define FN_IP2HEX  135			/* \fiptohex() */
#define FN_RADIX   136			/* \fradix()   */
#define FN_JOIN    137			/* \fjoin()    */
#define FN_SUBST   138			/* \fsubstitute() */
#define FN_SEXP    139			/* \fsexpression() */
#define FN_CMDSTK  140			/* \fcmdstack() */
#define FN_TOGMT   141			/* \ftogmt() */
#define FN_CMPDATE 142			/* \fcmpdates() */
#define FN_DIFDATE 143			/* \fdiffdates() */
#ifdef TCPSOCKET
#define FN_HSTADD  144			/* \faddr2name() */
#define FN_HSTNAM  145			/* \fname2addr() */
#endif /* TCPSOCKET */
#define FN_DELSEC  146			/* \fdelta2sec() */
#define FN_PC_DU   147			/* Path conversion DOS to Unix */
#define FN_PC_VU   148			/* Path conversion VMS to Unix */
#define FN_PC_UD   149			/* Path conversion Unix to DOS */
#define FN_PC_UV   150			/* Path conversion Unix to VMS */
#define FN_KWVAL   151			/* \fkeywordvalue() */
#define FN_SLEEP   152			/* \fsleep() */
#define FN_MSLEEP  153			/* \fmsleep() */
#define FN_LNAME   154			/* \fLongPathName() (Windows) */
#define FN_SNAME   155			/* \fShortPathName() (Windows) */
#define FN_UNTAB   156			/* \funtabify() */
#define FN_LOPX    157			/* \flopx() */
#define FN_EMAIL   158			/* \femailaddress() */
#define FN_PICTURE 159			/* \fpictureinfo() */
#define FN_PID     160			/* \fpidinfo() */
#define FN_COUNT   161			/* \fcount() */
#define FN_FUNC    162			/* \ffunction() */
#define FN_RECURSE 163			/* \frecurse() */
#define FN_SQUEEZE 164			/* \fsqueeze() */
#define FN_UNPCT   165			/* \fdecodehex() */
#define FN_STRINGT 166			/* \fstringtype() */
#define FN_STRCMP  167			/* \fstrcmp() */

#endif /* NOSPL */

/* Time Units */

#define TU_DAYS   0
#define TU_WEEKS  1
#define TU_MONTHS 2
#define TU_YEARS  3

#ifdef CK_CURSES
/* Screen line numbers for fullscreen file-transfer display */

#define CW_BAN  0			/* Curses Window Banner */
#define CW_DIR  2			/* Current directory */
#define CW_LIN  3			/* Communication device */
#define CW_SPD  4			/* Communication speed */
#define CW_PAR  5			/* Parity */
#define CW_TMO  6
#define CW_NAM  7			/* Filename */
#define CW_TYP  8			/* File type */
#define CW_SIZ  9			/* File size */
#define CW_PCD 10			/* Percent done */

#ifndef CK_PCT_BAR
#define CW_TR  11			/* Time remaining */
#define CW_CP  12			/* Characters per second */
#define CW_WS  13			/* Window slots */
#define CW_PT  14			/* Packet type */
#define CW_PC  15			/* Packet count */
#define CW_PL  16			/* Packet length */
#define CW_PR  17			/* Packet retry */
#ifdef COMMENT
#define CW_PB  17			/* Packet block check */
#endif /* COMMENT */
#else /* CK_PCT_BAR */
#define CW_BAR 11			/* Percent Bar Scale */
#define CW_TR  12			/* Time remaining */
#define CW_CP  13			/* Chars per sec */
#define CW_WS  14			/* Window slots */
#define CW_PT  15			/* Packet type */
#define CW_PC  16			/* Packet count */
#define CW_PL  17			/* Packet length */
#define CW_PR  18			/* Packet retry */
#ifdef COMMENT
#define CW_PB  18			/* Packet block check */
#endif /* COMMENT */
#endif /* CK_PCT_BAR */

#define CW_ERR 19			/* Error message */
#define CW_MSG 20			/* Info message */
#define CW_INT 22			/* Instructions */
#define CW_FFC 99                       /* File Characters Sent/Received */
#endif /* CK_CURSES */

#ifndef NOICP
/* Save Commands */
#define XSKEY   0			/* Key map file */
#define XSCMD   1                       /* Command mode */
#define XSTERM  2                       /* Terminal mode */
#endif /* NOICP */

#ifndef NODIAL
/* Dial routine sort priorities */
#define DN_INTERN 0
#define DN_FREE   1
#define DN_LOCAL  2
#define DN_UNK    3
#define DN_LONG   4
#define DN_INTL   5
#endif /* NODIAL */

#ifdef SSHBUILTIN
#define XSSH_OPN 1
#define XSSH_V2  2
#define XSSH_FLP 3
#define XSSH_FRP 4
#define XSSH_ADD 5
#define XSSH_KEY 6
#define XSSH_CLR 7
#define XSSH_AGT 8

#define SSHKT_1R   0			/* SSH KEY TYPE symbols */
#define SSHKT_2R   1                    /* must match ssh/key.h values */
#define SSHKT_2D   2
#define SSHKT_SRP  3

#define SSHKD_IN   1			/* SSH KEY DISPLAY /IN-FORMAT */
#define SSHKD_OUT  2			/* SSH KEY DISPLAY /OUT-FORMAT */

#define SKDF_OSSH  1			/* Key display format OpenSSH */
#define SKDF_SSHC  2			/* Key display format SSH.COM */
#define SKDF_IETF  3			/* Key display format IETF */
#define SKDF_FING  4			/* Key display format FINGERPRINT */

#define SSHSW_USR 1
#define SSHSW_VER 2
#define SSHSW_CMD 3
#define SSHSW_X11 4
#define SSHSW_PWD 5
#define SSHSW_SUB 6

#define SSHC_LPF 1
#define SSHC_RPF 2

#define XSSH2_RKE 1

#define SSHF_LCL   1
#define SSHF_RMT   2

#define SSHA_ADD   1
#define SSHA_DEL   2
#define SSHA_LST   3

#define SSHASW_FP 1

#define SSHK_PASS  1
#define SSHK_CREA  2
#define SSHK_DISP  3
#define SSHK_V1    4

#define SSHKC_BI  1
#define SSHKC_PP  2
#define SSHKC_TY  3
#define SSHKC_1R  4

#define SKRM_OPN  1
#endif /* SSHBUILTIN */

#ifdef SFTP_BUILTIN
#define SFTP_OPN    1
#define SFTP_CD     2
#define SFTP_CHGRP  3
#define SFTP_CHMOD  4
#define SFTP_CHOWN  5
#define SFTP_DIR    6
#define SFTP_GET    7
#define SFTP_MKDIR  8
#define SFTP_PWD    9
#define SFTP_PUT    10
#define SFTP_REN    11
#define SFTP_RM     12
#define SFTP_RMDIR  13
#define SFTP_LINK   14
#define SFTP_VER    15

#define XY_SFTP_RCS 1
#define XY_SFTP_EOL 2
#endif /* SFTP_BUILTIN */

/* ANSI-C prototypes for user interface functions */

#ifdef UNIX
_PROTOTYP( int doputenv, ( char *, char * ) );
#endif	/* UNIX */

_PROTOTYP( int chkaes, ( char, int ) );

#ifndef NOICP
_PROTOTYP( int matchname, ( char *, int, int ) );
_PROTOTYP( int ck_cls, ( void ) );
_PROTOTYP( int ck_cleol, ( void ) );
_PROTOTYP( int ck_curpos, ( int, int ) );
_PROTOTYP( int cmdsrc, ( void ) );
_PROTOTYP( int parser, ( int ) );
_PROTOTYP( int chkvar, (char *) );
_PROTOTYP( int zzstring, (char *, char **, int *) );
#ifndef NOFRILLS
_PROTOTYP( int yystring, (char *, char **) );
#endif /* NOFRILLS */
_PROTOTYP( int getncm, (char *, int) );
_PROTOTYP( int getnct, (char *, int, FILE *, int) );
#endif /* NOICP */
_PROTOTYP( VOID bgchk, (void) );
_PROTOTYP( char * nvlook, (char *) );
_PROTOTYP( int xarray, (char *) );
_PROTOTYP( int arraynam, (char *, int *, int *) );
_PROTOTYP( int arraybounds, (char *, int *, int *) );
_PROTOTYP( int boundspair, (char *, char *, int *, int *, char *) );
_PROTOTYP( int arrayitoa, (int) );
_PROTOTYP( int arrayatoi, (int) );
_PROTOTYP( char * bldlen, (char *, char *) );
_PROTOTYP( int chkarray, (int, int) );
_PROTOTYP( int dclarray, (char, int) );
_PROTOTYP( int pusharray, (int, int) );
_PROTOTYP( int parsevar, (char *, int *, int *) );
_PROTOTYP( int macini, (void) );
_PROTOTYP( VOID initmac, (void) );
_PROTOTYP( int delmac, (char *, int) );
_PROTOTYP( int addmac, (char *, char *) );
_PROTOTYP( int domac, (char *, char *, int) );
_PROTOTYP( int addmmac, (char *, char *[]) );
_PROTOTYP( int dobug, (void) );
_PROTOTYP( int docd, (int) );
_PROTOTYP( int doclslog, (int) );
_PROTOTYP( int docmd, (int) );
_PROTOTYP( int dodir, (int) );
_PROTOTYP( int dodo, (int, char *, int) );
_PROTOTYP( int doenable, (int, int) );
_PROTOTYP( int dogoto, (char *, int) );
_PROTOTYP( int dogta, (int) );
_PROTOTYP( int dohlp, (int) );
_PROTOTYP (int doincr, (int) );
_PROTOTYP( int dohrmt, (int) );
_PROTOTYP( int doif, (int) );
_PROTOTYP( int doinput, (int, char *[], int[], int, int) );
_PROTOTYP( int doreinp, (int, char *, int) );
_PROTOTYP( int dolog, (int) );
_PROTOTYP( int dologin, (char *) );
_PROTOTYP( int doopen, (void) );
_PROTOTYP( int doprm, (int, int) );
_PROTOTYP( int doreturn, (char *) );
_PROTOTYP( int dormt, (int) );
_PROTOTYP( int dosort, (void) );
_PROTOTYP( int dostat, (int) );
_PROTOTYP( int dostop, (void) );
_PROTOTYP( int dotype, (char *, int, int, int, char *, int, char *, int, int,
			char *, int));
_PROTOTYP( int transmit, (char *, char, int, int, int) );
_PROTOTYP( int xlate, (char *, char *, int, int) );
_PROTOTYP( int litcmd, (char **, char **, int) );
_PROTOTYP( int incvar, (char *, CK_OFF_T, int) );
_PROTOTYP( int ckdial, (char *, int, int, int, int) );
_PROTOTYP( int hmsg, (char *) );
_PROTOTYP( int hmsga, (char * []) );
_PROTOTYP( int mlook, (struct mtab [], char *, int) );
_PROTOTYP( int mxlook, (struct mtab [], char *, int) );
_PROTOTYP( int mxxlook, (struct mtab [], char *, int) );
_PROTOTYP( int prtopt, (int *, char *) );
_PROTOTYP( CHAR rfilop, (char *, char) );
_PROTOTYP( int setcc, (char *, int *) );
_PROTOTYP( int setnum, (int *, int, int, int) );
_PROTOTYP( int seton, (int *) );
_PROTOTYP( int setonaut, (int *) );
_PROTOTYP( VOID shmdmlin, (void) );
_PROTOTYP( VOID initmdm, (int) );
_PROTOTYP( char * showoff, (int) );
_PROTOTYP( char * showooa, (int) );
_PROTOTYP( char * showstring, (char *) );
_PROTOTYP( int pktopn, (char *,int) );
_PROTOTYP( int traopn, (char *,int) );
_PROTOTYP( int sesopn, (char *,int) );
_PROTOTYP( int debopn, (char *,int) );
_PROTOTYP( int diaopn, (char *,int,int) );
_PROTOTYP( int prepop, (void) );
_PROTOTYP( int popclvl, (void) );
_PROTOTYP( int varval, (char *, CK_OFF_T *) );
_PROTOTYP( char * evala, (char *) );
_PROTOTYP( char * evalx, (char *) );
_PROTOTYP( int setalarm, (long) );
_PROTOTYP( int setat, (int) );
_PROTOTYP( int setinp, (void) );
_PROTOTYP( VOID dolognet, (void) );
_PROTOTYP( VOID dologline, (void) );
_PROTOTYP( int setlin, (int, int, int) );
_PROTOTYP( int setmodem, (void) );
_PROTOTYP( int setfil, (int) );
_PROTOTYP( char * homepath, (void) );
#ifdef OS2
_PROTOTYP( int settapi, (void) ) ;
#ifdef OS2MOUSE
_PROTOTYP( int setmou, (void) );
#endif /* OS2MOUSE */
#endif /* OS2 */
#ifdef LOCUS
_PROTOTYP( VOID setlocus, (int,int) );
_PROTOTYP( VOID setautolocus, (int) );
#endif /* LOCUS */
_PROTOTYP( int setbell, (void) );
_PROTOTYP( VOID setcmask, (int));
_PROTOTYP( VOID setautodl, (int,int));
_PROTOTYP( VOID setdebses, (int));
_PROTOTYP( VOID setseslog, (int));
_PROTOTYP( VOID setaprint, (int));
_PROTOTYP( int settrm, (void) );
_PROTOTYP( int settrmtyp, (void) );
_PROTOTYP( int setsr, (int, int) );
_PROTOTYP( int setxmit, (void) );
_PROTOTYP( int dosetkey, (void) );
_PROTOTYP( int dochk, (void) );
_PROTOTYP( int ludial, (char *, int) );
_PROTOTYP( char * getdnum, (int) );
_PROTOTYP( VOID getnetenv, (void) );
_PROTOTYP( int getyesno, (char *, int) );
_PROTOTYP( VOID xwords, (char *, int, char *[], int) );
#ifdef OS2
_PROTOTYP( VOID keynaminit, (void) );
#endif /* OS2 */
_PROTOTYP( int xlookup, (struct keytab[], char *, int, int *) );
_PROTOTYP( char * rlookup, (struct keytab[], int, int) );
_PROTOTYP( int hupok, (int) );
_PROTOTYP( char * zzndate, (void) );
_PROTOTYP( char * zjdate, (char *) );
_PROTOTYP( char * jzdate, (char *) );
_PROTOTYP( char * ckdate, (void) );
_PROTOTYP( char * chk_ac, (int, char[]) );
_PROTOTYP( char * gmdmtyp, (void) );
_PROTOTYP( char * gfmode, (int, int) );
_PROTOTYP( int setdest, (void) );
_PROTOTYP( VOID ndinit, (void) );
_PROTOTYP( int doswitch, (void) );
_PROTOTYP( int dolocal, (void) );
_PROTOTYP( long tod2sec, (char *) );
_PROTOTYP( int lunet, (char *) );
_PROTOTYP( int doxdis, (int) );
_PROTOTYP( int dosave, (int) );
_PROTOTYP( int doxsend, (int) );
_PROTOTYP( int doxget, (int) );
_PROTOTYP( int doxconn, (int) );
_PROTOTYP( int clsconnx, (int) );
_PROTOTYP( VOID ftreset, (void) );
#ifdef CK_KERBEROS
_PROTOTYP (int cp_auth, ( void ) );
#endif /* CK_KERBEROS */
_PROTOTYP( long mjd, (char *) );
_PROTOTYP( char * mjd2date, (long) );
_PROTOTYP( char * ckgetpid, (void) );

_PROTOTYP( int dogrep, (void) );

#ifndef NOFTP
#ifndef SYSFTP
_PROTOTYP( int doxftp, (void) );
_PROTOTYP( int doftphlp, (void) );
_PROTOTYP( int dosetftp, (void) );
_PROTOTYP( int dosetftphlp, (void) );
_PROTOTYP( int shoftp, (int) );
#endif /* SYSFTP */
#endif /* NOFTP */

_PROTOTYP( VOID cmhistory, (void) );
_PROTOTYP( char * getdcset, (void) );
_PROTOTYP( char * ttgtpn, (void) );

#ifndef NOSHOW
_PROTOTYP( int doshow, (int) );
_PROTOTYP( int shotcp, (int) );
_PROTOTYP( VOID shopar, (void) );
_PROTOTYP( VOID shofil, (void) );
_PROTOTYP( VOID shoparp, (void) );
_PROTOTYP( int shoatt, (void) );
_PROTOTYP( VOID shover, (void) );
_PROTOTYP( VOID shoctl, (void) );
_PROTOTYP( VOID shodbl, (void) );
#ifndef NOSPL
_PROTOTYP( int shomac, (char *, char *) );
_PROTOTYP( int doshift, (int) );
#endif /* NOSPL */
#ifndef NOCSETS
_PROTOTYP( VOID shocharset, (void) );
_PROTOTYP( VOID shoparl, (void) );
_PROTOTYP( VOID shotcs, (int, int) );
#endif /* NOCSETS */
#ifndef NOLOCAL
_PROTOTYP( VOID shoparc, (void) );
_PROTOTYP( int shomodem, (void) );
#ifndef NODIAL
_PROTOTYP( VOID shods, (char *) );
_PROTOTYP( VOID shodial, (void) );
_PROTOTYP( int doshodial, (void) );
#endif /* NODIAL */
#ifndef NONET
_PROTOTYP( int shonet, (void) );
_PROTOTYP( int shotopt, (int) );
_PROTOTYP( int shotel, (int) );
#ifdef CK_AUTHENTICATION
_PROTOTYP (int sho_auth,( int  ) );
#endif /* CK_AUTHENTICATION */
#endif /* NONET */
_PROTOTYP( VOID shomdm, (void) );
#endif /* NOLOCAL */
#ifdef OS2
_PROTOTYP( VOID shokeycode, (int,int) );
#else
_PROTOTYP( VOID shokeycode, (int) );
#endif /* OS2 */
_PROTOTYP( VOID showassoc, (void) );
_PROTOTYP( VOID showdiropts, (void) );
_PROTOTYP( VOID showdelopts, (void) );
_PROTOTYP( VOID showtypopts, (void) );
_PROTOTYP( VOID showpurgopts, (void) );
_PROTOTYP( VOID shoflow, (void) );
_PROTOTYP( VOID shoxfer, (void) );
#ifdef ANYSSH
_PROTOTYP( VOID shossh, (void) );
#endif	/* ANYSSH */
#endif /* NOSHOW */

_PROTOTYP( VOID shostrdef, (CHAR *) );

#ifndef NOSPL
_PROTOTYP( int addlocal, (char *) );
#endif /* NOSPL */

_PROTOTYP( int setdelopts, (void) );

#ifdef VMS
_PROTOTYP( int cvtdir, (char *, char *, int) );
#endif /* VMS */

#ifdef FNFLOAT
_PROTOTYP( VOID initfloat, (void) );
#endif /* FNFLOAT */

#ifdef CKCHANNELIO
_PROTOTYP( int dofile, (int) );
#endif /* CKCHANNELIO */

#ifdef CKROOT
_PROTOTYP( int dochroot, (void) );
#endif /* CKROOT */

#ifdef NEWFTP
_PROTOTYP( int doftpusr,    (void) );
_PROTOTYP( int doftpput,    (int,int) );
_PROTOTYP( int doftpget,    (int,int) );
_PROTOTYP( int doftprmt,    (int,int) );
_PROTOTYP( int ftpopen,     (char *, char *, int) );
_PROTOTYP( int cmdlinget,   (int) );
_PROTOTYP( int cmdlinput,   (int) );
_PROTOTYP( int doftparg,    (char) );
_PROTOTYP( int doftpacct,   (void) );
_PROTOTYP( int doftpsite,   (void) );
_PROTOTYP( int dosetftppsv, (void) );
_PROTOTYP( int ftpbye,      (void) );
#endif /* NEWFTP */

#ifdef COMMENT
/* These prototypes are no longer used */
_PROTOTYP( char * getdws, (int) );
_PROTOTYP( char * getdcs, (int) );
_PROTOTYP( int doget, (int) );
_PROTOTYP( char * arrayval, (int, int) );
#endif /* COMMENT */

#ifdef KUI
_PROTOTYP(int BuildFontTable,
          (struct keytab ** pTable, struct keytab ** pTable2, int * pN));
#endif /* KUI */

_PROTOTYP(int cx_net, (int net, int protocol, char * xhost, char * svc, 
        char * username, char * password, char * command,
        int param1, int param2, int param3, 
        int cx, int sx, int flag, int gui));
_PROTOTYP(int cx_serial, (char *device, 
        int cx, int sx, int shr, int flag, int gui, int special));

#endif /* CKUUSR_H */

/* End of ckuusr.h */
