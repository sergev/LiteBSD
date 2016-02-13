char *cktelv = "Telnet support, 9.0.274, 16 Mar 2010";
#define CKCTEL_C

int sstelnet = 0;                       /* Do server-side Telnet negotiation */

/*  C K C T E L  --  Telnet support  */

/*
  Authors:
    Telnet protocol by Frank da Cruz and Jeffrey Altman.
    Telnet Forward X by Jeffrey Altman
    Telnet START_TLS support by Jeffrey Altman
    Telnet AUTH and ENCRYPT support by Jeffrey Altman
    Telnet COMPORT support by Jeffrey Altman
    Telnet NEW-ENVIRONMENT support by Jeffrey Altman
    Telnet NAWS support by Frank da Cruz and Jeffrey Altman
    Telnet TERMTYPE support by Jeffrey Altman
    Telnet KERMIT support by Jeffrey Altman
    Other contributions as indicated in the code.

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
  NOTE TO CONTRIBUTORS: This file, and all the other shared (ckc and cku)
  C-Kermit source files, must be compatible with C preprocessors that support
  only #ifdef, #else, #endif, #define, and #undef.  Please do not use #if,
  logical operators, or other preprocessor features in this module.  Also,
  don't use any ANSI C constructs except within #ifdef CK_ANSIC..#endif.
*/

#include "ckcsym.h"
#include "ckcdeb.h"

#ifdef TNCODE
#include "ckcker.h"
#define TELCMDS                         /* to define name array */
#define TELOPTS                         /* to define name array */
#define SLC_NAMES                       /* to define name array */
#define ENCRYPT_NAMES
#define AUTH_NAMES
#define TELOPT_STATES
#define TELOPT_MODES
#define TNC_NAMES
#include "ckcnet.h"
#include "ckctel.h"
#ifdef CK_AUTHENTICATION
#include "ckuath.h"
#endif /* CK_AUTHENTICATION */
#ifdef CK_SSL
#include "ck_ssl.h"
#endif /* CK_SSL */

#ifndef NOTERM
#ifdef OS2                              /* For terminal type name string */
#include "ckuusr.h"
#ifndef NT
#include <os2.h>
#undef COMMENT
#else
#define isascii __isascii
#endif /* NT */
#include "ckocon.h"
extern int tt_type, max_tt;
extern struct tt_info_rec tt_info[];
#endif /* OS2 */
#endif /* NOTERM */

#ifdef OS2
#include <assert.h>
#ifdef NT
#include <setjmpex.h>
#else /* NT */
#include <setjmp.h>
#endif /* NT */
#include <signal.h>
#include "ckcsig.h"
#include "ckosyn.h"
#endif /* OS2 */

#ifdef CK_NAWS                          /* Negotiate About Window Size */
#ifdef RLOGCODE
_PROTOTYP( int rlog_naws, (void) );
#endif /* RLOGCODE */
#endif /* CK_NAWS */

int tn_init = 0;                        /* Telnet protocol initialized flag */
int tn_begun = 0;                       /* Telnet protocol started flag */
static int tn_first = 1;                /* First time init flag */
extern int tn_exit;                     /* Exit on disconnect */
extern int nettype;
extern int inserver;                    /* Running as IKSD */
char *tn_term = NULL;                   /* Terminal type override */

#ifdef CK_SNDLOC
char *tn_loc = NULL;                    /* Location override */
#endif /* CK_SNDLOC */
int tn_nlm = TNL_CRLF;                  /* Telnet CR -> CR LF mode */
int tn_b_nlm = TNL_CR;                  /* Telnet Binary CR RAW mode */
int tn_b_meu = 0;                       /* Telnet Binary ME means U too */
int tn_b_ume = 0;                       /* Telnet Binary U means ME too */
int tn_wait_flg = 1;                    /* Telnet Wait for Negotiations */
int tn_infinite = 0;                    /* Telnet Bug Infinite-Loop-Check */
int tn_rem_echo = 1;                    /* We will echo if WILL ECHO */
int tn_b_xfer = 0;                      /* Telnet Binary for Xfers? */
int tn_sb_bug = 1;                      /* Telnet BUG - SB w/o WILL or DO */
int tn_auth_krb5_des_bug = 1;           /* Telnet BUG - AUTH KRB5 DES */
                                        /*              truncates long keys */
int tn_no_encrypt_xfer = 0;             /* Turn off Telnet Encrypt? */
int tn_delay_sb = 1;                    /* Delay SBs until safe */
int tn_auth_how = TN_AUTH_HOW_ANY;
int tn_auth_enc = TN_AUTH_ENC_ANY;
int tn_deb = 0;                         /* Telnet Debug mode */
int tn_sfu = 0;                         /* Microsoft SFU compatibility */
#ifdef CK_FORWARD_X
char * tn_fwdx_xauthority = NULL;       /* Xauthority File */
int    fwdx_no_encrypt = 0;             /* Forward-X requires encryption */
#endif /* CK_FORWARD_X */

#ifdef OS2
int ttnum = -1;                         /* Last Telnet Terminal Type sent */
int ttnumend = 0;                       /* Has end of list been found */
#endif /* OS2 */

char tn_msg[TN_MSG_LEN];                /* Telnet data can be rather long */
char hexbuf[TN_MSG_LEN];
char tn_msg_out[TN_MSG_LEN];
#ifdef CK_FORWARD_X
CHAR fwdx_msg_out[TN_MSG_LEN];
#endif /* CK_FORWARD_X */

/*
  In order to prevent an infinite telnet negotiation loop we maintain a
  count of the number of times the same telnet negotiation message is
  sent. When this count hits MAXTNCNT, we do not send any more of the
  message. The count is stored in the tncnts[][] array.

  The tncnts[][] array is indexed by negotiation option (SUPPRESS GO AHEAD,
  TERMINAL TYPE, NAWS, etc. - see the tnopts[] array) and the four
  negotiation message types (WILL, WONT, DO, DONT).  All telnet negotiations
  are kept track of in this way.

  The count for a message is zeroed when the "opposite" message is sent.
  WILL is the opposite of WONT, and DO is the opposite of DONT.
  For example sending "WILL SGA" increments tncnts[TELOPT_SGA][0]
  and zeroes tncnts[TELOPT_SGA][1].

  The code that does this is in tn_sopt().

  rogersh@fsj.co.jp, 18/3/1995

  8/16/1998 - with the recent rewrite of the telnet state machine I don't
  think this code is necessary anymore.  However, it can't do any harm so
  I am leaving it in.    - Jeff

  12/28/1998 - all references to tncnts[] must be done with TELOPT_INDEX(opt)
  because the Telnet option list is no longer contiguous.  We also must
  allocate NTELOPTS + 1 because the TELOPT_INDEX() macro returns NTELOPTS
  for an invalid option number.
*/

#define MAXTNCNT 4      /* Permits 4 intermediate telnet firewalls/gateways */

char tncnts[NTELOPTS+1][4];             /* Counts */
char tnopps[4] = { 1,0,3,2 };           /* Opposites */

#ifdef CK_ENVIRONMENT
#ifdef CK_FORWARD_X
#define TSBUFSIZ 2056
#else /* CK_FORWARD_X */
#define TSBUFSIZ 1024
#endif /* CK_FORWARD_X */
char tn_env_acct[64];
char tn_env_disp[64];
char tn_env_job[64];
char tn_env_prnt[64];
char tn_env_sys[64];
char * tn_env_uservar[8][2];
int tn_env_flg = 1;
#else /* CK_ENVIRONMENT */
#define TSBUFSIZ 41
int tn_env_flg = 0;
#endif /* CK_ENVIRONMENT */

#ifdef COMMENT
/* SIGWINCH handler moved to ckuusx.c */
#ifndef NOSIGWINCH
#ifdef CK_NAWS                          /* Window size business */
#ifdef UNIX
#include <signal.h>
#endif /* UNIX */
#endif /* CK_NAWS */
#endif /* NOSIGWINCH */
#endif /* COMMENT */

CHAR sb[TSBUFSIZ];                      /* Buffer - incoming subnegotiations */
CHAR sb_out[TSBUFSIZ];                  /* Buffer - outgoing subnegotiations */

int tn_duplex = 1;                      /* Local echo */

extern char uidbuf[];                   /* User ID buffer */
extern int quiet, ttnet, ttnproto, debses, what, duplex, oldplex, local;
extern int seslog, sessft, whyclosed;
#ifdef OS2
#ifndef NOTERM
extern int tt_rows[], tt_cols[];
extern int tt_status[VNUM];
extern int scrninitialized[];
#endif /* NOTERM */
#else /* OS2 */
extern int tt_rows, tt_cols;            /* Everybody has this */
#endif /* OS2 */
extern int cmd_cols, cmd_rows;
extern char namecopy[];
extern char myipaddr[];             /* Global copy of my IP address */

#ifndef TELOPT_MACRO
int
telopt_index(opt) int opt; {
    if (opt >= 0 && opt <= TELOPT_STDERR)
      return(opt);
    else if (opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT)
      return(opt-88);
    else if (opt == TELOPT_IBM_SAK)
      return(opt-147);
    else
      return(NTELOPTS);
}

int
telopt_ok(opt) int opt; {
    return((opt >= TELOPT_BINARY && opt <= TELOPT_STDERR) ||
           (opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT) ||
           (opt == TELOPT_IBM_SAK));
}

CHAR *
telopt(opt) int opt; {
    if (telopt_ok(opt))
      return((CHAR *)telopts[telopt_index(opt)]);
    else
      return((CHAR *)"UNKNOWN");
}

int
telopt_mode_ok(opt) int opt; {
    return((unsigned int)(opt) <= TN_NG_MU);
}

CHAR *
telopt_mode(opt) int opt; {
    if (telopt_mode_ok(opt))
      return((CHAR *)telopt_modes[opt-TN_NG_RF]);
    else
      return((CHAR *)"UNKNOWN");
}
#endif /* TELOPT_MACRO */

static int
tn_outst(notquiet) int notquiet; {
    int outstanding = 0;
    int x = 0;
#ifdef CK_ENCRYPTION
    int e = 0;
    int d = 0;
#endif /* CK_ENCRYPTION */

    if (tn_wait_flg) {
        for (x = TELOPT_FIRST; x <= TELOPT_LAST; x++) {
            if (TELOPT_OK(x)) {
                if (TELOPT_UNANSWERED_WILL(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to WILL %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered WILL",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_DO(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to DO %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered DO",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_WONT(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to WONT %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered WONT",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_DONT(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to DONT %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered DONT",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
                if (TELOPT_UNANSWERED_SB(x)) {
                    if ( notquiet )
                      printf("?Telnet waiting for response to SB %s\r\n",
                             TELOPT(x));
                    debug(F111,"tn_outst","unanswered SB",x);
                    outstanding = 1;
                    if ( !notquiet )
                      break;
                }
            }
        }
#ifdef CK_AUTHENTICATION
        if (ck_tn_auth_in_progress()) {
            if (TELOPT_ME(TELOPT_AUTHENTICATION)) {
                if (notquiet)
                  printf("?Telnet waiting for WILL %s subnegotiation\r\n",
                         TELOPT(TELOPT_AUTHENTICATION));
                debug(F111,
                      "tn_outst",
                      "ME authentication in progress",
                      TELOPT_AUTHENTICATION
                      );
                outstanding = 1;
            } else if (TELOPT_U(TELOPT_AUTHENTICATION)) {
                if (notquiet)
                  printf("?Telnet waiting for DO %s subnegotiation\r\n",
                         TELOPT(TELOPT_AUTHENTICATION));
                debug(F111,
                      "tn_outst",
                      "U authentication in progress",
                      TELOPT_AUTHENTICATION
                      );
                outstanding = 1;
            }
        }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
        if (!outstanding) {
            e = ck_tn_encrypting();
            d = ck_tn_decrypting();
            if (TELOPT_ME(TELOPT_ENCRYPTION)) {
                if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && e ||
                    !TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && !e
                    ) {
                    if ( notquiet )
                      printf("?Telnet waiting for WILL %s subnegotiation\r\n",
                             TELOPT(TELOPT_ENCRYPTION));
                    debug(F111,
                          "tn_outst",
                          "encryption mode switch",
                          TELOPT_ENCRYPTION
                          );
                    outstanding = 1;
                }
            }
            if (TELOPT_U(TELOPT_ENCRYPTION)) {
                if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && d ||
                    !TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop && !d
                    ) {
                    if ( notquiet )
                      printf("?Telnet waiting for DO %s subnegotiation\r\n",
                             TELOPT(TELOPT_ENCRYPTION));
                    debug(F111,
                          "tn_outst",
                          "decryption mode switch",
                           TELOPT_ENCRYPTION
                          );
                    outstanding = 1;
                }
            }
        }
#endif /* CK_ENCRYPTION */
    } /* if (tn_wait_flg) */

#ifdef IKS_OPTION
    /* Even if we are not waiting for Telnet options we must wait for */
    /* Kermit Telnet Subnegotiations if we have sent a request to the */
    /* other guy.  Otherwise we will get out of sync.                 */
    if (!outstanding) {
        if (TELOPT_U(TELOPT_KERMIT) &&
            (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start ||
             TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop ||
             !TELOPT_SB(TELOPT_KERMIT).kermit.sop)
            ) {
            if ( notquiet )
              printf("?Telnet waiting for SB %s negotiation\r\n",
                     TELOPT(TELOPT_KERMIT));
            debug(F111,"tn_outst","U kermit in progress",TELOPT_KERMIT);
            outstanding = 1;
        }
    }
#endif /* IKS_OPTION */

#ifdef TN_COMPORT
    if (!outstanding) {
        if (TELOPT_ME(TELOPT_COMPORT)) {
            if (TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb) {
                if (notquiet)
                    printf("?Telnet waiting for SB %s negotiation\r\n",
                            TELOPT(TELOPT_COMPORT));
               debug(F111,"tn_outst","ComPort SB in progress",TELOPT_COMPORT);
               outstanding = 1;
            }
            if (TELOPT_SB(TELOPT_COMPORT).comport.wait_for_ms) {
                if (notquiet)
              printf("?Telnet waiting for SB %s MODEM_STATUS negotiation\r\n",
                            TELOPT(TELOPT_COMPORT));
            debug(F111,"tn_outst","ComPort SB MS in progress",TELOPT_COMPORT);
               outstanding = 1;
            }
        }
    }
#endif /* TN_COMPORT */
    return(outstanding);
}

int
istncomport() {
#ifdef TN_COMPORT
    if (!local) return(0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (TELOPT_ME(TELOPT_COMPORT))
      return(1);
    else
#endif /* TN_COMPORT */
      return(0);
}

/* tn_wait() -- Wait for response to Telnet negotiation. */
/*
  Wait for up to <timeout> seconds for the response to arrive.
  Place all non-telnet data into Telnet Wait Buffer.
  If response does arrive return 1, else return 0.
*/
#ifndef TN_WAIT_BUF_SZ
#define TN_WAIT_BUF_SZ 4096
#endif /* TN_WAIT_BUF_SZ */
static char tn_wait_buf[TN_WAIT_BUF_SZ];
static int  tn_wait_idx = 0;
#ifndef TN_TIMEOUT
#define TN_TIMEOUT 120
#endif /* TN_TIMEOUT */
static int tn_wait_tmo = TN_TIMEOUT;

#ifdef CKSPINNER
VOID
prtwait(state) int state; {
    switch (state % 4) {
      case 0:
        printf("/");
        break;
      case 1:
        printf("-");
        break;
      case 2:
        printf("\\");
        break;
      case 3:
        printf("|");
        break;
    }
}
#endif /* CKSPINNER */

static int nflag = 0;

int
#ifdef CK_ANSIC
tn_wait(char * where)
#else
tn_wait(where) char * where;
#endif /* CK_ANSIC */
/* tn_wait */ {
    extern int ckxech, local;
    int ch = 0, count = 0;
#ifndef NOHINTS
    int nohintgiven = 1;
    extern int hints;
#endif /* NOHINTS */
    int outstanding;
#ifdef TN_COMPORT
    int savcarr;
    extern int ttcarr;
#endif /* TN_COMPORT */

    /* if (!IS_TELNET()) return(1); */

    rtimer();

    debug(F110,"tn_wait waiting for",where,0);
    tn_wait_tmo = TN_TIMEOUT;
    debug(F111,"tn_wait","timeout",tn_wait_tmo);
    outstanding = tn_outst(0);
    debug(F111,"tn_wait","outstanding",outstanding);
    debug(F111,"tn_wait","tn_wait_flg",tn_wait_flg);

    /* The following is meant to be !(||).  We only want to return */
    /* immediately if both the tn_wait_flg && tn_outst() are false */
    if (!(outstanding || tn_wait_flg))  /* If no need to wait */
      return(1);                        /* Don't. */

    if (tn_deb || debses) tn_debug("<wait for outstanding negotiations>");

#ifdef CKSPINNER
    if (!sstelnet && !quiet)
        prtwait(0);
#endif /* CKSPINNER */

    /* Wait up to TN_TIMEOUT sec for responses to outstanding telnet negs */
    do {
#ifdef NTSIG
        ck_ih();
#endif /* NTSIG */
        ch = ttinc(1);
        if (ch == -1) {                 /* Timed out */
            if (!sstelnet && !quiet) {  /* Let user know... */
#ifdef CKSPINNER
                printf("\b");
                prtwait(gtimer());
#else
                if (nflag == 0) {
                    printf(" Negotiations.");
                    nflag++;
                }
                if (nflag > 0) {
                    printf(".");
                    nflag++;
                    fflush(stdout);
                }
#endif /* CKSPINNER */
            }
        } else if (ch < -1) {
            printf("\r\n?Connection closed by peer.\r\n");
            if (tn_deb || debses) tn_debug("<connection closed by peer>");
            return(-1);
        } else
        switch (ch) {
          case IAC:
#ifdef CKSPINNER
            if (!sstelnet && !quiet)
              printf("\b");
#endif /* CKSPINNER */
            ch = tn_doop((CHAR)(ch & 0xff),inserver?ckxech:duplex,ttinc);
#ifdef CKSPINNER
            if (!sstelnet && !quiet) {
                prtwait(gtimer());
            }
#endif /* CKSPINNER */
            debug(F101,"tn_wait tn_doop","",ch);
            switch (ch) {
              case 1:
                duplex = 1;             /* Turn on echoing */
                if (inserver)
                  ckxech = 1;
                break;
              case 2:
                duplex = 0;             /* Turn off echoing */
                if (inserver)
                  ckxech = 0;
                break;
              case 3:
                tn_wait_buf[tn_wait_idx++] = IAC;
                break;
              case 4:                   /* IKS event */
              case 6:                   /* Logout */
                break;
              case -1:
		if (!quiet)
                printf("?Telnet Option negotiation error.\n");
                if (tn_deb || debses)
                  tn_debug("<Telnet Option negotiation error>");
                return(-1);
              case -2:
                printf("?Connection closed by peer.\n");
                if (tn_deb || debses) tn_debug("<Connection closed by peer>");
		ttclos(0);
                return(-2);
              default:
                if (ch < 0) {
                  if (tn_deb || debses) tn_debug("<Unknown connection error>");
                  return(ch);
                }
            } /* switch */
            break;
          default:
#ifdef CKSPINNER
            if (!sstelnet && !quiet) {
                printf("\b");
                prtwait(gtimer());
            }
#endif /* CKSPINNER */
            tn_wait_buf[tn_wait_idx++] = (CHAR)(ch & 0xff);
        } /* switch */

        outstanding = tn_outst(0);

        if ( outstanding && ch != IAC ) {
            int timer = gtimer();
            if ( timer > tn_wait_tmo ) {
                if (!sstelnet) {
                    printf(
                    "\r\n?Telnet Protocol Timeout - connection closed\r\n");
                    if (tn_deb || debses)
                        tn_debug(
                        "<telnet protocol timeout - connection closed>");
                    tn_outst(1);
                }
                /* if we do not close the connection, then we will block */
                /* the next time we hit a wait.  and if we don't we will */
                /* do the wrong thing if the host sends 0xFF and does    */
                /* not intend it to be an IAC.                           */
                ttclos(0);
                whyclosed = WC_TELOPT;
                return(-1);
            }
#ifndef NOHINTS
            else if ( hints && timer > 30 && nohintgiven && !inserver ) {
#ifdef CKSPINNER
                                printf("\b");
#else /* CKSPINNER */
                                printf("\r\n");
#endif /* CKSPINNER */
      printf("*************************\r\n");
        printf("The Telnet %s is not sending required responses.\r\n\r\n",
                sstelnet?"client":"server");
      tn_outst(1);
      printf("\nYou can continue to wait or you can cancel with Ctrl-C.\r\n");
      printf("In case the Telnet server never responds as required,\r\n");
      printf("you can try connecting to this host with TELNET /NOWAIT.\r\n");
      printf("Use SET HINTS OFF to suppress further hints.\r\n");
      printf("*************************\r\n");
      nohintgiven = 0;
            }
#endif /* NOHINTS */
        }

#ifdef TN_COMPORT
        /* Must disable carrier detect check if we are using Telnet Comport */
        savcarr = ttcarr;
        ttscarr(CAR_OFF);
        count = ttchk();
        ttscarr(savcarr);
#else /* TN_COMPORT */
        count = ttchk();
#endif /* TN_COMPORT */
    } while ((tn_wait_idx < TN_WAIT_BUF_SZ) &&
             (outstanding && count >= 0));

    if (tn_wait_idx == TN_WAIT_BUF_SZ) {
      if (tn_deb || debses) tn_debug("<Telnet Wait Buffer filled>");
      return(0);
    }

    if (!sstelnet && !quiet) {
#ifdef CKSPINNER
        printf("\b \b");
#else
        if (nflag > 0) {
            printf(" (OK)\n");
            nflag = -1;
        }
#endif /* CKSPINNER */
    }
    if (tn_deb || debses) tn_debug("<no outstanding negotiations>");
    return(0);
}

/* Push data from the Telnet Wait Buffer into the I/O Queue */
/* Return 1 on success                                      */

int
tn_push() {
#ifdef NETLEBUF
    extern int tt_push_inited;
#endif /* NETLEBUF */
    /* if (!IS_TELNET()) return(1); */

    if (tn_wait_idx) {
        ckhexdump((CHAR *)"tn_push",tn_wait_buf,tn_wait_idx);
#ifdef NETLEBUF
        if (!tt_push_inited)            /* Local handling */
          le_init();
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else                                   /* External handling... */
#ifdef OS2                              /* K95 has its own way */
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else
#ifdef TTLEBUF                          /* UNIX, etc */
        le_puts((CHAR *)tn_wait_buf,tn_wait_idx);
#else
/*
  If you see this message in AOS/VS, OS-9, VOS, etc, you need to copy
  the #ifdef TTLEBUF..#endif code from ckutio.c to the corresponding
  places in your ck?tio.c module.
*/
        printf("tn_push called but not implemented - data lost.\n");
#endif /* TTLEBUF */
#endif /* OS2 */
#endif /* NETLEBUF */
        tn_wait_idx = 0;
    }
    tn_wait_tmo = TN_TIMEOUT;           /* Reset wait timer stats */
    return(1);
}

/*  T N _ S O P T  */
/*
   Sends a telnet option, avoids loops.
   Returns 1 if command was sent, 0 if not, -1 on error.
*/
int
tn_sopt(cmd,opt) int cmd, opt; {        /* TELNET SEND OPTION */
    CHAR buf[5];
    char msg[128];
    int rc;
    if (ttnet != NET_TCPB) return(-1);  /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(-1); /* Must be telnet protocol */
    if (!TELCMD_OK(cmd)) return(-1);
    if (TELOPT_OK(opt)) {
        if (cmd == DO && TELOPT_UNANSWERED_DO(opt)) return(0);
        if (cmd == WILL && TELOPT_UNANSWERED_WILL(opt)) return(0);
        if (cmd == DONT && TELOPT_UNANSWERED_DONT(opt)) return(0);
        if (cmd == WONT && TELOPT_UNANSWERED_WONT(opt)) return(0);
    }
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (cmd == DO && opt == TELOPT_AUTHENTICATION)
      buf[0] = 0;

    if (tn_infinite && TELOPT_OK(opt)) { /* See comment above about   */
        int index = TELOPT_INDEX(opt);   /* preventing infinite loops */
        int m = cmd - WILL;

        if (tncnts[index][m] > MAXTNCNT) {
#ifdef DEBUG
            if (tn_deb || debses || deblog) {
                ckmakmsg(msg,sizeof(msg),
                           "TELNET negotiation loop ",
                           TELCMD(cmd), " ",
                           TELOPT(opt));
                debug(F101,msg,"",opt);
                if (tn_deb || debses) tn_debug(msg);
            }
#endif /* DEBUG */
            return(0);
        }
        tncnts[index][m]++;
        tncnts[index][tnopps[m]] = 0;
    }
    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) (cmd & 0xff);
    buf[2] = (CHAR) (opt & 0xff);
    buf[3] = (CHAR) 0;
#ifdef DEBUG
    if ((tn_deb || debses || deblog) && cmd != SB)
        ckmakmsg(msg,sizeof(msg),"TELNET SENT ",TELCMD(cmd)," ",
                  TELOPT(opt));
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    debug(F101,msg,"",opt);
    if ((tn_deb || debses) && cmd != SB)
      tn_debug(msg);
    rc = (ttol(buf,3) < 3);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);


    if (TELOPT_OK(opt)) {
        if (cmd == DONT && TELOPT_UNANSWERED_DO(opt))
          TELOPT_UNANSWERED_DO(opt) = 0;
        if (cmd == WONT && TELOPT_UNANSWERED_WILL(opt))
          TELOPT_UNANSWERED_WILL(opt) = 0;
        if (cmd == DO && TELOPT_UNANSWERED_DONT(opt))
          TELOPT_UNANSWERED_DONT(opt) = 0;
        if (cmd == WILL && TELOPT_UNANSWERED_WONT(opt))
          TELOPT_UNANSWERED_WONT(opt) = 0;
    }
    return(1);
}

/* Send a telnet sub-option */
/* Returns 1 if command was sent, 0 if not, -1 on error */

int
tn_ssbopt(opt,sub,data,len) int opt, sub; CHAR * data; int len; {
    CHAR buf[256];
    int n,m,rc;

    if (ttnet != NET_TCPB) return(0);   /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */
    if (!TELOPT_OK(opt)) return(-1);
    if (len < 0 || len > 250) {
        debug(F111,"Unable to Send TELNET SB - data too long","len",len);
        return(-1);                     /* Data too long */
    }
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        if (ttchk() < 0)
          return(-1);
        else
          return(1);
    }
#endif /* CK_SSL */

    if (!data) len = 0;

    buf[0] = (CHAR) IAC;
    buf[1] = (CHAR) (SB & 0xff);
    buf[2] = (CHAR) (opt & 0xff);
    buf[3] = (CHAR) (sub & 0xff);
    if (data && len > 0) {
        memcpy(&buf[4],data,len);
    }
    buf[4+len] = (CHAR) IAC;
    buf[5+len] = (CHAR) SE;

#ifdef DEBUG
    if (tn_deb || debses || deblog) {
        if (opt == TELOPT_START_TLS && sub == 1)
          ckmakmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                    TELOPT(opt)," FOLLOWS IAC SE",NULL);
        else if (opt == TELOPT_TTYPE && sub == 1)
          ckmakmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB ", TELOPT(opt),
                    " SEND IAC SE",NULL);
        else if (opt == TELOPT_TTYPE && sub == 0)
          ckmakxmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB ",TELOPT(opt)," IS ",
                    (char *)data," IAC SE",NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        else if (opt == TELOPT_NEWENVIRON) {
            int i, quote;
            ckmakmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                     TELOPT(TELOPT_NEWENVIRON)," ",
                     sub == TELQUAL_SEND ? "SEND" :
                     sub == TELQUAL_IS ? "IS" :
                     sub == TELQUAL_INFO ?"INFO" : "UNKNOWN" );
            for (i = 0, quote = 0; i < len; i++) {
                if (quote) {
                    sprintf(hexbuf,"%02x",data[i]); /* safe but ugly */
                    ckstrncat(tn_msg_out,hexbuf,TN_MSG_LEN);
                    quote = 0;
                } else {
                    switch (data[i]) {
                      case TEL_ENV_USERVAR:
                        ckstrncat(tn_msg_out," USERVAR ",TN_MSG_LEN);
                        break;
                      case TEL_ENV_VAR:
                        ckstrncat(tn_msg_out," VAR ",TN_MSG_LEN);
                        break;
                      case TEL_ENV_VALUE:
                        ckstrncat(tn_msg_out," VALUE ",TN_MSG_LEN);
                        break;
                      case TEL_ENV_ESC:
                        ckstrncat(tn_msg_out," ESC ",TN_MSG_LEN);
                        quote = 1;
                        break;
                      case IAC:
                        ckstrncat(tn_msg_out," IAC ",TN_MSG_LEN);
                        break;
                      default:
                        sprintf(hexbuf,"%c",data[i]); /* safe but ugly */
                        ckstrncat(tn_msg_out,hexbuf,TN_MSG_LEN);
                    }
                }
            }
            ckstrncat(tn_msg_out," IAC SE",TN_MSG_LEN);
        } else {
            sprintf(hexbuf,"%02x",sub);             /* safe but ugly */
            ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(opt),
                      " ",
                      hexbuf,
                      " <data> IAC SE",
                       NULL,NULL,NULL,NULL,NULL,NULL,NULL
                      );
        }
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif /* OS2 */
#ifdef DEBUG
    debug(F101,tn_msg_out,"",opt);
    if (tn_deb || debses)
      tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol(buf,6+len) < 6+len);
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    if (rc)
      return(-1);
    return(1);
}

/*
  tn_flui() -- Processes all waiting data for Telnet commands.
  All non-Telnet data is to be stored into the Telnet Wait Buffer.
  Returns 1 on success.
*/
int
tn_flui() {
    extern int ckxech;
    int x = 0;

    /* if (!IS_TELNET()) return(0); */

    /* Wait up to 5 sec for responses to outstanding telnet negotiations */
    while (x >= 0 && ttchk() > 0  && tn_wait_idx < TN_WAIT_BUF_SZ) {
        x = ttinc(1);
        switch (x) {
          case IAC:
            x = tn_doop((CHAR)(x & 0xff),inserver?ckxech:duplex,ttinc);
            debug(F101,"tn_flui tn_doop","",x);
            switch (x) {
              case 1:                   /* Turn on echoing */
                duplex = 1;
                if (inserver)
                  ckxech = 1;
                break;
              case 2:                   /* Turn off echoing */
                duplex = 0;
                if (inserver)
                  ckxech = 0;
                break;
              case 3:
                tn_wait_buf[tn_wait_idx++] = IAC;
                break;
              case 4:                   /* IKS event */
              case 6:                   /* Logout */
                break;
            }
            break;
          default:
            if (x >= 0)
              tn_wait_buf[tn_wait_idx++] = x;
        }
    }
    return(1);
}

unsigned char *
tn_get_display()
{
    char * disp = NULL;
    static char tmploc[256];

    /* Must compute the DISPLAY string we are going to send to the host */
    /* If one is not assigned, do not send a string unless the user has */
    /* explicitedly requested we try to send one via X-Display Location */
    /* But do not send a string at all if FORWARD_X is in use.          */

    /* if (!IS_TELNET()) return(0); */

    debug(F110,"tn_get_display() myipaddr",myipaddr,0);
#ifdef CK_ENVIRONMENT
    debug(F110,"tn_get_display() tn_env_disp",tn_env_disp,0);
    if (tn_env_disp[0]) {
        int colon = ckindex(":",tn_env_disp,0,0,1);
        if ( !colon ) {
            ckmakmsg(tmploc,256,myipaddr,":",tn_env_disp,NULL);
            disp = tmploc;
        } else if ( ckindex("localhost:",tn_env_disp,0,0,0) ||
                    ckindex("unix:",tn_env_disp,0,0,0) ||
                    ckindex("127.0.0.1:",tn_env_disp,0,0,0) ||
                    !ckstrcmp("0:",tn_env_disp,2,1) ||
                    tn_env_disp[0] == ':' ) {
            ckmakmsg(tmploc,256,myipaddr,":",&tn_env_disp[colon],NULL);
            disp = tmploc;
        } else
            disp = tn_env_disp;
    }
    else
#endif /* CK_ENVIRONMENT */
        if (TELOPT_ME(TELOPT_XDISPLOC) ||
              TELOPT_U(TELOPT_FORWARD_X)) {
        ckmakmsg(tmploc,256,myipaddr,":0.0",NULL,NULL);
        disp = tmploc;
    }
    debug(F110,"tn_get_display() returns",disp,0);
    return((CHAR *)disp);
}

#ifdef CK_FORWARD_X
static Xauth fake_xauth = {0,0,NULL,0,NULL,0,NULL,0,NULL};
static Xauth *real_xauth=NULL;

/*
 * Author:  Jim Fulton, MIT X Consortium
 *
 * fwdx_parse_displayname -
 * display a display string up into its component parts
 */
#ifdef UNIX
#define UNIX_CONNECTION "unix"
#define UNIX_CONNECTION_LENGTH 4
#endif

/*
 * private utility routines
 */

static int
#ifdef CK_ANSIC
XmuGetHostname (char *buf, int maxlen)
#else
XmuGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
#endif /* CK_ANSIC */
{
    int len;

#ifdef NEED_UTSNAME
    /*
     * same host name crock as in server and xinit.
     */
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* hpux */
    return len;
}

static char *
#ifdef CK_ANSIC
copystring (char *src, int len)
#else
copystring (src, len)
    char *src;
    int len;
#endif /* CK_ANSIC */
{
    char *cp;

    if (!src && len != 0) return NULL;
    cp = malloc (len + 1);
    if (cp) {
        if (src) strncpy (cp, src, len);
        cp[len] = '\0';
    }
    return cp;
}

static char *
#ifdef CK_ANSIC
get_local_hostname (char *buf, int maxlen)
#else
get_local_hostname (buf, maxlen)
    char *buf;
    int maxlen;
#endif
{
    buf[0] = '\0';
    (void) XmuGetHostname (buf, maxlen);
    return (buf[0] ? buf : NULL);
}

#ifndef UNIX
static char *
copyhostname ()
{
    char buf[256];

    return (get_local_hostname (buf, sizeof buf) ?
            copystring (buf, strlen (buf)) : NULL);
}
#endif


int
#ifdef CK_ANSIC
fwdx_parse_displayname (char *displayname, int *familyp, char **hostp,
                        int *dpynump, int *scrnump, char **restp)
#else
fwdx_parse_displayname (displayname, familyp, hostp, dpynump, scrnump, restp)
    char *displayname;
    int *familyp;                       /* return */
    char **hostp;                       /* return */
    int *dpynump, *scrnump;             /* return */
    char **restp;                       /* return */
#endif /* CK_ANSIC */
{
    char *ptr;                          /* work variables */
    int len;                            /* work variable */
    int family = -1;                    /* value to be returned */
    char *host = NULL;                  /* must free if set and error return */
    int dpynum = -1;                    /* value to be returned */
    int scrnum = 0;                     /* value to be returned */
    char *rest = NULL;                  /* must free if set and error return */
    int dnet = 0;                       /* if 1 then using DECnet */

                                        /* check the name */
    if (!displayname || !displayname[0])
        return 0;
                                        /* must have at least :number */
    ptr = (char *)strchr(displayname, ':');
    if (!ptr || !ptr[1]) return 0;
    if (ptr[1] == ':') {
        if (ptr[2] == '\0') return 0;
        dnet = 1;
    }

    /*
     * get the host string; if none is given, use the most effiecient path
     */

    len = (ptr - displayname);  /* length of host name */
    if (len == 0) {                     /* choose most efficient path */
#ifdef UNIX
        host = copystring (UNIX_CONNECTION, UNIX_CONNECTION_LENGTH);
        family = FamilyLocal;
#else
        if (dnet) {
            host = copystring ("0", 1);
            family = FamilyDECnet;
        } else {
            host = copyhostname ();
            family = FamilyInternet;
        }
#endif
    } else {
        host = copystring (displayname, len);
        if (dnet) {
            family = dnet;
        } else {
#ifdef UNIX
            if (host && strcmp (host, UNIX_CONNECTION) == 0)
              family = FamilyLocal;
            else
#endif
              family = FamilyInternet;
        }
    }

    if (!host) return 0;


    /*
     * get the display number; we know that there is something after the
     * colon (or colons) from above.  note that host is now set and must
     * be freed if there is an error.
     */

    if (dnet) ptr++;                    /* skip the extra DECnet colon */
    ptr++;                              /* move to start of display num */
    {
        register char *cp;

        for (cp = ptr; *cp && isascii(*cp) && isdigit(*cp); cp++) ;
        len = (cp - ptr);
                                        /* check present and valid follow */
        if (len == 0 || (*cp && *cp != '.')) {
            free (host);
            return 0;
        }

        dpynum = atoi (ptr);            /* it will handle num. as well */
        ptr = cp;
    }

    /*
     * now get screen number if given; ptr may point to nul at this point
     */
    if (ptr[0] == '.') {
        register char *cp;

        ptr++;
        for (cp = ptr; *cp && isascii(*cp) && isdigit(*cp); cp++) ;
        len = (cp - ptr);
        if (len == 0 || (*cp && *cp != '.')) {  /* all prop name */
            free (host);
            return 0;
        }

        scrnum = atoi (ptr);            /* it will handle num. as well */
        ptr = cp;
    }

    /*
     * and finally, get any additional stuff that might be following the
     * the screen number; ptr must point to a period if there is anything
     */

    if (ptr[0] == '.') {
        ptr++;
        len = strlen (ptr);
        if (len > 0) {
            rest = copystring (ptr, len);
            if (!rest) {
                free (host);
                return 1;
            }
        }
    }

    /*
     * and we are done!
     */

    if ( familyp )
        *familyp = family;
    if ( hostp )
        *hostp = host;
    else
        free(host);
    if ( dpynump )
        *dpynump = dpynum;
    if ( scrnump )
        *scrnump = scrnum;
    if ( restp )
        *restp = rest;
    else
        free(rest);
    return 1;
}


int
#ifdef CK_ANSIC
fwdx_tn_sb( unsigned char * sb, int n )
#else
fwdx_tn_sb( sb, n ) unsigned char * sb; int n;
#endif /* CK_ANSIC */
{
    unsigned short hchannel, nchannel;
    unsigned char * p;
    int i;
    int rc = -1;

    /* check to ensure we have negotiated Forward X */
    if ( sstelnet && !TELOPT_ME(TELOPT_FORWARD_X) ||
         !sstelnet && !TELOPT_U(TELOPT_FORWARD_X) ) {
        debug(F100,"fwdx_tn_sb() not negotiated","",0);
        return(0);
    }

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    switch (sb[0]) {
    case FWDX_SCREEN:
        if (sstelnet && n == 4)
            rc = fwdx_create_listen_socket(sb[1]);
        break;
    case FWDX_OPEN:
        if ( !sstelnet && n >= 5 ) {
            p = (unsigned char *) &nchannel;
            i = 1;
            /* IAC quoting has been stripped in tn_sb() */
            p[0] = sb[i++];
            p[1] = sb[i++];
            hchannel = ntohs(nchannel);
            rc = fwdx_open_client_channel(hchannel);
            if ( rc < 0 ) {
                /* Failed; Send CLOSE channel */
                fwdx_send_close(hchannel);
                rc = 0;         /* Do not consider this to be a telnet error */
            }
#ifdef NT
            if ( !TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started ) {
                ckThreadBegin( &fwdx_thread,32655, 0, FALSE, 0 ) ;
                TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started = 1;
            }
#endif /* NT */
        }
        break;
    case FWDX_CLOSE:
        p = (unsigned char *) &nchannel;
        i = 1;
        /* IAC quoting has been stripped in tn_sb() */
        p[0] = sb[i++];
        p[1] = sb[i++];
        hchannel = ntohs(nchannel);
        fwdx_close_channel(hchannel);
        rc = 0; /* no errors when closing */
        break;
    case FWDX_DATA:
        p = (unsigned char *) &nchannel;
        i = 1;
        /* IAC quoting has been stripped in tn_sb() */
        p[0] = sb[i++];
        p[1] = sb[i++];
        hchannel = ntohs(nchannel);
        rc = fwdx_send_xauth_to_xserver(hchannel,(CHAR *)&sb[3],n-5);
        if ( rc >= 0 && n-5-rc > 0) {
            rc = fwdx_write_data_to_channel(hchannel,(char *)&sb[3+rc],n-5-rc);
            if ( rc < 0 ) {
                /* Failed; Send CLOSE channel */
                rc = fwdx_send_close(hchannel);
            }
        }
        break;
    case FWDX_OPTIONS:
        if ( sstelnet ) {
#ifndef FWDX_SERVER
            rc = 0;
#else
            rc = fwdx_server_accept_options((char*)&sb[2],n-3);
#endif
        } else {
            rc = fwdx_client_reply_options((char *)&sb[2],n-3);
            if ( rc >= 0 ) {
                rc = tn_sndfwdx();
            }
        }
        break;
    case FWDX_OPT_DATA:
        switch ( sb[1] ) {
        default:
            rc = 0;             /* we don't recognize, not an error */
        }
        break;

    case FWDX_XOFF:
    case FWDX_XON:
        if ( !sstelnet ) {
            p = (unsigned char *) &nchannel;
            i = 1;
            /* IAC quoting has been stripped in tn_sb() */
            p[0] = sb[i++];
            p[1] = sb[i++];
            hchannel = ntohs(nchannel);
            TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[hchannel].suspend =
                (sb[0] == FWDX_XOFF);
            rc = 0;
        }
        break;
    }
    return(rc < 0 ? -1 : 0);
}

int
#ifdef CK_ANSIC
fwdx_send_xauth_to_xserver(int channel, unsigned char * data, int len)
#else
fwdx_send_xauth_to_xserver(channel, data, len)
    int channel; unsigned char * data; int len;
#endif /* CK_ANSIC */
{
    int name_len, data_len, i;

    for (i = 0; i < MAXFWDX ; i++) {
        if (TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].id == channel)
            break;
    }
    if ( i == MAXFWDX )
        goto auth_err;

    if (!TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth)
        return(0);

    if (len < 12)
        goto auth_err;

    /* Parse the lengths of variable-length fields. */
    if (data[0] == 0x42) {              /* byte order MSB first. */
        /* Xauth packets appear to always have this format */
        if ( data[1] != 0x00 ||
             data[2] != 0x00 ||
             data[3] != 0x0B ||
             data[4] != 0x00 ||
             data[5] != 0x00 )
            goto auth_err;

        name_len = (data[6] << 8) + data[7];
        data_len = (data[8] << 8) + data[9];
    } else if (data[0] == 0x6c) {       /* Byte order LSB first. */
        /* Xauth packets appear to always have this format */
        if ( data[1] != 0x00 ||
             data[2] != 0x0B ||
             data[3] != 0x00 ||
             data[4] != 0x00 ||
             data[5] != 0x00 )
            goto auth_err;

        name_len = data[6] + (data[7] << 8);
        data_len = data[8] + (data[9] << 8);
    } else {
        /* bad byte order byte */
        goto auth_err;
    }

    /* Check if the whole packet is in buffer. */
    if (len < 12 + ((name_len + 3) & ~3) + ((data_len + 3) & ~3))
        goto auth_err;
    /* If the Telnet Server allows a real Xauth message to be sent */
    /* Then let the message be processed by the Xserver.           */
    if (name_len + data_len > 0) {
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth = 0;
       return(0);
    }
    else
    /* If an empty Xauth message was received.  We are going to   */
    /* send our own Xauth message using the real Xauth data.  And */
    /* then send any other data in the buffer.                    */
    {
        int c, err, dpynum, scrnum, family, sb_len;
        char *display, *host = NULL, *rest = NULL;
        unsigned char *sb, *p;

        /* parse the local DISPLAY env var */
        display = getenv("DISPLAY");
        if ( !display )
            display = "127.0.0.1:0.0";

        if (fwdx_parse_displayname(display,
                                   &family, &host, &dpynum, &scrnum, &rest)) {
            char * disp_no = ckitoa(dpynum);    /* should be unsigned */
            if (family == FamilyLocal) {
                /* call with address = "<local host name>" */
                char address[300] = "localhost";
                gethostname(address, sizeof(address) - 1);
                real_xauth = XauGetAuthByAddr(family,
                                              strlen(address),
                                              address,
                                              strlen(disp_no),
                                              disp_no, 0, NULL);
            }
            else if (family == FamilyInternet) {
                /* call with address = 4 bytes numeric ip addr (MSB) */
                struct hostent *hi;
                if (hi = gethostbyname(host))
                    real_xauth = XauGetAuthByAddr(family, 4,
                                                  hi->h_addr, strlen(disp_no),
                                                  disp_no, 0, NULL);
            }
        }
        if (host) free(host);
        if (rest) free(rest);
        if (!real_xauth) {
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth = 0;
            return(0);
        }

        if (!strncmp(real_xauth->name,
                     "MIT-MAGIC-COOKIE-1",
                     real_xauth->name_length)) {
            char msg[64];

            name_len = real_xauth->name_length;
            data_len = 16;

            if ( data[0] == 0x42 ) {
                msg[0] = 0x42; /* MSB order */
                msg[1] = msg[2] = 0;
                msg[3] = 0x0B;
                msg[4] = msg[5] = 0;
                msg[6] = (name_len >> 8);
                msg[7] = (name_len & 0xFF);
                msg[8] = (data_len >> 8);
                msg[9] = (data_len & 0xFF);
            } else {
                msg[0] = 0x6c; /* LSB order */
                msg[1] = 0;
                msg[2] = 0x0B;
                msg[3] = msg[4] = msg[5] = 0;
                msg[6] = (name_len & 0xFF);
                msg[7] = (name_len >> 8);
                msg[8] = (data_len & 0xFF);
                msg[9] = (data_len >> 8);
            }
            msg[10] = msg[11] = 0;
            memcpy(&msg[12],real_xauth->name,18);
            msg[30] = msg[31] = 0;
            memcpy(&msg[32],real_xauth->data,16);

            if (fwdx_write_data_to_channel(channel,(char *)msg,48) < 0) {
  TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth = 0;
                return(-1);
            } else {
  TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth = 0;
                return(12);
            }
        } else {
  TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[i].need_to_send_xauth = 0;
            return(0);        /* we do not know how to handle this type yet */
        }
    }

  auth_err:
        debug(F100,"fwdx_send_xauth_to_xserver error","",0);
    return(-1);
}


#ifdef COMMENT
int
#ifdef CK_ANSIC
fwdx_authorize_channel(int channel, unsigned char * data, int len)
#else
fwdx_authorize_channel(channel, data, len)
    int channel; unsigned char * data; int len;
#endif /* CK_ANSIC */
{
    /* XXX maybe we should have some retry handling if not the whole first
    * authorization packet arrives complete
    */
    if ( !TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[channel].authorized ) {
        int name_len, data_len;

        if (len < 12)
            goto auth_err;

        /* Parse the lengths of variable-length fields. */
        if (data[0] == 0x42) {          /* byte order MSB first. */
            /* Xauth packets appear to always have this format */
            if ( data[1] != 0x00 ||
                 data[2] != 0x00 ||
                 data[3] != 0x0B ||
                 data[4] != 0x00 ||
                 data[5] != 0x00 )
                goto auth_err;

            name_len = (data[6] << 8) + data[7];
            data_len = (data[8] << 8) + data[9];
        } else if (data[0] == 0x6c) {   /* Byte order LSB first. */
            /* Xauth packets appear to always have this format */
            if ( data[1] != 0x00 ||
                 data[2] != 0x0B ||
                 data[3] != 0x00 ||
                 data[4] != 0x00 ||
                 data[5] != 0x00 )
                goto auth_err;

            name_len = data[6] + (data[7] << 8);
            data_len = data[8] + (data[9] << 8);
        } else {
            /* bad byte order byte */
            goto auth_err;
        }
        /* Check if authentication protocol matches. */
        if (name_len != fake_xauth.name_length ||
             memcmp(data + 12, fake_xauth.name, name_len) != 0) {
            /* connection uses different authentication protocol */
            goto auth_err;
        }
        /* Check if authentication data matches our fake data. */
        if (data_len != fake_xauth.data_length ||
             memcmp(data + 12 + ((name_len + 3) & ~3),
                     fake_xauth.data, fake_xauth.data_length) != 0) {
            /* auth data does not match fake data */
            goto auth_err;
        }
        /* substitute the fake data with real data if we have any */
        if (real_xauth && real_xauth->data)
            memcpy(data + 12 + ((name_len + 3) & ~3),
                   real_xauth->data, data_len);

        TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[channel].authorized = 1;
    }
    return(0);
  auth_err:
    return(-1);
}
#endif /* COMMENT */

int
#ifdef CK_ANSIC
fwdx_send_close(int channel)
#else
fwdx_send_close(channel) int channel;
#endif /* CK_ANSIC */
{
    unsigned short nchannel;
    int i,rc;
    CHAR * p;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    nchannel = htons(channel);
    p = (unsigned char *) &nchannel;

    i = 0;
    sb_out[i++] = (CHAR) IAC;               /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                /* Subnegotiation */
    sb_out[i++] = TELOPT_FORWARD_X;         /* Forward X */
    sb_out[i++] = FWDX_CLOSE;               /* Open */
    sb_out[i++] = p[0];                     /* First Byte of Channel */
    if ( p[0] == IAC )
        sb_out[i++] = IAC;
    sb_out[i++] = p[1];                     /* Second Byte of Channel */
    if ( p[1] == IAC )
        sb_out[i++] = IAC;
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg((char *)fwdx_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                  TELOPT(TELOPT_FORWARD_X),
                  " CLOSE CHANNEL=",ckitoa(channel)," IAC SE",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL
                  );
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);
    return(0);
}

int
#ifdef CK_ANSIC
fwdx_send_open(int channel)
#else /* CK_ANSIC */
fwdx_send_open(channel) int channel;
#endif /* CK_ANSIC */
{
    unsigned short nchannel;
    int i, rc;
    CHAR * p;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    nchannel = htons(channel);
    p = (unsigned char *) &nchannel;

    i = 0;
    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_FORWARD_X;           /* Forward X */
    sb_out[i++] = FWDX_OPEN;                  /* Open */
    sb_out[i++] = p[0];                       /* First Byte of Channel */
    if ( p[0] == IAC )
        sb_out[i++] = IAC;
    sb_out[i++] = p[1];                       /* Second Byte of Channel */
    if ( p[1] == IAC )
        sb_out[i++] = IAC;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg((char *)fwdx_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                  TELOPT(TELOPT_FORWARD_X),
                  " OPEN CHANNEL=",ckitoa(channel)," IAC SE",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);        /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);
    return(0);
}

int
#ifdef CK_ANSIC
fwdx_client_reply_options(char *opts, int n)
#else
fwdx_client_reply_options(opts, n) char *opts; int n;
#endif /* CK_ANSIC */
{
    int i,j,rc;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    i = 0;
    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_FORWARD_X;           /* Forward X */
    sb_out[i++] = FWDX_OPTIONS;               /* Options */

    /* Look for the options we recognize and will support for this session */
    /* and reply with their bytes set                                      */
    for (j=0; j<n; j++,i++) {
        sb_out[i] = FWDX_OPT_NONE;          /* Add zero byte - no options */
#ifdef COMMENT
        /* If we had any options to support, this is how we would do it */
        if ( j == 0 ) {
            if (opts[j] & FWDX_OPT_XXXX) {
                /* set flag to remember option is in use */
                flag = 1;
                sb_out[i] |= FWDX_OPT_XXXX;
            }
        }
#endif /* COMMENT */
    }
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg((char *)fwdx_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                  TELOPT(TELOPT_FORWARD_X),
                  " OPTIONS ",ckctox(sb_out[4],1)," IAC SE",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);        /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);
    return(0);
}


int
fwdx_send_options() {
    int i, rc;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    i = 0;
    sb_out[i++] = (CHAR) IAC;               /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                /* Subnegotiation */
    sb_out[i++] = TELOPT_FORWARD_X;         /* Forward X */
    sb_out[i++] = FWDX_OPTIONS;             /* Options */
    sb_out[i]   = FWDX_OPT_NONE;
    /* activate options here */
    i++;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakmsg((char *)fwdx_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                 TELOPT(TELOPT_FORWARD_X),
                 " OPTIONS 00 IAC SE",NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);        /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);
    return(0);
}

int
#ifdef CK_ANSIC
fwdx_send_data_from_channel(int channel, char * data, int len)
#else
fwdx_send_data_from_channel(channel, data, len)
    int channel; char * data; int len;
#endif
{
    unsigned short nchannel;
    /* static */ CHAR sb_priv[2048];
    CHAR * p;
    int i, j, j_sav, rc;
    unsigned int tmp;

    debug(F111,"fwdx_send_data_from_channel()","channel",channel);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    nchannel = htons(channel);
    p = (unsigned char *) &nchannel;

    j = 0;
    sb_priv[j++] = (CHAR) IAC;                 /* I Am a Command */
    sb_priv[j++] = (CHAR) SB;                  /* Subnegotiation */
    sb_priv[j++] = TELOPT_FORWARD_X;           /* Forward X */
    sb_priv[j++] = FWDX_DATA;                  /* Data */
    sb_priv[j++] = p[0];                       /* First Byte of Channel */
    if ( p[0] == IAC )
        sb_priv[j++] = IAC;
    sb_priv[j++] = p[1];                       /* Second Byte of Channel */
    if ( p[1] == IAC )
        sb_priv[j++] = IAC;
    j_sav = j;

    for (i = 0; i < len; i++) {
        tmp = (unsigned int)data[i];
        if ( tmp == IAC ) {
            sb_priv[j++] = IAC;
            sb_priv[j++] = IAC;
        } else {
            sb_priv[j++] = tmp;
        }
        if ( j >= 2045 && (i < len-1) ) {
            sb_priv[j++] = (CHAR) IAC;  /* End of Subnegotiation */
            sb_priv[j++] = (CHAR) SE;   /* marked by IAC SE */

#ifdef DEBUG
            if (deblog || tn_deb || debses) {
                ckmakxmsg( (char *)fwdx_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                           TELOPT(TELOPT_FORWARD_X),
                           " DATA CHANNEL=",ckitoa(channel)," ",
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL );
                tn_hex((CHAR *)fwdx_msg_out,
		       TN_MSG_LEN,&sb_priv[j_sav],j-(j_sav+2));
                ckstrncat((char *)fwdx_msg_out," IAC SE",TN_MSG_LEN);
            }
#endif /* DEBUG */
#ifdef OS2
            RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
            debug(F100,(char *)fwdx_msg_out,"",0);
            if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
            rc = (ttol(sb_priv,j) < 0);                /* Send it. */
#ifdef OS2
            ReleaseTelnetMutex();
#endif
            if (rc) {
                debug(F110,"fwdx_send_data_from_channel()","ttol() failed",0);
                return(-1);
            }

            j = 0;
            sb_priv[j++] = (CHAR) IAC;                 /* I Am a Command */
            sb_priv[j++] = (CHAR) SB;                  /* Subnegotiation */
            sb_priv[j++] = TELOPT_FORWARD_X;           /* Forward X */
            sb_priv[j++] = FWDX_DATA;                  /* Data */
            sb_priv[j++] = p[0];                       /* First Byte of Channel */
            if ( p[0] == IAC )
                sb_priv[j++] = IAC;
            sb_priv[j++] = p[1];                       /* Second Byte of Channel */
            if ( p[1] == IAC )
                sb_priv[j++] = IAC;
        }
    }

    sb_priv[j++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_priv[j++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg( (char *)fwdx_msg_out,TN_MSG_LEN,
                   "TELNET SENT SB ",TELOPT(TELOPT_FORWARD_X),
                   " DATA ",ckctox(p[0],1)," ",ckctox(p[1],1)," ",
                   NULL,NULL,NULL,NULL,NULL);
        tn_hex((CHAR *)fwdx_msg_out,TN_MSG_LEN,&sb_priv[6],j-8);
        ckstrncat((char *)fwdx_msg_out," IAC SE",TN_MSG_LEN);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = (ttol(sb_priv,j) < 0);                /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if ( rc ) {
        debug(F110,"fwdx_send_data_from_channel()","ttol() failed",0);
        return(-1);
    }


    return(0);
}

static unsigned char *
#ifdef CK_ANSIC
fwdx_add_quoted_twobyte(unsigned char *p, unsigned short twobyte)
#else
fwdx_add_quoted_twobyte(p, twobyte)
    unsigned char *p; unsigned short twobyte;
#endif /* CK_ANSIC */
/* adds the IAC quoted (MSB) representation of 'channel' at buffer pointer 'p',
 * returning pointer to new buffer position. NO OVERFLOW CHECK!
 */
{
    *p++ = (unsigned char)((twobyte >> 8) & 0xFF);
    if (*(p - 1) == 0xFF)
        *p++ = 0xFF;
    *p++ = (unsigned char)(twobyte & 0xFF);
    if (*(p - 1) == 0xFF)
        *p++ = 0xFF;
    return p;
}

int
#ifdef CK_ANSIC
fwdx_create_fake_xauth(char *name, int name_len, int data_len)
#else
fwdx_create_fake_xauth(name, name_len, data_len)
    char *name; int name_len; int data_len;
#endif /* CK_ANSIC */
{
    char stackdata[256];
    unsigned int c, n;

    if (!name_len || !data_len)
        return 1;
    fake_xauth.name = malloc(name_len);
    fake_xauth.data = malloc(data_len);
    if (!fake_xauth.name || !fake_xauth.data)
        return 2;
    fake_xauth.name_length = name_len;
    memcpy(fake_xauth.name, name, name_len);
    fake_xauth.data_length = data_len;

    /* try to make a random unsigned int to feed srand() */
    c = time(NULL);
    c *= getpid();
    for (n = 0; n < sizeof(stackdata); n++)
        c += stackdata[n];
    srand((unsigned int)c);
    for (c = 0; c < data_len; c++)
        fake_xauth.data[c] = (unsigned char)rand();
    return 0;
}

#ifdef COMMENT
/* No longer used */
int
fwdx_send_xauth(void)
{
    int c, err, dpynum, family, sb_len, rc;
    char *display, *host = NULL;
    unsigned char *sb_priv, *p;

    /* parse the local DISPLAY env var */
    if (!(display = tn_get_display()))
        return (-1);
    if (fwdx_parse_displayname(display, &family, &host, &dpynum, NULL, NULL)) {
        char * disp_no = ckitoa(dpynum);
        if (family == FamilyLocal) {
            /* call with address = "<local host name>" */
            char address[300] = "localhost";
            gethostname(address, sizeof(address) - 1);
            real_xauth = XauGetAuthByAddr(family,
                                          strlen(address),
                                          address,
                                          strlen(disp_no),
                                          disp_no, 0, NULL
                                          );
        }
        else if (family == FamilyInternet) {
            /* call with address = 4 bytes numeric ip addr (MSB) */
            struct hostent *hi;
            if (hi = gethostbyname(host))
                real_xauth = XauGetAuthByAddr(family, 4,
                                              hi->h_addr,
                                              strlen(disp_no),
                                              disp_no, 0, NULL
                                              );
        }
    }
    if (host) {
        free(host);
        host = NULL;
    }
    if (real_xauth)
        err = fwdx_create_fake_xauth(real_xauth->name,
                                     real_xauth->name_length,
                                     real_xauth->data_length
                                     );
    else
      err = fwdx_create_fake_xauth("MIT-MAGIC-COOKIE-1",
                                   strlen("MIT-MAGIC-COOKIE-1"), 16);
    if (err)
        return(-1);

    /* allocate memory for the SB block, alloc for worst case              */
    /* the following sprintf() calls are safe due to length checking       */
    /* buffer is twice as big as the input just in case every byte was IAC */
    sb_len = 5 + 2 + 2 + fake_xauth.name_length + fake_xauth.data_length + 2;
    if (!(sb_priv = malloc(2 * sb_len)))
        return(-1);
    p = sb_priv;
    sprintf(p, "%c%c%c%c%c", IAC, SB, TELOPT_FORWARD_X,
            FWDX_OPT_DATA, FWDX_OPT_XAUTH);
    p += 5;
    p = fwdx_add_quoted_twobyte(p, fake_xauth.name_length);
    p = fwdx_add_quoted_twobyte(p, fake_xauth.data_length);
    for (c = 0; c < fake_xauth.name_length; c++) {
        *p++ = fake_xauth.name[c];
        if ((unsigned char)fake_xauth.name[c] == 0xFF)
            *p++ = 0xFF;
    }
    for (c = 0; c < fake_xauth.data_length; c++) {
        *p++ = fake_xauth.data[c];
        if ((unsigned char)fake_xauth.data[c] == 0xFF)
            *p++ = 0xFF;
    }
    sprintf(p, "%c%c", IAC, SE);
    p += 2;

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sprintf((char *)fwdx_msg_out,"TELNET SENT SB %s OPTION_DATA XAUTH ",
                 TELOPT(TELOPT_FORWARD_X));
        tn_hex((char *)fwdx_msg_out,TN_MSG_LEN,&sb_priv[5],(p-sb_priv)-7);
        ckstrncat((char *)fwdx_msg_out," IAC SE",TN_MSG_LEN);
    }
#endif /* DEBUG */

    /* Add Telnet Debug info here */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,(char *)fwdx_msg_out,"",0);
    if (tn_deb || debses) tn_debug((char *)fwdx_msg_out);
#endif /* DEBUG */
    rc = ( ttol(sb_priv,p-sb_priv) < 0 );                /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc) {
        debug(F110,"fwdx_send_xauth()","ttol() failed",0);
        return(-1);
    }


    free(sb_priv);
    return(0);
}
#endif /* COMMENT */
#ifdef FWDX_SERVER
/* Only if we ever become a server - not yet ported to Kermit   */
/* And even so most of this code does not belong in this module */

int
fwdx_write_xauthfile(void)
{
    int dpynum, scrnum, family;
    char myhost[300], *host, *rest = NULL;
    FILE *file;
    struct sockaddr_in saddr;
    struct hostent *hi;

    if (!fwdx_display && !fwdx_xauthfile)
        return 1;
    if (!parse_displayname(fwdx_display,
                           &family, &host, &dpynum, &scrnum, &rest))
        return 2;
    if (rest) free(rest);
    if (host) free(host);
    if (family != FamilyInternet)
        return 3; /* every thing but FamilyInternet is unexpected */

    /* X connections to localhost:1 is actually treated as local unix sockets,
     * see the 'xauth' man page.
     */
    xauth.family = FamilyLocal;
    if (gethostname(myhost, sizeof(myhost) - 1))
        return 5;
    xauth.address_length = strlen(myhost);
    if (!(xauth.address = malloc(xauth.address_length)))
        return 5;
    memcpy(xauth.address, myhost, xauth.address_length);

    /* the display number is written as a string, not numeric */
    if (!(xauth.number = malloc(6)))
        return 6;
    snprintf(xauth.number, 5, "%u", dpynum);
    xauth.number_length = strlen(xauth.number);
    if (!(file = fopen(fwdx_xauthfile, "wb")))
        return 7;
    if (!XauWriteAuth(file, &xauth))
        return 8;
    fclose(file);
    setenv("XAUTHORITY", fwdx_xauthfile, 1);
    return 0;
}

int
fwdx_setup_xauth(unsigned char *sp, int len)
/* called with 'len' xauth bytes, starting at 'sp'
 * the data format is: <uint16 name_length> <uint16 data_length> <name> <data>
 */
{
    int xauthfd;

    if (!fwdx_options[FWDX_OPT_XAUTH])
        return 1;
    if (len < 4)
        return 2;

    /* setup the xauth struct */
    xauth.name_length = (sp[0] << 8) + sp[1];
    xauth.data_length = (sp[2] << 8) + sp[3];
    if (len != 4 + xauth.name_length + xauth.data_length)
        return 3;
    xauth.name = malloc(xauth.name_length);
    xauth.data = malloc(xauth.data_length);
    if (!xauth.name || !xauth.data)
        return 4;
    memcpy(xauth.name, sp + 4, xauth.name_length);
    memcpy(xauth.data, sp + 4 + xauth.name_length, xauth.data_length);

    /* Setup to always have a local .Xauthority. */
    fwdx_xauthfile = malloc(MAXPATHLEN+1);
    snprintf(fwdx_xauthfile, MAXPATHLEN, "/tmp/XauthXXXXXX");
    if ((xauthfd = mkstemp(fwdx_xauthfile)) != -1)
        /* we change file ownership later, when we know who is to be owner! */
        close(xauthfd);
    else {
        free(fwdx_xauthfile);
        fwdx_xauthfile = NULL;
        return 5;
    }
/* Must have the subshell's new DISPLAY env var to write xauth to xauthfile */
    if (fwdx_display)
        if (fwdx_write_xauthfile())
            return 6;

    return 0;
}

void fwdx_set_xauthfile_owner(int uid)
{
    struct passwd *pwd;

    if (!fwdx_xauthfile || !(pwd = getpwuid(uid)))
        return;
    chown(fwdx_xauthfile, pwd->pw_uid, pwd->pw_gid);
}

int
fwdx_server_accept_options(unsigned char *sp, int len)
/* called with 'len' option bytes, starting at 'sp' */
{
    int c;

    for (c = 0; c < len-2; c++) {
        if (c == 0) {
            if (sp[c] & FWDX_OPT_XAUTH)
                flag = 1;
        }
    }
    return(0);
}
#endif /* FWDX_SERVER */
#endif /* CK_FORWARD_X */

#ifdef IKS_OPTION
/*
  iks_wait() -- Wait for an IKS subnegotiation response.
  sb - is either KERMIT_REQ_START or KERMIT_REQ_STOP depending on the desired
       state of the peer's Kermit server.
  flushok - specifies whether it is ok to throw away non-Telnet data
       if so, then we call ttflui() instead of tn_flui().
  Returns:
   1 if the desired state is achieved or if it is unknown.
   0 if the desired state is not achieved.
*/
int
#ifdef CK_ANSIC
iks_wait(int sb, int flushok)
#else /* CK_ANSIC */
iks_wait(sb,flushok) int sb; int flushok;
#endif /* CK_ANSIC */
{
    int tn_wait_save = tn_wait_flg;
    int x;

    if (TELOPT_U(TELOPT_KERMIT)) {
        switch (sb) {
          case KERMIT_REQ_START:
            debug(F111,
                  "iks_wait KERMIT_REQ_START",
                  "u_start",
                  TELOPT_SB(TELOPT_KERMIT).kermit.u_start
                  );
            tn_siks(KERMIT_REQ_START);
            tn_wait_flg = 1;            /* Kermit Option MUST wait */
            do {
                if (flushok)
                  tn_wait_idx = 0;
                x = tn_wait("iks_wait() me_iks_req_start");
            } while (x == 0 && flushok && tn_wait_idx == TN_WAIT_BUF_SZ);
            tn_wait_flg = tn_wait_save;
            if (flushok)
              tn_wait_idx = 0;
            if (tn_wait_idx == TN_WAIT_BUF_SZ) {
                /*
                 * We are attempting to start a kermit server on the peer
                 * the most likely reason is because we want to perform a
                 * file transfer.  But there is a huge amount of non telnet
                 * negotiation data coming in and so we have not been able
                 * to find the response.  So we will lie and assume that
                 * response is 'yes'.  The worse that will happen is that
                 * a RESP_STOP is received after we enter protocol mode.
                 * And the protocol operation will be canceled.
                 */
                tn_push();
                return(1);
            } else {
                tn_push();
                return(TELOPT_SB(TELOPT_KERMIT).kermit.u_start);
            }
          case KERMIT_REQ_STOP:
            debug(F111,
                  "iks_wait KERMIT_REQ_STOP",
                  "u_start",
                  TELOPT_SB(TELOPT_KERMIT).kermit.u_start
                  );
            tn_siks(KERMIT_REQ_STOP);
            tn_wait_flg = 1;            /* Kermit Option MUST wait */
            do {
                if (flushok)
                  tn_wait_idx = 0;
                x = tn_wait("iks_wait() me_iks_req_stop");
            } while (x == 0 && flushok && tn_wait_idx == TN_WAIT_BUF_SZ);
            tn_wait_flg = tn_wait_save;
            if (flushok)
              tn_wait_idx = 0;

            if (tn_wait_idx == TN_WAIT_BUF_SZ) {
                /*
                 * We are attempting to stop a kermit server on the peer
                 * the most likely reason being that we want to enter
                 * CONNECT mode.  But there is a huge amount of non telnet
                 * negotiation data coming in and so we have not been able
                 * to find the response.  So we will lie and assume that
                 * the answer is 'yes' and allow the CONNECT command to
                 * succeed.  The worst that happens is that CONNECT mode
                 * swallows the incoming data displaying it to the user
                 * and then it resumes Kermit client mode.
                 */
                tn_push();
                return(1);
            } else {
                tn_push();
                return(!TELOPT_SB(TELOPT_KERMIT).kermit.u_start);
            }
        }
        tn_push();
    }
    return(1);
}

int
#ifdef CK_ANSIC
iks_tn_sb(CHAR * sb, int n)
#else
iks_tn_sb(sb, n) CHAR * sb; int n;
#endif /* CK_ANSIC */
{
    extern int server;
    extern CHAR sstate;
#ifdef NOICP
    extern int autodl;
    int inautodl = 0, cmdadl = 1;
#else
#ifdef CK_AUTODL
    extern int autodl, inautodl, cmdadl;
#endif /* CK_AUTODL */
#endif /* NOICP */
    switch (sb[0]) {
      case KERMIT_START:                /* START */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 1;
        return(4);

      case KERMIT_STOP:                 /* STOP */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
        return(4);

      case KERMIT_REQ_START:            /* REQ-START */
#ifndef NOXFER
        if (inserver) {
#ifdef CK_AUTODL
            cmdadl = 1;                 /* Turn on packet detection */
#endif /* CK_AUTODL */
            TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 1;
            tn_siks(KERMIT_RESP_START);
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_start) {
            tn_siks(KERMIT_RESP_START);
        } else {
#ifndef IKSDONLY
#ifdef CK_AUTODL
#ifdef OS2
            if (local && (IsConnectMode() && autodl) ||
                (!IsConnectMode() && 
                  (inautodl || sstate == 'x' || sstate == 'v'))
                )
              tn_siks(KERMIT_RESP_START); /* START */
            else

#else /* OS2 */
            if ((local && what == W_CONNECT && autodl) ||
                (local && what != W_CONNECT &&
                  (inautodl || sstate == 'x' || sstate == 'v')
                ))
              tn_siks(KERMIT_RESP_START); /* START */
            else
#endif /* OS2 */
#endif /* CK_AUTODL */
#endif /* IKSDONLY */
              tn_siks(KERMIT_RESP_STOP);
        }
#else /* NOXFER */
          tn_siks(KERMIT_RESP_STOP);
#endif /* NOXFER */
        return(4);

      case KERMIT_REQ_STOP:             /* REQ-STOP */
        /* The protocol requires that the request be responded to */
        /* either by changing states or by reporting the current  */
        /* state.  */

        /* We need to provide the user some way of dictating what */
        /* the policies should be.  For instance, if we are in    */
        /* CONNECT mode with autodownload ON and we get a REQ-STOP*/
        /* what should the proper response be?                    */
#ifndef NOXFER
        if (inserver
#ifdef CK_AUTODL
            || !local && cmdadl
#endif /* CK_AUTODL */
            ) {
#ifdef CK_AUTODL
            cmdadl = 0;                 /* Turn off packet detection */
#endif /* CK_AUTODL */
            tn_siks(KERMIT_RESP_STOP);
        } else if (server) {
            extern int en_fin;
            if (en_fin) {               /* If the server is allowed to stop */
                tn_siks(KERMIT_RESP_STOP);
            } else {                    /* We are not allowed to stop */
                tn_siks(KERMIT_RESP_START);
            }
        }
#ifndef IKSDONLY
#ifdef CK_AUTODL
#ifdef OS2
        else if (local && (IsConnectMode() && autodl) ||
                   (!IsConnectMode() && inautodl)
                   ) {
            /* If we are a pseudo-server and the other side requests */
            /* that we stop, tell then that we have even though we   */
            /* have not.  Otherwise, the other side might refuse to  */
            /* enter SERVER mode.                                    */

            tn_siks(KERMIT_RESP_STOP);  /* STOP */
        }
#else /* OS2 */
        else if ((local && what == W_CONNECT && autodl) ||
                   (local && what != W_CONNECT && inautodl)
                   ) {
            /* If we are a pseudo-server and the other side requests */
            /* that we stop, tell then that we have even though we   */
            /* have not.  Otherwise, the other side might refuse to  */
            /* enter SERVER mode.                                    */

            tn_siks(KERMIT_RESP_STOP);  /* STOP */
        }
#endif /* OS2 */
#endif /* CK_AUTODL */
#endif /* IKSDONLY */
        else
#endif /* NOXFER */
        {
            /* If we are not currently in any mode that accepts */
            /* Kermit packets then of course report that we are */
            /* not being a Kermit server.                       */

            tn_siks(KERMIT_RESP_STOP);  /* STOP */
        }
        return(4);

      case KERMIT_SOP: {                /* SOP */
#ifndef NOXFER
          extern CHAR stchr;            /* Incoming SOP character */
          stchr = sb[1];
#endif /* NOXFER */
          TELOPT_SB(TELOPT_KERMIT).kermit.sop = 1;
          return(4);
      }

      case KERMIT_RESP_START:           /* START */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 1;
        if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop) {
            /* If we have issued a request to stop a Kermit Server */
            /* and the response is Start, then we must report this */
            /* to the caller.                                      */
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
        }
        return(4);

      case KERMIT_RESP_STOP:            /* STOP */
        TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
        if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
            /* If we have issued a request to start a Kermit Server */
            /* and the response is Stop, then we must report this   */
            /* to the caller.                                       */
        } else if (TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop) {
            TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
        }
        return(4);

      default:
        return(0);

    } /* switch (sb[0]) */
}
#endif /* IKS_OPTION */

/* Initialize telnet settings - set default values for ME and U modes */
int
tn_set_modes() {
    int opt,cmd;
#ifdef CK_FORWARD_X
    int x;
#endif /* CK_FORWARD_X */
#ifdef CK_ENVIRONMENT
    {
        int i,j;
        for (i = 0; i < 8; i++) {
            tn_env_uservar[i][0] = NULL;
            tn_env_uservar[i][1] = NULL;
        }
    }
#endif /* CK_ENVIRONMENT */

    /* initialize all options to refuse in both directions */
    for (opt = 0; opt < NTELOPTS; opt++) {
        TELOPT_ME(opt) = 0;
        TELOPT_U(opt)  = 0;
        TELOPT_UNANSWERED_WILL(opt) = 0;
        TELOPT_UNANSWERED_DO(opt)   = 0;
        TELOPT_UNANSWERED_WONT(opt) = 0;
        TELOPT_UNANSWERED_DONT(opt)   = 0;
        TELOPT_UNANSWERED_SB(opt)   = 0;
        TELOPT_ME_MODE(opt) = TN_NG_RF;
        TELOPT_U_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_S_ME_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_S_U_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_C_ME_MODE(opt) = TN_NG_RF;
        TELOPT_DEF_C_U_MODE(opt) = TN_NG_RF;
        for (cmd = 0; cmd < 4; cmd ++)
          tncnts[TELOPT_INDEX(opt)][cmd] = 0;
    }
#ifdef IKS_OPTION
    TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.sop = 0;
#endif /* IKS_OPTION */

#ifdef CK_ENCRYPTION
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
#endif /* CK_ENCRYPTION */

#ifdef  CK_NAWS
    TELOPT_SB(TELOPT_NAWS).naws.x = 0;
    TELOPT_SB(TELOPT_NAWS).naws.y = 0;
#endif /* CK_NAWS */

#ifdef CK_SSL
    TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.auth_request = 0;
#endif /* CK_SSL */

    /* Now set the ones we want to accept to the proper values */
    TELOPT_DEF_S_ME_MODE(TELOPT_SGA) = TN_NG_RQ;
    TELOPT_DEF_S_U_MODE(TELOPT_SGA) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_SGA) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_SGA) = TN_NG_AC;

    TELOPT_DEF_S_ME_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_S_U_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_BINARY) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_BINARY) = TN_NG_AC;

    TELOPT_DEF_S_ME_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_S_U_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_LOGOUT) = TN_NG_AC;
    TELOPT_DEF_C_U_MODE(TELOPT_LOGOUT) = TN_NG_AC;

#ifdef IKS_OPTION
    TELOPT_DEF_S_ME_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_S_U_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_KERMIT) = TN_NG_RQ;
    TELOPT_DEF_C_U_MODE(TELOPT_KERMIT) = TN_NG_RQ;
#endif /* IKS_OPTION */

#ifdef CK_ENCRYPTION
    TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
    TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RQ;
#endif /* CK_ENCRYPTION */

    TELOPT_DEF_S_ME_MODE(TELOPT_ECHO) = TN_NG_RQ;
#ifdef IKSD
    if ( !inserver )
#endif /* IKSD */
      TELOPT_DEF_S_U_MODE(TELOPT_TTYPE) = TN_NG_RQ;

#ifdef CK_ENVIRONMENT
    TELOPT_DEF_S_U_MODE(TELOPT_NEWENVIRON) = TN_NG_RQ;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
    TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
#endif /* CK_AUTHENTICATION */

#ifdef CK_SSL
    if (ck_ssleay_is_installed()) {
        TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RQ;
        TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = TN_NG_AC;
    }
#endif /* CK_SSL */

#ifdef CK_NAWS
    TELOPT_DEF_S_U_MODE(TELOPT_NAWS) = TN_NG_RQ;
#endif /* CK_NAWS */

    TELOPT_DEF_C_U_MODE(TELOPT_ECHO) = TN_NG_AC;
    TELOPT_DEF_C_ME_MODE(TELOPT_TTYPE) = TN_NG_RQ;

#ifdef CK_ENVIRONMENT
    TELOPT_DEF_C_ME_MODE(TELOPT_NEWENVIRON) = TN_NG_RQ;
#endif /* CK_ENVIRONMENT */

#ifdef CK_AUTHENTICATION
    TELOPT_DEF_C_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RQ;
#endif /* CK_AUTHENTICATION */

#ifdef CK_NAWS
    TELOPT_DEF_C_ME_MODE(TELOPT_NAWS) = TN_NG_RQ;
#endif /* CK_NAWS */

#ifdef CK_SNDLOC
    TELOPT_DEF_C_ME_MODE(TELOPT_SNDLOC) = TN_NG_RQ;
#endif /* CK_SNDLOC */

#ifdef CK_FORWARD_X
    TELOPT_DEF_C_U_MODE(TELOPT_FORWARD_X) = TN_NG_AC;
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.listen_socket = -1;
    for (x = 0; x < MAXFWDX; x++) {
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].fd = -1;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].id = -1;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].need_to_send_xauth = 0;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].suspend = 0;
    }
#endif /* CK_FORWARD_X */

#ifdef TN_COMPORT
    TELOPT_DEF_C_ME_MODE(TELOPT_COMPORT) = TN_NG_RQ;
#endif /* TN_COMPORT */

    /* Set the initial values for currently known mode */
    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        if (TELOPT_OK(opt)) {
            TELOPT_ME_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_ME_MODE(opt) :
                TELOPT_DEF_C_ME_MODE(opt);
            TELOPT_U_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_U_MODE(opt) :
                TELOPT_DEF_C_U_MODE(opt);
        }
    }
    return(1);
}


/* Send Delayed Subnegotiations */

VOID
tn_sdsb() {
    /* if (!IS_TELNET()) return; */

    if (TELOPT_SB(TELOPT_TTYPE).term.need_to_send) {
        tn_sttyp();
        TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 0;
    }
#ifdef CK_ENVIRONMENT
    if (TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send &&
        TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
        tn_snenv((CHAR *)TELOPT_SB(TELOPT_NEWENVIRON).env.str,
                 TELOPT_SB(TELOPT_NEWENVIRON).env.len);
        free(TELOPT_SB(TELOPT_NEWENVIRON).env.str);
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
        TELOPT_SB(TELOPT_NEWENVIRON).env.len=0;
        TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 0;
    }
#ifdef CK_XDISPLOC
    if (TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send) {
        tn_sxdisploc();
        TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 0;
    }
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
#ifdef CK_NAWS
    if (TELOPT_SB(TELOPT_NAWS).naws.need_to_send) {
        tn_snaws();
        TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 0;
    }
#endif /* CK_NAWS */
#ifdef CK_SNDLOC
    if (TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send) {
        tn_sndloc();
        TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 0;
    }
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    if (TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send) {
        if ( sstelnet )
            fwdx_send_options();
        TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 0;
    }
#endif /* CK_FORWARD_X */
#ifdef TN_COMPORT
    if (TELOPT_SB(TELOPT_COMPORT).comport.need_to_send) {
        tn_sndcomport();
        TELOPT_SB(TELOPT_COMPORT).comport.need_to_send = 0;
    }
#endif /* TN_COMPORT */

}

int
tn_reset() {
    int x,opt,cmd;

    /* if (!IS_TELNET()) return(1); */

    tn_wait_idx = 0;                    /* Clear the tn_push() buffer */
    tn_wait_tmo = TN_TIMEOUT;           /* Reset wait timer stats */

    nflag = 0;

    /* Reset the TELNET OPTIONS counts */
    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        if (TELOPT_OK(opt)) {
            TELOPT_ME(opt) = 0;
            TELOPT_U(opt)  = 0;
            TELOPT_UNANSWERED_WILL(opt) = 0;
            TELOPT_UNANSWERED_DO(opt)   = 0;
            TELOPT_UNANSWERED_WONT(opt) = 0;
            TELOPT_UNANSWERED_DONT(opt)   = 0;
            TELOPT_UNANSWERED_SB(opt)   = 0;
            TELOPT_ME_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_ME_MODE(opt) :
                TELOPT_DEF_C_ME_MODE(opt);
            TELOPT_U_MODE(opt) = sstelnet ?
              TELOPT_DEF_S_U_MODE(opt) :
                TELOPT_DEF_C_U_MODE(opt);

#ifdef DEBUG
            if (deblog) {
                switch (TELOPT_ME_MODE(opt)) {
                  case TN_NG_RF:
                    debug(F110,"tn_ini ME REFUSE ",TELOPT(opt),0);
                    break;
                  case TN_NG_AC:
                    debug(F110,"tn_ini ME ACCEPT ",TELOPT(opt),0);
                    break;
                  case TN_NG_RQ:
                    debug(F110,"tn_ini ME REQUEST",TELOPT(opt),0);
                    break;
                  case TN_NG_MU:
                    debug(F110,"tn_ini ME REQUIRE",TELOPT(opt),0);
                    break;
                }
                switch (TELOPT_U_MODE(opt)) {
                  case TN_NG_RF:
                    debug(F110,"tn_ini U  REFUSE ",TELOPT(opt),0);
                    break;
                  case TN_NG_AC:
                    debug(F110,"tn_ini U  ACCEPT ",TELOPT(opt),0);
                    break;
                  case TN_NG_RQ:
                    debug(F110,"tn_ini U  REQUEST",TELOPT(opt),0);
                    break;
                  case TN_NG_MU:
                    debug(F110,"tn_ini U  REQUIRE",TELOPT(opt),0);
                    break;
                }
            }
#endif /* DEBUG */
            for (cmd = 0; cmd < 4; cmd ++)
              tncnts[TELOPT_INDEX(opt)][cmd] = 0;
        }
    }
#ifdef CK_ENVIRONMENT
    if (!tn_env_flg) {
        TELOPT_ME_MODE(TELOPT_NEWENVIRON) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_NEWENVIRON) = TN_NG_RF;
    }
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    if (!tn_loc)
        TELOPT_DEF_C_ME_MODE(TELOPT_SNDLOC) = TN_NG_RF;
#endif /* CK_SNDLOC */
#ifdef IKS_OPTION
    TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.u_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 0;
    TELOPT_SB(TELOPT_KERMIT).kermit.sop = 0;
#endif /* IKS_OPTION */
#ifdef CK_ENCRYPTION
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 0;
#endif /* CK_ENCRYPTION */
#ifdef  CK_NAWS
    TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 0;
    TELOPT_SB(TELOPT_NAWS).naws.x = 0;
    TELOPT_SB(TELOPT_NAWS).naws.y = 0;
#endif /* CK_NAWS */
    TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 0;
    TELOPT_SB(TELOPT_TTYPE).term.type[0] = '\0';
#ifdef CK_ENVIRONMENT
    TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 0;
    if (tn_first)
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
    else if (TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
        free(TELOPT_SB(TELOPT_NEWENVIRON).env.str);
        TELOPT_SB(TELOPT_NEWENVIRON).env.str=NULL;
    }
    TELOPT_SB(TELOPT_NEWENVIRON).env.len=0;
#ifdef CK_XDISPLOC
    TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 0;
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
    TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 0;
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 0;
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.listen_socket = -1;
    for (x = 0; x < MAXFWDX; x++) {
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].fd = -1;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].id = -1;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].need_to_send_xauth = 0;
       TELOPT_SB(TELOPT_FORWARD_X).forward_x.channel[x].suspend = 0;
    }
    /* Reset Xauth data */
    if ( real_xauth ) {
        XauDisposeAuth(real_xauth);
        real_xauth = NULL;
    }
    if ( fake_xauth.name )
        free(fake_xauth.name);
    if ( fake_xauth.data )
        free(fake_xauth.data);
    if ( fake_xauth.address )
        free(fake_xauth.address);
    if ( fake_xauth.number )
        free(fake_xauth.number);
    memset(&fake_xauth,0,sizeof(fake_xauth));
#ifdef NT
    TELOPT_SB(TELOPT_FORWARD_X).forward_x.thread_started = 0;
#endif /* NT */
#endif /* CK_FORWARD_X */
#ifdef CK_SSL
    if (tls_only_flag || ssl_only_flag) {
        TELOPT_ME_MODE(TELOPT_START_TLS) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
    }
    TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 0;
    TELOPT_SB(TELOPT_START_TLS).start_tls.auth_request = 0;
#endif /* CK_SSL */

#ifdef CK_ENCRYPTION
    if (!ck_crypt_is_installed()
#ifdef CK_SSL
        || tls_only_flag || ssl_only_flag
#endif /* CK_SSL */
        ) {
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    }
#endif /* CK_ENCRYPTION */

#ifdef TN_COMPORT
    TELOPT_SB(TELOPT_COMPORT).comport.need_to_send = 0;
    TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
    TELOPT_SB(TELOPT_COMPORT).comport.wait_for_ms = 0;
    tnc_init();
#endif /* TN_COMPORT */

    tn_first = 0;                       /* No longer the first time init */

#ifdef OS2
    ttnum = -1;                         /* Reset TermType negotiation */
    ttnumend = 0;
#endif /* OS2 */

    return(0);
}

int
tn_start() {
    int wait, x, opt;

    /* if (!IS_TELNET()) return(0); */

    if (tn_init && tn_begun)
        return(0);
    tn_begun = 1;

    debug(F111,"tn_start","sstelnet",sstelnet);
    wait = 0;
    if (tn_duplex)  {
        oldplex = duplex;               /* save old duplex value */
        duplex = 1;                     /* and set to half duplex for telnet */
    }
#ifdef CK_SSL
    if (!TELOPT_ME(TELOPT_START_TLS) &&
        TELOPT_ME_MODE(TELOPT_START_TLS) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_START_TLS) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_START_TLS) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_START_TLS) &&
        TELOPT_U_MODE(TELOPT_START_TLS) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_START_TLS) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_START_TLS) = 1;
        wait = 1;
    }
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
    debug(F110,"tn_ini() CK_AUTHENTICATION","",0);
    if (tn_init)                /* tn_ini() might be called recursively */
      return(0);
    if (!TELOPT_ME(TELOPT_AUTHENTICATION) &&
        TELOPT_ME_MODE(TELOPT_AUTHENTICATION) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_AUTHENTICATION) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_AUTHENTICATION) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_AUTHENTICATION) &&
        TELOPT_U_MODE(TELOPT_AUTHENTICATION) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_AUTHENTICATION) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_AUTHENTICATION) = 1;
        wait = 1;
    }
#ifdef CK_ENCRYPTION
    if (TELOPT_U_MODE(TELOPT_AUTHENTICATION) == TN_NG_RF &&
         TELOPT_ME_MODE(TELOPT_AUTHENTICATION) == TN_NG_RF) {
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    }
#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */

#ifdef CK_NAWS
#ifndef NOLOCAL
    debug(F110,"tn_ini() CK_NAWS !NOLOCAL","",0);
    if (!sstelnet) {
        /* Console terminal screen rows and columns */
#ifdef OS2
        debug(F101,
              "tn_ini tt_rows 1",
              "",
              VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0)
              );
        debug(F101,"tn_ini tt_cols 1","",VscrnGetWidth(VTERM));
        /* Not known yet */
        if (VscrnGetWidth(VTERM) < 0 ||
            VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) < 0) {
            ttgwsiz();                  /* Try to find out */
        }
        debug(F101,
              "tn_ini tt_rows 2",
              "",
              VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0)
              );
        debug(F101,"tn_ini tt_cols 2","",VscrnGetWidth(VTERM));
        /* Now do we know? */
        if (VscrnGetWidth(VTERM) > 0 &&
            VscrnGetHeight(VTERM)-(tt_status[VTERM]?1:0) > 0) {
            if (!TELOPT_ME(TELOPT_NAWS) &&
                TELOPT_ME_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
                if (tn_sopt(WILL, TELOPT_NAWS) < 0)
                  return(-1);
                TELOPT_UNANSWERED_WILL(TELOPT_NAWS) = 1;
                wait = 1;
            }
        }
#else /* OS2 */
        debug(F101,"tn_ini tt_rows 1","",tt_rows);
        debug(F101,"tn_ini tt_cols 1","",tt_cols);
        if (tt_rows < 0 || tt_cols < 0) { /* Not known yet */
            ttgwsiz();                  /* Try to find out */
        }
        debug(F101,"tn_ini tt_rows 2","",tt_rows);
        debug(F101,"tn_ini tt_cols 2","",tt_cols);
        if (tt_rows > 0 && tt_cols > 0) { /* Now do we know? */
            if (!TELOPT_ME(TELOPT_NAWS) &&
                TELOPT_ME_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
                if (tn_sopt(WILL, TELOPT_NAWS) < 0)
                  return(-1);
                TELOPT_UNANSWERED_WILL(TELOPT_NAWS) = 1;
                wait = 1;
            }
        }
#endif /* OS2 */
    } else
#endif /* NOLOCAL */
    {
        if (!TELOPT_U(TELOPT_NAWS) &&
            TELOPT_U_MODE(TELOPT_NAWS) >= TN_NG_RQ) {
            if (tn_sopt(DO, TELOPT_NAWS) < 0)
              return(-1);
            TELOPT_UNANSWERED_DO(TELOPT_NAWS) = 1;
            wait = 1;
        }
    }
#endif /* CK_NAWS */

    if (!TELOPT_ME(TELOPT_SGA) &&
        TELOPT_ME_MODE(TELOPT_SGA) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_SGA) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_SGA) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_SGA) &&
        TELOPT_U_MODE(TELOPT_SGA) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_SGA) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_SGA) = 1;
        wait = 1;
    }
    if (!tn_duplex) {
        if (!TELOPT_U(TELOPT_ECHO) &&
            TELOPT_U_MODE(TELOPT_ECHO) >= TN_NG_RQ) {
            if (tn_sopt(DO, TELOPT_ECHO) < 0)
              return(-1);
            TELOPT_UNANSWERED_DO(TELOPT_ECHO) = 1;
            wait = 1;
        }
    }
    if (!TELOPT_ME(TELOPT_ECHO) &&
        TELOPT_ME_MODE(TELOPT_ECHO) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_ECHO) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_ECHO) = 1;
        wait = 1;
    }

    debug(F100,"tn_ini about to send WILL TTYPE if requested","",0);
/*
  Talking to TELNET port, so send WILL TERMINAL TYPE and DO SGA.
  Also send WILL NAWS if we know our screen dimensions.
*/
    if (!TELOPT_ME(TELOPT_TTYPE) &&
        TELOPT_ME_MODE(TELOPT_TTYPE) >= TN_NG_RQ) {
        if ((x = tn_sopt(WILL,TELOPT_TTYPE)) < 0) {
            debug(F101,"tn_ini tn_sopt WILL TTYPE failed","",x);
            return(-1);
        }
        TELOPT_UNANSWERED_WILL(TELOPT_TTYPE) = 1;
        wait = 1;
        debug(F100,"tn_ini sent WILL TTYPE ok","",0);
    }
    if (!TELOPT_U(TELOPT_TTYPE) &&
        TELOPT_U_MODE(TELOPT_TTYPE) >= TN_NG_RQ) {
        if ((x = tn_sopt(DO,TELOPT_TTYPE)) < 0) {
            debug(F101,"tn_ini tn_sopt DO TTYPE failed","",x);
            return(-1);
        }
        TELOPT_UNANSWERED_DO(TELOPT_TTYPE) = 1;
        wait = 1;
        debug(F100,"tn_ini sent DO TTYPE ok","",0);
    }
    if (!TELOPT_ME(TELOPT_BINARY) &&
        TELOPT_ME_MODE(TELOPT_BINARY) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_BINARY) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_BINARY) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_BINARY) &&
        TELOPT_U_MODE(TELOPT_BINARY) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_BINARY) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_BINARY) = 1;
        wait = 1;
    }
#ifdef CK_SNDLOC
    if (tn_loc) {
        if (!TELOPT_ME(TELOPT_SNDLOC) &&
            TELOPT_ME_MODE(TELOPT_SNDLOC) >= TN_NG_RQ) {
            if (tn_sopt(WILL, TELOPT_SNDLOC) < 0)
              return(-1);
            TELOPT_UNANSWERED_WILL(TELOPT_SNDLOC) = 1;
            wait = 1;
        }
    }
#endif /* CK_SNDLOC */
#ifdef CK_ENVIRONMENT
#ifdef CK_FORWARD_X
    if (!TELOPT_U(TELOPT_FORWARD_X) &&
         TELOPT_U_MODE(TELOPT_FORWARD_X) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_FORWARD_X) < 0)
            return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_FORWARD_X) = 1;
        wait = 1;
    }
#endif /* FORWARD_X */
#ifdef CK_XDISPLOC
    if (!TELOPT_ME(TELOPT_XDISPLOC) &&
         TELOPT_ME_MODE(TELOPT_XDISPLOC) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_XDISPLOC) < 0)
            return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_XDISPLOC) = 1;
        wait = 1;
    }
#endif /* CK_XDISPLOC */
    /* Will send terminal environment. */
    if (!TELOPT_ME(TELOPT_NEWENVIRON) &&
        TELOPT_ME_MODE(TELOPT_NEWENVIRON) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_NEWENVIRON) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_NEWENVIRON) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_NEWENVIRON) &&
        TELOPT_U_MODE(TELOPT_NEWENVIRON) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_NEWENVIRON) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_NEWENVIRON) = 1;
        wait = 1;
    }
#endif /* CK_ENVIRONMENT */

    /* Take care of any other telnet options that require handling. */

    for (opt = TELOPT_FIRST; opt <= TELOPT_LAST; opt++) {
        switch (opt) {
          case TELOPT_AUTHENTICATION:
          case TELOPT_ENCRYPTION:
          case TELOPT_TTYPE:
          case TELOPT_NAWS:
          case TELOPT_BINARY:
          case TELOPT_NEWENVIRON:
          case TELOPT_SNDLOC:
          case TELOPT_XDISPLOC:
          case TELOPT_SGA:
          case TELOPT_ECHO:
          case TELOPT_KERMIT:
          case TELOPT_START_TLS:
          case TELOPT_FORWARD_X:
                break;
            break;
          default:
            if (TELOPT_OK(opt)) {
                if (!TELOPT_ME(opt) &&
                    TELOPT_ME_MODE(opt) >= TN_NG_RQ) {
                    if (tn_sopt(WILL, opt) < 0)
                      return(-1);
                    TELOPT_UNANSWERED_WILL(opt) = 1;
                    wait = 1;
                }
                if (!TELOPT_U(opt) &&
                    TELOPT_U_MODE(opt) >= TN_NG_RQ) {
                    if (tn_sopt(DO, opt) < 0)
                      return(-1);
                    TELOPT_UNANSWERED_DO(opt) = 1;
                    wait = 1;
                }
            }
        }
    }
    if (wait) {
        if (tn_wait("pre-encrypt") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }

#ifdef CK_ENCRYPTION
    if (tn_init)                /* tn_ini() may be called recursively */
      return(0);

    if (!TELOPT_ME(TELOPT_ENCRYPTION) &&
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_ENCRYPTION) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_ENCRYPTION) = 1;
        wait = 1;
    }
    if (!TELOPT_U(TELOPT_ENCRYPTION) &&
        TELOPT_U_MODE(TELOPT_ENCRYPTION) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_ENCRYPTION) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_ENCRYPTION) = 1;
        wait = 1;
    }

    /* If we are going to encrypt, we want to do it before we send any more */
    /* data, especially the terminal type and environment variables.        */
    if (wait) {
        if (tn_wait("post-encrypt") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }
#endif /* CK_ENCRYPTION */

    tn_sdsb();

    if (tn_init)                   /* tn_ini() may be called recursively */
        return(0);

#ifdef IKS_OPTION
    /* Kermit Server negotiation must go last */
    /* Send U before ME */

    if (!TELOPT_U(TELOPT_KERMIT) &&
        TELOPT_U_MODE(TELOPT_KERMIT) >= TN_NG_RQ) {
        if (tn_sopt(DO, TELOPT_KERMIT) < 0)
          return(-1);
        TELOPT_UNANSWERED_DO(TELOPT_KERMIT) = 1;
        wait = 1;
    }
    if (!TELOPT_ME(TELOPT_KERMIT) &&
        TELOPT_ME_MODE(TELOPT_KERMIT) >= TN_NG_RQ) {
        if (tn_sopt(WILL, TELOPT_KERMIT) < 0)
          return(-1);
        TELOPT_UNANSWERED_WILL(TELOPT_KERMIT) = 1;
        wait = 1;
    }
#endif /* IKS_OPTION */

    if (wait) {
        if (tn_wait("end of telnet negotiations") < 0) {
            tn_push();
            return(-1);
        }
        wait = 0;
    }

    tn_sdsb();                          /* Send delayed subnegotiations */
    tn_push();
    return(0);
}

/* Start a telnet connection. */
/* Returns -1 on error, 0 if nothing happens, 1 if init msgs sent ok */

int
tn_ini() {
    int x;

    debug(F101,"tn_ini ttnproto","",ttnproto);
    debug(F101,"tn_ini tn_init","",tn_init);

    if (ttnet != NET_TCPB)              /* Make sure connection is TCP/IP */
      return(0);
/*
    if (!IS_TELNET())
      return(0);
*/
    if (tn_init)                        /* Have we done this already? */
      return(0);                        /* Don't do it again. */

    tn_reset();                         /* Reset telnet parameters */
    tn_begun = 0;                       /* Reset; will be set by tn_start() */

    switch ( ttnproto ) {
      case NP_RLOGIN:
      case NP_K4LOGIN:
      case NP_EK4LOGIN:
      case NP_K5LOGIN:
      case NP_EK5LOGIN:
      case NP_K5U2U:
        tn_init = 1;
        debug(F100,"tn_ini telnet negotiations ignored","tn_init",tn_init);
        return(0);
#ifdef COMMENT
      /* Jeff's code from 30 Dec 2005 - doesn't work with SSL POP server */
      case NP_NONE:
      case NP_SSL:
      case NP_TLS:
        ttnproto = NP_TELNET;           /* pretend it's telnet anyway, */
        oldplex = duplex;               /* save old duplex value */
        duplex = 1;                     /* and set to half duplex for telnet */
        if (inserver)
          debug(F100,"tn_ini skipping telnet negotiations","",0);
	else
	  tn_wait("tn_ini - waiting to see if telnet negotiations were sent");
	tn_push();
        return(0);
      case NP_SSL_RAW:
      case NP_TLS_RAW:
      case NP_TCPRAW:                   /* Raw socket requested. */
        debug(F100,"tn_ini telnet negotiations ignored","tn_init",tn_init);
        return(0);
#else
      /* My code from 4 Dec 2005 - works with SSL POP server */
      case NP_NONE:
      case NP_SSL:
      case NP_TLS:                      /* If not talking to a telnet port, */
      case NP_SSL_RAW:			/* SSL and TLS with Telnet */
      case NP_TLS_RAW:			/* negotiations disabled. */
        ttnproto = NP_TELNET;           /* pretend it's telnet anyway, */
        oldplex = duplex;               /* save old duplex value */
        duplex = 1;                     /* and set to half duplex for telnet */
        if (inserver)
          debug(F100,"tn_ini skipping telnet negotiations","",0);
	else
	  tn_wait("tn_ini - waiting to see if telnet negotiations were sent");
	tn_push();
        return(0);
      case NP_TCPRAW:                   /* Raw socket requested. */
        debug(F100,"tn_ini telnet negotiations ignored","tn_init",tn_init);
        return(0);
#endif	/* COMMENT */
      case NP_KERMIT:                   /* switching to Telnet protocol */
      case NP_SSL_TELNET:
      case NP_TLS_TELNET:
        debug(F101,"tn_ini switching from XXX to Telnet","",ttnproto);
        ttnproto = NP_TELNET;
        /* fall through */
      default:
        /* We are already using a variation on Telnet protocol */
        ;
    }

    x = tn_start();
    tn_init = 1;                        /* Remember successful completion. */

    /* Don't send anything else! */
    debug(F101,"tn_ini duplex","",duplex);
    debug(F101,"tn_ini done, tn_init","",tn_init);
    return(x);
}

int
#ifdef CK_ANSIC
tn_hex(CHAR * buf, int buflen, CHAR * data, int datalen)
#else /* CK_ANSIC */
tn_hex(buf, buflen, data, datalen)
    CHAR * buf;
    int buflen;
    CHAR * data;
    int datalen;
#endif /* CK_ANSIC */
{
    int i = 0, j = 0, k = 0;
    CHAR tmp[16];		/* in case value is treated as negative */
#ifdef COMMENT
    int was_hex = 1;

    for (k=0; k < datalen; k++) {
        if (data[k] < 32 || data[k] >= 127) {
            sprintf(tmp,"%s%02X ",was_hex?"":"\" ",data[k]);
            was_hex = 1;
        } else {
            sprintf(tmp,"%s%c",was_hex?"\"":"",data[k]);
            was_hex = 0;
        }
        ckstrncat((char *)buf,tmp,buflen);
    }
    if (!was_hex)
        ckstrncat((char *)buf,"\" ",buflen);
#else /* COMMENT */
    if (datalen <= 0 || data == NULL || buf == NULL || buflen <= 0)
        return(0);

    for (i = 0; i < datalen; i++) {
        ckstrncat((char *)buf,"\r\n  ",buflen);
        for (j = 0 ; (j < 16); j++) {
            if ((i + j) < datalen)
              sprintf((char *)tmp,
                      "%s%02x ",
                      (j == 8 ? "| " : ""),
                      (unsigned int) data[i + j]
                      );
            else
              sprintf((char *)tmp,
                      "%s   ",
                      (j == 8 ? "| " : "")
                      );
            ckstrncat((char *)buf,(char *)tmp,buflen);
        }
        ckstrncat((char *)buf," ",buflen);
        for (k = 0; (k < 16) && ((i + k) < datalen); k++) {
            sprintf((char *)tmp,
                     "%s%c",
                     (k == 8 ? " " : ""),
                     isprint((char)(data[i+k])) ? data[i+k] : '.'
                     );
            ckstrncat((char *)buf,(char *)tmp,buflen);
        }
        i += j - 1;
    } /* end for */
    ckstrncat((char *)buf,"\r\n  ",buflen);
#endif /* COMMENT */
    return(strlen((char *)buf));
}

VOID
tn_debug(s) char *s; {
#ifdef NOLOCAL
    return;
#else /* NOLOCAL */
#ifdef OS2
    void cwrite(unsigned short);
    char *p = s;
    _PROTOTYP (void os2bold, (void));
    extern int tt_type_mode;
#endif /* OS2 */

    if (!(tn_deb || debses))
      return;
    debug(F111,"tn_debug",s,what);
#ifdef OS2
    if (1) {
        extern unsigned char colorcmd;
        colorcmd ^= 0x8 ;
        printf("%s\r\n",s);
        colorcmd ^= 0x8 ;
    }
    if (!scrninitialized[VTERM]) {
        USHORT x,y;
        checkscreenmode();
        GetCurPos(&y, &x);
        SaveCmdMode(x+1,y+1);
        scrninit();
        RestoreCmdMode();
    }

    if ( ISVTNT(tt_type_mode) && ttnum != -1 && !debses )
        return;

    RequestVscrnMutex( VTERM, SEM_INDEFINITE_WAIT ) ;

    os2bold();                          /* Toggle boldness */
    while (*p)
      cwrite((CHAR) *p++);              /* Go boldly ... */
    os2bold();                          /* Toggle boldness back */
    if (debses) {
        debses = 0;
        cwrite((CHAR) '\015');
        cwrite((CHAR) '\012');
        debses = 1;
    } else {
        cwrite((CHAR) '\015');
        cwrite((CHAR) '\012');
    }
    ReleaseVscrnMutex(VTERM) ;
#else
    if (what != W_CONNECT && what != W_DIALING && 
        what != W_COMMAND && what != W_NOTHING)
      return;                           /* CONNECT/command must be active */
    conoll(s);
#endif /* OS2 */
#endif /* NOLOCAL */
}

/*
  Process in-band Telnet negotiation characters from the remote host.
  Call with the telnet IAC character and the current duplex setting
  (0 = remote echo, 1 = local echo), and a pointer to a function to call
  to read more characters.  Returns:
    6 if DO LOGOUT was received and accepted
    5 if the Kermit start of packet character has changed
    4 if state of remote Internet Kermit Service has changed
    3 if a quoted IAC was received
    2 if local echo must be changed to remote
    1 if remote echo must be changed to local
    0 if nothing happens or no action necessary
   -1 on failure (= internal or i/o error)
*/
#ifdef IKS_OPTION
int
tn_siks(cmd) int cmd; {         /* TELNET SEND IKS SUB */
    CHAR buf[8];
#ifndef NOXFER
    extern CHAR mystch;                 /* Outgoing Start of Packet Char */
#else
    CHAR mystch = '\1';
#endif /* NOXFER */
    int n,m,rc;

    if (ttnet != NET_TCPB) return(0);   /* Must be TCP/IP */
    if (ttnproto != NP_TELNET) return(0); /* Must be telnet protocol */

    if (cmd < KERMIT_START || cmd > KERMIT_RESP_STOP) /* Illegal subcommand */
      return(-1);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    if (cmd == KERMIT_START || cmd == KERMIT_RESP_START) {
        TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 1;
    } else if (cmd == KERMIT_STOP || cmd == KERMIT_RESP_STOP) {
        TELOPT_SB(TELOPT_KERMIT).kermit.me_start = 0;
    } else if (cmd == KERMIT_REQ_STOP)
      TELOPT_SB(TELOPT_KERMIT).kermit.me_req_stop = 1;
    else if (cmd == KERMIT_REQ_START)
      TELOPT_SB(TELOPT_KERMIT).kermit.me_req_start = 1;

    if (cmd == KERMIT_SOP) {
        buf[0] = (CHAR) IAC;
        buf[1] = (CHAR) SB;
        buf[2] = (CHAR) TELOPT_KERMIT;
        buf[3] = (CHAR) (cmd & 0xff);
        buf[4] = (CHAR) mystch;
        buf[5] = (CHAR) IAC;
        buf[6] = (CHAR) SE;
        buf[7] = (CHAR) 0;
#ifdef DEBUG
        if (tn_deb || debses || deblog)
            ckmakmsg( tn_msg_out,TN_MSG_LEN,"TELNET SENT SB KERMIT SOP ",
                      ckctox(mystch,1)," IAC SE",NULL);
#endif /* DEBUG */
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
        debug(F101,tn_msg_out,"",cmd);
        if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
        rc = ( ttol(buf,7) < 7 );                /* Send it. */
#ifdef OS2
        ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    } else {
        buf[0] = (CHAR) IAC;
        buf[1] = (CHAR) SB;
        buf[2] = (CHAR) TELOPT_KERMIT;
        buf[3] = (CHAR) (cmd & 0xff);
        buf[4] = (CHAR) IAC;
        buf[5] = (CHAR) SE;
        buf[6] = (CHAR) 0;

#ifdef DEBUG
        if (tn_deb || debses || deblog) {
            char * s = 0;
            switch (cmd) {
              case KERMIT_START: s = "START"; break;
              case KERMIT_STOP: s = "STOP"; break;
              case KERMIT_REQ_START: s = "REQ-START"; break;
              case KERMIT_REQ_STOP: s = "REQ-STOP"; break;
              case KERMIT_RESP_START: s = "RESP-START"; break;
              case KERMIT_RESP_STOP:  s = "RESP-STOP"; break;
            }
            ckmakmsg( tn_msg_out,TN_MSG_LEN,
                      "TELNET SENT SB kermit ",s," IAC SE",NULL);
        }
#endif /* DEBUG */
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
        debug(F101,tn_msg_out,"",cmd);
        if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
        rc = ( ttol(buf,6) < 6 );                /* Send it. */
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        if (rc)
            return(-1);
    }
    return(1);
}
#endif /* IKS_OPTION */

/* tn_sb() performs Telnet Subnegotiation Parsing and Debugging */
/* returns <= 0 on error, 1 on success */
/* the length returned includes the IAC SE bytes */

int
#ifdef CK_ANSIC                         /* TELNET SB */
tn_sb( int opt, int * len, int (*fn)(int) )
#else
tn_sb( opt, len, fn ) int opt; int * len; int (*fn)();
#endif /* CK_ANSIC */
/* tn_sb */ {
    int c, x, y, n, m, flag;
    /* if (!IS_TELNET()) return(1); */

    debug(F100,"Entering tn_sb()","",0);
    *len = 0;                   /* Initialize Len to 0 */
    n = flag = 0;               /* Flag for when done reading SB */
    while (n < TSBUFSIZ) {      /* Loop looking for IAC SE */
        if ((y = (*fn)(0)) < 0) /* Read a byte */
          return(y);
        y &= 0xff;              /* Make sure it's just 8 bits. */
        sb[n++] = (char) y;     /* Deposit in buffer. */
        if (seslog && sessft == XYFT_D) { /* Take care of session log */
            logchar((char) y);
        }
        if (y == IAC) {         /* If this is an IAC                */
            if (flag) {         /* If previous char was IAC         */
                n--;            /* it's quoted, keep one IAC        */
                flag = 0;       /* and turn off the flag.           */
            } else flag = 1;    /* Otherwise set the flag.          */
        } else if (flag) {      /* Something else following IAC     */
            if (y == SE)        /* If not SE, it's a protocol error */
              break;
            else if (y == DONT) { /* Used DONT instead of SE */
                debug(F100,
                      "TELNET Subnegotiation error - used DONT instead of SE!",
                      ""
                      ,0
                      );
                if (tn_deb || debses)
                  tn_debug(
                     "TELNET Subnegotiation error - used DONT instead of SE!");
                flag = 3;
                break;
            } else {            /* Other protocol error */
                flag = 0;
                break;
            }
        }

#ifdef CK_FORWARD_X
        if ( opt == TELOPT_FORWARD_X && sb[0] == FWDX_DATA &&
             n >= (TSBUFSIZ-4) && !flag ) {
            /* do not let the buffer over flow */
            /* write the data to the channel and continue processing */
            /* the incoming data until IAC SE is reached. */
            sb[n++] = IAC;
            sb[n++] = SE;

#ifdef DEBUG
            if ( deblog || tn_deb || debses ) {
                int i;
                ckmakmsg( tn_msg,TN_MSG_LEN,
                          "TELNET RCVD SB ",TELOPT(opt),
			  " DATA(buffer-full) ",NULL);
                tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&sb[1],n-3);
                if (flag == 2)
                    ckstrncat(tn_msg," SE",TN_MSG_LEN);
                else if (flag == 3)
                    ckstrncat(tn_msg," IAC DONT",TN_MSG_LEN);
                else
                    ckstrncat(tn_msg," IAC SE",TN_MSG_LEN);
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses)
                    tn_debug(tn_msg);
            }
#endif /* DEBUG */

            if ( fwdx_tn_sb(sb,n) < 0 ) {
                debug(F100,"fxdx_tn_sb() failed","",0);
                /* We can't return though because that would leave  */
                /* data to be forwarded in the queue to the be sent */
                /* to the terminal emulator.                        */
            }
            /* reset leave the msg type and channel number in place */
            n = 3;
        }
#endif /* CK_FORWARD_X */
    }
    debug(F111,"tn_sb end of while loop","flag",flag);
    if (!flag) {                        /* Make sure we got a valid SB */
        debug(F111, "TELNET Subnegotiation prematurely broken","opt",opt);
        if (tn_deb || debses) {
            ckmakmsg( tn_msg, TN_MSG_LEN,
                      "TELNET ", TELOPT(opt),
                      " Subnegotiation prematurely broken",NULL
                      );

          tn_debug(tn_msg);
        }
        /* Was -1 but that would be an I/O error, so absorb it and go on. */
        return(0);
    }
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        int i;
        char * s[16];
        for (i = 0; i < 16; i++)
          s[i] = "";
        if (opt == TELOPT_NAWS) {
            i = 0;
        } else {
            i = 1;
            s[0] = "UNKNOWN";

            switch (sb[0]) {
              case 0:
                if (opt == TELOPT_FORWARD_X)
                  s[0] = "SCREEN";
                else if (opt == TELOPT_KERMIT)
                  s[0] = "START";
                else if (opt == TELOPT_LFLOW)
                  s[0] = "OFF";
                else if (opt == TELOPT_COMPORT)
                  s[0] = "SIGNATURE";
                else
                  s[0] = "IS";
                if (opt == TELOPT_ENCRYPTION) {
                    i++;
                    if (sb[1] < ENCTYPE_CNT) {
                        s[1] = enctype_names[sb[1]];
                        i++;
                        switch(sb[2]) {
                          case 1:
                            s[2] = "FB64_IV";
                            break;
                          case 2:
                            s[2] = "FB64_IV_OK";
                            break;
                          case 3:
                            s[2] = "FB64_IV_BAD";
                            break;
                          case 4:
                            s[2] = "FB64_CHALLENGE";
                            break;
                          case 5:
                            s[2] = "FB64_RESPONSE";
                            break;
                        }
                    } else {
                        s[1] = "UNKNOWN";
                    }
                }
                if (opt == TELOPT_AUTHENTICATION) {
                    i += 2;
                    s[1] = AUTHTYPE_NAME(sb[1]);
                    s[2] = AUTHMODE_NAME(sb[2]);
                    if (sb[1]) {
                        i++;
                        switch (sb[3]) {
                          case 0:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_AUTH";
                                break;
                              default:
                                s[3] = "AUTH";
                            }
                            break;
                          case 1:
                            switch (sb[1]) {
                              case AUTHTYPE_SSL:
                                s[3] = "START";
                                break;
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_CHALLENGE";
                                break;
                              default:
                                s[3] = "REJECT";
                            }
                            break;
                          case 2:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_RESPONSE";
                                break;
                              default:
                                s[3] = "ACCEPT";
                            }
                            break;
                          case 3:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_ACCEPT";
                                break;
                              case 1:   /* KERBEROS_v4 */
                              case 5:   /* SRP */
                                s[3] = "CHALLENGE";
                                break;
                              case 2:   /* KERBEROS_v5 */
                                s[3] = "RESPONSE";
                                break;
                              case AUTHTYPE_SSL:
                                s[3] = "REJECT";
                                break;
                            }
                            break;
                          case 4:
                            switch (sb[1]) {
                              case AUTHTYPE_NTLM:
                                s[3] = "NTLM_REJECT";
                                break;
                              case 1:   /* KERBEROS_V4 */
                              case 5:   /* SRP */
                                s[3] = "RESPONSE";
                                break;
                              case 2:   /* KERBEROS_V5 */
                                s[3] = "FORWARD";
                                break;
                            }
                            break;
                          case 5:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD";
                                break;
                              case 2:   /* KERBEROS_V5 */
                                s[3] = "FORWARD_ACCEPT";
                                break;
                            }
                            break;
                          case 6:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD_ACCEPT";
                                break;
                              case 2: /* KERBEROS_V5 */
                                s[3] = "FORWARD_REJECT";
                                break;
                            }
                            break;
                          case 7:
                            switch (sb[1]) {
                              case 5:   /* SRP */
                                s[3] = "FORWARD_REJECT";
                                break;
                              case 2: /* KERBEROS_V5 */
                                s[3] = "TLS_VERIFY";
                                break;
                              }
                            break;
                          case 8:
                            switch (sb[1]) {
                              case 5: /* SRP */
                                s[3] = "EXP";
                                break;
                            }
                            break;
                          case 9:
                            switch (sb[1]) {
                              case 5: /* SRP */
                                s[3] = "PARAMS";
                                break;
                            }
                            break;
                        }
                    }
                }
                break;
              case 1:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "OPEN";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "ON";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "STOP";
                    break;
                  case TELOPT_COMPORT:
                    s[0] = "SET-BAUDRATE";
                      break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "SEND";
                    hexbuf[0] = '\0';
                    for (; i < n-2; i += 2) {
                        if ( AUTHTYPE_NAME_OK(sb[i]) &&
                             AUTHMODE_NAME_OK(sb[i]))
                            ckmakmsg( tn_msg, TN_MSG_LEN,
                                      AUTHTYPE_NAME(sb[i])," ",
                                      AUTHMODE_NAME(sb[i+1])," "
                                      );
                        else
                          ckmakxmsg(tn_msg, TN_MSG_LEN,
                                    AUTHTYPE_NAME(sb[i]),
                                    "=",
                                    ckitoa(sb[i]),
                                    " ",
                                    AUTHMODE_NAME(sb[i+1]),
                                    "=",
                                    ckitoa(sb[i+1]),
                                    " ",
                                    NULL,NULL,NULL,NULL
                                    );
                        ckstrncat(hexbuf,tn_msg,sizeof(hexbuf));
                    }
                    s[1] = hexbuf;
                    break;

                  case TELOPT_ENCRYPTION:
                    s[0] = "SUPPORT";
                    while (i < n-2) {
                        s[i] = enctype_names[sb[i]];
                        i++;
                    }
                    break;

                  case TELOPT_START_TLS:
                    s[0] = "FOLLOWS";
                    break;
                  default:
                    s[0] = "SEND";
                }
                break;

              case 2:
                switch (opt) {
                case TELOPT_FORWARD_X:
                    s[0] = "CLOSE";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "RESTART-ANY";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "REQ-START";
                    break;
                  case TELOPT_COMPORT:
                    s[0] = "SET-DATASIZE";
                    break;
                  case TELOPT_NEWENVIRON:
                    s[0] = "INFO";
                    break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "REPLY";
                    i=4;
                    s[1] = AUTHTYPE_NAME(sb[1]);
                    s[2] = AUTHMODE_NAME(sb[2]);
                    switch (sb[3]) {
                      case 0:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_AUTH";
                            break;
                          default:
                            s[3] = "AUTH";
                        }
                        break;
                      case 1:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_CHALLENGE";
                            break;
                          default:
                            s[3] = "REJECT";
                        }
                        break;
                      case 2:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_RESPONSE";
                            break;
                          default:
                            s[3] = "ACCEPT";
                        }
                        break;
                      case 3:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_ACCEPT";
                            break;
                          case AUTHTYPE_KERBEROS_V4:
                          case AUTHTYPE_SRP:
                            s[3] = "CHALLENGE";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "RESPONSE";
                            break;
                        }
                        break;
                      case 4:
                        switch (sb[1]) {
                          case AUTHTYPE_NTLM:
                            s[3] = "NTLM_REJECT";
                            break;
                          case AUTHTYPE_KERBEROS_V4:
                          case AUTHTYPE_SRP:
                            s[3] = "RESPONSE";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD";
                            break;
                        }
                        break;
                      case 5:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD_ACCEPT";
                            break;
                        }
                        break;
                      case 6:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD_ACCEPT";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "FORWARD_REJECT";
                            break;
                        }
                        break;
                      case 7:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "FORWARD_REJECT";
                            break;
                          case AUTHTYPE_KERBEROS_V5:
                            s[3] = "TLS_VERIFY";
                            break;
                        }
                        break;
                      case 8:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "EXP";
                            break;
                        }
                        break;
                      case 9:
                        switch (sb[1]) {
                          case AUTHTYPE_SRP:
                            s[3] = "PARAMS";
                            break;
                        }
                        break;
                    }
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "REPLY";
                    s[1] = enctype_names[sb[1]];
                    i++;
                    switch (sb[2]) {
                      case 1:
                        i++;
                        s[2] = "FB64_IV";
                        break;
                      case 2:
                        i++;
                        s[2] = "FB64_IV_OK";
                        break;
                      case 3:
                        i++;
                        s[2] = "FB64_IV_BAD";
                        break;
                      case 4:
                        i++;
                        s[2] = "FB64_CHALLENGE";
                        break;
                      case 5:
                        i++;
                        s[2] = "FB64_RESPONSE";
                        break;
                    }
                    break;
                }
                break;
              case 3:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "DATA";
                    break;
                  case TELOPT_LFLOW:
                    s[0] = "RESTART-XON";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "REQ-STOP";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "SET-PARITY";
                      break;
                  case TELOPT_AUTHENTICATION:
                    s[0] = "NAME";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "START";
                    break;
                }
                break;
              case 4:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "OPTIONS";
                    break;
                  case TELOPT_KERMIT:
                    s[0] = "SOP";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "SET-STOPSIZE";
                      break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "END";
                    break;
                }
                break;
              case 5:
                switch (opt) {
                case TELOPT_FORWARD_X:
                    s[0] = "OPTION_DATA";
                    break;
                case TELOPT_ENCRYPTION:
                    s[0] = "REQUEST-START";
                    break;
                case TELOPT_COMPORT:
                    s[0] = "SET-CONTROL";
                    break;
                }
                break;
              case 6:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "XOFF";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "REQUEST-END";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "NOTIFY-LINESTATE";
                      break;
                  }
                break;
              case 7:
                switch (opt) {
                  case TELOPT_FORWARD_X:
                    s[0] = "XON";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "ENC-KEYID";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "NOTIFY-MODEMSTATE";
                      break;
                  }
                break;
              case 8:
                switch (opt) {
                  case TELOPT_KERMIT:
                    s[0] = "RESP-START";
                    break;
                  case TELOPT_ENCRYPTION:
                    s[0] = "DEC-KEYID";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "FLOWCONTROL-SUSPEND";
                      break;
                  }
                break;
              case 9:
                switch (opt) {
                  case TELOPT_KERMIT:
                    s[0] = "RESP-STOP";
                    break;
                  case TELOPT_COMPORT:
                      s[0] = "FLOWCONTROL-RESUME";
                      break;
                  }
                break;
              case 10:
                  switch (opt) {
                  case TELOPT_COMPORT:
                    s[0] = "SET-LINESTATE-MASK";
                    break;
                  }
                  break;
            case 11:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "SET-MODEMSTATE-MASK";
                      break;
                  }
                  break;
            case 12:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "PURGE-DATA";
                      break;
                  }
                  break;


            case 100:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SIGNATURE";
                      break;
                  }
                  break;
            case 101:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-BAUDRATE";
                      break;
                  }
                  break;
            case 102:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-DATASIZE";
                      break;
                  }
                  break;
            case 103:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-PARITY";
                      break;
                  }
                  break;
            case 104:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-STOPSIZE";
                      break;
                  }
                  break;
            case 105:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-CONTROL";
                      break;
                  }
                  break;
            case 106:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_NOTIFY-LINESTATE";
                      break;
                  }
                  break;
            case 107:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_NOTIFY-MODEMSTATE";
                      break;
                  }
                  break;
            case 108:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_FLOWCONTROL-SUSPEND";
                      break;
                  }
                  break;
            case 109:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_FLOWCONTROL-RESUME";
                      break;
                  }
                  break;
            case 110:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-LINESTATE-MASK";
                      break;
                  }
                  break;
            case 111:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_SET-MODEMSTATE-MASK";
                      break;
                  }
                  break;
            case 112:
                  switch (opt) {
                  case TELOPT_COMPORT:
                      s[0] = "S_PURGE-DATA";
                      break;
                  }
                  break;
              }
        }
#ifdef M_XENIX
        {
          int len, param, param_len;
          ckmakmsg( tn_msg, TN_MSG_LEN,
                    "TELNET RCVD SB ",
                    TELOPT(opt)," ",NULL);
          len = strlen(tn_msg);
          for (param = 0; param <= 15; param++) {
            param_len = strlen(s[param]);
            if (param_len > 0) {
              strcpy(&tn_msg[len], s[param]);
              len += param_len;
              tn_msg[len++] = ' ';
            }
          }
          tn_msg[len] = '\0';
        }
#else /* M_XENIX */
        ckmakxmsg(tn_msg,TN_MSG_LEN,"TELNET RCVD SB ",TELOPT(opt)," ",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        {
            int i;
            for (i = 0; i < 16; i++) {
                if (s[i][0]) {
                    ckstrncat(tn_msg,s[i],TN_MSG_LEN);
                    ckstrncat(tn_msg," ",TN_MSG_LEN);
                }
            }
        }
#endif /* M_XENIX */
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&sb[i],n-2-i);
        if (flag == 2)
          ckstrncat(tn_msg," SE",TN_MSG_LEN);
        else if (flag == 3)
          ckstrncat(tn_msg," IAC DONT",TN_MSG_LEN);
        else
          ckstrncat(tn_msg," IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses)
          tn_debug(tn_msg);
    }
    debug(F111,"tn_sb","len",n);
#endif /* DEBUG */
    *len = n;           /* return length */
    return(1);          /* success */
}

static char rows_buf[16] = { 0, 0 }; /* LINES Environment variable */
static char cols_buf[16] = { 0, 0 }; /* COLUMNS Enviornment variable */
static char term_buf[64] = { 0, 0 }; /* TERM Environment variable */

#ifdef CK_CURSES
#ifndef VMS
#ifndef COHERENT
_PROTOTYP(int tgetent,(char *, char *));
#endif /* COHERENT */
#else
#ifdef __DECC
_PROTOTYP(int tgetent,(char *, char *));
#endif /* __DECC */
#endif /* VMS */
extern char * trmbuf;                   /* Real curses */
#endif /* CK_CURSES */

#ifdef CK_ENCRYPTION
static int
tn_no_encrypt()
{
    /* Prevent Encryption from being negotiated */
    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;

    /* Cancel any negotiation that might have started */
    ck_tn_enc_stop();

    if (TELOPT_ME(TELOPT_ENCRYPTION) ||
         TELOPT_UNANSWERED_WILL(TELOPT_ENCRYPTION)) {
        TELOPT_ME(TELOPT_ENCRYPTION) = 0;
        if (tn_sopt(WONT,TELOPT_ENCRYPTION) < 0)
            return(-1);
        TELOPT_UNANSWERED_WONT(TELOPT_ENCRYPTION) = 1;
    }
    if (TELOPT_U(TELOPT_ENCRYPTION) ||
         TELOPT_UNANSWERED_DO(TELOPT_ENCRYPTION)) {
        TELOPT_U(TELOPT_ENCRYPTION) = 0;
        if (tn_sopt(DONT,TELOPT_ENCRYPTION) < 0)
            return(-1);
        TELOPT_UNANSWERED_DONT(TELOPT_ENCRYPTION) = 1;
    }
    return(0);
}
#endif /* CK_ENCRYPTION */

/* The following note came from the old SGA negotiation code.  This should */
/* no longer be necessary with the New Telnet negotiation state machine.   */
/*
  Note: The following is proper behavior, and required for talking to the
  Apertus interface to the NOTIS library system, e.g. at Iowa State U:
  scholar.iastate.edu.  Without this reply, the server hangs forever.  This
  code should not be loop-inducing, since C-Kermit never sends WILL SGA as
  an initial bid, so if DO SGA comes, it is never an ACK.
*/
/*
  Return values:
  -1 = Telnet Option negotiation error
  -2 = Connection closed by peer
  -3 = Connection closed by us
  0  = Success
  1  = Echoing on
  2  = Echoing off
  3  = Quoted IAC
  4  = IKS Event
  5  = (unassigned)
  6  = Logout
*/

static int
#ifdef CK_ANSIC                         /* TELNET DO OPTION */
tn_xdoop(CHAR z, int echo, int (*fn)(int))
#else
tn_xdoop(z, echo, fn) CHAR z; int echo; int (*fn)();
#endif /* CK_ANSIC */
/* tn_xdoop */ {
    int c, x, y, n, m;
#ifdef IKS_OPTION
    extern int server;
#ifdef NOICP
    extern int autodl;
    int inautodl = 0, cmdadl = 1;
#else
#ifdef CK_AUTODL
    extern int autodl, inautodl, cmdadl;
#endif /* CK_AUTODL */
#endif /* NOICP */
#endif /* IKS_OPTION */

    /* if (!IS_TELNET()) return(7); */

/* Have IAC, read command character. */

    while ((c = (*fn)(0)) == -1);       /* Read command character */
    if (c < 0)
      return(c);
    c &= 0xFF;                          /* Strip high bits */

    if (!TELCMD_OK(c)) {
#ifdef DEBUG
        if (tn_deb || debses || deblog) {
            ckmakmsg(tn_msg,TN_MSG_LEN,"TELNET RCVD UNKNOWN (",
                     ckitoa(c),")",NULL);
            debug(F101,tn_msg,"",c);
            if (tn_deb || debses)
              tn_debug(tn_msg);
        }
#endif /* DEBUG */
        return(0);
    }
    if (ttnproto == NP_NONE) {
        debug(F100,"tn_doop discovered a Telnet command",
              "ttnproto = NP_TELNET",0);
        ttnproto = NP_TELNET;
    }
    if (seslog && sessft == XYFT_D) {   /* Copy to session log, if any. */
        logchar((char)z);
        logchar((char)c);
    }

    if (c == (CHAR) IAC)                /* Quoted IAC */
      return(3);

    if (c < SB) {                       /* Other command with no arguments. */
#ifdef DEBUG
        if (deblog || tn_deb || debses) {
            ckmakmsg(tn_msg,TN_MSG_LEN,"TELNET RCVD ",TELCMD(c),NULL,NULL);
            debug(F101,tn_msg,"",c);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#endif /* DEBUG */
        switch (c) {                    /* What we would like to do here    */
          case TN_GA:                   /* Is substitute ASCII characters   */
            break;                      /* for the Telnet Command so that   */
          case TN_EL:                   /* the command may be processed by  */
            break;                      /* either the internal emulator or  */
          case TN_EC:                   /* by the superior process or shell */
            break;
          case TN_AYT:
#ifdef OS2
            RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
            ttol((CHAR *)"[Yes]\015\012",7);
#ifdef OS2
            ReleaseTelnetMutex();
#endif
            break;
          case TN_AO:
#ifdef BETADEBUG
            bleep(BP_NOTE);
#endif /* BETADEBUG */
            break;
          case TN_IP:
            break;
          case BREAK:
            break;
          case TN_DM:
            break;
          case TN_NOP:
            break;
          case SE:
            break;
          case TN_EOR:
            break;
          case TN_ABORT:
            break;
          case TN_SUSP:
            break;
          case TN_EOF:
            break;
          case TN_SAK:
            break;
        }
        return(0);
    }

/* SB, WILL, WONT, DO, or DONT need more bytes... */

    if ((x = (*fn)(0)) < 0)             /* Get the option. */
      return(x);
    x &= 0xff;                          /* Trim to 8 bits. */

    if (seslog && sessft == XYFT_D) {   /* Session log */
        logchar((char) x);
    }
#ifdef DEBUG
    if ((deblog || tn_deb || debses) && c != SB) {
        ckmakmsg(tn_msg,TN_MSG_LEN,"TELNET RCVD ",TELCMD(c)," ",TELOPT(x));
        debug(F101,tn_msg,"",x);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#endif /* DEBUG */
    /* Now handle the command */
    switch (c) {
      case WILL:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
#ifdef CK_FORWARD_X
          if (x == TELOPT_FORWARD_X) {
              if (!fwdx_server_avail() || !(fwdx_no_encrypt ||
#ifdef CK_SSL
                 (ssl_active_flag || tls_active_flag)
#else /* CK_SSL */
                 0
#endif /* CK_SSL */
                 ||
#ifdef CK_ENCRYPTION
                 (ck_tn_encrypting() && ck_tn_decrypting())
#else /* CK_ENCRYPTION */
                 0
#endif /* CK_ENCRYPTION */
                 )) {
                  TELOPT_U_MODE(TELOPT_FORWARD_X) = TN_NG_RF;
                  TELOPT_ME_MODE(TELOPT_FORWARD_X) = TN_NG_RF;
              }
          }
#endif /* CK_FORWARD_X */
        if (!TELOPT_OK(x) || TELOPT_U_MODE(x) == TN_NG_RF) {
            if (tn_sopt(DONT,x) < 0)
              return(-1);
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        } else if (!TELOPT_U(x)) {
            if (!TELOPT_UNANSWERED_DO(x)) {
                if (tn_sopt(DO,x) < 0)
                  return -1;
            }
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
            TELOPT_U(x) = 1;

            switch (x) {
#ifdef CK_SSL
              case TELOPT_START_TLS:
                /*
                   If my proposal is accepted, at this point the Telnet
                   protocol is turned off and a TLS negotiation takes
                   place.

                   Start by sending SB START_TLS FOLLOWS  to signal
                   we are ready.  Wait for the peer to send the same
                   and then start the TLS negotiation.

                   If the TLS negotiation succeeds we call tn_ini()
                   again to reset the telnet state machine and restart
                   the negotiation process over the now secure link.

                   If the TLS negotiation fails, we call ttclos()
                   to terminate the connection.

                   Only the server should receive a WILL START_TLS
                 */
                tn_ssbopt(TELOPT_START_TLS,1,NULL,0);
                TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 1;
                break;
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION: {
                  /* We now have to perform a SB SEND to identify the  */
                  /* supported authentication types to the other side. */
                  extern int authentication_version;

#ifdef CK_SSL
                  /* if we have an outstanding DO START_TLS then we must 
                   * wait for the response before we determine what to do
                   */
                  if (TELOPT_UNANSWERED_DO(TELOPT_START_TLS)) {
                      TELOPT_SB(TELOPT_START_TLS).start_tls.auth_request = 1;
                      break;
                  }
#endif /* CK_SSL */
                  authentication_version = AUTHTYPE_AUTO;
                  ck_tn_auth_request();
                  break;
              }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                if (!(TELOPT_ME(TELOPT_AUTHENTICATION) ||
                      TELOPT_U(TELOPT_AUTHENTICATION))
                    ) {
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                    TELOPT_U(x) = 0;
                } else {
                    if (ck_tn_auth_in_progress()) {
                        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 1;
                    } else {
                        /* Perform subnegotiation */
                        ck_encrypt_send_support();
                    }
                    if (!(TELOPT_ME(x) || TELOPT_UNANSWERED_WILL(x))
                        && TELOPT_ME_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
                if (!TELOPT_ME(x)) {
                    /* Tell the other side what Start of Packet Character */
                    tn_siks(KERMIT_SOP); /* SOP */

                    if (!TELOPT_UNANSWERED_WILL(x) &&
                        TELOPT_ME_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
#endif /* IKS_OPTION */
              case TELOPT_BINARY:
                if (!TELOPT_ME(x)) {
                    if (!TELOPT_UNANSWERED_WILL(x) &&
                        TELOPT_ME_MODE(x) >= TN_NG_RQ) {
                        if (tn_sopt(WILL, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_WILL(x) = 1;
                    }
                }
                break;
              case TELOPT_ECHO:
                if (echo) {
                    if (TELOPT_UNANSWERED_DO(x))
                      TELOPT_UNANSWERED_DO(x) = 0;
                    return(2);
                }
                break;
              case TELOPT_TTYPE:
                /* SB TTYPE SEND */
                tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                break;
#ifdef CK_ENVIRONMENT
              case TELOPT_NEWENVIRON:   /* SB NEW-ENVIRON SEND */
                {
                  char request[6];      /* request it */
                  sprintf(request,"%cUSER",TEL_ENV_VAR);        /* safe */
                  tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_SEND,request,
                            strlen(request));
                  TELOPT_UNANSWERED_SB(TELOPT_NEWENVIRON)=1;
                }
                break;
#endif /* CK_ENVIRONMENT */
            } /* switch */
        } else {
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        }
        break;
      case WONT:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (TELOPT_U(x) || TELOPT_UNANSWERED_DO(x)) {
            /* David Borman says we should not respond DONT when
             * the WONT is a response to a DO that we sent.
             * Nor should we send one if the state is already WONT
             * such as when we do not recognize the option since
             * options are initialized in the WONT/DONT state.
             */
            if (!(TELOPT_UNANSWERED_DO(x) || TELOPT_UNANSWERED_DONT(x)))
              if (tn_sopt(DONT,x) < 0)
                return(-1);
            if (TELOPT_UNANSWERED_DONT(x))
                TELOPT_UNANSWERED_DONT(x) = 0;
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
            if (TELOPT_U(x)) {
                TELOPT_U(x) = 0;
            }
            switch(x) {
#ifdef CK_SSL
            case TELOPT_START_TLS:
                if (sstelnet) {
                    if (TELOPT_U_MODE(x) == TN_NG_MU) {
                        printf("Telnet Start-TLS refused.\n");
                        ttclos(0);
                        whyclosed = WC_TELOPT;
                        return(-3);
                    }
                    if (TELOPT_SB(x).start_tls.auth_request) {
                        extern int authentication_version;
                        TELOPT_SB(x).start_tls.auth_request = 0;
                        authentication_version = AUTHTYPE_AUTO;
                        ck_tn_auth_request();
                    }
                }
                break;
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION:
                if (sstelnet && TELOPT_U_MODE(x) == TN_NG_MU) {
                    printf("Telnet authentication refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else if (TELOPT_U_MODE(x) == TN_NG_RQ) {
                    TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (ck_tn_auth_in_progress())
                  printf("Telnet authentication refused.\n");
#ifdef CK_ENCRYPTION
                if (sstelnet) {
                    if (tn_no_encrypt()<0)
                        return(-1);
                }
#endif /* CK_ENCRYPTION */
                break;
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                ck_tn_enc_stop();
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
                TELOPT_SB(x).kermit.u_start = 0;
                TELOPT_SB(x).kermit.me_req_start = 0;
                TELOPT_SB(x).kermit.me_req_stop = 0;
                break;
#endif /* IKS_OPTION */
              case TELOPT_NAWS: {
                  /* The client does not support NAWS. */
                  /* Assume a height of 24 and a width of 80 */
                  if (sstelnet
#ifdef IKSD
                   || inserver
#endif /* IKSD */
                   ) {
                      int w = 80, h = 24;
#ifndef NOLOCAL
                      if (tcp_incoming) {
#ifdef OS2
                          tt_cols[VTERM] = w;
                          tt_rows[VTERM] = h;
                          VscrnSetWidth(VTERM, w);
                          VscrnSetHeight(VTERM, h+(tt_status[VTERM]?1:0));
#else /* OS2 */
                          tt_cols = w;
                          tt_rows = h;
#endif /* OS2 */
                      } else {
#ifdef OS2
                          tt_cols[VCMD] = w;
                          tt_rows[VCMD] = h;
                          VscrnSetWidth(VCMD, w);
                          VscrnSetHeight(VCMD, h);
#endif /* OS2 */
                          cmd_cols = w;
                          cmd_rows = h;
                      }
#else /* NOLOCAL */
                      cmd_cols = w;
                      cmd_rows = h;
#endif /* NOLOCAL */
                      /* Add LINES and COLUMNS to the environment */
                      ckmakmsg((char *)rows_buf,16,"LINES=",ckitoa(h),
                                NULL,NULL);
                      ckmakmsg((char *)cols_buf,16,"COLUMNS=",ckitoa(w),
                                NULL,NULL);
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                      putenv(rows_buf);
                      putenv(cols_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
                  }
                  break;
              }
              case TELOPT_ECHO:
                if (!echo) {
                    if (TELOPT_UNANSWERED_DO(x))
                      TELOPT_UNANSWERED_DO(x) = 0;
                    return(1);
                }
                break;
            }
        } else {
            if (TELOPT_UNANSWERED_DONT(x))
                TELOPT_UNANSWERED_DONT(x) = 0;
            if (TELOPT_UNANSWERED_DO(x))
                TELOPT_UNANSWERED_DO(x) = 0;
        }
        if (TELOPT_U_MODE(x) == TN_NG_MU) {
            ckmakmsg( tn_msg,TN_MSG_LEN,
                      "Peer refuses TELNET DO ",TELOPT(x),
                      " negotiations - terminating connection",NULL
                    );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
            printf("%s\n",tn_msg);
            ttclos(0);
            whyclosed = WC_TELOPT;
            return(-3);
        }
#ifdef COMMENT
        if (x == TELOPT_ECHO && !echo) /* Special handling for echo */
          return(1);                   /* because we allow 'duplex' */
#endif /* COMMENT */
        break;

      case DO:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (!TELOPT_OK(x) || TELOPT_ME_MODE(x) == TN_NG_RF) {
            if (tn_sopt(WONT,x) < 0)
              return(-1);
            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
        } else if (!TELOPT_ME(x)) {
            if (!TELOPT_UNANSWERED_WILL(x)) {
                if (tn_sopt(WILL,x) < 0)
                  return(-1);
            }
            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
            TELOPT_ME(x) = 1;

            switch (x) {
#ifdef CK_SSL
              case TELOPT_START_TLS:
                /*
                   If my proposal is accepted at this point the Telnet
                   protocol is turned off and a TLS negotiation takes
                   place.

                   Start by sending SB START_TLS FOLLOWS  to signal
                   we are ready.  Wait for the peer to send the same
                   and then start the TLS negotiation.

                   If the TLS negotiation succeeds we call tn_ini()
                   again to reset the telnet state machine and restart
                   the negotiation process over the now secure link.

                   If the TLS negotiation fails, we call ttclos()
                   to terminate the connection.  Then we set the
                   U_MODE and ME_MODE for TELOPT_START_TLS to REFUSE
                   and then call ttopen() to create a new connection
                   to the same host but this time do not attempt
                   TLS security.

                   Only the client should receive DO START_TLS.
                */
                tn_ssbopt(TELOPT_START_TLS,1,NULL,0);
                TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows = 1;
                break;
#endif /* CK_SSL */

#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION: {
                  /* We don't know what authentication we are using yet */
                  /* but it is not NULL until a failure is detected so */
                  /* use AUTO in the meantime. */
                  extern int authentication_version;
                  authentication_version = AUTHTYPE_AUTO;
                  break;
              }
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
              case TELOPT_ENCRYPTION:
                if (!(TELOPT_ME(TELOPT_AUTHENTICATION) ||
                      TELOPT_U(TELOPT_AUTHENTICATION))
                    ) {
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                    TELOPT_ME(x) = 0;
                } else {
                    if (!(TELOPT_U(x) || TELOPT_UNANSWERED_DO(x))
                        && TELOPT_U_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
              case TELOPT_KERMIT:
/* If currently processing Kermit server packets, must tell the other side */

                debug(F111,"tn_doop","what",what);
                debug(F111,"tn_doop","server",server);
#ifdef CK_AUTODL
                debug(F111,"tn_doop","autodl",autodl);
                debug(F111,"tn_doop","inautodl",inautodl);
                debug(F111,"tn_doop","cmdadl",cmdadl);
#endif /* CK_AUTODL */

                if (server
#ifdef CK_AUTODL
                    || (local && ((what == W_CONNECT && autodl) ||
                                  (what != W_CONNECT && inautodl)))
                    || (!local && cmdadl)
#endif /* CK_AUTODL */
                    ) {
                    tn_siks(KERMIT_START);      /* START */
                }
                if (!TELOPT_U(x)) {
                    /* Tell the other side what Start of Packet Character */
                    tn_siks(KERMIT_SOP);             /* SOP */
                    if (!TELOPT_UNANSWERED_DO(x) &&
                        TELOPT_U_MODE(x) != TN_NG_RF) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
#endif /* IKS_OPTION */

              case TELOPT_BINARY:
                if (!TELOPT_U(x)) {
                    if (!TELOPT_UNANSWERED_DO(x) &&
                        TELOPT_U_MODE(x) >= TN_NG_RQ) {
                        if (tn_sopt(DO, x) < 0)
                          return(-1);
                        TELOPT_UNANSWERED_DO(x) = 1;
                    }
                }
                break;
              case TELOPT_NAWS:
#ifdef CK_NAWS
                if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                    if (tn_snaws() < 0)
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_NAWS).naws.need_to_send = 1;
                }
#endif /* CK_NAWS */
                break;
              case TELOPT_LOGOUT:
                ttclos(0);              /* And then hangup */
                whyclosed = WC_TELOPT;
                return(6);
#ifdef CK_SNDLOC
               case TELOPT_SNDLOC:
                  if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                      if (tn_sndloc() < 0)
                          return(-1);
                  } else {
                      TELOPT_SB(TELOPT_SNDLOC).sndloc.need_to_send = 1;
                  }
                  break;
#endif /* CK_SNDLOC */
#ifdef CK_FORWARD_X
               case TELOPT_FORWARD_X:
                  if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                      if (fwdx_send_options() < 0) {
                          if (tn_sopt(DONT,x) < 0)
                              return(-1);
                          TELOPT_UNANSWERED_DONT(x) = 1;
                      }
                  } else {
                      TELOPT_SB(TELOPT_FORWARD_X).forward_x.need_to_send = 1;
                  }
                  break;
#endif /* CK_FORWARD_X */
#ifdef TN_COMPORT
              case TELOPT_COMPORT: {
                extern int reliable;
                if (!tn_delay_sb || !tn_outst(0) || tn_init) {
                    if (tn_sndcomport() < 0)
                      return(-1);
                } else {
                    TELOPT_SB(TELOPT_COMPORT).comport.need_to_send = 1;
                }
                /* Telnet -> Serial -> ??? is not a reliable connection. */
                reliable = SET_OFF;
                break;
              }
#endif /* TN_COMPORT */
            } /* switch */
        } else {
          if (TELOPT_UNANSWERED_WILL(x))
            TELOPT_UNANSWERED_WILL(x) = 0;
        }
        break;

      case DONT:
#ifdef CK_SSL
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
            return(0);
#endif /* CK_SSL */
        if (TELOPT_ME(x) || TELOPT_UNANSWERED_WILL(x)) {
            /* David Borman says we should not respond WONT when
             * the DONT is a response to a WILL that we sent.
             * Nor should we send one if the state is already WONT
             * such as when we do not recognize the option since
             * options are initialized in the WONT/DONT state.
             */
            if (!(TELOPT_UNANSWERED_WILL(x) || TELOPT_UNANSWERED_WONT(x)))
              if (tn_sopt(WONT,x) < 0)
                return(-1);

            if (TELOPT_UNANSWERED_WILL(x))
                TELOPT_UNANSWERED_WILL(x) = 0;
            if (TELOPT_UNANSWERED_WONT(x))
                TELOPT_UNANSWERED_WONT(x) = 0;
            if (TELOPT_ME(x))
              TELOPT_ME(x) = 0;

            switch (x) {
#ifdef CK_SSL
            case TELOPT_START_TLS:
                if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
                    printf("Telnet Start-TLS refused.\n");
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                }
                break;
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
              case TELOPT_AUTHENTICATION:
                if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
#ifdef CK_SSL
                    if (tls_active_flag) {
                        TELOPT_ME_MODE(x) = TN_NG_AC;
                        break;
                    } else
#endif /* CK_SSL */
                    {
                        printf("Telnet authentication refused.\n");
                        ttclos(0);
                        whyclosed = WC_TELOPT;
                        return(-3);
                    }
                } else if (TELOPT_ME_MODE(x) == TN_NG_RQ) {
                    TELOPT_ME_MODE(x) = TN_NG_AC;
                }
                if (ck_tn_auth_in_progress())
                  printf("Telnet authentication refused.\n");
#ifdef CK_ENCRYPTION
                if (!sstelnet) {
                    if (tn_no_encrypt()<0)
                        return(-1);
                }
#endif /* CK_ENCRYPTION */
                break;
#endif /* CK_AUTHENTICATION */
              case TELOPT_ENCRYPTION:
#ifdef CK_ENCRYPTION
                ck_tn_enc_stop();
#endif /* CK_ENCRYPTION */
                break;
              case TELOPT_KERMIT:
#ifdef IKS_OPTION
                TELOPT_SB(x).kermit.me_start = 0;
#endif /* IKS_OPTION */
                break;
              default:
                break;
            } /* switch */
        } else {
          if (TELOPT_UNANSWERED_WILL(x))
              TELOPT_UNANSWERED_WILL(x) = 0;
          if (TELOPT_UNANSWERED_WONT(x))
              TELOPT_UNANSWERED_WONT(x) = 0;
        }
        if (TELOPT_ME_MODE(x) == TN_NG_MU) {
            ckmakmsg( tn_msg,TN_MSG_LEN,
                      "Peer refuses TELNET WILL ",TELOPT(x),
                      " negotiations - terminating connection",
                      NULL
                      );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
            printf("%s\n",tn_msg);
            ttclos(0);
            whyclosed = WC_TELOPT;
            return(-3);
        }
        break;
      case SB:
        if ((y = tn_sb(x,&n,fn)) <= 0)
          return(y);

#ifdef CK_SSL
        /* Do not process subnegotiations other than START_TLS after we */
        /* have agreed to begin the TLS negotiation sequence.           */
        if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows &&
             x != TELOPT_START_TLS)
            break;
#endif /* CK_SSL */

        if (!TELOPT_OK(x)) {
            ckhexdump("unknown telnet subnegotiation",sb,n);
            break;
        } else if ( !(TELOPT_ME(x) || TELOPT_U(x)) ) {
            ckhexdump("telnet option not negotiated",sb,n);
            if (!tn_sb_bug)
                break;
            if (TELOPT_UNANSWERED_WILL(x)) {
                TELOPT_UNANSWERED_WILL(x) = 0;
                TELOPT_U(x) = 1;
                ckmakmsg(tn_msg,TN_MSG_LEN,
                         "TELNET DO ",TELOPT(x),
                         "(implied by receipt of SB - protocol error ignored)",
                         NULL
                         );
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses) tn_debug(tn_msg);
            }
            if (TELOPT_UNANSWERED_DO(x)) {
                TELOPT_UNANSWERED_DO(x) = 0;
                TELOPT_ME(x) = 1;
                ckmakmsg(tn_msg,TN_MSG_LEN,"TELNET WILL ",TELOPT(x),
                        " (implied by receipt of SB - protocol error ignored)",
                        NULL);
                debug(F100,tn_msg,"",0);
                if (tn_deb || debses) tn_debug(tn_msg);
             }
        }

        TELOPT_UNANSWERED_SB(x)=0;
        switch (x) {
#ifdef CK_FORWARD_X
          case TELOPT_FORWARD_X:
            return(fwdx_tn_sb(sb, n));
#endif /* CK_FORWARD_X */
#ifdef CK_SSL
          case TELOPT_START_TLS: {
              /*
                 the other side is saying SB START_TLS FOLLOWS
                 the incoming channel is now ready for starting the
                 TLS negotiation.
                 */
              int def_tls_u_mode, def_tls_me_mode;
              int def_enc_u_mode, def_enc_me_mode;
              int rc = 0;

                          if (sb[0] != 1) {
                                  break;
                          }

              TELOPT_SB(TELOPT_START_TLS).start_tls.u_follows = 1;
              /* Preserve the default modes and make sure we will */
              /* refuse START_TLS when we retry. */
              if (sstelnet) {
                  def_tls_u_mode = TELOPT_DEF_S_U_MODE(TELOPT_START_TLS);
                  def_tls_me_mode = TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS);
                  TELOPT_DEF_S_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
                  TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS)= TN_NG_RF;
#ifdef CK_ENCRYPTION
                  def_enc_u_mode = TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION);
                  def_enc_me_mode = TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION);
                  TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                  TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION)= TN_NG_RF;
#endif /* CK_ENCRYPTION */
              } else {
                  def_tls_u_mode = TELOPT_DEF_C_U_MODE(TELOPT_START_TLS);
                  def_tls_me_mode = TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS);
                  TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) = TN_NG_RF;
                  TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS)= TN_NG_RF;
#ifdef CK_ENCRYPTION
                  def_enc_u_mode = TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION);
                  def_enc_me_mode = TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION);
                  TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                  TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION)= TN_NG_RF;
#endif /* CK_ENCRYPTION */
              }
              /* Negotiate TLS */
              ttnproto = NP_TLS;
              tn_init = 0;
              tn_begun = 0;
              if (ck_tn_tls_negotiate()<0) {
                  /* we failed.  disconnect and if we are the client */
                  /* then reconnect and try without START_TLS.       */
                  extern char * line;
                  int x = -1;
                  extern int mdmtyp;

                  if (sstelnet) {
                      printf("TLS failed:  Disconnecting.\n");
                      TELOPT_DEF_S_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                      TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                     TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                     TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
                      ttclos(0);
                      whyclosed = WC_TELOPT;
                      ttnproto = NP_TELNET;
                      rc = -3;
                  } else {
#ifndef NOLOCAL
                      extern int tls_norestore;
#endif /* NOLOCAL */
                      printf("TLS failed:  Disconnecting...\n");
#ifdef CK_ENCRYPTION
                     TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                     TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
                      /* if START_TLS is not REQUIRED, then retry without it */
                      if ( def_tls_me_mode != TN_NG_MU ) {
                          extern char ttname[];
#ifndef NOLOCAL
                          tls_norestore = 1;
#endif /* NOLOCAL */
                          ttclos(0);
                          whyclosed = WC_TELOPT;
#ifndef NOLOCAL
                          tls_norestore = 0;
#endif /* NOLOCAL */
                          ttnproto = NP_TELNET;
                          printf("Reconnecting without TLS.\n");
                          sleep(2);
                          if (ttopen(ttname,&x,mdmtyp,0)<0)
                              rc = -3;
                      } else {
                          TELOPT_DEF_C_U_MODE(TELOPT_START_TLS) =
                            def_tls_u_mode;
                          TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) =
                            def_tls_me_mode;
                          ttclos(0);
                          whyclosed = WC_TELOPT;
                          ttnproto = NP_TELNET;
                          rc = -3;
                      }
                  }
              } else {
#ifdef CK_AUTHENTICATION
                  /* we succeeded.  restart telnet negotiations from */
                  /* the beginning.  However, if we have received a  */
                  /* client certificate and we are a server, then do */
                  /* not offer TELOPT_AUTH.                          */
                  if ( ck_tn_auth_valid() == AUTH_VALID ) {
                      TELOPT_DEF_S_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_AC;
                      TELOPT_DEF_S_ME_MODE(TELOPT_AUTHENTICATION)= TN_NG_AC;
                  }
#endif /* CK_AUTHENTICATION */
                  ttnproto = NP_TELNET;
                  if (tn_ini() < 0)
                    if (ttchk() < 0)
                      rc = -1;
              }
              /* Restore the default modes */
              if (sstelnet) {
                  TELOPT_DEF_S_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                  TELOPT_DEF_S_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                  TELOPT_DEF_S_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                  TELOPT_DEF_S_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
              } else {
                  TELOPT_DEF_C_U_MODE(TELOPT_START_TLS)  = def_tls_u_mode;
                  TELOPT_DEF_C_ME_MODE(TELOPT_START_TLS) = def_tls_me_mode;
#ifdef CK_ENCRYPTION
                  TELOPT_DEF_C_U_MODE(TELOPT_ENCRYPTION)  = def_enc_u_mode;
                  TELOPT_DEF_C_ME_MODE(TELOPT_ENCRYPTION) = def_enc_me_mode;
#endif /* CK_ENCRYPTION */
              }
              return(rc);
          }
#endif /* CK_SSL */
#ifdef CK_AUTHENTICATION
          case TELOPT_AUTHENTICATION:
            if (ck_tn_sb_auth((char *)sb,n) < 0) {
                if (sstelnet && TELOPT_U_MODE(x) == TN_NG_MU) {
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else if (!sstelnet && TELOPT_ME_MODE(x) == TN_NG_MU) {
                    ttclos(0);
                    whyclosed = WC_TELOPT;
                    return(-3);
                } else {
                    if (TELOPT_ME_MODE(x) == TN_NG_RQ)
                      TELOPT_ME_MODE(x) = TN_NG_AC;
                    if (TELOPT_U_MODE(x) == TN_NG_RQ)
                      TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (TELOPT_ME(x)) {
                    TELOPT_ME(x) = 0;
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                }
                if (TELOPT_U(x)) {
                    TELOPT_U(x) = 0;
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                }
#ifdef CK_ENCRYPTION
                if (tn_no_encrypt()<0)
                    return(-1);
#endif /* CK_ENCRYPTION */
            } else {
#ifdef CK_ENCRYPTION
                if (!ck_tn_auth_in_progress()) { /* we are finished */
                    if (ck_tn_authenticated() == AUTHTYPE_SSL) {
                        /* TLS was successful.  Disable ENCRYPTION */
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    }
                    if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send) {
                        ck_encrypt_send_support();
                        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.need_to_send = 0;
                    }
                }
#endif /* CK_ENCRYPTION */
            }
            break;
#endif /* CK_AUTHENTICATION */
#ifdef CK_ENCRYPTION
          case TELOPT_ENCRYPTION:
            if (ck_tn_sb_encrypt((char *)sb, n) < 0) {
                if (TELOPT_U_MODE(x) == TN_NG_MU ||
                    TELOPT_ME_MODE(x) == TN_NG_MU)
                  {
                      ttclos(0);
                      whyclosed = WC_TELOPT;
                      return(-3);
                } else {
                    if (TELOPT_ME_MODE(x) == TN_NG_RQ)
                      TELOPT_ME_MODE(x) = TN_NG_AC;
                    if (TELOPT_U_MODE(x) == TN_NG_RQ)
                      TELOPT_U_MODE(x) = TN_NG_AC;
                }
                if (TELOPT_ME(x)) {
                    TELOPT_ME(x) = 0;
                    if (tn_sopt(WONT,x) < 0)
                      return(-1);
                }
                if (TELOPT_U(x)) {
                    TELOPT_U(x) = 0;
                    if (tn_sopt(DONT,x) < 0)
                      return(-1);
                }
            }
            break;
#endif /* CK_ENCRYPTION */
#ifdef IKS_OPTION
          case TELOPT_KERMIT:
            return(iks_tn_sb(sb, n-2));
#endif /* IKS_OPTION */
#ifdef TN_COMPORT
          case TELOPT_COMPORT:
            return(tnc_tn_sb(sb, n-2));
#endif /* TN_COMPORT */
          case TELOPT_TTYPE:
            switch (sb[0]) {
              case TELQUAL_SEND:        /* SEND terminal type? */
                if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                    if (tn_sttyp() < 0) /* Yes, so send it. */
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_TTYPE).term.need_to_send = 1;
                }
                break;
              case TELQUAL_IS: {        /* IS terminal type? */
                  /* IS terminal type -- remote gave us its current type */
                  int i = 0;
#ifndef OS2
                  CHAR oldterm[64], *p;
#endif /* OS2 */
                  /* Isolate the specified terminal type string */
                  while (sb[i++] != IAC) {
                      if (i == 40 ||    /* max len of term string - RFC */
                          sb[i] == IAC) {
                          sb[i] = '\0';
                          break;
                      }
                  }
#ifdef OS2
#ifndef NOTERM
                  strupr(&(sb[1]));     /* Upper case it */
                  for (i = 0; i <= max_tt; i++) { /* find it in our list */
                      if (!strcmp(&(sb[1]),tt_info[i].x_name)
                          && i != TT_VTNT) /* can't support VTNT as server */
                        {
                          /* Set terminal type to the one chosen */
                          if (i != tt_type)
                            settermtype(i,0);
                          break;
                      }
                  }
                  if (i > max_tt &&
                      strcmp(&(sb[1]),TELOPT_SB(TELOPT_TTYPE).term.type)) {
                      /* Couldn't find the specified term type */
                      sb[40] = '\0';
                      strcpy(TELOPT_SB(TELOPT_TTYPE).term.type,&(sb[1]));
                      /* SB TTYPE SEND */
                      tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                      TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                  }
#endif /* NOTERM */
#else /* OS2 */
                  p = (CHAR *) getenv("TERM");
                  if (p)
                    ckstrncpy((char *)oldterm,(char *)p,63);
                  else
                    oldterm[0] = '\0';
                  cklower((char *)&(sb[1])); /* Lower case new term */
                  ckmakmsg(term_buf,64,"TERM=",(char *)&(sb[1]),NULL,NULL);
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                  putenv(term_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
#ifdef CK_CURSES
#ifndef MYCURSES
#ifndef COHERENT
                  if (trmbuf) {
                      if (tgetent(trmbuf,(char *)&sb[1]) < 1) {
                          /* Unsupported terminal.  If new and old terminal */
                          /* types do not match, ask for another type. */
                          if (strcmp((char *)oldterm,(char *)&sb[1])) {
                              /* SB TTYPE SEND */
                              tn_ssbopt(TELOPT_TTYPE,TELQUAL_SEND,NULL,0);
                              TELOPT_UNANSWERED_SB(TELOPT_TTYPE)=1;
                          }
                      }
                  }
#endif /* COHERENT */
#endif /* MYCURSES */
#endif /* CK_CURSES */
#endif /* OS2 */
              }
            }
            break;
#ifdef CK_ENVIRONMENT
#ifdef CK_XDISPLOC
          case TELOPT_XDISPLOC:         /* Send X-Display Location */
            if (sb[0] == TELQUAL_SEND) {/* SEND X-Display Loc? */
                if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                    if (tn_sxdisploc() < 0)     /* Yes, so send it. */
                        return(-1);
                } else {
                    TELOPT_SB(TELOPT_XDISPLOC).xdisp.need_to_send = 1;
                }
            }
            /* IS -- X Display Location (not supported) */
            else if (sb[0] == TELQUAL_IS) {
                int i = 0;
                /* Isolate the specified X-display string */
                while (sb[i++] != IAC) {
                    if (i >= TSBUFSIZ)
                      return (-1);
                    if (sb[i] == IAC) {
                        sb[i] = '\0';
                        break;
                    }
                }
                debug(F110,"TELNET SB XDISPLOC IS",&sb[1],0);
            }
            break;
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */
          case TELOPT_NAWS:
              if (sstelnet
#ifdef IKSD
                   || inserver
#endif /* IKSD */
                   ) {
                  int w = 0, h = 0;
                  int i = 0;
                  /* At this point sb[] should contain width and height */
                  if (sb[i] == IAC) i++;
                  w = (sb[i++] << 8);   /* save upper height */
                  if (sb[i] == IAC) i++;
                  w += sb[i++];         /* save the width */
                  if (sb[i] == IAC) i++;
                  h = (sb[i++] << 8);   /* save upper height */
                  if (sb[i] == IAC) i++;
                  h += sb[i++];
                  debug(F111,"tn_doop NAWS SB","width",w);
                  debug(F111,"tn_doop NAWS SB","height",h);

                  if (w == 0)
                    w = 80;
                  if (h == 0)
                    h = 24;
#ifndef NOLOCAL
                  if (tcp_incoming || inserver) {
#ifdef OS2
                      tt_cols[VTERM] = w;
                      tt_rows[VTERM] = h;
                      VscrnSetWidth(VTERM, w);
                      VscrnSetHeight(VTERM, h+(tt_status[VTERM]?1:0));
#ifdef IKSD
                      if (inserver) {
                          cmd_cols = tt_cols[VCMD] = w;
                          cmd_rows = tt_rows[VCMD] = h;
                          VscrnSetWidth(VCMD, w);
                          VscrnSetHeight(VCMD, h);
                      }
#endif /* IKSD */
#else /* OS2 */
                      tt_cols = w;
                      tt_rows = h;
#endif /* OS2 */
                  } else {
#ifdef OS2
                      tt_cols[VCMD] = w;
                      tt_rows[VCMD] = h;
                      VscrnSetWidth(VCMD, w);
                      VscrnSetHeight(VCMD, h);
#endif /* OS2 */
                      cmd_cols = w;
                      cmd_rows = h;
                  }
#else /* NOLOCAL */
                  cmd_cols = w;
                  cmd_rows = h;
#endif /* NOLOCAL */

                  /* Add LINES and COLUMNS to the environment */
                  ckmakmsg((char *)rows_buf,16,"LINES=",ckitoa(h),NULL,NULL);
                  ckmakmsg((char *)cols_buf,16,"COLUMNS=",ckitoa(w),NULL,NULL);
#ifdef OS2ORUNIX
#ifndef NOPUTENV
                  putenv(rows_buf);
                  putenv(cols_buf);
#endif /* NOPUTENV */
#endif /* OS2ORUNIX */
              }
              break;
#ifdef CK_ENVIRONMENT
          case TELOPT_NEWENVIRON:
            switch (sb[0]) {
              case TELQUAL_IS:                  /* IS */
              case TELQUAL_INFO:                /* INFO */
                if (sb[0] == TELQUAL_IS)
                  debug(F101,"tn_doop NEW-ENV SB IS","",n-3);
                else
                  debug(F101,"tn_doop NEW-ENV SB INFO","",n-3);
                if (sstelnet || inserver) { /* Yes, receive it. */
                    if (tn_rnenv((CHAR *)&sb[1],n-3) < 0)
                      return(-1);
                }
                break;
              case TELQUAL_SEND:        /* SEND */
                if ( sstelnet || inserver )         /* ignore if server */
                    break;
                /* We need to take the sb[] and build a structure */
                /* containing all of the variables and types that */
                /* we are supposed to keep track of and send to   */
                /* the host, then call tn_snenv().                  */
                /* Or we can punt ...                               */
                if ( !tn_delay_sb || !tn_outst(0) || tn_init ) {
                  if (tn_snenv((CHAR *)&sb[1],n-3) < 0) /* Yes, send it. */
                     return(-1);
                } else {
#ifndef VMS
                  CHAR * xxx;
                  xxx = (CHAR *) malloc(n-1);
#else
                  unsigned char * xxx;
                  xxx = (unsigned char *) malloc(n-1);
#endif /* VMS */
                  /* Postpone sending until end of tn_ini() */
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str = xxx;
                  if (TELOPT_SB(TELOPT_NEWENVIRON).env.str) {
                  memcpy((char *)TELOPT_SB(TELOPT_NEWENVIRON).env.str,
                            (char *)&sb[1],n-3);
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str[n-3] = IAC;
                  TELOPT_SB(TELOPT_NEWENVIRON).env.str[n-2] = '\0';
                  TELOPT_SB(TELOPT_NEWENVIRON).env.len = n-3;
                  TELOPT_SB(TELOPT_NEWENVIRON).env.need_to_send = 1;
                  }
                }
                break;
              }
              break;
#endif /* CK_ENVIRONMENT */
#ifdef CK_SNDLOC
          case TELOPT_SNDLOC: {
              if ( deblog ) {
                  sb[n-2] = '\0';
                  debug(F110,"TELNET Send-Location",sb,0);
              }
              break;
          }
#endif /* CK_SNDLOC */
          } /* switch */
        break;
    }
    return(0);
}

int
#ifdef CK_ANSIC                         /* TELNET DO OPTION */
tn_doop(CHAR z, int echo, int (*fn)(int))
#else
tn_doop(z, echo, fn) CHAR z; int echo; int (*fn)();
#endif /* CK_ANSIC */
/* tn_doop */ {
    int x=0, y=0;

    debug(F101,"tn_doop char","",z);
    debug(F101,"tn_doop ttnproto","",ttnproto);

    if (!IS_TELNET()) return(3);

#ifdef CK_SSL
    debug(F101,"tn_doop ssl_raw_flag","",ssl_raw_flag);
    if (ssl_raw_flag || tls_raw_flag) return(7);
#endif	/* CK_SSL */
    debug(F100,"tn_doop ttnproto proceeding...","",0);

    if (z != (CHAR) IAC) {
        debug(F101,"tn_doop bad call","",z);
        return(-1);
    }
    if (ttnet != NET_TCPB)              /* Check network type */
      return(0);
    if (ttnproto != NP_TELNET &&
         ttnproto != NP_NONE)           /* Check protocol */
        return(0);

    x = tn_xdoop(z,echo,fn);
    if (x >= 0 && !tn_begun) {
        y = tn_start();
    }
    return(y < 0 ? y : x);
}

#ifdef CK_ENVIRONMENT

/* Telnet receive new environment */
/* Returns -1 on error, 0 if nothing happens, 1 on success */
/* In order for this code to work, sb[len] == IAC          */
/* We currently only support the USER environment variable */

int
#ifdef CK_ANSIC
tn_rnenv(CHAR * sb, int len)
#else
tn_rnenv(sb, len) CHAR * sb; int len;
#endif /* CK_ANSIC */
/* tn_rnenv */ {                        /* Receive new environment */
    char varname[17];
    char value[65];
    char * reply = 0, * s = 0;
    int i,j,k,n;                                /* Worker. */
    int type = 0; /* 0 for NONE, 1 for VAR, 2 for USERVAR, */
                  /* 3 for VALUE in progress */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (sb == NULL) return(-1);

    if (len == 0) return(1);

    /*
    Pairs of <type> [VAR=0, VALUE=1, ESC=2, USERVAR=3] <value> "unterminated"
    follow here until done...
    */
    for (i = 0, j = 0, k = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
        case TEL_ENV_VAR:               /* VAR */
        case TEL_ENV_USERVAR:           /* USERVAR */
        case IAC:                       /* End of the list */
            switch (type) {
              case 0:                   /* Nothing in progress */
                /* If we get IAC only, then that means there were */
                /* no environment variables to send.  we are done */
                if (j == 0 && sb[i] == IAC)
                    return(1);
              case 1:                   /* VAR in progress */
              case 2:                   /* USERVAR in progress */
              case 3:                   /* VALUE in progress */
                value[k] = '\0';
                varname[j] = '\0';
                debug(F111,"tn_rnenv varname",varname,type);
                debug(F111,"tn_rnenv value",value,type);
                if (!strcmp(varname,"USER")) {
#ifdef CK_AUTHENTICATION
                    if (ck_tn_auth_valid() != AUTH_VALID) {
                        extern char szUserNameRequested[];
                        debug(F100,"tn_rnenv != AUTH_VALID","",0);
                        ckstrncpy(szUserNameRequested,value,UIDBUFLEN);
                        ckstrncpy(uidbuf,value,UIDBUFLEN);
#ifdef CK_SSL
                        if (ssl_active_flag) {
                            if ( tls_is_user_valid(ssl_con, uidbuf) ) {
                                extern char szUserNameAuthenticated[];
                                ckstrncpy(szUserNameAuthenticated,uidbuf,
                                           UIDBUFLEN);
                                auth_finished(AUTH_VALID);
                            }
                        } else if (tls_active_flag) {
                            if ( tls_is_user_valid(tls_con, uidbuf) ) {
                                extern char szUserNameAuthenticated[];
                                ckstrncpy(szUserNameAuthenticated,uidbuf,
                                           UIDBUFLEN);
                                auth_finished(AUTH_VALID);
                            }
                        }
#endif /* CK_SSL */
                    } else {    /* AUTH_VALID */
                        int x = 0;
                        debug(F110,"tn_rnenv AUTH_VALID uidbuf",uidbuf,0);

#ifdef OS2
                        x = ckstrcmp(value,uidbuf,-1,0); /* case insensitive */
#ifdef NT
                        /* NTLM authentication returns names of the form */
                        /* DOMAIN\user.  We need to check to see of the  */
                        /* USER VAR contains a domain name or not.  If   */
                        /* not, then we do not want to change state if   */
                        /* the uidbuf matches the USER VAR when the      */
                        /* DOMAIN is ignored.                            */
                        if ( x && ck_tn_authenticated() == AUTHTYPE_NTLM ) {
                            char * s1=NULL, * s2=NULL;
                            int    len1, len2, i;

                            len1 = strlen(value);
                            for ( i=len1-1 ; i>=0 ; i--) {
                                if ( value[i] == '\\' ) {
                                    s1 = &value[i+1];   /* DOMAIN found */
                                    break;
                                }
                            }

                            if ( s1 == NULL ) {
                                len2 = strlen(uidbuf);
                                for ( i=len2-1 ; i>=0 ; i--) {
                                    if ( uidbuf[i] == '\\' ) {
                                        s2 = &uidbuf[i+1];   /* DOMAIN found */
                                        break;
                                    }
                                }

                                if ( s2 )
                                    x = ckstrcmp(value,s2,-1,0);
                            }
                        }
#endif /* NT */
#else /* OS2 */
                        x = ckstrcmp(value,uidbuf,-1,1); /* case sensitive */
#endif /* OS2 */
                        if ( x ) {
                            extern char szUserNameRequested[];
                            ckstrncpy(uidbuf,value,UIDBUFLEN);
                            ckstrncpy(szUserNameRequested,value,UIDBUFLEN);
                            auth_finished(AUTH_USER);
#ifdef CK_SSL
                            if (ssl_active_flag || tls_active_flag) {
                                if ( tls_is_user_valid(ssl_con, uidbuf) )
                                    auth_finished(AUTH_VALID);
                            }
#endif /* CK_SSL */
                        }
                    }
#else /* CK_AUTHENTICATION */
                    ckstrncpy(uidbuf,value,UIDBUFLEN);
#endif /* CK_AUTHENTICATION */
                }
                break;
            }
            varname[0] = '\0';
            value[0] = '\0';
            j = 0;
            k = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :  /* VAR */
                     0
                     );
            break;
        case TEL_ENV_VALUE: /* VALUE */
            if ( type == 1 || type == 2 )
                type = 3;
            break;
        case TEL_ENV_ESC:       /* ESC */
            /* Take next character literally */
            if ( ++i >= len )
                break;
            /* otherwise, fallthrough so byte will be added to string. */
        default:
            switch (type) {
            case 1:     /* VAR in progress */
            case 2:     /* USERVAR in progress */
                if ( j < 16 )
                    varname[j++] = sb[i];
                break;
            case 3:
                if ( k < 64 )
                    value[k++] = sb[i];
                break;
            }
        }
    }
    return(0);
}

/* These are for Microsoft SFU version 2 Telnet Server */
#define SFUTLNTVER "SFUTLNTVER"
#define SFUTLNTMODE "SFUTLNTMODE"
#define SFUTLNTVER_VALUE  "2"
#define SFUTLNTMODE_VALUE "console"	/* The other value is "stream" */

/* Telnet send new environment */
/* Returns -1 on error, 0 if nothing happens, 1 on success */
/* In order for this code to work, sb[len] == IAC          */

int
#ifdef CK_ANSIC
tn_snenv(CHAR * sb, int len)
#else
tn_snenv(sb, len) CHAR * sb; int len;
#endif /* CK_ANSIC */
/* tn_snenv */ {                        /* Send new environment */
    char varname[16];
    char * reply = 0, * s = 0;
    int i,j,n;                          /* Worker. */
    int type = 0;       /* 0 for NONE, 1 for VAR, 2 for USERVAR in progress */
    extern int ck_lcname;
    char localuidbuf[UIDBUFLEN];	/* (Initialized just below) */
    char * uu = uidbuf;
    char * disp = NULL;

    localuidbuf[0] = '\0';
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!sb) return(-1);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

#ifdef CK_FORWARD_X
    if (TELOPT_U(TELOPT_FORWARD_X)) {
        disp = NULL;
    } else
#endif /* CK_FORWARD_X */
        disp = (char *)tn_get_display();

    if (ck_lcname) {
        ckstrncpy(localuidbuf,uidbuf,UIDBUFLEN);
        cklower(localuidbuf);
        uu = localuidbuf;
    }

    ckhexdump((CHAR *)"tn_snenv sb[]",sb,len);
    debug(F110,"tn_snenv uidbuf",uidbuf,0);
    debug(F110,"tn_snenv localuidbuf",localuidbuf,0);
    debug(F110,"tn_snenv tn_env_sys",tn_env_sys,0);
    debug(F110,"tn_snenv tn_env_disp",tn_env_disp,0);
    debug(F110,"tn_snenv disp",disp,0);

    /* First determine the size of the buffer we will need */
    for (i = 0, j = 0, n = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
          case TEL_ENV_VAR:             /* VAR */
          case TEL_ENV_USERVAR:         /* USERVAR */
        case IAC:                     /* End of the list */
            switch (type) {
            case 0:                   /* Nothing in progress */
                /* If we get IAC only, then that means send all */
                /* VAR and USERVAR.                             */
                if (!(j == 0 && sb[i] == IAC))
                  break;
            case 1:                   /* VAR in progress */
                varname[j] = '\0' ;
                if (!varname[0]) {      /* Send All */
                    if (uu[0])
                      n += strlen(uu) + 4 + 2;
                    if (tn_env_job[0])
                      n += strlen(tn_env_job) + 3 + 2;
                    if (tn_env_acct[0])
                      n += strlen(tn_env_acct) + 4 + 2;
                    if (tn_env_prnt[0])
                      n += strlen(tn_env_prnt) + 7 + 2;
                    if (tn_env_sys[0])
                      n += strlen(tn_env_sys) + 10 + 2;
                    if (disp)
                      n += strlen(disp) + 7 + 2;
                } else if (!strcmp(varname,"USER") && uu[0])
                  n += strlen(uu) + 4 + 2;
                else if (!strcmp(varname,"JOB") && tn_env_job[0])
                  n += strlen(tn_env_job) + 3 + 2;
                else if (!strcmp(varname,"ACCT") && tn_env_acct[0])
                  n += strlen(tn_env_acct) + 4 + 2;
                else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0])
                  n += strlen(tn_env_prnt) + 7 + 2;
                else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0])
                  n += strlen(tn_env_sys) + 10 + 2;
                else if (!strcmp(varname,"DISPLAY") && disp)
                  n += strlen(disp) + 7 + 2;
                /* If we get IAC only, then that means send all */
                /* VAR and USERVAR.                             */
                  if (!(j == 0 && sb[i] == IAC))
                      break;
            case 2:                   /* USERVAR in progress */
                varname[j] = '\0' ;
                if (!varname[0]) {      /* Send All */
                    int x;
                    for ( x=0 ; x<8 ; x++ ) {
                        if ( tn_env_uservar[x][0] &&
                             tn_env_uservar[x][1] )
                            n += strlen(tn_env_uservar[x][0])
                                + strlen(tn_env_uservar[x][1]) + 2;
                    }
                    if ( tn_sfu ) {
                        /* For compatibility with Microsoft Telnet Server */
                        n += strlen(SFUTLNTVER) + strlen(SFUTLNTVER_VALUE) + 2;
                        n += strlen(SFUTLNTMODE) +
                          strlen(SFUTLNTMODE_VALUE) + 2;
                    }
#ifdef CK_SNDLOC
                    if ( tn_loc && tn_loc[0] )
                        n += strlen("LOCATION") + strlen(tn_loc) + 2;
#endif /* CK_SNDLOC */
                }
                else if (tn_sfu && !strcmp(varname,SFUTLNTVER))
                    n += strlen(SFUTLNTVER) + strlen(SFUTLNTVER_VALUE) + 2;
                else if (tn_sfu && !strcmp(varname,SFUTLNTMODE))
                    n += strlen(SFUTLNTMODE) + strlen(SFUTLNTMODE_VALUE) + 2;
#ifdef CK_SNDLOC
                else if ( tn_loc && tn_loc[0] && !strcmp(varname,"LOCATION"))
                    n += strlen("LOCATION") + strlen(tn_loc) + 2;
#endif /* CK_SNDLOC */
                else {
                    int x;
                    for ( x=0 ; x<8 ; x++ ) {
                        if ( tn_env_uservar[x][0] &&
                             tn_env_uservar[x][1] &&
                             !strcmp(varname,tn_env_uservar[x][0]))
                            n += strlen(tn_env_uservar[x][0])
                                + strlen(tn_env_uservar[x][1]) + 2;
                    }
                }
                break;
            }
            varname[0] = '\0';
            j = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :          /* VAR */
                    0
                   );
            break;
          case TEL_ENV_VALUE:           /* VALUE */
            /* Protocol Error */
            debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
            if (tn_deb || debses)
              tn_debug("TELNET Subnegotiation error - VALUE in SEND");
            return(0);
          case TEL_ENV_ESC:     /* ESC */
            if (++i >= len)
              break;
          default:
            if (j < 16 )
              varname[j++] = sb[i];
        }
    }
    reply = malloc(n + 16);              /* Leave room for IAC stuff */
    if (!reply) {
        debug(F100, "TELNET Subnegotiation error - malloc failed", "",0);
        if (tn_deb || debses)
          tn_debug("TELNET Subnegotiation error - malloc failed");

        /* Send a return packet with no variables so that the host */
        /* may continue with additional negotiations               */
        if (tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_IS,"",0) < 0)
          return(-1);
        return(0);
    }

    /* Now construct the real reply */

    n = 0;                              /* Start at beginning of buffer */
/*
  Pairs of <type> [VAR=0, VALUE=1, ESC=2, USERVAR=3] <value> "unterminated"
  follow here until done...
*/
    for (i = 0, j = 0, type = 0, varname[0]= '\0'; i <= len; i++) {
        switch (sb[i]) {
          case TEL_ENV_VAR:             /* VAR */
          case TEL_ENV_USERVAR:         /* USERVAR */
          case IAC:                     /* End of the list */
            switch (type) {
              case 0:                   /* Nothing in progress */
                /* If we get IAC only, then that means send all */
                /* VAR and USERVAR.                             */
                if (!(j == 0 && sb[i] == IAC))
                  break;
              case 1:                   /* VAR in progress */
                varname[j] = '\0';
                if (!varname[0]) {
                    /* Send All */
                    if (uu[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"USER");
                        reply[n+5] = TEL_ENV_VALUE;             /* VALUE */
                        strcpy(&reply[n+6],uu);
                        n += strlen(uu) + 4 + 2;
                    }
                    if (tn_env_job[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"JOB");
                        reply[n+4] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+5],tn_env_job);
                        n += strlen(tn_env_job) + 3 + 2;
                    }
                    if (tn_env_acct[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"ACCT");
                        reply[n+5] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+6],tn_env_acct);
                        n += strlen(tn_env_acct) + 4 + 2;
                    }
                    if (tn_env_prnt[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"PRINTER");
                        reply[n+8] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+9],tn_env_prnt);
                        n += strlen(tn_env_prnt) + 7 + 2;
                    }
                    if (tn_env_sys[0]) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"SYSTEMTYPE");
                        reply[n+11] = TEL_ENV_VALUE; /* VALUE */
                        strcpy(&reply[n+12],tn_env_sys);
                        n += strlen(tn_env_sys) + 10 + 2;
                    }
                    if (disp) {
                        reply[n] = TEL_ENV_VAR; /* VAR */
                        strcpy(&reply[n+1],"DISPLAY");
                        reply[n+8] = TEL_ENV_VALUE;     /* VALUE */
                        strcpy(&reply[n+9],disp);
                        n += strlen(disp) + 7 + 2;
                    }
                } else if (!strcmp(varname,"USER") && uu[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"USER");
                    reply[n+5] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+6],uu);
                    n += strlen(uu) + 4 + 2;
                } else if (!strcmp(varname,"JOB") && tn_env_job[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"JOB");
                    reply[n+4] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+5],tn_env_job);
                    n += strlen(tn_env_job) + 3 + 2;
                } else if (!strcmp(varname,"ACCT") && tn_env_acct[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"ACCT");
                    reply[n+5] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+6],tn_env_acct);
                    n += strlen(tn_env_acct) + 4 + 2;
                } else if (!strcmp(varname,"PRINTER") && tn_env_prnt[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"PRINTER");
                    reply[n+8] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+9],tn_env_prnt);
                    n += strlen(tn_env_prnt) + 7 + 2;
                } else if (!strcmp(varname,"SYSTEMTYPE") && tn_env_sys[0]) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"SYSTEMTYPE");
                    reply[n+11] = TEL_ENV_VALUE;        /* VALUE */
                    strcpy(&reply[n+12],tn_env_sys);
                    n += strlen(tn_env_sys) + 10 + 2;
                } else if (!strcmp(varname,"DISPLAY") && disp) {
                    reply[n] = TEL_ENV_VAR;     /* VAR */
                    strcpy(&reply[n+1],"DISPLAY");
                    reply[n+8] = TEL_ENV_VALUE; /* VALUE */
                    strcpy(&reply[n+9],disp);
                    n += strlen(disp) + 7 + 2;
                }
                  /* If we get IAC only, then that means send all */
                  /* VAR and USERVAR.                             */
                  if (!(j == 0 && sb[i] == IAC))
                      break;
            case 2:     /* USERVAR in progress */
                  varname[j] = '\0';
                  if (!varname[0]) {
                      /* Send All */
                      int x,y;
                      for ( x=0 ; x<8 ; x++ ) {
                          if ( tn_env_uservar[x][0] &&
                               tn_env_uservar[x][1] ) {
                              reply[n] = TEL_ENV_USERVAR;     /* VAR */
                              y = strlen(tn_env_uservar[x][0]);
                              strcpy(&reply[n+1],tn_env_uservar[x][0]);
                              reply[n+y+1] = TEL_ENV_VALUE; /* VALUE */
                              strcpy(&reply[n+y+2],tn_env_uservar[x][1]);
                              n += y+strlen(tn_env_uservar[x][1])+2;
                          }
                      }
                      if ( tn_sfu ) {
                          /* Compatibility with Microsoft Telnet Server */
                          reply[n] = TEL_ENV_USERVAR;     /* VAR */
                          strcpy(&reply[n+1],SFUTLNTVER);
                          reply[n+11] = TEL_ENV_VALUE; /* VALUE */
                          strcpy(&reply[n+12],SFUTLNTVER_VALUE);
                          n += strlen(SFUTLNTVER)+strlen(SFUTLNTVER_VALUE)+2;

                          reply[n] = TEL_ENV_USERVAR;     /* VAR */
                          strcpy(&reply[n+1],SFUTLNTMODE);
                          reply[n+12] = TEL_ENV_VALUE; /* VALUE */
                          strcpy(&reply[n+13],SFUTLNTMODE_VALUE);
                          n += strlen(SFUTLNTMODE)+strlen(SFUTLNTMODE_VALUE)+2;
                      }
                      if (tn_loc && tn_loc[0]) {
                          reply[n] = TEL_ENV_USERVAR;     /* VAR */
                          strcpy(&reply[n+1],"LOCATION");
                          reply[n+9] = TEL_ENV_VALUE; /* VALUE */
                          strcpy(&reply[n+10],tn_loc);
                          n += strlen("LOCATION") + strlen(tn_loc) + 2;
                      }
                  }  else if (tn_sfu && !strcmp(varname,SFUTLNTVER)) {
                      reply[n] = TEL_ENV_USERVAR;     /* VAR */
                      strcpy(&reply[n+1],SFUTLNTVER);
                      reply[n+11] = TEL_ENV_VALUE; /* VALUE */
                      strcpy(&reply[n+12],SFUTLNTVER_VALUE);
                      n += strlen(SFUTLNTVER) + strlen(SFUTLNTVER_VALUE) + 2;
                  }  else if (tn_sfu && !strcmp(varname,SFUTLNTMODE)) {
                      reply[n] = TEL_ENV_USERVAR;     /* VAR */
                      strcpy(&reply[n+1],SFUTLNTMODE);
                      reply[n+12] = TEL_ENV_VALUE; /* VALUE */
                      strcpy(&reply[n+13],SFUTLNTMODE_VALUE);
                      n += strlen(SFUTLNTMODE) + strlen(SFUTLNTMODE_VALUE) + 2;
                  }
#ifdef CK_SNDLOC
                  else if (tn_loc && tn_loc[0] && !strcmp(varname,"LOCATION")){
                      reply[n] = TEL_ENV_USERVAR;     /* VAR */
                      strcpy(&reply[n+1],"LOCATION");
                      reply[n+9] = TEL_ENV_VALUE; /* VALUE */
                      strcpy(&reply[n+10],tn_loc);
                      n += strlen("LOCATION") + strlen(tn_loc) + 2;
                  }
#endif /* CK_SNDLOC */
                  else {
                      int x,y;
                      for ( x=0 ; x<8 ; x++ ) {
                          if ( tn_env_uservar[x][0] &&
                               tn_env_uservar[x][1] &&
                               !strcmp(varname,tn_env_uservar[x][0])) {
                              reply[n] = TEL_ENV_USERVAR;     /* VAR */
                              y = strlen(tn_env_uservar[x][0]);
                              strcpy(&reply[n+1],tn_env_uservar[x][0]);
                              reply[n+y+1] = TEL_ENV_VALUE; /* VALUE */
                              strcpy(&reply[n+y+2],tn_env_uservar[x][1]);
                              n += y+strlen(tn_env_uservar[x][1])+2;
                          }
                      }
                  }
                break;
            }
            varname[0] = '\0';
            j = 0;
            type = (sb[i] == TEL_ENV_USERVAR ? 2 :      /* USERVAR */
                    sb[i] == TEL_ENV_VAR ? 1 :  /* VAR */
                    0
                   );
            break;
          case TEL_ENV_VALUE: /* VALUE */
            /* Protocol Error */
            debug(F100, "TELNET Subnegotiation error - VALUE in SEND", "",0);
            if (tn_deb || debses)
              tn_debug("TELNET Subnegotiation error - VALUE in SEND");
            return(0);  /* Was -1 but that would be taken as */
                        /* an I/O error, so absorb it and go on. */
          case TEL_ENV_ESC:     /* ESC */
            /* Not sure what this for.  Quote next character? */
            break;
          default:
            varname[j++] = sb[i];
        }
    }
    if (tn_ssbopt(TELOPT_NEWENVIRON,TELQUAL_IS,reply,n) < 0) {
        free(reply);
        return(-1);
    }
    free(reply);
    return(1);
}
#endif /* CK_ENVIRONMENT */

/* Telnet send terminal type */
/* Returns -1 on error, 0 if nothing happens, 1 if type sent successfully */

int
tn_sttyp() {                            /* Send telnet terminal type. */
    char *ttn;                          /* Name of terminal type. */
#ifdef OS2
    static int alias = -1;              /* which alias are we using ? */
    int settype = 0;
#endif /* OS2 */
    int i, rc;                          /* Worker. */
    int tntermflg = 0;

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_TTYPE)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    ttn = NULL;

#ifndef NOTERM
#ifdef OS2
    if (!tn_term) {
        if (ttnum == -1) {
            ttnum = tt_type;
            settype = 0;
            alias = -1;
        } else if (ttnumend) {
            ttnumend = 0;
            settype = 0;
        } else {
            if (tt_info[tt_type].x_aliases[++alias] == NULL)  {
                if (--tt_type < 0)
                  tt_type = max_tt;
                if (ttnum == tt_type)
                  ttnumend = 1;
                settype = 1;
                alias = -1;
            }
        }
        if (tt_type >= 0 && tt_type <= max_tt) {
            if (alias == -1)
              ttn = tt_info[tt_type].x_name;
            else
              ttn = tt_info[tt_type].x_aliases[alias];
        } else
          ttn = NULL;
    }
    else settype = 0;
#endif /* OS2 */
#endif /* NOTERM */

    if (tn_term) {                      /* Terminal type override? */
        debug(F110,"tn_sttyp",tn_term,0);
        if (*tn_term) {
            ttn = tn_term;
            tntermflg = 1;
        }
    } else debug(F100,"tn_sttyp no term override","",0);

#ifndef datageneral
    if (!ttn) {                         /* If no override, */
        ttn = getenv("TERM");           /* get it from the environment. */
    }
#endif /* datageneral */
    if ((ttn == ((char *)0)) || ((int)strlen(ttn) >= TSBUFSIZ))
      ttn = "UNKNOWN";
    sb_out[0] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[1] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[2] = TELOPT_TTYPE;               /* Terminal Type */
    sb_out[3] = (CHAR) 0;                   /* Is... */
    for (i = 4; *ttn; ttn++,i++) {      /* Copy and uppercase it */
#ifdef VMS
        if (!tntermflg && *ttn == '-' &&
            (!strcmp(ttn,"-80") || !strcmp(ttn,"-132")))
          break;
        else
#endif /* VMS */
          sb_out[i] = (char) ((!tntermflg && islower(*ttn)) ?
                              toupper(*ttn) :
                              *ttn);
    }
    ttn = (char *)sb_out;                   /* Point back to beginning */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sb_out[i] = '\0';                   /* For debugging */
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB ",
                 TELOPT(TELOPT_TTYPE)," IS ",(char *)sb_out+4," IAC SE",
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);       /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
#ifndef NOTERM
#ifdef OS2
    if (settype)
        settermtype(tt_type,0);
    else {
        ipadl25();
        VscrnIsDirty(VTERM);
    }
#endif /* OS2 */
#endif /* NOTERM */
    return(1);
}

#ifdef CK_ENVIRONMENT
#ifdef CK_XDISPLOC

/* Telnet send xdisplay location */
/* Returns -1 on error, 0 if nothing happens, 1 if type sent successfully */

int
tn_sxdisploc() {                        /* Send telnet X display location. */
    char * disp=NULL;
    int i,rc;

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_XDISPLOC)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

#ifdef CK_FORWARD_X
    if (TELOPT_U(TELOPT_FORWARD_X)) {
        disp = NULL;
    } else
#endif /* CK_FORWARD_X */
        disp = (char *)tn_get_display();
    debug(F110,"tn_sxdisploc",disp,0);

    if (!disp) {
        /* Can't do both, send WONT */
        if (tn_sopt(WONT,TELOPT_XDISPLOC) < 0)
            return(-1);
        TELOPT_UNANSWERED_WONT(TELOPT_XDISPLOC) = 1;
        return(0);
    }

    sb_out[0] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[1] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[2] = TELOPT_XDISPLOC;            /* X-Display Location */
    sb_out[3] = (CHAR) 0;                   /* Is... */
    for (i = 4; *disp; disp++,i++) {      /* Copy and uppercase it */
        sb_out[i] = (char) *disp;
    }
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        sb_out[i] = '\0';                   /* For debugging */
        ckmakxmsg( tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_XDISPLOC),
                  " IS ",(char *)sb_out+4," IAC SE",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    return(1);
}
#endif /* CK_XDISPLOC */
#endif /* CK_ENVIRONMENT */

#ifdef CK_FORWARD_X
int
tn_sndfwdx() {                          /* Send Fwd X Screen number to host */
    unsigned char screen = 0;
    char * disp;
    int i,rc;

    /* if (!IS_TELNET()) return(0); */

    if (!TELOPT_U(TELOPT_FORWARD_X)) return(0);
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    /*
     * The format of the DISPLAY variable is [<host>:]<display>[.<screen>]
     * where <host> is an optional DNS name or ip address with a default of
     * the localhost; the screen defaults to 0
     */

    disp = (char *)tn_get_display();
    if (disp) {
        int colon,dot;
        colon = ckindex(":",disp,0,0,1);
        dot   = ckindex(".",&disp[colon],0,0,1);

        if ( dot ) {
            screen = atoi(&disp[colon+dot]);
        }
    } else {
        screen = 0;
    }

    i = 0;
    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_FORWARD_X;           /* Forward X */
    sb_out[i++] = FWDX_SCREEN;                /* Screen */
    sb_out[i++] = screen;
    if ( screen == IAC )
        sb_out[i++] = IAC;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg( tn_msg_out,TN_MSG_LEN,
                   "TELNET SENT SB ",TELOPT(TELOPT_FORWARD_X),
                   " SCREEN ",ckctox(screen,1)," IAC SE",
                   NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    return(0);
}
#endif /* CK_FORWARD_X */

#ifdef CK_SNDLOC
int
tn_sndloc() {                           /* Send location. */
    int i,rc;                              /* Worker. */
    char *ttloc;

    /* if (!IS_TELNET()) return(0); */

    if (!TELOPT_ME(TELOPT_SNDLOC)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    ttloc = (tn_loc ? tn_loc : "");     /* In case we are being called even */
                                        /* though there is no location. */
    sb_out[0] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[1] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[2] = TELOPT_SNDLOC;              /* Location */
    for (i = 3; *ttloc && i < TSBUFSIZ; ttloc++,i++) /* Copy it */
      sb_out[i] = (char) *ttloc;
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_SNDLOC)," ",(char *)sb_out+3,
                  " IAC SE", NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    sb_out[i-2] = '\0';                     /* For debugging */
    return(0);
}
#endif /* CK_SNDLOC */

#ifdef CK_NAWS                  /*  NAWS = Negotiate About Window Size  */
int
tn_snaws() {                    /*  Send terminal width and height, RFC 1073 */
#ifndef NOLOCAL
    CHAR sb_out[24];            /*  multiple threads */
    int i = 0,rc;
#ifdef OS2
    int x = VscrnGetWidth(VTERM),
    y = VscrnGetHeight(VTERM) - (tt_status[VTERM] ? 1 : 0);
#else /* OS2 */
    int x = tt_cols, y = tt_rows;
#endif /* OS2 */

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!TELOPT_ME(TELOPT_NAWS)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x == TELOPT_SB(TELOPT_NAWS).naws.x && /* Only send if changed */
        y == TELOPT_SB(TELOPT_NAWS).naws.y
        )
      return(0);
    TELOPT_SB(TELOPT_NAWS).naws.x = x;  /* Remember the size     */
    TELOPT_SB(TELOPT_NAWS).naws.y = y;

    sb_out[i++] = (CHAR) IAC;               /* Send the subnegotiation */
    sb_out[i++] = (CHAR) SB;
    sb_out[i++] = TELOPT_NAWS;
    sb_out[i++] = (CHAR) (x >> 8) & 0xff;
    if ((CHAR) sb_out[i-1] == (CHAR) IAC)   /* IAC in data must be doubled */
      sb_out[i++] = (CHAR) IAC;
    sb_out[i++] = (CHAR) (x & 0xff);
    if ((CHAR) sb_out[i-1] == (CHAR) IAC)
      sb_out[i++] = (CHAR) IAC;
    sb_out[i++] = (CHAR) (y >> 8) & 0xff;
    if ((CHAR) sb_out[i-1] == (CHAR) IAC)
      sb_out[i++] = (CHAR) IAC;
    sb_out[i++] = (CHAR) (y & 0xff);
    if ((CHAR) sb_out[i-1] == (CHAR) IAC)
      sb_out[i++] = (CHAR) IAC;
    sb_out[i++] = (CHAR) IAC;
    sb_out[i++] = (CHAR) SE;
#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,"TELNET SENT SB NAWS ",
                  ckitoa(x)," ",ckitoa(y)," IAC SE",
                   NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
#endif /* NOLOCAL */
    return (0);
}
#endif /* CK_NAWS */

#ifdef TN_COMPORT
static char * tnc_signature = NULL;
static int tnc_ls_mask = 0;
static int tnc_ls = 0;
static int tnc_ms_mask = 255;
static int tnc_ms = 0;
static int tnc_oflow = 0;
static int tnc_iflow = 0;
static int tnc_bps = 0;
static int tnc_datasize = 0;
static int tnc_parity = 0;
static int tnc_stopbit = 0;
static int tnc_break = 0;
static int tnc_dtr = 0;
static int tnc_rts = 0;
static int tnc_suspend_xmit = 0;
static int tnc_bps_index = -1;

int
#ifdef CK_ANSIC
tnc_init(void)
#else /* CK_ANSIC */
tnc_init()
#endif /* CK_ANSIC */
/* tnc_init */ {
    debug(F100,"tnc_init","",0);

    /* if (!IS_TELNET()) return(0); */

    if (tnc_signature) {
        free(tnc_signature);
        tnc_signature = NULL;
    }
    tnc_ls_mask = 0;
    tnc_ls = 0;
    tnc_ms_mask = 255;
    tnc_ms = 0;
    tnc_oflow = 0;
    tnc_iflow = 0;
    tnc_bps = 0;
    tnc_datasize = 0;
    tnc_parity = 0;
    tnc_stopbit = 0;
    tnc_break = 0;
    tnc_dtr = 0;
    tnc_rts = 0;
    tnc_suspend_xmit = 0;
    tnc_bps_index = -1;
    return(0);
}

int
#ifdef CK_ANSIC
tn_sndcomport(void)
#else /* CK_ANSIC */
tn_sndcomport()
#endif /* CK_ANSIC */
/* tn_sndcomport */ {
    int baud, datasize, parity, stopsize, oflow, iflow;
    CONST char * signature;

    /* if (!IS_TELNET()) return(0); */

    debug(F100,"tnc_sndcomport","",0);
    signature = tnc_get_signature();
    baud = tnc_get_baud();
    datasize = tnc_get_datasize();
    parity = tnc_get_parity();
    stopsize = tnc_get_stopsize();
    oflow = tnc_get_oflow();
    iflow = tnc_get_iflow();
    tnc_set_ls_mask(255);
    tnc_set_ms_mask(255);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_wait(CHAR * msg, int ms)
#else /* CK_ANSIC */
tnc_wait(msg, ms) CHAR * msg; int ms;
#endif /* CK_ANSIC */
/* tnc_wait */ {
    int rc, tn_wait_save = tn_wait_flg;

    /* if (!IS_TELNET()) return(0); */

    debug(F111,"tnc_wait","begin",ms);
    if ( ms )
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_ms = 1;
    else
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 1;
    tn_wait_flg = 1;
    rc = tn_wait((char *)msg);
    tn_push();
    debug(F110,"tnc_wait","end",0);
    tn_wait_flg = tn_wait_save;
    return(rc);
}

/* Returns -1 on error, 0 on success */
/* In order for this code to work, sb[len] == IAC          */

int
#ifdef CK_ANSIC
tnc_tn_sb(CHAR * sb, int len)
#else
tnc_tn_sb(sb, len) CHAR * sb; int len;
#endif /* CK_ANSIC */
/* tnc_tn_sb */ {
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);
    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

    if (!sb) return(-1);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    debug(F111,"tnc_tn_sb","sb[0]",sb[0]);
    debug(F111,"tnc_tn_sb","len",len);

    switch (sb[0]) {
      case TNC_C2S_SIGNATURE:
      case TNC_S2C_SIGNATURE:
        debug(F111,"tnc_tn_sb","signature",len);
        if (len == 1) {
            tnc_send_signature("Kermit Telnet Com Port Option");
        } else {
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            if (tnc_signature)
              free(tnc_signature);
            tnc_signature = malloc(len);
            if (tnc_signature) {
                memcpy(tnc_signature,&sb[1],len-1);
                tnc_signature[len-1] = '\0';
            }
        }
        break;

      case TNC_C2S_SET_BAUDRATE:
      case TNC_S2C_SET_BAUDRATE: {
          long baudrate;
          char * br = (char *)&baudrate;
          TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
          if (len == 2) {
            /* Actual behavior of the Access Server... */
            debug(F111,"tnc_tn_sb","baudrate index",sb[1]);
            tnc_bps_index = 1;
            switch (sb[1]) {
            case TNC_BPS_300:
              tnc_bps = 300;
              break;
            case TNC_BPS_600:
              tnc_bps = 600;
              break;
            case TNC_BPS_1200:
              tnc_bps = 1200;
              break;
            case TNC_BPS_2400:
              tnc_bps = 2400;
              break;
            case TNC_BPS_4800:
              tnc_bps = 4800;
              break;
            case TNC_BPS_9600:
              tnc_bps = 9600;
              break;
            case TNC_BPS_14400:
              tnc_bps = 14400;
              break;
            case TNC_BPS_19200:
              tnc_bps = 19200;
              break;
            case TNC_BPS_28800:
              tnc_bps = 28800;
              break;
            case TNC_BPS_38400:
              tnc_bps = 38400;
              break;
            case TNC_BPS_57600:
              tnc_bps = 57600;
              break;
            case TNC_BPS_115200:
              tnc_bps = 115200;
              break;
            case TNC_BPS_230400:
              tnc_bps = 230400;
              break;
            case TNC_BPS_460800:
              tnc_bps = 460800;
              break;
            default:
              tnc_bps = -1;
            }
          } else if (len == 5) {
            /* This section attempts to follow RFC 2217 */
              tnc_bps_index = 0;
              br[0] = sb[1];
              br[1] = sb[2];
              br[2] = sb[3];
              br[3] = sb[4];
#ifdef datageneral
              /* AOS/VS doesn't have ntohl() but MV's are big-endian */
              tnc_bps = baudrate;
#else
              tnc_bps = ntohl(baudrate);
#endif /* datageneral */
              debug(F111,"tnc_tn_sb","baudrate rfc",tnc_bps);
          } else {
              debug(F111,"tnc_tn_sb","baudrate invalid len",len);
              return(-1);
          }
          break;
      }
      case TNC_C2S_SET_DATASIZE:
      case TNC_S2C_SET_DATASIZE:
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
        if (len < 2)
          return(-1);
        tnc_datasize = sb[1];
        debug(F111,"tnc_tn_sb","datasize",sb[1]);
        break;

      case TNC_C2S_SET_PARITY:
      case TNC_S2C_SET_PARITY:
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
        if (len < 2)
          return(-1);
        tnc_parity = sb[1];
        debug(F111,"tnc_tn_sb","parity",sb[1]);
        break;

      case TNC_C2S_SET_STOPSIZE:
      case TNC_S2C_SET_STOPSIZE:
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
        if (len < 2)
          return(-1);
        tnc_stopbit = sb[1];
        debug(F111,"tnc_tn_sb","stopsize",sb[1]);
        break;

      case TNC_C2S_SET_CONTROL:
      case TNC_S2C_SET_CONTROL:
        if (len < 2) {
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            return(-1);
        }

#ifdef COMMENT
        /* This line should be removed when testing is complete. */
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
#endif /* COMMENT */

        switch ( sb[1] ) {
          case TNC_CTL_OFLOW_REQUEST:
            /* determine local outbound flow control and send to peer */
	    /* Cisco IOS returns 0 (TNC_CTL_OFLOW_REQUEST) when attempting */
	    /* to set the inbound flow control if it is not supported      */
	    /* separately from outbound flow control.  So must reset       */
	    /* wait for sb flag.                                           */
            debug(F110,"tnc_tn_sb","oflow request",0);
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            break;
          case TNC_CTL_OFLOW_NONE:
          case TNC_CTL_OFLOW_XON_XOFF:
          case TNC_CTL_OFLOW_RTS_CTS:
          case TNC_CTL_OFLOW_DCD:
          case TNC_CTL_OFLOW_DSR:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_oflow = sb[1];
            debug(F111,"tnc_tn_sb","oflow",sb[1]);
            break;
          case TNC_CTL_BREAK_REQUEST:
            /* determine local break state and send to peer */
            debug(F110,"tnc_tn_sb","break request",0);
            break;
          case TNC_CTL_BREAK_ON:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_break = 1;
            debug(F110,"tnc_tn_sb","break on",0);
            break;

          case TNC_CTL_BREAK_OFF:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_break = 0;
            debug(F110,"tnc_tn_sb","break off",0);
            break;

          case TNC_CTL_DTR_REQUEST:
            /* determine local dtr state and send to peer */
            debug(F110,"tnc_tn_sb","dtr request",0);
            break;

          case TNC_CTL_DTR_ON:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_dtr = 1;
            debug(F110,"tnc_tn_sb","dtr on",0);
            break;

          case TNC_CTL_DTR_OFF:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_dtr = 0;
            debug(F110,"tnc_tn_sb","dtr off",0);
            break;

          case TNC_CTL_RTS_REQUEST:
            /* determine local rts state and send to peer */
            debug(F110,"tnc_tn_sb","rts request",0);
            break;

          case TNC_CTL_RTS_ON:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_rts = 1;
            debug(F110,"tnc_tn_sb","rts on",0);
            break;

          case TNC_CTL_RTS_OFF:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_rts = 0;
            debug(F110,"tnc_tn_sb","rts off",0);
            break;

          case TNC_CTL_IFLOW_REQUEST:
            /* determine local inbound flow control and send to peer */
            debug(F110,"tnc_tn_sb","iflow request",0);
            break;

          case TNC_CTL_IFLOW_NONE:
          case TNC_CTL_IFLOW_XON_XOFF:
          case TNC_CTL_IFLOW_RTS_CTS:
          case TNC_CTL_IFLOW_DTR:
            TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
            tnc_iflow = sb[1];
            debug(F111,"tnc_tn_sb","iflow",sb[1]);
            break;
          default:
            return(-1);
        }
        break;

      case TNC_C2S_NOTIFY_LINESTATE:
      case TNC_S2C_SEND_LS:
        if (len < 2)
          return(-1);
        tnc_ls = sb[1];
        debug(F111,"tnc_tn_sb","linestate",sb[1]);
        if (tn_deb || debses) {
            if (tnc_ls & TNC_MS_DATA_READY )
              tn_debug("  ComPort Linestate Data Ready");
            if (tnc_ls & TNC_MS_OVERRUN_ERROR )
              tn_debug("  ComPort Linestate Overrun Error");
            if (tnc_ls & TNC_MS_PARITY_ERROR )
              tn_debug("  ComPort Linestate Parity Error");
            if (tnc_ls & TNC_MS_FRAME_ERROR )
              tn_debug("  ComPort Linestate Framing Error");
            if (tnc_ls & TNC_MS_BREAK_ERROR )
              tn_debug("  ComPort Linestate Break Detect Error");
            if (tnc_ls & TNC_MS_HR_EMPTY )
              tn_debug("  ComPort Linestate Holding Register Empty");
            if (tnc_ls & TNC_MS_SR_EMPTY )
              tn_debug("  ComPort Linestate Shift Register Empty");
            if (tnc_ls & TNC_MS_TIMEOUT_ERROR )
              tn_debug("  ComPort Linestate Timeout Error");
        }
        break;

      case TNC_C2S_NOTIFY_MODEMSTATE:
      case TNC_S2C_SEND_MS:
        TELOPT_SB(TELOPT_COMPORT).comport.wait_for_ms = 0;
        if (len < 2)
          return(-1);
        tnc_ms = sb[1];
        debug(F111,"tnc_tn_sb","modemstate",sb[1]);
        if (tn_deb || debses) {
            if (tnc_ms & TNC_MS_CTS_DELTA )
              tn_debug("  ComPort Modemstate CTS State Change");
            if (tnc_ms & TNC_MS_DSR_DELTA )
              tn_debug("  ComPort Modemstate DSR State Change");
            if (tnc_ms & TNC_MS_EDGE_RING )
              tn_debug("  ComPort Modemstate Trailing Edge Ring Detector On");
            else
              tn_debug("  ComPort Modemstate Trailing Edge Ring Detector Off");
            if (tnc_ms & TNC_MS_RLSD_DELTA )
              tn_debug("  ComPort Modemstate RLSD State Change");
            if (tnc_ms & TNC_MS_CTS_SIG )
              tn_debug("  ComPort Modemstate CTS Signal On");
            else
              tn_debug("  ComPort Modemstate CTS Signal Off");
            if (tnc_ms & TNC_MS_DSR_SIG )
              tn_debug("  ComPort Modemstate DSR Signal On");
            else
              tn_debug("  ComPort Modemstate DSR Signal Off");
            if (tnc_ms & TNC_MS_RI_SIG )
              tn_debug("  ComPort Modemstate Ring Indicator On");
            else
              tn_debug("  ComPort Modemstate Ring Indicator Off");
            if (tnc_ms & TNC_MS_RLSD_SIG )
              tn_debug("  ComPort Modemstate RLSD Signal On");
            else
              tn_debug("  ComPort Modemstate RLSD Signal Off");
        }
        break;

      case TNC_C2S_FLOW_SUSPEND:
      case TNC_S2C_FLOW_SUSPEND:
        debug(F110,"tnc_tn_sb","flow suspend",0);
        tnc_suspend_xmit = 1;
        break;

      case TNC_C2S_FLOW_RESUME:
      case TNC_S2C_FLOW_RESUME:
        debug(F110,"tnc_tn_sb","flow resume",0);
        tnc_suspend_xmit = 0;
        break;

      case TNC_C2S_SET_LS_MASK:
      case TNC_S2C_SET_LS_MASK:
          TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
          if (len < 2)
          return(-1);
        debug(F111,"tnc_tn_sb","linestate mask",sb[1]);
        tnc_ls_mask = sb[1];
        break;

      case TNC_C2S_SET_MS_MASK:
      case TNC_S2C_SET_MS_MASK:
          TELOPT_SB(TELOPT_COMPORT).comport.wait_for_sb = 0;
          if (len < 2)
          return(-1);
        debug(F111,"tnc_tn_sb","modemstate mask",sb[1]);
        tnc_ls_mask = sb[1];
        break;

      case TNC_C2S_PURGE:
      case TNC_S2C_PURGE:
        if (len < 2)
          return(-1);
        debug(F111,"tnc_tn_sb","purge",sb[1]);
        switch ( sb[1] ) {
          case TNC_PURGE_RECEIVE:
          case TNC_PURGE_TRANSMIT:
          case TNC_PURGE_BOTH:
            /* purge local buffers */
            break;
          default:
            return(-1);
        }
        break;
      default:
        return(-1);
    }
    return(0);
}

CONST char *
#ifdef CK_ANSIC
tnc_get_signature(void)
#else /* CK_ANSIC */
tnc_get_signature()
#endif /* CK_ANSIC */
/* tnc_get_signature */ {
    /* send IAC SB COM-PORT SIGNATURE IAC SE */
    /* wait for response */
    int i = 0, rc;

    if (ttnet != NET_TCPB) return(NULL);
    if (ttnproto != NP_TELNET) return(NULL);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(NULL);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(NULL);
    }
#endif /* CK_SSL */

    if ( tnc_signature )
        return(tnc_signature);

    sb_out[i++] = (CHAR) IAC;           /* I Am a Command */
    sb_out[i++] = (CHAR) SB;            /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;               /* ComPort */
    sb_out[i++] = TNC_C2S_SIGNATURE;    /* Signature */
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakmsg(tn_msg_out,TN_MSG_LEN,
                 "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                 " SIGNATURE IAC SE", NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(NULL);

    if (tnc_wait((CHAR *)"comport signature request",0) < 0) {
        tn_push();
        return(NULL);
    }
    debug(F110,"tnc_get_signature",tnc_signature,0);
    return(tnc_signature);
}

int
#ifdef CK_ANSIC
tnc_send_signature(char * signature)
#else /* CK_ANSIC */
tnc_send_signature(signature) char * signature;
#endif /* CK_ANSIC */
/* tnc_send_signature */ {
    /* send IAC SB COM-PORT SIGNATURE <text> IAC SE */
    int i = 0, j = 0, rc;

    debug(F110,"tnc_send_signature",signature,0);

    if (!signature || !signature[0])
      return(0);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SIGNATURE;      /* Signature */
    for (; signature[j]; i++,j++)
      sb_out[i] = signature[j];
    sb_out[i++] = (CHAR) IAC;               /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SIGNATURE ", signature, " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_set_baud( long baud )
#else /* CK_ANSIC */
tnc_set_baud(baud) long baud;
#endif /* CK_ANSIC */
/* tnc_set_baud */ {
    /* send IAC SB COM-PORT SET-BAUD <value(4)> IAC SE  */
    /* wait for response */
    /* 0 is used to request the current baud rate and */
    /* may not be sent by this func */
    /* return new host value */

    /*
     *   the above comes from the RFC.  But that is not what I am seeing
     *   instead I appear to be seeing to following:
     *
     *      Value               Baud
     *          1               ?
     *          2               ?
     *          3               300
     *          4               600
     *          5               1200
     *          6               2400
     *          7               4800      ?
     *          8               9600
     *          9                         ?
     *          10              19200     ?
     *          11                        ?
     *          12              38400
     *          13              57600     ?
     *          14              115200
     *          15              230400    ?
     *          16              460800    ?
     */

    int i = 0, rc;
#ifdef datageneral
    /* AOS/VS doesn't have htonl() but MV's are big-endian */
    long net_baud = baud;
#else
    long net_baud = htonl(baud);
#endif /* datageneral */
    CHAR b;

    debug(F111,"tnc_set_baud","begin",baud);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (baud <= 0)
        return(0);

    if ( net_baud != 0 && net_baud == tnc_bps)
        return(tnc_bps);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_BAUDRATE;   /* Set Baud Rate */

    if (tnc_bps_index) {
        /* IOS Access Server */
        if (baud <= 300)
            b = TNC_BPS_300;
        else if (baud <= 600)
            b = TNC_BPS_600;
        else if (baud <= 1200)
            b = TNC_BPS_1200;
        else if (baud <= 2400)
            b = TNC_BPS_2400;
        else if (baud <= 4800)
            b = TNC_BPS_4800;
        else if (baud <= 9600)
            b = TNC_BPS_9600;
        else if (baud <= 14400)
            b = TNC_BPS_14400;
        else if (baud <= 19200)
            b = TNC_BPS_19200;
        else if (baud <= 28800)
            b = TNC_BPS_28800;
        else if (baud <= 38400)
            b = TNC_BPS_38400;
        else if (baud <= 57600)
            b = TNC_BPS_57600;
        else if (baud <= 115200)
            b = TNC_BPS_115200;
        else if (baud <= 230400)
            b = TNC_BPS_230400;
        else
            b = TNC_BPS_460800;
        sb_out[i++] = b;
    } else {
        /* RFC 2217 */
        sb_out[i++] = ((char *)&net_baud)[0];
        sb_out[i++] = ((char *)&net_baud)[1];
        sb_out[i++] = ((char *)&net_baud)[2];
        sb_out[i++] = ((char *)&net_baud)[3];
    }
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-BAUD-RATE ", ckltoa(baud)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */

#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set baud rate",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_baud","end",tnc_bps);
    return(tnc_bps);
}

int
#ifdef CK_ANSIC
tnc_get_baud(void)
#else /* CK_ANSIC */
tnc_get_baud()
#endif /* CK_ANSIC */
/* tnc_get_baud */ {
    /* send IAC SB COM-PORT SET-BAUD <value(4)=0> IAC SE  */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_baud","begin",0);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_BAUDRATE;   /* Set Baud Rate */

    if (tnc_bps_index > 0) {
        /* Access Server */
        sb_out[i++] = 0;
    } else {
        /* RFC 2217 */
        sb_out[i++] = 0;
        sb_out[i++] = 0;
        sb_out[i++] = 0;
        sb_out[i++] = 0;
    }
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-BAUD-RATE ", ckltoa(0)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get baud rate",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_baud","end",tnc_bps);
    return(tnc_bps);
}

int
#ifdef CK_ANSIC
tnc_set_datasize(int datasize)
#else /* CK_ANSIC */
tnc_set_datasize(datasize) int datasize;
#endif /* CK_ANSIC */
/* tnc_set_datasize */ {
    /* IAC SB COM-PORT SET_DATASIZE <value(1)> IAC SE */
    /* Valid <value>s are 5 through 8 */
    /* Wait for response */
    /* return new host value */

    int i = 0, rc;

    debug(F111,"tnc_set_datasize","begin",datasize);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( !(datasize >= 5 && datasize <= 8) )
        return(0);

    if ( datasize  != 0 &&  datasize == tnc_datasize )
        return(tnc_datasize);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_DATASIZE;   /* Set DataSize */
    sb_out[i++] = (unsigned char)(datasize & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-DATASIZE ", ckitoa(datasize)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set datasize",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_datasize","end",tnc_datasize);
    return(tnc_datasize);
}

int
#ifdef CK_ANSIC
tnc_get_datasize(void)
#else /* CK_ANSIC */
tnc_get_datasize()
#endif /* CK_ANSIC */
/* tnc_get_datasize */ {
    /* IAC SB COM-PORT SET_DATASIZE <value(1)=0> IAC SE */
    /* Wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_datasize","begin",0);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_DATASIZE;   /* Set DataSize */
    sb_out[i++] = 0;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-DATASIZE ", ckltoa(0)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get datasize",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_datasize","end",tnc_datasize);
    return(tnc_datasize);
}

int
#ifdef CK_ANSIC
tnc_set_parity(int parity)
#else /* CK_ANSIC */
tnc_set_parity(parity) int parity;
#endif /* CK_ANSIC */
/* tnc_set_parity */ {
    /* IAC SB COM-PORT SET_PARITY <value(1)> IAC SE */
    /*        Value     Parity
     *          1       None
     *          2       Odd
     *          3       Even
     *          4       Mark
     *          5       Space
     */
    /* Wait for response.  Return new host value. */
    int i = 0, rc;

    debug(F110,"tnc_set_parity","begin",parity);

    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( !(parity >= 1 && parity <= 5) )
        return(0);

    if ( parity != 0 && parity == tnc_parity )
        return(tnc_parity);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_PARITY;     /* Set Parity */
    sb_out[i++] = (unsigned char)(parity & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-PARITY ", ckitoa(parity)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set parity",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_parity","end",tnc_parity);
    return(tnc_parity);
}

int
#ifdef CK_ANSIC
tnc_get_parity(void)
#else /* CK_ANSIC */
tnc_get_parity()
#endif /* CK_ANSIC */
/* tnc_get_parity */ {
    /* IAC SB COM-PORT SET_PARITY <value(1)=0> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_parity","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_PARITY;     /* Set Parity */
    sb_out[i++] = 0;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-PARITY ", ckitoa(0)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get parity",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_parity","end",tnc_parity);
    return(tnc_parity);
}

int
#ifdef CK_ANSIC
tnc_set_stopsize(int stopsize)
#else /* CK_ANSIC */
tnc_set_stopsize(stopsize) int stopsize;
#endif /* CK_ANSIC */
/* tnc_set_stopsize */ {
    /* IAC SB COM-PORT SET_STOPSIZE <value(1)> IAC SE */
    /*        Value     Stop Bit Size
     *          1       1
     *          2       2
     *          3       1.5
     */
    /* Wait for response.  Return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_stopsize","begin",stopsize);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (!(stopsize >= 1 && stopsize <= 3) )
      return(0);

    if ( stopsize != 0 && stopsize == tnc_stopbit )
        return(tnc_stopbit);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_STOPSIZE;   /* Set Stop Bits */
    sb_out[i++] = (unsigned char)(stopsize & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-STOPSIZE ", ckitoa(stopsize)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set stopsize",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_stopsize","end",tnc_stopbit);
    return(tnc_stopbit);
}

int
#ifdef CK_ANSIC
tnc_get_stopsize(void)
#else /* CK_ANSIC */
tnc_get_stopsize()
#endif /* CK_ANSIC */
/* tnc_get_stopsize */ {
    /* IAC SB COM-PORT SET_STOPSIZE <value(1)=0> IAC SE */
    /* Wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_stopsize","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_STOPSIZE;   /* Set Stop Bits */
    sb_out[i++] = 0;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-STOPSIZE ", ckitoa(0)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set stopsize",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_stopsize","end",tnc_stopbit);
    return(tnc_stopbit);
}

int
#ifdef CK_ANSIC
tnc_set_oflow(int control)
#else /* CK_ANSIC */
tnc_set_oflow(control) int control;
#endif /* CK_ANSIC */
/* tnc_set_oflow */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)> IAC SE */
    /*        Value     Flow Control
     *          1       No Flow Control
     *          2       Xon/Xoff
     *          3       Rts/Cts
     *         17       DCD
     *         19       DSR
     */
    /* wait for response, return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_oflow","begin",control);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (control != 1 && control != 2 && control != 3 &&
        control != 17 && control != 19)
      return(0);

    if ( control != 0 && control == tnc_oflow )
        return(tnc_oflow);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = (unsigned char)(control & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ", ckitoa(control)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set outbound flow control",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_oflow","end",tnc_oflow);
    return(tnc_oflow);
}

int
#ifdef CK_ANSIC
tnc_get_oflow(void)
#else /* CK_ANSIC */
tnc_get_oflow()
#endif /* CK_ANSIC */
/* tnc_get_oflow */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)=0> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_oflow","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = TNC_CTL_OFLOW_REQUEST;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                   ckitoa(TNC_CTL_OFLOW_REQUEST),
                   " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get outbound flow control",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_oflow","end",tnc_oflow);
    return(tnc_oflow);
}

int
#ifdef CK_ANSIC
tnc_set_iflow(int control)
#else /* CK_ANSIC */
tnc_set_iflow(control) int control;
#endif /* CK_ANSIC */
/* tnc_set_iflow */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)> IAC SE */
    /*        Value     Flow Control
     *         14       No Flow Control
     *         15       Xon/Xoff
     *         16       Rts/Cts
     *         18       DTR
     */
    /* wait for response, return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_iflow","begin",control);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if (control != 14 && control != 15 && control != 16 && control != 18)
      return(0);

    if ( control != 0 && control == tnc_iflow )
        return(tnc_iflow);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = (unsigned char)(control & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ", ckitoa(control)," IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    if (tnc_wait((CHAR *)"comport set inbound flow control",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_iflow","end",tnc_iflow);
    return(tnc_iflow);
}

int
#ifdef CK_ANSIC
tnc_get_iflow(void)
#else /* CK_ANSIC */
tnc_get_iflow()
#endif /* CK_ANSIC */
/* tnc_get_iflow */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)=13> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_iflow","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = TNC_CTL_IFLOW_REQUEST;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  ckitoa(TNC_CTL_IFLOW_REQUEST),
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get inbound flow control",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_iflow","end",tnc_iflow);
    return(tnc_iflow);
}

int
#ifdef CK_ANSIC
tnc_set_break_state(int onoff)
#else /* CK_ANSIC */
tnc_set_break_state(onoff) int onoff;
#endif /* CK_ANSIC */
/* tnc_set_break_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)> IAC SE */
    /*        Value     Break State
     *          5       On
     *          6       Off
     */
    /* wait for response, return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_break_state","begin",onoff);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( onoff != 0 && onoff == tnc_break )
        return(tnc_break);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = onoff ?
      TNC_CTL_BREAK_ON : TNC_CTL_BREAK_OFF;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  onoff ? "BREAK-ON" : "BREAK-OFF",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport set break state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_break_state","end",tnc_break);
    return(tnc_break);
}

int
#ifdef CK_ANSIC
tnc_get_break_state(void)
#else /* CK_ANSIC */
tnc_get_break_state()
#endif /* CK_ANSIC */
/* tnc_get_break_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)=4> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_break_state","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = TNC_CTL_BREAK_REQUEST;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  "BREAK-REQUEST",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get break state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_break_state","end",tnc_break);
    return(tnc_break);
}

int
#ifdef CK_ANSIC
tnc_set_dtr_state(int onoff)
#else /* CK_ANSIC */
tnc_set_dtr_state(onoff) int onoff;
#endif /* CK_ANSIC */
/* tnc_set_dtr_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)> IAC SE */
    /*        Value     Dtr State
     *          8       On
     *          9       Off
     */
    /* wait for response, return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_dtr_state","begin",onoff);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( onoff != 0 && onoff == tnc_dtr )
        return(tnc_dtr);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = onoff ?
        TNC_CTL_DTR_ON : TNC_CTL_DTR_OFF;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  onoff ? "DTR-ON" : "DTR-OFF",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    if (tnc_wait((CHAR *)"comport set dtr state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_dtr_state","end",tnc_dtr);
    return(tnc_dtr);
}

int
#ifdef CK_ANSIC
tnc_get_dtr_state(void)
#else /* CK_ANSIC */
tnc_get_dtr_state()
#endif /* CK_ANSIC */
/* tnc_get_dtr_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)=7> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_dtr_state","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = TNC_CTL_DTR_REQUEST;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  "DTR-REQUEST",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    if (tnc_wait((CHAR *)"comport get dtr state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_dtr_state","end",tnc_dtr);
    return(tnc_dtr);
}

int
#ifdef CK_ANSIC
tnc_set_rts_state(int onoff)
#else /* CK_ANSIC */
tnc_set_rts_state(onoff) int onoff;
#endif /* CK_ANSIC */
/* tnc_set_rts_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)> IAC SE */
    /*        Value     Rts State
     *          5       On
     *          6       Off
     */
    /* wait for response, return new host value. */
    int i = 0, rc;

    debug(F111,"tnc_set_rts_state","begin",onoff);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( onoff != 0 && onoff == tnc_rts )
        return(tnc_rts);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = onoff ?
      TNC_CTL_RTS_ON : TNC_CTL_RTS_OFF;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  onoff ? "RTS-ON" : "RTS-OFF",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    if (tnc_wait((CHAR *)"comport set rts state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_set_rts_state","end",tnc_rts);
    return(tnc_rts);
}

int
#ifdef CK_ANSIC
tnc_get_rts_state(void)
#else /* CK_ANSIC */
tnc_get_rts_state()
#endif /* CK_ANSIC */
/* tnc_get_rts_state */ {
    /* IAC SB COM_PORT SET_CONTROL <value(1)=10> IAC SE */
    /* wait for response */
    int i = 0, rc;

    debug(F110,"tnc_get_rts_state","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_CONTROL;    /* Set Control */
    sb_out[i++] = TNC_CTL_RTS_REQUEST;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-CONTROL ",
                  "RTS-REQUEST",
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);

    if (tnc_wait((CHAR *)"comport get rts state",0) < 0) {
        tn_push();
        return(-1);
    }
    debug(F111,"tnc_get_rts_state","end",tnc_rts);
    return(tnc_rts);
}

int
#ifdef CK_ANSIC
tnc_set_ls_mask(int mask)
#else /* CK_ANSIC */
tnc_set_ls_mask(mask) int mask;
#endif /* CK_ANSIC */
/* tnc_set_ls_mask */ {
    /* IAC SB COM_PORT SET_LINESTATE_MASK <value(1)> IAC SE */
    /*        Bit       Meaning
     *          0       Data Ready
     *          1       Overrun Error
     *          2       Parity Error
     *          3       Framing Error
     *          4       Break Detect Error
     *          5       Transfer Holding Register Empty
     *          6       Transfer Shift Register Empty
     *          7       Timeout Error
     */
    int i = 0, rc;

    debug(F111,"tnc_set_ls_mask","begin",mask);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( mask != 0 && mask == tnc_ls_mask )
        return(tnc_ls_mask);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_LS_MASK;
    sb_out[i++] = (unsigned char)(mask & 0xFF);
    if (sb_out[i-1] == IAC )
      sb_out[i++] = IAC;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-LINESTATE-MASK ",
                  ckitoa(mask & 0xFF),
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    tnc_ls_mask = mask;
    debug(F111,"tnc_set_ls_mask","end",tnc_ls_mask);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_get_ls_mask(void)
#else /* CK_ANSIC */
tnc_get_ls_mask()
#endif /* CK_ANSIC */
/* tnc_get_ls_mask */ {
    debug(F101,"tnc_get_ls_mask","",tnc_ls_mask);
    return(tnc_ls_mask);
}

int
#ifdef CK_ANSIC
tnc_get_ls(void)
#else /* CK_ANSIC */
tnc_get_ls()
#endif /* CK_ANSIC */
/* tnc_get_ls */ {
    int ls = tnc_ls;
    debug(F101,"tnc_get_ls","",tnc_ls);
    return(ls);
}

int
#ifdef CK_ANSIC
tnc_set_ms_mask(int mask)
#else /* CK_ANSIC */
tnc_set_ms_mask(mask) int mask;
#endif /* CK_ANSIC */
/* tnc_set_ms_mask */ {
    /* IAC SB COM_PORT SET_MODEMSTATE_MASK <value(1)> IAC SE */
    /*        Bit       Meaning
     *          0       Delta Clear To Send
     *          1       Delta Data Set Ready
     *          2       Trailing Edge Ring Detector
     *          3       Delta Receive Line Signal (Carrier) Detect
     *          4       Clear To Send Signal State
     *          5       Data-Set-Ready Signal State
     *          6       Ring Indicator
     *          7       Receive Line Signal (Carrier) Detect
     */

    int i = 0, rc;

    debug(F111,"tnc_set_ms_mask","begin",mask);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( mask != 0 && mask == tnc_ms_mask )
        return(tnc_ms_mask);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_SET_MS_MASK;
    sb_out[i++] = (unsigned char)(mask & 0xFF);
    if (sb_out[i-1] == IAC )
      sb_out[i++] = IAC;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " SET-MODEMSTATE-MASK ",
                  ckitoa(mask & 0xFF),
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);

    tnc_ms_mask = mask;
    debug(F111,"tnc_set_ms_mask","end",tnc_ms_mask);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_get_ms_mask(void)
#else /* CK_ANSIC */
tnc_get_ms_mask()
#endif /* CK_ANSIC */
/* tnc_get_ms_mask */ {
    debug(F101,"tnc_get_gs_mask","",tnc_ms_mask);
    return(tnc_ms_mask);
}

int
#ifdef CK_ANSIC
tnc_get_ms(void)
#else /* CK_ANSIC */
tnc_get_ms()
#endif /* CK_ANSIC */
/* tnc_get_ms */ {
    int ms = tnc_ms;
    debug(F101,"tnc_get_ms","",tnc_ms);
    return(ms);
}

int
#ifdef CK_ANSIC
tnc_send_purge_data(int mode)
#else /* CK_ANSIC */
tnc_send_purge_data(mode) int mode;
#endif /* CK_ANSIC */
/* tnc_send_purge_data */ {
    /* IAC SB COM_PORT PURGE_DATA <value(1)> IAC SE */
    /*        Value     Meaning
     *          1       Purge access server receive data buffer
     *          2       Purge access server transmit data buffer
     *          3       Purge access server receive and transmit data buffers
     */
    /* No response */
    int i = 0, rc;

    debug(F111,"tnc_send_purge_data","begin",mode);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( !(mode >= 1 && mode <= 3) )
        return(0);

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_PURGE;
    sb_out[i++] = (unsigned char)(mode & 0xFF);
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg_out,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                  " PURGE-DATA ",
                  ckitoa(mode & 0xFF),
                  " IAC SE", NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
      return(-1);
    debug(F110,"tnc_send_purge_data","end",0);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_flow_suspended(void)
#else /* CK_ANSIC */
tnc_flow_suspended()
#endif /* CK_ANSIC */
/* tnc_flow_suspended */ {
    debug(F111,"tnc_flow_suspended","",tnc_suspend_xmit);
    return(tnc_suspend_xmit);
}

int
#ifdef CK_ANSIC
tnc_suspend_flow(void)
#else /* CK_ANSIC */
tnc_suspend_flow()
#endif /* CK_ANSIC */
/* tnc_suspend_flow */ {
    /* IAC SB COM_PORT FLOWCONTROL_SUSPEND IAC SE */
    int i = 0, rc;

    debug(F110,"tnc_suspend_flow","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_FLOW_SUSPEND;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakmsg(tn_msg_out,TN_MSG_LEN,
                 "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                 " FLOWCONTROL-SUSPEND IAC SE", NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    debug(F110,"tnc_suspend_flow","end",0);
    return(0);
}

int
#ifdef CK_ANSIC
tnc_resume_flow(void)
#else /* CK_ANSIC */
tnc_resume_flow()
#endif /* CK_ANSIC */
/* tnc_resume_flow */ {
    /* IAC SB COM_PORT FLOWCONTROL_RESUME IAC SE */
    int i = 0, rc;

    debug(F110,"tnc_resume_flow","begin",0);
    if (ttnet != NET_TCPB) return(0);
    if (ttnproto != NP_TELNET) return(0);

    if (!TELOPT_ME(TELOPT_COMPORT)) return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    sb_out[i++] = (CHAR) IAC;                 /* I Am a Command */
    sb_out[i++] = (CHAR) SB;                  /* Subnegotiation */
    sb_out[i++] = TELOPT_COMPORT;             /* ComPort */
    sb_out[i++] = TNC_C2S_FLOW_RESUME;
    sb_out[i++] = (CHAR) IAC;                 /* End of Subnegotiation */
    sb_out[i++] = (CHAR) SE;                  /* marked by IAC SE */

#ifdef DEBUG
    if (deblog || tn_deb || debses) {
        ckmakmsg(tn_msg_out,TN_MSG_LEN,
                 "TELNET SENT SB ",TELOPT(TELOPT_COMPORT),
                 " FLOWCONTROL-RESUME IAC SE", NULL);
    }
#endif /* DEBUG */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
#ifdef DEBUG
    debug(F100,tn_msg_out,"",0);
    if (tn_deb || debses) tn_debug(tn_msg_out);
#endif /* DEBUG */
    rc = (ttol((CHAR *)sb_out,i) < 0);      /* Send it. */
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if (rc)
        return(-1);
    debug(F110,"tnc_resume_flow","end",0);
    return(0);
}

int
#ifdef CK_ANSIC
tnsetflow(int nflow)
#else
tnsetflow(nflow) int nflow;
#endif /* CK_ANSIC */
/* tnsetflow */ {

    int rc = -1;

    debug(F111,"tnsetflow","begin",nflow);
    if (ttnet != NET_TCPB || ttnproto != NP_TELNET)
        return(-1);

    if (TELOPT_ME(TELOPT_COMPORT)) {
        switch(nflow) {
          case FLO_XONX:
            rc = tnc_set_oflow(
                 TNC_CTL_OFLOW_XON_XOFF
                 );
            if (rc >= 0)
              rc = tnc_set_iflow(
                   TNC_CTL_IFLOW_XON_XOFF
                   );
            break;
          case FLO_RTSC:
            rc = tnc_set_oflow(
                 TNC_CTL_OFLOW_RTS_CTS
                 );
            if (rc >= 0)
              rc = tnc_set_iflow(
                   TNC_CTL_IFLOW_RTS_CTS
                   );
            break;
          case FLO_KEEP:
            /* leave things exactly as they are */
            rc = 0;
            break;
          case FLO_NONE:
          case FLO_DIAL:  /* dialing hack */
          case FLO_DIAX:  /* cancel dialing hack */
            rc = tnc_set_oflow(
                 TNC_CTL_OFLOW_NONE
                 );
            if (rc >= 0)
              rc = tnc_set_iflow(
                   TNC_CTL_IFLOW_NONE
                   );
            break;
          case FLO_DTRC:
          case FLO_ETXA:
          case FLO_STRG:
          case FLO_DTRT:
          default:
            /* not supported */
            rc = -1;
            break;
        }
    }
    debug(F111,"tnsetflow","end",rc);
    return(rc >= 0 ? 0 : -1);
}

int
#ifdef CK_ANSIC
tnsettings(int par, int stop)
#else
tnsettings(par, stop) int par, stop;
#endif /* CK_ANSIC */
/* tnsettings */ {
    int rc = -1;
    int datasize = 0;
    extern int hwparity;

    debug(F111,"tnsettings begin","par",par);
    debug(F111,"tnsettings begin","stop",stop);
    if (ttnet != NET_TCPB || ttnproto != NP_TELNET)
      return(-1);

    datasize = par ? TNC_DS_7 : TNC_DS_8;
    if (!par) par = hwparity;

    if (TELOPT_ME(TELOPT_COMPORT)) {
        switch (par) {
          case 'e':
            rc = tnc_set_parity(TNC_PAR_EVEN);
            if (rc >= 0)
              rc = tnc_set_datasize(datasize);
            break;
          case 'o':
            rc = tnc_set_parity(TNC_PAR_ODD);
            if (rc >= 0)
              rc = tnc_set_datasize(datasize);
            break;
          case 'm':
            rc = tnc_set_parity(TNC_PAR_MARK);
            if (rc >= 0)
              rc = tnc_set_datasize(datasize);
            break;
          case 's':
            rc = tnc_set_parity(TNC_PAR_SPACE);
            if (rc >= 0)
              rc = tnc_set_datasize(datasize);
            break;
          case 0:
          case 'n':
            rc = tnc_set_parity(TNC_PAR_NONE);
            if (rc >= 0)
              rc = tnc_set_datasize(datasize);
            break;
          default:
            /* no change */
            rc = 0;
        }
        switch(stop) {
          case 2:
            if (rc >= 0)
              rc = tnc_set_stopsize(TNC_SB_2);
            break;
          case 1:
            if (rc >= 0)
              rc = tnc_set_stopsize(TNC_SB_1);
            break;
          default:
            /* no change */
            if (rc >= 0)
              rc = 0;
        }
    }
    debug(F111,"tnsettings","end",rc);
    return((rc >= 0) ? 0 : -1);
}

/*  T N G M D M  --  Telnet Get modem signals  */
/*
 Looks for the modem signals CTS, DSR, and CTS, and returns those that are
 on in as its return value, in a bit mask as described for ttwmdm.
 Returns:
  -3 Not implemented
  -2 if the line does not have modem control
  -1 on error.
  >= 0 on success, with a bit mask containing the modem signals that are on.
*/
int
#ifdef CK_ANSIC
tngmdm(void)
#else
tngmdm()
#endif /* CK_ANSIC */
/* tngmdm */ {

    int rc = -1;

    debug(F110,"tngmdm","begin",0);
    if (ttnet != NET_TCPB || ttnproto != NP_TELNET)
      return(-1);

    if (TELOPT_ME(TELOPT_COMPORT)) {
        int modemstate = tnc_get_ms();
        int modem = 0;
        if (modemstate & TNC_MS_CTS_SIG)
          modem |= BM_CTS;
        if (modemstate & TNC_MS_DSR_SIG)
          modem |= BM_DSR;
        if (modemstate & TNC_MS_RI_SIG)
          modem |= BM_RNG;
        if (modemstate & TNC_MS_RLSD_SIG)
          modem |= BM_DCD;
        debug(F111,"tngmdm","end",modem);
        return(modem);
    } else {
        debug(F111,"tngmdm","end",-2);
        return(-2);
    }
}

int
#ifdef CK_ANSIC
tnsndb(long wait)
#else
tnsndb(wait) long wait;
#endif /* CK_ANSIC */
/* tnsndb */ {
    int rc = -1;

    debug(F111,"tnsndb","begin",wait);
    if (ttnet != NET_TCPB || ttnproto != NP_TELNET)
      return(-1);

    if (TELOPT_ME(TELOPT_COMPORT)) {
        rc  = tnc_set_break_state(1);
        if (rc >= 0) {
            msleep(wait);                         /* ZZZzzz */
            rc = tnc_set_break_state(0);
        }
    }
    debug(F111,"tnsndb","end",rc);
    return((rc >= 0) ? 0 : -1);
}
#endif /* TN_COMPORT */
#endif /* TNCODE */
