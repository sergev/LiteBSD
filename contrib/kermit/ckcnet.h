/* ckcnet.h -- Symbol and macro definitions for C-Kermit network support */

/*
  Author: Frank da Cruz <fdc@columbia.edu>
  Columbia University Academic Information Systems, New York City.

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/
#ifndef CKCNET_H
#define CKCNET_H

/* Network types */

#define NET_NONE 0                      /* None */
#define NET_TCPB 1                      /* TCP/IP Berkeley (socket) */
#define NET_TCPA 2                      /* TCP/IP AT&T (streams) */
#define NET_SX25 3                      /* SUNOS SunLink X.25 */
#define NET_DEC  4                      /* DECnet */
#define NET_VPSI 5                      /* VAX PSI */
#define NET_PIPE 6                      /* LAN Manager Named Pipe */
#define NET_VX25 7                      /* Stratus VOS X.25 */
#define NET_BIOS 8                      /* IBM NetBios */
#define NET_SLAT 9                      /* Meridian Technologies' SuperLAT */
#define NET_FILE 10                     /* Read from a file */
#define NET_CMD  11                     /* Read from a sub-process */
#define NET_DLL  12                     /* Load a DLL for use as comm channel*/
#define NET_IX25 13                     /* IBM AIX 4.1 X.25 */
#define NET_HX25 14                     /* HP-UX 10 X.25 */
#define NET_PTY  15                     /* Pseudoterminal */
#define NET_SSH  16                     /* SSH */

#ifdef OS2                              /* In OS/2, only the 32-bit */
#ifndef __32BIT__                       /* version gets NETBIOS */
#ifdef CK_NETBIOS
#undef CK_NETBIOS
#endif /* CK_NETBIOS */
#endif /* __32BIT__ */
#endif /* OS2 */

#ifdef _M_PPC
#ifdef SUPERLAT
#undef SUPERLAT
#endif /* SUPERLAT */
#endif /* _M_PPC */

#ifdef NPIPE                            /* For items in common to */
#define NPIPEORBIOS                     /* Named Pipes and NETBIOS */
#endif /* NPIPE */
#ifdef CK_NETBIOS
#ifndef NPIPEORBIOS
#define NPIPEORBIOS
#endif /* NPIPEORBIOS */
#endif /* CK_NETBIOS */

/* Network virtual terminal protocols (for SET HOST connections) */
/* FTP, HTTP and SSH have their own stacks                       */

#define NP_DEFAULT    255
#define NP_NONE         0               /* None (async) */
#define NP_TELNET       1               /* TCP/IP telnet */
#define NP_VTP          2               /* ISO Virtual Terminal Protocol */
#define NP_X3           3               /* CCITT X.3 */
#define NP_X28          4               /* CCITT X.28 */
#define NP_X29          5               /* CCITT X.29 */
#define NP_RLOGIN       6               /* TCP/IP Remote login */
#define NP_KERMIT       7               /* TCP/IP Kermit */
#define NP_TCPRAW       8               /* TCP/IP Raw socket */
#define NP_TCPUNK       9               /* TCP/IP Unknown */
#define NP_SSL         10               /* TCP/IP SSLv23 */
#define NP_TLS         11               /* TCP/IP TLSv1 */
#define NP_SSL_TELNET  12               /* TCP/IP Telnet over SSLv23 */
#define NP_TLS_TELNET  13               /* TCP/IP Telnet over TLSv1 */
#define NP_K4LOGIN     14               /* TCP/IP Kerberized remote login */
#define NP_EK4LOGIN    15               /* TCP/IP Encrypted Kerberized ... */
#define NP_K5LOGIN     16               /* TCP/IP Kerberized remote login */
#define NP_EK5LOGIN    17               /* TCP/IP Encrypted Kerberized ... */
#define NP_K5U2U       18               /* TCP/IP Kerberos 5 User to User */
#define NP_CTERM       19               /* DEC CTERM */
#define NP_LAT         20               /* DEC LAT */
#define NP_SSL_RAW     21		/* SSL with no Telnet permitted */
#define NP_TLS_RAW     22		/* TLS with no Telnet permitted */

/* others here... */

#ifdef CK_SSL
#define IS_TELNET()      (nettype == NET_TCPB && (ttnproto == NP_TELNET \
                         || ttnproto == NP_SSL_TELNET \
                         || ttnproto == NP_TLS_TELNET \
                         || ttnproto == NP_KERMIT))
#else /* CK_SSL */
#define IS_TELNET()      (nettype == NET_TCPB && (ttnproto == NP_TELNET \
                         || ttnproto == NP_KERMIT))
#endif /* CK_SSL */

#ifdef CK_KERBEROS
#ifdef KRB5
#ifdef KRB4
#define IS_RLOGIN()      (nettype == NET_TCPB && (ttnproto == NP_RLOGIN \
                         || ttnproto == NP_K5LOGIN \
                         || ttnproto == NP_EK5LOGIN \
                         || ttnproto == NP_K4LOGIN \
                         || ttnproto == NP_EK4LOGIN \
                         ))
#else /* KRB4 */
#define IS_RLOGIN()      (nettype == NET_TCPB && (ttnproto == NP_RLOGIN \
                         || ttnproto == NP_K5LOGIN \
                         || ttnproto == NP_EK5LOGIN \
                         ))
#endif /* KRB4 */
#else /* KRB5 */
#ifdef KRB4
#define IS_RLOGIN()      (nettype == NET_TCPB && (ttnproto == NP_RLOGIN \
                         || ttnproto == NP_K4LOGIN \
                         || ttnproto == NP_EK4LOGIN \
                         ))
#else /* KRB4 */
KERBEROS defined without either KRB4 or KRB5
#endif /* KRB4 */
#endif /* KRB5 */
#else /* CK_KERBEROS */
#define IS_RLOGIN()      (nettype == NET_TCPB && (ttnproto == NP_RLOGIN))
#endif /* CK_KERBEROS */

#define IS_SSH()         (nettype == NET_SSH)

/* RLOGIN Modes */
#define    RL_RAW     0                 /*  Do Not Process XON/XOFF */
#define    RL_COOKED  1                 /*  Do Process XON/XOFF */

/* Encryption types */

#define CX_NONE    999

#ifdef ENCTYPE_ANY
#define CX_AUTO ENCTYPE_ANY
#else
#define CX_AUTO 0
#endif /* ENCTYPE_ANY */

#ifdef ENCTYPE_DES_CFB64
#define CX_DESC64 ENCTYPE_DES_CFB64
#else
#define CX_DESC64 1
#endif /* ENCTYPE_DES_CFB64 */

#ifdef ENCTYPE_DES_OFB64
#define CX_DESO64 ENCTYPE_DES_OFB64
#else
#define CX_DESO64 2
#endif /* ENCTYPE_DES_OFB64 */

#ifdef ENCTYPE_DES3_CFB64
#define CX_DES3C64 ENCTYPE_DES3_CFB64
#else
#define CX_DES3C64 3
#endif /* ENCTYPE_DES_CFB64 */

#ifdef ENCTYPE_DES3_OFB64
#define CX_DESO64 ENCTYPE_DES3_OFB64
#else
#define CX_DES3O64 4
#endif /* ENCTYPE_DES_OFB64 */

#ifdef ENCTYPE_CAST5_40_CFB64
#define CX_C540C64 ENCTYPE_CAST5_40_CFB64
#else
#define CX_C540C64 8
#endif /* ENCTYPE_CAST5_40_CFB64 */

#ifdef ENCTYPE_CAST5_40_OFB64
#define CX_C540O64 ENCTYPE_CAST5_40_OFB64
#else
#define CX_C540O64 9
#endif /* ENCTYPE_CAST5_40_OFB64 */

#ifdef ENCTYPE_CAST128_CFB64
#define CX_C128C64 ENCTYPE_CAST128_CFB64
#else
#define CX_C128C64 10
#endif /* ENCTYPE_CAST128_CFB64 */

#ifdef ENCTYPE_CAST128_OFB64
#define CX_C128O64 ENCTYPE_CAST128_OFB64
#else
#define CX_C128O64 11
#endif /* ENCTYPE_CAST128_OFB64 */

/* Basic network function prototypes, common to all. */

_PROTOTYP( int netopen, (char *, int *, int) );
_PROTOTYP( int netclos, (void) );
_PROTOTYP( int netflui, (void) );
_PROTOTYP( int nettchk, (void) );
_PROTOTYP( int netxchk, (int) );
_PROTOTYP( int netbreak, (void) );
_PROTOTYP( int netinc, (int) );
_PROTOTYP( int netxin, (int, CHAR *) );
_PROTOTYP( int nettol, (CHAR *, int) );
_PROTOTYP( int nettoc, (CHAR) );
#ifdef TCPSOCKET
_PROTOTYP( int gettcpport, (void) );
_PROTOTYP( int gettcpport, (void) );
#endif	/* TCPSOCKET */

/*
  SunLink X.25 support by Marcello Frutig, Catholic University,
  Rio de Janeiro, Brazil, 1990.

  Maybe this can be adapted to VAX PSI and other X.25 products too.
*/
#ifndef SUNOS4                          /* Only valid for SUNOS4 */
#ifndef SOLARIS
#ifdef SUNX25
#undef SUNX25
#endif /* SUNX25 */
#endif /* SOLARIS */
#endif /* SUNOS4 */

#ifdef STRATUSX25
#define ANYX25
#define MAX_USER_DATA 128 /* SUN defines this in a header file, I believe. */
#endif /* STRATUSX25 */

#ifdef SUNX25
#define ANYX25
#endif /* SUNX25 */

#ifdef IBMX25                           /* AIX 4.1 X.25 */
#ifndef AIX41
#undef IBMX25
#else /* AIX41 */
#define ANYX25
#define MAX_USER_DATA NPI_MAX_DATA      /* used for buffer sizes */
#endif /* AIX41 */
#endif /* IBMX25 */

#ifdef HPX25                            /* HP-UX 10.* X.25 */
#ifndef HPUX10
#undef HPX25
#else /* HPUX10 */
#define ANYX25
#endif /* HPUX10 */
#endif /* HPX25 */

#ifdef ANYX25
#ifndef NETCONN                         /* ANYX25 implies NETCONN */
#define NETCONN
#endif /* NETCONN */

#define MAXPADPARMS                22   /* Number of PAD parameters */
#define MAXCUDATA                  12   /* Max length of X.25 call user data */
#define X29PID                      1   /* X.29 protocol ID */
#define X29PIDLEN                   4   /* X.29 protocol ID length */

#define X29_SET_PARMS               2
#define X29_READ_PARMS              4
#define X29_SET_AND_READ_PARMS      6
#define X29_INVITATION_TO_CLEAR     1
#define X29_PARAMETER_INDICATION    0
#define X29_INDICATION_OF_BREAK     3
#define X29_ERROR                   5

#define INVALID_PAD_PARM            1

#define PAD_BREAK_CHARACTER         0

#define PAD_ESCAPE                  1
#define PAD_ECHO                    2
#define PAD_DATA_FORWARD_CHAR       3
#define PAD_DATA_FORWARD_TIMEOUT    4
#define PAD_FLOW_CONTROL_BY_PAD     5
#define PAD_SUPPRESSION_OF_SIGNALS  6
#define PAD_BREAK_ACTION            7
#define PAD_SUPPRESSION_OF_DATA     8
#define PAD_PADDING_AFTER_CR        9
#define PAD_LINE_FOLDING           10
#define PAD_LINE_SPEED             11
#define PAD_FLOW_CONTROL_BY_USER   12
#define PAD_LF_AFTER_CR            13
#define PAD_PADDING_AFTER_LF       14
#define PAD_EDITING                15
#define PAD_CHAR_DELETE_CHAR       16
#define PAD_BUFFER_DELETE_CHAR     17
#define PAD_BUFFER_DISPLAY_CHAR    18

#define MAXIX25 MAX_USER_DATA*7
#define MAXOX25 MAX_USER_DATA
#endif /* ANYX25 */

#ifdef SUNX25
#ifdef SOLARIS25                        /* and presumably SunLink 9.xx */
#include <fcntl.h>
#include <errno.h>
#include <sys/ioccom.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sundev/syncstat.h>
#include <netx25/x25_pk.h>
#include <netx25/x25_ctl.h>
#include <netx25/x25_ioctl.h>
#else
#include <sys/ioctl.h>                  /* X.25 includes, Sun only */
#include <sys/systm.h>
#ifndef SOLARIS
#include <sys/mbuf.h>
#endif /* SOLARIS */
#include <sys/socket.h>
#include <sys/protosw.h>
#ifdef SOLARIS
#include <sys/sockio.h>
#else
#include <sys/domain.h>
#endif /* SOLARIS */
#include <sys/socketvar.h>
#include <net/if.h>
#include <sundev/syncstat.h>
#include <netx25/x25_pk.h>
#include <netx25/x25_ctl.h>
#include <netx25/x25_ioctl.h>
#endif /* SOLARIS25 */
#endif /* SUNX25 */

#ifdef ANYX25

#ifdef IBMX25                           /* X.25 includes, AIX only */
#include <fcntl.h>
#include <sys/twtypes.h>
#include <sys/twlib.h>

#include <sys/stream.h>
#include <stropts.h>

#define NPI_20                          /* required to include the whole NPI */
#include <sys/npi_20.h>
#include <sys/npiapi.h>
#include <sys/pktintf.h>

#include <odmi.h>                       /* required for access to the ODM   */
#include <sys/cfgodm.h>                 /* database, needed to find out the */
                                        /* local NUA. see x25local_nua()    */


/* IBM X25 NPI generic primitive type */
typedef union N_npi_ctl_t {
    ulong               PRIM_type;              /* generic primitive type */
    char                buffer[NPI_MAX_CTL];    /* maximum primitive size */
    N_bind_ack_t        bind_ack;
    N_bind_req_t        bind_req;
    N_conn_con_t        conn_con;
    N_conn_ind_t        conn_ind;
    N_conn_req_t        conn_req;
    N_conn_res_t        conn_res;
    N_data_req_t        data_req;
    N_data_ind_t        data_ind;
    N_discon_ind_t      discon_ind;
    N_discon_req_t      discon_req;
    N_error_ack_t       error_ack;
    N_exdata_ind_t      exdata_ind;
    N_info_ack_t        info_ack;
    N_ok_ack_t          ok_ack;
    N_reset_con_t       reset_con;
    N_reset_req_t       reset_req;
    N_reset_ind_t       reset_ind;
} N_npi_ctl_t;

/* some extra definitions to help out */
typedef char    x25addr_t[45];          /* max 40 defined by CCITT */
typedef char    N_npi_data_t[NPI_MAX_DATA];

/* fd or server waiting for connections, used by netclos and netopen */
extern int x25serverfd;

#endif /* IBMX25 */

#ifdef HPX25                            /* X.25 includes, HP-UX only */
#include <x25/ccittproto.h>
#include <x25/x25.h>
#include <x25/x25addrstr.h>
#include <x25/x25codes.h>
#include <x25/x25hd_ioctl.h>
#include <x25/x25ioctls.h>
#include <x25/x25str.h>
#include <sys/ioctl.h>
#endif /* HPX25 */

/* C-Kermit X.3 / X.25 / X.29 / X.121 support functions */

/* (riehm: this list of functions isn't quite right for AIX) */

_PROTOTYP( int shopad, (int) );
_PROTOTYP( int shox25, (int) );
_PROTOTYP( VOID initpad, (void) );
_PROTOTYP( VOID setpad, (CHAR *, int) );
_PROTOTYP( VOID readpad, (CHAR *, int, CHAR *) );
_PROTOTYP( int qbitpkt, (CHAR *, int) );
_PROTOTYP( VOID setqbit, (void) );
_PROTOTYP( VOID resetqbit, (void) );
_PROTOTYP( VOID breakact, (void) );
_PROTOTYP( int pkx121, (char *, CHAR *) );
_PROTOTYP( SIGTYP x25oobh, (int) );
_PROTOTYP( int x25diag, (void) );
_PROTOTYP( int x25intr, (char) );
_PROTOTYP( int x25reset, (char, char) );
_PROTOTYP( int x25clear, (void) );
_PROTOTYP( int x25stat, (void) );
_PROTOTYP( int x25in, (int, CHAR *) );
_PROTOTYP( int setpadp, (void) );
_PROTOTYP( int setx25, (void) );
_PROTOTYP( int x25xin, (int, CHAR *) );
_PROTOTYP( int x25inl, (CHAR *, int, int, CHAR) );

#ifdef IBMX25
                                        /* setup x25 */
_PROTOTYP( ulong x25bind, (int, char *, char *, int, int, int, ulong) );
_PROTOTYP( int x25call, (int, char *, char *) ); /* connect to remote */
_PROTOTYP( int x25unbind, (int) );      /* disconnect */
_PROTOTYP( char *x25prim, (int) );      /* display primitives */
_PROTOTYP( int x25local_nua, (char *) ); /* find local NUA */
#endif /* IBMX25 */

#endif /* ANYX25 */

/* CMU-OpenVMS/IP */

#ifdef CMU_TCPIP                        /* CMU_TCPIP implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef TCPIPLIB
#define TCPIPLIB
#endif /* TCPIPLIB */
#endif /* CMU_TCPIP */

/* DEC TCP/IP for (Open)VMS, previously known as UCX */

#ifdef DEC_TCPIP                        /* DEC_TCPIP implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef TCPIPLIB
#define TCPIPLIB
#endif /* TCPIPLIB */
#endif /* DEC_TCPIP */

/* SRI/TGV/Cisco/Process MultiNet, TCP/IP for VAX/VMS */

#ifdef MULTINET                         /* MULTINET implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef TCPIPLIB
#define TCPIPLIB
#endif /* TCPIPLIB */
#ifndef TGVORWIN                        /* MULTINET and WINTCP */
#define TGVORWIN                        /* share a lot of code... */
#endif /* TGVORWIN */
#endif /* MULTINET */

/* Wollongong TCP/IP for VAX/VMS */

#ifdef WINTCP                           /* WINTCP implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef TCPIPLIB
#define TCPIPLIB
#endif /* TCPIPLIB */
#ifndef TGVORWIN                        /* WINTCP and MULTINET */
#define TGVORWIN                        /* share a lot of code... */
#endif /* TGVORWIN */
#endif /* WINTCP */

/* Wollongong TCP/IP for AT&T Sys V */

#ifdef WOLLONGONG                       /* WOLLONGONG implies TCPSOCKET */
#ifndef TCPSOCKET                       /* Don't confuse WOLLONGONG */
#define TCPSOCKET                       /* (which is for UNIX) with */
#endif /* TCPSOCKET */                  /* WINTCP, which is for VMS! */
#endif /* WOLLONGONG */

#ifdef EXCELAN                          /* EXCELAN implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* EXCELAN */

#ifdef INTERLAN                         /* INTERLAN implies TCPSOCKET */
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* INTERLAN */

#ifdef BEBOX
#ifndef TCPSOCKET
#define TCPSOCKET
#endif /* TCPSOCKET */
#ifndef TCPIPLIB
#define TCPIPLIB
#endif /* TCPIPLIB */
#define socket_errno    h_errno
#define socket_read(x,y,z)      recv(x,y,sizeof(char),z)
#define socket_write(x,y,z)     send(x,y,sizeof(char),z)
#define socket_ioctl    ioctl
#define socket_close(x)         closesocket(x)
#ifndef FIONBIO
#define FIONBIO 2
#endif /* FIONBIO */
#ifndef FIONREAD
#define FIONREAD       1
#endif /* FIONREAD */
#ifndef SIOCATMARK
#define SIOCATMARK     3
#endif /* SIOCATMARK */
#endif /* BEBOX */

#ifdef COMMENT /* no longer used but might come in handy again later... */
/*
  CK_READ0 can (and should) be defined if and only if:
  (a) read(fd,&x,0) can be used harmlessly on a TCP/IP socket connection.
  (b) read(fd,&x,0) returns 0 if the connection is up, -1 if it is down.
*/
#ifndef CK_READ0
#ifdef TCPSOCKET
#ifdef SUNOS41                          /* It works in SunOS 4.1 */
#define CK_READ0
#else
#ifdef NEXT                             /* and NeXTSTEP */
#define CK_READ0
#endif /* NEXT */
#endif /* SUNOS41 */
#endif /* TCPSOCKET */
#endif /* CK_READ0 */
#endif /* COMMENT */

/* Telnet protocol */

#ifdef TCPSOCKET                        /* TCPSOCKET implies TNCODE */
#ifndef TNCODE                          /* Which means... */
#define TNCODE                          /* Compile in telnet code */
#endif /* TNCODE */

/*
   Platforms where we must call gethostname(buf,len) and then
   gethostbyname(buf) to get local IP address, rather than calling
   gethostbyname("").
*/
#ifndef CKGHNLHOST
#ifdef datageneral
#define CKGHNLHOST
#else
#ifdef SOLARIS
#define CKGHNLHOST
#else
#ifdef SUNOS4
#define CKGHNLHOST
#else
#ifdef UNIXWARE
#define CKGHNLHOST
#else
#ifdef SINIX
#define CKGHNLHOST
#endif /* SINIX */
#endif /* UNIXWARE */
#endif /* SUNOS4 */
#endif /* SOLARIS */
#endif /* datageneral */
#endif /* CKGHNLHOST */

#ifndef RLOGCODE                        /* What about Rlogin? */
#ifndef NORLOGIN
/*
  Rlogin can be enabled only for UNIX versions that have both SIGURG
  (SCO doesn't) and CK_TTGWSIZ (OSF/1 doesn't), so we don't assume that
  any others have these without verifying first.  Not that it really makes
  much difference since you can only use Rlogin if you are root...
*/
#ifdef SUNOS41
#define RLOGCODE
#else
#ifdef SOLARIS
#define RLOGCODE
#else
#ifdef HPUX9
#define RLOGCODE
#else
#ifdef HPUX10
#define RLOGCODE
#else
#ifdef OSF40
#define RLOGCODE
#else
#ifdef NEXT
#define RLOGCODE
#else
#ifdef AIX41
#define RLOGCODE
#else
#ifdef UNIXWARE
#define RLOGCODE
#else
#ifdef IRIX51
#define RLOGCODE
#else
#ifdef IRIX60
#define RLOGCODE
#else
#ifdef QNX
#define RLOGCODE
#else
#ifdef __linux__
#define RLOGCODE
#else
#ifdef BSD44
#define RLOGCODE
#endif /* BSD44 */
#endif /* __linux__ */
#endif /* QNX */
#endif /* IRIX60 */
#endif /* IRIX51 */
#endif /* UNIXWARE */
#endif /* AIX41 */
#endif /* NEXT */
#endif /* OSF40 */
#endif /* HPUX10 */
#endif /* HPUX9 */
#endif /* SOLARIS */
#endif /* SUNOS41 */
#endif /* NORLOGIN */
#ifdef VMS                              /* VMS */
#define RLOGCODE
#endif /* VMS */
#endif /* RLOGCODE */
#endif /* TCPSOCKET */

#ifdef TNCODE
/*
  Telnet local-echo buffer, used for saving up user data that can't be
  properly displayed and/or evaluated until pending Telnet negotiations are
  complete.  TTLEBUF is defined for platforms (like UNIX) where net i/o is
  done by the same routines that do serial i/o (in which case the relevant
  code goes into the ck?tio.c module, in the ttinc(), ttchk(), etc, routines);
  NETLETBUF is defined for platforms (like VMS) that use different APIs for
  network and serial i/o, and enables the copies of the same routines that
  are in ckcnet.c.
*/
#ifndef TTLEBUF
#ifdef UNIX
#define TTLEBUF
#else
#ifdef datageneral
#define TTLEBUF
#endif /* datageneral */
#endif /* UNIX */
#endif /* TTLEBUF */

#ifndef NETLEBUF
#ifdef VMS
#define NETLEBUF
#endif /* VMS */
#endif /* NETLEBUF */
#endif /* TNCODE */

#ifdef SUNX25                           /* SUNX25 implies TCPSOCKET */
#ifndef TCPSOCKET                       /* But doesn't imply TNCODE */
#define TCPSOCKET
#endif /* TCPSOCKET */
#endif /* SUNX25 */

#ifndef TCPSOCKET
#ifndef NO_DNS_SRV
#define NO_DNS_SRV
#endif /* NO_DNS_SRV */
#endif /* TCPSOCKET */

/* This is another TCPSOCKET section... */

#ifdef TCPSOCKET
#ifndef NETCONN                         /* TCPSOCKET implies NETCONN */
#define NETCONN
#endif /* NETCONN */

#ifndef NO_DNS_SRV
#ifdef NOLOCAL
#define NO_DNS_SRV
#endif /* NOLOCAL */
#ifdef OS2ONLY
#define NO_DNS_SRV
#endif /* OS2ONLY */
#ifdef NT
#ifdef _M_PPC
#define NO_DNS_SRV
#endif /* _M_DNS */
#endif /* NO_DNS_SRV */
#ifdef VMS
#define NO_DNS_SRV
#endif /* VMS */
#ifdef STRATUS
#define NO_DNS_SRV
#endif /* STRATUS */
#ifdef datageneral
#define NO_DNS_SRV
#endif /* datageneral */
#ifdef ultrix
#define NO_DNS_SRV
#endif /* ultrix */
#ifdef NEXT
#define NO_DNS_SRV
#endif /* NEXT */
#endif /* NO_DNS_SRV */

#ifndef CK_DNS_SRV                      /* Use DNS SRV records to determine */
#ifndef NO_DNS_SRV                      /* host and ports */
#define CK_DNS_SRV
#endif /* NO_DNS_SRV */
#endif /* CK_DNS_SRV */

#ifndef NOLISTEN                        /* select() is required to support */
#ifndef SELECT                          /* incoming connections. */
#ifndef VMS
#ifndef OS2
#define NOLISTEN
#endif /* OS2 */
#endif /* VMS */
#endif /* SELECT */
#endif /* NOLISTEN */

/* BSD sockets library header files */

#ifdef VMS
/*
  Because bzero() and bcopy() are not portable among VMS versions,
  or compilers, or TCP/IP products, etc.
*/
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif /* bzero */
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif /* bcopy */
#endif /* VMS */

#ifdef HPUX6
/* These are missing in HP-UX 6.xx */
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif /* bzero */
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif /* bcopy */
#endif /* HPUX6 */

#ifdef UNIX                             /* UNIX section */

#ifdef SVR4
/*
  These suggested by Rob Healey, rhealey@kas.helios.mn.org, to avoid
  bugs in Berkeley compatibility library on Sys V R4 systems, but untested
  by me (fdc).  Remove this bit if it gives you trouble.
  (Later corrected by Marc Boucher <mboucher@iro.umontreal.ca> because
  bzero/bcopy are not argument-compatible with memset/memcpy|memmove.)
*/
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif
#ifdef SOLARIS
#ifdef SUNX25
#undef bzero
/*
  WOULD YOU BELIEVE... That the Solaris X.25 /opt/SUNWcomm/lib/libsockx25
  library references bzero, even though the use of bzero is forbidden in
  Solaris?  Look for the function definition in ckcnet.c.
*/
_PROTOTYP( void bzero, (char *, int) );
#endif /* SUNX25 */
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif
#else
#ifndef bcopy
#define bcopy(h,a,l) memmove(a,h,l)
#endif
#endif /* SOLARIS */
#else /* !SVR4 */
#ifdef PTX                              /* Sequent DYNIX PTX 1.3 */
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif
#endif /* PTX */
#endif /* SVR4 */

#ifdef INTERLAN                         /* Racal-Interlan TCP/IP */
#include <interlan/socket.h>
#include <interlan/il_types.h>
#include <interlan/telnet.h>
#include <interlan/il_errno.h>
#include <interlan/in.h>
#include <interlan/telnet.h>            /* Why twice ? ? ? */
#else /* Not Interlan */
#ifdef BEBOX
#include <socket.h>
#else /* Not BEBOX */                   /* Normal BSD TCP/IP library */
#ifdef COMMENT
#ifndef HPUX
#include <arpa/telnet.h>
#endif /* HPUX */
#endif /* COMMENT */
#ifdef SCO234
#include <sys/errno.tcp.h>
#include <sys/types.tcp.h>
#endif /* SCO234 */
#include <sys/socket.h>
#ifdef WOLLONGONG
#include <sys/in.h>
#else
#include <netinet/in.h>
#ifndef SV68R3V6                /* (maybe this should be SVR3 in general) */
#include <netinet/tcp.h>        /* Added June 2001 */
#endif /* SV68R3V6 */
#endif /* WOLLONGONG */
#endif /* BEBOX */
#endif /* INTERLAN */

#ifndef EXCELAN
#include <netdb.h>
#ifndef INTERLAN
#ifdef WOLLONGONG
#define minor                           /* Do not include <sys/macros.h> */
#include <sys/inet.h>
#else
#ifndef OXOS
#ifndef HPUX
#ifndef BEBOX
#include <arpa/inet.h>
#endif /* BEBOX */
#endif /* HPUX */
#else /* OXOS */
/* In too many releases of X/OS, <arpa/inet.h> declares inet_addr() as
 * ``struct in_addr''.  This is definitively wrong, and could cause
 * core dumps.  Instead of including that bad file, inet_addr() is
 * correctly declared here.  Of course, all the declarations done there
 * has been copied here.
 */
unsigned long inet_addr();
char    *inet_ntoa();
struct  in_addr inet_makeaddr();
unsigned long inet_network();
#endif /* OXOS */
#endif /* WOLLONGONG */
#endif /* INTERLAN */
#endif /* EXCELAN */

#ifdef EXCELAN                          /* Excelan TCP/IP */
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif /* bzero */
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif /* bcopy */
#include <ex_errno.h>
#endif /* EXCELAN */

#ifdef I386IX                           /* Interactive Sys V R3 network. */
/* #define TELOPTS */                   /* This might need defining. */
#define ORG_NLONG ENAMETOOLONG          /* Resolve conflicting symbols */
#undef ENAMETOOLONG                     /* in <errno.h> and <net/errno.h> */
#define ORG_NEMPTY ENOTEMPTY
#undef ENOTEMPTY
#include <net/errno.h>
#undef ENAMETOOLONG
#define ENAMETOOLONG ORG_NLONG
#undef ENOTEMPTY
#define ENOTEMPTY ORG_NEMPTY
#include <netinet/tcp.h>                /* for inet_addr() */
#endif /* I386IX */
/*
  Data type of the inet_addr() function...
  We define INADDRX if it is of type struct inaddr.
  If it is undefined, unsigned long is assumed.
  Look at <arpa/inet.h> to find out.  The following known cases are
  handled here.  Other systems that need it can be added here, or else
  -DINADDRX can be included in the CFLAGS on the cc command line.
*/
#ifndef NOINADDRX
#ifdef DU2                              /* DEC Ultrix 2.0 */
#define INADDRX
#endif /* DU2 */
#endif /* NOINADDRX */

#else /* Not UNIX */

#ifdef VMS                              /* (Open)VMS section */

#ifdef MULTINET                         /* TGV MultiNet */
/*
  In C-Kermit 7.0 Beta.08 we started getting scads of compile time warnings
  in Multinet builds: "blah" is implicitly declared as a function, where blah
  is socket_read/write/close, ntohs, htons, getpeername, accept, select, etc.
  I have no idea why -- these routines are declared in the header files below,
  and the includes haven't changed.  The executable still seems to work OK.
  Messing with the order of the following includes is disastrous.
*/
#ifdef MULTINET_NO_PROTOTYPES
#undef MULTINET_NO_PROTOTYPES
#endif /* MULTINET_NO_PROTOTYPES */

#ifdef  __cplusplus
#undef  __cplusplus
#endif /*  __cplusplus */

#include "multinet_root:[multinet.include]errno.h"
#include "multinet_root:[multinet.include.sys]types.h"
#include "multinet_root:[multinet.include.sys]socket.h"
#include "multinet_root:[multinet.include]netdb.h"
#include "multinet_root:[multinet.include.netinet]in.h"
#include "multinet_root:[multinet.include.arpa]inet.h"
#include "multinet_root:[multinet.include.sys]ioctl.h"

#ifdef COMMENT
/*
  No longer needed because now bzero/bcopy are macros defined as
  memset/memmove in all VMS builds.
*/
/*
  We should be able to pick these up from <strings.h> but it's
  not portable between VAXC and DECC.  And even with DECC 5.x we have a
  difference between VAX and Alpha.  We get warnings here on the VAX
  with DECC 5.6-003 but they are not fatal.
*/
#ifndef __DECC_VER
#ifndef bzero
_PROTOTYP( void bzero, (char *, int) );
#endif /* bzero */
#ifndef bcopy
_PROTOTYP( void bcopy, (char *, char *, int) );
#endif /* bcopy */
#endif /* __DECC_VER */
#endif /* COMMENT */

#ifdef __DECC
/*
   If compiling under DEC C the socket calls must not be prefixed with
   DECC$.  This is done by using the compiler switch /Prefix=Ansi_C89.
   However, this causes some calls that should be prefixed to not be
   (which I think is a bug in the compiler - I've been told these calls
   are present in ANSI compilers).  At any rate, such calls are fixed
   here by explicitly prefixing them.
*/
#ifdef COMMENT
/*
   But this causes errors with VMS 6.2 / DEC C 5.3-006 / MultiNet 4.0A on
   a VAX (but not on an Alpha).  So now what happens if we skip doing this?
*/
#define close decc$close
#define alarm decc$alarm
#endif /* COMMENT */
#endif /* __DECC */

#else /* Not MULTINET */

#ifdef WINTCP                           /* WIN/TCP = PathWay for VMS */
#ifdef OLD_TWG
#include "twg$tcp:[netdist.include.sys]errno.h"
#include "twg$tcp:[netdist.include.sys]types2.h" /* avoid some duplicates */
#else
#include "twg$tcp:[netdist.include]socket_aliases.h"
#include <errno.h>
#include "twg$tcp:[netdist.include.sys]types.h"
#endif /* OLD_TWG */
#include "twg$tcp:[netdist.include.sys]socket.h"
#include "twg$tcp:[netdist.include]netdb.h"
#include "twg$tcp:[netdist.include.sys]domain.h"
#include "twg$tcp:[netdist.include.sys]protosw.h"
#include "twg$tcp:[netdist.include.netinet]in.h"
#include "twg$tcp:[netdist.include.arpa]inet.h"
#include "twg$tcp:[netdist.include.sys]ioctl.h"

#else /* Not WINTCP */

#ifdef DEC_TCPIP
#ifdef UCX50
#ifndef IF_DOT_H
#define IF_DOT_H
#endif /*  IF_DOT_H */
#endif /* UCX50 */

#ifdef IF_DOT_H
#include <if.h>                         /* Needed to put up u_int typedef */
#else
#ifdef NEEDUINT
typedef unsigned int u_int;
#endif /* NEEDUINT */
#endif /* IF_DOT_H */

#include <in.h>
#ifdef VMS
#include <inet.h>			/* (SMS 2007/02/15) */
#endif	/* VMS */
#include <netdb.h>
#include <socket.h>
#include "ckvioc.h"
#define socket_errno errno

#ifdef COMMENT
/*
  No longer needed because now bzero/bcopy are macros defined as
  memset/memmove in all VMS builds.
*/
/*
  Translation: In <strings.h>, which exists only for DECC >= 5.2, bzero()
  and bcopy() are declared only for OpenVMS >= 7.0.  This still might need
  adjustment for DECC 5.0 and higher.
*/
#ifdef __DECC_VER
#ifdef VMSV70
/*
  Note: you can't use "#if (__VMS_VER>=70000000)" because that is not
  portable and kills non-VMS builds.
*/
#include <strings.h>
#else
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif
#ifndef bcopy
#define bcopy(h,a,l) memmove(a,h,l)
#endif
#endif /* VMSV70 */
#else
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif
#ifndef bcopy
#define bcopy(h,a,l) memmove(a,h,l)
#endif
#endif /* __DECC_VER */
#endif /* COMMENT */

#define socket_read     read
#define socket_write    write
#define socket_ioctl    ioctl
#define socket_close    close

#ifdef __DECC
int ioctl (int d, int request, void *argp);
#else
int ioctl (int d, int request, char *argp);
#endif /* DECC */
/*
  UCX supports select(), but does not provide the needed symbol and
  structure definitions in any header file, so ...
*/
#include <types.h>
#ifndef NBBY
/*-
 * Copyright (c) 1982, 1986, 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *      @(#)types.h     8.1 (Berkeley) 6/2/93
 */

#define NBBY    8               /* number of bits in a byte */

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef FD_SETSIZE
#define FD_SETSIZE      256
#endif

typedef long    fd_mask;
#define NFDBITS (sizeof(fd_mask) * NBBY)        /* bits per mask */

#ifndef howmany
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif

typedef struct fd_set {
        fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_COPY(f, t)   bcopy(f, t, sizeof(*(f)))
#define FD_ZERO(p)      bzero(p, sizeof(*(p)))
#endif /* !NBBY */

#else  /* Not DEC_TCPIP */

#ifdef CMU_TCPIP
#include <types.h>
#include <in.h>
#include <netdb.h>
#include <socket.h>
#include <inet.h>
#include <ioctl.h>
#include "ckvioc.h"
#define socket_errno errno

/*
 * Routines supplied in LIBCMU.OLB
 */
#define socket_ioctl    ioctl
#define socket_read     cmu_read
#define socket_write    cmu_write
#define socket_close    cmu_close

#endif /* CMU_TCPIP */
#endif /* DEC_TCPIP */
#endif /* WINTCP */
#endif /* MULTINET */

#else /* Not VMS */

#ifdef OS2
#include "ckonet.h"
#ifndef NT
#include <nerrno.h>
#endif
#endif /* OS2 */

#ifdef STRATUS  /* Stratus VOS using OS TCP/IP products S235, S236, S237 */
#include <tcp_socket.h>
/* This gets used some places when TCPSOCKET is defined. */
/* OS TCP provides bzero(), but not bcopy()... go figure. */
#define bcopy(s,d,z) memcpy(d,s,z)
#endif /* STRATUS */

#ifdef OSK
#ifndef OSKXXC
#include <inet/in.h>
#include <inet/netdb.h>
#include <inet/socket.h>
#else
#include <INET/in.h>
#include <INET/netdb.h>
#include <INET/socket.h>
#endif /* OSKXXC */
#ifndef bzero
#define bzero(s,n) memset(s,0,n)
#endif
#ifndef bcopy
#define bcopy(h,a,l) memcpy(a,h,l)
#endif
typedef char * caddr_t; /* core address type */
#endif /* OSK */

#endif /* VMS */
#endif /* UNIX */
#endif /* TCPSOCKET */

#ifndef NOINADDRX		      /* 301 - Needed for Solaris 10 and 11 */
#ifdef SOLARIS
#define NOINADDRX
#ifdef INADDR_NONE
#undef INADDR_NONE
#endif	/* INADDR_NONE */
#endif	/* SOLARIS */
#endif	/* NOINADDRX */

#ifdef NOINADDRX
#ifdef INADDRX
#undef INADDRX
#endif	/* INADDRX */
#endif	/* NOINADDRX */

#ifdef TCPSOCKET
#ifndef NOHADDRLIST
#ifndef HADDRLIST
#ifdef SUNOS41
#define HADDRLIST
#endif /* SUNOS41 */
#ifdef SOLARIS
#define HADDRLIST
#endif /* SOLARIS */
#ifdef LINUX
#define HADDRLIST
#endif /* LINUX */
#ifdef AIXRS
#define HADDRLIST
#endif /* AIXRS */
#ifdef HPUX
#define HADDRLIST
#endif /* HPUX */
#ifdef IRIX
#define HADDRLIST
#endif /* IRIX */
#ifdef I386IX
#define HADDRLIST
#endif /* I386IX */
#endif /* HADDRLIST */
/* A system that defines h_addr as h_addr_list[0] should be HADDRLIST */
#ifndef HADDRLIST
#ifdef h_addr
#define HADDRLIST
#endif /* h_addr */
#endif /* HADDRLIST */
#endif /* NOHADDRLIST */
#endif /* TCPSOCKET */

#ifdef TNCODE                           /* If we're compiling telnet code... */
#ifndef IKS_OPTION
#ifndef STRATUS
#define IKS_OPTION
#endif /* STRATUS */
#endif /* IKS_OPTION */
#include "ckctel.h"
#else
extern int sstelnet;
#ifdef IKSD
#undef IKSD
#endif /* IKSD */
#ifndef NOIKSD
#define NOIKSD
#endif /* NOIKSD */
#ifdef IKS_OPTION
#undef IKS_OPTION
#endif /* IKS_OPTION */
#endif /* TNCODE */

#ifndef NOTCPOPTS
/*
  Automatically define NOTCPOPTS for configurations where they can't be
  used at runtime or cause too much trouble at compile time.
*/
#ifdef CMU_TCPIP                        /* CMU/Tek */
#define NOTCPOPTS
#endif /* CMU_TCPIP */
#ifdef MULTINET                         /* Multinet on Alpha */
#ifdef __alpha
#define NOTCPOPTS
#endif /* __alpha */
#endif /* MULTINET */
#endif /* NOTCPOPTS */

#ifdef NOTCPOPTS
#ifdef TCP_NODELAY
#undef TCP_NODELAY
#endif /* TCP_NODELAY */
#ifdef SO_LINGER
#undef SO_LINGER
#endif /* SO_LINGER */
#ifdef SO_KEEPALIVE
#undef SO_KEEPALIVE
#endif /* SO_KEEPALIVE */
#ifdef SO_SNDBUF
#undef SO_SNDBUF
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
#undef SO_RCVBUF
#endif /* SO_RCVBUF */
#endif /* NOTCPOPTS */

/* This function is declared even when TCPSOCKET is not available */
_PROTOTYP( char * ckgetpeer, (VOID));

#ifdef TCPSOCKET
#ifdef SOL_SOCKET
#ifdef TCP_NODELAY
_PROTOTYP( int no_delay, (int, int) );
#endif /* TCP_NODELAY */
#ifdef SO_KEEPALIVE
_PROTOTYP( int keepalive, (int, int) ) ;
#endif /* SO_KEEPALIVE */
#ifdef SO_LINGER
_PROTOTYP( int ck_linger, (int, int, int) ) ;
#endif /* SO_LINGER */
#ifdef SO_SNDBUF
_PROTOTYP( int sendbuf,(int, int) ) ;
#endif /* SO_SNDBUF */
#ifdef SO_RCVBUF
_PROTOTYP( int recvbuf, (int, int) ) ;
#endif /* SO_RCVBUF */
#ifdef SO_DONTROUTE
_PROTOTYP(int dontroute, (int, int));
#endif /* SO_DONTROUTE */
#endif /* SOL_SOCKET */
_PROTOTYP( int getlocalipaddr, (VOID));
_PROTOTYP( int getlocalipaddrs, (char *,int,int));
_PROTOTYP( char * ckgetfqhostname,(char *));
_PROTOTYP( struct hostent * ck_copyhostent,(struct hostent *));
_PROTOTYP( char * ckname2addr, (char *));
_PROTOTYP( char * ckaddr2name, (char *));

/* AIX */

#ifdef AIXRS
#ifndef TCP_NODELAY
#define TCP_NODELAY 0x1
#endif /* TCP_NODELAY */
#ifndef TCP_MAXSEG
#define TCP_MAXSEG 0x2
#endif /* TCP_MAXSEG */
#ifndef TCP_KEEPALIVE
#define TCP_KEEPALIVE 0x8
#endif /* TCP_KEEPALIVE */
#endif /* AIXRS */
#endif /* TCPSOCKET */

#ifdef RLOGCODE
#ifndef CK_TTGWSIZ
SORRY_RLOGIN_REQUIRES_TTGWSIZ_see_ckcplm.doc
#endif /* CK_TTGWSIZ */
#endif /* RLOGCODE */

#ifdef CK_NAWS
#ifndef CK_TTGWSIZ
SORRY_CK_NAWS_REQUIRES_TTGWSIZ_see_ckcplm.doc
#endif /* CK_TTGWSIZ */
#endif /* CK_NAWS */

#ifndef PF_INET
#ifdef  AF_INET
#define PF_INET AF_INET
#endif /* AF_INET */
#endif /* PF_INET */

#ifndef IPPORT_ECHO
#define IPPORT_ECHO 7
#endif /* IPPORT_ECHO */

#ifdef CK_KERBEROS
#ifdef RLOGCODE
_PROTOTYP(int ck_krb_rlogin,(CHAR *, int, CHAR *, CHAR *, CHAR *,
                              struct sockaddr_in *,
                              struct sockaddr_in *, int, int));
#endif /* RLOGCODE */
#endif /* CK_KERBEROS */

_PROTOTYP( VOID ini_kerb, ( void ) );   /* Kerberos initialization routine */
_PROTOTYP( int doauth, (int) );         /* AUTHENTICATE action routine */

#ifdef CK_DNS_SRV
_PROTOTYP(int locate_srv_dns,(char *host, char *service,
                              char *protocol, struct sockaddr **addr_pp,
                              int *naddrs));
#endif /* CK_DNS_SRV */

#ifndef NOHTTP
_PROTOTYP(int http_open, (char *, char *, int, char *, int, char *));
_PROTOTYP(int http_reopen, (VOID));
_PROTOTYP(int http_close, (VOID));
_PROTOTYP(int http_get, (char *,char **,char *,char *,char,char *,char *,
                         int));
_PROTOTYP(int http_head, (char *,char **,char *,char *,char,char *,char *,
                          int));
_PROTOTYP(int http_put, (char *,char **,char *,char *,char *,char,char *,
                         char *, char *, int));
_PROTOTYP(int http_delete, (char *,char **,char *,char *,char,char *));
_PROTOTYP(int http_connect, (int, char *,char **,char *,char *,char,char *));
_PROTOTYP(int http_post, (char *,char **,char *,char *,char *,char,char *,
                  char *,char *, int));
_PROTOTYP(int http_index, (char *,char **,char *,char *,char,char *,char *,
                           int));
_PROTOTYP(int http_inc, (int));
_PROTOTYP(int http_isconnected, (void));

extern char * tcp_http_proxy;           /* Name[:port] of http proxy server */
extern int    tcp_http_proxy_errno;     /* Return value from server */
extern char * tcp_http_proxy_user;      /* Name of user for authentication */
extern char * tcp_http_proxy_pwd;       /* Password of user */
#endif /* NOHTTP */

#ifdef TCPSOCKET

/* Type needed as 5th argument (length) to get/setsockopt() */

#ifdef TRU64
/* They say it themselves - this does not conform to standards */
#define socklen_t int
#else
#ifdef HPUX
#define socklen_t int
#endif	/* HPUX */
#endif	/* TRU64 */

#ifndef SOCKOPT_T
#ifdef CK_64BIT
#define SOCKOPT_T socklen_t
#endif	/* CK_64BIT */
#endif	/* SOCKOPT_T */

#ifndef SOCKOPT_T
#define SOCKOPT_T int
#ifdef MACOSX10
#undef SOCKOPT_T
#define SOCKOPT_T unsigned int
#else
#ifdef AIX42
#undef SOCKOPT_T
#define SOCKOPT_T unsigned long
#else
#ifdef PTX
#undef SOCKOPT_T
#define SOCKOPT_T size_t
#else
#ifdef NT
#undef SOCKOPT_T
#define SOCKOPT_T int
#else /* NT */
#ifdef UNIXWARE
#undef SOCKOPT_T
#define SOCKOPT_T size_t
#else /* UNIXWARE */
#ifdef VMS
#ifdef DEC_TCPIP
#ifdef __DECC_VER
#undef SOCKOPT_T
#define SOCKOPT_T size_t
#endif /* __DECC_VER */
#endif /* DEC_TCPIP */
#endif /* VMS */
#endif /* UNIXWARE */
#endif /* NT */
#endif /* PTX */
#endif /* AIX42 */
#endif /* MACOSX10 */
#endif /* SOCKOPT_T */

/* Ditto for getsockname() */

#ifndef GSOCKNAME_T
#ifdef CK_64BIT
#define GSOCKNAME_T socklen_t
#endif	/* CK_64BIT */
#endif	/* GSOCKNAME_T */

#ifndef GSOCKNAME_T
#define GSOCKNAME_T int
#ifdef MACOSX10
#undef GSOCKNAME_T
#define GSOCKNAME_T unsigned int
#else
#ifdef PTX
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#else
#ifdef AIX42                            /* size_t in 4.2++, int in 4.1-- */
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#else
#ifdef UNIXWARE
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#else
#ifdef VMS
#ifdef DEC_TCPIP
#ifdef __DECC_VER
#undef GSOCKNAME_T
#define GSOCKNAME_T size_t
#endif /* __DECC_VER */
#endif /* DEC_TCPIP */
#endif /* VMS */
#endif /* UNIXWARE */
#endif /* AIX41 */
#endif /* PTX */
#endif /* MACOSX10 */
#endif /* GSOCKNAME_T */

#endif /* TCPSOCKET */

#ifdef MACOSX10
#ifdef bcopy
#undef bcopy
#endif
#ifdef bzero
#undef bzero
#endif
#endif /* MACOSX10 */

#endif /* CKCNET_H */
