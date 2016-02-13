char *ckathv = "Authentication, 9.0.235, 16 Mar 2010";
/*
  C K U A T H . C  --  Authentication for C-Kermit

  Copyright (C) 1999, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

    Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
               Secure Endpoints Inc., New York City
*/
/*
 * Additional copyrights included with affected code.
 */

#ifdef HEIMDAL
/*
  Turned off User to User support
  Turned off KDESTROY support
  Turned off KLIST support
  Turned off krb5_prompter() support
  Turned off ticket validation
  Turned off ticket renewal
  Turned off alternative cache support in k5_get_ccache()

  Remaining link problems:

  ckuath.o: In function `ck_krb5_initTGT':
  ckuath.o(.text+0x50c2): undefined reference to `krb5_string_to_deltat'
  ckuath.o(.text+0x516d): undefined reference to `krb5_string_to_deltat'
  ckuath.o(.text+0x51ef): undefined reference to `krb5_string_to_deltat'
*/
#endif /* HEIMDAL */

/*
 * Implements Kerberos 4/5, SRP, SSL, NTLM authentication and START_TLS
 */

#include "ckcsym.h"
#include "ckcdeb.h"

#ifdef CK_SECURITY

#define CKUATH_C
#include "ckcker.h"
#include "ckuusr.h"
#include "ckucmd.h"                             /* For struct keytab */
#include "ckcnet.h"
#include "ckctel.h"

char szUserNameRequested[UIDBUFLEN+1];    /* for incoming connections */
char szUserNameAuthenticated[UIDBUFLEN+1];/* for incoming connections */
char szHostName[UIDBUFLEN+1];
char szUserName[UIDBUFLEN+1];
static char szIP[16];
static int  validUser = AUTH_REJECT;    /* User starts out invalid */
int authentication_version = AUTHTYPE_NULL;
int accept_complete = 0;

#ifdef CK_AUTHENTICATION
#ifdef CK_SSL
#ifdef KRB5
#define TLS_VERIFY
#endif /* KRB5 */
#endif /* CK_SSL */

#ifdef CK_DES
#ifdef CK_SSL
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */
#endif /* CK_SSL */
#endif /* CK_DES */

#ifdef CRYPT_DLL
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */
#ifdef OS2
#ifdef NT
#include <windows.h>
#else /* NT */
#define INCL_DOSMODULEMGR
#include <os2.h>
#endif /* NT */
#endif /* OS2 */
#endif /* CRYPT_DLL */

#ifdef NT
#define KRB5_AUTOCONF__
#define NTLM
#endif /* NT */

#ifdef CK_KERBEROS
#define KINIT
#ifndef HEIMDAL
#define KLIST
#define KDESTROY
#endif /* HEIMDAL */
#define CHECKADDRS
#else /* CK_KERBEROS */
#ifdef KRB4
#undef KRB4
#endif /* KRB4 */
#ifdef KRB5
#undef KRB5
#endif /* KRB5 */
#ifdef KRB524
#undef KRB524
#endif /* KRB524 */
#endif /* CK_KERBEROS */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#ifndef malloc
#ifndef VMS
#ifndef FREEBSD4
#ifndef OpenBSD
#ifdef MACOSX
#include <sys/malloc.h>
#else /* MACOSX */
#include <malloc.h>
#endif /* MACOSX */
#endif /* OpenBSD */
#endif /* FREEBSD4 */
#endif /* VMS */
#endif /* malloc */
#ifdef OS2
#include <io.h>
#endif /* OS2 */

#ifdef KRB5
#ifdef HEIMDAL
#ifdef printf
#define saveprintf printf
#undef printf
#endif /* printf */
#include "krb5.h"

#include "com_err.h"

#ifdef saveprintf
#define printf saveprintf
#endif /* saveprintf */
#else /* HEIMDAL */
#include "krb5.h"
#include "profile.h"
#include "com_err.h"
#ifdef KRB5_GET_INIT_CREDS_OPT_TKT_LIFE
#define KRB5_HAVE_GET_INIT_CREDS
#else
#define krb5_free_unparsed_name(con,val) krb5_xfree((char *)(val))
#endif
#ifndef KRB5_HAVE_GET_INIT_CREDS
#define krb5_free_data_contents(c,v) krb5_xfree((char *)(v)->data)
#endif
#endif /* HEIMDAL */
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#endif /* KRB5 */

#ifdef KRB4
#define  des_cblock Block
#define  const_des_cblock const Block
#define  des_key_schedule Schedule
#ifdef KRB524
#ifdef NT
#define _WINDOWS
#endif /* NT */
#include "kerberosIV/krb.h"
#ifndef OS2
#ifdef KRB524_CONV
#include "krb524.h"
#endif /* KRB524_CONV */
_PROTOTYP(const char * krb_get_err_text_entry, (int));
#endif /* OS2 */
#else /* KRB524 */
#ifdef SOLARIS
#ifndef sun
/* for some reason the Makefile entries for the Solaris systems have -Usun */
#define sun
#endif /* sun */
#endif /* SOLARIS */
#include "krb.h"
#define krb_get_err_text_entry krb_get_err_text
#endif /* KRB524 */
#else /* KRB4 */
#ifdef CK_SSL
#define  des_cblock Block
#ifdef COMMENT
#define  const_des_cblock const Block
#endif /* COMMENT */
#define  des_key_schedule Schedule
#endif /* CK_SSL */
#endif /* KRB4 */

#include "ckuath.h"
#ifdef CK_KERBEROS
#ifndef KRB5
#define NOBLOCKDEF
#else /* KRB5 */
#ifdef KRB524
#define NOBLOCKDEF
#endif /* KRB524 */
#endif /* KRB5 */
#endif /* CK_KERBEROS */
#include "ckuat2.h"

#ifdef CK_SSL
#ifdef LIBDES
#ifdef OPENSSL_097
#ifdef CK_DES
#define OPENSSL_ENABLE_OLD_DES_SUPPORT
#include <openssl/des.h>
#endif	/* CK_DES */
#endif /* OPENSSL_097 */
#ifndef HEADER_DES_H
#define HEADER_DES_H
#endif /* HEADER_DES_H */
#endif /* LIBDES */
#include "ck_ssl.h"
extern int ssl_finished_messages;
#endif /* SSL */

#define PWD_SZ 128

#ifndef LIBDES
#ifdef UNIX
#define des_set_random_generator_seed(x) des_init_random_number_generator(x)
#endif /* UNIX */
#else /* LIBDES */
#define des_fixup_key_parity des_set_odd_parity
#endif /* LIBDES */

#ifdef OS2
#ifdef CK_ENCRYPTION
#define MAP_DES
#endif /* CK_ENCRYPTION */
#ifdef KRB4
#define MAP_KRB4
#endif /* KRB4 */
#ifdef SRPDLL
#define MAP_SRP
#endif /* SRPDLL */
#ifdef KRB5
#define MAP_KRB5
#endif /* KRB5 */
#ifdef CRYPT_DLL
#define MAP_CRYPT
#endif /* CRYPT_DLL */
#define MAP_NTLM
#include "ckoath.h"
#include "ckosyn.h"
#endif /* OS2 */

/*
 * Globals
 */
int auth_type_user[AUTHTYPLSTSZ] = {AUTHTYPE_AUTO, AUTHTYPE_NULL};
int auth_how=0;
int auth_crypt=0;
int auth_fwd=0;

/* These are state completion variables */
static int mutual_complete = 0;

#ifdef KRB4
#ifdef OS2
static LEASH_CREDENTIALS cred;
#else /* OS2 */
static CREDENTIALS cred;
#endif /* OS2 */
static KTEXT_ST k4_auth;
static char     k4_name[ANAME_SZ];
static AUTH_DAT k4_adat  = { 0 };
static MSG_DAT  k4_msg_data;
#ifdef CK_ENCRYPTION
static Block    k4_session_key     = { 0 };
static Schedule k4_sched;
static Block    k4_challenge       = { 0 };
#ifdef MIT_CURRENT
static krb5_keyblock k4_krbkey;
#endif /* MIT_CURRENT */
#endif /* ENCRYPTION */
#define KRB4_SERVICE_NAME    "rcmd"

_PROTOTYP(static int k4_auth_send,(VOID));
_PROTOTYP(static int k4_auth_reply,(unsigned char *, int));
_PROTOTYP(static int k4_auth_is,(unsigned char *, int));
#endif /* KRB4 */

#ifdef KRB5
static krb5_data          k5_auth;
static krb5_auth_context  auth_context;
static krb5_keyblock     *k5_session_key = NULL;
static krb5_ticket       *k5_ticket = NULL;
#ifndef KRB5_SERVICE_NAME
#define KRB5_SERVICE_NAME    "host"
#ifdef MACOSX
#define MIT_CURRENT 1
#define decode_krb5_ticket  krb5_decode_ticket
#define krb5_read_message   ck_krb5_read_message
#define krb5_write_message  ck_krb5_write_message
#endif /* MACOSX */
#endif

_PROTOTYP(static int k5_auth_send,(int,int,int));
_PROTOTYP(static int k5_auth_reply,(int, unsigned char *, int));
_PROTOTYP(static int k5_auth_is,(int,unsigned char *, int));
_PROTOTYP(static int SendK5AuthSB,(int, void *, int));
#ifdef TLS_VERIFY
static int krb5_tls_verified = 0;
#endif /* TLS_VERIFY */
#endif /* KRB5 */

#ifdef GSSAPI_KRB5
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_generic.h>
#include <gssapi/gssapi_krb5.h>

static gss_ctx_id_t gcontext;
#define GSS_BUFSIZ 4096
static gss_buffer_desc gss_send_tok, gss_recv_tok, *gss_token_ptr;
static char gss_stbuf[GSS_BUFSIZ];
static gss_name_t gss_target_name;
static struct gss_channel_bindings_struct gss_chan;

_PROTOTYP(static int gssk5_auth_send,(int,int,int));
_PROTOTYP(static int gssk5_auth_reply,(int, unsigned char *, int));
_PROTOTYP(static int gssk5_auth_is,(int,unsigned char *, int));
_PROTOTYP(static int SendGSSK5AuthSB,(int, void *, int));
#endif /* GSSAPI_KRB5 */

#ifdef CK_SRP
#ifdef PRE_SRP_1_7_3
_PROTOTYP(static int srp_reply,(int, unsigned char *, int));
_PROTOTYP(static int srp_is,(int, unsigned char *, int));
#else /* PRE_SRP_1_7_3 */
_PROTOTYP(static int new_srp_reply,(int, unsigned char *, int));
_PROTOTYP(static int new_srp_is,(int, unsigned char *, int));
#endif /* PRE_SRP_1_7_3 */
#endif /* SRP */

#ifdef CK_ENCRYPTION
int encrypt_flag = 1;
#endif
#ifdef FORWARD
int forward_flag = 0;              /* forward tickets? */
int forwardable_flag = 1;          /* get forwardable tickets to forward? */
int forwarded_tickets = 0;         /* were tickets forwarded? */
#endif

static unsigned char str_data[4096] = { IAC, SB, TELOPT_AUTHENTICATION, 0,
                                        AUTHTYPE_KERBEROS_V5, };
#define AUTHTMPBL 2048
static char strTmp[AUTHTMPBL+1];
static char szLocalHostName[UIDBUFLEN+1];
static kstream g_kstream=NULL;

#ifdef KRB5
krb5_context k5_context=NULL;
static krb5_creds * ret_cred=NULL;
static krb5_context telnet_context=NULL;
static char * telnet_krb5_realm = NULL;
static krb5_principal fwd_server = NULL;
#endif /* KRB5 */

#ifdef CK_SRP
#ifdef PRE_SRP_1_4_4
#ifndef PRE_SRP_1_4_5
#define PRE_SRP_1_4_5
#endif /* PRE_SRP_1_4_5 */
#endif /* PRE_SRP_1_4_5 */
#ifdef PRE_SRP_1_4_5
#ifndef PRE_SRP_1_7_3
#define PRE_SRP_1_7_3
#endif /* PRE_SRP_1_7_3 */
#endif /* PRE_SRP_1_4_5 */
#include <t_pwd.h>
#include <t_client.h>
#include <t_server.h>
static struct t_server * ts = NULL;
static struct t_client * tc = NULL;
#ifdef PRE_SRP_1_4_4
static struct t_pw * tpw = NULL;
static struct t_conf * tconf = NULL;
#endif /* PRE_SRP_1_4_4 */
#ifndef PRE_SRP_1_7_3
#ifndef STDC_HEADERS
#define STDC_HEADERS 1
#endif /* STDC_HEADERS */
#include <srp.h>
static SRP * s_srp = NULL;
static cstr * s_key = NULL;
static SRP * c_srp = NULL;
static cstr * c_key = NULL;
#endif /* PRE_SRP_1_7_3 */
static int srp_waitresp = 0;    /* Flag to indicate readiness for response */
static char srp_passwd[PWD_SZ];
#endif /* CK_SRP */

#ifdef CK_KERBEROS
#ifdef RLOGCODE
#define OPTS_FORWARD_CREDS           0x00000020
#define OPTS_FORWARDABLE_CREDS       0x00000010
#define KCMD_KEYUSAGE                1026

#define RLOG_BUFSIZ 5120
static int rlog_encrypt = 0;
char des_inbuf[2*RLOG_BUFSIZ];       /* needs to be > largest read size */
char des_outpkt[2*RLOG_BUFSIZ+4];    /* needs to be > largest write size */
#ifdef KRB5
krb5_data desinbuf,desoutbuf;
krb5_encrypt_block eblock;             /* eblock for encrypt/decrypt */
static krb5_data encivec_i[2], encivec_o[2];

enum krb5_kcmd_proto {
  /* Old protocol: DES encryption only.  No subkeys.  No protection
     for cleartext length.  No ivec supplied.  OOB hacks used for
     rlogin.  Checksum may be omitted at connection startup.  */
  KCMD_OLD_PROTOCOL = 1,
  /* New protocol: Any encryption scheme.  Client-generated subkey
     required.  Prepend cleartext-length to cleartext data (but don't
     include it in count).  Starting ivec defined, chained.  In-band
     signalling.  Checksum required.  */
  KCMD_NEW_PROTOCOL,
  /* Hack: Get credentials, and use the old protocol iff the session
     key type is single-DES.  */
  KCMD_PROTOCOL_COMPAT_HACK,
  KCMD_UNKNOWN_PROTOCOL
};
enum krb5_kcmd_proto krb5_rlog_ver = KCMD_PROTOCOL_COMPAT_HACK;
#endif /* KRB5 */
#endif /* RLOGCODE */
static char storage[65536];            /* storage for the decryption */
static int nstored = 0;
static char *store_ptr = storage;

extern char * krb5_d_principal;         /* Default principal */
extern char * krb5_d_instance;          /* Default instance */
extern char * krb5_d_realm;             /* Default realm */
extern char * krb5_d_cc;                /* Default credentials cache */
extern char * krb5_d_srv;               /* Default service name */
extern int    krb5_d_lifetime;          /* Default lifetime */
extern int    krb5_d_forwardable;
extern int    krb5_d_proxiable;
extern int    krb5_d_renewable;
extern int    krb5_autoget;
extern int    krb5_checkaddrs;
extern int    krb5_d_getk4;
extern int    krb5_d_no_addresses;
extern char * k5_keytab;

extern int    krb5_errno;
extern char * krb5_errmsg;

extern char * krb4_d_principal;         /* Default principal */
extern char * krb4_d_realm;             /* Default realm */
extern char * krb4_d_srv;               /* Default service name */
extern int    krb4_d_lifetime;          /* Default lifetime */
extern int    krb4_d_preauth;
extern char * krb4_d_instance;
extern int    krb4_autoget;
extern int    krb4_checkaddrs;
extern char * k4_keytab;

extern int    krb4_errno;
extern char * krb4_errmsg;
#endif /* CK_KERBEROS */

extern char tn_msg[], hexbuf[];         /* from ckcnet.c */
extern CHAR pwbuf[];
extern int  pwflg, pwcrypt;
extern int deblog, debses, tn_deb;
extern int sstelnet, inserver;
#ifdef CK_LOGIN
extern int ckxanon;
#endif /* CK_LOGIN */
extern int tn_auth_how;
extern int tn_auth_enc;
#ifdef CK_ENCRYPTION
extern int cx_type;
#endif /* CK_ENCRYPTION */
extern int quiet, ttyfd, ttnproto;

int
ck_gssapi_is_installed()
{
#ifdef KRB5
#ifdef OS2
    return(hGSSAPI != NULL);
#else /* OS2 */
    return(1);
#endif /* OS2 */
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}

int
ck_krb5_is_installed()
{
#ifdef KRB5
#ifdef OS2
    return(hKRB5_32 != NULL);
#else /* OS2 */
    return(1);
#endif /* OS2 */
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}


int
ck_krb5_is_installed_as_server()
{
#ifdef KRB5
#ifdef HEIMDAL
    krb5_error_code ret;
    krb5_keytab kt;
    krb5_kt_cursor cursor;

    ret = krb5_kt_default(k5_context, &kt);
    if ( ret ) {
        krb5_kt_close(k5_context, kt);
        return(0);
    } else {
        krb5_kt_end_seq_get(k5_context, kt, &cursor);
        krb5_kt_close(k5_context, kt);
        return(1);
    }
#else /* HEIMDAL */
#ifndef COMMENT
    char ktname[CKMAXPATH]="";

    if ( k5_keytab ) {
        ckstrncpy(ktname,k5_keytab,CKMAXPATH);
    } else {
        krb5_error_code code;

        if ( k5_context == NULL)
            if (krb5_init_context(&k5_context))
                return(0);

        code = krb5_kt_default_name(k5_context,ktname,CKMAXPATH);
        debug(F101,"krb5_kt_default_name","",code);
        if ( code ) {
            /* We can't check the existence of the file since we can't   */
            /* determine the file name.  So we return TRUE and let       */
            /* Krb5 be offered to the user even though it may fail later */
            return(1);
        }
    }

    if ( !strncmp("FILE:",ktname,5) ) {
        if ( zchki(&ktname[5]) > 0 )
            return(1);
        else
            return(0);
    } else {
        if (ktname[0])
            return(1);
        else
            return(0);
    }
#else /* COMMENT */
    krb5_error_code             krb5rc = KRB5KRB_ERR_GENERIC;
    krb5_context                krb5context = NULL;
    krb5_ccache                 krb5ccdef = NULL;
    krb5_creds                  krb5creds, *krb5credsp = NULL;
    int                         rc = 0;

    if ( !ck_krb5_is_installed() )
        return(0);

    memset((char *)&krb5creds, 0, sizeof(krb5creds));

    if ((krb5rc = krb5_init_context(&krb5context)) != 0)
        goto err;

    if ((krb5rc = krb5_sname_to_principal(krb5context,
                                          szHostName,
                                          krb5_d_srv ?
                                          krb5_d_srv :
                                          KRB5_SERVICE_NAME,
                                          KRB5_NT_SRV_HST,
                                          &krb5creds.server)) != 0)
      goto err;

    if ((krb5rc = krb5_cc_default(krb5context, &krb5ccdef)) != 0)
        goto err;

    if ((krb5rc = krb5_cc_get_principal(krb5context, krb5ccdef,
                                         &krb5creds.client)) != 0)
        goto err;

    if ((krb5rc = krb5_get_credentials(krb5context, 0, krb5ccdef,
                                        &krb5creds, &krb5credsp)) != 0)
        goto err;
    rc = 1;

  err:

    if (krb5creds.client)
      krb5_free_principal(krb5context, krb5creds.client);
    if (krb5creds.server)
      krb5_free_principal(krb5context, krb5creds.server);
    if (krb5context)
      krb5_free_context(krb5context);
    return(rc);

#endif /* COMMENT */
#endif /* HEIMDAL */
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}

int
ck_krb4_is_installed()
{
#ifdef KRB4
#ifdef OS2
    return(hKRB4_32 != NULL);
#else /* OS2 */
    return(1);
#endif /* OS2 */
#else /* KRB4 */
    return(0);
#endif /* KRB4 */
}

int
ck_krb4_is_installed_as_server()
{
    if ( !ck_krb4_is_installed() )
        return(0);

#ifdef KRB4
    if ( !k4_keytab ) {
#ifdef NT
        char name[CKMAXPATH]="";
        DWORD len = CKMAXPATH;

        len = GetWindowsDirectory(name,len);
        if ( len > 0 )
            ckstrncat(name,"/srvtab",CKMAXPATH);
        if ( name[0] )
            makestr(&k4_keytab,name);
#else /* NT */
        makestr(&k4_keytab,"/etc/srvtab");
#endif /* NT */
    }

    if ( !k4_keytab )
        return(0);

    if ( zchki(k4_keytab) > 0 )
        return(1);
#ifdef KRB524
    else if (ck_krb5_is_installed_as_server())
        return(1);
#endif /* KRB524 */
    else
        return(0);
#endif /* KRB4 */
}

int
ck_srp_is_installed_as_server()
{
#ifdef CK_SRP
#ifdef SRPDLL
    if ( hSRP == NULL )
        return(0);
#endif /* SRPDLL */
#ifdef COMMENT
    /* This is the new API as of 1.7.4.  However, all it does
       is allocate a data structure.  It can never fail.
     */
    {
        SRP * s_srp = SRP_new(SRP_RFC2945_server_method());
        if ( s_srp ) {
            SRP_free(s_srp);
            s_srp = NULL;
            return(1);
        }
        return(0);
    }
#else /* COMMENT */
    {
        struct t_pw * tpw = NULL;
        struct t_conf * tconf = NULL;
        if((tconf = t_openconf(NULL)) == NULL)
            return(0);
        if((tpw = t_openpw(NULL)) == NULL) {
            t_closeconf(tconf);
            return(0);
        }
        t_closeconf(tconf);
        t_closepw(tpw);
        return(1);
    }
#endif /* COMMENT */
#else /* SRP */
    return(0);
#endif /* SRP */
}

int
ck_srp_is_installed()
{
#ifdef CK_SRP
#ifdef SRPDLL
    if ( hSRP == NULL )
        return(0);
#endif /* SRPDLL */
    return(1);
#else /* CK_SRP */
    return(0);
#endif /* CK_SRP */
}

int
ck_krypto_is_installed()
{
#ifdef CK_SRP
#ifdef OS2
    if ( hLIBKRYPTO == NULL )
        return(0);
#endif /* OS2 */
    return(1);
#else /* CK_SRP */
    return(0);
#endif /* CK_SRP */
}

int
ck_crypt_is_installed()
{
#ifdef CK_ENCRYPTION
#ifdef CRYPT_DLL
    return(hCRYPT != NULL);
#else /* CRYPT_DLL */
    return(1);
#endif /* CRYPT_DLL */
#else /* ENCRYPTION */
    return(0);
#endif /* ENCRYPTION */
}

int
ck_ntlm_is_installed()
{
#ifdef NT
    return(hSSPI != NULL);
#else /* NT */
    return(0);
#endif /* NT */
}

int
ck_tn_auth_valid()
{
    return(validUser);
}

/* C K _ K R B _ A U T H _ I N _ P R O G R E S S
 *
 * Is an authentication negotiation still in progress?
 *
 */

int
#ifdef CK_ANSIC
ck_tn_auth_in_progress(void)
#else
ck_tn_auth_in_progress()
#endif
{
    switch (authentication_version) {
    case AUTHTYPE_AUTO:
        return(1);
    case AUTHTYPE_NULL:
        return(0);
#ifdef KRB4
    case AUTHTYPE_KERBEROS_V4:
        if (!accept_complete) {
            debug(F100,"ck_auth_in_progress() Kerberos 4 !accept_complete",
                   "",0);
            return(1);
        }
        else if ((auth_how & AUTH_HOW_MASK) && !mutual_complete) {
            debug(F100,"ck_auth_in_progress() Kerberos 4 !mutual_complete",
                   "",0);
            return(1);
        }
        else
            return(0);
#endif /* KRB4 */
#ifdef KRB5
    case AUTHTYPE_KERBEROS_V5:
        if (!accept_complete) {
            debug(F100,"ck_auth_in_progress() Kerberos 5 !accept_complete",
                   "",0);
            return(1);
        }
        else if ((auth_how & AUTH_HOW_MASK) && !mutual_complete) {
            debug(F100,"ck_auth_in_progress() Kerberos 5 !mutual_complete",
                   "",0);
            return(1);
        }
        else
            return(0);
#ifdef GSSAPI_K5
    case AUTHTYPE_GSSAPI_KRB5:
        if (!accept_complete) {
            debug(F100,
		  "ck_auth_in_progress() GSSAPI Kerberos 5 !accept_complete",
		  "",
		  0
		  );
            return(1);
        }
        else if ((auth_how & AUTH_HOW_MASK) && !mutual_complete) {
            debug(F100,
		  "ck_auth_in_progress() GSSAPI Kerberos 5 !mutual_complete",
		  "",
		  0
		  );
            return(1);
        } else
	  return(0);
        break;
#endif /* GSSAPI_K5 */
#endif /* KRB5 */
#ifdef CK_SRP
    case AUTHTYPE_SRP:
        if (!accept_complete || srp_waitresp)
            return(1);
        else
            return(0);
#endif /* CK_SRP */
#ifdef NTLM
    case AUTHTYPE_NTLM:
        if (!accept_complete) {
            debug(F100,"ck_auth_in_progress() NTLM !accept_complete",
                   "",0);
            return(1);
        }
        else
            return(0);
#endif /* NTLM */
    case AUTHTYPE_SSL:
        if (!accept_complete) {
            debug(F100,"ck_auth_in_progress() SSL !accept_complete",
                   "",0);
            return(1);
        }
        else
            return(0);
    default:
        return(0);
    }
    return(0);
}


/*  C K _ K R B _ T N _ A U T H _ R E Q U E S T
 *
 *  Builds a Telnet Authentication Send Negotiation providing the
 *  list of supported authentication methods.  To be used only
 *  when accepting incoming connections as only the server (DO) side of the
 *  Telnet negotiation is allowed to send an AUTH SEND.
 *
 *  Returns: 0 on success and -1 on failure
 */

static unsigned char str_request[64] = { IAC, SB,
                                             TELOPT_AUTHENTICATION,
                                             TELQUAL_SEND };
#ifdef GSSAPI_K5
static int
ck_tn_auth_request_gsskrb5(int i)
{
    if (ck_gssapi_is_installed() && ck_krb5_is_installed_as_server()) {
        if ( (tn_auth_how == TN_AUTH_HOW_ANY ||
               tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_EXCH) ) {
            str_request[i++] = AUTHTYPE_KERBEROS_V5;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
            str_request[i] |= AUTH_ENCRYPT_AFTER_EXCHANGE;

            if ( deblog || tn_deb || debses )
                ckstrncat(tn_msg,
                "KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL|ENCRYPT_AFTER_EXCHANGE ",
                          TN_MSG_LEN);
            i++;
        }
    }
}
#endif /* GSSAPI_K5 */

#ifdef KRB5
static int
ck_tn_auth_request_krb5(int i)
{
    if (ck_krb5_is_installed_as_server()) {
#ifdef CK_SSL
        if ( ck_ssleay_is_installed() &&
             (tls_active_flag || ssl_active_flag) &&
             ssl_finished_messages )
        {
#ifdef USE_INI_CRED_FWD
            if ( forward_flag &&
                 (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT)
                 )
            {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_START_TLS;
                str_request[i] |= INI_CRED_FWD_ON;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
 "KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL|ENCRYPT_START_TLS|INI_CRED_FWD_ON ",
                               TN_MSG_LEN);
                i++;
            }
#endif /* USE_INI_CRED_FWD */
            if ( (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT) ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_START_TLS;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
                      "KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL|ENCRYPT_START_TLS ",
                              TN_MSG_LEN);
                i++;
            }
            if ( tn_auth_how == TN_AUTH_HOW_ANY ||
                 tn_auth_how == TN_AUTH_HOW_ONE_WAY ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
                str_request[i] |= AUTH_ENCRYPT_START_TLS;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
                    "KERBEROS_V5 CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_START_TLS ",
                               TN_MSG_LEN);
                i++;
            }
        }
#ifdef CK_ENCRYPTION
        else
        {
#endif /* CK_ENCRYPTION */
#endif /* CK_SSL */
#ifdef CK_ENCRYPTION
#ifdef USE_INI_CRED_FWD
            if ( forward_flag &&
                 TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT)
                 )
            {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_USING_TELOPT;
                str_request[i] |= INI_CRED_FWD_ON;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
  "KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL|ENCRYPT_USING_TELOPT|INI_CRED_FWD_ON ",
                               TN_MSG_LEN);
                i++;
            }
#endif /* USE_INI_CRED_FWD */

            if ( TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT) ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_USING_TELOPT;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
              "KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL|ENCRYPT_USING_TELOPT ",
                               TN_MSG_LEN);
                i++;
            }
#ifdef CK_SSL
        }
#endif /* CK_SSL */
#endif /* CK_ENCRYPTION */

        if ( TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_NONE)
#ifdef CK_SSL
             && !(ck_ssleay_is_installed() &&
                   (tls_active_flag || ssl_active_flag) &&
                   tls_is_anon(0))
#endif /* CK_SSL */
             )
        {
#ifdef CK_ENCRYPTION
            /* Can't perform mutual authentication without encryption */
            if ( tn_auth_how == TN_AUTH_HOW_ANY ||
                 tn_auth_how == TN_AUTH_HOW_MUTUAL ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_OFF;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,"KERBEROS_V5 CLIENT_TO_SERVER|MUTUAL ",
                               TN_MSG_LEN);
                i++;
            }
#endif /* CK_ENCRYPTION */
            if ( tn_auth_how == TN_AUTH_HOW_ANY ||
                 tn_auth_how == TN_AUTH_HOW_ONE_WAY ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V5;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
                str_request[i] |= AUTH_ENCRYPT_OFF;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,"KERBEROS_V5 CLIENT_TO_SERVER|ONE_WAY ",
                               TN_MSG_LEN);
                i++;
            }
        }
    }
    return(i);
}
#endif /* KRB5 */
#ifdef KRB4
static int
ck_tn_auth_request_krb4(int i)
{
    if (ck_krb4_is_installed_as_server()) {
#ifdef CK_ENCRYPTION
        if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
             (tn_auth_how == TN_AUTH_HOW_ANY ||
               tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_TELOPT) )
        {
            str_request[i++] = AUTHTYPE_KERBEROS_V4;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
            str_request[i] |= AUTH_ENCRYPT_USING_TELOPT;

            if ( deblog || tn_deb || debses )
              ckstrncat(tn_msg,"KERBEROS_V4 CLIENT_TO_SERVER|MUTUAL|ENCRYPT ",
                        TN_MSG_LEN);
            i++;
        }
#endif /* CK_ENCRYPTION */

        if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_NONE) )
        {
#ifdef CK_ENCRYPTION
            /* Can't perform mutual authentication without encryption */
            if ( tn_auth_how == TN_AUTH_HOW_ANY ||
                 tn_auth_how == TN_AUTH_HOW_MUTUAL ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V4;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_MUTUAL;
                str_request[i] |= AUTH_ENCRYPT_OFF;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,"KERBEROS_V4 CLIENT_TO_SERVER|MUTUAL ",
                               TN_MSG_LEN);
                i++;
            }
#endif /* CK_ENCRYPTION */
            if ( tn_auth_how == TN_AUTH_HOW_ANY ||
                 tn_auth_how == TN_AUTH_HOW_ONE_WAY ) {
                str_request[i++] = AUTHTYPE_KERBEROS_V4;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
                str_request[i] |= AUTH_ENCRYPT_OFF;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,"KERBEROS_V4 CLIENT_TO_SERVER|ONE_WAY ",
                               TN_MSG_LEN);
                i++;
            }
        }
    }

    return(i);
}
#endif /* KRB4 */

#ifdef CK_SRP
static int
ck_tn_auth_request_srp(int i)
{
    if (ck_srp_is_installed_as_server()) {
#ifndef PRE_SRP_1_4_5
        /* Dont' do this yet.  SRP when it uses the ENCRYPT_USING_TELOPT   */
        /* flag it must perform a checksum of the auth-type-pair but there */
        /* is no mechansim to do that yet.                                 */
#ifdef CK_SSL
        if ( ck_ssleay_is_installed() &&
             (tls_active_flag || ssl_active_flag) &&
             ssl_finished_messages &&
                 (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_ONE_WAY)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT))
        {
            str_request[i++] = AUTHTYPE_SRP;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
            str_request[i] |= AUTH_ENCRYPT_START_TLS;

            if ( deblog || tn_deb || debses )
                ckstrncat(tn_msg,
                           "SRP CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_START_TLS ",
                           TN_MSG_LEN);
            i++;
        }
#ifdef CK_ENCRYPTION
        else {
#endif /* CK_ENCRYPTION */
#endif /* CK_SSL */
#ifdef CK_ENCRYPTION
            if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_RF &&
                 (tn_auth_how == TN_AUTH_HOW_ANY ||
                   tn_auth_how == TN_AUTH_HOW_ONE_WAY)  &&
                 (tn_auth_enc == TN_AUTH_ENC_ANY ||
                   tn_auth_enc == TN_AUTH_ENC_TELOPT)
                 ) {
                str_request[i++] = AUTHTYPE_SRP;
                str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
                str_request[i] |= AUTH_ENCRYPT_USING_TELOPT;

                if ( deblog || tn_deb || debses )
                    ckstrncat(tn_msg,
                    "SRP CLIENT_TO_SERVER|ONE_WAY|ENCRYPT_USING_TELOPT ",
                               TN_MSG_LEN);
                i++;
            }
#ifdef CK_SSL
        }
#endif /* CK_SSL */
#endif /* CK_ENCRYPTION */
#endif /* PRE_SRP_1_4_5 */
        if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             (tn_auth_how == TN_AUTH_HOW_ANY ||
               tn_auth_how == TN_AUTH_HOW_MUTUAL)  &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_NONE)
#ifdef CK_SSL
             && !(ck_ssleay_is_installed() &&
                   (tls_active_flag || ssl_active_flag) &&
                   tls_is_anon(0))
#endif /* CK_SSL */
             )
        {
            str_request[i++] = AUTHTYPE_SRP;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
            str_request[i] |= AUTH_ENCRYPT_OFF;

            if ( deblog || tn_deb || debses )
                ckstrncat(tn_msg,"SRP CLIENT_TO_SERVER|ONE_WAY ",
                           TN_MSG_LEN);
            i++;
        }
    }

    return(i);
}
#endif /* CK_SRP */

#ifdef CK_SSL
static int
ck_tn_auth_request_ssl(int i)
{
    if (ck_ssleay_is_installed()
         && !tls_active_flag && !ssl_active_flag && ssl_initialized
         ) {
        if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             (tn_auth_how == TN_AUTH_HOW_ANY ||
               tn_auth_how == TN_AUTH_HOW_ONE_WAY)  &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_NONE) )
        {
            str_request[i++] = AUTHTYPE_SSL;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
            str_request[i] |= AUTH_ENCRYPT_OFF;
            if ( deblog || tn_deb || debses )
                ckstrncat(tn_msg,"SSL CLIENT_TO_SERVER|ONE_WAY ",
                           TN_MSG_LEN);
            i++;
        }
    }

    return(i);
}
#endif /* CK_SSL */
#ifdef NTLM
static int
ck_tn_auth_request_ntlm(int i)
{
    /* Microsoft's Telnet client won't perform authentication if */
    /* NTLM is not first.                                        */
    if ( ck_ntlm_is_valid(1) ) {
        if (TELOPT_ME_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             TELOPT_U_MODE(TELOPT_ENCRYPTION) != TN_NG_MU &&
             (tn_auth_how == TN_AUTH_HOW_ANY ||
               tn_auth_how == TN_AUTH_HOW_ONE_WAY)  &&
             (tn_auth_enc == TN_AUTH_ENC_ANY ||
               tn_auth_enc == TN_AUTH_ENC_NONE) )
        {
            str_request[i++] = AUTHTYPE_NTLM;
            str_request[i] = AUTH_CLIENT_TO_SERVER | AUTH_HOW_ONE_WAY;
            str_request[i] |= AUTH_ENCRYPT_OFF;
            if ( deblog || tn_deb || debses )
                ckstrncat(tn_msg,"NTLM CLIENT_TO_SERVER|ONE_WAY ",
                           TN_MSG_LEN);
            i++;
        }
    }

    return(i);
}
#endif /* NTLM */
int
#ifdef CK_ANSIC
ck_tn_auth_request(void)
#else
ck_tn_auth_request()
#endif
{
    int i = 4, rc = -1;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return(0);
    }
#endif /* CK_SSL */

    if ( deblog || tn_deb || debses )
        strcpy(tn_msg,"TELNET SENT SB AUTHENTICATION SEND ");

    /* Create a list of acceptable Authentication types to send to */
    /* the client and let it choose find one that we support       */

    /* For those authentication methods that support Encryption or */
    /* Credentials Forwarding we must send all of the appropriate  */
    /* combinations based upon the state of                        */
    /* TELOPT_x_MODE(TELOPT_ENCRYPTION) and forward_flag.          */

    if ( auth_type_user[0] == AUTHTYPE_AUTO ) {
#ifdef GSSAPI_K5
        i = ck_tn_auth_request_gsskrb5(i);
#endif /* GSSAPI_K5 */
#ifdef KRB5
        i = ck_tn_auth_request_krb5(i);
#endif /* KRB5 */
#ifdef KRB4
        i = ck_tn_auth_request_krb4(i);
#endif /* KRB4 */
#ifdef CK_SRP
        i = ck_tn_auth_request_srp(i);
#endif /* SRP */
#ifdef CK_SSL
        i = ck_tn_auth_request_ssl(i);
#endif /* CK_SSL */
#ifdef NTLM
        i = ck_tn_auth_request_ntlm(i);
#endif /* NTLM */
    } else {
        int j;
        for ( j=0;
              j<AUTHTYPLSTSZ && auth_type_user[j] != AUTHTYPE_NULL;
              j++) {
#ifdef NTLM
            if (auth_type_user[j] == AUTHTYPE_NTLM)
                i = ck_tn_auth_request_ntlm(i);
#endif /* NTLM */
#ifdef CK_SSL
            if ( auth_type_user[j] == AUTHTYPE_SSL )
                i = ck_tn_auth_request_ssl(i);
#endif /* CK_SSL */
#ifdef CK_SRP
            if ( auth_type_user[j] == AUTHTYPE_SRP )
                i = ck_tn_auth_request_srp(i);
#endif /* SRP */
#ifdef GSSAPI_K5
            if ( auth_type_user[j] == AUTHTYPE_GSSAPI_KRB5 )
                i = ck_tn_auth_request_gsskrb5(i);
#endif /* GSSAPI_K5 */
#ifdef KRB5
            if ( auth_type_user[j] == AUTHTYPE_KERBEROS_V5 )
                i = ck_tn_auth_request_krb5(i);
#endif /* KRB5 */
#ifdef KRB4
            if ( auth_type_user[j] == AUTHTYPE_KERBEROS_V4 )
                i = ck_tn_auth_request_krb4(i);
#endif /* KRB4 */
        }
    }

    str_request[i++] = IAC;
    str_request[i++] = SE;
    if ( deblog || tn_deb || debses ) {
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Send data */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    rc = ttol((CHAR *)str_request, i);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    if ( rc == i )
        return(0);
    else
        return(-1);
}

#ifdef CK_ENCRYPTION
VOID
ck_tn_enc_start()
{
    if (!TELOPT_ME(TELOPT_ENCRYPTION) && !TELOPT_U(TELOPT_ENCRYPTION))
        return;
    if (!TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop &&
         (!encrypt_is_decrypting() || !encrypt_is_encrypting())) {
        debug(F110,"ck_tn_enc_start","nothing to do",0);
        return;
    }
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
    if (TELOPT_ME(TELOPT_ENCRYPTION) && !encrypt_is_encrypting()) {
        debug(F110,"ck_tn_enc_start","encrypt_request_start",0);
        encrypt_request_start();
    }
    if (TELOPT_U(TELOPT_ENCRYPTION) && !encrypt_is_decrypting()) {
        debug(F110,"ck_tn_enc_start","encrypt_send_request_start",0);
        encrypt_send_request_start();
    }
    tn_wait("encrypt start");
    tn_push();
}

VOID
ck_tn_enc_stop()
{
    if (!TELOPT_ME(TELOPT_ENCRYPTION) && !TELOPT_U(TELOPT_ENCRYPTION))
        return;
    if (TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop ||
         !(encrypt_is_decrypting() || encrypt_is_encrypting())) {
        debug(F110,"ck_tn_enc_stop","nothing to do",0);
      return;
    }
    TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 1;
    if (TELOPT_U(TELOPT_ENCRYPTION) && encrypt_is_decrypting()) {
        debug(F110,"ck_tn_enc_stop","encrypt_send_request_end",0);
        encrypt_send_request_end();
    }
    if (TELOPT_ME(TELOPT_ENCRYPTION) && encrypt_is_encrypting()) {
        debug(F110,"ck_tn_enc_stop","encrypt_send_end",0);
        encrypt_send_end();
    }
    tn_wait("encrypt stop");
    tn_push();
}
#endif /* CK_ENCRYPTION */

/*  C K _ K R B _ T N _ S B _ A U T H
 *  An interface between the C-Kermit Telnet Command Parser and the Authent-
 *  ication option parser implemented in the Kerberos Telnet client.
 *
 *  sb   - the subnegotiation as calculated in ckcnet.c
 *  len  - the length of the buffer
 *
 *  Returns: 0 on success and -1 on failure
 */

int
#ifdef CK_ANSIC
ck_tn_sb_auth(char * sb, int len)
#else /* CK_ANSIC */
ck_tn_sb_auth(sb,len) char * sb; int len;
#endif /* CK_ANSIC */
{
    /* auth_parse() assumes that sb starts at pos 1 not 0 as in ckcnet.c */
    /* and it wants the length to exclude the IAC SE bytes               */
    CHAR * buf;
    int rc = -1;

    buf = malloc(len-1);
    if ( !buf ) return(-1);

    buf[0] = SB;
    memcpy( &buf[1], sb, len-2 );
    rc = auth_parse(buf,len-1);
    free(buf);
    debug(F111,"ck_tn_sb_auth","rc",rc);
    if (rc == AUTH_FAILURE) {
        authentication_version = AUTHTYPE_NULL;
#ifndef NOLOCAL
#ifdef OS2
        ipadl25();
#endif /* OS2 */
#endif /* NOLOCAL */
        return(-1);
    }
#ifndef NOLOCAL
#ifdef OS2
    ipadl25();
#endif /* OS2 */
#endif /* NOLOCAL */
    return(0);
}

/*  C K _ K R B _ T N _ S B _ E N C R Y P T
 *  An interface between the C-Kermit Telnet Command Parser and the Encryption
 *  option parser implemented in the Kerberos Telnet client.
 *
 *  sb   - the subnegotiation as calculated in ckcnet.c
 *  len  - the length of the buffer
 *
 *  Returns: Always returns 0 for success since encrypt_parse is void
 */


int
#ifdef CK_ANSIC
ck_tn_sb_encrypt(char * sb, int len)
#else
ck_tn_sb_encrypt(sb,len) char * sb; int len;
#endif /* CK_ANSIC */
{
    /* encrypt_parse() assumes that sb starts at pos 1 not 0 as in ckcnet.c */
    /* and it wants the length to exclude the IAC SE bytes                  */
#ifdef CK_ENCRYPTION
    char * buf;
    int rc = -1;

    buf = malloc(len-1);
    if ( !buf ) return(-1);

    buf[0] = SB;
    memcpy( &buf[1], sb, len-2 );
    rc = encrypt_parse((CHAR *)buf,len-1);

    if (rc < 0) {
        free(buf);
        return(-1);
    }

    /* This is a hack.  It does not belong here but should really be in */
    /* encrypt_parse() but in K95 the encrypt_parse() routine does not  */
    /* have access to the telopt_states array.                          */
    if ( buf[1] == ENCRYPT_REQEND )
        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 1;
    else if ( buf[1] == ENCRYPT_REQSTART )
        TELOPT_SB(TELOPT_ENCRYPTION).encrypt.stop = 0;
#ifndef NOLOCAL
#ifdef OS2
    ipadl25();
#endif /* OS2 */
#endif /* NOLOCAL */
    free(buf);
#endif /* ENCRYPTION */
    return(0);
}


/*  C K _ K R B _ E N C R Y P T I N G
 *  Returns 1 if we are encrypting and 0 if we are not
 */

int
#ifdef CK_ANSIC
ck_tn_encrypting(VOID)
#else /* CK_ANSIC */
ck_tn_encrypting()
#endif /* CK_ANSIC */
{
#ifdef CK_ENCRYPTION
    if ( g_kstream == NULL )
        return(0);
    if ( g_kstream->encrypt && encrypt_is_encrypting()) {
        debug(F111,"ck_tn_encrypting","encrypting",
               g_kstream->encrypt_type);
        return(g_kstream->encrypt_type);
    }
#endif /* CK_ENCRYPTION */
    debug(F110,"ck_tn_encrypting","not encrypting",0);
    return(0);
}

/*  C K _ K R B _ D E C R Y P T I N G
 *  Returns 1 if we are decrypting and 0 if we are not
 */

int
#ifdef CK_ANSIC
ck_tn_decrypting(VOID)
#else
ck_tn_decrypting()
#endif /* CK_ANSIC */
{
#ifdef CK_ENCRYPTION
    if ( g_kstream == NULL )
        return(0);
    if ( g_kstream->decrypt && encrypt_is_decrypting()) {
        debug(F111,"ck_tn_decrypting","decrypting",
               g_kstream->decrypt_type);
        return(g_kstream->decrypt_type);
    }
#endif /* CK_ENCRYPTION */
    debug(F110,"ck_tn_decrypting","not decrypting",0);
    return(0);
}

/*  C K _ K R B _ A U T H E N T I C A T E D
 *  Returns the authentication type: AUTHTYPE_NULL, AUTHTYPE_KERBEROS4,
 *  or AUTHTYPE_KERBEROS5, AUTHTYPE_SRP, ... (see ckctel.h)
 */

int
#ifdef CK_ANSIC
ck_tn_authenticated(VOID)
#else
ck_tn_authenticated()
#endif
{
    return(authentication_version);
}

/*  C K _ K R B _ E N C R Y P T
 *  encrypts n characters in s if we are encrypting
 */

VOID
#ifdef CK_ANSIC
ck_tn_encrypt( char * s, int n )
#else
ck_tn_encrypt( s,n ) char * s; int n;
#endif
{
#ifdef CK_ENCRYPTION
    struct kstream_data_block i;

    if (g_kstream->encrypt && encrypt_is_encrypting()) {
#ifdef DEBUG
      ckhexdump("from plaintext", s, n);
#endif
        i.ptr = s;
        i.length = n;
        g_kstream->encrypt(&i, NULL);
#ifdef DEBUG
        ckhexdump("to cyphertext", s, n);
#endif
    }
    else debug(F101,"ck_tn_encrypt not encrypting","",n);
#endif /* ENCRYPTION */
}

/*  C K _ K R B _ D E C R Y P T
 *  decrypts n characters in s if we are decrypting
 */

VOID
#ifdef CK_ANSIC
ck_tn_decrypt( char * s, int n )
#else
ck_tn_decrypt( s,n ) char * s; int n;
#endif
{
#ifdef CK_ENCRYPTION
    struct kstream_data_block i;

    if (g_kstream->decrypt && encrypt_is_decrypting()) {

#ifdef DEBUG
        ckhexdump("from cyphertext", s, n);
#endif

        i.ptr = s;
        i.length = n;
        g_kstream->decrypt(&i, NULL);
#ifdef DEBUG
        ckhexdump("to plaintext", s, n);
#endif
    }
    else debug(F101,"ck_tn_decrypt not decrypting","",n);
#endif /* ENCRYPTION */
}

/*  S E N D K 5 A U T H S B
 *  Send a Kerberos 5 Authentication Subnegotiation to host and
 *  output appropriate Telnet Debug messages
 *
 *  type - Sub Negotiation type
 *  data - ptr to buffer containing data
 *  len  - len of buffer if not NUL terminated
 *
 *  returns number of characters sent or error value
 */

static int
#ifdef CK_ANSIC
SendK5AuthSB(int type, void *data, int len)
#else
SendK5AuthSB(type,data,len) int type; void *data; int len;
#endif
{
    int rc;
    unsigned char *p = str_data + 3;
    unsigned char *cd = (unsigned char *)data;
    extern int sstelnet;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        if (ttchk() < 0)
          return(0);
        else
          return(1);
    }
#endif /* CK_SSL */

    if ( type < 0 || type > 7 )         /* Check for invalid values */
        return(0);

    if (!cd) {
        cd = (unsigned char *)"";
        len = 0;
    }

    if (len == -1)                        /* Use strlen() for len */
        len = strlen((char *)cd);

    /* Construct Message */
    *p++ = sstelnet ? TELQUAL_REPLY : TELQUAL_IS;
    *p++ = AUTHTYPE_KERBEROS_V5;
    *p = AUTH_CLIENT_TO_SERVER;
    *p |= auth_how;
#ifdef CK_ENCRYPTION
    *p |= auth_crypt;
#endif
#ifdef USE_INI_CRED_FWD
    if (auth_fwd)
        *p |= INI_CRED_FWD_ON;
#endif /* USE_INI_CRED_FWD */
    p++;
    *p++ = type;
    while (len-- > 0) {
        if ((*p++ = *cd++) == IAC)
            *p++ = IAC;
    }
    *p++ = IAC;
    *p++ = SE;

    /* Handle Telnet Debugging Messages */
    if (deblog || tn_deb || debses) {
        int i;
        int deblen=p-str_data-2;
        char *s=NULL;
        int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK) |
            auth_crypt
#ifdef USE_INI_CRED_FWD
              | (auth_fwd?INI_CRED_FWD_ON:INI_CRED_FWD_OFF)
#endif /* USE_INI_CRED_FWD */
                    ;

        switch (type) {
        case 0:
            s = "AUTH";
            break;
        case 1:
            s = "REJECT";
            break;
        case 2:
            s = "ACCEPT";
            break;
        case 3:
            s = "RESPONSE";
            break;
        case 4:
            s = "FORWARD";
            break;
        case 5:
            s = "FORWARD_ACCEPT";
            break;
        case 6:
            s = "FORWARD_REJECT";
            break;
        case 7:
            s = "TLS_VERIFY";
                break;
        }

        ckmakxmsg(tn_msg,TN_MSG_LEN,
                  "TELNET SENT SB ",
                 TELOPT(TELOPT_AUTHENTICATION)," ",
                 str_data[3] == TELQUAL_IS ? "IS" :
                 str_data[3] == TELQUAL_REPLY ? "REPLY" : "???"," ",
                 AUTHTYPE_NAME(authentication_version)," ",
                 AUTHMODE_NAME(mode)," ",
                 s," ",NULL);
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_data[7],deblen-7);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Send data */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    rc = ttol((CHAR *)str_data, p - str_data);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    debug(F111,"SendK5AuthSB","ttol()",rc);
    return(rc);
}

/*  S E N D K 4 A U T H S B
 *  Send a Kerberos 4 Authentication Subnegotiation to host and
 *  output appropriate Telnet Debug messages
 *
 *  type - Sub Negotiation type
 *  data - ptr to buffer containing data
 *  len  - len of buffer if not NUL terminated
 *
 *  returns number of characters sent or error value
 */

static int
#ifdef CK_ANSIC
SendK4AuthSB(int type, void *data, int len)
#else
SendK4AuthSB(type,data,len) int type; void *data; int len;
#endif
{
    int rc;
    unsigned char *p = str_data + 3;
    unsigned char *cd = (unsigned char *)data;
    extern int sstelnet;
    int mode = (auth_how & AUTH_HOW_MASK) |
        auth_crypt;

    if ( type < 0 || type > 4 )         /* Check for invalid values */
        return(0);

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        if (ttchk() < 0)
          return(0);
        else
          return(1);
    }
#endif /* CK_SSL */

    if (!cd) {
        cd = (unsigned char *)"";
        len = 0;
    }

    if (len == -1)                        /* Use strlen() for len */
        len = strlen((char *)cd);


    /* Construct Message */
    *p++ = sstelnet ? TELQUAL_REPLY : TELQUAL_IS;
    *p++ = AUTHTYPE_KERBEROS_V4;
    *p = AUTH_CLIENT_TO_SERVER;
    *p |= mode;
    p++;
    *p++ = type;
    while (len-- > 0) {
        if ((*p++ = *cd++) == IAC)
            *p++ = IAC;
        }
    *p++ = IAC;
    *p++ = SE;

    /* Handle Telnet Debugging Messages */
    if (deblog || tn_deb || debses) {
        int i;
        int deblen=p-str_data-2;
        char *s=NULL;

        switch (type) {
        case 0:
            s = "AUTH";
            break;
        case 1:
            s = "REJECT";
            break;
        case 2:
            s = "ACCEPT";
            break;
        case 3:
            s = "CHALLENGE";
            break;
        case 4:
            s = "RESPONSE";
            break;
        }

        ckmakxmsg(tn_msg,TN_MSG_LEN,"TELNET SENT SB ",
                 TELOPT(TELOPT_AUTHENTICATION)," ",
                 str_data[3] == TELQUAL_IS ? "IS" :
                 (str_data[3] == TELQUAL_REPLY ? "REPLY" : "???")," ",
                 AUTHTYPE_NAME(authentication_version)," ",
                 AUTHMODE_NAME(mode)," ",
                 s," ",NULL);
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_data[7],deblen-7);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Send data */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    rc = ttol((CHAR *)str_data, p - str_data);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    debug(F111,"SendK4AuthSB","ttol()",rc);
    return(rc);
}

/*  S E N D S R P A U T H S B
 *  Send a SRP Authentication Subnegotiation to host and
 *  output appropriate Telnet Debug messages
 *
 *  type - Sub Negotiation type
 *  data - ptr to buffer containing data
 *  len  - len of buffer if not NUL terminated
 *
 *  returns number of characters sent or error value
 */

static int
#ifdef CK_ANSIC
SendSRPAuthSB(int type, void *data, int len)
#else
SendSRPAuthSB(type,data,len) int type; void *data; int len;
#endif
{
    int rc;
    unsigned char *p = str_data + 3;
    unsigned char *cd = (unsigned char *)data;
    extern int sstelnet;

    /* Check for invalid values */
    if ( type != SRP_EXP && type != SRP_RESPONSE &&
         type != SRP_REJECT && type != SRP_ACCEPT &&
         type != SRP_CHALLENGE && type != SRP_PARAMS &&
         type != SRP_AUTH)
        return(0);

    if (len == -1)                        /* Use strlen() for len */
        len = strlen((char *)cd);

    /* Construct Message */
    *p++ = sstelnet ? TELQUAL_REPLY : TELQUAL_IS;
    *p++ = AUTHTYPE_SRP;
    *p = AUTH_CLIENT_TO_SERVER;
    *p |= auth_how;
#ifdef CK_ENCRYPTION
    *p |= auth_crypt;
#endif
    p++;
    *p++ = type;
    while (len-- > 0) {
        if ((*p++ = *cd++) == IAC)
            *p++ = IAC;
        }
    *p++ = IAC;
    *p++ = SE;

    /* Handle Telnet Debugging Messages */
    if (deblog || tn_deb || debses) {
        int i;
        int deblen=p-str_data-2;
        char *s=NULL;
        int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK) |
            auth_crypt;

        switch (type) {
        case 0:
            s = "AUTH";
            break;
        case 1:
            s = "REJECT";
            break;
        case 2:
            s = "ACCEPT";
            break;
        case 3:
            s = "CHALLENGE";
            break;
        case 4:
            s = "RESPONSE";
            break;
        case 5:
            s = "FORWARD";
            break;
        case 6:
            s = "FORWARD_ACCEPT";
            break;
        case 7:
            s = "FORWARD_REJECT";
            break;
        case 8:
            s = "EXP";
            break;
        case 9:
            s = "PARAMS";
            break;
        }

        ckmakxmsg(tn_msg,TN_MSG_LEN,
                  "TELNET SENT SB ",
                 TELOPT(TELOPT_AUTHENTICATION)," ",
                 str_data[3] == TELQUAL_REPLY ? "REPLY" :
                 str_data[3] == TELQUAL_IS ? "IS" : "???"," ",
                 AUTHTYPE_NAME(authentication_version)," ",
                 AUTHMODE_NAME(mode)," ",
                 s," ",NULL);
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_data[7],deblen-7);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Send data */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    rc = ttol((CHAR *)str_data, p - str_data);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    return(rc);
}

#ifdef CK_ENCRYPTION
/*
 * Function: Enable or disable the encryption process.
 *
 * Parameters:
 *      enable - TRUE to enable, FALSE to disable.
 */
static VOID
#ifdef CK_ANSIC
auth_encrypt_enable(BOOL enable)
#else
auth_encrypt_enable(enable) BOOL enable;
#endif
{
  encrypt_flag = enable;
}
#endif

/*
 * Function: Abort the authentication process
 *
 * Parameters:
 */
static VOID
#ifdef CK_ANSIC
auth_abort(char *errmsg, long r)
#else
auth_abort(errmsg,r) char *errmsg; long r;
#endif
{
    char buf[9];
    extern int sstelnet;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        return;
    }
#endif /* CK_SSL */
    debug(F111,"auth_abort",errmsg,r);

    /* Construct Telnet Debugging messages */
    if (deblog || tn_deb || debses) {
        ckmakxmsg(tn_msg,TN_MSG_LEN,
                  "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION),
                  " IS ",AUTHTYPE_NAME(AUTHTYPE_NULL)," ",
                   AUTHTYPE_NAME(AUTHTYPE_NULL)," IAC SE",
                   NULL,NULL,NULL,NULL,NULL
                 );
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Construct the Abort message to send to the host   */
    /* Basicly we change the authentication type to NULL */
    sprintf(buf, "%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_AUTHENTICATION,
             sstelnet ? TELQUAL_REPLY : TELQUAL_IS, AUTHTYPE_NULL,
             AUTHTYPE_NULL, IAC, SE);   /* safe */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol((CHAR *)buf, 8);
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    /* If there is an error message, and error number construct */
    /* an explanation to display to the user                    */
    if (errmsg != NULL) {
        ckstrncpy(strTmp, errmsg, AUTHTMPBL);
    } else
        strTmp[0] = '\0';


    if (r != AUTH_SUCCESS) {
        ckstrncat(strTmp, "\r\n",AUTHTMPBL);
#ifdef KRB4
        if ( authentication_version == AUTHTYPE_KERBEROS_V4 ) {
            ckstrncat(strTmp, (char *)krb_get_err_text_entry(r),
                       AUTHTMPBL);
            debug(F111,"auth_abort",(char *)krb_get_err_text_entry(r),r);
        }
#endif
#ifdef KRB5
        if ( authentication_version == AUTHTYPE_KERBEROS_V5 ) {
            ckstrncat(strTmp, error_message(r),AUTHTMPBL);
            debug(F111,"auth_abort",error_message(r),r);
        }
#endif
    }
    printf("Authentication failed: %s\r\n",strTmp);
#ifdef CKSYSLOG
    if (ckxsyslog >= SYSLG_LI && ckxlogging) {
        cksyslog(SYSLG_LI, 0, "Telnet authentication failure",
                  (char *) szUserNameRequested,
                  strTmp);
    }
#endif /* CKSYSLOG */
    authentication_version = AUTHTYPE_NULL;
}


/*
 * Function: Copy data to buffer, doubling IAC character if present.
 *
 */
int
#ifdef CK_ANSIC
copy_for_net(unsigned char *to, unsigned char *from, int c)
#else
copy_for_net(to,from,c) unsigned char *to; unsigned char *from; int c;
#endif
{
    int n;

    n = c;
    debug(F111,"copy_for_net","before",n);
    while (c-- > 0) {
        if ((*to++ = *from++) == IAC) {
            n++;
            *to++ = IAC;
        }
    }
    debug(F111,"copy_for_net","after",n);
    return n;
}

#ifdef CK_SSL
/*  S E N D S S L A U T H S B
 *  Send a SSL Authentication Subnegotiation to host and
 *  output appropriate Telnet Debug messages
 *
 *  type - Sub Negotiation type
 *  data - ptr to buffer containing data
 *  len  - len of buffer if not NUL terminated
 *
 *  returns number of characters sent or error value
 */

int
#ifdef CK_ANSIC
SendSSLAuthSB(int type, void *data, int len)
#else
SendSSLAuthSB(type,data,len) int type; void *data; int len;
#endif
{
    int rc;
    unsigned char *p = str_data + 3;
    unsigned char *cd = (unsigned char *)data;
    extern int sstelnet;

    /* Check for invalid values */
    if ( type != SSL_START && type != SSL_ACCEPT &&
         type != SSL_REJECT)
        return(0);

    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows) {
        if (ttchk() < 0)
          return(0);
        else
          return(1);
    }

    if (len == -1)                        /* Use strlen() for len */
        len = strlen((char *)cd);

    /* Construct Message */
    *p++ = sstelnet ? TELQUAL_REPLY : TELQUAL_IS;
    *p++ = AUTHTYPE_SSL;
    *p = AUTH_CLIENT_TO_SERVER;
    *p |= auth_how;
#ifdef CK_ENCRYPTION
    *p |= auth_crypt;
#endif
    p++;
    *p++ = type;
    while (len-- > 0) {
        if ((*p++ = *cd++) == IAC)
            *p++ = IAC;
        }
    *p++ = IAC;
    *p++ = SE;

    /* Handle Telnet Debugging Messages */
    if (deblog || tn_deb || debses) {
        int i;
        int deblen=p-str_data-2;
        char *s=NULL;
        int mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK) |
            (auth_crypt?AUTH_ENCRYPT_USING_TELOPT:AUTH_ENCRYPT_OFF);

        switch (type) {
        case SSL_START:
            s = "START";
            break;
        case SSL_ACCEPT:
            s = "ACCEPT";
            break;
        case SSL_REJECT:
            s = "REJECT";
            break;
        }

        ckmakxmsg(tn_msg,TN_MSG_LEN,
                  "TELNET SENT SB ",
                 TELOPT(TELOPT_AUTHENTICATION)," ",
                 str_data[3] == TELQUAL_REPLY ? "REPLY" :
                 str_data[3] == TELQUAL_IS ? "IS" : "???"," ",
                 AUTHTYPE_NAME(authentication_version)," ",
                 AUTHMODE_NAME(mode)," ",
                 s," ",NULL);
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_data[7],deblen-7);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }

    /* Send data */
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    rc = ttol((CHAR *)str_data, p - str_data);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    return(rc);
}
#endif  /* CK_SSL */

int
tn_how_ok(int how)
{
    switch ( tn_auth_how ) {
    case TN_AUTH_HOW_ANY:
        return(1);
    case TN_AUTH_HOW_ONE_WAY:
        return((how & AUTH_HOW_MASK) == AUTH_HOW_ONE_WAY);
    case TN_AUTH_HOW_MUTUAL:
        return((how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL);
    default:
        return(0);
    }
}

int
tn_enc_ok(int enc)
{
    switch ( tn_auth_enc ) {
    case TN_AUTH_ENC_ANY:
        if ((enc & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS &&
            (!ck_ssleay_is_installed()
#ifdef CK_SSL
             || !ssl_finished_messages ||
             !(tls_active_flag || ssl_active_flag)
#endif /* CK_SSL */
             )) {
#ifdef CK_SSL
            if (!ssl_finished_messages)
                debug(F100,"tn_enc_ok !ssl_finished_messages","",0);
#endif /* CK_SSL */
            return(0);
        }
        return(1);
    case TN_AUTH_ENC_NONE:
        return((enc & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_OFF);
    case TN_AUTH_ENC_TELOPT:
        return((enc & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_USING_TELOPT);
    case TN_AUTH_ENC_EXCH:
        return((enc & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_AFTER_EXCHANGE);
    case TN_AUTH_ENC_TLS:
        return(((enc & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) &&
               ck_ssleay_is_installed()
#ifdef CK_SSL
               && ssl_finished_messages &&
               (tls_active_flag || ssl_active_flag)
#endif /* CK_SSL */
           );
    default:
        return(0);
    }
}

static int
atok(int at) {
    int i;
    if ( auth_type_user[0] == AUTHTYPE_AUTO )
        return(1);
    if ( auth_type_user[0] == AUTHTYPE_NULL )
        return(0);

    for ( i=0;
          i<AUTHTYPLSTSZ && auth_type_user[i] != AUTHTYPE_NULL;
          i++ ) {
        if ( auth_type_user[i] == at )
            return(1);
    }

    return(0);
}


/*
 * Function: Parse authentication send command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *      end_sub - index of the character in the 'parsedat' array which
 *              is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */

static unsigned char send_list[512];
static int  send_len = 0;

_PROTOTYP(static int auth_send, (unsigned char *parsedat, int end_sub));

static int
#ifdef CK_ANSIC
auth_resend(int type)
#else
auth_resend(type) int type;
#endif /* CK_ANSIC */
{
    int i=2;
    while (i+1 <= send_len) {
        if (send_list[i] == type) {
            int j;
            send_len -= 2;
            for (j = i; j < send_len; j++)
                send_list[j] = send_list[j+2];
        } else {
            i += 2;
        }
    }
    return(auth_send(send_list,send_len));
}

static int
#ifdef CK_ANSIC
auth_send(unsigned char *parsedat, int end_sub)
#else
auth_send(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
    static unsigned char buf[4096];
    unsigned char *pname;
    int plen;
    int r;
    int i;
    int mode;
#ifdef MIT_CURRENT
#ifdef CK_ENCRYPTION
    krb5_data data;
    krb5_enc_data encdata;
    krb5_error_code code;
    krb5_keyblock random_key;
#endif /* ENCRYPTION */
#endif /* MIT_CURRENT */
#ifdef KRB5
    int krb5_msg = 0;
#endif /* KRB5 */
#ifdef KRB4
    int krb4_msg = 0;
#endif /* KRB4 */
#ifdef GSSAPI_KRB5
    int gssk5_msg = 0;
#endif /* GSSAPI_KRB5 */
    int iaccnt=0;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return(AUTH_SUCCESS);
#endif /* CK_SSL */

    auth_how = -1;              /* We have not found an auth method  */
    auth_crypt = 0;             /* We are not using encryption (yet) */
    send_len = end_sub > 512 ? 512 : end_sub;
    memcpy(send_list,parsedat,send_len);

    /* Search the list of acceptable Authentication types sent from */
    /* the host and find one that we support                        */

    /* For Kerberos authentications, try to determine if we have a  */
    /* valid TGT, if not skip over the authentication type because  */
    /* we wouldn't be able to successfully login anyway.  Perhaps   */
    /* there is another supported authentication which we could use */

#ifdef NO_FTP_AUTH
    /* If the userid is "ftp" or "anonymous" refuse to perform AUTH */
    /* for Kerberos or SRP.                                         */
#endif /* NO_FTP_AUTH */

    if ( auth_type_user[0] == AUTHTYPE_AUTO ) {
    for (i = 2; i+1 <= end_sub; i += 2) {
#ifdef NTLM
        if (parsedat[i] == AUTHTYPE_NTLM &&
             ck_ntlm_is_valid(1) &&
             ntlm_auth_send() == 0) {
            if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                 tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef CK_ENCRYPTION
                /* NTLM does not support Telnet Encryption */
                if ((parsedat[i+1] & AUTH_ENCRYPT_MASK))
                    continue;
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#endif /* CK_ENCRYPTION */
                TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                authentication_version = AUTHTYPE_NTLM;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* NTLM */
#ifdef CK_SSL
        if ( parsedat[i] == AUTHTYPE_SSL && ssl_initialized &&
#ifdef SSLDLL
             ck_ssleay_is_installed() &&
#endif /* SSLDLL */
             !tls_active_flag && !ssl_active_flag
#ifndef USE_CERT_CB
             && tls_load_certs(ssl_ctx,ssl_con,0)
#endif /* USE_CERT_CB */
             ) {

            if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                 tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef CK_ENCRYPTION
                /* SSL does not support Telnet Encryption */
                if ((parsedat[i+1] & AUTH_ENCRYPT_MASK))
                    continue;
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#endif /* CK_ENCRYPTION */
                TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                authentication_version = AUTHTYPE_SSL;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* SSL */
#ifdef CK_SRP
        if ( parsedat[i] == AUTHTYPE_SRP
#ifdef SRPDLL
             && hSRP
#endif /* SRPDLL */
#ifdef NO_FTP_AUTH
             && strcmp("ftp",szUserName) && strcmp("anonymous",szUserName)
#endif /* NO_FTP_AUTH */
             ) {
            if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                 tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef PRE_SRP_1_4_5
                if (parsedat[i+1] & AUTH_ENCRYPT_MASK)
                     /* Do not support ENCRYPT_USING_TELOPT yet. */
                    continue;
#endif /* PRE_SRP_1_4_5 */
                if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                     AUTH_ENCRYPT_USING_TELOPT) &&
                    (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                     TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                    continue;

                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                if ( auth_crypt == AUTH_ENCRYPT_USING_TELOPT ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                }
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
                if ( auth_crypt == AUTH_ENCRYPT_START_TLS &&
                     ck_ssleay_is_installed() &&
                     (tls_active_flag || ssl_active_flag) ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
#endif /* CK_SSL */
                authentication_version = AUTHTYPE_SRP;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* SRP */
#ifdef GSSAPI_KRB5
        if (parsedat[i] == AUTHTYPE_GSSAPI_KRB5 &&
            (parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
            AUTH_ENCRYPT_AFTER_EXCHANGE &&
#ifdef OS2
            hGSSAPI &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
             strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
             ck_gssapi_is_installed() && !gssk5_msg)
        {
            if ( !gssk5_auth_send(parsedat[i+1] & AUTH_HOW_MASK,
                                  parsedat[i+1] & AUTH_ENCRYPT_MASK,
                                  parsedat[i+1] & INI_CRED_FWD_MASK) )
            {
                /* If we are auto-getting TGTs, try */
                if ( !ck_krb5_is_tgt_valid() ) {
                printf("Kerberos 5: Ticket Getting Ticket not valid.\r\n");
                }
                gssk5_msg = 1;
            }
            else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                      AUTH_CLIENT_TO_SERVER &&
                      tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                if ( auth_crypt == AUTH_ENCRYPT_AFTER_EXCHANGE ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
#endif /* CK_ENCRYPTION */
                auth_fwd = parsedat[i+1] & INI_CRED_FWD_MASK;
                authentication_version = AUTHTYPE_GSSAPI_KRB5;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* GSSAPI_KRB5 */
#ifdef KRB5
        if (parsedat[i] == AUTHTYPE_KERBEROS_V5 &&
#ifdef OS2
             hKRB5_32 &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
             strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
             ck_krb5_is_installed() && !krb5_msg) {

            /* Without encryption we can't perform mutual authentication */
            if ( (parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL &&
                 !ck_crypt_is_installed())
                continue;

            /* Skip over entries that request credential forwarding */
            /* if we are not forwarding.                            */
            if ((!forward_flag && (parsedat[i+1] & INI_CRED_FWD_MASK)) ||
                (forward_flag &&
                  ((parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_ONE_WAY)))
                continue;

            if ( !k5_auth_send(parsedat[i+1] & AUTH_HOW_MASK,
                                parsedat[i+1] & AUTH_ENCRYPT_MASK,
                                parsedat[i+1] & INI_CRED_FWD_MASK) )
            {
                /* If we are auto-getting TGTs, try */
                if ( !ck_krb5_is_tgt_valid() ) {
                printf("Kerberos 5: Ticket Getting Ticket not valid.\r\n");
                }
                krb5_msg = 1;
            }
            else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                      AUTH_CLIENT_TO_SERVER &&
                      tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
                if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                     AUTH_ENCRYPT_USING_TELOPT) &&
                     (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                       TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                    continue;
                if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                     AUTH_ENCRYPT_START_TLS) &&
                     (!ck_ssleay_is_installed()
#ifdef CK_SSL
                       || !(tls_active_flag || ssl_active_flag)
#endif /* CK_SSL */
                       ))
                    continue;

                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                if ( auth_crypt == AUTH_ENCRYPT_USING_TELOPT ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                }
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
                if ( auth_crypt == AUTH_ENCRYPT_START_TLS &&
                     ck_ssleay_is_installed() &&
                     (tls_active_flag || ssl_active_flag) ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
#endif /* CK_SSL */
                auth_fwd = parsedat[i+1] & INI_CRED_FWD_MASK;
                authentication_version = AUTHTYPE_KERBEROS_V5;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                if ( auth_how == AUTH_HOW_ONE_WAY ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
                break;
            }
        }
#endif /* KRB5 */
#ifdef KRB4
        if (parsedat[i] == AUTHTYPE_KERBEROS_V4 &&
#ifdef OS2
             hKRB4_32 &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
             strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
             ck_krb4_is_installed() && !krb4_msg) {
            int rc = 0;

            /* Without encryption we can't perform mutual authentication */
            if ( (parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL &&
                 !ck_crypt_is_installed() )
                continue;

            if ( !k4_auth_send() )
            {
                /* If we are auto-getting TGTs, try */
                if ( !ck_krb4_is_tgt_valid() ) {
                    printf("Kerberos 4: Ticket Getting Ticket not valid.\r\n");
                }
                krb4_msg = 1;
            }
            else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                      AUTH_CLIENT_TO_SERVER &&
                      tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef CK_ENCRYPTION
                if ((parsedat[i+1] & AUTH_ENCRYPT_MASK) &&
                     (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                       TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                    continue;
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
                if ( auth_crypt == AUTH_ENCRYPT_USING_TELOPT ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                }
#endif /* CK_ENCRYPTION */
                authentication_version = AUTHTYPE_KERBEROS_V4;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                if ( auth_how == AUTH_HOW_ONE_WAY ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
                break;
            }
        }
#endif /* KRB4 */
    }
    } else {
        for (i = 2; i+1 <= end_sub; i += 2) {
#ifdef CK_SSL
            if ( atok(AUTHTYPE_SSL) && parsedat[i] == AUTHTYPE_SSL &&
#ifdef SSLDLL
                 ck_ssleay_is_installed() &&
#endif /* SSLDLL */
                 !tls_active_flag && !ssl_active_flag && ssl_initialized
#ifndef USE_CERT_CB
                 && tls_load_certs(ssl_ctx,ssl_con,0)
#endif /* USE_CERT_CB */
                 )
            {
                if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                     tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef CK_ENCRYPTION
                    /* SSL does not support Telnet Encryption */
                    if ((parsedat[i+1] & AUTH_ENCRYPT_MASK))
                        continue;
                    auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#endif /* CK_ENCRYPTION */
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    authentication_version = AUTHTYPE_SSL;
                    auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                    break;
                }
            }
#endif /* SSL */
#ifdef CK_SRP
            if ( atok(AUTHTYPE_SRP) &&
                 parsedat[i] == AUTHTYPE_SRP
#ifdef SRPDLL
                 && hSRP
#endif /* SRPDLL */
#ifdef NO_FTP_AUTH
                 && strcmp("ftp",szUserName) && strcmp("anonymous",szUserName)
#endif /* NO_FTP_AUTH */
                 ) {
                if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                     tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef PRE_SRP_1_4_5
                if (parsedat[i+1] & AUTH_ENCRYPT_MASK)
                     /* Do not support ENCRYPT_USING_TELOPT yet. */
                    continue;
#endif /* PRE_SRP_1_4_5 */
                if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                     AUTH_ENCRYPT_USING_TELOPT) &&
                     (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                       TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                    continue;
                if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                     AUTH_ENCRYPT_START_TLS) &&
                     (!ck_ssleay_is_installed()
#ifdef CK_SSL
                       || !(tls_active_flag || ssl_active_flag)
#endif /* CK_SSL */
                       ))
                    continue;
                    auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                    if ( auth_crypt == AUTH_ENCRYPT_USING_TELOPT ) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    }
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
                if ( auth_crypt == AUTH_ENCRYPT_START_TLS &&
                     ck_ssleay_is_installed() &&
                     (tls_active_flag || ssl_active_flag) ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
#endif /* CK_SSL */
                    authentication_version = AUTHTYPE_SRP;
                    auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                    break;
                }
            }
#endif /* SRP */
#ifdef GSSAPI_KRB5
        if (atok(AUTHTYPE_GSSAPI_KRB5) &&
            parsedat[i] == AUTHTYPE_GSSAPI_KRB5 &&
            (parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
            AUTH_ENCRYPT_AFTER_EXCHANGE &&
#ifdef OS2
            hGSSAPI &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
            strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
            ck_gssapi_is_installed() && !gssk5_msg)
        {
            if ( !gssk5_auth_send(parsedat[i+1] & AUTH_HOW_MASK,
                                  parsedat[i+1] & AUTH_ENCRYPT_MASK,
                                  parsedat[i+1] & INI_CRED_FWD_MASK) )
            {
                /* If we are auto-getting TGTs, try */
                if ( !ck_krb5_is_tgt_valid() ) {
                printf("Kerberos 5: Ticket Getting Ticket not valid.\r\n");
                }
                gssk5_msg = 1;
            }
            else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                      AUTH_CLIENT_TO_SERVER &&
                      tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                if ( auth_crypt == AUTH_ENCRYPT_AFTER_EXCHANGE ) {
                    TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                }
#endif /* CK_ENCRYPTION */
                auth_fwd = parsedat[i+1] & INI_CRED_FWD_MASK;
                authentication_version = AUTHTYPE_GSSAPI_KRB5;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* GSSAPI_KRB5 */
#ifdef KRB5
            if ( atok(AUTHTYPE_KERBEROS_V5) &&
                 parsedat[i] == AUTHTYPE_KERBEROS_V5 &&
#ifdef OS2
                 hKRB5_32 &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
                 strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
                 ck_krb5_is_installed() && !krb5_msg) {

                /* Without encryption we can't perform mutual authentication */
                if ( (parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL &&
                     !ck_crypt_is_installed())
                    continue;

                /* Skip over entries that request credential forwarding */
                /* if we are not forwarding.                            */
                if ((!forward_flag && (parsedat[i+1] & INI_CRED_FWD_MASK)) ||
                     (forward_flag &&
                       ((parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_ONE_WAY)))
                    continue;

                if ( !k5_auth_send(parsedat[i+1] & AUTH_HOW_MASK,
                                    parsedat[i+1] & AUTH_ENCRYPT_MASK,
                                    parsedat[i+1] & INI_CRED_FWD_MASK) )
                {
                    /* If we are auto-getting TGTs, try */
                    if ( !ck_krb5_is_tgt_valid() ) {
                        printf(
                           "Kerberos 5: Ticket Getting Ticket not valid.\r\n");
                    }
                    krb5_msg = 1;
                }
                else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                          AUTH_CLIENT_TO_SERVER &&
                          tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1]))
                {
                    if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                         AUTH_ENCRYPT_USING_TELOPT) &&
                         (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                           TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                        continue;
                    if (((parsedat[i+1] & AUTH_ENCRYPT_MASK) ==
                         AUTH_ENCRYPT_START_TLS) &&
                         (!ck_ssleay_is_installed()
#ifdef CK_SSL
                           || !(tls_active_flag || ssl_active_flag)
#endif /* CK_SSL */
                           ))
                        continue;
                    auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#ifdef CK_ENCRYPTION
                    if (auth_crypt) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    }
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
                    if ( auth_crypt == AUTH_ENCRYPT_START_TLS &&
                         ck_ssleay_is_installed() &&
                         (tls_active_flag || ssl_active_flag) ) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    }
#endif /* CK_SSL */
                    authentication_version = AUTHTYPE_KERBEROS_V5;
                    auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                    if ( auth_how == AUTH_HOW_ONE_WAY ) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    }
                    break;
                }
            }
#endif /* KRB5 */
#ifdef KRB4
            if ( atok(AUTHTYPE_KERBEROS_V4) &&
                 parsedat[i] == AUTHTYPE_KERBEROS_V4 &&
#ifdef OS2
                 hKRB4_32 &&
#endif /* OS2 */
#ifdef NO_FTP_AUTH
                 strcmp("ftp",szUserName) && strcmp("anonymous",szUserName) &&
#endif /* NO_FTP_AUTH */
                 ck_krb4_is_installed() && !krb4_msg) {
                int rc = 0;

                /* Without encryption we can't perform mutual authentication */
                if ( (parsedat[i+1] & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL &&
                     !ck_crypt_is_installed())
                    continue;

                if ( !k4_auth_send() )
                {
                    /* If we are auto-getting TGTs, try */
                    if ( !ck_krb4_is_tgt_valid() ) {
                    printf("Kerberos 4: Ticket Getting Ticket not valid.\r\n");
                    }
                    krb4_msg = 1;
                }
                else if ((parsedat[i+1] & AUTH_WHO_MASK) ==
                          AUTH_CLIENT_TO_SERVER &&
                          tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1]))
                {
#ifdef CK_ENCRYPTION
                    if ((parsedat[i+1] & AUTH_ENCRYPT_MASK) &&
                        (TELOPT_ME_MODE(TELOPT_ENCRYPTION) == TN_NG_RF ||
                         TELOPT_U_MODE(TELOPT_ENCRYPTION) == TN_NG_RF))
                      continue;
                    auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
                    if (auth_crypt) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_MU;
                    }
#endif /* CK_ENCRYPTION */
                    authentication_version = AUTHTYPE_KERBEROS_V4;
                    auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                    if ( auth_how == AUTH_HOW_ONE_WAY ) {
                        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                    }
                    break;
                }
            }
#endif /* KRB4 */
#ifdef NTLM
        if ( atok(AUTHTYPE_NTLM) &&
             parsedat[i] == AUTHTYPE_NTLM &&
             ck_ntlm_is_valid(1) &&
             ntlm_auth_send() == 0) {
            if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER &&
                 tn_how_ok(parsedat[i+1]) && tn_enc_ok(parsedat[i+1])) {
#ifdef CK_ENCRYPTION
                /* NTLM does not support Telnet Encryption */
                if ((parsedat[i+1] & AUTH_ENCRYPT_MASK))
                    continue;
                auth_crypt = parsedat[i+1] & AUTH_ENCRYPT_MASK;
#endif /* CK_ENCRYPTION */
                TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
                authentication_version = AUTHTYPE_NTLM;
                auth_how = parsedat[i+1] & AUTH_HOW_MASK;
                break;
            }
        }
#endif /* NTLM */
        }
    }

    if (auth_how == -1) {               /* Did we find one? */
        switch ( auth_type_user[0] ) {  /* If not, abort the negotiation */
        case AUTHTYPE_NULL:
            auth_abort("User refused to accept any authentication method",0);
            break;
        case AUTHTYPE_AUTO:
            auth_abort("No authentication method available", 0);
            break;
        default: {
            char msg[80];
            ckmakmsg(msg,80,AUTHTYPE_NAME(auth_type_user[0]),
                      " could not be negotiated",NULL,NULL
                     );
            auth_abort(msg, 0);
        }
        }
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    printf("Authenticating with %s\r\n",
            AUTHTYPE_NAME(authentication_version));

    /* Send Telnet Auth Name message (if necessary) */
    switch ( authentication_version ) {
    case AUTHTYPE_SRP:
    case AUTHTYPE_KERBEROS_V4:
    case AUTHTYPE_KERBEROS_V5:
    case AUTHTYPE_GSSAPI_KRB5:
        /* if we do not have a name to login with get one now. */
        while ( szUserName[0] == '\0' ) {
            extern char * tn_pr_uid;
            int ok = uq_txt(NULL,
                     tn_pr_uid && tn_pr_uid[0] ? tn_pr_uid : "Host Userid: ",
                            1, NULL, szUserName, 63, NULL,DEFAULT_UQ_TIMEOUT);
            if ( !ok )
                return AUTH_FAILURE;
        }
        plen = strlen(szUserName);
        pname = (unsigned char *) szUserName;

        /* Construct Telnet Debugging Message */
        if (deblog || tn_deb || debses) {
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                       "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION),
                       " NAME ",(char *)pname," IAC SE",NULL,
                       NULL,NULL,NULL,NULL,NULL,NULL
                     );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }

        /* Construct and send Authentication Name subnegotiation */
        if ( plen < sizeof(buf) - 6 ) {
            sprintf((char *)buf, "%c%c%c%c", IAC, SB, 
                     TELOPT_AUTHENTICATION,
                     TELQUAL_NAME);
            memcpy(&buf[4], pname, plen);               /* safe */
            sprintf((char *)&buf[plen + 4], "%c%c", IAC, SE);   /* safe */
#ifdef OS2
            RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
            ttol((CHAR *)buf, plen+6);
#ifdef OS2
            ReleaseTelnetMutex();
#endif
        } else {
            sprintf((char *)buf, "%c%c%c%c%c%c", IAC, SB, 
                     TELOPT_AUTHENTICATION,
                     TELQUAL_NAME, IAC, SE);    /* safe */
#ifdef OS2
            RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
            ttol((CHAR *)buf, 6);
#ifdef OS2
            ReleaseTelnetMutex();
#endif
        }
    }

    /* Construct Authentication Mode subnegotiation message (if necessary) */
    switch ( authentication_version ) {
    case AUTHTYPE_SRP:
    case AUTHTYPE_KERBEROS_V4:
    case AUTHTYPE_KERBEROS_V5:
    case AUTHTYPE_GSSAPI_KRB5:
    case AUTHTYPE_NTLM:
        mode = AUTH_CLIENT_TO_SERVER | (auth_how & AUTH_HOW_MASK) | auth_crypt
#ifdef USE_INI_CRED_FWD
               | (((authentication_version == AUTHTYPE_KERBEROS_V5) &&
                  auth_fwd)?INI_CRED_FWD_ON:INI_CRED_FWD_OFF)
#endif /* USE_INI_CRED_FWD */
               ;
        sprintf((char *)buf, "%c%c%c%c%c%c%c",
                 IAC, SB, TELOPT_AUTHENTICATION,
                 TELQUAL_IS,
                 authentication_version,
                 mode,
                 KRB_AUTH);     /* safe */
        break;
    }

    /* Send initial authentication data */
    switch ( authentication_version ) {
#ifdef CK_SSL
    case AUTHTYPE_SSL:
        SendSSLAuthSB(SSL_START,NULL,0);
        break;
#endif /* SSL */
#ifdef CK_SRP
    case AUTHTYPE_SRP:
        sprintf(&buf[7], "%c%c", IAC, SE);      /* safe */
        if (deblog || tn_deb || debses) {
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION),
                      " IS ",AUTHTYPE_NAME(authentication_version),
                      " AUTH ",AUTHMODE_NAME(mode)," IAC SE",
                      NULL,NULL,NULL,NULL,NULL
                     );
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((CHAR *)buf, 9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
#endif /* SRP */
#ifdef NTLM
    case AUTHTYPE_NTLM: {
        int length = 0;

        for ( i=0 ; i<NTLMSecBuf[0].cbBuffer ; i++ ) {
            if ( ((char *)NTLMSecBuf[0].pvBuffer)[i] == IAC )
                iaccnt++;
        }

        if ( ( 2*sizeof(ULONG) + NTLMSecBuf[0].cbBuffer + iaccnt + 10)  <
             sizeof(buf) ) {
            length = copy_for_net(&buf[7],(char *)&NTLMSecBuf[0],
                                   2*sizeof(ULONG));
            length += copy_for_net(&buf[7+length], NTLMSecBuf[0].pvBuffer,
                                  NTLMSecBuf[0].cbBuffer);
        }
        sprintf(&buf[7+length], "%c%c", IAC, SE);

        if (deblog || tn_deb || debses) {
            int i;
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION),
                      " IS ",AUTHTYPE_NAME(authentication_version)," ",
                      AUTHMODE_NAME(mode)," NTLM_AUTH ",
                       NULL,NULL,NULL,NULL,NULL
                      );
            tn_hex((char *)tn_msg,TN_MSG_LEN,&buf[7],length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((CHAR *)buf, length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    }
#endif /* NTLM */
#ifdef KRB4
    case AUTHTYPE_KERBEROS_V4:
        for ( i=0 ; i<k4_auth.length ; i++ ) {
            if ( k4_auth.dat[i] == IAC )
                iaccnt++;
        }

        if ( k4_auth.length + iaccnt + 10 < sizeof(buf) )
          k4_auth.length = copy_for_net(&buf[7], k4_auth.dat, k4_auth.length);
        else
            k4_auth.length = 0;
        sprintf(&buf[k4_auth.length+7], "%c%c", IAC, SE);

        if (deblog || tn_deb || debses) {
            int i;
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION)," IS ",
                      AUTHTYPE_NAME(authentication_version)," ",
                      AUTHMODE_NAME(mode)," AUTH ",
                      NULL,NULL,NULL,NULL,NULL
                     );
            tn_hex((char *)tn_msg,TN_MSG_LEN,&buf[7],k4_auth.length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((CHAR *)buf, k4_auth.length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif

#ifndef REMOVE_FOR_EXPORT
#ifdef CK_ENCRYPTION
        /*
         * If we are doing mutual authentication, get set up to send
         * the challenge, and verify it when the response comes back.
         */
        if ((auth_how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL) {
            register int i;
            int rc = 0;

#ifdef MIT_CURRENT
            data.data = cred.session;
            data.length = 8; /* sizeof(cred.session) */;

            if (code = krb5_c_random_seed(k5_context, &data)) {
                com_err("libtelnet", code,
                         "while seeding random number generator");
                return(0);
            }

            if (code = krb5_c_make_random_key(k5_context,
                                               ENCTYPE_DES_CBC_RAW,
                                               &random_key)) {
                com_err("libtelnet", code,
                         "while creating random session key");
                return(0);
            }

            /* the krb4 code uses ecb mode, but on a single block
            with a zero ivec, ecb and cbc are the same */
            k4_krbkey.enctype = ENCTYPE_DES_CBC_RAW;
            k4_krbkey.length = 8;
            k4_krbkey.contents = cred.session;

            encdata.ciphertext.data = random_key.contents;
            encdata.ciphertext.length = random_key.length;
            encdata.enctype = ENCTYPE_UNKNOWN;

            data.data = k4_session_key;
            data.length = 8;

            code = krb5_c_decrypt(k5_context, &k4_krbkey, 0, 0,
                                   &encdata, &data);

            krb5_free_keyblock_contents(k5_context, &random_key);

            if (code) {
                com_err("libtelnet", code, "while encrypting random key");
                return(0);
            }

            encdata.ciphertext.data = k4_session_key;
            encdata.ciphertext.length = 8;
            encdata.enctype = ENCTYPE_UNKNOWN;

            data.data = k4_challenge;
            data.length = 8;

            code = krb5_c_decrypt(k5_context, &k4_krbkey, 0, 0,
                                   &encdata, &data);
#else /* MIT_CURRENT */
            memset(k4_sched,0,sizeof(Schedule));
            ckhexdump("auth_send",cred.session,8);
            rc = des_key_sched(cred.session, k4_sched);
            if ( rc == -1 ) {
                printf("?Invalid DES key specified in credentials\r\n");
                debug(F110,"auth_send",
                      "invalid DES Key specified in credentials",0);
            } else if ( rc == -2 ) {
                printf("?Weak DES key specified in credentials\r\n");
                debug(F110,"auth_send",
                      "weak DES Key specified in credentials",0);
            } else if ( rc != 0 ) {
                printf("?DES Key Schedule not set by credentials\r\n");
                debug(F110,"auth_send",
                      "DES Key Schedule not set by credentials",0);
            }
            ckhexdump("auth_send schedule",k4_sched,8*16);

            des_set_random_generator_seed(cred.session);

            do {
                des_new_random_key(k4_session_key);
                des_fixup_key_parity(k4_session_key);
            } while ( ck_des_is_weak_key(k4_session_key) );

            ckhexdump("auth_send des_new_random_key(k4_session_key)",
                     k4_session_key,8);

            /* Decrypt the session key so that we can send it to the */
            /* host as a challenge                                   */
#ifdef NT
            des_ecb_encrypt(k4_session_key, k4_session_key, k4_sched, 0);
#else /* NT */
            des_ecb_encrypt(&k4_session_key, &k4_session_key, k4_sched, 0);
#endif /* NT */
            ckhexdump(
                "auth_send des_ecb_encrypt(k4_session_key,k4_session_key,0)",
                k4_session_key,8
                    );
            /* Prepare the result of the challenge */
            /* Decrypt the session_key, add 1, and then encrypt it */
            /* The result stored in k4_challenge should match the  */
            /* KRB4_RESPONSE value from the host.                  */
#ifdef NT
            des_ecb_encrypt(k4_session_key, k4_challenge, k4_sched, 0);
#else /* NT */
            des_ecb_encrypt(&k4_session_key, &k4_challenge, k4_sched, 0);
#endif /* NT */

            ckhexdump("auth_send des_ecb_encrypt(k4_session_key,k4_challenge,0)",
                     k4_challenge,8);
#endif /* MIT_CURRENT */
            /*
            * Increment the challenge by 1, and encrypt it for
            * later comparison.
            */
            for (i = 7; i >= 0; --i) {
                register int x;
                x = (unsigned int)k4_challenge[i] + 1;
                k4_challenge[i] = x;    /* ignore overflow */
                if (x < 256)            /* if no overflow, all done */
                    break;
            }
            ckhexdump("auth_send k4_challenge+1",k4_challenge,8);
#ifdef MIT_CURRENT
            data.data = k4_challenge;
            data.length = 8;

            encdata.ciphertext.data = k4_challenge;
            encdata.ciphertext.length = 8;
            encdata.enctype = ENCTYPE_UNKNOWN;

            if (code = krb5_c_encrypt(k5_context, &k4_krbkey, 0, 0, &data,
                                       &encdata)) {
                com_err("libtelnet", code, "while encrypting random key");
                return(0);
            }
#else /* MIT_CURRENT */
#ifdef NT
            des_ecb_encrypt(k4_challenge, k4_challenge, k4_sched, 1);
#else /* NT */
            des_ecb_encrypt(&k4_challenge, &k4_challenge, k4_sched, 1);
#endif /* NT */
            ckhexdump("auth_send des_ecb_encrypt(k4_session_key,k4_challenge,1)",
                     k4_challenge,8);
#endif /* MIT_CURRENT */
        }
#endif  /* ENCRYPTION */
#endif /* REMOVE_FOR_EXPORT */
        break;
#endif /* KRB4 */
#ifdef GSSAPI_KRB5
    case AUTHTYPE_GSSAPI_KRB5:
        for ( i=0 ; i<gss_send_tok.length ; i++ ) {
            if ( ((char *)gss_send_tok.value)[i] == IAC )
                iaccnt++;
        }

        if ( gss_send_tok.length + iaccnt + 10 < sizeof(buf) )
            gss_send_tok.length = copy_for_net(&buf[7], gss_send_tok.value,
                                               gss_send_tok.length);
        else
            gss_send_tok.length = 0;
        sprintf(&buf[gss_send_tok.length+7], "%c%c", IAC, SE);       /* safe */
        if (deblog || tn_deb || debses) {
            int i;
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION)," IS ",
                      AUTHTYPE_NAME(authentication_version)," ",
                      AUTHMODE_NAME(mode)," AUTH ",
                      NULL,NULL,NULL,NULL,NULL
                     );
            tn_hex((char *)tn_msg,TN_MSG_LEN,&buf[7],gss_send_tok.length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((CHAR *)buf, gss_send_tok.length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
#endif /* GSSAPI_KRB5 */
#ifdef KRB5
    case AUTHTYPE_KERBEROS_V5:
        debug(F111,"auth_send KRB5","k5_auth.length",k5_auth.length);
        for ( i=0 ; i<k5_auth.length ; i++ ) {
            if ( (char *)k5_auth.data[i] == IAC )
                iaccnt++;
        }
        if ( k5_auth.length + iaccnt + 10 < sizeof(buf) ) {
	    k5_auth.length = copy_for_net(&buf[7],
					  (CHAR *)k5_auth.data,
					  k5_auth.length);
	} else {
          debug(F100,"auth_send() KRB5 auth data too large for buffer","",0);
          k5_auth.length = 0;
        }

        sprintf((char *)&buf[k5_auth.length+7], "%c%c", IAC, SE); /* safe */
        if (deblog || tn_deb || debses) {
            int i;
            ckmakxmsg(tn_msg,TN_MSG_LEN,
                      "TELNET SENT SB ",TELOPT(TELOPT_AUTHENTICATION)," IS ",
                      AUTHTYPE_NAME(authentication_version)," ",
                      AUTHMODE_NAME(mode)," AUTH ",
                      NULL,NULL,NULL,NULL,NULL
                     );
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&buf[7],k5_auth.length);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol((CHAR *)buf, k5_auth.length+9);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
#ifdef HEIMDAL
        krb5_data_free(&k5_auth);
#else /* HEIMDAL */
        krb5_free_data_contents(k5_context,&k5_auth);
        memset(&k5_auth,0,sizeof(krb5_data));
#endif /* HEIMDAL */
        break;
#endif /* KRB5 */
    }
    return AUTH_SUCCESS;
}

/*
 * Function: Parse authentication REPLY command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *      end_sub - index of the character in the 'parsedat' array which
 *              is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int
#ifdef CK_ANSIC
auth_reply(unsigned char *parsedat, int end_sub)
#else
auth_reply(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
    int n = AUTH_FAILURE;

    if ( parsedat[2] != authentication_version ) {
        printf("Authentication version mismatch (%s [%d] != %s [%d])\r\n",
                AUTHTYPE_NAME(parsedat[2]),parsedat[2],
                AUTHTYPE_NAME(authentication_version),authentication_version);
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }
    if ( parsedat[3] != (auth_how|auth_crypt|auth_fwd) ) {
        printf("Authentication mode mismatch (%s != %s)\r\n",
                AUTHMODE_NAME(parsedat[3]),
                AUTHMODE_NAME(auth_how|auth_crypt|auth_fwd));
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }

#ifdef KRB4
    if (authentication_version == AUTHTYPE_KERBEROS_V4)
        n = k4_auth_reply(parsedat, end_sub);
#endif
#ifdef KRB5
    if (authentication_version == AUTHTYPE_KERBEROS_V5)
        n = k5_auth_reply(auth_how|auth_crypt|auth_fwd, parsedat, end_sub);
#endif
#ifdef CK_SRP
    if (authentication_version == AUTHTYPE_SRP) {
#ifndef PRE_SRP_1_7_3
        n = new_srp_reply(auth_how|auth_crypt|auth_fwd, parsedat, end_sub);
#else /* PRE_SRP_1_7_3 */
        n = srp_reply(auth_how|auth_crypt|auth_fwd, parsedat, end_sub);
#endif /* PRE_SRP_1_7_3 */
    }
#endif /* SRP */
#ifdef CK_SSL
    if (authentication_version == AUTHTYPE_SSL)
        n = ssl_reply(auth_how|auth_crypt|auth_fwd, parsedat, end_sub);
#endif /* SSL */
#ifdef NTLM
    if (authentication_version == AUTHTYPE_NTLM)
        n = ntlm_reply(auth_how|auth_crypt|auth_fwd, parsedat, end_sub);
#endif /* NTLM */
    return n;
}


/*
 * Function: Parse authentication IS command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *      end_sub - index of the character in the 'parsedat' array which
 *              is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int
#ifdef CK_ANSIC
auth_is(unsigned char *parsedat, int end_sub)
#else
auth_is(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
    int n = AUTH_FAILURE;

    if ( parsedat[2] == AUTHTYPE_NULL ) {
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }

    /*
     *  If CLIENT_CHOOSE_ONCE is selected the server will not allow the
     *  client to switch to an alternate authentication method if the one
     *  it originally selected fails.  (ie, if the host's SRP parameters
     *  are invalid.)  However, I think this is a bit of a security risk
     *  since allowing that functionality means that it is impossible to
     *  detect if an attack is being carried out on
     */
#define CLIENT_CHOOSE_ONCE
#ifdef CLIENT_CHOOSE_ONCE
    if ( authentication_version == AUTHTYPE_AUTO )
#endif /* CLIENT_CHOOSE_ONCE */
    {
        /* this block of code needs to check the initial parameters */
        /* to ensure that those returned match one of the sets that */
        /* were sent to the client in the first place.              */

        int i=0;
        for ( i=4; str_request[i] != IAC ; i+=2) {
            if (str_request[i] == parsedat[2] &&
                 str_request[i+1] == parsedat[3])
                break;
        }

        if ( str_request[i] == IAC ) {
            printf("Invalid authentication type pair (%s,%s)\r\n",
                    AUTHTYPE_NAME(parsedat[2]),
                    AUTHMODE_NAME(parsedat[3]));
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }

        if (authentication_version != parsedat[2]) {
            authentication_version = parsedat[2];
            auth_how = (parsedat[3] & AUTH_HOW_MASK);
            auth_crypt = (parsedat[3] & AUTH_ENCRYPT_MASK);
            auth_fwd = (parsedat[3] & INI_CRED_FWD_MASK);
            debug(F111,"auth_is","authentication_version",
                  authentication_version);
            debug(F111,"auth_is","auth_how",auth_how);
            debug(F111,"auth_is","auth_crypt",auth_crypt);
            debug(F111,"auth_is","auth_fwd",auth_fwd);
        }
    }

#ifdef CLIENT_CHOOSE_ONCE
    if ( parsedat[2] != authentication_version ) {
        printf("Authentication version mismatch (%s [%d] != %s [%d])\r\n",
                AUTHTYPE_NAME(parsedat[2]),parsedat[2],
                AUTHTYPE_NAME(authentication_version),authentication_version);
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }
    if ( parsedat[3] != (auth_how|auth_crypt|auth_fwd) ) {
        printf("Authentication mode mismatch (%s != %s)\r\n",
                AUTHMODE_NAME(parsedat[3]),
                AUTHMODE_NAME(auth_how|auth_crypt|auth_fwd));
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }
#endif /* CLIENT_CHOOSE_ONCE */

    switch (authentication_version) {
#ifdef KRB4
    case AUTHTYPE_KERBEROS_V4:
        n = k4_auth_is(parsedat, end_sub);
        break;
#endif
#ifdef KRB5
    case AUTHTYPE_KERBEROS_V5:
        n = k5_auth_is(parsedat[3],parsedat, end_sub);
        break;
#endif
#ifdef CK_SRP
    case AUTHTYPE_SRP:
#ifndef PRE_SRP_1_7_3
        n = new_srp_is(parsedat[3], parsedat, end_sub);
#else /* PRE_SRP_1_7_3 */
        n = srp_is(parsedat[3], parsedat, end_sub);
#endif /* PRE_SRP_1_7_3 */
        break;
#endif /* SRP */
#ifdef CK_SSL
    case AUTHTYPE_SSL:
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        n = ssl_is(parsedat, end_sub);
        break;
#endif /* SSL */
#ifdef NTLM
    case AUTHTYPE_NTLM:
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        n = ntlm_is(parsedat, end_sub);
        break;
#endif /* NTLM */
    case AUTHTYPE_NULL:
    default:
        n = AUTH_FAILURE;
    }
    debug(F111,"auth_is","n",n);
    return n;
}

/*
 * Function: Parse authentication NAME command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *      end_sub - index of the character in the 'parsedat' array which
 *              is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int
#ifdef CK_ANSIC
auth_name(unsigned char *parsedat, int end_sub)
#else
auth_name(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
    int len = (end_sub-2) > 63 ? 63 : (end_sub-2);
    if ( len > 0 && (len + 1) < sizeof(szUserNameRequested)) {
        memcpy(szUserNameRequested,&parsedat[2],len);           /* safe */
        szUserNameRequested[len] = '\0';
    } else
      szUserNameRequested[0] = '\0';
    debug(F111,"auth_name szUserNameRequested",szUserNameRequested,len);
    return(AUTH_SUCCESS);
}

/*
 * Function: Parse the athorization sub-options and reply.
 *
 * Parameters:
 *      parsedat - sub-option string to parse.
 *
 *      end_sub - last charcter position in parsedat.
 */
int
auth_parse(unsigned char *parsedat, int end_sub)
{
    int rc = AUTH_FAILURE;
    switch (parsedat[1]) {
    case TELQUAL_SEND:
        rc = auth_send(parsedat, end_sub);
        break;
    case TELQUAL_REPLY:
        rc= auth_reply(parsedat, end_sub);
        break;
    case TELQUAL_IS:
        rc = auth_is(parsedat, end_sub);
        break;
    case TELQUAL_NAME:
        rc = auth_name(parsedat, end_sub);
        break;
    }
    debug(F111,"auth_parse","rc",rc);
    return(rc);
}


/*
 * Function: Initialization routine called kstream encryption system.
 *
 * Parameters:
 *  data - user data.
 */
int
#ifdef CK_ANSIC
auth_init(kstream ks)
#else
auth_init(ks) kstream_ptr ks;
#endif
{
#ifdef FORWARD
    forwarded_tickets = 0;  /* were tickets forwarded? */
#endif /* FORWARD */
#ifdef CK_ENCRYPTION
    encrypt_init(ks,cx_type);
#endif
    return 0;
}


/*
 * Function: Destroy routine called kstream encryption system.
 *
 * Parameters:
 *  data - user data.
 */
VOID
#ifdef CK_ANSIC
auth_destroy(void)
#else
auth_destroy()
#endif
{
}


/*
 * Function: Callback to encrypt a block of characters
 *
 * Parameters:
 *  out - return as pointer to converted buffer.
 *
 *  in - the buffer to convert
 *
 * Returns: number of characters converted.
 */
int
#ifdef CK_ANSIC
auth_encrypt(struct kstream_data_block *out,
             struct kstream_data_block *in)
#else
auth_encrypt(out,in)
    struct kstream_data_block *out; struct kstream_data_block *in;
#endif
{
    out->ptr = in->ptr;

    out->length = in->length;

    return(out->length);
}


/*
 * Function: Callback to decrypt a block of characters
 *
 * Parameters:
 *  out - return as pointer to converted buffer.
 *
 *  in - the buffer to convert
 *
 * Returns: number of characters converted.
 */
int
#ifdef CK_ANSIC
auth_decrypt(struct kstream_data_block *out,
             struct kstream_data_block *in)
#else
auth_decrypt(out,in)
    struct kstream_data_block *out; struct kstream_data_block *in;
#endif
{
    out->ptr = in->ptr;

    out->length = in->length;

    return(out->length);
}

#ifdef KRB4
#ifdef NT
void
ck_krb4_debug(int x)
{
    set_krb_debug(x);
    set_krb_ap_req_debug(x);
}
#endif /* NT */
int
ck_krb4_autoget_TGT(char * realm)
{
    extern struct krb_op_data krb_op;
    extern struct krb4_init_data krb4_init;
    char passwd[PWD_SZ];
    char prompt[256];
    char * saverealm=NULL;
    int  rc = -1;
    extern char * k4prprompt;
    extern char * k4pwprompt;

    ini_kerb();         /* Place defaults in above structs */
    passwd[0] = '\0';

    if ( krb4_init.principal == NULL ||
         krb4_init.principal[0] == '\0') {
        int ok = uq_txt(NULL, 
                 k4prprompt && k4prprompt[0] ?
                 k4prprompt :
                 "Kerberos 4 Principal: ",
                 2, NULL, passwd,PWD_SZ-1, NULL, DEFAULT_UQ_TIMEOUT);
        if ( ok && passwd[0] )
            makestr(&krb4_init.principal,passwd);
        else
            return(0);
    }

    /* Save realm in init structure so it can be restored */
    if ( realm ) {
        saverealm = krb4_init.realm;
        krb4_init.realm = realm;
    }

    if ( passwd[0] || !(pwbuf[0] && pwflg) ) {
        int ok;
        if ( k4pwprompt && k4pwprompt[0] &&
             (strlen(k4pwprompt) + strlen(krb4_init.principal) +
               strlen(krb4_init.realm) - 4) < sizeof(prompt)) {
            sprintf(prompt,k4pwprompt,krb4_init.principal,krb4_init.realm);
        } else
            ckmakxmsg(prompt,sizeof(prompt),
                  "Kerberos 4 Password for ",krb4_init.principal,"@",
                  krb4_init.realm,": ",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        ok = uq_txt(NULL,prompt,2,NULL,passwd,PWD_SZ-1,NULL,
		    DEFAULT_UQ_TIMEOUT);
        if ( !ok )
            passwd[0] = '\0';
    } else {
        ckstrncpy(passwd,pwbuf,sizeof(passwd));
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)passwd);
#endif /* OS2 */
    }

    if ( passwd[0] ) {
        makestr(&krb4_init.password,passwd);
        rc = ck_krb4_initTGT(&krb_op, &krb4_init);
        free(krb4_init.password);
        krb4_init.password = NULL;
    }

    krb4_init.password = NULL;
    memset(passwd,0,PWD_SZ);

    /* restore realm to init structure if needed */
    if ( saverealm )
        krb4_init.realm = saverealm;
    return(rc == 0);
}

char *
ck_krb4_realmofhost(char *host)
{
    return (char *)krb_realmofhost(host);
}

/*
 *
 * K4_auth_send - gets authentication bits we need to send to KDC.
 *
 * Result is left in auth
 *
 * Returns: 0 on failure, 1 on success
 */
static int
#ifdef CK_ANSIC
k4_auth_send(void)
#else
k4_auth_send()
#endif
{
    int r=0;                                    /* Return value */
    char instance[INST_SZ+1]="";
    char *realm=NULL;
    char tgt[4*REALM_SZ+1];

    memset(instance, 0, sizeof(instance));

#ifdef COMMENT
    /* we only need to call krb_get_phost if the hostname */
    /* is not fully qualified.  But we have already done  */
    /* this in netopen() call.  This will save a round of */
    /* DNS queries.                                       */
    debug(F110,"k4_auth_send","krb_get_phost",0);
    if (realm = (char *)krb_get_phost(szHostName)) {
        ckstrncpy(instance, realm, INST_SZ);
    }
#else /* COMMENT */
    {
        char *p;
        ckstrncpy(instance, szHostName, INST_SZ);
        for ( p=instance; *p && *p != '.' ; p++ );
        *p = '\0';
    }
#endif /* COMMENT */

    debug(F110,"k4_auth_send","krb_get_realmofhost",0);
    realm = (char *)krb_realmofhost(szHostName);

    if (!realm) {
        strcpy(strTmp, "Can't find realm for host \"");
        ckstrncat(strTmp, szHostName,AUTHTMPBL);
        ckstrncat(strTmp, "\"",AUTHTMPBL);
        printf("?Kerberos 4 error: %s\r\n",strTmp);
        krb4_errno = r;
        makestr(&krb4_errmsg,strTmp);
        return(0);
    }

    ckmakmsg(tgt,sizeof(tgt),"krbtgt.",realm,"@",realm);
    r = ck_krb4_tkt_isvalid(tgt);

    if ( r <= 0 && krb4_autoget )
        ck_krb4_autoget_TGT(realm);

    debug(F110,"k4_auth_send","krb_mk_req",0);
    r = krb_mk_req(&k4_auth, krb4_d_srv ? krb4_d_srv : KRB4_SERVICE_NAME,
                    instance, realm, 0);

    if (r == 0) {
        debug(F110,"k4_auth_send","krb_get_cred",0);
        r = krb_get_cred(krb4_d_srv ? krb4_d_srv : KRB4_SERVICE_NAME,
                          instance, realm, &cred);
        if (r)
            debug(F111,"k4_auth_send","krb_get_cred() failed",r);
    }
    else
        debug(F111,"k4_auth_send","krb_mk_req() failed",r);

    if (r) {
        strcpy(strTmp, "Can't get \"");
        ckstrncat(strTmp,
                  krb4_d_srv ? krb4_d_srv : KRB4_SERVICE_NAME,AUTHTMPBL);
        if (instance[0] != 0) {
            ckstrncat(strTmp, ".",AUTHTMPBL);
            ckstrncat(strTmp, instance,AUTHTMPBL);
        }
        ckstrncat(strTmp, "@",AUTHTMPBL);
        ckstrncat(strTmp, realm,AUTHTMPBL);
        ckstrncat(strTmp, "\" ticket\r\n  ",AUTHTMPBL);
        ckstrncat(strTmp, (char *)krb_get_err_text_entry(r),AUTHTMPBL);
        debug(F111,"k4_auth_send",(char *)krb_get_err_text_entry(r),r);
        printf("?Kerberos 4 error: %s\r\n",strTmp);
        krb4_errno = r;
        makestr(&krb4_errmsg,krb_get_err_text_entry(krb4_errno));
        return(0);
    }

#ifdef OS2
    if ( !szUserName[0] || !stricmp(szUserName,cred.pname) ) {
        ckstrncpy(szUserName, cred.pname, UIDBUFLEN);
    }
#endif /* OS2 */
    krb4_errno = r;
    makestr(&krb4_errmsg,krb_get_err_text_entry(krb4_errno));
    debug(F110,"k4_auth_send",krb4_errmsg,0);
    return(1);
}

/*
 * Function: K4 parse authentication reply command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *  end_sub - index of the character in the 'parsedat' array which
 *              is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int
#ifdef CK_ANSIC
k4_auth_reply(unsigned char *parsedat, int end_sub)
#else
k4_auth_reply(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
#ifdef CK_ENCRYPTION
    Session_Key skey;
#ifdef MIT_CURRENT
    krb5_data kdata;
    krb5_enc_data encdata;
    krb5_error_code code;
#endif /* MIT_CURRENT */
#endif
    time_t t;
    int x;
    int i;

    if (end_sub < 4 || parsedat[2] != AUTHTYPE_KERBEROS_V4) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    if (parsedat[4] == KRB_REJECT) {
        strTmp[0] = 0;

        for (i = 5; i <= end_sub; i++) {
            if (parsedat[i] == IAC)
                break;
            strTmp[i-5] = parsedat[i];
            strTmp[i-4] = 0;
        }

        if (!strTmp[0])
            strcpy(strTmp, "Authentication rejected by remote machine!");
        printf("Kerberos V4 authentication failed!\r\n%s\r\n",strTmp);
        krb4_errno = -1;
        makestr(&krb4_errmsg,strTmp);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    if (parsedat[4] == KRB_ACCEPT) {
        int net_len;
        if ((parsedat[3] & AUTH_HOW_MASK) == AUTH_HOW_ONE_WAY) {
            ckmakmsg(strTmp,sizeof(strTmp),"Kerberos V4 accepts you as ",
                      szUserName,NULL,NULL);
            printf("%s\r\n",strTmp);
            accept_complete = 1;
            krb4_errno = 0;
            makestr(&krb4_errmsg,strTmp);
            auth_finished(AUTH_USER);
            return AUTH_SUCCESS;
        }

        if ((parsedat[3] & AUTH_HOW_MASK) != AUTH_HOW_MUTUAL) {
            printf("Kerberos V4 authentication failed!\r\n");
            ckstrncpy(strTmp,
        "Kerberos V4 accepted you, but didn't provide mutual authentication",
                       sizeof(strTmp));
            printf("%s\r\n",strTmp);
            krb4_errno = -1;
            makestr(&krb4_errmsg,strTmp);
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

#ifndef REMOVE_FOR_EXPORT
#ifdef CK_ENCRYPTION
        SendK4AuthSB(KRB4_CHALLENGE,k4_session_key,sizeof(k4_session_key));

        /* We have sent the decrypted session key to the host as a challenge */
        /* now encrypt it to restore it to its original valid DES key value */
#ifdef MIT_CURRENT
        kdata.data = k4_session_key;
        kdata.length = 8;

        encdata.ciphertext.data = k4_session_key;
        encdata.ciphertext.length = 8;
        encdata.enctype = ENCTYPE_UNKNOWN;

        if (code = krb5_c_encrypt(k5_context, &k4_krbkey,
                                   0, 0, &kdata, &encdata)) {
            com_err("k4_auth_reply", code,
                     "while encrypting session_key");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
#else /* MIT_CURRENT */
#ifdef NT
        des_ecb_encrypt(k4_session_key, k4_session_key, k4_sched, 1);
#else /* NT */
        des_ecb_encrypt(&k4_session_key, &k4_session_key, k4_sched, 1);
#endif /* NT */
        ckhexdump(
            "k4_auth_reply des_ecb_encrypt(k4_session_key,k4_session_key,1)",
             k4_session_key,
             8
                );
#endif /* MIT_CURRENT */

#ifdef CK_SSL
        if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
        {
        /* And then use it to configure the encryption state machine. */
            skey.type = SK_DES;
            skey.length = 8;
            skey.data = k4_session_key;
            encrypt_session_key(&skey, AUTH_CLIENT_TO_SERVER);
        }
#endif /* ENCRYPTION */
#endif /* REMOVE_FOR_EXPORT */
        accept_complete = 1;
        ckmakmsg(strTmp,sizeof(strTmp),
                 "Kerberos V4 accepts you as ",szUserName,NULL,NULL);
        printf("%s\r\n",strTmp);
        krb4_errno = 0;
        makestr(&krb4_errmsg,strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;
    }

    if (parsedat[4] == KRB4_RESPONSE) {
        if (end_sub < 12) {
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

        ckhexdump("KRB4_RESPONSE &parsedat[5]",&parsedat[5],8);
#ifdef CK_ENCRYPTION
        ckhexdump("KRB4_RESPONSE k4_challenge",k4_challenge,8);

        /* The datablock returned from the host should match the value */
        /* we stored in k4_challenge.                                  */
        if (memcmp(&parsedat[5], k4_challenge, sizeof(k4_challenge)) != 0) {
            printf("Kerberos V4 authentication failed!\r\n%s\r\n",
            "Remote machine is being impersonated!");
            krb4_errno = -1;
            makestr(&krb4_errmsg,"Remote machine is being impersonated!");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
#else /* ENCRYPTION */
        makestr(&krb4_errmsg,"Kermit built without support for encryption.");
        return AUTH_FAILURE;
#endif /* ENCRYPTION */
        mutual_complete = 1;
        ckstrncpy(strTmp,"Remote machine has been mutually authenticated",
                   sizeof(strTmp));
        printf("%s\r\n",strTmp);
        krb4_errno = 0;
        makestr(&krb4_errmsg,strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;
    }
    auth_finished(AUTH_REJECT);
    return AUTH_FAILURE;
}

/*
 * Function: K4 parse authentication IS command
 *
 * Parameters:
 *  parsedat - the sub-command data.
 *
 *  end_sub - index of the character in the 'parsedat' array which
 *            is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */

static int
#ifdef CK_ANSIC
k4_auth_is(unsigned char *parsedat, int end_sub)
#else
k4_auth_is(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
#ifdef CK_ENCRYPTION
    Session_Key skey;
#ifdef MIT_CURRENT
    Block datablock, tmpkey;
    krb5_data kdata;
    krb5_enc_data encdata;
    krb5_error_code code;
#else /* MIT_CURRENT */
    Block datablock;
#endif /* MIT_CURRENT */
#endif  /* ENCRYPTION */
    char realm[REALM_SZ+1];
    char instance[INST_SZ];
    int r = 0;
    char * data = &parsedat[5];
    int    cnt = end_sub - 5;
    extern char myipaddr[];
    struct hostent *host;
    struct in_addr inaddr;
    int i;

    if (end_sub < 4 || parsedat[2] != AUTHTYPE_KERBEROS_V4) {
        debug(F110,"k4_auth_is","Not kerberos v4",0);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    switch (parsedat[4]) {
    case KRB_AUTH:
        debug(F110,"k4_auth_is","KRB_AUTH",0);
        ckstrncpy(realm,ck_krb4_getrealm(),REALM_SZ+1);
        if (realm[0] == '\0') {
            SendK4AuthSB(KRB_REJECT, (void *)"No local V4 Realm.", -1);
            printf("\r\n? Kerberos 4 - No Local Realm\r\n");
            debug(F110,"k4_auth_is","No local realm",0);
            krb4_errno = -1;
            makestr(&krb4_errmsg,"No local realm");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
        debug(F110,"k4_auth_is",realm,0);
        if ( cnt < sizeof(k4_auth.dat) ) {
            k4_auth.length = cnt;
            memcpy((void *)k4_auth.dat, (void *)data, k4_auth.length);
        } else
            k4_auth.length = 0;
        ckhexdump("k4_auth.dat",k4_auth.dat, k4_auth.length);

        /* Get Instance */
        inaddr.s_addr = inet_addr(myipaddr);
        host = gethostbyaddr((unsigned char *)&inaddr,4,PF_INET);
        if ( host ) {
#ifdef HADDRLIST
            host = ck_copyhostent(host);
#endif /* HADDRLIST */
            ckstrncpy(instance,host->h_name,INST_SZ);
            for ( i=0;i<INST_SZ;i++ ) {
                if ( instance[i] == '.' )
                    instance[i] = '\0';
                else
                    instance[i] = tolower(instance[i]);
            }
        } else {
            instance[0] = '*';
            instance[1] = 0;
        }

        if (r = krb_rd_req(&k4_auth,
                            krb4_d_srv ? krb4_d_srv : KRB4_SERVICE_NAME,
                            instance, 0, &k4_adat, k4_keytab)) {

            ckhexdump("k4_adat", &k4_adat, sizeof(AUTH_DAT));
            krb_kntoln(&k4_adat, k4_name);
            ckmakmsg(strTmp,sizeof(strTmp),
                     "Kerberos failed him as ", k4_name,NULL,NULL);
            printf("%s\r\n",strTmp);
            krb4_errno = r;
            makestr(&krb4_errmsg,strTmp);
            SendK4AuthSB(KRB_REJECT, (void *)krb_get_err_text_entry(r), -1);
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

#ifdef CK_ENCRYPTION
        memcpy((void *)k4_session_key, (void *)k4_adat.session,
                sizeof(Block));                 /* safe */
        ckhexdump("k4_auth_is k4_session_key",k4_session_key,sizeof(Block));
#endif /* ENCRYPTION */
        krb_kntoln(&k4_adat, k4_name);

        ckstrncpy(szUserNameAuthenticated,k4_name,UIDBUFLEN);
        if (szUserNameRequested && !kuserok(&k4_adat, k4_name)) {
            SendK4AuthSB(KRB_ACCEPT, (void *)0, 0);
            if ( !strcmp(k4_name,szUserNameRequested) )
                auth_finished(AUTH_VALID);
            else
                auth_finished(AUTH_USER);
            accept_complete = 1;
        }
        else {
            SendK4AuthSB(KRB_REJECT,
                  (void *)"user is not authorized", -1);
            auth_finished(AUTH_REJECT);
            krb4_errno = r;
            makestr(&krb4_errmsg,"user is not authorized");
            return(AUTH_FAILURE);
        }
        break;

    case KRB4_CHALLENGE:
        debug(F110,"k4_auth_is","KRB_CHALLENGE",0);
#ifndef CK_ENCRYPTION
        SendK4AuthSB(KRB4_RESPONSE, (void *)0, 0);
#else   /* ENCRYPTION */
        if (!VALIDKEY(k4_session_key)) {
            /*
            * We don't have a valid session key, so just
            * send back a response with an empty session
            * key.
            */
            SendK4AuthSB(KRB4_RESPONSE, (void *)0, 0);
            mutual_complete = 1;
            break;
        }

        /*
        * Initialize the random number generator since it's
        * used later on by the encryption routine.
        */
#ifdef MIT_CURRENT
        kdata.data = k4_session_key;
        kdata.length = 8;

        if (code = krb5_c_random_seed(k5_context, &kdata)) {
            com_err("k4_auth_is", code,
                     "while seeding random number generator");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

        memcpy((void *)datablock, (void *)data, sizeof(Block)); /* safe */
        /*
        * Take the received encrypted challenge, and encrypt
        * it again to get a unique session_key for the
        * ENCRYPT option.
        */
        k4_krbkey.enctype = ENCTYPE_DES_CBC_RAW;
        k4_krbkey.length = 8;
        k4_krbkey.contents = k4_session_key;

        kdata.data = datablock;
        kdata.length = 8;

        encdata.ciphertext.data = tmpkey;
        encdata.ciphertext.length = 8;
        encdata.enctype = ENCTYPE_UNKNOWN;

        if (code = krb5_c_encrypt(k5_context, &k4_krbkey, 0, 0,
                                   &kdata, &encdata)) {
            com_err("k4_auth_is", code, "while encrypting random key");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

#ifdef CK_SSL
        if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
        {
            skey.type = SK_DES;
            skey.length = 8;
            skey.data = tmpkey;
            encrypt_session_key(&skey, AUTH_SERVER_TO_CLIENT);
        }
        /*
        * Now decrypt the received encrypted challenge,
        * increment by one, re-encrypt it and send it back.
        */
        encdata.ciphertext.data = datablock;
        encdata.ciphertext.length = 8;
        encdata.enctype = ENCTYPE_UNKNOWN;

        kdata.data = k4_challenge;
        kdata.length = 8;

        if (code = krb5_c_decrypt(k5_context, &k4_krbkey, 0, 0,
                                   &encdata, &kdata)) {
            com_err("k4_auth_is", code, "while decrypting challenge");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
#else /* MIT_CURRENT */
        des_set_random_generator_seed(k4_session_key);
        r = des_key_sched(k4_session_key, k4_sched);
        if ( r == -1 ) {
            printf("?Invalid DES key specified in credentials\r\n");
            debug(F110,"auth_is CHALLENGE",
                   "invalid DES Key specified in credentials",0);
        } else if ( r == -2 ) {
            printf("?Weak DES key specified in credentials\r\n");
            debug(F110,"auth_is CHALLENGE",
                   "weak DES Key specified in credentials",0);
        } else if ( r != 0 ) {
            printf("?DES Key Schedule not set by credentials\r\n");
            debug(F110,"auth_is CHALLENGE",
                   "DES Key Schedule not set by credentials",0);
        }
        ckhexdump("auth_is schedule",k4_sched,8*16);

        memcpy((void *)datablock, (void *)data, sizeof(Block)); /* safe */
        ckhexdump("auth_is challege",datablock,sizeof(Block));

        /*
        * Take the received encrypted challenge, and encrypt
        * it again to get a unique k4_session_key for the
        * ENCRYPT option.
        */
#ifdef NT
        des_ecb_encrypt(datablock, k4_session_key, k4_sched, 1);
#else /* NT */
        des_ecb_encrypt(&datablock, &k4_session_key, k4_sched, 1);
#endif /* NT */
        ckhexdump("auth_is des_ecb_encrypt(datablock,k4_session_key,1)",
                 k4_session_key,8);

#ifdef CK_SSL
        if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
        {
            skey.type = SK_DES;
            skey.length = 8;
            skey.data = k4_session_key;
            encrypt_session_key(&skey, AUTH_SERVER_TO_CLIENT);
        }
        /*
        * Now decrypt the received encrypted challenge,
        * increment by one, re-encrypt it and send it back.
        */
#ifdef NT
        des_ecb_encrypt(datablock, k4_challenge, k4_sched, 0);
#else /* NT */
        des_ecb_encrypt(&datablock, &k4_challenge, k4_sched, 0);
#endif /* NT */
        ckhexdump("auth_is des_ecb_encrypt(datablock,k4_challenge,0)",
                 k4_session_key,8);
#endif /* MIT_CURRENT */
        for (r = 7; r >= 0; r--) {
            register int t;
            t = (unsigned int)k4_challenge[r] + 1;
            k4_challenge[r] = t;        /* ignore overflow */
            if (t < 256)                /* if no overflow, all done */
                break;
        }
        ckhexdump("auth_is k4_challenge+1",k4_challenge,8);

#ifdef MIT_CURRENT
        kdata.data = k4_challenge;
        kdata.length = 8;

        encdata.ciphertext.data = k4_challenge;
        encdata.ciphertext.length = 8;
        encdata.enctype = ENCTYPE_UNKNOWN;

        if (code = krb5_c_encrypt(k5_context, &k4_krbkey, 0, 0,
                                   &kdata, &encdata)) {
            com_err("k4_auth_is", code, "while decrypting challenge");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
#else /* MIT_CURRENT */
#ifdef NT
        des_ecb_encrypt(k4_challenge, k4_challenge, k4_sched, 1);
#else /* NT */
        des_ecb_encrypt(&k4_challenge, &k4_challenge, k4_sched, 1);
#endif /* NT */
        ckhexdump("auth_is des_ecb_encrypt(k4_challenge_key,k4_challenge,1)",
                 k4_challenge,8);

#endif /* MIT_CURRENT */
        SendK4AuthSB(KRB4_RESPONSE,(void *)k4_challenge,sizeof(k4_challenge));
#endif  /* ENCRYPTION */
        mutual_complete = 1;
        break;

    default:
        if (1)
            printf("Unknown Kerberos option %d\r\n", data[-1]);
        SendK4AuthSB(KRB_REJECT, 0, 0);
        return(AUTH_FAILURE);
    }
    krb4_errno = r;
    makestr(&krb4_errmsg,krb_get_err_text_entry(krb4_errno));
    return(AUTH_SUCCESS);
}
#endif /* KRB4 */

#ifdef KRB5
int
ck_krb5_autoget_TGT(char * realm)
{
    extern struct krb_op_data krb_op;
    extern struct krb5_init_data krb5_init;
    char passwd[PWD_SZ];
    char prompt[64];
    char * saverealm=NULL;
    int  rc = -1;
    extern char * k5prprompt;
    extern char * k5pwprompt;

    ini_kerb();         /* Place defaults in above structs */
    passwd[0] = '\0';

    if ( krb5_init.principal == NULL ||
         krb5_init.principal[0] == '\0') {
        int ok = uq_txt(NULL,k5prprompt && k5prprompt[0] ? k5prprompt :
                  "Kerberos 5 Principal: ",2,NULL,passwd,PWD_SZ-1,NULL,
			DEFAULT_UQ_TIMEOUT);
        if ( ok && passwd[0] )
            makestr(&krb5_init.principal,passwd);
        else
            return(0);
    }

    /* Save realm in init structure so it can be restored */
    if ( realm ) {
        saverealm = krb5_init.realm;
        krb5_init.realm = realm;
    }

    if ( passwd[0] || !(pwbuf[0] && pwflg) ) {
        int ok;
        if ( k5pwprompt && k5pwprompt[0] &&
             (strlen(k5pwprompt) + strlen(krb5_init.principal) +
              strlen(krb5_init.realm) - 4) < sizeof(prompt)) {
            sprintf(prompt,k5pwprompt,krb5_init.principal,krb5_init.realm);
        } else
        ckmakxmsg(prompt,sizeof(prompt),
                  k5pwprompt && k5pwprompt[0] ? k5pwprompt :
                  "Kerberos 5 Password for ",
                  krb5_init.principal,"@",krb5_init.realm,": ",
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL
                 );
        ok = uq_txt(NULL,prompt,2,NULL,passwd,PWD_SZ-1,NULL,
		    DEFAULT_UQ_TIMEOUT);
        if ( !ok )
            passwd[0] = '\0';
    } else {
        ckstrncpy(passwd,(char *)pwbuf,sizeof(passwd));
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)passwd);
#endif /* OS2 */
    }

    if ( passwd[0] ) {
        extern struct krb4_init_data krb4_init;
        char * savek4realm=NULL;

        makestr(&krb5_init.password,passwd);

        if ( krb5_d_getk4 ) {
            krb5_init.getk4 = 1;
            makestr(&krb4_init.principal,krb5_init.principal);
            makestr(&krb4_init.password,passwd);
            if ( realm ) {
                savek4realm = krb4_init.realm;
                krb4_init.realm = realm;
            }
            rc = ck_krb5_initTGT(&krb_op, &krb5_init,&krb4_init);

            if ( savek4realm )
                krb4_init.realm = savek4realm;
            free(krb4_init.password);
            krb4_init.password = NULL;
        } else {
            rc = ck_krb5_initTGT(&krb_op, &krb5_init,NULL);
        }

        free(krb5_init.password);
        krb5_init.password = NULL;

        memset(passwd,0,PWD_SZ);
    }

    /* restore realm to init structure if needed */
    if ( saverealm )
        krb5_init.realm = saverealm;
    return(rc == 0);
}

static krb5_error_code
#ifdef CK_ANSIC
k5_get_ccache( krb5_context k5_context, krb5_ccache * p_ccache,
               char * cc_name )
#else  /* CK_ANSIC */
k5_get_ccache(k5_context, p_ccache, cc_name)
    krb5_context k5_context;
    krb5_ccache * p_ccache;
    char * cc_name;
#endif /* CK_ANSIC */
{
    krb5_error_code r=0;
    char cc_tmp[CKMAXPATH+1];
    const char * def_name = NULL;

#ifndef HEIMDAL
    if ( cc_name ) {
        if ( strncmp("FILE:",cc_name,5) &&
             strncmp("MEMORY:",cc_name,7) &&
             strncmp("API:",cc_name,4) &&
             strncmp("STDIO:",cc_name,6) &&
	     strncmp("MSLSA:",cc_name,6))
#ifdef NT
            ckmakmsg(cc_tmp,CKMAXPATH,"API:",cc_name,NULL,NULL);
#else /* NT */
            ckmakmsg(cc_tmp,CKMAXPATH,"FILE:",cc_name,NULL,NULL);
#endif /* NT */
        else {
            ckstrncpy(cc_tmp,cc_name,CKMAXPATH);
        }
        r = krb5_cc_resolve (k5_context, cc_tmp, p_ccache);
        if (r != 0) {
            com_err("k5_get_ccache resolving ccache",r,
                     cc_tmp);
        } else {
            /* Make sure GSSAPI sees the same cache we are using */
            char buf[128];
            ckmakmsg((char *)buf,128,"KRB5CCNAME=",cc_tmp,NULL,NULL);
            putenv(buf);
        }
    } else if ( krb5_d_cc ) {
        if ( strncmp("FILE:",krb5_d_cc,5) &&
             strncmp("MEMORY:",krb5_d_cc,7) &&
             strncmp("API:",krb5_d_cc,4) &&
             strncmp("STDIO:",krb5_d_cc,6) &&
	     strncmp("MSLSA:", krb5_d_cc,6))
#ifdef NT
            ckmakmsg(cc_tmp,CKMAXPATH,"API:",krb5_d_cc,NULL,NULL);
#else /* NT */
            ckmakmsg(cc_tmp,CKMAXPATH,"FILE:",krb5_d_cc,NULL,NULL);
#endif /* NT */
        else {
            ckstrncpy(cc_tmp,krb5_d_cc,CKMAXPATH);
        }
        r = krb5_cc_resolve (k5_context, cc_tmp, p_ccache);
        if (r != 0) {
            com_err("k5_get_ccache resolving ccache",r,
                     krb5_d_cc);
        } else {
            /* Make sure GSSAPI sees the same cache we are using */
            char buf[128];
            ckmakmsg((char *)buf,128,"KRB5CCNAME=",cc_tmp,NULL,NULL);
            putenv(buf);
        }
    } else
#endif /* HEIMDAL */
    {
        if ((r = krb5_cc_default(k5_context, p_ccache))) {
            com_err("k5_get_ccache",r,"while getting default ccache");
        }
    }
    /* do not set krb5_errno/krb5_errmsg here since the value returned */
    /* is being passed internally within the krb5 functions.           */
    return(r);
}


char *
ck_krb5_realmofhost(char *host)
{
    char ** realmlist=NULL;
    krb5_context private_context=NULL;
    static char * realm = NULL;

    if ( !host )
        return NULL;

    if ( realm ) {
        free(realm);
        realm = NULL;
    }

    /* create private_context */
    if (krb5_init_context(&private_context)) {
        debug(F110,"ck_krb5_realmofhost()","unable to init_context",0);
        return(NULL);
    }

    krb5_get_host_realm(private_context,host,&realmlist);
    if (realmlist && realmlist[0]) {
        makestr(&realm,realmlist[0]);
        krb5_free_host_realm(private_context,realmlist);
        realmlist = NULL;
    }

    if ( private_context ) {
        krb5_free_context(private_context);
        private_context = NULL;
    }

    if (ckstrchr(realm,'.') == NULL) {
        int n = 0;
        char * p = host;
        while ( (p = ckstrchr(p,'.')) != NULL ) {
            n++;
            p++;
        }
        if (n == 1) {
            makestr(&realm,host);
            ckupper(realm);
        } else {
            free(realm);
            realm = NULL;
        }
    }
    return(realm);
}

/*
 *
 * K5_auth_send - gets authentication bits we need to send to KDC.
 *
 * Code lifted from telnet sample code in the appl directory.
 *
 * Result is left in k5_auth
 *
 * Returns: 0 on failure, 1 on success
 *
 */

static int
#ifdef CK_ANSIC
k5_auth_send(int how, int encrypt, int forward)
#else
k5_auth_send(how,encrypt,forward) int how; int encrypt; int forward;
#endif
{
    krb5_error_code r=0;
    krb5_ccache ccache=NULL;
#ifndef HEIMDAL
    krb5_creds creds;
#endif /* HEIMDAL */
    krb5_creds * new_creds=NULL;
#ifdef CK_ENCRYPTION
    krb5_keyblock *newkey = 0;
#endif /* CK_ENCRYPTION */
    krb5_flags ap_opts, auth_flags;
    char type_check[32];
    krb5_data checksum;
    int len=0;
    char * realm = NULL;
    char tgt[256];

    realm = ck_krb5_realmofhost(szHostName);
    if (!realm) {
        ckstrncpy(strTmp, "Can't find realm for host \"",AUTHTMPBL);
        ckstrncat(strTmp, szHostName,AUTHTMPBL);
        ckstrncat(strTmp, "\"",AUTHTMPBL);
        printf("?Kerberos 5 error: %s\r\n",strTmp);
        krb5_errno = KRB5_ERR_HOST_REALM_UNKNOWN;
        makestr(&krb5_errmsg,strTmp);
        return(0);
    }

    ckmakmsg(tgt,sizeof(tgt),"krbtgt/",realm,"@",realm);
    debug(F110,"k5_auth_send TGT",tgt,0);
    if ( krb5_autoget &&
         !((ck_krb5_tkt_isvalid(NULL,tgt) > 0) ||
          (ck_krb5_is_tgt_valid() > 0)) )
        ck_krb5_autoget_TGT(realm);

    r = k5_get_ccache(k5_context,&ccache,NULL);
    if ( r ) {
        com_err(NULL, r, "while authorizing (0).");
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }

#ifndef HEIMDAL
    memset((char *)&creds, 0, sizeof(creds));
    if (r = krb5_sname_to_principal(k5_context, szHostName,
                                krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME,
                                KRB5_NT_SRV_HST, &creds.server)) {
        com_err(NULL, r, "while authorizing (1).");
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }

    if (forward_flag) {
        if (fwd_server) {
            krb5_free_principal(k5_context,fwd_server);
            fwd_server = NULL;
        }
        krb5_copy_principal(k5_context,creds.server,&fwd_server);
    }

    r = krb5_cc_get_principal(k5_context, ccache, &creds.client);
    if (r) {
        com_err(NULL, r, "while authorizing (2).");
        krb5_free_cred_contents(k5_context, &creds);
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }

    if (szUserName[0] == '\0') {                /* Get user name now */
        len  = krb5_princ_component(k5_context, creds.client, 0)->length;
        if ( len < sizeof(szUserName) ) {
            memcpy(szUserName,
                    krb5_princ_component(k5_context, creds.client, 0)->data,
                    len);                       /* safe */
        } else
            len = 0;
        szUserName[len] = '\0';
    } else {
        char * name = NULL;
        len  = krb5_princ_component(k5_context, creds.client, 0)->length;
        if ( len == strlen(szUserName) ) {
            name = krb5_princ_component(k5_context, creds.client, 0)->data;
#ifdef OS2
            if ( !strnicmp(szUserName,name,len) )
                memcpy(szUserName,name,len);    /* safe */
#endif /* OS2 */
        }
    }
    if ( tn_auth_krb5_des_bug ) {   /* !ALLOW_KRB_3DES_ENCRYPT */
        /* Not sure if this is necessary anymore.  What impact does it have
         * on Win2000 TGTs that use DES_CBC_MD5 or RC4_HMAC?
         *
         * This prevents using 3DES Service Tickets.
         */
        creds.keyblock.enctype=ENCTYPE_DES_CBC_CRC;
    }

    if (r = krb5_get_credentials(k5_context, 0,
                                  ccache, &creds, &new_creds)) {
        com_err(NULL, r, "while authorizing (3).");
        krb5_free_cred_contents(k5_context, &creds);
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }
#endif /* HEIMDAL */

    if (auth_context) {
        krb5_auth_con_free(k5_context, auth_context);
        auth_context = 0;
    }
    if (r = krb5_auth_con_init(k5_context, &auth_context)) {
        com_err(NULL, r, "while initializing auth context");
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }

    /* UPDATE for START_TLS.  AUTH_ENCRYPT_START_TLS and inclusion of */
    /* client and then server finished messages.                      */

    type_check[0] = AUTHTYPE_KERBEROS_V5;
    type_check[1] = AUTH_CLIENT_TO_SERVER |
        (how ? AUTH_HOW_MUTUAL : AUTH_HOW_ONE_WAY) |
        (encrypt) |
        (forward ? INI_CRED_FWD_ON : INI_CRED_FWD_OFF);
#ifdef CK_SSL
    if (encrypt == AUTH_ENCRYPT_START_TLS) {
        ssl_get_client_finished(&type_check[2],12);
        ssl_get_server_finished(&type_check[14],12);
    }
#endif /* CK_SSL */

#ifndef HEIMDAL
    checksum.magic = KV5M_DATA;
#endif /* HEIMDAL */
    checksum.length =
#ifdef CK_SSL
        (encrypt == AUTH_ENCRYPT_START_TLS) ? 26 :
#endif /* CK_SSL */
        2;
    checksum.data = (char *)&type_check;

    ap_opts = 0;
    if ((how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL)
        ap_opts |= AP_OPTS_MUTUAL_REQUIRED;

#ifdef HEIMDAL
    r = krb5_auth_setkeytype(k5_context, auth_context, KEYTYPE_DES);
    if (r)
        com_err(NULL, r, "while setting auth keytype");
    r = krb5_auth_con_setaddrs_from_fd(k5_context,auth_context, &ttyfd);
    if (r)
        com_err(NULL, r, "while setting auth addrs");
    r = krb5_mk_req(k5_context, &auth_context, ap_opts,
                    krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME,
                    szHostName, &checksum, ccache, &k5_auth);
    if (r)
        com_err(NULL, r, "while making request");
#else /* HEIMDAL */
    auth_flags = KRB5_AUTH_CONTEXT_RET_TIME;
#ifdef CK_ENCRYPTION
    ap_opts |= AP_OPTS_USE_SUBKEY;
#endif /* CK_ENCRYPTION */
#ifdef TLS_VERIFY
    if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
        auth_flags |= KRB5_AUTH_CONTEXT_DO_SEQUENCE;
        if (!krb5_d_no_addresses)
            r = krb5_auth_con_genaddrs(k5_context, auth_context, ttyfd,
                                 KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR);
    }
#endif /* CK_SSL */
    krb5_auth_con_setflags(k5_context, auth_context, auth_flags);
    r = krb5_mk_req_extended(k5_context, &auth_context, ap_opts,
                              &checksum, new_creds, &k5_auth);
#endif /* HEIMDAL */

#ifdef CK_ENCRYPTION
    if (!r) {
        r = krb5_auth_con_getlocalsubkey(k5_context, auth_context, &newkey);
        if (r)
            r = krb5_auth_con_getkey(k5_context, auth_context, &newkey);

        if (k5_session_key) {
            krb5_free_keyblock(k5_context, k5_session_key);
            k5_session_key = 0;
        }
    }
    if (newkey) {
        /*
        * keep the key in our private storage, but don't use it
        * yet---see kerberos5_reply() below
        */
#ifdef HEIMDAL
        if ((newkey->keytype == ETYPE_DES_CBC_CRC) ||
             (newkey->keytype == ETYPE_DES_CBC_MD5) ||
             (newkey->keytype == ETYPE_DES_CBC_MD4))
        {
            debug(F111,"k5_auth_send()","newkey->keytype",newkey->keytype);
            krb5_copy_keyblock(k5_context, newkey, &k5_session_key);
        }
#else /* HEIMDAL */
        /* look for all possible DES keys first - just for compatibility */
        /* other key types are much less likely to be available          */
        if ((newkey->enctype == ENCTYPE_DES_CBC_CRC) ||
             (newkey->enctype == ENCTYPE_DES_CBC_MD5) ||
             (newkey->enctype == ENCTYPE_DES_CBC_MD4))
        {
            debug(F111,"k5_auth_send()","newkey->enctype",newkey->enctype);
            krb5_copy_keyblock(k5_context, newkey, &k5_session_key);
        }
        else if ((new_creds->keyblock.enctype == ENCTYPE_DES_CBC_CRC) ||
                 (new_creds->keyblock.enctype == ENCTYPE_DES_CBC_MD5))
        {
            /* use the session key in credentials instead */
            debug(F111,"k5_auth_send()","new_creds->keyblock.enctype",
                   new_creds->keyblock.enctype);
            krb5_copy_keyblock(k5_context,
                                &new_creds->keyblock, &k5_session_key);
        }
        else if (newkey->enctype != 0)
        {
            debug(F111,"k5_auth_send()","newkey->enctype",newkey->enctype);
            krb5_copy_keyblock(k5_context, newkey, &k5_session_key);
        }
        else if (new_creds->keyblock.enctype != 0)
        {
            /* use the session key in credentials instead */
            debug(F111,"k5_auth_send()","new_creds->keyblock.enctype",
                   new_creds->keyblock.enctype);
            krb5_copy_keyblock(k5_context,
                                &new_creds->keyblock, &k5_session_key);
        }
        else {
            debug(F110,"k5_auth_send()","NO KEY in newkey",0);
        }
#endif /* HEIMDAL */
        krb5_free_keyblock(k5_context, newkey);
    }
#endif /* CK_ENCRYPTION */
#ifndef HEIMDAL
    krb5_free_cred_contents(k5_context, &creds);
    krb5_free_creds(k5_context, new_creds);
#endif /* HEIMDAL */
    krb5_cc_close(k5_context,ccache);

    if (r) {
        com_err(NULL, r, "while authorizing (4).");
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(0);
    }
    krb5_errno = 0;
    makestr(&krb5_errmsg,"OK");
    return(1);
}

/*
 * K5_auth_reply -- checks the reply for mutual authentication.
 */
static int
#ifdef CK_ANSIC
k5_auth_reply(int how, unsigned char *data, int cnt)
#else
k5_auth_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
#ifdef CK_ENCRYPTION
    Session_Key skey;
#endif /* CK_ENCRYPTION */

    data += 4;                                  /* Point to status byte */
    cnt -=5;

    switch (*data++) {
    case KRB_REJECT:
        if (cnt > 0) {
            char *s;
            int len;
            ckstrncpy(strTmp,"Kerberos V5 refuses authentication because\r\n",
                      sizeof(strTmp));
            len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                s = strTmp + strlen(strTmp);
                memcpy(s, data, cnt);           /* safe */
                s[cnt] = 0;
            }
        } else
            ckstrncpy(strTmp,"Kerberos V5 refuses authentication",
                      sizeof(strTmp));
        krb5_errno = -1;
        makestr(&krb5_errmsg,strTmp);
        printf("Kerberos authentication failed!\r\n%s\r\n",strTmp);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;

    case KRB_ACCEPT:
        if (!mutual_complete) {
            if ((how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL && !mutual_complete) {
                ckstrncpy(strTmp,
                          "Kerberos V5 accepted you, but didn't provide"
                          " mutual authentication",sizeof(strTmp));
                printf("Kerberos authentication failed!\r\n%s\r\n",strTmp);
                krb5_errno = -1;
                makestr(&krb5_errmsg,strTmp);
                auth_finished(AUTH_REJECT);
                return AUTH_FAILURE;
            }

#ifdef CK_ENCRYPTION
            if (k5_session_key) {
                if ( tn_auth_krb5_des_bug ) {   /* !ALLOW_KRB_3DES_ENCRYPT */
                    skey.type = SK_DES;
                    skey.length = 8;
#ifdef HEIMDAL
                    skey.data = k5_session_key->keyvalue.data;
#else /* HEIMDAL */
                    skey.data = k5_session_key->contents;
#endif /* HEIMDAL */
                } else {
#ifdef HEIMDAL
                    switch ( k5_session_key->keytype ) {
                    case ETYPE_DES_CBC_CRC:
                    case ETYPE_DES_CBC_MD5:
                    case ETYPE_DES_CBC_MD4:
                        skey.type = SK_DES;
                        skey.length = 8;
                        break;
                    default:
                        skey.type = SK_GENERIC;
                        skey.length = k5_session_key->length;
                        encrypt_dont_support(ENCTYPE_DES_CFB64);
                        encrypt_dont_support(ENCTYPE_DES_OFB64);
                    }
                    skey.data = k5_session_key->keyvalue.data;
#else /* HEIMDAL */
                    switch ( k5_session_key->enctype ) {
                    case ENCTYPE_DES_CBC_CRC:
                    case ENCTYPE_DES_CBC_MD5:
                    case ENCTYPE_DES_CBC_MD4:
                        skey.type = SK_DES;
                        skey.length = 8;
                    default:
                        skey.type = SK_GENERIC;
                        skey.length = k5_session_key->length;
                        encrypt_dont_support(ENCTYPE_DES_CFB64);
                        encrypt_dont_support(ENCTYPE_DES_OFB64);
                    }
                    skey.data = k5_session_key->contents;
#endif /* HEIMDAL */
                }
                encrypt_session_key(&skey, AUTH_CLIENT_TO_SERVER);
            }
#endif /* CK_ENCRYPTION */
        }
        if ( cnt > 0 ) {
            char *s;
            int len;
            ckstrncpy(strTmp,"Kerberos V5 accepts you as ",sizeof(strTmp));
            len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                s = strTmp + strlen(strTmp);
                memcpy(s,data,cnt);
                s[cnt] = 0;
            }
        }
        accept_complete = 1;
        printf("%s\r\n",strTmp);

#ifdef FORWARD
        if (forward_flag
#ifdef COMMENT
             /* Marc Horowitz <marc@mit.edu> has successfully argued
                that it is indeed safe to send Forwarded credentials
                to an untrusted host.
              */
             && (auth_how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL
#endif /* COMMENT */
             )
            kerberos5_forward();
#endif /* FORWARD */
        krb5_errno = 0;
        makestr(&krb5_errmsg,strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;

    case KRB5_RESPONSE:
#ifdef TLS_VERIFY
        if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS &&
            !krb5_tls_verified) {
            printf(
    "Man in the middle attack detected.  Session terminated.\r\n");
#ifndef BETATEST
            netclos();
#endif /* BETATEST */
            krb5_errno = -1;
            makestr(&krb5_errmsg,"TLS not verified");
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
        if((ssl_active_flag || tls_active_flag) &&
            (how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
            printf("TLS session parameters verified by Kerberos 5\r\n");
        }
#endif /* TLS_VERIFY */
        if ((how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL) {
            /* the rest of the reply should contain a krb_ap_rep */
            krb5_ap_rep_enc_part *reply;
            krb5_data inbuf;
            krb5_error_code r;

            inbuf.length = cnt;
            inbuf.data = (char *)data;

            if (r = krb5_rd_rep(k5_context, auth_context, &inbuf, &reply)) {
                com_err(NULL, r, "while authorizing. (5)");
                krb5_errno = r;
                makestr(&krb5_errmsg,error_message(krb5_errno));
                auth_finished(AUTH_REJECT);
                return AUTH_FAILURE;
            }
            krb5_free_ap_rep_enc_part(k5_context, reply);

#ifdef CK_ENCRYPTION
            if (encrypt_flag && k5_session_key) {
                if ( tn_auth_krb5_des_bug ) {   /* !ALLOW_KRB_3DES_ENCRYPT */
                    skey.type = SK_DES;
                    skey.length = 8;
#ifdef HEIMDAL
                    skey.data = k5_session_key->keyvalue.data;
#else /* HEIMDAL */
                    skey.data = k5_session_key->contents;
#endif /* HEIMDAL */
                } else {
#ifdef HEIMDAL
                    switch ( k5_session_key->keytype ) {
                    case ETYPE_DES_CBC_CRC:
                    case ETYPE_DES_CBC_MD5:
                    case ETYPE_DES_CBC_MD4:
                        skey.type = SK_DES;
                        skey.length = 8;
                    default:
                        skey.type = SK_GENERIC;
                        skey.length = k5_session_key->length;
                    }
                    skey.data = k5_session_key->keyvalue.data;
#else /* HEIMDAL */
                    switch ( k5_session_key->enctype ) {
                    case ENCTYPE_DES_CBC_CRC:
                    case ENCTYPE_DES_CBC_MD5:
                    case ENCTYPE_DES_CBC_MD4:
                        skey.type = SK_DES;
                        skey.length = 8;
                        break;
                    default:
                        skey.type = SK_GENERIC;
                        skey.length = k5_session_key->length;
                    }
                    skey.data = k5_session_key->contents;
#endif /* HEIMDAL */
                }
                encrypt_session_key(&skey, AUTH_CLIENT_TO_SERVER);
            }
#endif /* ENCRYPTION */
            mutual_complete = 1;
        }
        ckstrncpy(strTmp,"Remote machine has been mutually authenticated",
                  sizeof(strTmp));
        krb5_errno = 0;
        makestr(&krb5_errmsg,strTmp);
        printf("%s\r\n",strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;

#ifdef FORWARD
    case KRB5_FORWARD_ACCEPT:
        forwarded_tickets = 1;
        ckstrncpy(strTmp,"Remote machine has accepted forwarded credentials",
                  sizeof(strTmp));
        krb5_errno = 0;
        makestr(&krb5_errmsg,strTmp);
        printf("%s\r\n",strTmp);
        return AUTH_SUCCESS;

    case KRB5_FORWARD_REJECT:
        forwarded_tickets = 0;
        if (cnt > 0) {
            char *s;
            int len;
            len = ckstrncpy(strTmp,
                      "Kerberos V5 refuses forwarded credentials because ",
                       sizeof(strTmp));
            if ( len + cnt < sizeof(strTmp) ) {
                s = strTmp + strlen(strTmp);
                memcpy(s, data, cnt);
                s[cnt] = 0;
            }
        } else
            ckstrncpy(strTmp, "Kerberos V5 refuses forwarded credentials",
                      sizeof(strTmp));

        printf("%s\r\n",strTmp);
        krb5_errno = -1;
        makestr(&krb5_errmsg,strTmp);
        return AUTH_SUCCESS;
#endif  /* FORWARD */

#ifdef TLS_VERIFY
    case KRB5_TLS_VERIFY:
        if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
            krb5_data reply, msg;
            char tls_verify[24];
            krb5_replay_data repdata;
            krb5_error_code r;

            ssl_get_server_finished(&tls_verify[0],12);
            ssl_get_client_finished(&tls_verify[12],12);

            reply.data = (char *)data;
            reply.length = cnt;

			krb5_auth_con_genaddrs(k5_context, auth_context, ttyfd,
								   KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR);

            if (r = krb5_rd_safe(k5_context,auth_context,&reply,&msg,&repdata))
              {
                com_err("", r, "decoding tls verifier");
                krb5_errno = r;
                makestr(&krb5_errmsg,"TLS verify failure");
                auth_finished(AUTH_REJECT);
                return(AUTH_FAILURE);
            }
            if ( msg.length == 24 && !memcmp(msg.data,tls_verify,24) )
                 krb5_tls_verified = 1;
            krb5_free_data_contents(k5_context,&msg);
            if (krb5_tls_verified)
                return(AUTH_SUCCESS);
        }
        printf("Man in the middle attack detected.  Session terminated.\r\n");
        netclos();
        krb5_errno = -1;
        makestr(&krb5_errmsg,"TLS verify failure");
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
#endif /* CK_SSL */

    default:
        krb5_errno = -1;
        makestr(&krb5_errmsg,"Unknown reply type");
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;                        /* Unknown reply type */
    }
}

#ifdef FORWARD
/* Decode, decrypt and store the forwarded creds in the local ccache. */
/* Needed for KRB5_FORWARD                                            */
static krb5_error_code
rd_and_store_for_creds(context, auth_context, inbuf, client)
    krb5_context context;
    krb5_auth_context auth_context;
    krb5_data *inbuf;
    krb5_const_principal client;
{
    krb5_creds ** creds=NULL;
    krb5_error_code retval;
    krb5_ccache ccache=NULL;

#ifdef HEIMDAL
    /*
    Heimdal Telnetd creates the cache file at this point and sets
    the KRB5CCNAME environment variable.

    struct passwd *pwd;
    char ccname[1024];

    pwd = getpwnam(szUserNameRequested);
    if (pwd == NULL)
        break;
    snprintf(ccname, sizeof(ccname)-1, "FILE:/tmp/krb5cc_%u",pwd->pw_uid);
    retval = krb5_cc_resolve(context,ccname,&ccache);

    chown(ccname + 5, pwd->pw_uid, -1);
    */
#endif /* HEIMDAL */

    if (retval = k5_get_ccache(context,&ccache,NULL))
        return(retval);

#ifdef HEIMDAL
    if ((retval = krb5_cc_initialize(context, ccache, client)))
        return(retval);

    if ((retval = krb5_rd_cred(context, auth_context, ccache, inbuf)))
        return(retval);
#else /* HEIMDAL */
    if ((retval = krb5_rd_cred(context, auth_context, inbuf, &creds, NULL)))
        return(retval);

    if ((retval = krb5_cc_initialize(context, ccache, creds[0]->client)))
        goto cleanup;

    if ((retval = krb5_cc_store_cred(context, ccache, creds[0])))
        goto cleanup;

    if ((retval = krb5_cc_close(context, ccache)))
        goto cleanup;

  cleanup:
    krb5_free_tgt_creds(context, creds);
#endif /* HEIMDAL */
    return retval;
}
#endif /* FORWARD */

/*
 *
 * K5_auth_is.
 *
 */

static int
#ifdef CK_ANSIC
k5_auth_is(int how, unsigned char *data, int cnt)
#else
k5_auth_is(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    int r = 0;
    krb5_principal server;
    krb5_keyblock *newkey = NULL;
    krb5_data outbuf;
    char errbuf[128]="";
    char *getenv();
#ifndef HEIMDAL
    krb5_authenticator *authenticator;
    krb5_keytab keytabid = 0;
#endif /* HEIMDAL */
    krb5_data inbuf;
#ifdef CK_ENCRYPTION
    Session_Key skey;
#endif /* CK_ENCRYPTION */
    char princ[256]="";
    int len;

    data += 4;                                  /* Point to status byte */
    cnt -= 4;

    ckhexdump("k5_auth_is data",data,cnt);
    debug(F111,"k5_auth_is","how",how);

    if (cnt-- < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }
    switch (*data++) {
    case KRB_AUTH:
        k5_auth.data = (char *)data;
        k5_auth.length = cnt;

        debug(F110,"k5_auth_is","KRB_AUTH",0);
        debug(F111,"k5_auth_is","auth_context",auth_context);

        if (!r && !auth_context) {
            r = krb5_auth_con_init(k5_context, &auth_context);
            debug(F111,"k5_auth_is","krb5_auth_con_init",r);
        }

#ifdef HEIMDAL
        if (!r)
            r = krb5_auth_con_setaddrs_from_fd(k5_context,
					       auth_context,
					       &ttyfd);
        if (!r)
            r = krb5_sock_to_principal(k5_context,
				       0,
				       "host",
                                       KRB5_NT_SRV_HST,
				       &server);
        if (!r)
#else /* HEIMDAL */
        if (!r) {
            krb5_rcache rcache = NULL;

            r = krb5_auth_con_getrcache(k5_context, auth_context,
                                         &rcache);
            debug(F111,"k5_auth_is","krb5_auth_con_getrcache",r);

            if (!r && !rcache) {
                /* Do not resolve server's principal name, we will check */
                /* for validity after the krb5_rd_req() call.            */
                r = krb5_sname_to_principal(k5_context, 0, 0,
                                             KRB5_NT_SRV_HST, &server);
                debug(F111,"k5_auth_is","krb5_sname_to_principal",r);

                if (!r) {
                    r = krb5_get_server_rcache(k5_context,
                        krb5_princ_component(k5_context, server, 0),
                                                &rcache);
                    debug(F111,"k5_auth_is","krb5_get_server_rcache",r);
                    krb5_free_principal(k5_context, server);
                }
            }
            if (!r) {
                r = krb5_auth_con_setrcache(k5_context,
                                             auth_context, rcache);
                debug(F111,"k5_auth_is","krb5_auth_con_setrcache",r);
            }
        }
        if (!r && k5_keytab) {
            r = krb5_kt_resolve(k5_context,
                                 k5_keytab, &keytabid);
            debug(F111,"k5_auth_is","krb5_kt_resolve",r);
        }
#endif /* HEIMDAL */
        if (!r) {
            r = krb5_rd_req(k5_context, &auth_context, &k5_auth,
#ifdef HEIMDAL
                             server, NULL, NULL,
#else /* HEIMDAL */
                             NULL, keytabid, NULL,
#endif /* HEIMDAL */
                             &k5_ticket);
            debug(F111,"k5_auth_is","krb5_rd_req",r);
        }
        if (r) {
            (void) ckstrncpy(errbuf, "krb5_rd_req failed: ",sizeof(errbuf));
            (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
            goto errout;
        }
#ifdef HEIMDAL
        krb5_free_principal(k5_context, server);

        {
            char type_check[26];

            /* UPDATE for START_TLS. AUTH_ENCRYPT_START_TLS and inclusion of */
            /* client and then server finished messages. */

            type_check[0] = AUTHTYPE_KERBEROS_V5;
            type_check[1] = how;        /* not broken into parts */
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
                ckhexdump("k5_auth_is type_check",type_check,26);
            }
#endif /* CK_SSL */

            r = krb5_verify_authenticator_checksum(k5_context,
                                                    auth_context,
                                                    type_check,
#ifdef CK_SSL
                ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) ? 26 :
#endif /* CK_SSL */
                                                    2);
        }
#else /* HEIMDAL */
        len = krb5_princ_component(k5_context,k5_ticket->server,0)->length;
        if (len < 256)
        {
            memcpy(princ,
                   krb5_princ_component(k5_context,k5_ticket->server,0)->data,
                   len);
            princ[len] = '\0';
        }
        if ( strcmp((krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME), princ) )
        {
            debug(F110,"k5_auth_is incorrect service name",princ,0);
            ckstrncpy(errbuf,"incorrect service name: ",sizeof(errbuf));
            ckstrncat(errbuf,krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME,
                     sizeof(errbuf));
            ckstrncat(errbuf," != ",sizeof(errbuf));
            ckstrncat(errbuf,princ,sizeof(errbuf));
            goto errout;
        }

        r = krb5_auth_con_getauthenticator(k5_context,
                                            auth_context,
                                            &authenticator);
        debug(F111,"k5_auth_is","krb5_auth_con_getauthenticator",r);
        if (r) {
            (void) ckstrncpy(errbuf,
                             "krb5_auth_con_getauthenticator failed: ",
                             sizeof(errbuf)
                             );
            (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
            goto errout;
        }

        if (authenticator->checksum) {
            char type_check[26];
            krb5_checksum *cksum = authenticator->checksum;
            krb5_keyblock *key;

            /* UPDATE for START_TLS. AUTH_ENCRYPT_START_TLS and inclusion of */
            /* client and then server finished messages. */

            type_check[0] = AUTHTYPE_KERBEROS_V5;
            type_check[1] = how;        /* not broken into parts */
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
                ckhexdump("k5_auth_is type_check",type_check,26);
            }
#endif /* CK_SSL */

            r = krb5_auth_con_getkey(k5_context, auth_context,
                                      &key);
            debug(F111,"k5_auth_is","krb5_auth_con_getkey",r);
            if (r) {
                (void) ckstrncpy(errbuf, "krb5_auth_con_getkey failed: ",
                                  sizeof(errbuf));
                (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
                goto errout;
            }

            r = krb5_verify_checksum(k5_context,
                                      cksum->checksum_type,
                                      cksum,
                                      &type_check,
                  ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) ? 26 :
                                      2,
                                      key->contents,
                                      key->length
                                      );
            debug(F111,"k5_auth_is","krb5_verify_checksum",r);
            if (r) {
                (void) ckstrncpy(errbuf,
                                 "checksum verification failed: ",
                                 sizeof(errbuf)
                                 );
                (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
                goto errout;
            }
            krb5_free_keyblock(k5_context, key);
        } else {
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_USING_TELOPT) {
                (void) strcpy(errbuf,
                               "authenticator is missing required checksum");
                goto errout;
            }
        }

        krb5_free_authenticator(k5_context, authenticator);
#endif /* HEIMDAL */

#ifdef TLS_VERIFY
        if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
            krb5_data in, msg;
            char tls_verify[24];
            krb5_replay_data repdata;

            ssl_get_server_finished(&tls_verify[0],12);
            ssl_get_client_finished(&tls_verify[12],12);

            in.data = tls_verify;
            in.length = 24;

            krb5_auth_con_genaddrs(k5_context, auth_context, ttyfd,
								   KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR);
            if (r = krb5_mk_safe(k5_context,auth_context,&in,&msg,&repdata)) {
                com_err("", r, "encoding tls verifier");
                (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
                goto errout;
            }
            SendK5AuthSB(KRB5_TLS_VERIFY, msg.data, msg.length);
            krb5_free_data_contents(k5_context,&msg);
        }
#endif /* CK_SSL */
        if ((how & AUTH_HOW_MASK) == AUTH_HOW_MUTUAL) {
            /* do ap_rep stuff here */
            if ((r = krb5_mk_rep(k5_context,
#ifdef HEIMDAL
                                  &auth_context,
#else /* HEIMDAL */
                                  auth_context,
#endif /* HEIMDAL */
                                  &outbuf))) {
                debug(F111,"k5_auth_is","krb5_mk_rep",r);
                (void) ckstrncpy(errbuf, "Make reply failed: ",sizeof(errbuf));
                (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
                goto errout;
            }
            debug(F111,"k5_auth_is","krb5_mk_rep",r);

            SendK5AuthSB(KRB5_RESPONSE, outbuf.data, outbuf.length);
            mutual_complete = 1;
        }

#ifdef HEIMDAL
        {
            char * name = NULL;
            if (krb5_unparse_name(k5_context, k5_ticket->client,
                                   &name))
            {
                szUserNameAuthenticated[0] = '\0';
            } else {
                ckstrncpy(szUserNameAuthenticated,UIDBUFLEN,name);
                free(name);
            }
        }
#else /* HEIMDAL */
        if ( krb5_aname_to_localname(k5_context,
                                      k5_ticket->enc_part2->client,
                                      UIDBUFLEN,szUserNameAuthenticated) )
            szUserNameAuthenticated[0] = '\0';
#endif /* HEIMDAL */

        SendK5AuthSB(KRB_ACCEPT, szUserNameAuthenticated,
                      szUserNameAuthenticated[0] ? -1 : 0);
        accept_complete = 1;
        ckmakmsg(strTmp,sizeof(strTmp),
                 "Kerberos5 identifies him as ``",
                 szUserNameAuthenticated,"''",NULL);
        printf("%s\r\n",strTmp);

        if (szUserNameRequested[0] &&
            krb5_kuserok(k5_context,
#ifdef HEIMDAL
                          k5_ticket->client,
#else /* HEIMDAL */
                          k5_ticket->enc_part2->client,
#endif /* HEIMDAL */
                          szUserNameRequested))
            auth_finished(AUTH_VALID);
        else
            auth_finished(AUTH_USER);

        krb5_auth_con_getremotesubkey(k5_context, auth_context,
                                       &newkey);
        if (k5_session_key) {
            krb5_free_keyblock(k5_context, k5_session_key);
            k5_session_key = 0;
        }
        if (newkey) {
            krb5_copy_keyblock(k5_context, newkey, &k5_session_key);
            krb5_free_keyblock(k5_context, newkey);
        } else {
            krb5_copy_keyblock(k5_context,
#ifdef HEIMDAL
                                &k5_ticket->ticket.key,
#else /* HEIMDAL */
                                k5_ticket->enc_part2->session,
#endif /* HEIMDAL */
                                &k5_session_key);
        }

#ifdef CK_ENCRYPTION
#ifdef HEIMDAL
        skey.type = k5_session_key->keyvalue.length == 8 ? SK_DES : SK_GENERIC;
        skey.length = k5_session_key->keyvalue.length;
        skey.data = k5_session_key->keyvalue.data;
#else /* HEIMDAL */
        skey.type = k5_session_key->length == 8 ? SK_DES : SK_GENERIC;
        skey.length = k5_session_key->length;
        skey.data = k5_session_key->contents;
#endif /* HEIMDAL */
        encrypt_session_key(&skey, AUTH_SERVER_TO_CLIENT);
#endif /* CK_ENCRYPTION */
        debug(F100,"k5_auth_is AUTH_SUCCESS","",0);
        krb5_errno = r;
        if ( krb5_errno )
            makestr(&krb5_errmsg,error_message(krb5_errno));
        else
            makestr(&krb5_errmsg,strTmp);
        return AUTH_SUCCESS;

#ifdef FORWARD
    case KRB5_FORWARD:
        if ( !forward_flag ) {
            SendK5AuthSB(KRB5_FORWARD_REJECT,
                          "forwarded credentials are being refused.",
                          -1);
            return(AUTH_SUCCESS);
        }

        inbuf.length = cnt;
        inbuf.data = (char *)data;
        if (
#ifndef HEIMDAL
            (!krb5_d_no_addresses &&
            (r = krb5_auth_con_genaddrs(k5_context,auth_context,g_kstream->fd,
                              KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR))) ||
#endif /* HEIMDAL */
            (r = rd_and_store_for_creds(k5_context, auth_context,&inbuf,
#ifdef HEIMDAL
                                         k5_ticket->client
#else /* HEIMDAL */
                                         k5_ticket->enc_part2->client
#endif /* HEIMDAL */
                                         ))) {
            (void) ckstrncpy(errbuf, "Read forwarded creds failed: ",
                              sizeof(errbuf));
            (void) ckstrncat(errbuf, error_message(r),sizeof(errbuf));
            SendK5AuthSB(KRB5_FORWARD_REJECT, errbuf, -1);
            printf("Could not read forwarded credentials\r\n");
            krb5_errno = r;
            makestr(&krb5_errmsg,error_message(krb5_errno));
        }
        else {
            SendK5AuthSB(KRB5_FORWARD_ACCEPT, 0, 0);
            ckstrncpy(strTmp,"Forwarded credentials obtained",sizeof(strTmp));
            printf("%s\r\n",strTmp);
            krb5_errno = 0;
            makestr(&krb5_errmsg,strTmp);
        }
        /* A failure to accept forwarded credentials is not an */
        /* authentication failure.                             */
        return AUTH_SUCCESS;
#endif  /* FORWARD */
    default:
        printf("Unknown Kerberos option %d\r\n", data[-1]);
        SendK5AuthSB(KRB_REJECT, 0, 0);
        break;
    }
    auth_finished(AUTH_REJECT);
    return AUTH_FAILURE;

  errout:
    SendK5AuthSB(KRB_REJECT, errbuf, -1);
    krb5_errno = r;
    makestr(&krb5_errmsg,errbuf);
    printf("%s\r\n", errbuf);
    if (auth_context) {
        krb5_auth_con_free(k5_context, auth_context);
        auth_context = 0;
    }
    auth_finished(AUTH_REJECT);
    return AUTH_FAILURE;
}

#ifdef FORWARD
int
#ifdef CK_ANSIC
kerberos5_forward(void)
#else
kerberos5_forward()
#endif
{
    krb5_error_code r;
    krb5_ccache ccache=NULL;
    krb5_principal client = 0;
    krb5_principal server = 0;
    krb5_data forw_creds;
#ifdef HEIMDAL
    krb5_creds      creds;
#endif /* HEIMDAL */

    forw_creds.data = 0;

    r = k5_get_ccache(k5_context,&ccache,NULL);
    if ( r ) {
        com_err(NULL, r, "Kerberos V5: could not get default ccache");
        krb5_errno = r;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(AUTH_FAILURE);
    }

    if ((r = krb5_cc_get_principal(k5_context, ccache, &client))) {
        com_err(NULL, r, "Kerberos V5: could not get default principal");
        goto cleanup;
    }

#ifdef HEIMDAL
    memset(&creds, 0, sizeof(creds));
    creds.client = client;

    if (r = krb5_build_principal(k5_context,
                             &creds.server,
                             strlen(client->realm),
                              client->realm,
                              "krbtgt",
                              client->realm,
                                  NULL)) {
        com_err(NULL, r, "Kerberos V5: could not get principal");
        goto cleanup;
    }

    creds.times.endtime = 0;

    if (r = krb5_get_forwarded_creds(k5_context,
                                      auth_context,
                                      ccache,
                                      0,
                                      szHostName,
                                      &creds,
                                      &forw_creds)) {
        com_err(NULL, r, "Kerberos V5: error getting forwarded creds");
        goto cleanup;
    }
#else /* HEIMDAL */
    /* we should not need to make this call since we are storing the */
    /* server's principal in fwd_server from our call to             */
    /* krb5_sname_to_principal() in k5_auth_send()                   */
    if (fwd_server == NULL) {
        if ((r = krb5_sname_to_principal(k5_context, szHostName,
                                 krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME,
                                          KRB5_NT_SRV_HST, &server))) {
            com_err(NULL, r, "Kerberos V5: could not make server principal");
            goto cleanup;
        }
    }

    if (!krb5_d_no_addresses &&
        (r = krb5_auth_con_genaddrs(k5_context, auth_context, g_kstream->fd,
                             KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR)))
    {
        com_err(NULL, r, "Kerberos V5: could not gen local full address");
        goto cleanup;
    }

    if (r = krb5_fwd_tgt_creds(k5_context, auth_context, 0, client,
                                fwd_server ? fwd_server : server,
                                ccache, forwardable_flag, &forw_creds)) {
        com_err(NULL, r, "Kerberos V5: error getting forwardable credentials");
        goto cleanup;
    }
#endif /* HEIMDAL */

    /* Send forwarded credentials */
    if (!SendK5AuthSB(KRB5_FORWARD, forw_creds.data, forw_creds.length)) {
        printf("Kerberos V5 forwarding error!\r\n%s\r\n",
                    "Not enough room for authentication data");
    }

cleanup:
    if (client)
        krb5_free_principal(k5_context, client);
    if (server)
        krb5_free_principal(k5_context, server);
#ifdef HEIMDAL
    krb5_data_free(&forw_creds);
#else /* HEIMDAL */
    krb5_free_data_contents(k5_context,&forw_creds);
#endif /* HEIMDAL */
    krb5_cc_close(k5_context, ccache);

    krb5_errno = r;
    makestr(&krb5_errmsg,krb5_errno?error_message(krb5_errno):"OK");
    return(r?AUTH_FAILURE:AUTH_SUCCESS);
}
#endif /* FORWARD */
#else /* KRB5 */
int
ck_krb5_autoget_TGT(char * dummy)
{
    return(0);
}
#ifdef CK_KERBEROS
int
#ifdef CK_ANSIC
ck_krb5_initTGT( struct krb_op_data * op, struct krb5_init_data * init,
                 struct krb4_init_data * k4_init)
#else
ck_krb5_initTGT(op,init,k4_init)
    krb_op_data * op; struct krb5_init_data * init;
    struct krb4_init_data * k4_init;
#endif /* CK_ANSIC*/
{
    return(-1);
}

int
#ifdef CK_ANSIC
ck_krb5_destroy(struct krb_op_data * op)
#else
ck_krb5_destroy(op) struct krb_op_data * op;
#endif
{
    return(-1);
}

int
#ifdef CK_ANSIC
ck_krb5_list_creds(struct krb_op_data * op, struct krb5_list_cred_data * lc)
#else
ck_krb5_list_creds(op,lc)
    struct krb_op_data * op; struct krb5_list_cred_data * lc;
#endif
{
    return(-1);
}
#else /* CK_KERBEROS */
int
#ifdef CK_ANSIC
ck_krb5_initTGT(void * op, void * init, void * k4_init )
#else
ck_krb5_initTGT(op,init,k4_init)
    void * op; void * init; void * k4_init;
#endif /* CK_ANSIC*/
{
    return(-1);
}

int
#ifdef CK_ANSIC
ck_krb5_destroy(void * op)
#else
ck_krb5_destroy(op) void * op;
#endif
{
    return(-1);
}

int
#ifdef CK_ANSIC
ck_krb5_list_creds(void * op, void * lc)
#else
ck_krb5_list_creds(op,lc)
    void * op; void * lc;
#endif
{
    return(-1);
}
#endif /* CK_KERBEROS */
#endif /* KRB5 */

#ifdef GSSAPI_KRB5
/*
 *
 * gssk5_auth_send - gets authentication bits we need to send to KDC.
 *
 * Result is left in k5_auth
 *
 * Returns: 0 on failure, 1 on success
 *
 */

static int
#ifdef CK_ANSIC
gssk5_auth_send(int how, int encrypt, int forward)
#else
gssk5_auth_send(how,encrypt,forward) int how; int encrypt; int forward;
#endif
{
    OM_uint32 maj_stat, min_stat;
#ifdef KRB5
    char * realm = NULL;
    char tgt[256];
#endif /* KRB5 */

    gss_chan.initiator_addrtype = GSS_C_AF_INET; /* OM_uint32  */
    gss_chan.initiator_address.length = 4;
    gss_chan.initiator_address.value = &myctladdr.sin_addr.s_addr;
    gss_chan.acceptor_addrtype = GSS_C_AF_INET; /* OM_uint32 */
    gss_chan.acceptor_address.length = 4;
    gss_chan.acceptor_address.value = &hisctladdr.sin_addr.s_addr;
    gss_chan.application_data.length = 0;
    gss_chan.application_data.value = 0;

#ifdef KRB5
    realm = ck_krb5_realmofhost(ftp_host);
    if (realm) {
        ckmakmsg(tgt,sizeof(tgt),"krbtgt/",realm,"@",realm);
        debug(F110,"ftp_auth(GSSAPI) TGT",tgt,0);
        if ( krb5_autoget &&
             !((ck_krb5_tkt_isvalid(NULL,tgt) > 0) ||
                (ck_krb5_is_tgt_valid() > 0)) )
            ck_krb5_autoget_TGT(realm);
    }
#endif /* KRB5 */

    /* Blob from gss-client */
    /* host@hostname */
    /* the V5 GSSAPI binding canonicalizes this for us... */
    ckmakmsg(gss_stbuf,GSS_BUFSIZ,
             krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME,
             "@",
             szHostName,
              NULL
              );
    fprintf(stderr, "Authenticating to <%s>...\n", gss_stbuf);
    gss_send_tok.value = gss_stbuf;
    gss_send_tok.length = strlen(gss_stbuf);
    maj_stat = gss_import_name(&min_stat, &gss_send_tok,
                                gss_nt_service_name,
                                &gss_target_name
                                );
    if (maj_stat != GSS_S_COMPLETE) {
        user_gss_error(maj_stat, min_stat, "parsing name");
        secure_error("name parsed <%s>\n", gss_stbuf);
        return(0);
    }
    token_ptr = GSS_C_NO_BUFFER;
    gcontext = GSS_C_NO_CONTEXT; /* structure copy */

    fprintf(stderr, "calling gss_init_sec_context\n");
    maj_stat =
        gss_init_sec_context(&min_stat,
                              GSS_C_NO_CREDENTIAL,
                              &gcontext,
                              gss_target_name,
                              gss_mech_krb5,
                              GSS_C_MUTUAL_FLAG |
                              GSS_C_REPLAY_FLAG |
                              ((forward && forward_flag) ?
                                GSS_C_DELEG_FLAG : 0),
                              0,
                              (krb5_d_no_addresses ? /* channel bindings */
                                GSS_C_NO_CHANNEL_BINDINGS :
                                &gss_chan),
                              gss_token_ptr,
                              NULL,     /* ignore mech type */
                              &gss_send_tok,
                              NULL,     /* ignore ret_flags */
                              NULL
                              );        /* ignore time_rec */


        if (maj_stat != GSS_S_COMPLETE &&
             maj_stat != GSS_S_CONTINUE_NEEDED) {
            user_gss_error(maj_stat,
                            min_stat,
                            "initializing context"
                            );
            gss_release_name(&min_stat, &gss_target_name);
            return(0);
        }
        return(1);
}

/*
 * gssk5_auth_reply -- checks the reply for mutual authentication.
 */
static int
#ifdef CK_ANSIC
gssk5_auth_reply(int how, unsigned char *data, int cnt)
#else
gssk5_auth_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    data += 4;                                  /* Point to status byte */
    cnt -=5;

    switch (*data++) {
    case GSS_REJECT:
        if (cnt > 0) {
            char *s;
            int len;
            ckstrncpy(strTmp,"GSSAPI refuses authentication because\r\n",
                      sizeof(strTmp));
            len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                s = strTmp + strlen(strTmp);
                memcpy(s, data, cnt);           /* safe */
                s[cnt] = 0;
            }
        } else
            ckstrncpy(strTmp,"GSSAPI refuses authentication",
                      sizeof(strTmp));
        printf("GSSAPI authentication failed!\r\n%s\r\n",strTmp);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;

    case GSS_ACCEPT:
        if ( cnt > 0 ) {
            char *s;
            int len;
            ckstrncpy(strTmp,"GSSAPI accepts you as ",sizeof(strTmp));
            len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                s = strTmp + strlen(strTmp);
                memcpy(s,data,cnt);
                s[cnt] = 0;
            }
        }
        accept_complete = 1;
        printf("%s\r\n",strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;

    case GSS_RESPONSE:
        gss_token_ptr = &gss_recv_tok;
        gss_recv_tok.value = data;
        gss_recv_tok.length = cnt;

        maj_stat =
            gss_init_sec_context(&min_stat,
                                  GSS_C_NO_CREDENTIAL,
                                  &gcontext,
                                  gss_target_name,
                                  gss_krb5_mech,
                                  GSS_C_MUTUAL_FLAG |
                                  GSS_C_REPLAY_FLAG |
                                  (forward_flag ?
                                    GSS_C_DELEG_FLAG : 0),
                                  0,
                                  (krb5_d_no_addresses ? /* channel bindings */
                                    GSS_C_NO_CHANNEL_BINDINGS :
                                    &gss_chan),
                                  gss_token_ptr,
                                  NULL, /* ignore mech type */
                                  &gss_send_tok,
                                  NULL, /* ignore ret_flags */
                                  NULL
                                  );    /* ignore time_rec */

        if ( maj_stat == GSS_S_COMPLETE )
        {

        } else if ( maj_stat == CSS_S_CONTINUE_NEEDED ) {
        } else {
        }

        ckstrncpy(strTmp,"Remote machine has been mutually authenticated",
                  sizeof(strTmp));
        printf("%s\r\n",strTmp);
        auth_finished(AUTH_USER);
        return AUTH_SUCCESS;

    default:
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;                        /* Unknown reply type */
    }
}

/*
 *
 * gssk5_auth_is.
 *
 */

static int
#ifdef CK_ANSIC
k5_auth_is(int how, unsigned char *data, int cnt)
#else
k5_auth_is(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    int replied = 0;
    gss_cred_id_t server_creds, deleg_creds;
    gss_name_t client;
    int ret_flags;
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    OM_uint32 acquire_maj,
      acquire_min,
      accept_maj,
      accept_min,
      stat_maj,
      stat_min;
    gss_OID mechid;
    gss_buffer_desc tok, out_tok;
    char gbuf[GSS_BUFSIZ];
    u_char gout_buf[GSS_BUFSIZ];
    char localname[MAXHOSTNAMELEN];
    char service_name[MAXHOSTNAMELEN+10];
    char **service;
    struct hostent *hp;

    data += 4;                                  /* Point to status byte */
    cnt -= 4;

    ckhexdump("gssk5_auth_is data",data,cnt);
    debug(F111,"gssk5_auth_is","how",how);

    if (cnt-- < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }
    switch (*data++) {
    case GSS_AUTH:
        gss_chan.initiator_addrtype = GSS_C_AF_INET;
        gss_chan.initiator_address.length = 4;
        gss_chan.initiator_address.value = &his_addr.sin_addr.s_addr;
        gss_chan.acceptor_addrtype = GSS_C_AF_INET;
        gss_chan.acceptor_address.length = 4;
        gss_chan.acceptor_address.value = &ctrl_addr.sin_addr.s_addr;
        gss_chan.application_data.length = 0;
        gss_chan.application_data.value = 0;

        tok.value = data;
        tok.length = cnt;

        if (gethostname(localname, MAXHOSTNAMELEN)) {
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
        if (!(hp = gethostbyname(localname))) {
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }
#ifdef HADDRLIST
        hp = ck_copyhostent(hp);
#endif /* HADDRLIST */
        strncpy(localname, hp->h_name, sizeof(localname) - 1);
        localname[sizeof(localname) - 1] = '\0';

        sprintf(service_name, "%s@%s", *service, localname);
        name_buf.value = service_name;
        name_buf.length = strlen(name_buf.value) + 1;
        stat_maj = gss_import_name(&stat_min, &name_buf,
                                    gss_nt_service_name,
                                    &server_name);
        if (stat_maj != GSS_S_COMPLETE) {
            auth_finished(AUTH_REJECT);
            return AUTH_FAILURE;
        }

        acquire_maj = gss_acquire_cred(&acquire_min, server_name, 0,
                                        GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
                                        &server_creds, NULL, NULL);
        (void) gss_release_name(&stat_min, &server_name);

        if (acquire_maj != GSS_S_COMPLETE) {
            reply_gss_error(535, accept_maj, accept_min,
                                 "accepting context");
            syslog(LOG_ERR, "failed accepting context");
            (void) gss_release_cred(&stat_min, &server_creds);
            if (ret_flags & GSS_C_DELEG_FLAG)
                (void) gss_release_cred(&stat_min,
                                         &deleg_creds);
            return 0;
        }

        gcontext = GSS_C_NO_CONTEXT;
        accept_maj = gss_accept_sec_context(&accept_min,
                                            &gcontext, /* context_handle */
                                            /* verifier_cred_handle */
                                            server_creds,
                                            &tok, /* input_token */
					    (krb5_d_no_addresses ?
					     /* channel bindings */
                                               GSS_C_NO_CHANNEL_BINDINGS :
                                               &gss_chan),
                                             &client, /* src_name */
                                            &mechid, /* mech_type */
                                            &out_tok, /* output_token */
                                            &ret_flags,
                                            NULL,       /* ignore time_rec */
                                            /* forwarded credentials */
                                            &deleg_creds
                                            );

        if (accept_maj!=GSS_S_COMPLETE && accept_maj!=GSS_S_CONTINUE_NEEDED) {
            reply_gss_error(535, accept_maj, accept_min,
                             "accepting context");
            syslog(LOG_ERR, "failed accepting context");
            (void) gss_release_cred(&stat_min, &server_creds);
            if (ret_flags & GSS_C_DELEG_FLAG)
                (void) gss_release_cred(&stat_min,
                                         &deleg_creds);
            return 0;
        }

        if (out_tok.length) {
            if (kerror = radix_encode(out_tok.value,gbuf,&out_tok.length, 0)) {
                secure_error("Couldn't encode ADAT reply (%s)",
                             radix_error(kerror));
                syslog(LOG_ERR, "couldn't encode ADAT reply");
                (void) gss_release_cred(&stat_min, &server_creds);
                if (ret_flags & GSS_C_DELEG_FLAG)
                        (void) gss_release_cred(&stat_min,
                                                &deleg_creds);
                return(0);
            }
            if (stat_maj == GSS_S_COMPLETE) {
                reply(235, "ADAT=%s", gbuf);
                replied = 1;
            } else {
                /* If the server accepts the security data, and
                   requires additional data, it should respond
                   with reply code 335. */
                reply(335, "ADAT=%s", gbuf);
            }
            (void) gss_release_buffer(&stat_min, &out_tok);
        }

        if (stat_maj == GSS_S_COMPLETE) {
            /* GSSAPI authentication succeeded */
            stat_maj = gss_display_name(&stat_min, client,
                                         &client_name, &mechid);
            if (stat_maj != GSS_S_COMPLETE) {
                /* "If the server rejects the security data (if
                   a checksum fails, for instance), it should
                   respond with reply code 535." */
                reply_gss_error(535, stat_maj, stat_min,
                                "extracting GSSAPI identity name");
                syslog(LOG_ERR, "gssapi error extracting identity");
                (void) gss_release_cred(&stat_min, &server_creds);
                if (ret_flags & GSS_C_DELEG_FLAG)
                        (void) gss_release_cred(&stat_min,
                                                &deleg_creds);
                return 0;
            }
            auth_type = temp_auth_type;
            temp_auth_type = NULL;

            (void) gss_release_cred(&stat_min, &server_creds);
            if (ret_flags & GSS_C_DELEG_FLAG) {
                if (want_creds)
                    ftpd_gss_convert_creds(client_name.value,
                                            deleg_creds);
                (void) gss_release_cred(&stat_min, &deleg_creds);
            }

            /* If the server accepts the security data, but does
               not require any additional data (i.e., the security
               data exchange has completed successfully), it must
               respond with reply code 235. */
            if (!replied)
            {
                if (ret_flags & GSS_C_DELEG_FLAG && !have_creds)
                  reply(235,
 "GSSAPI Authentication succeeded, but could not accept forwarded credentials"
                        );
                else
                  reply(235, "GSSAPI Authentication succeeded");
            }
            return(1);
        } else if (stat_maj == GSS_S_CONTINUE_NEEDED) {
            /* If the server accepts the security data, and
            requires additional data, it should respond with
            reply code 335. */
            reply(335, "more data needed");
            (void) gss_release_cred(&stat_min, &server_creds);
            if (ret_flags & GSS_C_DELEG_FLAG)
                (void) gss_release_cred(&stat_min, &deleg_creds);
            return(0);
        } else {
            /* "If the server rejects the security data (if
            a checksum fails, for instance), it should
            respond with reply code 535." */
            reply_gss_error(535, stat_maj, stat_min,
                             "GSSAPI failed processing ADAT");
            syslog(LOG_ERR, "GSSAPI failed processing ADAT");
            (void) gss_release_cred(&stat_min, &server_creds);
            if (ret_flags & GSS_C_DELEG_FLAG)
                (void) gss_release_cred(&stat_min, &deleg_creds);
            return(0);
        }

        debug(F100,"gssk5_auth_is AUTH_SUCCESS","",0);
        krb5_errno = r;
        if ( krb5_errno )
            makestr(&krb5_errmsg,error_message(krb5_errno));
        else
            makestr(&krb5_errmsg,strTmp);
        return AUTH_SUCCESS;

    default:
        printf("Unknown Kerberos option %d\r\n", data[-1]);
        SendGSSK5AuthSB(GSS_REJECT, 0, 0);
        break;
    }
    auth_finished(AUTH_REJECT);
    return AUTH_FAILURE;
}
#endif /* GSSAPI_KRB5 */

#ifdef CK_SRP
/*
 * Copyright (c) 1997 Stanford University
 *
 * The use of this software for revenue-generating purposes may require a
 * license from the owners of the underlying intellectual property.
 * Specifically, the SRP-3 protocol may not be used for revenue-generating
 * purposes without a license.
 *
 * NOTE: Columbia University has a license.
 *
 * Within that constraint, permission to use, copy, modify, and distribute
 * this software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notices and this permission
 * notice appear in all copies of the software and related documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

static void
srp_encode_length(data, num)
    unsigned char * data;
    int num;
{
    *data = (num >> 8) & 0xff;
    *++data = num & 0xff;
}

static int
srp_decode_length(data)
    unsigned char * data;
{
    return (((int) *data & 0xff) << 8) | (*(data + 1) & 0xff);
}

#ifdef PRE_SRP_1_7_3
static int
#ifdef CK_ANSIC
srp_reply(int how, unsigned char *data, int cnt)
#else
srp_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    struct t_num n;
    struct t_num g;
    struct t_num s;
    struct t_num B;
    struct t_num * A;
    char type_check[26];
    int pflag;

#ifdef CK_ENCRYPTION
    Session_Key skey;
#endif /* ENCRYPTION */

    char * str=NULL;

    data += 4;                          /* Point to status byte */
    cnt  -= 4;

    if(cnt-- < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    switch(*data++) {
    case SRP_REJECT:
        ckmakmsg(strTmp,sizeof(strTmp),
                  "SRP refuses authentication for '",szUserName,
                  "'\r\n",NULL);
        if (cnt > 0) {
            int len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                str = strTmp + strlen(strTmp);
                memcpy(str,data,cnt);
                str[cnt] = 0;
            }
        }
        printf("SRP authentication failed!\r\n%s\r\n",strTmp);
        if (tc != NULL) {
            t_clientclose(tc);
            tc = NULL;
        }
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;

    case SRP_ACCEPT:
        if(cnt < RESPONSE_LEN || !srp_waitresp ||
            tc == NULL
            ) {
            printf("SRP Protocol error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        srp_waitresp = 0;

        if(t_clientverify(tc, data) == 0) {
            printf("SRP accepts you as %s\r\n",szUserName);
#ifdef CK_SSL
            if((ssl_active_flag || tls_active_flag) &&
                (how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                printf("TLS session parameters verified by SRP\r\n");
            } else
#endif /* CK_SSL */

#ifdef CK_ENCRYPTION
            {
                skey.type = SK_GENERIC;
                skey.length = SESSION_KEY_LEN;
                skey.data = tc->session_key;
                encrypt_session_key(&skey, AUTH_CLIENT_TO_SERVER);
            }
#endif /* ENCRYPTION */
            t_clientclose(tc);
            tc = NULL;
            accept_complete = 1;
            auth_finished(AUTH_VALID);
            return AUTH_SUCCESS;
        } else {
            printf("SRP server authentication failed!\r\n");
            t_clientclose(tc);
            tc = NULL;
            return(auth_resend(AUTHTYPE_SRP));
        }
        break;

    case SRP_PARAMS:
        if(!szUserName) {
            printf("No username available\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        n.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(n.len > cnt) {
            printf("n too long\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        n.data = data;
        data += n.len;
        cnt -= n.len;

        g.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(g.len > cnt) {
            printf("g too long\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        g.data = data;
        data += g.len;
        cnt -= g.len;

        s.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(s.len > cnt) {
            printf("salt too long\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        s.data = data;
        data += s.len;
        cnt -= s.len;

        /* If the parameters provided by the server cannot be
         * validated the following function will fail.
         */
        tc = t_clientopen(szUserName, &n, &g, &s);
        if (tc == NULL) {
            printf("SRP parameter initialization error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        A = t_clientgenexp(tc);
        if(A == NULL) {
            printf("SRP protocol error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        SendSRPAuthSB(SRP_EXP, A->data, A->len);

        if ( pwbuf[0] && pwflg ) {
            printf("SRP using %d-bit modulus for '%s'\r\n",
                   8 * n.len,
                   szUserName
                   );
            ckstrncpy(srp_passwd,pwbuf,sizeof(srp_passwd));
#ifdef OS2
            if ( pwcrypt )
                ck_encrypt((char *)srp_passwd);
#endif /* OS2 */
        } else {
            extern char * srppwprompt;
            char preface[128];
            int ok;

            if (srppwprompt && srppwprompt[0] &&
		(strlen(srppwprompt) + strlen(szUserName) - 2) <
		sizeof(preface)) {
                sprintf(preface,srppwprompt,szUserName);
            } else {
                ckmakxmsg( preface,sizeof(preface),
                          "SRP using ",ckitoa(8*n.len),"-bit modulus for '",
                          szUserName, "'", NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL);
            }
            ok = uq_txt( preface,"Password: ",2,NULL,
                         srp_passwd,sizeof(srp_passwd)-1,NULL,
			 DEFAULT_UQ_TIMEOUT);
            if ( !ok )
                srp_passwd[0] = '\0';
        }

        t_clientpasswd(tc, srp_passwd);
        memset(srp_passwd, 0, sizeof(srp_passwd));
        return AUTH_SUCCESS;

    case SRP_CHALLENGE:
        if(tc == NULL) {
            printf("SRP protocol error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

#ifndef PRE_SRP_1_4_5
        /*
         * The original SRP AUTH implementation did not protect against
         * tampering of the auth-type-pairs.  Therefore, when the
         * AUTH_ENCRYPT_MASK bits are zero, no extra data is inserted
         * into the SRP hash computation.  When AUTH_ENCRYPT_START_TLS
         * is set we also insert the SSL/TLS client and server finished
         * messages to ensure that there is no man in the middle attack
         * underway on the SSL/TLS connection.
         */
        if ((how & AUTH_ENCRYPT_MASK) != AUTH_ENCRYPT_OFF) {
            type_check[0] = AUTHTYPE_SRP;
            type_check[1] = how;
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
                t_clientaddexdata(tc,type_check,26);
            } else
#endif /* CK_SSL */
                t_clientaddexdata(tc,type_check,2);
        }
#endif /* PRE_SRP_1_4_5 */

        B.data = data;
        B.len = cnt;
        t_clientgetkey(tc, &B);

        SendSRPAuthSB(SRP_RESPONSE, t_clientresponse(tc), RESPONSE_LEN);
        srp_waitresp = 1;
        return AUTH_SUCCESS;

    default:
        return(auth_resend(AUTHTYPE_SRP));
    }
    return AUTH_FAILURE;
}

static int
#ifdef CK_ANSIC
srp_is(int how, unsigned char *data, int cnt)
#else
srp_is(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    char * pbuf = NULL;
    char * ptr;
#ifdef CK_ENCRYPTION
    Session_Key skey;
#endif
    struct t_num A;
    struct t_pw * tpw = NULL;
    struct t_conf * tconf = NULL;
    struct passwd * pass;
    static struct t_num * B = NULL;     /* Holder for B */
#ifdef CK_SSL
    char type_check[26];
#else
    char type_check[2];
#endif /* CK_SSL */

    if ((cnt -= 4) < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    data += 4;
    cnt  -= 1;
    switch(*data++) {
    case SRP_AUTH:
        /* Send parameters back to client */
        if(ts != NULL) {
            t_serverclose(ts);
            ts = NULL;
        }
        if(!szUserNameRequested[0]) {
            if (1)
                printf("No username available\r\n");
            SendSRPAuthSB(SRP_REJECT, (void *) "No username supplied", -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && ckxanon &&
             !strcmp(szUserNameRequested,"anonymous")) {
            SendSRPAuthSB(SRP_REJECT, (void *)
            "anonymous login cannot be performed with Secure Remote Password",
            -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
#endif /* CK_LOGIN */
#endif /* IKSD */
#ifndef PRE_SRP_1_4_4
        if(tpw == NULL) {
            if((tpw = t_openpw(NULL)) == NULL) {
                if (1)
                    printf("Unable to open password file\r\n");
                SendSRPAuthSB(SRP_REJECT, (void *) "No password file", -1);
                return(AUTH_FAILURE);
            }
        }
        if(tconf == NULL) {
            if((tconf = t_openconf(NULL)) == NULL) {
                if (1)
                  printf("Unable to open configuration file\r\n");
                SendSRPAuthSB(SRP_REJECT, (void *)"No configuration file", -1);
                return(AUTH_FAILURE);
            }
        }
        ts = t_serveropenfromfiles(szUserNameRequested, tpw, tconf);
        t_closepw(tpw);
        tpw = NULL;
        t_closeconf(tconf);
        tconf = NULL;
#else /* PRE_SRP_1_4_4 */
#ifdef COMMENT
        /* the code in this block should no longer be necessary on OS/2
           or Windows because I have added functionality to libsrp.lib
           to find the srp files.   4/22/2000
        */

        /* On Windows and OS/2 there is no well defined place for the */
        /* ETC directory.  So we look for either an SRP_ETC or ETC    */
        /* environment variable in that order.  If we find one we     */
        /* attempt to open the files manually.                        */
        /* We will reuse the strTmp[] for the file names. */
        ptr = getenv("SRP_ETC");
        if ( !ptr )
            ptr = getenv("ETC");
#ifdef NT
        if ( !ptr ) {
            DWORD len;
            len = AUTHTMPBL;

            len = GetWindowsDirectory(strTmp,len);
            if ( len > 0 && len < AUTHTMPBL) {
                if ( !isWin95() ) {
                    if ( len == 1 )
		      ckstrncat(strTmp,"SYSTEM32/DRIVERS/ETC",sizeof(strTmp));
                    else
		      ckstrncat(strTmp,"/SYSTEM32/DRIVERS/ETC",sizeof(strTmp));
                }
            }
            ptr = strTmp;
        }
#endif /* NT */
        if ( ptr ) {
            int len = strlen(ptr);
            int i;
	    if (ptr != strTmp)
		strcpy(strTmp,ptr);
            for ( i=0;i<len;i++ ) {
                if ( strTmp[i] == '\\' )
                    strTmp[i] = '/';
            }
            if ( strTmp[len-1] != '/' )
                ckstrncat(strTmp,"/tpasswd",sizeof(strTmp));
            else
                ckstrncat(strTmp,"tpasswd",sizeof(strTmp));
            tpw = t_openpwbyname(strTmp);

            ckstrncat(strTmp,".conf",sizeof(strTmp));
            tconf = t_openconfbyname(strTmp);
        }

        if ( tpw && tconf )
            ts = t_serveropenfromfiles(szUserNameRequested, tpw, tconf);
        else
            ts = t_serveropen(szUserNameRequested);
        if ( tpw ) {
            t_closepw(tpw);
            tpw = NULL;
        }
        if ( tconf ) {
            t_closeconf(tconf);
            tconf = NULL;
        }
#else /* COMMENT */
        ts = t_serveropen(szUserNameRequested);
#endif /* COMMENT */
#endif /* PRE_SRP_1_4_4 */

        if( ts == NULL ) {
            printf("User %s not found\r\n", szUserNameRequested);
            SendSRPAuthSB(SRP_REJECT, (void *) "Password not set", -1);
            return(AUTH_FAILURE);
        }

	pbuf = (char *)malloc(ts->n.len + ts->g.len + ts->s.len + 7);
	ptr = pbuf;

        srp_encode_length(ptr, ts->n.len);
        ptr += 2;
        memcpy(ptr, ts->n.data, ts->n.len);     /* safe */
        ptr += ts->n.len;

        srp_encode_length(ptr, ts->g.len);
        ptr += 2;
        memcpy(ptr, ts->g.data, ts->g.len);     /* safe */
        ptr += ts->g.len;

        srp_encode_length(ptr, ts->s.len);
        ptr += 2;
        memcpy(ptr, ts->s.data, ts->s.len);     /* safe */
        ptr += ts->s.len;

        SendSRPAuthSB(SRP_PARAMS, pbuf, ptr - pbuf);
	free(pbuf); pbuf = NULL;

        B = t_servergenexp(ts);
        ckstrncpy(szUserNameAuthenticated,szUserNameRequested,UIDBUFLEN);
        return AUTH_SUCCESS;

    case SRP_EXP:
        /* Client is sending A to us, compute challenge & expected response. */
        if (ts == NULL || B == NULL) {
            printf("Protocol error: SRP_EXP unexpected\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Protocol error: unexpected EXP",
                          -1
                          );
            return(AUTH_FAILURE);
        }

        /* Wait until now to send B, since it contains the key to "u" */
        SendSRPAuthSB(SRP_CHALLENGE, B->data, B->len);
        B = NULL;

#ifndef PRE_SRP_1_4_5
        /*
         * The original SRP AUTH implementation did not protect against
         * tampering of the auth-type-pairs.  Therefore, when the
         * AUTH_ENCRYPT_MASK bits are zero, no extra data is inserted
         * into the SRP hash computation.  When AUTH_ENCRYPT_START_TLS
         * is set we also insert the SSL/TLS client and server finished
         * messages to ensure that there is no man in the middle attack
         * underway on the SSL/TLS connection.
         */
        if ( (how & AUTH_ENCRYPT_MASK) != AUTH_ENCRYPT_OFF ) {
            type_check[0] = AUTHTYPE_SRP;
            type_check[1] = how;
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
            }
#endif /* CK_SSL */
            t_serveraddexdata(ts,type_check,
#ifdef CK_SSL
                  ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) ? 26 :
#endif /* CK_SSL */
                               2);
        }
#endif /* PRE_SRP_1_4_5 */

        A.data = data;
        A.len = cnt;
        ptr = t_servergetkey(ts, &A);

        if(ptr == NULL) {
            if (1)
              printf("Security alert: Trivial session key attempted\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Trivial session key detected",
                          -1
                          );
            return(AUTH_FAILURE);
        }
        srp_waitresp = 1;
        return AUTH_SUCCESS;

    case SRP_RESPONSE:
        /* Got the response; see if it's correct */
        if (!srp_waitresp ||
             ts == NULL
             ) {
            if (1)
              printf("Protocol error: SRP_RESPONSE unexpected\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Protocol error: unexpected RESPONSE",
                          -1
                          );
            return(AUTH_FAILURE);
        }
        srp_waitresp = 0;       /* we got a response */

        if (cnt < RESPONSE_LEN) {
            if (1)
              printf("Protocol error: malformed response\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Protocol error: malformed response",
                          -1
                          );
            return(AUTH_FAILURE);
        }

        if (t_serververify(ts, data) == 0) {
            SendSRPAuthSB(SRP_ACCEPT, t_serverresponse(ts), RESPONSE_LEN);
            accept_complete = 1;
#ifdef CK_ENCRYPTION
#ifdef CK_SSL
            if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
            {
                ckhexdump("SRP_RESPONSE ts",ts,sizeof(ts));
                ckhexdump("SRP_RESPONSE session_key",
                         ts->session_key,
                         SESSION_KEY_LEN
                         );
                skey.type = SK_GENERIC;
                skey.length = SESSION_KEY_LEN;
                skey.data = ts->session_key;
                encrypt_session_key(&skey, AUTH_SERVER_TO_CLIENT);
            }
#endif /* CK_ENCRYPTION */
            auth_finished(AUTH_VALID);
        }
        else {
            SendSRPAuthSB(SRP_REJECT, (void *) "Login incorrect", -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
        return AUTH_SUCCESS;

    default:
        printf("Unknown SRP option %d\r\n", data[-1]);
        SendSRPAuthSB(SRP_REJECT, (void *) "Unknown option received", -1);
        return(AUTH_FAILURE);
    }
}
#else /* PRE_SRP_1_7_3 */
static int
#ifdef CK_ANSIC
new_srp_reply(int how, unsigned char *data, int cnt)
#else
new_srp_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    data += 4;                          /* Point to status byte */
    cnt  -= 4;

    if(cnt-- < 1) {                     /* Matches with data++ */
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    switch(*data++) {
    case SRP_PARAMS: {
        struct t_num n;
        struct t_num g;
        struct t_num s;
        cstr * A;

        if(!szUserName) {
            printf("No username available\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        n.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(n.len > cnt) {
            printf("n too long\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        n.data = data;
        data += n.len;
        cnt -= n.len;

        g.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(g.len > cnt) {
            printf("g too long\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        g.data = data;
        data += g.len;
        cnt -= g.len;

        s.len = srp_decode_length(data);
        data += 2;
        cnt -= 2;
        if(s.len != cnt) {
            printf("invalid salt\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        s.data = data;
        data += s.len;
        cnt -= s.len;

        /* If the parameters provided by the server cannot be
         * validated the following function will fail.
         */
        c_srp = SRP_new(SRP_RFC2945_client_method());
        if (c_srp == NULL ||
	    SRP_set_username(c_srp, szUserName) != SRP_SUCCESS ||
	    SRP_set_params(c_srp,n.data,n.len,g.data,g.len,s.data,s.len) !=
	    SRP_SUCCESS) {
            printf("SRP Parameter initialization error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        A = cstr_new();
        if(SRP_gen_pub(c_srp, &A) != SRP_SUCCESS) {
            printf("SRP Error generating key exchange\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        SendSRPAuthSB(SRP_EXP, A->data, A->length);
        cstr_free(A);

        if ( pwbuf[0] && pwflg ) {
            printf("SRP using %d-bit modulus for '%s'\r\n",
                   8 * n.len,
                   szUserName
                   );
            ckstrncpy(srp_passwd,pwbuf,sizeof(srp_passwd));
#ifdef OS2
            if ( pwcrypt )
                ck_encrypt((char *)srp_passwd);
#endif /* OS2 */
        } else {
            extern char * srppwprompt;
            char preface[128];
            int ok;

            if (srppwprompt && srppwprompt[0] &&
		(strlen(srppwprompt) + strlen(szUserName) - 2) <
		sizeof(preface)) {
                sprintf(preface,srppwprompt,szUserName);
            } else {
                ckmakxmsg( preface,sizeof(preface),
                          "SRP using ",ckitoa(8*n.len),"-bit modulus for '",
                          szUserName, "'", NULL, NULL, NULL, NULL, NULL,
                          NULL, NULL);
            }
            ok = uq_txt(preface,"Password: ",2,NULL,
                        srp_passwd,sizeof(srp_passwd)-1,NULL,
			DEFAULT_UQ_TIMEOUT);
            if ( !ok )
                srp_passwd[0] = '\0';
        }

        if(SRP_set_auth_password(c_srp, srp_passwd) != SRP_SUCCESS) {
            memset(srp_passwd, 0, sizeof(srp_passwd));
            printf("SRP Error setting client password\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        memset(srp_passwd, 0, sizeof(srp_passwd));
        return AUTH_SUCCESS;
    }
    case SRP_CHALLENGE: {
        char type_check[26];
        cstr * resp = NULL;

        if(c_srp == NULL) {
            printf("SRP protocol error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        /*
         * The original SRP AUTH implementation did not protect against
         * tampering of the auth-type-pairs.  Therefore, when the
         * AUTH_ENCRYPT_MASK bits are zero, no extra data is inserted
         * into the SRP hash computation.  When AUTH_ENCRYPT_START_TLS
         * is set we also insert the SSL/TLS client and server finished
         * messages to ensure that there is no man in the middle attack
         * underway on the SSL/TLS connection.
         */
        if ((how & AUTH_ENCRYPT_MASK) != AUTH_ENCRYPT_OFF) {
            type_check[0] = AUTHTYPE_SRP;
            type_check[1] = how;
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
                SRP_add_ex_data(c_srp, type_check, 26);
            } else
#endif /* CK_SSL */
                SRP_add_ex_data(c_srp, type_check, 2);
        }

        if(SRP_compute_key(c_srp, &c_key, data, cnt) != SRP_SUCCESS) {
            printf("SRP ERROR: unable to compute client key\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }

        resp = cstr_new();
        if(SRP_respond(c_srp, &resp) != SRP_SUCCESS) {
            printf("SRP ERROR: unable to compute client response\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        SendSRPAuthSB(SRP_RESPONSE, resp->data, resp->length);
        cstr_free(resp);
        srp_waitresp = 1;
        return AUTH_SUCCESS;
    }
    case SRP_ACCEPT: {
#ifdef CK_ENCRYPTION
        Session_Key skey;
#endif /* ENCRYPTION */

        if(cnt < RESPONSE_LEN || !srp_waitresp || c_srp == NULL) {
            printf("SRP Protocol error\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
        srp_waitresp = 0;

        if(SRP_verify(c_srp, data, cnt) == SRP_SUCCESS) {
            printf("SRP accepts you as %s\r\n",szUserName);

#ifdef CK_SSL
            if((ssl_active_flag || tls_active_flag) &&
                (how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                printf("TLS session parameters verified by SRP\r\n");
            } else
#endif /* CK_SSL */
#ifdef CK_ENCRYPTION
            {
                skey.type = SK_GENERIC;
                skey.length = c_key->length;
                skey.data = c_key->data;
                encrypt_session_key(&skey, AUTH_CLIENT_TO_SERVER);
                cstr_clear_free(c_key);
                c_key = NULL;
            }
#endif /* CK_ENCRYPTION */
            accept_complete = 1;
            auth_finished(AUTH_VALID);
            SRP_free(c_srp);
            c_srp = NULL;
            return AUTH_SUCCESS;
        }
        else {
            printf("[ Error: SRP server authentication failed ]\r\n");
            return(auth_resend(AUTHTYPE_SRP));
        }
    }
    case SRP_REJECT: {
        char * str=NULL;

        ckmakmsg(strTmp,sizeof(strTmp),
                  "SRP refuses authentication for '",szUserName,
                  "'\r\n",NULL);
        if (cnt > 0) {
            int len = strlen(strTmp);
            if ( len + cnt < sizeof(strTmp) ) {
                str = strTmp + strlen(strTmp);
                memcpy(str,data,cnt);
                str[cnt] = 0;
            }
        }
        printf("SRP authentication failed!\r\n%s\r\n",strTmp);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }
    default:
        printf("Unknown SRP option %d\r\n", data[-1]);
        return(auth_resend(AUTHTYPE_SRP));
    }
    /* NEVER REACHED */
}

static int
#ifdef CK_ANSIC
new_srp_is(int how, unsigned char *data, int cnt)
#else
new_srp_is(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    char * pbuf = NULL;
    char * ptr;
#ifdef CK_ENCRYPTION
    Session_Key skey;
#endif
    static cstr * B = NULL;             /* Holder for B */
    struct t_passwd * pass;
    cstr * resp;
    char type_check[26];

    if ((cnt -= 4) < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    data += 4;
    cnt  -= 1;
    switch(*data++) {
    case SRP_AUTH:
        /* Send parameters back to client */
        if(s_srp != NULL) {
            SRP_free(s_srp);
            s_srp = NULL;
        }
        if (B != NULL) {
            cstr_free(B);
            B = NULL;
        }
        if(!szUserNameRequested[0]) {
            if (1)
                printf("No username available\r\n");
            SendSRPAuthSB(SRP_REJECT, (void *) "No username supplied", -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
#ifdef IKSD
#ifdef CK_LOGIN
        if (inserver && ckxanon &&
             !strcmp(szUserNameRequested,"anonymous")) {
            SendSRPAuthSB(SRP_REJECT, (void *)
            "anonymous login cannot be performed with Secure Remote Password",
            -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
#endif /* CK_LOGIN */
#endif /* IKSD */
        s_srp = SRP_new(SRP_RFC2945_server_method());
        if(s_srp == NULL) {
            printf("Error initializing SRP server\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "SRP server init failed",
                          -1
                          );
            return(AUTH_FAILURE);
        }
        pass = gettpnam(szUserNameRequested);
        if(pass == NULL) {
            printf("User %s not found\r\n", szUserNameRequested);
            SendSRPAuthSB(SRP_REJECT, (void *) "Password not set", -1);
            return(AUTH_FAILURE);
        }
        if(SRP_set_username(s_srp, szUserNameRequested) != SRP_SUCCESS ||
	   SRP_set_params(s_srp, pass->tc.modulus.data,
			  pass->tc.modulus.len,
			  pass->tc.generator.data,
			  pass->tc.generator.len,
			  pass->tp.salt.data,
			  pass->tp.salt.len) != SRP_SUCCESS ||
	   SRP_set_authenticator(s_srp,
				 pass->tp.password.data,
				 pass->tp.password.len) != SRP_SUCCESS) {
            printf("Error initializing SRP parameters\r\n");
            SendSRPAuthSB(SRP_REJECT,(void *)"SRP parameter init failed", -1);
            return(AUTH_FAILURE);
        }

	pbuf = (char *)malloc(pass->tc.modulus.len + pass->tc.generator.len +
			       pass->tp.salt.len + 7);
        ptr = pbuf;

        srp_encode_length(ptr, pass->tc.modulus.len);
        ptr += 2;
        memcpy(ptr, pass->tc.modulus.data, pass->tc.modulus.len);
        ptr += pass->tc.modulus.len;

        srp_encode_length(ptr, pass->tc.generator.len);
        ptr += 2;
        memcpy(ptr, pass->tc.generator.data, pass->tc.generator.len);
        ptr += pass->tc.generator.len;

        srp_encode_length(ptr, pass->tp.salt.len);
        ptr += 2;
        memcpy(ptr, pass->tp.salt.data, pass->tp.salt.len);
        ptr += pass->tp.salt.len;

        SendSRPAuthSB(SRP_PARAMS, pbuf, ptr - pbuf);
	free(pbuf);
	pbuf = NULL;

        if(SRP_gen_pub(s_srp, &B) != SRP_SUCCESS) {
            printf("Error generating SRP public value\r\n");
            SendSRPAuthSB(SRP_REJECT, (void *) "SRP_gen_pub failed", -1);
            return(AUTH_FAILURE);
        }
        ckstrncpy(szUserNameAuthenticated,szUserNameRequested,UIDBUFLEN);
        return AUTH_SUCCESS;

    case SRP_EXP:
      /* Client is sending A to us, compute challenge and expected response. */
        if (s_srp == NULL || B == NULL) {
	    printf("Protocol error: SRP_EXP unexpected\r\n");
	    SendSRPAuthSB(SRP_REJECT,
		  	(void *)"Protocol error: unexpected EXP", -1);
	    return(AUTH_FAILURE);
	}
        /* Wait until now to send B, since it contains the key to "u" */
        SendSRPAuthSB(SRP_CHALLENGE, B->data, B->length);
        cstr_free(B);
        B = NULL;

        /*
         * The original SRP AUTH implementation did not protect against
         * tampering of the auth-type-pairs.  Therefore, when the
         * AUTH_ENCRYPT_MASK bits are zero, no extra data is inserted
         * into the SRP hash computation.  When AUTH_ENCRYPT_START_TLS
         * is set we also insert the SSL/TLS client and server finished
         * messages to ensure that there is no man in the middle attack
         * underway on the SSL/TLS connection.
         */
        if ( (how & AUTH_ENCRYPT_MASK) != AUTH_ENCRYPT_OFF ) {
            type_check[0] = AUTHTYPE_SRP;
            type_check[1] = how;
#ifdef CK_SSL
            if ((how & AUTH_ENCRYPT_MASK) == AUTH_ENCRYPT_START_TLS) {
                ssl_get_client_finished(&type_check[2],12);
                ssl_get_server_finished(&type_check[14],12);
                SRP_add_ex_data(s_srp, type_check, 26);
            } else
#endif /* CK_SSL */
                SRP_add_ex_data(s_srp, type_check, 2);
        }

        if(SRP_compute_key(s_srp, &s_key, data, cnt) != SRP_SUCCESS) {
            printf("Security alert: Trivial session key attempted\r\n");
            SendSRPAuthSB(SRP_REJECT,
			  (void *) "Trivial session key detected", -1);
            return(AUTH_FAILURE);
        }
        srp_waitresp = 1;
        return AUTH_SUCCESS;

    case SRP_RESPONSE:
        /* Got the response; see if it's correct */
        if (!srp_waitresp || s_srp == NULL) {
            if (1)
              printf("Protocol error: SRP_RESPONSE unexpected\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Protocol error: unexpected RESPONSE",
                          -1
                          );
            return(AUTH_FAILURE);
        }
        srp_waitresp = 0;       /* we got a response */

        if (cnt < RESPONSE_LEN) {
            if (1)
              printf("Protocol error: malformed response\r\n");
            SendSRPAuthSB(SRP_REJECT,
                          (void *) "Protocol error: malformed response",
                          -1
                          );
            return(AUTH_FAILURE);
        }

        if(SRP_verify(s_srp, data, cnt) == SRP_SUCCESS) {
            resp = cstr_new();
            if(SRP_respond(s_srp, &resp) != SRP_SUCCESS) {
                printf("Error computing response\r\n");
                SendSRPAuthSB(SRP_REJECT,
                              (void *) "Error computing response", -1);
                return(AUTH_FAILURE);
            }
            SendSRPAuthSB(SRP_ACCEPT, resp->data, resp->length);
            accept_complete = 1;
            cstr_free(resp);

#ifdef CK_ENCRYPTION
#ifdef CK_SSL
            if (!(ssl_active_flag || tls_active_flag))
#endif /* CK_SSL */
            {
                skey.type = SK_GENERIC;
                skey.length = s_key->length;
                skey.data = s_key->data;
                encrypt_session_key(&skey, AUTH_SERVER_TO_CLIENT);
                cstr_clear_free(s_key);
                s_key = NULL;
            }
#endif /* CK_ENCRYPTION */
            auth_finished(AUTH_VALID);
        }
        else {
            SendSRPAuthSB(SRP_REJECT, (void *) "Login incorrect", -1);
            auth_finished(AUTH_REJECT);
            return(AUTH_FAILURE);
        }
        return AUTH_SUCCESS;

    default:
        printf("Unknown SRP option %d\r\n", data[-1]);
        SendSRPAuthSB(SRP_REJECT, (void *) "Unknown option received", -1);
        return(AUTH_FAILURE);
    }
}
#endif /* PRE_SRP_1_7_3 */
#endif /* SRP */

#ifdef KRB5
#ifdef KINIT
/*
 * clients/kinit/kinit.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 *
 * Initialize a credentials cache.
 */

#define KRB5_DEFAULT_OPTIONS 0
#define KRB5_DEFAULT_LIFE 60*60*10 /* 10 hours */

static krb5_data tgtname = {
#ifndef HEIMDAL
    0,
#endif /* HEIMDAL */
    KRB5_TGS_NAME_SIZE,
    KRB5_TGS_NAME
};

/* Internal prototypes */
_PROTOTYP(static krb5_error_code krb5_validate_tgt,
        (krb5_context, krb5_ccache,krb5_principal, krb5_data *));
_PROTOTYP(static krb5_error_code krb5_renew_tgt,
        (krb5_context, krb5_ccache,
                        krb5_principal, krb5_data *));
_PROTOTYP(static krb5_error_code krb5_tgt_gen,
        (krb5_context, krb5_ccache,
                        krb5_principal, krb5_data *, int opt));

#ifdef KRB5_HAVE_GET_INIT_CREDS
static krb5_error_code KRB5_CALLCONV
ck_krb5_prompter( krb5_context context,
                  void *data,
                  const char *name,
                  const char *banner,
                  int num_prompts,
                  krb5_prompt prompts[])
{
    krb5_error_code     errcode = 0;
    int                 i;
#ifdef KUI
    struct txtbox * tb = NULL;
#else /* KUI */
    char * prompt = NULL;
#endif /* KUI */
    int    len = 0, blen=0, nlen=0;

    debug(F110,"ck_krb5_prompter name",name,0);
    debug(F110,"ck_krb5_prompter banner",banner,0);
    debug(F101,"ck_krb5_prompter num_prompts","",num_prompts);

    if (name)
        nlen = strlen(name)+2;

    if (banner)
        blen = strlen(banner)+2;

#ifdef KUI
    tb = (struct txtbox *) malloc(sizeof(struct txtbox) * num_prompts);
    if ( tb != NULL ) {
        int ok;
        memset(tb,0,sizeof(struct txtbox) * num_prompts);
        for ( i=0; i < num_prompts; i++ ) {
            tb[i].t_buf = prompts[i].reply->data;
            tb[i].t_len = prompts[i].reply->length;
            tb[i].t_lbl = prompts[i].prompt;
            tb[i].t_dflt = NULL;
            tb[i].t_echo = (prompts[i].hidden ? 2 : 1);
        }   

        ok = uq_mtxt((char *)banner,NULL,num_prompts,tb);
        if ( ok ) {
            for ( i=0; i < num_prompts; i++ )
                prompts[i].reply->length = strlen(prompts[i].reply->data);
        } else
            errcode = -2;
    }
#else /* KUI */
    for (i = 0; i < num_prompts; i++) {
        debug(F111,"ck_krb5_prompter prompt",prompts[i].prompt,i);

        if ( prompt && len < (nlen + blen + strlen(prompts[i].prompt)+2) ) {
            free(prompt);
            prompt = NULL;
        }
        if ( !prompt )
            prompt = (char *)malloc(nlen + blen + strlen(prompts[i].prompt)+2);
        if ( !prompt ) {
            errcode = KRB5_RC_MALLOC;
            goto cleanup;
        }
        len = nlen + blen + strlen(prompts[i].prompt)+2;
        ckmakxmsg(prompt,len,
                 (char *) (name?name:""),
                 name?"\r\n":"",
                 (char *) (banner?banner:""),
                 banner?"\r\n":"",
                 (char *)prompts[i].prompt,
                 ": ",NULL,NULL,NULL,NULL,NULL,NULL);

        memset(prompts[i].reply->data, 0, prompts[i].reply->length);
        if (prompts[i].hidden) {
            readpass(prompt, prompts[i].reply->data,
                      prompts[i].reply->length);
        } else {
            readtext(prompt, prompts[i].reply->data,
                      prompts[i].reply->length);
        }
        prompts[i].reply->length = strlen(prompts[i].reply->data);
    }
#endif /* KUI */

  cleanup:
#ifdef KUI
    if ( tb )
        free(tb);
#else /* KUI */
    if ( prompt )
        free(prompt);
#endif /* KUI */
    if (errcode) {
        for (i = 0; i < num_prompts; i++) {
            memset(prompts[i].reply->data, 0, prompts[i].reply->length);
        }
    }
    return errcode;
}

/*
 *      I'm not really sure what to do with this.  The NRL DLLs use a
 *      different interface for the krb5_prompter callback.  It has
 *      one less parameter.   This is going to be ugly.
 */
static krb5_error_code KRB5_CALLCONV
ck_NRL_krb5_prompter( krb5_context context,
                      const char *name,
                      const char *banner,
                      int num_prompts,
                      krb5_prompt prompts[])
{
    return(ck_krb5_prompter(context,NULL,name,banner,num_prompts,prompts));
}
#endif /* KRB5_HAVE_GET_INIT_CREDS */

#ifdef KRB524_CONV
long
try_convert524(krb5_context ctx, krb5_principal me, krb5_ccache cc)
{
    char * progname = "convert524";
    krb5_error_code code = 0;
    int icode = 0;
    krb5_principal kpcserver = 0;
    krb5_creds *v5creds = 0;
    krb5_creds increds;
#ifdef OS2
    LEASH_CREDENTIALS v4creds;
#else /* OS2 */
    CREDENTIALS v4creds;
#endif /* OS2 */

    memset((char *) &increds, 0, sizeof(increds));
    /*
      From this point on, we can goto cleanup because increds is
      initialized.
    */

    if ((code = krb5_build_principal(ctx,
                                     &kpcserver,
                                     krb5_princ_realm(ctx, me)->length,
                                     krb5_princ_realm(ctx, me)->data,
                                     "krbtgt",
                                     krb5_princ_realm(ctx, me)->data,
                                     NULL))) {
        com_err(progname, code,
                "while creating service principal name");
        goto cleanup;
    }

    memset((char*) &increds, 0, sizeof(increds));
    increds.client = me;
    increds.server = kpcserver;
    /* Prevent duplicate free calls.  */
    kpcserver = 0;

    increds.times.endtime = 0;
    increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
    if ((code = krb5_get_credentials(ctx, 0,
                                     cc,
                                     &increds,
                                     &v5creds))) {
        com_err(progname, code,
                "getting V5 credentials");
        goto cleanup;
    }
    if ((icode = krb524_convert_creds_kdc(ctx,
                                          v5creds,
                                          &v4creds))) {
        com_err(progname, icode,
                "converting to V4 credentials");
        goto cleanup;
    }
    /* this is stolen from the v4 kinit */
    /* initialize ticket cache */
    if ((icode = krb_in_tkt(v4creds.pname, v4creds.pinst, v4creds.realm)
         != KSUCCESS)) {
        com_err(progname, icode,
                "trying to create the V4 ticket file");
        goto cleanup;
    }
    /* stash ticket, session key, etc. for future use */
    if ((icode = krb_save_credentials(v4creds.service,
                                      v4creds.instance,
                                      v4creds.realm,
                                      v4creds.session,
                                      v4creds.lifetime,
                                      v4creds.kvno,
                                      &(v4creds.ticket_st),
                                      v4creds.issue_date))) {
        com_err(progname, icode,
                "trying to save the V4 ticket");
        goto cleanup;
    }

 cleanup:
    memset(&v4creds, 0, sizeof(v4creds));
    if (v5creds)
        krb5_free_creds(ctx, v5creds);
    increds.client = 0;
    krb5_free_cred_contents(ctx, &increds);
    if (kpcserver)
        krb5_free_principal(ctx, kpcserver);
    return !(code || icode);
}
#endif /* KRB524_CONV */

#define NO_KEYTAB

int
#ifdef CK_ANSIC
ck_krb5_initTGT( struct krb_op_data * op, struct krb5_init_data * init,
                 struct krb4_init_data * k4_init)
#else
ck_krb5_initTGT(op,init,k4_init)
    krb_op_data * op; struct krb5_init_data * init;
    struct krb4_init_data * k4_init;
#endif /* CK_ANSIC*/
{
    krb5_context kcontext;
    krb5_ccache ccache = NULL;
    krb5_deltat lifetime = KRB5_DEFAULT_LIFE;   /* -l option */
    krb5_timestamp starttime = 0;
    krb5_deltat rlife = 0;
    int options = KRB5_DEFAULT_OPTIONS;
    int option;
    int errflg = 0;
    krb5_error_code code;
    krb5_principal me=NULL;
    krb5_principal server=NULL;
    krb5_creds my_creds;
    krb5_timestamp now;
#ifndef HEIMDAL
    krb5_address **addrs = (krb5_address **)0;
#endif /* HEIMDAL */
    int addr_count=0;
    int i,j;
#ifndef NO_KEYTAB
    int use_keytab = 0;                 /* -k option */
    krb5_keytab keytab = NULL;
#endif /* NO_KEYTAB */
    struct passwd *pw = 0;
    int pwsize;
    char *client_name=NULL, principal[256]="", realm[256]="", numstr[40]="";
    char *password=NULL, passwd[80]="";
#ifdef KRB5_HAVE_GET_INIT_CREDS
    krb5_get_init_creds_opt opts;
#endif
    char * name;
    int len;

    if ( !ck_krb5_is_installed() )
        return(-1);

#ifdef COMMENT
    printf("Kerberos V initialization\r\n");
#endif /* COMMENT */

    code = krb5_init_context(&kcontext);
    if (code) {
        com_err("krb5_kinit",code,"while init_context");
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    debug(F110,"krb5_init","krb5_init_context",0);

    if ((code = krb5_timeofday(kcontext, &now))) {
        com_err("krb5_kinit",code,"while getting time of day");
        goto exit_k5_init;
    }

#ifdef KRB5_HAVE_GET_INIT_CREDS
    memset(&opts, 0, sizeof(opts));
    krb5_get_init_creds_opt_init(&opts);
    debug(F110,"krb5_init","krb5_get_init_creds_opt_init",0);
#endif

    if ( init->renewable ) {
        options |= KDC_OPT_RENEWABLE;
        ckmakmsg(numstr,sizeof(numstr),ckitoa(init->renewable),"m",NULL,NULL);
#ifdef HEIMDAL
        code = -1;
#else /* HEIMDAL */
        code = krb5_string_to_deltat(numstr, &rlife);
#endif /* HEIMDAL */
        if (code != 0 || rlife == 0) {
            printf("Bad renewable time value %s\r\n", numstr);
            errflg++;
        }
#ifdef KRB5_HAVE_GET_INIT_CREDS
        krb5_get_init_creds_opt_set_renew_life(&opts, rlife);
#endif
    }
    if ( init->renew ) {
        /* renew the ticket */
        options |= KDC_OPT_RENEW;
    }

    if ( init->validate ) {
        /* validate the ticket */
        options |= KDC_OPT_VALIDATE;
    }
    if ( init->proxiable ) {
        options |= KDC_OPT_PROXIABLE;
#ifdef KRB5_HAVE_GET_INIT_CREDS
        krb5_get_init_creds_opt_set_proxiable(&opts, 1);
#endif
    }
    if ( init->forwardable ) {
        options |= KDC_OPT_FORWARDABLE;
#ifdef KRB5_HAVE_GET_INIT_CREDS
        krb5_get_init_creds_opt_set_forwardable(&opts, 1);
#endif
    }
#ifndef NO_KEYTAB
    if (  ) {
        use_keytab = 1;
    }
    if (  ) {
        if (keytab == NULL && keytab_name != NULL) {
            code = krb5_kt_resolve(kcontext, keytab_name, &keytab);
            if (code != 0) {
                debug(F111,"krb5_init resolving keytab",
                         keytab_name,code);
                errflg++;
            }
        }
    }
#endif /* NO_KEYTAB */
    if ( init->lifetime ) {
        ckmakmsg(numstr,sizeof(numstr),ckitoa(init->lifetime),"m",NULL,NULL);
#ifdef HEIMDAL
        code = -1;
#else /* HEIMDAL */
        code = krb5_string_to_deltat(numstr, &lifetime);
#endif /* HEIMDAL */
        if (code != 0 || lifetime == 0) {
            printf("Bad lifetime value %s\r\n", numstr);
            errflg++;
        }
#ifdef KRB5_HAVE_GET_INIT_CREDS
        krb5_get_init_creds_opt_set_tkt_life(&opts, lifetime);
#endif
    }
    if ( init->postdate ) {
        /* Convert cmdate() to a time_t value */
        struct tm * time_tm;
        struct tm * cmdate2tm(char *,int);
        time_tm = cmdate2tm(init->postdate,0);
        if ( time_tm )
            starttime = (krb5_timestamp) mktime(time_tm);

        if (code != 0 || starttime == 0 || starttime == -1) {
            krb5_deltat ktmp;
#ifdef HEIMDAL
            code = -1;
#else /* HEIMDAL */
            code = krb5_string_to_deltat(init->postdate, &ktmp);
#endif /* HEIMDAL */
            if (code == 0 && ktmp != 0) {
                starttime = now + ktmp;
                options |= KDC_OPT_POSTDATED;
            } else {
                printf("Bad postdate start time value %s\r\n",
                        init->postdate);
                errflg++;
            }
        } else {
            options |= KDC_OPT_POSTDATED;
        }
    }

    debug(F110,"krb5_init searching for ccache",op->cache,0);

    code = k5_get_ccache(kcontext,&ccache,op->cache);
    if (code != 0) {
        com_err("krb5_kinit",code,"while getting default ccache");
        goto exit_k5_init;
    }

    /* This is our realm unless it is changed */
    ckstrncpy(realm,init->realm ? init->realm : krb5_d_realm, 256);

#ifdef BETATEST
    /* This code is going to take the realm and attempt to correct */
    /* the case.                                                   */
    {
        profile_t profile;

        code = krb5_get_profile(kcontext, &profile);
        if ( !code ) {
            const char  *names[4];
            char ** realms;
            int found = 0;

            names[0] = "realms";
            names[1] = NULL;

            code = profile_get_subsection_names(profile,names,&realms);
            if ( code == 0 ) {
                int i=0;
                while ( realms[i] ) {
                    if (ckstrcmp(realm,realms[i],-1,0) == 0) {
                        strcpy(realm,realms[i]);
                        found = 1;
                        break;
                    }
                    i++;
                }
            }

#ifdef CK_DNS_SRV
            if ( !found ) {
                char * dns_realm = NULL;

                /* We did not find the realm in the profile so let's try DNS */
                locate_txt_rr("_kerberos",realm,&dns_realm);
                if ( dns_realm &&
                     ckstrcmp(realm,dns_realm,-1,0) == 0 &&
                     ckstrcmp(realm,dns_realm,-1,1) != 0
                     ) {
                    ckstrncpy(realm,dns_realm,256);
                    free(dns_realm);
                }
            }
#endif /* CK_DNS_SRV */
        }

        if (init->realm &&
             ckstrcmp(realm,init->realm,-1,0) == 0 &&
             ckstrcmp(realm,init->realm,-1,1) != 0)
            strcpy(init->realm,realm);
        if (ckstrcmp(realm,krb5_d_realm,-1,0) == 0 &&
             ckstrcmp(realm,krb5_d_realm,-1,1) != 0)
            strcpy(krb5_d_realm,realm);
    }
#endif /* BETATEST */

    if (init->principal == NULL) {       /* No principal name specified */
#ifndef NO_KEYTAB
        if (use_keytab) {
            /* Use the default host/service name */
            code = krb5_sname_to_principal(kcontext, NULL, NULL,
                                            KRB5_NT_SRV_HST, &me);
            if (code == 0 &&
                krb5_princ_realm(kcontext, me)->length < sizeof(realm))
            {
                /* Save the realm */
                memcpy(realm,krb5_princ_realm(kcontext, me)->data,
                        krb5_princ_realm(kcontext, me)->length); /* safe */
                realm[krb5_princ_realm(kcontext, me)->length]='\0';
            } else {
                com_err("krb5_kinit",
                        code,
                        "when creating default server principal name");
                goto exit_k5_init;
            }
        } else
#endif /* NO_KEYTAB */
        {
            int len;
            char * name;

            /* Get default principal from cache if one exists */
            code = krb5_cc_get_principal(kcontext, ccache, &me);
#ifdef HEIMDAL
            name = me->realm;
            len = strlen(name);
#else /* HEIMDAL */
            len = krb5_princ_realm(kcontext, me)->length;
            name = krb5_princ_realm(kcontext, me)->data;
#endif /* HEIMDAL */
            if (code == 0 && len < sizeof(realm))
            {
                /* Save the realm */
                memcpy(realm,name,len); /* safe */
                realm[len]='\0';
            } else {
#ifdef HAVE_PWD_H
                /* Else search passwd file for client */

                pw = getpwuid((int) getuid());
                if (pw) {
                    char princ_realm[256];
                    if ( (strlen(pw->pw_name) + strlen(realm) + 1) > 255 )
                        goto exit_k5_init;

                    ckstrncpy(principal,pw->pw_name,256);
                    ckstrncpy(princ_realm,pw->pw_name,256);
                    ckstrncat(princ_realm,"@",256);
                    ckstrncat(princ_realm,realm,256);

                    if ((code = krb5_parse_name(kcontext,princ_realm,&me))) {
                        krb5_errno = code;
                        com_err("krb5_kinit",code,"when parsing name",
                                  princ_realm);
                        goto exit_k5_init;
                    }
                } else {
                    printf(
                        "Unable to identify user from password file\r\n");
                    goto exit_k5_init;
                }
#else /* HAVE_PWD_H */
                printf("Unable to identify user\r\n");
                goto exit_k5_init;
#endif /* HAVE_PWD_H */
            }
        }

#ifdef HEIMDAL
        len = me->name.name_string.len;
        name = *me->name.name_string.val;
#else /* HEIMDAL */
        len = krb5_princ_name(kcontext, me)->length;
        name = krb5_princ_name(kcontext, me)->data;
#endif /* HEIMDAL */
        if ( len < sizeof(principal) ) {
            memcpy(principal,name,len);     /* safe */
            principal[len]='\0';
        }
    } /* Use specified name */
    else {
        char princ_realm[256];
        if ( (strlen(init->principal) +
              (init->instance ? strlen(init->instance)+1 : 0) +
              strlen(realm)
              + 2) > 255 )
             goto exit_k5_init;

        ckstrncpy(principal,init->principal,256);
        ckstrncpy(princ_realm,init->principal,256);
        if (init->instance) {
            ckstrncat(princ_realm,"/",256);
            ckstrncat(princ_realm,init->instance,256);
        }
        if (realm[0]) {
          ckstrncat(princ_realm,"@",256);
          ckstrncat(princ_realm,realm,256);
        }
        if ((code = krb5_parse_name (kcontext, princ_realm, &me))) {
            com_err("krb5_kinit",code,"when parsing name",princ_realm);
            goto exit_k5_init;
        }
    }

    if ((code = krb5_unparse_name(kcontext, me, &client_name))) {
        com_err("krb5_kinit",code,"when unparsing name");
        goto exit_k5_init;
    }
    debug(F110,"krb5_init client_name",client_name,0);


    memset((char *)&my_creds, 0, sizeof(my_creds));
    my_creds.client = me;

    if (init->service == NULL) {
        if ((code =
             krb5_build_principal_ext(kcontext,
                                      &server,
                                      strlen(realm),realm,
                                      tgtname.length, tgtname.data,
                                      strlen(realm),realm,
                                      0))) {
            com_err("krb5_kinit",code,"while building server name");
            goto exit_k5_init;
        }
    } else {
        if (code = krb5_parse_name(kcontext, init->service, &server)) {
            com_err("krb5_kinit",code,"while parsing service name",
                    init->service);
            goto exit_k5_init;
        }
    }

    my_creds.server = server;

    if (options & KDC_OPT_POSTDATED) {
        my_creds.times.starttime = starttime;
        my_creds.times.endtime = starttime + lifetime;
    } else {
        my_creds.times.starttime = 0;   /* start timer when request
                                           gets to KDC */
        my_creds.times.endtime = now + lifetime;
    }
    if (options & KDC_OPT_RENEWABLE) {
        my_creds.times.renew_till = now + rlife;
    } else
        my_creds.times.renew_till = 0;

    if (options & KDC_OPT_VALIDATE) {
        krb5_data outbuf;

#ifdef KRB5_HAVE_GET_INIT_CREDS
        code = krb5_get_validated_creds(kcontext,
                                        &my_creds, me, ccache, init->service);
        if ( code == -1 )
#endif
        {
#ifdef HEIMDAL
            printf("?validate not implemented\r\n");
            code = -1;
            goto exit_k5_init;
#else /* HEIMDAL */
            code = krb5_validate_tgt(kcontext, ccache, server, &outbuf);
#endif /* HEIMDAL */
        }
        if (code) {
            com_err("krb5_kinit",code,"validating tgt");
            goto exit_k5_init;
        }
        /* should be done... */
        goto exit_k5_init;
    }

    if (options & KDC_OPT_RENEW) {
        krb5_data outbuf;

#ifdef KRB5_HAVE_GET_INIT_CREDS
        code = krb5_get_renewed_creds(kcontext,
                                      &my_creds, me, ccache, init->service);
        if ( code == -1 )
#endif
        {
#ifdef HEIMDAL
            printf("?renew not implemented\r\n");
            code = -1;
            goto exit_k5_init;
#else /* HEIMDAL */
            code = krb5_renew_tgt(kcontext, ccache, server, &outbuf);
#endif /* HEIMDAL */
        }
        if (code) {
            com_err("krb5_kinit",code,"while renewing tgt");
            goto exit_k5_init;
        }
        /* should be done... */
        goto store_cred;
    }

#ifndef HEIMDAL
    if ( init->addrs && !init->no_addresses ) {
        /* construct an array of krb5_address structs to pass to get_in_tkt */
        /* include both the local ip addresses as well as any other that    */
        /* are specified.                                                   */
        unsigned long ipaddr;

        for ( addr_count=0;addr_count<KRB5_NUM_OF_ADDRS;addr_count++ )
            if ( init->addrs[addr_count] == NULL )
                break;

        if (addr_count > 0) {
            krb5_address ** local_addrs=NULL;
            krb5_os_localaddr(kcontext, &local_addrs);
            i = 0;
            while ( local_addrs[i] )
                i++;
            addr_count += i;

            addrs = (krb5_address **)
              malloc((addr_count+1) * sizeof(krb5_address *));
            if ( !addrs ) {
                krb5_free_addresses(kcontext, local_addrs);
                goto exit_k5_init;
            }
            memset(addrs, 0, sizeof(krb5_address *) * (addr_count+1));
            i = 0;
            while ( local_addrs[i] ) {
                addrs[i] = (krb5_address *)malloc(sizeof(krb5_address));
                if (addrs[i] == NULL) {
                    krb5_free_addresses(kcontext, local_addrs);
                    goto exit_k5_init;
                }

                addrs[i]->magic = local_addrs[i]->magic;
                addrs[i]->addrtype = local_addrs[i]->addrtype;
                addrs[i]->length = local_addrs[i]->length;
                addrs[i]->contents = (unsigned char *)malloc(addrs[i]->length);
                if (!addrs[i]->contents) {
                    krb5_free_addresses(kcontext, local_addrs);
                    goto exit_k5_init;
                }

                memcpy(addrs[i]->contents,local_addrs[i]->contents,
                        local_addrs[i]->length);        /* safe */
                i++;
            }
            krb5_free_addresses(kcontext, local_addrs);

            for ( j=0;i<addr_count;i++,j++ ) {
                addrs[i] = (krb5_address *)malloc(sizeof(krb5_address));
                if (addrs[i] == NULL)
                    goto exit_k5_init;

                addrs[i]->magic = KV5M_ADDRESS;
                addrs[i]->addrtype = AF_INET;
                addrs[i]->length = 4;
                addrs[i]->contents = (unsigned char *)malloc(addrs[i]->length);
                if (!addrs[i]->contents)
                    goto exit_k5_init;

                ipaddr = inet_addr(init->addrs[j]);
                memcpy(addrs[i]->contents,&ipaddr,4);   /* safe */
            }
#ifdef KRB5_HAVE_GET_INIT_CREDS
            krb5_get_init_creds_opt_set_address_list(&opts,addrs);
#endif
        }
     }
#endif /* !HEIMDAL */
#ifdef KRB5_HAVE_GET_INIT_CREDS
    if ( init->no_addresses )
        krb5_get_init_creds_opt_set_address_list(&opts,NULL);
#endif

#ifndef NO_KEYTAB
    if (!use_keytab)
#endif
    {
        if ( init->password ) {
            pwsize = strlen(init->password);
            if ( pwsize )
                password = init->password;
        } else if (init->getk4 && k4_init) {
            /* When we are requesting that K4 tickets be automatically */
            /* acquired when K5 tickets are acquired, we must get the  */
            /* password up front.                                      */
            char prmpt[256];
            extern char * k5prprompt;
            extern char * k5pwprompt;
            int ok = 0;

            if ( k5pwprompt && k5pwprompt[0] &&
                 (strlen(k5pwprompt) + strlen(principal) +
                  strlen(realm) - 4) < sizeof(prmpt)) {
                sprintf(prmpt,k5pwprompt,principal,realm);
            } else
                ckmakxmsg(prmpt,sizeof(prmpt),
                           k5pwprompt && k5pwprompt[0] ? k5pwprompt :
                           "Kerberos 5 Password for ",
                           principal,"@",realm,": ",
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL
                           );
            ok = uq_txt(NULL,prmpt,2,NULL,passwd,80,NULL,DEFAULT_UQ_TIMEOUT);
            if ( ok )
                password = passwd;

            if ( k4_init->password == NULL )
                makestr(&k4_init->password,passwd);
        }
#ifdef KRB5_HAVE_GET_INIT_CREDS
        debug(F100,"krb5_init calling krb5_get_init_creds_password()","",0);
#ifdef OS2
        if ( is_NRL_KRB5() )
            code = krb5_get_init_creds_password(kcontext, &my_creds, me,
                                                 password,
                                                 (void *)ck_NRL_krb5_prompter,
                                                 NULL,
                                                 starttime, init->service,
                                                 &opts);
        else
#endif /* OS2 */
            code = krb5_get_init_creds_password(kcontext, &my_creds, me,
                                                 password,
                                                 ck_krb5_prompter,
                                                 NULL,
                                                 starttime, init->service,
                                                 &opts);
        debug(F111,"krb5_init","krb5_get_init_creds_password()",code);

        if ( code == -1 )
        {
            if (!password) {
                char prmpt[256];
                int ok;

                ckmakmsg(prmpt,sizeof(prmpt),"Kerberos 5 Password for ",
                          client_name,": ",NULL);
                ok = uq_txt(NULL,prmpt,2,NULL,passwd,80,NULL,
			    DEFAULT_UQ_TIMEOUT);
                if ( ok )
                    password = passwd;
                else {
                    code = -2;
                    goto exit_k5_init;
                }
            }

            if ( !password ) 
                password = "";
            code = krb5_get_in_tkt_with_password(kcontext, options,
#ifdef HEIMDAL
                                                  NULL,
#else /* HEIMDAL */
                                            init->no_addresses ? NULL :addrs,
#endif /* HEIMDAL */
                                                  NULL, NULL,
                                                  password,
                                                  NULL, &my_creds, NULL);
            if ( code )
                debug(F111,"krb5_init","krb5_get_in_tkt_with_password()",code);
        }
#else /* KRB5_HAVE_GET_INIT_CREDS */
        if (!password) {
            char prmpt[256];
            int ok;

            ckmakmsg(prmpt,sizeof(prmpt),"Kerberos 5 Password for ",
                      client_name,": ",NULL);
            ok = uq_txt(NULL,prmpt,2,NULL,passwd,80,NULL,DEFAULT_UQ_TIMEOUT);
            if ( ok )
                password = passwd;
            else {
                code = -2;
                goto exit_k5_init;
            }
        }
        if ( !password ) 
            password = "";
        code = krb5_get_in_tkt_with_password(kcontext, options,
#ifdef HEIMDAL
                                              NULL,
#else /* HEIMDAL */
                                            init->no_addresses ? NULL :addrs,
#endif /* HEIMDAL */
                                              NULL, NULL,
                                              password,
                                              NULL, &my_creds, NULL);
        if ( code )
            debug(F111,"krb5_init","krb5_get_in_tkt_with_password()",code);
#endif /* KRB5_HAVE_GET_INIT_CREDS */

        if ( init->password && pwsize > 0 )
            memset(init->password, 0, pwsize);
        memset(passwd,0,80);
    }
#ifndef NO_KEYTAB
    else {
#ifdef KRB5_HAVE_GET_INIT_CREDS
        code = krb5_get_init_creds_keytab(kcontext, &my_creds, me, keytab,
                                           starttime, init->service,
                                           &opts);
#ifdef OS2
        if ( code == -1)
            code = krb5_get_in_tkt_with_keytab(kcontext, options,
                                           init->no_addresses ? NULL :addrs,
                                                NULL, NULL, keytab, NULL,
                                                &my_creds, 0);
#endif /* OS2 */
#else /* KRB5_HAVE_GET_INIT_CREDS */
        code = krb5_get_in_tkt_with_keytab(kcontext, options,
#ifdef HEIMDAL
                                                  NULL,
#else /* HEIMDAL */
                                            init->no_addresses ? NULL :addrs,
#endif /* HEIMDAL */
                                            NULL, NULL, keytab, NULL,
                                            &my_creds, 0);
#endif /* KRB5_HAVE_GET_INIT_CREDS */
    }
#endif

    if (code) {
        switch (code) {
        case KRB5KRB_AP_ERR_BAD_INTEGRITY:
            printf("Password incorrect\r\n");
            goto exit_k5_init;
        case KRB5KRB_AP_ERR_V4_REPLY:
            if (init->getk4 && k4_init) {
                printf("Kerberos 5 Tickets not support by server.  ");
                printf("A version 4 Ticket will be requested.\r\n");
            }
            goto exit_k5_init;
        default:
            goto exit_k5_init;
        }
    }

  store_cred:
    debug(F100,"krb5_init calling krb5_cc_initialize()","",0);

    code = krb5_cc_initialize (kcontext, ccache, me);
    if ( code == KRB5_CC_BADNAME ) {
        /* This is a really ugly hack that should not have to be here.
         * krb5_cc_initialize should not fail with an error if the
         * cache already exists.  The reason the problem is occuring
         * is that the krb5 library is no longer calling cc_destroy()
         * when cc_initialize() is called and the CCAPI implementation
         * on Windows has not yet been corrected to handle it.  To
         * ensure that K95 will continue to work with both we will call
         * cc_destroy() if the cc_initialize() call fails with a BADNAME
         * error.  If the cc_destroy() is successful, we will try again.
         */

        debug(F100,"krb5_init calling krb5_cc_destroy()","",0);
        code = krb5_cc_destroy (kcontext, ccache);
        if ( !code ) {
            debug(F100,"krb5_init calling k5_get_ccache()","",0);
            code = k5_get_ccache(kcontext,&ccache,op->cache);
            debug(F100,"krb5_init calling krb5_cc_initialize()","",0);
            code = krb5_cc_initialize (kcontext, ccache, me);
        } else
            code = KRB5_CC_BADNAME;
    }
    if (code) {
        com_err("krb5_kinit",code,"when initializing cache",op->cache);
        goto exit_k5_init;
    }

    debug(F100,"krb5_init calling krb5_cc_store_cred()","",0);
    code = krb5_cc_store_cred(kcontext, ccache, &my_creds);
    if (code) {
        com_err("krb5_kinit",code,"while storing credentials");
        goto exit_k5_init;
    }

    if ( init->getk4 && 
#ifdef KRB524_CONV
         !try_convert524(kcontext,me,ccache) && 
#endif /* KRB524_CONV */
         k4_init ) {
        int k4rc = ck_krb4_initTGT(op,k4_init);
        if (k4rc < 0)
            code = -3;
    }

exit_k5_init:
    debug(F100,"krb5_init exit_k5_init","",0);

#ifndef HEIMDAL
    /* Free krb5_address structures if we created them */
    if ( addrs ) {
        for ( i=0;i<addr_count;i++ ) {
            if ( addrs[i] ) {
                if ( addrs[i]->contents )
                    free(addrs[i]->contents);
                free(addrs[i]);
            }
        }
    }
#endif /* HEIMDAL */


    krb5_errno = code;
    makestr(&krb5_errmsg,krb5_errno ? error_message(krb5_errno) : "OK");

    if (client_name)
        krb5_free_unparsed_name(kcontext, client_name);

    /* my_creds is pointing at server */
    debug(F100,"krb5_init calling krb5_free_principal()","",0);
    krb5_free_principal(kcontext, server);
    debug(F100,"krb5_init calling krb5_cc_close()","",0);
    krb5_cc_close(kcontext,ccache);
    debug(F100,"krb5_init calling krb5_free_context()","",0);
    krb5_free_context(kcontext);

    if (code != -2)
        printf("Result from realm %s: %s\r\n",realm,
                code==-3?"Unable to retrieve Kerberos IV credentials":
                code?error_message(code):"OK");
    return(code?-1:0);
}

#ifndef HEIMDAL
#define VALIDATE 0
#define RENEW 1

/* stripped down version of krb5_mk_req */
static krb5_error_code
#ifdef CK_ANSIC
krb5_validate_tgt( krb5_context context,
                   krb5_ccache ccache,
                   krb5_principal     server, /* tgtname */
                   krb5_data *outbuf )
#else
krb5_validate_tgt(context, ccache, server, outbuf)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal     server; /* tgtname */
     krb5_data *outbuf;
#endif
{
    return krb5_tgt_gen(context, ccache, server, outbuf, VALIDATE);
}

/* stripped down version of krb5_mk_req */
static krb5_error_code
#ifdef CK_ANSIC
krb5_renew_tgt(krb5_context context,
                krb5_ccache ccache,
                krb5_principal    server, /* tgtname */
                krb5_data *outbuf)
#else
krb5_renew_tgt(context, ccache, server, outbuf)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal       server; /* tgtname */
     krb5_data *outbuf;
#endif
{
    return krb5_tgt_gen(context, ccache, server, outbuf, RENEW);
}


/* stripped down version of krb5_mk_req */
static krb5_error_code
#ifdef CK_ANSIC
krb5_tgt_gen(krb5_context context,
              krb5_ccache ccache,
              krb5_principal      server, /* tgtname */
              krb5_data *outbuf,
              int opt)
#else
krb5_tgt_gen(context, ccache, server, outbuf, opt)
     krb5_context context;
     krb5_ccache ccache;
     krb5_principal       server; /* tgtname */
     krb5_data *outbuf;
     int opt;
#endif
{
    krb5_error_code       retval;
    krb5_creds          * credsp;
    krb5_creds            creds;

    /* obtain ticket & session key */
    memset((char *)&creds, 0, sizeof(creds));
    if ((retval = krb5_copy_principal(context, server, &creds.server)))
        goto cleanup;

    if ((retval = krb5_cc_get_principal(context, ccache, &creds.client)))
        goto cleanup_creds;

    if (opt == VALIDATE) {
        if ((retval = krb5_get_credentials_validate(context, 0,
                                                    ccache, &creds, &credsp)))
          goto cleanup_creds;
    } else {
        if ((retval = krb5_get_credentials_renew(context, 0,
                                                 ccache, &creds, &credsp)))
          goto cleanup_creds;
    }

    /* we don't actually need to do the mk_req, just get the creds. */
cleanup_creds:
    krb5_free_cred_contents(context, &creds);

cleanup:

    return retval;
}
#endif /* HEIMDAL */
#endif /* KINIT */
#ifdef KDESTROY
int
#ifdef CK_ANSIC
ck_krb5_destroy(struct krb_op_data * op)
#else
ck_krb5_destroy(op) struct krb_op_data * op;
#endif
{
    krb5_context kcontext;
    krb5_error_code retval;
    int c;
    krb5_ccache ccache = NULL;
    char *cache_name = NULL;
    int code;
    int errflg=0;
    int quiet = 0;

    if ( !ck_krb5_is_installed() )
        return(-1);

    code = krb5_init_context(&kcontext);
    if (code) {
        debug(F101,"ck_krb5_destroy while initializing krb5","",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    code = k5_get_ccache(kcontext,&ccache,op->cache);
    if (code != 0) {
        debug(F101,"ck_krb5_destroy while getting ccache",
               "",code);
        krb5_free_context(kcontext);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    code = krb5_cc_destroy (kcontext, ccache);
    if (code != 0) {
        debug(F101,"ck_krb5_destroy while destroying cache","",code);
        if ( code == KRB5_FCC_NOFILE )
            printf("No ticket cache to destroy.\r\n");
        else
            printf("Ticket cache NOT destroyed!\r\n");
        krb5_cc_close(kcontext,ccache);
        krb5_free_context(kcontext);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    printf("Tickets destroyed.\r\n");
    /* Do not call krb5_cc_close() because cache has been destroyed */
    krb5_free_context(kcontext);
    krb5_errno = 0;
    makestr(&krb5_errmsg,"OK");
    return (0);
}
#else /* KDESTROY */
#ifdef KRB5
int
#ifdef CK_ANSIC
ck_krb5_destroy(struct krb_op_data * op)
#else
ck_krb5_destroy(op) struct krb_op_data * op;
#endif
{
    printf("?Not implemented.\r\n");
    return(-1);
}
#endif /* KRB5 */
#endif /* KDESTROY */
#ifndef KLIST
#ifdef KRB5
int
#ifdef CK_ANSIC
ck_krb5_list_creds(struct krb_op_data * op, struct krb5_list_cred_data * lc)
#else
ck_krb5_list_creds(op,lc)
    struct krb_op_data * op; struct krb5_list_cred_data * lc;
#endif
{
    printf("?Not implemented.\r\n");
    return(-1);
}
#endif /* KRB5 */
#else /* KLIST */
static int show_flags = 0, show_time = 0, status_only = 0, show_keys = 0;
static int show_etype = 0, show_addr = 0;
static char *defname;
static char *progname;
static krb5_int32 now;
static int timestamp_width;

_PROTOTYP(static char * etype_string, (krb5_enctype ));
_PROTOTYP(static void show_credential,(krb5_context,krb5_creds *));
_PROTOTYP(static int do_ccache, (krb5_context,char *));
_PROTOTYP(static int do_keytab, (krb5_context,char *));
_PROTOTYP(static void printtime, (time_t));
_PROTOTYP(static void fillit, (int, int));

#define DEFAULT 0
#define CCACHE 1
#define KEYTAB 2

int
#ifdef CK_ANSIC
ck_krb5_list_creds(struct krb_op_data * op, struct krb5_list_cred_data * lc)
#else
ck_krb5_list_creds(op,lc)
    struct krb_op_data * op; struct krb5_list_cred_data * lc;
#endif
{
    krb5_context kcontext;
    krb5_error_code retval;
    int code;
    char *name = op->cache;
    int mode;

    if ( !ck_krb5_is_installed() )
        return(-1);

    code = krb5_init_context(&kcontext);
    if (code) {
        debug(F101,"ck_krb5_list_creds while initializing krb5","",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    name = op->cache;
    mode = DEFAULT;
    show_flags = 0;
    show_time = 0;
    status_only = 0;
    show_keys = 0;
    show_etype = 0;
    show_addr = 0;

    show_flags = lc->flags;
    show_etype = lc->encryption;
    show_addr  = lc->addr;
    show_time = 1;
    show_keys = 1;
    mode = CCACHE;

    if ((code = krb5_timeofday(kcontext, &now))) {
        if (!status_only)
            debug(F101,"ck_krb5_list_creds while getting time of day.",
                   "",code);
        krb5_free_context(kcontext);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }
    else {
        char tmp[BUFSIZ];

        if (!krb5_timestamp_to_sfstring(now, tmp, 20, (char *) NULL) ||
            !krb5_timestamp_to_sfstring(now, tmp, sizeof(tmp), (char *) NULL))
            timestamp_width = (int) strlen(tmp);
        else
            timestamp_width = 15;
    }

    if (mode == DEFAULT || mode == CCACHE)
         retval = do_ccache(kcontext,name);
    else
         retval = do_keytab(kcontext,name);
    krb5_free_context(kcontext);
    return(retval);
}

static int
#ifdef CK_ANSIC
do_keytab(krb5_context kcontext, char * name)
#else
do_keytab(kcontext,name) krb5_context kcontext; char * name;
#endif
{
    krb5_keytab kt;
    krb5_keytab_entry entry;
    krb5_kt_cursor cursor;
    char buf[BUFSIZ]; /* hopefully large enough for any type */
    char *pname;
    int code = 0;

    if (name == NULL) {
        if ((code = krb5_kt_default(kcontext, &kt))) {
            debug(F101,"ck_krb5_list_creds while getting default keytab",
                   "",code);
            krb5_errno = code;
            makestr(&krb5_errmsg,error_message(krb5_errno));
            return(-1);
        }
    } else {
        if ((code = krb5_kt_resolve(kcontext, name, &kt))) {
            debug(F111,"ck_krb5_list_creds while resolving keytab",
                     name,code);
            krb5_errno = code;
            makestr(&krb5_errmsg,error_message(krb5_errno));
            return(-1);
        }
    }

    if ((code = krb5_kt_get_name(kcontext, kt, buf, BUFSIZ))) {
        debug(F101,"ck_krb5_list_creds while getting keytab name",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

     printf("Keytab name: %s\r\n", buf);

     if ((code = krb5_kt_start_seq_get(kcontext, kt, &cursor))) {
         debug(F101,"ck_krb5_list_creds while starting keytab scan",
                "",code);
         krb5_errno = code;
         makestr(&krb5_errmsg,error_message(krb5_errno));
         return(-1);
     }

     if (show_time) {
          printf("KVNO Timestamp");
          fillit(timestamp_width - sizeof("Timestamp") + 2, (int) ' ');
          printf("Principal\r\n");
          printf("---- ");
          fillit(timestamp_width, (int) '-');
          printf(" ");
          fillit(78 - timestamp_width - sizeof("KVNO"), (int) '-');
          printf("\r\n");
     } else {
          printf("KVNO Principal\r\n");
          printf(
"---- --------------------------------------------------------------------\
------\r\n");
     }

    while ((code = krb5_kt_next_entry(kcontext, kt, &entry, &cursor)) == 0) {
        if ((code = krb5_unparse_name(kcontext, entry.principal, &pname))) {
            debug(F101,"ck_krb5_list_creds while unparsing principal name",
                   "",code);
            krb5_errno = code;
            makestr(&krb5_errmsg,error_message(krb5_errno));
            return(-1);
        }
        printf("%4d ", entry.vno);
        if (show_time) {
            printtime(entry.timestamp);
            printf(" ");
        }
        printf("%s", pname);
        if (show_etype)
            printf(" (%s) " ,
#ifdef HEIMDAL
                    etype_string(entry.key.keytype)
#else /* HEIMDAL */
                    etype_string(entry.key.enctype)
#endif /* HEIMDAL */
                    );
        if (show_keys) {
            printf(" (0x");
            {
                int i;
                for (i = 0; i < entry.key.length; i++)
                    printf("%02x",
#ifdef HEIMDAL
                            entry.key.keyvalue[i]
#else /* HEIMDAL */
                            entry.key.contents[i]
#endif /* HEIMDAL */
                            );
            }
            printf(")");
        }
        printf("\r\n");
        krb5_free_unparsed_name(kcontext,pname);
    }
    if (code && code != KRB5_KT_END) {
        debug(F101,"ck_krb5_list_creds while scanning keytab",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }
    if ((code = krb5_kt_end_seq_get(kcontext, kt, &cursor))) {
        debug(F101,"ck_krb5_list_creds while ending keytab scan",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }
    krb5_errno = 0;
    makestr(&krb5_errmsg,"OK");
    return(0);
}

static int
#ifdef CK_ANSIC
do_ccache(krb5_context kcontext, char * cc_name)
#else
do_ccache(kcontext,name) krb5_context kcontext; char * cc_name;
#endif
{
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ=NULL;
    krb5_flags flags=0;
    krb5_error_code code = 0;
    int exit_status = 0;

    if (status_only)
        /* exit_status is set back to 0 if a valid tgt is found */
        exit_status = 1;

    code = k5_get_ccache(kcontext,&cache,cc_name);
    if (code != 0) {
        debug(F111,"do_ccache while getting ccache",
               error_message(code),code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return(-1);
    }

    flags = 0;                          /* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
        if (code == ENOENT) {
            debug(F111,"ck_krb5_list_creds (ticket cache)",
                   krb5_cc_get_name(kcontext, cache),code);
        } else {
            debug(F111,
                 "ck_krb5_list_creds while setting cache flags (ticket cache)",
                  krb5_cc_get_name(kcontext, cache),code);
        }
        printf("No ticket File.\r\n");
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        krb5_cc_close(kcontext,cache);
        return(-1);
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
        debug(F101,"ck_krb5_list_creds while retrieving principal name",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        krb5_cc_close(kcontext,cache);
        return(-1);
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
        debug(F101,"ck_krb5_list_creds while unparsing principal name",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        krb5_cc_close(kcontext,cache);
        return(-1);
    }
    if (!status_only) {
        printf("Ticket cache:      %s:%s\r\nDefault principal: %s\r\n\r\n",
                krb5_cc_get_type(kcontext, cache),
                krb5_cc_get_name(kcontext, cache), defname);
        printf("Valid starting");
        fillit(timestamp_width - sizeof("Valid starting") + 3,
               (int) ' ');
        printf("Expires");
        fillit(timestamp_width - sizeof("Expires") + 3,
               (int) ' ');
        printf("Service principal\r\n");
    }
    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
        debug(F101,"ck_krb5_list_creds while starting to retrieve tickets",
               "",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        krb5_cc_close(kcontext,cache);
        return(-1);
    }
    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
        if (status_only) {
            if (exit_status && creds.server->length == 2 &&
                strcmp(creds.server->realm.data, princ->realm.data) == 0 &&
                strcmp((char *)creds.server->data[0].data, "krbtgt") == 0 &&
                strcmp((char *)creds.server->data[1].data,
                       princ->realm.data) == 0 &&
                creds.times.endtime > now)
                exit_status = 0;
        } else {
            show_credential(kcontext, &creds);
        }
        krb5_free_cred_contents(kcontext, &creds);
    }
    printf("\r\n");
    if (code == KRB5_CC_END || code == KRB5_CC_NOTFOUND) {
        if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
            debug(F101,"ck_krb5_list_creds while finishing ticket retrieval",
                   "",code);
            krb5_errno = code;
            makestr(&krb5_errmsg,error_message(krb5_errno));
            krb5_cc_close(kcontext,cache);
            return(-1);
        }
        flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
        if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
            debug(F101,"ck_krb5_list_creds while closing ccache",
                   "",code);
            krb5_errno = code;
            makestr(&krb5_errmsg,error_message(krb5_errno));
            krb5_cc_close(kcontext,cache);
            return(-1);
        }
        krb5_errno = 0;
        makestr(&krb5_errmsg,"OK");
        krb5_cc_close(kcontext,cache);
        return(0);
    } else {
        debug(F101,"ck_krb5_list_creds while retrieving a ticket","",code);
        krb5_errno = code;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        krb5_cc_close(kcontext,cache);
        return(-1);
    }
    krb5_errno = 0;
    makestr(&krb5_errmsg,"OK");
    krb5_cc_close(kcontext,cache);
    return(0);
}

static char *
#ifdef CK_ANSIC
#ifdef HEIMDAL
etype_string(krb5_keytype enctype)
#else /* HEIMDAL */
etype_string(krb5_enctype enctype)
#endif /* HEIMDAL */
#else
#ifdef HEIMDAL
etype_string(enctype) krb5_keytype enctype;
#else /* HEIMDAL */
etype_string(enctype) krb5_enctype enctype;
#endif /* HEIMDAL */
#endif
{
    static char buf[12];

    switch (enctype) {
    case ENCTYPE_NULL:
        return "NULL";
    case ENCTYPE_DES_CBC_CRC:
        return "DES-CBC-CRC";
    case ENCTYPE_DES_CBC_MD4:
        return "DES-CBC-MD4";
    case ENCTYPE_DES_CBC_MD5:
        return "DES-CBC-MD5";
    case ENCTYPE_DES_CBC_RAW:
        return "DES-CBC-RAW";
    case ENCTYPE_DES3_CBC_SHA:
        return "DES3-CBC-SHA";
    case ENCTYPE_DES3_CBC_RAW:
        return "DES3-CBC-RAW";
    case ENCTYPE_DES_HMAC_SHA1:
        return "DES-HMAC-SHA1";
    case ENCTYPE_DES3_CBC_SHA1:
        return "DES3-CBC-SHA1";
    case ENCTYPE_AES128_CTS_HMAC_SHA1_96:
        return "AES128_CTS-HMAC-SHA1_96";
    case ENCTYPE_AES256_CTS_HMAC_SHA1_96:
        return "AES256_CTS-HMAC-SHA1_96";
    case ENCTYPE_ARCFOUR_HMAC:
        return "RC4-HMAC-NT";
    case ENCTYPE_ARCFOUR_HMAC_EXP:
        return "RC4-HMAC-NT-EXP";
    case ENCTYPE_UNKNOWN:
        return "UNKNOWN";
    case ENCTYPE_LOCAL_DES3_HMAC_SHA1:
        return "LOCAL-DES3-HMAC-SHA1";
    case ENCTYPE_LOCAL_RC4_MD4:
        return "LOCAL-RC4-MD4";
    default:
        ckmakmsg(buf, sizeof(buf),"etype ", ckitoa(enctype),NULL,NULL);
        return buf;
        break;
    }
}

static char *
#ifdef CK_ANSIC
flags_string(register krb5_creds *cred)
#else
flags_string(cred) register krb5_creds *cred;
#endif
{
    static char buf[32];
    int i = 0;

    if (cred->ticket_flags & TKT_FLG_FORWARDABLE)
        buf[i++] = 'F';
    if (cred->ticket_flags & TKT_FLG_FORWARDED)
        buf[i++] = 'f';
    if (cred->ticket_flags & TKT_FLG_PROXIABLE)
        buf[i++] = 'P';
    if (cred->ticket_flags & TKT_FLG_PROXY)
        buf[i++] = 'p';
    if (cred->ticket_flags & TKT_FLG_MAY_POSTDATE)
        buf[i++] = 'D';
    if (cred->ticket_flags & TKT_FLG_POSTDATED)
        buf[i++] = 'd';
    if (cred->ticket_flags & TKT_FLG_INVALID)
        buf[i++] = 'i';
    if (cred->ticket_flags & TKT_FLG_RENEWABLE)
        buf[i++] = 'R';
    if (cred->ticket_flags & TKT_FLG_INITIAL)
        buf[i++] = 'I';
    if (cred->ticket_flags & TKT_FLG_HW_AUTH)
        buf[i++] = 'H';
    if (cred->ticket_flags & TKT_FLG_PRE_AUTH)
        buf[i++] = 'A';
    buf[i] = '\0';
    return(buf);
}

static char   *
#ifdef CK_ANSIC
short_date(long   *dp)
#else
short_date(dp) long   *dp;
#endif
{
    register char *cp;
#ifndef ctime
    extern char *ctime();
#endif /* ctime */
    cp = ctime(dp) + 4;
    cp[15] = '\0';
    return (cp);
}


static VOID
#ifdef CK_ANSIC
printtime(time_t tv)
#else
printtime(tv) time_t tv;
#endif
{
    char timestring[BUFSIZ];
    char format[12];
    char fill;

    fill = ' ';
    sprintf(format,"%%-%ds",timestamp_width);   /* safe */
    if (!krb5_timestamp_to_sfstring((krb5_timestamp) tv,
                                     timestring,
                                     timestamp_width+1,
                                     &fill)) {
        printf(format,timestring);
    }
    else {
        printf(format,short_date(&tv));
    }

}

static void
#ifdef CK_ANSIC
one_addr(krb5_address *a)
#else
one_addr(a) krb5_address *a;
#endif
{
    struct hostent *h;
    extern tcp_rdns;

    if ((a->addrtype == ADDRTYPE_INET) &&
        (a->length == 4)) {
        if (tcp_rdns != SET_OFF) {
            h = gethostbyaddr(a->contents, 4, AF_INET);
            if (h) {
#ifdef HADDRLIST
                h = ck_copyhostent(h);
#endif /* HADDRLIST */
                printf("%s (%d.%d.%d.%d)", h->h_name,
                        a->contents[0], a->contents[1],
                        a->contents[2], a->contents[3]);
            }
        }
        if (tcp_rdns == SET_OFF || !h) {
            printf("%d.%d.%d.%d", a->contents[0], a->contents[1],
                   a->contents[2], a->contents[3]);
        }
    } else {
        printf("unknown addr type %d", a->addrtype);
    }
}

static VOID
#ifdef CK_ANSIC
show_credential(krb5_context kcontext, register krb5_creds * cred)
#else
show_credential(kcontext, cred)
    krb5_context          kcontext;
    register krb5_creds * cred;
#endif
{
    krb5_error_code retval=0;
    krb5_ticket *tkt=NULL;
    char *name=NULL, *sname=NULL, *flags=NULL;
    int extra_field = 0;

    retval = krb5_unparse_name(kcontext, cred->client, &name);
    if (retval) {
        debug(F101,"ck_krb5_list_creds while unparsing client name","",retval);
        krb5_errno = retval;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return;
    }
    retval = krb5_unparse_name(kcontext, cred->server, &sname);
    if (retval) {
        debug(F101,"ck_krb5_list_creds while unparsing server name","",retval);
        free(name);
        krb5_errno = retval;
        makestr(&krb5_errmsg,error_message(krb5_errno));
        return;
    }
    if (!cred->times.starttime)
        cred->times.starttime = cred->times.authtime;

    printtime(cred->times.starttime);
    printf("  ");

    if ( time(0) < cred->times.endtime )
        printtime(cred->times.endtime);
    else
        printf("** expired ** ");

    printf("  %s\r\n", sname);

    if (strcmp(name, defname)) {
        printf("   for client %s", name);
        extra_field++;
    }

    if (cred->times.renew_till) {
        if (!extra_field)
            printf("   ");
        else
            printf(", ");
        printf("renew until ");
        printtime(cred->times.renew_till);
        extra_field += 2;
    }

    if (extra_field > 3) {
        printf("\r\n");
        extra_field = 0;
    }

    if (show_flags) {
        flags = flags_string(cred);
        if (flags && *flags) {
            if (!extra_field)
                printf("   ");
            else
                printf(", ");
            printf("Flags: %s", flags);
            extra_field++;
        }
    }

    if (extra_field > 2) {
        printf("\r\n");
        extra_field = 0;
    }

    if (show_etype) {
        retval = decode_krb5_ticket(&cred->ticket, &tkt);
        if (!extra_field)
            printf("   ");
        else
            printf(", ");
#ifdef HEIMDAL
        printf("Etype (skey, tkt): %s, %s ",
               etype_string(cred->session.keytype),
               etype_string(tkt->enc_part.keytype));
#else /* HEIMDAL */
        printf("Etype (skey, tkt): %s, %s ",
               etype_string(cred->keyblock.enctype),
               etype_string(tkt->enc_part.enctype));
#endif /* HEIMDAL */
        krb5_free_ticket(kcontext, tkt);
        extra_field++;
    }

    /* if any additional info was printed, extra_field is non-zero */
    if (extra_field)
        printf("\r\n");

    if ( show_addr ) {
        if (!cred->addresses || !cred->addresses[0]) {
            printf("\tAddresses: (none)\r\n");
        } else {
            int i;
            for (i=0; cred->addresses[i]; i++) {
                if (i)
                    printf("              ");
                else
                    printf("   Addresses: ");
                one_addr(cred->addresses[i]);
                printf("\r\n");
            }
        }
    }

    krb5_free_unparsed_name(kcontext,name);
    krb5_free_unparsed_name(kcontext,sname);

    krb5_errno = 0;
    makestr(&krb5_errmsg,"OK");
}

static VOID
#ifdef CK_ANSIC
fillit(int num, int c)
#else
fillit(num, c) int num; int c;
#endif
{
    int i;

    for (i=0; i<num; i++)
        printf("%c",c);
}
#endif /* KLIST */
#endif /* KRB5 */

#ifdef KRB4
#define KDEBUG 1
int k4debug = 0;                /* Kerberos 4 runtime debugging */

#ifdef KINIT
#define KRB_DEFAULT_LIFE 120 /* 10 hours in 5 minute intervals */

#ifdef SNK4
/* SNK4 is a hardware authentication system used to pre-authenticate    */
/* a ticket getting ticket.  We do not support this code at the present */
/* time in Kermit.                                                      */
void
get_input(s, size, stream)
char *s;
int size;
FILE *stream;
{
    char *p;

    if (fgets(s, size, stream) == NULL)
        exit(1);
    if ( (p = strchr(s, '\n')) != NULL)
        *p = '\0';
}
#endif /* SNK4 */

#ifdef COMMENT
static char
#ifdef CK_ANSIC
hex_scan_nybble(char c)
#else
hex_scan_nybble(c) char c;
#endif
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

/* returns: NULL for ok, pointer to error string for bad input */
static char*
#ifdef CK_ANSIC
hex_scan_four_bytes(char *out, char *in)
#else
hex_scan_four_bytes(out, in) char *out; char *in;
#endif
{
    int i;
    int c;
    char c1;
    for (i=0; i<8; i++) {
        if(!in[i])
            return "not enough input";
        c = hex_scan_nybble(in[i]);
        if(c<0)
            return "invalid digit";
        c1 = c;
        i++;
        if(!in[i])
            return "not enough input";
        c = hex_scan_nybble(in[i]);
        if(c<0)
            return "invalid digit";
        *out++ = (c1 << 4) + c;
    }
    switch(in[i]) {
    case 0:
    case '\r':
    case '\n':
        return NULL;
    default:
        return "extra characters at end of input";
    }
}
#endif /* COMMENT */

/* ck_krb4_initTGT() returns 0 on success */
int
#ifdef CK_ANSIC
ck_krb4_initTGT(struct krb_op_data * op, struct krb4_init_data * init)
#else
ck_krb4_initTGT(op,init)
    struct krb_op_data * op, struct krb4_init_data * init
#endif
{
    char    aname[ANAME_SZ+1];
    char    inst[INST_SZ+1];
    char    realm[REALM_SZ+1];
    char    *password=NULL;
    char    passwd[80]="";
    char    *username = NULL;
    char    *usernameptr=NULL;
    int     iflag,      /* Instance */
            rflag,      /* Realm */
            vflag,      /* Verbose */
            lflag,      /* Lifetime */
            pflag,      /* Preauth */
            lifetime=KRB_DEFAULT_LIFE,   /* Life Time */
            k_errno;
    register char *cp;
    register i;

    if ( !ck_krb4_is_installed() )
        return(-1);

    *inst = *realm = '\0';
    iflag = rflag = vflag = lflag = pflag = 0;

    vflag = init->verbose;
    pflag = init->preauth;

    if ( init->lifetime ) {
        lifetime = init->lifetime<5?1:init->lifetime/5;
        if ( lifetime > 255 ) lifetime = 255;
    }
    else
        lifetime = KRB_DEFAULT_LIFE;

    username = init->principal;

    if (username && username[0] &&
        (k_errno = kname_parse(aname, inst, realm, username))
        != AUTH_SUCCESS) {
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        printf("%s\r\n", krb_get_err_text_entry(k_errno));
        iflag = rflag = 1;
        username = NULL;
    }

    if ( init->realm ) {
        ckstrncpy(realm,init->realm,REALM_SZ);
    }

    if ( init->instance ) {
        ckstrncpy(inst,init->instance, INST_SZ);
    }

#ifdef COMMENT
    if ( vflag )
        printf("Kerberos IV initialization\r\n");
#endif /* COMMENT */

    if (!username || !username[0]) {
        debug(F100,"ck_krb4_initTGT no username specified","",0);
        printf("?Invalid principal specified.\r\n");
        krb4_errno = -1;
        makestr(&krb4_errmsg,"No principal specified");
        return(-1);
    }
    if (!*realm) {
        ckstrncpy(realm,ck_krb4_getrealm(),REALM_SZ);
    }

    if ( init->password )
        password = init->password;
    else {
        char prmpt[80];
        int ok;

        ckmakxmsg(prmpt,sizeof(prmpt),
                  "Kerberos 4 Password for ",username,"@",realm,": ",
                   NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        ok = uq_txt(NULL,prmpt,2,NULL,passwd,80,NULL,DEFAULT_UQ_TIMEOUT);
        if ( ok )
            password = passwd;
    }

    if (pflag) {
        k_errno = krb_get_pw_in_tkt_preauth( aname, inst, realm,
                                             "krbtgt", realm,
                                             lifetime,
                                             password);
        if (k_errno == -1) {    /* preauth method not available */
            k_errno = krb_get_pw_in_tkt(aname,
                                         inst, realm,
                                         "krbtgt", realm,
                                         lifetime,
                                         password);
        }
    } else {
        k_errno = krb_get_pw_in_tkt(aname,
                                     inst, realm,
                                     "krbtgt", realm,
                                     lifetime,
                                     password);
    }

    memset(passwd,0,sizeof(passwd));
    if (k_errno) {
        printf("%s for principal %s%s%s@%s\r\n",
                krb_get_err_text_entry(k_errno), aname,
                inst[0]?".":"", inst, realm);
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        return(-1);
    } else if (vflag) {
        printf("Result from realm %s: ", realm);
        printf("%s\r\n", krb_get_err_text_entry(k_errno));
    }
    krb4_errno = k_errno;
    makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
    return(0);
}
#endif /* KINIT */
#ifdef KDESTROY
int
#ifdef CK_ANSIC
ck_krb4_destroy(struct krb_op_data * op)
#else
ck_krb4_destroy(op) struct krb_op_data * op;
#endif
{
    int k_errno=0;

    if ( !ck_krb4_is_installed() )
        return(-1);

    k_errno = dest_tkt();

    krb4_errno = k_errno;
    makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));

    if (k_errno == 0)
        printf("Tickets destroyed.\r\n");
    else if (k_errno == RET_TKFIL)
        printf("No tickets to destroy.\r\n");
    else {
        printf("Tickets MAY NOT be destroyed.\r\n");
        return(-1);
    }
    return(0);
}
#endif /* KDESTROY */
#ifdef KLIST
_PROTOTYP(static int display_tktfile,(char *, int, int, int));

int
#ifdef CK_ANSIC
ck_krb4_list_creds(struct krb_op_data * op)
#else
ck_krb4_list_creds(op) struct krb_op_data * op;
#endif
{
    int     long_form = 1;
    int     tgt_test = 0;
    int     do_srvtab = 0;
    int     show_kvnos = 0;
    char   *tkt_file = NULL;

    if ( !ck_krb4_is_installed() )
        return(-1);

    if ( op->cache )
        tkt_file = op->cache;

    if ( k4debug ) {
        show_kvnos = 1;
    }

    if (do_srvtab)
        return(display_srvtab(tkt_file));
    else
        return(display_tktfile(tkt_file, tgt_test, long_form, show_kvnos));
}

#ifndef KRB5
static int timestamp_width=0;

static char   *
#ifdef CK_ANSIC
short_date(long   *dp)
#else
short_date(dp) long   *dp;
#endif
{
    register char *cp;
    extern char *ctime();
    cp = ctime(dp) + 4;
    cp[15] = '\0';
    return (cp);
}


static VOID
#ifdef CK_ANSIC
printtime(time_t tv)
#else
printtime(tv) time_t tv;
#endif
{
    char timestring[BUFSIZ];
    char format[12];
    char fill;

    fill = ' ';
    sprintf(format,"%%-%ds",timestamp_width);   /* safe */
    printf(format,short_date(&tv));
}
#endif /* KRB5 */

static int
#ifdef CK_ANSIC
display_tktfile(char *file, int tgt_test, int long_form, int show_kvnos)
#else
display_tktfile(file,tgt_test,long_form,show_kvnos)
    char *file; int tgt_test; int long_form; int show_kvnos;
#endif
{
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf1[20], buf2[20];
    int     k_errno;
#ifdef OS2
    LEASH_CREDENTIALS creds;
#else /* OS2 */
    CREDENTIALS creds;
#endif /* OS2 */
    int     header = 1;

    file = tkt_string();

    if (long_form) {
        printf("Ticket cache:      %s\r\n", file);
    }

    /*
     * Since krb_get_tf_realm will return a ticket_file error,
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that
     * the user would see would be
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        if (!tgt_test)
            printf("%s\r\n", krb_get_err_text_entry (k_errno));
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        return(-1);
    }


    /* Close ticket file */
    (void) tf_close();

    /*
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    if ((k_errno = krb_get_tf_realm(file, prealm)) != AUTH_SUCCESS) {
        if (!tgt_test)
            printf("can't find realm of ticket file: %s\r\n",
                    krb_get_err_text_entry (k_errno));
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        return(-1);
    }

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        if (!tgt_test)
            printf("%s\r\n", krb_get_err_text_entry (k_errno));
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        return(-1);
    }
    /* Get principal name and instance */
    if ((k_errno = tf_get_pname(pname)) ||
         (k_errno = tf_get_pinst(pinst))) {
        (void) tf_close();
        if (!tgt_test)
            printf("%s\r\n", krb_get_err_text_entry (k_errno));
        krb4_errno = k_errno;
        makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
        return(-1);
    }

    /*
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why
     * it was done before tf_init.
     */

    if (!tgt_test && long_form)
        printf("Default principal: %s%s%s%s%s\r\n\r\n", pname,
               (pinst[0] ? "." : ""), pinst,
               (prealm[0] ? "@" : ""), prealm);

    while ((k_errno = tf_get_cred(&creds)) == AUTH_SUCCESS) {
        if (!tgt_test && long_form && header) {
            printf("%-17s  %-17s  %s\r\n",
                   "Valid starting", "Expires", "Service principal");
            header = 0;
        }
        if (tgt_test) {
            creds.issue_date += ((unsigned char) creds.lifetime) * 5 * 60;
            if (!strcmp(creds.service, "krbtgt") &&
                !strcmp(creds.instance, prealm)) {
                krb4_errno = k_errno;
                makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));

                (void) tf_close();
                if (time(0) < creds.issue_date) {
                    return(0);          /* tgt hasn't expired */
                } else {
                    return(-1);         /* has expired */
                }
            }
            continue;                   /* not a tgt */
        }
        if (long_form) {
            timestamp_width = 17;       /* for k5 display function */
                                        /* if available            */
            printtime(creds.issue_date);
            printf("  ");
            creds.issue_date += ((unsigned char) creds.lifetime) * 5 * 60;
            if ( time(0) < creds.issue_date )
                printtime(creds.issue_date);
            else
                printf("*** expired ***  ");
            printf("  ");
        }
        if (show_kvnos)
          printf("%s%s%s%s%s (%d)\r\n",
                 creds.service, (creds.instance[0] ? "." : ""), creds.instance,
                 (creds.realm[0] ? "@" : ""), creds.realm, creds.kvno);
        else
          printf("%s%s%s%s%s\r\n",
                 creds.service, (creds.instance[0] ? "." : ""), creds.instance,
                 (creds.realm[0] ? "@" : ""), creds.realm);

#ifdef OS2
        if ( creds.address[0] )
            printf("   Address: %s\r\n",creds.address);
#endif /* OS2 */
    }

    (void) tf_close();

    if (tgt_test) {
        return(-1);
    }/* no tgt found */
    if (header && long_form && k_errno == EOF) {
        printf("No tickets in file.\r\n");
    }
    krb4_errno = k_errno;
    makestr(&krb4_errmsg,krb_get_err_text_entry(k_errno));
    return(0);
}

#ifdef COMMENT
/* Just so we remember what the command line interface looked like */
usage()
{
    printf(
        "Usage: [ -s | -t ] [ -file filename ] [ -srvtab ] [ -version ]\r\n");
    return(-1);
}
#endif /* COMMENT */

/* adapted from getst() in librkb */
/*
 * ok_getst() takes a file descriptor, a string and a count.  It reads
 * from the file until either it has read "count" characters, or until
 * it reads a null byte.  When finished, what has been read exists in
 * the given string "s".  If "count" characters were actually read, the
 * last is changed to a null, so the returned string is always null-
 * terminated.  ok_getst() returns the number of characters read, including
 * the null terminator.
 *
 * If there is a read error, it returns -1 (like the read(2) system call)
 */

static int
#ifdef CK_ANSIC
ok_getst(int fd, register char *s, int n)
#else
ok_getst(fd, s, n) int fd; register char *s; int n;
#endif
{
    register int count = n;
    int err;
    while ((err = read(fd, s, 1)) > 0 && --count)
        if (*s++ == '\0')
            return (n - count);
    if (err < 0)
        return(-1);
    *s = '\0';
    return (n - count);
}

int
#ifdef CK_ANSIC
display_srvtab(char *file)
#else
display_srvtab(file) char *file;
#endif
{
    int stab;
    char serv[SNAME_SZ];
    char inst[INST_SZ];
    char rlm[REALM_SZ];
    unsigned char key[8];
    unsigned char vno;
    int count;

    printf("Server key file:   %s\r\n", file);
#ifdef NT
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif /* O_RDONLY */
#endif /* NT */

    if ((stab = open(file, O_RDONLY, 0400)) < 0) {
        perror(file);
        return(-1);
    }
    printf("%-15s %-15s %-10s %s\r\n","Service","Instance","Realm",
           "Key Version");
    printf("------------------------------------------------------\r\n");

    /* argh. getst doesn't return error codes, it silently fails */
    while (((count = ok_getst(stab, serv, SNAME_SZ)) > 0)
           && ((count = ok_getst(stab, inst, INST_SZ)) > 0)
           && ((count = ok_getst(stab, rlm, REALM_SZ)) > 0)) {
        if (((count = read(stab,(char *) &vno,1)) != 1) ||
             ((count = read(stab,(char *) key,8)) != 8)) {
            if (count < 0)
                perror("reading from key file");
            else
                printf("key file truncated\r\n");
            return(-1);
        }
        printf("%-15s %-15s %-15s %d\r\n",serv,inst,rlm,vno);
    }
    if (count < 0)
        perror(file);
    (void) close(stab);
    return(0);
}
#endif /* KLIST */
#else /* KRB4 */
int
ck_krb4_autoget_TGT(char * dummy)
{
    return(-1);
}
#ifdef CK_KERBEROS
int
#ifdef CK_ANSIC
ck_krb4_initTGT(struct krb_op_data * op, struct krb4_init_data * init)
#else
ck_krb4_initTGT(op,init)
    struct krb_op_data * op, struct krb4_init_data * init
#endif
{
    return(-1);
}

#ifdef CK_ANSIC
ck_krb4_destroy(struct krb_op_data * op)
#else
ck_krb4_destroy(op) struct krb_op_data * op;
#endif
{
    return(-1);
}
int
#ifdef CK_ANSIC
ck_krb4_list_creds(struct krb_op_data * op)
#else
ck_krb4_list_creds(op) struct krb_op_data * op;
#endif
{
    return(-1);
}
#else /* CK_KERBEROS */
int ck_krb4_initTGT(void * a, void *b)
{
    return(-1);
}
int ck_krb4_destroy(void *a)
{
    return(-1);
}
int ck_krb4_list_creds(void *a)
{
    return(-1);
}
#endif /* CK_KERBEROS */
#endif /* KRB4 */

/* The following functions are used to implement the Kermit Script Language */
/* functions                                                                */

struct tkt_list_item {
    char * name;
    struct tkt_list_item * next;
};

static struct tkt_list_item * k4_tkt_list = NULL;

int
#ifdef CK_ANSIC
ck_krb4_get_tkts(VOID)
#else
ck_krb4_get_tkts()
#endif
{
#ifdef KRB4
    char   *file=NULL;
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf1[20], buf2[20];
    int     k_errno;
#ifdef OS2
    LEASH_CREDENTIALS creds;
#else /* OS2 */
    CREDENTIALS creds;
#endif /* OS2 */
    int     tkt_count=0;
    struct  tkt_list_item ** list = &k4_tkt_list;

    while ( k4_tkt_list ) {
        struct tkt_list_item * next;
        next = k4_tkt_list->next;
        free(k4_tkt_list->name);
        free(k4_tkt_list);
        k4_tkt_list = next;
    }

    if ( !ck_krb4_is_installed() )
        return(-1);

    file = tkt_string();

    /*
     * Since krb_get_tf_realm will return a ticket_file error,
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that
     * the user would see would be
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }

    /* Close ticket file */
    (void) tf_close();

    /*
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    if ((k_errno = krb_get_tf_realm(file, prealm)) != AUTH_SUCCESS) {
        return(-1);
    }

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }
    /* Get principal name and instance */
    if ((k_errno = tf_get_pname(pname)) ||
         (k_errno = tf_get_pinst(pinst))) {
        return(-1);
    }

    /*
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why
     * it was done before tf_init.
     */

    while ((k_errno = tf_get_cred(&creds)) == AUTH_SUCCESS) {
        char tkt_buf[256];
        ckmakxmsg(tkt_buf,sizeof(tkt_buf),
                 creds.service, (creds.instance[0] ? "." : ""), creds.instance,
                 (creds.realm[0] ? "@" : ""), creds.realm,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        *list = (struct tkt_list_item *) malloc(sizeof(struct tkt_list_item));
        (*list)->name = strdup(tkt_buf);
        (*list)->next = NULL;
        list = &((*list)->next);
        tkt_count++;
    }

    tf_close();
    return(tkt_count);
#else /* KRB4 */
    return(0);
#endif /* KRB4 */
}

char *
#ifdef CK_ANSIC
ck_krb4_get_next_tkt(VOID)
#else
ck_krb4_get_next_tkt()
#endif
{
#ifdef KRB4
    static char * s=NULL;
    struct tkt_list_item * next=NULL;

    if ( s ) {
        free(s);
        s = NULL;
    }

    if ( k4_tkt_list == NULL )
        return(NULL);

    next = k4_tkt_list->next;
    s = k4_tkt_list->name;
    free(k4_tkt_list);
    k4_tkt_list = next;
    return(s);
#else /* KRB4 */
    return(NULL);
#endif /* KRB4 */
}

int
#ifdef CK_ANSIC
ck_krb4_tkt_isvalid(char * tktname)
#else
ck_krb4_tkt_isvalid(tktname) char * tktname;
#endif
{
#ifdef KRB4
    char   *file=NULL;
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf1[20], buf2[20];
    int     k_errno;
    time_t  issue_t, expire_t, now_t;
#ifdef OS2
    LEASH_CREDENTIALS creds;
#else /* OS2 */
    CREDENTIALS creds;
#endif /* OS2 */

    if ( !ck_krb4_is_installed() )
        return(-1);

    debug(F110,"ck_krb4_tkt_isvalid","tkt_string",0);
    file = tkt_string();

    /*
     * Since krb_get_tf_realm will return a ticket_file error,
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that
     * the user would see would be
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */

    /* Open ticket file */
    debug(F110,"ck_krb4_tkt_isvalid","tf_init",0);
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }

    /* Close ticket file */
    debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
    (void) tf_close();

    /*
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    debug(F110,"ck_krb4_tkt_isvalid","krb_get_tf_realm",0);
    if ((k_errno = krb_get_tf_realm(file, prealm)) != AUTH_SUCCESS) {
        return(-1);
    }

    /* Open ticket file */
    debug(F110,"ck_krb4_tkt_isvalid","tf_init",0);
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }
    /* Get principal name and instance */
    debug(F110,"ck_krb4_tkt_isvalid","tf_get_name/tf_get_pinst",0);
    if ((k_errno = tf_get_pname(pname)) ||
         (k_errno = tf_get_pinst(pinst))) {

        /* Close ticket file */
        debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
        (void) tf_close();

        return(-1);
    }

    /*
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why
     * it was done before tf_init.
     */

    debug(F110,"ck_krb4_tkt_isvalid","tf_get_cred",0);
    while ((k_errno = tf_get_cred(&creds)) == AUTH_SUCCESS) {
        char tkt_buf[256];
        ckmakxmsg(tkt_buf,sizeof(tkt_buf),
                 creds.service, (creds.instance[0] ? "." : ""), creds.instance,
                 (creds.realm[0] ? "@" : ""), creds.realm,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        if ( !strcmp(tktname,tkt_buf) ) {

            /* we found the ticket we are looking for */
            issue_t = creds.issue_date;
            expire_t = creds.issue_date
                + ((unsigned char) creds.lifetime) * 5 * 60;
            now_t = time(0);

            /* We add a 5 minutes fudge factor to compensate for potential */
            /* clock skew errors between the KDC and K95's host OS         */

            if ( now_t >= (issue_t-300) && now_t < expire_t) {
#ifdef OS2
#ifdef CHECKADDRS
                if ( krb4_checkaddrs ) {
                    extern char myipaddr[20];       /* From ckcnet.c */
                    if ( !myipaddr[0] ) {
                        int i;
                        char buf[60];
                        for ( i=0;i<64;i++ ) {
                            if ( getlocalipaddrs(buf,60,i) < 0 )
                                break;

                            if ( !strcmp(buf,creds.address) ) {
                                /* Close ticket file */
                                debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                                (void) tf_close();
                                return(1); /* They're the same */
                            }
                        }

                        /* Close ticket file */
                        debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                        (void) tf_close();
                        return(0);                  /* They're different */
                    } else if ( strcmp(myipaddr,creds.address) ) {
                        /* Close ticket file */
                        debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                        (void) tf_close();
                        return(0);                  /* They're different */
                    }
                    else {
                        /* Close ticket file */
                        debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                        (void) tf_close();
                        return(1);                  /* They're the same */
                    }
                } else {
                    /* Close ticket file */
                    debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                    (void) tf_close();
                    return(1);                  /* They're the same */
                }
#else /* CHECKADDRS */
                /* Close ticket file */
                debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                (void) tf_close();
                return(1);      /* valid but no ip address check */
#endif /* CHECKADDRS */
#else /* OS2 */
                /* Close ticket file */
                debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                (void) tf_close();
                return(1);      /* Valid but no ip address check */
#endif /* OS2 */
            }
            else {
                /* Close ticket file */
                debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
                (void) tf_close();
                return(0);      /* expired or otherwise invalid */
            }
        }
    }
    /* Close ticket file */
    debug(F110,"ck_krb4_tkt_isvalid","tf_close",0);
    (void) tf_close();
    return(0);                  /* could not find the desired ticket */
#else /* KRB4 */
    return(-1);
#endif /* KRB4 */
}

int
#ifdef CK_ANSIC
ck_krb4_is_tgt_valid(VOID)
#else
ck_krb4_is_tgt_valid()
#endif
{
#ifdef KRB4
    char tgt[256];
    char * s;
    int rc = 0;

    s = krb4_d_realm ? krb4_d_realm : ck_krb4_getrealm();
    ckmakmsg(tgt,sizeof(tgt),"krbtgt.",s,"@",s);
    rc = ck_krb4_tkt_isvalid(tgt);
    debug(F111,"ck_krb4_is_tgt_valid",tgt,rc);
    return(rc > 0);
#else /* KRB4 */
    return(0);
#endif /* KRB4 */
}

int
#ifdef CK_ANSIC
ck_krb4_tkt_time(char * tktname)
#else
ck_krb4_tkt_time(tktname) char * tktname;
#endif
{
#ifdef KRB4
    char   *file=NULL;
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf1[20], buf2[20];
    int     k_errno;
#ifdef OS2
    LEASH_CREDENTIALS creds;
#else /* OS2 */
    CREDENTIALS creds;
#endif /* OS2 */

    if ( !ck_krb4_is_installed() )
        return(-1);

    file = tkt_string();

    /*
     * Since krb_get_tf_realm will return a ticket_file error,
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that
     * the user would see would be
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }

    /* Close ticket file */
    (void) tf_close();

    /*
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    if ((k_errno = krb_get_tf_realm(file, prealm)) != AUTH_SUCCESS) {
        return(-1);
    }

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
        return(-1);
    }
    /* Get principal name and instance */
    if ((k_errno = tf_get_pname(pname)) ||
         (k_errno = tf_get_pinst(pinst))) {
        tf_close();
        return(-1);
    }

    /*
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why
     * it was done before tf_init.
     */

    while ((k_errno = tf_get_cred(&creds)) == AUTH_SUCCESS) {
        char tkt_buf[256];
        ckmakxmsg(tkt_buf,sizeof(tkt_buf),
                 creds.service, (creds.instance[0] ? "." : ""),
                 creds.instance,
                 (creds.realm[0] ? "@" : ""), creds.realm,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL);
        if ( !strcmp(tktname,tkt_buf) ) {
            /* we found the ticket we are looking for */
            int n = (creds.issue_date
                      + (((unsigned char) creds.lifetime) * 5 * 60))
                - time(0);
            tf_close();
            return(n <= 0 ? 0 : n);
        }
    }
    tf_close();
    return(0);                  /* could not find the desired ticket */
#else /* KRB4 */
    return(-1);
#endif /* KRB4 */
}

char *
#ifdef CK_ANSIC
ck_krb4_getrealm(void)
#else
ck_krb4_getrealm()
#endif
{
#ifdef KRB4
    char   *file=NULL;
    int     k_errno;
    static char realm[256]="";
    realm[0]='\0';

    if ( !ck_krb4_is_installed() )
        return(realm);

    /* Try to get realm from ticket file */
    /* If failure get the local realm    */

    /*
    * Since krb_get_tf_realm will return a ticket_file error,
    * we will call tf_init and tf_close first to filter out
    * things like no ticket file.
    */

    /* Open ticket file */
    file = tkt_string();
    if (file == NULL || !file[0])
        return(realm);

    if ((k_errno = tf_init(file, R_TKT_FIL)) == KSUCCESS) {
        /* Close ticket file */
        (void) tf_close();

        k_errno = krb_get_tf_realm(file, realm);
    }
    if (k_errno != KSUCCESS) {
        k_errno = krb_get_lrealm(realm, 1);
    }
    return(realm);
#else /* KRB4 */
    return("");
#endif /* KRB4 */
}

char *
#ifdef CK_ANSIC
ck_krb4_getprincipal(void)
#else
ck_krb4_getprincipal()
#endif
{
#ifdef KRB4
    char   *file=NULL;
    int     k_errno;
    static char principal[256]="";
    char        instance[256]="";
    char        realm[256]="";
    principal[0]='\0';

    if ( !ck_krb4_is_installed() )
        return(principal);

    /* Try to get realm from ticket file */
    /* If failure get the local realm    */

    /*
    * Since krb_get_tf_realm will return a ticket_file error,
    * we will call tf_init and tf_close first to filter out
    * things like no ticket file.
    */

    /* Open ticket file */
    file = tkt_string();
    if (file == NULL || !file[0])
        return(principal);

    if ((k_errno = tf_init(file, R_TKT_FIL)) == KSUCCESS) {
        /* Close ticket file */
        (void) tf_close();

        k_errno = krb_get_tf_fullname(file, principal, instance, realm);
    }
    return(principal);
#else /* KRB4 */
    return("");
#endif /* KRB4 */
}

static struct tkt_list_item * k5_tkt_list = NULL;

int
#ifdef CK_ANSIC
ck_krb5_get_tkts(char * cc_name)
#else
ck_krb5_get_tkts(cc_name) char * cc_name;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    krb5_context kcontext;
    krb5_error_code retval;
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ=NULL;
    krb5_flags flags=0;
    krb5_error_code code=0;
    int exit_status = 0;

    int     tkt_count=0;
    struct  tkt_list_item ** list = &k5_tkt_list;

    while ( k5_tkt_list ) {
        struct tkt_list_item * next;
        next = k5_tkt_list->next;
        free(k5_tkt_list->name);
        free(k5_tkt_list);
        k5_tkt_list = next;
    }

    if ( !ck_krb5_is_installed() )
        return(-1);

    retval = krb5_init_context(&kcontext);
    if (retval) {
        debug(F101,"ck_krb5_get_tkts while initializing krb5","",retval);
        return(-1);
    }

    code = k5_get_ccache(kcontext,&cache,cc_name);
    if (code != 0) {
        debug(F111,"ck_krb5_get_tkts while getting ccache",
               error_message(code),code);
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }

    flags = 0;                          /* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
        if (code == ENOENT) {
            debug(F111,"ck_krb5_get_tkts (ticket cache)",
                   krb5_cc_get_name(kcontext, cache),code);
        } else {
            debug(F111,
                 "ck_krb5_get_tkts while setting cache flags (ticket cache)",
                  krb5_cc_get_name(kcontext, cache),code);
        }
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
        debug(F101,"ck_krb5_get_tkts while retrieving principal name",
               "",code);
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
        debug(F101,"ck_krb5_get_tkts while unparsing principal name",
               "",code);
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
        debug(F101,"ck_krb5_get_tkts while starting to retrieve tickets",
               "",code);
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }

    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
        char *sname=NULL;

        retval = krb5_unparse_name(kcontext, creds.server, &sname);
        if (retval) {
            debug(F101,
                  "ck_krb5_get_tkts while unparsing server name","",retval);
            tkt_count = -1;
            goto exit_k5_get_tkt;
        }

        *list = (struct tkt_list_item *) malloc(sizeof(struct tkt_list_item));
        (*list)->name = sname;
        (*list)->next = NULL;
        list = &((*list)->next);

        krb5_free_unparsed_name(kcontext,sname);
        krb5_free_cred_contents(kcontext, &creds);
        tkt_count++;
    }

    if (code == KRB5_CC_END) {
        if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
            debug(F101,"ck_krb5_get_tkts while finishing ticket retrieval",
                   "",code);
            tkt_count = -1;
            goto exit_k5_get_tkt;
        }
        flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
        if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
            debug(F101,"ck_krb5_get_tkts while closing ccache",
                   "",code);
            tkt_count = -1;
            goto exit_k5_get_tkt;
        }
    } else {
        debug(F101,"ck_krb5_get_tkts while retrieving a ticket","",code);
        tkt_count = -1;
        goto exit_k5_get_tkt;
    }

  exit_k5_get_tkt:
    krb5_free_principal(kcontext,princ);
    krb5_free_unparsed_name(kcontext,defname);
    krb5_cc_close(kcontext,cache);
    krb5_free_context(kcontext);
    return(tkt_count);
#else /* HEIMDAL */
    return(-1);
#endif /* HEIMDAL */
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}

char *
#ifdef CK_ANSIC
ck_krb5_get_next_tkt(VOID)
#else
ck_krb5_get_next_tkt()
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    static char * s=NULL;
    struct tkt_list_item * next=NULL;

    if ( s ) {
        free(s);
        s = NULL;
    }

    if ( k5_tkt_list == NULL )
        return(NULL);

    next = k5_tkt_list->next;
    s = k5_tkt_list->name;
    free(k5_tkt_list);
    k5_tkt_list = next;
    return(s);
#else /* HEIMDAL */
    return("Not implemented");
#endif /* HEIMDAL */
#else /* KRB5 */
    return(NULL);
#endif /* KRB5 */
}

char *
#ifdef CK_ANSIC
ck_krb5_tkt_flags(char * cc_name, char * tktname)
#else
ck_krb5_tkt_flags(cc_name,tktname) char * cc_name; char * tktname;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    krb5_context kcontext;
    krb5_error_code retval;
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ=NULL;
    krb5_flags flags=0;
    krb5_error_code code=0;
    char * flag_str = "";

    if ( !ck_krb5_is_installed() )
        return("");

    retval = krb5_init_context(&kcontext);
    if (retval) {
        debug(F101,"ck_krb5_tkt_flags while initializing krb5","",retval);
        return("");
    }

    code = k5_get_ccache(kcontext,&cache,cc_name);
    if (code != 0) {
        debug(F111,"ck_krb5_tkt_isvalid while getting ccache",
               error_message(code),code);
        goto exit_k5_get_tkt;
    }

    flags = 0;                          /* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
        if (code == ENOENT) {
            debug(F111,"ck_krb5_tkt_flags (ticket cache)",
                   krb5_cc_get_name(kcontext, cache),code);
        } else {
            debug(F111,
                 "ck_krb5_tkt_flags while setting cache flags (ticket cache)",
                  krb5_cc_get_name(kcontext, cache),code);
        }
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
        debug(F101,"ck_krb5_tkt_flags while retrieving principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
        debug(F101,"ck_krb5_tkt_flags while unparsing principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
        debug(F101,"ck_krb5_tkt_flags while starting to retrieve tickets",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_timeofday(kcontext, &now))) {
        if (!status_only)
            debug(F101,"ck_krb5_tkt_flags while getting time of day.",
                   "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
        char *sname=NULL;

        retval = krb5_unparse_name(kcontext, creds.server, &sname);
        if (retval) {
            debug(F101,
                  "ck_krb5_tkt_flags while unparsing server name","",retval);
            retval = -1;
            krb5_free_cred_contents(kcontext, &creds);
            goto exit_k5_get_tkt;
        }

        if ( !strcmp(sname,tktname) ) {
            /* we found the ticket we are looking for */

            flag_str = flags_string(&creds);

            krb5_free_unparsed_name(kcontext,sname);
            krb5_free_cred_contents(kcontext, &creds);
            code = KRB5_CC_END;
            break;
        }
        krb5_free_unparsed_name(kcontext,sname);
        krb5_free_cred_contents(kcontext, &creds);
    }

    if (code == KRB5_CC_END) {
        if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
            debug(F101,"ck_krb5_tkt_flags while finishing ticket retrieval",
                   "",code);
            goto exit_k5_get_tkt;
        }
        flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
        if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
            debug(F101,"ck_krb5_tkt_flags while closing ccache",
                   "",code);
            goto exit_k5_get_tkt;
        }
    } else {
        debug(F101,"ck_krb5_tkt_flags while retrieving a ticket","",code);
        goto exit_k5_get_tkt;
    }

  exit_k5_get_tkt:
    krb5_free_principal(kcontext,princ);
    krb5_free_unparsed_name(kcontext,defname);
    krb5_cc_close(kcontext,cache);
    krb5_free_context(kcontext);
    return(flag_str);
#else /* HEIMDAL */
    return("Not implemented");
#endif /* HEIMDAL */
#else /* KRB5 */
    return("");
#endif /* KRB5 */
}


int
#ifdef CK_ANSIC
ck_krb5_tkt_isvalid(char * cc_name, char * tktname)
#else
ck_krb5_tkt_isvalid(cc_name,tktname) char * cc_name; char * tktname;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    krb5_context kcontext=NULL;
    krb5_error_code retval;
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ=NULL;
    krb5_flags flags=0;
    krb5_error_code code=0;
#ifdef CHECKADDRS
    krb5_address **     myAddrs=NULL;
    krb5_address **     p=NULL;
    BOOL                Addrfound = FALSE;
#endif /*CHECKADDRS*/

    if ( !ck_krb5_is_installed() )
        return(-1);

    retval = krb5_init_context(&kcontext);
    if (retval) {
        debug(F101,"ck_krb5_tkt_isvalid while initializing krb5","",retval);
        return(-1);
    }

    code = k5_get_ccache(kcontext,&cache,cc_name);
    if (code != 0) {
        debug(F111,"ck_krb5_tkt_isvalid while getting ccache",
               error_message(code),code);
        goto exit_k5_get_tkt;
    }

    flags = 0;                          /* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
        if (code == ENOENT) {
            debug(F111,"ck_krb5_tkt_isvalid (ticket cache)",
                   krb5_cc_get_name(kcontext, cache),code);
        } else {
            debug(F111,
                "ck_krb5_tkt_isvalid while setting cache flags (ticket cache)",
                  krb5_cc_get_name(kcontext, cache),code);
        }
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
        debug(F101,"ck_krb5_tkt_isvalid while retrieving principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
        debug(F101,"ck_krb5_tkt_isvalid while unparsing principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
        debug(F101,"ck_krb5_tkt_isvalid while starting to retrieve tickets",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_timeofday(kcontext, &now))) {
        if (!status_only)
            debug(F101,"ck_krb5_tkt_isvalid while getting time of day.",
                   "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
        char *sname=NULL;

        retval = krb5_unparse_name(kcontext, creds.server, &sname);
        if (retval) {
            debug(F101,
                  "ck_krb5_tkt_isvalid while unparsing server name","",retval);
            retval = -1;
            krb5_free_cred_contents(kcontext, &creds);
            goto exit_k5_get_tkt;
        }

        if ( !strcmp(sname,tktname) ) {
            /* we found the ticket we are looking for */

            /* We add a 5 minutes fudge factor to compensate for potential */
            /* clock skew errors between the KDC and K95's host OS         */

            retval = ((creds.times.starttime > 0) &&
                       now >= (creds.times.starttime - 300) &&
                       now < (creds.times.endtime + 300) &&
                       !(creds.ticket_flags & TKT_FLG_INVALID));

#ifdef CHECKADDRS
            if ( retval && krb5_checkaddrs &&
                                 creds.addresses && creds.addresses[0] ) {
                /* if we think it is valid, then lets check the IP Addresses */
                /* to make sure it is valid for our current connection.      */
                /* Also make sure it's for the correct IP address */
                retval = krb5_os_localaddr(kcontext, &myAddrs);
                if (retval) {
                    com_err(NULL, retval, "retrieving my IP address");
                    krb5_free_unparsed_name(kcontext,sname);
                    krb5_free_cred_contents(kcontext, &creds);
                    code = KRB5_CC_END;
                    retval = -1;
                    break;
                }

             /* See if any of our addresses match any in cached credentials */

                for (Addrfound=FALSE, p=myAddrs;
                     (Addrfound==FALSE) && (*p);
                     p++
                     ) {
                    if (krb5_address_search(kcontext, *p, creds.addresses)) {
                        Addrfound = TRUE;
                    }
                }
                krb5_free_addresses(k5_context, myAddrs);

                if (Addrfound) {
                    krb5_free_unparsed_name(kcontext,sname);
                    krb5_free_cred_contents(kcontext, &creds);
                    code = KRB5_CC_END;
                    retval = 1;
                    break;
                } else {
                    krb5_free_unparsed_name(kcontext,sname);
                    krb5_free_cred_contents(kcontext, &creds);
                    code = KRB5_CC_END;
                    retval = 0;
                    break;
                }
            }
#endif /* CHECKADDRS */

            krb5_free_unparsed_name(kcontext,sname);
            krb5_free_cred_contents(kcontext, &creds);
            code = KRB5_CC_END;
            break;
        }
        krb5_free_unparsed_name(kcontext,sname);
        krb5_free_cred_contents(kcontext, &creds);
    }

    if (code == KRB5_CC_END) {
        if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
            debug(F101,"ck_krb5_tkt_isvalid while finishing ticket retrieval",
                   "",code);
            retval = -1;
            goto exit_k5_get_tkt;
        }
        flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
        if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
            debug(F101,"ck_krb5_tkt_isvalid while closing ccache",
                   "",code);
            retval = -1;
            goto exit_k5_get_tkt;
        }
    } else {
        debug(F101,"ck_krb5_tkt_isvalid while retrieving a ticket","",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

  exit_k5_get_tkt:
    krb5_free_principal(kcontext,princ);
    krb5_free_unparsed_name(kcontext,defname);
    krb5_cc_close(kcontext,cache);
    krb5_free_context(kcontext);
    return(retval);
#else /* HEIMDAL */
    return(-1);
#endif /* HEIMDAL */
#else /* KRB5 */
    return(-1);
#endif /* KRB5 */
}

int
#ifdef CK_ANSIC
ck_krb5_is_tgt_valid(VOID)
#else
ck_krb5_is_tgt_valid()
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    char tgt[256];
    char * s;
    int rc = 0;

    s = ck_krb5_getrealm(krb5_d_cc);
    ckmakmsg(tgt,sizeof(tgt),"krbtgt/",s,"@",s);
    rc = ck_krb5_tkt_isvalid(krb5_d_cc,tgt);
    debug(F111,"ck_krb5_is_tgt_valid",tgt,rc);
    return(rc>0);
#else /* HEIMDAL */
    return(-1);
#endif /* HEIMDAL */
#else /* KRB5 */
    return(0);
#endif /* KRB5 */
}

int
#ifdef CK_ANSIC
ck_krb5_tkt_time(char * cc_name, char * tktname)
#else
ck_krb5_tkt_time(cc_name, tktname) char * cc_name; char * tktname;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    krb5_context kcontext;
    krb5_error_code retval;
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ=NULL;
    krb5_flags flags=0;
    krb5_error_code code=0;

    if ( !ck_krb5_is_installed() )
        return(-1);

    retval = krb5_init_context(&kcontext);
    if (retval) {
        debug(F101,"ck_krb5_list_creds while initializing krb5","",retval);
        return(-1);
    }

    code = k5_get_ccache(kcontext,&cache,cc_name);
    if (code != 0) {
        debug(F111,"ck_krb5_tkt_time while getting ccache",
               error_message(code),code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    flags = 0;                          /* turns off OPENCLOSE mode */
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
        if (code == ENOENT) {
            debug(F111,"ck_krb5_list_creds (ticket cache)",
                   krb5_cc_get_name(kcontext, cache),code);
        } else {
            debug(F111,
                 "ck_krb5_list_creds while setting cache flags (ticket cache)",
                  krb5_cc_get_name(kcontext, cache),code);
        }
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
        debug(F101,"ck_krb5_list_creds while retrieving principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
        debug(F101,"ck_krb5_list_creds while unparsing principal name",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
        debug(F101,"ck_krb5_list_creds while starting to retrieve tickets",
               "",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

    if ((code = krb5_timeofday(kcontext, &now))) {
        if (!status_only)
            debug(F101,"ck_krb5_list_creds while getting time of day.",
                   "",code);
        krb5_free_context(kcontext);
        return(-1);
    }

    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
        char *sname=NULL;

        retval = krb5_unparse_name(kcontext, creds.server, &sname);
        if (retval) {
            debug(F101,
                  "ck_krb5_list_creds while unparsing server name","",retval);
            retval = -1;
            krb5_free_unparsed_name(kcontext,sname);
            krb5_free_cred_contents(kcontext, &creds);
            goto exit_k5_get_tkt;
        }

        if ( !strcmp(sname,tktname) ) {
            /* we found the ticket we are looking for */
            int valid = (creds.times.starttime &&
                       now > creds.times.starttime &&
                       now < creds.times.endtime &&
                       !(creds.ticket_flags & TKT_FLG_INVALID));
            if ( valid ) {
                retval = creds.times.endtime - now;
            }
            else
                retval = 0;
            krb5_free_unparsed_name(kcontext,sname);
            krb5_free_cred_contents(kcontext, &creds);
            code = KRB5_CC_END;
            break;
        }
        krb5_free_unparsed_name(kcontext,sname);
        krb5_free_cred_contents(kcontext, &creds);
    }

    if (code == KRB5_CC_END) {
        if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
            debug(F101,"ck_krb5_list_creds while finishing ticket retrieval",
                   "",code);
            retval = -1;
            goto exit_k5_get_tkt;
        }
        flags = KRB5_TC_OPENCLOSE;      /* turns on OPENCLOSE mode */
        if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
            debug(F101,"ck_krb5_list_creds while closing ccache",
                   "",code);
            retval = -1;
            goto exit_k5_get_tkt;
        }
    } else {
        debug(F101,"ck_krb5_list_creds while retrieving a ticket","",code);
        retval = -1;
        goto exit_k5_get_tkt;
    }

  exit_k5_get_tkt:
    krb5_free_principal(kcontext,princ);
    krb5_free_unparsed_name(kcontext,defname);
    krb5_cc_close(kcontext,cache);
    krb5_free_context(kcontext);
    return(retval);
#else /* HEIMDAL */
    return(-1);
#endif /* HEIMDAL */
#else /* KRB5 */
    return(-1);
#endif /* KRB5 */
}

char *
#ifdef CK_ANSIC
ck_krb5_get_cc_name(void)
#else
ck_krb5_get_cc_name()
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    static char cc_name[CKMAXPATH+1]="";
    krb5_context kcontext = NULL;
    krb5_ccache ccache = NULL;
    krb5_error_code code;
    char * p=NULL;

    cc_name[0] = '\0';

    if ( !ck_krb5_is_installed() )
        return(cc_name);

    p = getenv("KRB5CCNAME");
    if ( !p ) {
        code = krb5_init_context(&kcontext);
        if (code) {
            com_err("ck_krb5_get_cc_name",code,"while init_context");
            return(cc_name);
        }
        if ((code = krb5_cc_default(kcontext, &ccache))) {
            com_err("ck_krb5_get_cc_name",code,"while getting default ccache");
            goto exit_k5_get_cc;
        }

        ckmakmsg(cc_name,sizeof(cc_name),
                 (char *)krb5_cc_get_type(kcontext,ccache),":",
                 (char *)krb5_cc_get_name(kcontext,ccache),NULL);
    } else {
        ckstrncpy(cc_name,p,CKMAXPATH);
    }

    if ( !strncmp("FILE:",cc_name,5) ) {
        for ( p=cc_name; *p ; p++ )
            if ( *p == '\\' ) *p = '/';
    }

  exit_k5_get_cc:
    if ( ccache )
        krb5_cc_close(kcontext,ccache);
    if ( kcontext )
        krb5_free_context(kcontext);
    return(cc_name);
#else /* HEIMDAL */
    return("Not implemented");
#endif /* HEIMDAL */
#else /* KRB5 */
    return("");
#endif /* KRB5 */
}

char *
#ifdef CK_ANSIC
ck_krb5_getrealm(char * cc_name)
#else
ck_krb5_getrealm(cc_name) char * cc_name;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    static char realm[256]="";
    krb5_context kcontext;
    krb5_ccache ccache = NULL;
    krb5_error_code code;
    krb5_principal me=NULL;

    realm[0] = '\0';

    if ( !ck_krb5_is_installed() )
        return(realm);

    code = krb5_init_context(&kcontext);
    if (code) {
        return(realm);
    }

    code = k5_get_ccache(kcontext,&ccache,cc_name);
    if (code != 0) {
        goto exit_k5_getrealm;
    }

	code = krb5_cc_get_principal(kcontext, ccache, &me);
	if (code)
		code = krb5_parse_name(kcontext, "foo", &me);
    if (code) {
        goto exit_k5_getrealm;
    }
    if ( krb5_princ_realm(kcontext, me)->length < sizeof(realm) ) {
        memcpy(realm,krb5_princ_realm(kcontext, me)->data,
                krb5_princ_realm(kcontext, me)->length);        /* safe */
       realm[krb5_princ_realm(kcontext, me)->length]='\0';
    }
  exit_k5_getrealm:
    if ( me )
        krb5_free_principal(kcontext,me);
    if ( ccache )
        krb5_cc_close(kcontext,ccache);
    if (kcontext)
        krb5_free_context(kcontext);
    return(realm);
#else /* HEIMDAL */
    return("Not implemented");
#endif /* HEIMDAL */
#else /* KRB5 */
    return("");
#endif /* KRB5 */
}

char *
#ifdef CK_ANSIC
ck_krb5_getprincipal(char * cc_name)
#else
ck_krb5_getprincipal(cc_name) char * cc_name;
#endif
{
#ifdef KRB5
#ifndef HEIMDAL
    static char principal[UIDBUFLEN+1]="";
    krb5_context kcontext;
    krb5_ccache ccache = NULL;
    krb5_error_code code;
    krb5_principal me;
    char * p=NULL;
    int i;

    principal[0] = '\0';

    if ( !ck_krb5_is_installed() )
        return(principal);

    code = krb5_init_context(&kcontext);
    if (code) {
        return(principal);
    }

    code = k5_get_ccache(kcontext,&ccache,cc_name);
    if (code != 0) {
        goto exit_k5_getprincipal;
    }

    if ((code = krb5_cc_get_principal(kcontext, ccache, &me))) {
        goto exit_k5_getprincipal;
    }

    if ((code = krb5_unparse_name (kcontext, me, &p))) {
        krb5_free_principal(kcontext,me);
        goto exit_k5_getprincipal;
    }

    ckstrncpy(principal,p,UIDBUFLEN);
    i = ckindex("@",principal,0,0,0);
    if (i)
      principal[i-1] = '\0';

    krb5_free_unparsed_name(kcontext,p);

  exit_k5_getprincipal:
    if ( ccache )
        krb5_cc_close(kcontext,ccache);
    if (kcontext)
        krb5_free_context(kcontext);
    return(principal);
#else /* HEIMDAL */
    return("Not implemented");
#endif /* HEIMDAL */
#else /* KRB5 */
    return("");
#endif /* KRB5 */
}

#ifndef CRYPT_DLL
int
ck_get_crypt_table(struct keytab ** pTable, int * pN)
{
#ifdef CK_ENCRYPTION
    return(get_crypt_table(pTable, pN));
#else /* ENCRYPTION */
    int i=0;
#ifndef OS2
    char * tmpstring = NULL;
#endif /* OS2 */

    if ( *pTable )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable)[i].kwd ) ;
        free ( *pTable )  ;
    }
    *pTable = NULL;
    *pN = 0;

    *pTable = malloc( sizeof(struct keytab) * 2 ) ;
    if ( !(*pTable) )
        return(0);

#ifdef OS2
    (*pTable)[0].kwd =strdup("automatic");
#else /* OS2 */
    makestr(&tmpstring,"automatic");
    (*pTable)[0].kwd = tmpstring;
    tmpstring = NULL;
#endif /* OS2 */
    (*pTable)[0].kwval = ENCTYPE_ANY;
    (*pTable)[0].flgs = 0;
#ifdef OS2
    (*pTable)[1].kwd =strdup("none");
#else /* OS2 */
    makestr(&tmpstring,"none");
    (*pTable)[1].kwd = tmpstring;
    tmpstring = NULL;
#endif /* OS2 */
    (*pTable)[1].kwval = 999;
    (*pTable)[1].flgs = 0;
    (*pN) = 2;

    return(2);
#endif /* ENCRYPTION */
}

VOID
ck_encrypt_send_support()
{
#ifdef CK_ENCRYPTION
    encrypt_send_support();
#endif /* ENCRYPTION */
}
#endif /* CRYPT_DLL */

/*
 *
 * Kstream
 *
 * Emulates the kstream package in Kerberos 4
 *
 */

int
kstream_destroy()
{
    if (g_kstream != NULL) {
        auth_destroy();                       /* Destroy authorizing */
        free(g_kstream);
        g_kstream=NULL;
    }
    return 0;
}

VOID
#ifdef CK_ANSIC
kstream_set_buffer_mode(int mode)
#else
kstream_set_buffer_mode(mode) int mode;
#endif
{
}


int
#ifdef CK_ANSIC
kstream_create_from_fd(int fd,
                       kstream_ptr data)
#else
kstream_create_from_fd(fd,data)
    int fd; kstream_ptr data;
#endif
{
    int n;

    g_kstream = malloc(sizeof(struct kstream_int));
    if (g_kstream == NULL)
        return 0;

    g_kstream->fd = fd;

    n = auth_init(g_kstream);                   /* Initialize authorizing */
    if (n) {
        free(g_kstream);
        g_kstream = NULL;
        return 0;
    }

    g_kstream->encrypt = NULL;
    g_kstream->decrypt = NULL;
    g_kstream->encrypt_type = ENCTYPE_ANY;
    g_kstream->decrypt_type = ENCTYPE_ANY;
    return 1;
}

#ifdef CK_KERBEROS
#ifdef RLOGCODE
static int do_lencheck, use_ivecs;
extern int rlog_inband;

#ifdef KRB5
void
rcmd_stream_init_krb5(in_keyblock, encrypt_flag, lencheck, am_client,
                           protonum)
     krb5_keyblock *in_keyblock;
     int encrypt_flag;
     int lencheck;
     int am_client;
     enum krb5_kcmd_proto protonum;
{
    krb5_error_code status;
    size_t blocksize;

    if (!encrypt_flag)
        return;

    desinbuf.data = des_inbuf;
    desoutbuf.data = des_outpkt+4;      /* Set up des buffers */
    k5_session_key = in_keyblock;

    do_lencheck = lencheck;

    if ( protonum == KCMD_OLD_PROTOCOL ) {
        use_ivecs = 0;
        return;
    }

    use_ivecs = 1;

    if (status = krb5_c_block_size(k5_context, k5_session_key->enctype,
                                   &blocksize)) {
        /* XXX what do I do? */
        printf("fatal kerberos 5 crypto library error\n");
        ttclos(0);
        return;
    }

    encivec_i[0].length = encivec_i[1].length = 
    encivec_o[0].length = encivec_o[1].length = blocksize;

    if ((encivec_i[0].data = malloc(encivec_i[0].length * 4)) == NULL) {
        /* XXX what do I do? */
        printf("fatal malloc failed\n");
        ttclos(0);
        return;
    }

    encivec_i[1].data = encivec_i[0].data + encivec_i[0].length;
    encivec_o[0].data = encivec_i[1].data + encivec_i[1].length;
    encivec_o[1].data = encivec_o[0].data + encivec_o[0].length;

    /* is there a better way to initialize this? */
    memset(encivec_i[0].data, am_client, blocksize);
    memset(encivec_o[0].data, 1 - am_client, blocksize);
    memset(encivec_i[1].data, 2 | am_client, blocksize);
    memset(encivec_o[1].data, 2 | (1 - am_client), blocksize);
}
#endif /* KRB5 */

int
#ifdef CK_ANSIC
ck_krb_rlogin(CHAR * hostname, int port,
               CHAR * localuser, CHAR * remoteuser, CHAR * term_speed,
               struct sockaddr_in * l_addr, struct sockaddr_in * r_addr,
               int kversion, int encrypt_flag)
#else /* CK_ANSIC */
ck_krb_rlogin(hostname, port,
               localuser, remoteuser, term_speed, l_addr, r_addr, encrypt_flag)
    CHAR * hostname; int port;
    CHAR * localuser; CHAR * remoteuser; CHAR * term_speed;
    struct sockaddr_in * l_addr; struct sockaddr_in * r_addr;
    int kversion; int encrypt_flag;
#endif /* CK_ANSIC */
{
    unsigned long status;
    char * realm=NULL;
    extern int ttyfd;
    int c;
    long msglen;

    debug(F111,"ck_krb_rlogin",hostname,port);

    if ( kversion == 4 && !ck_krb4_is_installed() ) {
        printf("?Kerberos 4 is not installed\r\n");
        return(-1);
    } else if ( kversion == 5 && !ck_krb5_is_installed() ) {
        printf("?Kerberos 5 is not installed\r\n");
        return(-1);
    }

    if ( encrypt_flag && !ck_crypt_is_installed() ) {
        printf("?Encryption is not installed\r\n");
        return(-1);
    }

    if ( kversion == 5 ) {
#ifdef KRB5
        krb5_flags authopts=0;
        krb5_ccache ccache=NULL;
        char *cksumbuf=NULL;
        char *service=NULL;
        char * kcmd_version=NULL;
        enum krb5_kcmd_proto use_proto;
        krb5_data cksumdat;
        krb5_creds *get_cred = 0;
        krb5_error_code status;
        krb5_error      *error = 0;
        krb5_ap_rep_enc_part *rep_ret = NULL;
        krb5_data outbuf;
        int rc;
        krb5_int32 seqno=0;
        krb5_int32 server_seqno=0;
        char ** realmlist=NULL;
        int buflen;
        char tgt[256];

        debug(F100,"ck_krb_rlogin version 5","",0);

        realm = ck_krb5_realmofhost((char *)hostname);
        if (!realm) {
            ckstrncpy(strTmp, "Can't find realm for host \"",AUTHTMPBL);
            ckstrncat(strTmp, (char *)hostname,AUTHTMPBL);
            ckstrncat(strTmp, "\"",AUTHTMPBL);
            printf("?Kerberos 5 error: %s\r\n",strTmp);
            krb5_errno = KRB5_ERR_HOST_REALM_UNKNOWN;
            makestr(&krb5_errmsg,strTmp);
            return(0);
        }

        ckmakmsg(tgt,sizeof(tgt),"krbtgt/",realm,"@",realm);
        debug(F110,"ck_rlog_rlogin TGT",tgt,0);
        if ( krb5_autoget &&
             !((ck_krb5_tkt_isvalid(NULL,tgt) > 0) ||
                (ck_krb5_is_tgt_valid() > 0)) )
            ck_krb5_autoget_TGT(realm);

        buflen = strlen((char *)term_speed) + strlen((char *)remoteuser) + 64;
        if ((cksumbuf = malloc(buflen)) == 0) {
            printf("Unable to allocate memory for checksum buffer.\r\n");
            return(-1);
        }
        ckmakmsg(cksumbuf,buflen,ckuitoa((unsigned short) ntohs(port)),":",
		 (char *)term_speed,(char *)remoteuser);
        cksumdat.data = cksumbuf;
        cksumdat.length = strlen(cksumbuf);

        status = krb5_init_context(&k5_context);
        if (status) {
            debug(F110,"ck_krb_rlogin()","unable to init_context",0);
            return(-1);
        }

        desinbuf.data = des_inbuf;
        desoutbuf.data = des_outpkt+4;  /* Set up des buffers */

        rc = k5_get_ccache(k5_context,&ccache,NULL);
        if (rc != 0) {
            com_err(NULL, rc, "while getting ccache.");
            return(0);
        }

        service = krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME;

        if (!(get_cred = (krb5_creds *)calloc(1, sizeof(krb5_creds)))) {
            printf("ck_krb_rlogin: no memory\r\n");
            return(-1);
        }
        memset(get_cred,0,sizeof(krb5_creds));
        status = krb5_sname_to_principal(k5_context, (char *) hostname,
					 service, KRB5_NT_SRV_HST,
					 &get_cred->server);
        if (status) {
            printf("ck_krb_rlogin: krb5_sname_to_principal failed: %s\r\n",
                     error_message(status));
            return(-1);
        }

        ttoc(0);

        if (status = krb5_cc_get_principal(k5_context,
                                           ccache,
                                           &get_cred->client)
            ) {
            (void) krb5_cc_close(k5_context, ccache);
            krb5_free_creds(k5_context, get_cred);
            goto bad;
        }

        if (krb5_rlog_ver == KCMD_OLD_PROTOCOL)
            get_cred->keyblock.enctype=ENCTYPE_DES_CBC_CRC;

        /* Get ticket from credentials cache or kdc */
        status = krb5_get_credentials(k5_context,
                                      0,
                                      ccache,
                                      get_cred,
                                      &ret_cred
                                      );
        krb5_free_creds(k5_context, get_cred);
        get_cred = NULL;
        (void) krb5_cc_close(k5_context, ccache);

        if (status) 
            goto bad;

        /* Reset internal flags; these should not be set. */
        authopts &= (~OPTS_FORWARD_CREDS);
        authopts &= (~OPTS_FORWARDABLE_CREDS);

        if (krb5_auth_con_init(k5_context, &auth_context))
            goto bad;

        if (krb5_auth_con_setflags(k5_context, auth_context,
                                    KRB5_AUTH_CONTEXT_RET_TIME))
            goto bad;

        /* Only need local address for mk_cred() to send to krlogind */
        if (!krb5_d_no_addresses)
            if (status = krb5_auth_con_genaddrs(k5_context,
                                                auth_context,
                                                 ttyfd,
                                KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR
                                                 ))
                goto bad;

        /* Here is where we start to handle the new protocol in earnest */
        if ( krb5_rlog_ver == KCMD_PROTOCOL_COMPAT_HACK ) {
            krb5_boolean is_des;

            if (status = krb5_c_enctype_compare( k5_context,
                                                 ENCTYPE_DES_CBC_CRC,
#ifdef HEIMDAL
                                                 ret_cred->session.keytype,
#else /* HEIMDAL */
                                                 ret_cred->keyblock.enctype,
#endif /* HEIMDAL */
                                                 &is_des)) {
                krb5_free_creds(k5_context, ret_cred);
                ret_cred = NULL;
                goto bad;
            }

            if ( is_des ) {
                kcmd_version = "KCMDV0.1";
                use_proto = KCMD_OLD_PROTOCOL;
            } else {
                authopts = AP_OPTS_USE_SUBKEY;
                kcmd_version = "KCMDV0.2";
                use_proto = KCMD_NEW_PROTOCOL;
            }
        } else {
            use_proto = krb5_rlog_ver;
            switch ( krb5_rlog_ver ) {
            case KCMD_NEW_PROTOCOL:
                authopts = AP_OPTS_USE_SUBKEY;
                kcmd_version = "KCMDV0.2";
                break;
            case KCMD_OLD_PROTOCOL:
                kcmd_version = "KCMDV0.1";
                break;
            default:
                goto bad;
            }
        }

        /* call Kerberos library routine to obtain an authenticator,
           pass it over the socket to the server, and obtain mutual
           authentication.
         */
        status = krb5_sendauth(k5_context,
                               &auth_context,
                               (krb5_pointer) &ttyfd,
                               kcmd_version,
                               ret_cred->client,
                               ret_cred->server,
                                authopts,
                               &cksumdat,
                               ret_cred,
                               0,
                               &error,
                               &rep_ret,
                               NULL
                               );
        krb5_free_data_contents(k5_context,&cksumdat);

        if (status) {
            if ( !quiet )
                printf("Couldn't authenticate to server: %s\r\n",
                        error_message(status));
            if (error) {
                if ( !quiet ) {
                    printf("Server returned error code %d (%s)\r\n",
                        error->error,
                        error_message(ERROR_TABLE_BASE_krb5 + error->error));
                    if (error->text.length) {
                        printf("Error text sent from server: %s\r\n",
                                error->text.data);
                    }
                }
                krb5_free_error(k5_context, error);
                error = 0;
            }
            goto bad;
        }

        if (rep_ret) {
            server_seqno = rep_ret->seq_number;
            krb5_free_ap_rep_enc_part(k5_context, rep_ret);
        }

        (void) ttol(remoteuser, strlen((char *)remoteuser)+1);
        (void) ttol(term_speed, strlen((char *)term_speed)+1);
        (void) ttol(localuser, strlen((char *)localuser)+1);

        if (forward_flag) {   /* Forward credentials (global) */
            if (status = krb5_fwd_tgt_creds( k5_context,
                                             auth_context,
                                             (char *)hostname,
                                             ret_cred->client,
                                             ret_cred->server,
                                             0,
                                             (forwardable_flag ?
                                               OPTS_FORWARDABLE_CREDS :
                                               0),
                                             &outbuf
                                             )
                 )
            {
                printf("Error forwarding credentials: %s\r\n",
                         error_message(status));
                goto bad2;
            }

            /* Send forwarded credentials */
            status = krb5_write_message(k5_context,
                                         (krb5_pointer)&ttyfd,
                                         &outbuf
                                         );
        }
        else { /* Dummy write to signal no forwarding */
          bad2:
            outbuf.length = 0;
            status = krb5_write_message(k5_context,
                                         (krb5_pointer)&ttyfd,
                                         &outbuf);
        }       

        if ((c = ttinc(0)) < 0) {
            if (c==-1) {
                perror((char *)hostname);
            } else {
                printf("ck_krb_rlogin: bad connection with remote host\r\n");
            }
            status = -1;
            goto bad;
        }
        if (c != 0) {
            while ((c = ttinc(1)) >= 0) {
                (void) printf("%c",c);
                if (c == '\n')
                    break;
            }
            status = -1;
            goto bad;
        }

        if ( status == 0 ) {        /* success */
            krb5_keyblock * key = 0;

            if ( use_proto == KCMD_NEW_PROTOCOL ) {
                int on = 1;
                rlog_inband = 1;
                setsockopt(ttyfd, SOL_SOCKET, SO_OOBINLINE,
                           (char *) &on, sizeof on);

                status = krb5_auth_con_getlocalsubkey( k5_context,
                                                       auth_context,
                                                       &key);
                if ((status || !key) && encrypt_flag )
                    goto bad2;
            }
            if ( key == 0 ) {
#ifdef HEIMDAL
                key = &ret_cred->session;
#else /* HEIMDAL */
                key = &ret_cred->keyblock;
#endif /* HEIMDAL */
            }

            rcmd_stream_init_krb5(key, encrypt_flag, 1, 1, use_proto);
            if ( encrypt_flag )
                rlog_encrypt = 1;
        }
        return (0);     /* success */

      bad:
        if ( status && !quiet ) {
            printf("Kerberos authentication error: %s\r\n",
                    error_message(status));
        }
        if (ret_cred) {
            krb5_free_creds(k5_context, ret_cred);
            ret_cred = NULL;
        }
        return (status);
#else /* KRB5 */
        return(-1);
#endif /* KRB5 */
    } else if (kversion == 4) {
#ifdef KRB4
        char tgt[4*REALM_SZ+1];
        debug(F100,"ck_krb_rlogin version 4","",0);

        realm = (char *)krb_realmofhost(hostname);
        if (!realm) {
            strcpy(strTmp, "Can't find realm for host \"");
            ckstrncat(strTmp, hostname,AUTHTMPBL);
            ckstrncat(strTmp, "\"",AUTHTMPBL);
            printf("?Kerberos 4 error: %s\r\n",strTmp);
            krb4_errno = 0;
            makestr(&krb4_errmsg,strTmp);
            return(0);
        }

        ckmakmsg(tgt,sizeof(tgt),"krbtgt.",realm,"@",realm);
        status = ck_krb4_tkt_isvalid(tgt);

        if ( status <= 0 && krb4_autoget )
            ck_krb4_autoget_TGT(realm);

        ttoc(0);        /* write a NUL */

        status = krb_sendauth(encrypt_flag?KOPT_DO_MUTUAL:0,
                               ttyfd,
                               &k4_auth,
                               krb4_d_srv ? krb4_d_srv : KRB4_SERVICE_NAME,
                               hostname,
                               realm,
                               (unsigned long) getpid(),
                               &k4_msg_data,
                               &cred,
#ifdef CK_ENCRYPTION
                               &k4_sched,
#else /* ENCRYPTION */
                               NULL,
#endif /* ENCRYPTION */
                               l_addr,
                               r_addr,
                               "KCMDV0.1");
        debug(F111,"ck_krb_rlogin","krb_sendauth",status);
        if (status != KSUCCESS) {
            printf( "krb_sendauth failed: %s\r\n",
                    krb_get_err_text_entry(status)
                    );
            return(-1);
        }
        ttol(remoteuser,strlen(remoteuser)+1);
        ttol(term_speed,strlen(term_speed)+1);

      reread:
        if ((c = ttinc(0)) < 0) {
            printf("rcmd: bad connection with remote host\r\n");
            return(-1);
        }
        debug(F111,"ck_krb_rlogin","first byte",c);

        if (c != 0) {
            char *check = "ld.so: warning:";
            /* If rlogind was compiled on SunOS4, and it somehow
            got the shared library version numbers wrong, it
            may give an ld.so warning about an old version of a
            shared library.  Just ignore any such warning.
            Note that the warning is a characteristic of the
            server; we may not ourselves be running under
            SunOS4.  */
            if (c == 'l') {
                char *p;
                char cc;

                p = &check[1];
                while ((c = ttinc(0)) >= 0) {
                    if (*p == '\0') {
                        if (c == '\n')
                            break;
                    } else {
                        if (c != *p)
                            break;
                        ++p;
                    }
                }

                if (*p == '\0')
                    goto reread;
            }

            printf(check);
            while ((c = ttinc(1)) >= 0) {
                printf("%c",c);
                if (c == '\n')
                    break;
            }
            debug(F110,"ck_krb_rlogin","fatal error 1",0);
            return(-1);
        }

#ifdef CK_ENCRYPTION
        if ( encrypt_flag ) {
            /* if we are encrypting we need to setup the encryption */
            /* routines.                                            */
            des_key_sched(cred.session, k4_sched);
            rlog_encrypt = 1;
        }
#endif /* ENCRYPTION */
#else /* KRB4 */
        return(-1);
#endif /* KRB4 */
    }
    return(0); /* success */
}

#define SRAND   srand
#define RAND    rand
#define RAND_TYPE       int

static long
random_confounder(size, fillin)
size_t size;
char * fillin;
{
    static int seeded = 0;
    register unsigned char *real_fill;
    RAND_TYPE   rval;

    if (!seeded) {
        /* time() defined in 4.12.2.4, but returns a time_t, which is an
           "arithmetic type" (4.12.1) */
        rval = (RAND_TYPE) time(0);
        SRAND(rval);
        rval = RAND();
        rval ^= getpid();
        SRAND(rval);
        seeded = 1;
    }

    real_fill = (unsigned char *)fillin;
    while (size > 0) {
        rval = RAND();
        *real_fill = rval & 0xff;
        real_fill++;
        size--;
        if (size) {
            *real_fill = (rval >> 8) & 0xff;
            real_fill++;
            size--;
        }
    }
    return 0;
}

#ifdef KRB5
int
krb5_des_avail(fd)
    int fd;
{
    return(nstored);
}

int
krb5_des_read(fd, buf, len, secondary)
    int fd;
    register char *buf;
    int len;
    int secondary;
{
    int nreturned = 0;
    long net_len,rd_len;
    int cc;
    krb5_error_code status;
    unsigned char c;
    krb5_data plain;
    krb5_enc_data cipher;

    debug(F111,"krb5_des_read","len",len);
    debug(F111,"krb5_des_read","rlog_encrypt",rlog_encrypt);
    if ( !rlog_encrypt ) {
        cc = net_read(fd, buf, len);
        debug(F111,"krb5_des_read","chars read",cc);
        if ( cc < 0 )
            netclos();
        return(cc);
    }

    if (nstored >= len) {
        if ( buf ) {
            memcpy(buf, store_ptr, len);        /* safe */
            store_ptr += len;
            nstored -= len;
            return(len);
        } else
            return(0);
    } else if (nstored) {
        if ( buf ) {
            memcpy(buf, store_ptr, nstored);    /* safe */
            nreturned += nstored;
            buf += nstored;
            len -= nstored;
            nstored = 0;
        }
        else
            return(0);
    }

    /* See the comment in v4_des_read. */
    while (1) {
        cc = net_read(fd, &c, 1);
        /* we should check for non-blocking here, but we'd have
        to make it save partial reads as well. */
        if (cc <= 0) {
            return cc; /* read error */
        }
        if (cc == 1) {
            if (c == 0 || !do_lencheck) 
                break;
        }
    }

    rd_len = c;
    if ((cc = net_read(fd, &c, 1)) != 1) return 0;
    rd_len = (rd_len << 8) | c;
    if ((cc = net_read(fd, &c, 1)) != 1) return 0;
    rd_len = (rd_len << 8) | c;
    if ((cc = net_read(fd, &c, 1)) != 1) return 0;
    rd_len = (rd_len << 8) | c;

    if (status = krb5_c_encrypt_length(k5_context, 
                                    k5_session_key->enctype,
                                    use_ivecs ? rd_len + 4 : rd_len,
				    (size_t *)&net_len)) {
        errno = status;
        return(-1);
    }

    if ((net_len <= 0) || (net_len > sizeof(des_inbuf))) {
        /* preposterous length; assume out-of-sync; only
           recourse is to close connection, so return 0 */
        printf("Read size problem.\r\n");
        return(0);
    }
    if ((cc = net_read(fd, desinbuf.data, net_len)) != net_len )
    {
        /* pipe must have closed, return 0 */
        printf( "Read error: length received %d != expected %d.\r\n",
                cc,
                net_len
                );
        return(cc);
    }


    /* decrypt info */
    cipher.enctype = ENCTYPE_UNKNOWN;
    cipher.ciphertext.length = net_len;
    cipher.ciphertext.data = desinbuf.data;
    plain.length = sizeof(storage);
    plain.data = storage;

    if ( status = krb5_c_decrypt(k5_context, k5_session_key, KCMD_KEYUSAGE,
                                 use_ivecs ? encivec_i + secondary : 0,
                                  &cipher,&plain) ) {
        /* probably out of sync */
        printf("Cannot decrypt data from network: %s\r\n",
                 error_message(status));
        errno = EIO;
        return(-1);
    }
    
    store_ptr = storage;
    nstored = rd_len;

    if ( use_ivecs ) {
        int rd_len2;
        rd_len2 = storage[0] & 0xff;
        rd_len2 <<= 8; rd_len2 |= storage[1] & 0xff;
        rd_len2 <<= 8; rd_len2 |= storage[2] & 0xff;
        rd_len2 <<= 8; rd_len2 |= storage[3] & 0xff;
        if (rd_len2 != rd_len) {
            /* cleartext length trashed? */
            errno = EIO;
            return -1;
        }
        store_ptr += 4;
    }

    if ( !buf )
        return(0);

#ifdef RLOGCODE                         /* blah */
    if (rlog_inband && (ttnproto == NP_K5LOGIN || ttnproto == NP_EK5LOGIN))
    {
        int i, left, n;

        for (i = 0; i < nstored; i++) {
            if (store_ptr[i] == '\377' &&
                store_ptr[i+1] == '\377') {
                left = nstored - i;
                n = rlog_ctrl(&store_ptr[i], left);
                if (n < 0) {
                    left -= (-n);
                    nstored = left;
                    /* flush before, and (-n) bytes */
                    if (left > 0)
                        memmove(store_ptr, &store_ptr[i-n], left);
                } else if (n) {
                    left -= n;
                    nstored -= n;
                    if (left > 0)
                        memmove(store_ptr, &store_ptr[n], left);
                }
            }
        }
    }
#endif /* RLOGCODE */

    if (nstored > len) {
        memcpy(buf, store_ptr, len);            /* safe */
        nreturned += len;
        store_ptr += len;
        nstored -= len;
    } else {
        memcpy(buf, store_ptr, nstored);        /* safe */
        nreturned += nstored;
        nstored = 0;
    }
    return(nreturned);
}

int
krb5_des_write(fd, buf, len, secondary)
    int fd;
    char *buf;
    int len;
    int secondary;
{
    char tmpbuf[2*RLOG_BUFSIZ+8];
    unsigned char *len_buf = (unsigned char *) tmpbuf;
    krb5_error_code status;
    krb5_data plain;
    krb5_enc_data cipher;

    debug(F111,"krb5_des_write","rlog_encrypt",rlog_encrypt);
    if ( !rlog_encrypt ) {
        int cc = net_write(fd, buf, len);
        debug(F111,"net_write","chars written",cc);
        return(cc != len ? -1 : len);
    }

    if (use_ivecs) {
        unsigned char *lenbuf2 = (unsigned char *) tmpbuf;
        if (len + 4 > sizeof(tmpbuf))
            abort ();
        lenbuf2[0] = (len & 0xff000000) >> 24;
        lenbuf2[1] = (len & 0xff0000) >> 16;
        lenbuf2[2] = (len & 0xff00) >> 8;
        lenbuf2[3] = (len & 0xff);
        memcpy (tmpbuf + 4, buf, len);

        plain.data = tmpbuf;
        plain.length = len + 4;
    } else {
        plain.data = buf;
        plain.length = len;
    }

    cipher.ciphertext.length = sizeof(des_outpkt)-4;
    cipher.ciphertext.data = desoutbuf.data;

    if ( status = krb5_c_encrypt(k5_context, k5_session_key, KCMD_KEYUSAGE,
                         use_ivecs ? encivec_o + secondary : 0,
                         &plain, &cipher)) {
        printf("Write encrypt problem: %s.\r\n",
                 error_message(status));
        errno = EIO;
        return(-1);
    }
    desoutbuf.length = cipher.ciphertext.length;

    len_buf = (unsigned char *) des_outpkt;
    len_buf[0] = (len & 0xff000000) >> 24;
    len_buf[1] = (len & 0xff0000) >> 16;
    len_buf[2] = (len & 0xff00) >> 8;
    len_buf[3] = (len & 0xff);

    if (net_write(fd, des_outpkt,desoutbuf.length+4)
         != desoutbuf.length+4){
        printf("Could not write out all data\r\n");
        return(-1);
    }
    else return(len);
}
#endif /* KRB5 */

#ifdef KRB4
/*
 * Note that the encrypted rlogin packets take the form of a four-byte
 * length followed by encrypted data.  On writing the data out, a significant
 * performance penalty is suffered (at least one RTT per character, two if we
 * are waiting for a shell to echo) by writing the data separately from the
 * length.  So, unlike the input buffer, which just contains the output
 * data, the output buffer represents the entire packet.
 */

int
krb4_des_avail(fd)
    int fd;
{
    return(nstored);
}

int
krb4_des_read(fd, buf, len)
int fd;
register char *buf;
int len;
{
    int nreturned = 0;
    unsigned long net_len, rd_len;
    int cc;
    unsigned char c;
    int gotzero = 0;

    debug(F111,"krb4_des_read","rlog_encrypt",rlog_encrypt);
    debug(F111,"krb4_des_read","len",len);
    if ( !rlog_encrypt ) {
        cc = net_read(fd, buf, len);
        debug(F111,"krb4_des_read","chars read",cc);
        if ( cc < 0 )
            netclos();
        return(cc);
    }

    if (nstored >= len) {
        if ( buf ) {
            debug(F111,"krb4_des_read (nstored >= len)","nstored",nstored);
            memcpy(buf, store_ptr, len);        /* safe */
            store_ptr += len;
            nstored -= len;
            return(len);
        } else
            return(0);
    } else if (nstored) {
        if ( buf ) {
            debug(F111,"krb4_des_read (nstored)","nstored",nstored);
            memcpy(buf, store_ptr, nstored);    /* safe */
            nreturned += nstored;
            buf += nstored;
            len -= nstored;
            nstored = 0;
        } else
            return(0);
    }

    /* We're fetching the length which is MSB first, and the MSB
    has to be zero unless the client is sending more than 2^24
    (16M) bytes in a single write (which is why this code is in
    rlogin but not rcp or rsh.) The only reasons we'd get something
    other than zero are:
    -- corruption of the tcp stream (which will show up when
    everything else is out of sync too)
    -- un-caught Berkeley-style "pseudo out-of-band data" which
    happens any time the user hits ^C twice.
    The latter is *very* common, as shown by an 'rlogin -x -d'
    using the CNS V4 rlogin.         Mark EIchin 1/95
    */
    debug(F110,"krb4_des_read",
          "about to call net_read() this will block",
          0
          );
    do {
        cc = net_read(fd, &c, 1);
        debug(F111,"net_read","chars read",cc);
        if (cc <= 0) {
            netclos();
            return(-1);
        }
        if (cc != 1) return 0; /* read error */
        if (cc == 1) {
            if (c == 0) gotzero = 1;
        }
    } while (!gotzero);

    debug(F110,"krb4_des_read","gotzero",0);
    cc = net_read(fd, &c, 1);
    debug(F111,"net_read","chars read",cc);
    if (cc < 0) {
        netclos();
        return(-1);
    } else if ( cc != 1 )
        return(0);
    net_len = c;
    cc = net_read(fd, &c, 1);
    debug(F111,"net_read","chars read",cc);
    if (cc < 0) {
        netclos();
        return(-1);
    } else if ( cc != 1 )
        return(0);
    net_len = (net_len << 8) | c;
    debug(F111,"net_read","chars read",cc);
    cc = net_read(fd, &c, 1);
    if (cc < 0) {
        netclos();
        return(-1);
    } else if ( cc != 1 )
        return(0);
    net_len = (net_len << 8) | c;
    debug(F111,"krb4_des_read","net_len",net_len);

    /* Note: net_len is unsigned */
    if (net_len > sizeof(des_inbuf)) {
        /* XXX preposterous length, probably out of sync.
        act as if pipe closed */
        return(0);
    }
    /* the writer tells us how much real data we are getting, but
    we need to read the pad bytes (8-byte boundary) */
#ifndef roundup
#define roundup(x,y) ((((x)+(y)-1)/(y))*(y))
#endif /* roundup */
    rd_len = roundup(net_len, 8);
    debug(F111,"krb4_des_read","rd_len",rd_len);
    cc = net_read(fd, des_inbuf, rd_len);
    debug(F111,"net_read","chars read",cc);
    if (cc < 0) {
        netclos();
        return(-1);
    } else if ( cc != rd_len )
        return(0);

    ckhexdump("krb4_des_read des_inbuf",des_inbuf,8);
#ifdef CK_ENCRYPTION
#ifdef KRB524
    (void) des_pcbc_encrypt(des_inbuf,
                             storage,
                             (net_len < 8) ? 8 : net_len,
                             k4_sched,
                             cred.session,
                             DECRYPT);
#else /* KRB524 */
    (void) des_pcbc_encrypt((Block *)des_inbuf,
                             (Block *)storage,
                             (net_len < 8) ? 8 : net_len,
                             k4_sched,
                             &cred.session,
                             DECRYPT);
#endif /* KRB524 */
#endif /* ENCRYPTION */
    ckhexdump("krb4_des_read storage",storage,8);

    /*
    * when the cleartext block is < 8 bytes, it is "right-justified"
    * in the block, so we need to adjust the pointer to the data
    */
    if (net_len < 8)
        store_ptr = storage + 8 - net_len;
    else
        store_ptr = storage;
    nstored = net_len;

    if ( !buf )
        return(0);

    if (nstored > len) {
        memcpy(buf, store_ptr, len);            /* safe */
        nreturned += len;
        store_ptr += len;
        nstored -= len;
    } else {
        memcpy(buf, store_ptr, nstored);        /* safe */
        nreturned += nstored;
        nstored = 0;
    }

    debug(F111,"net_read","nreturned",nreturned);
    return(nreturned);
}

int
krb4_des_write(fd, buf, len)
int fd;
char *buf;
int len;
{
    static char garbage_buf[8];
    unsigned char *len_buf = (unsigned char *) des_outpkt;
    int cc;

    debug(F111,"krb4_des_write","rlog_encrypt",rlog_encrypt);
    if ( !rlog_encrypt ) {
        cc = net_write(fd, buf, len);
        debug(F111,"net_write","chars written",cc);
        return(cc);
    }

    /*
    * pcbc_encrypt outputs in 8-byte (64 bit) increments
    *
    * it zero-fills the cleartext to 8-byte padding,
    * so if we have cleartext of < 8 bytes, we want
    * to insert random garbage before it so that the ciphertext
    * differs for each transmission of the same cleartext.
    * if len < 8 - sizeof(long), sizeof(long) bytes of random
    * garbage should be sufficient; leave the rest as-is in the buffer.
    * if len > 8 - sizeof(long), just garbage fill the rest.
    */
    if (len < 8) {
        random_confounder(8 - len, garbage_buf);
        /* this "right-justifies" the data in the buffer */
        (void) memcpy(garbage_buf + 8 - len, buf, len); /* safe */
        ckhexdump("krb4_des_write garbage_buf",garbage_buf,8);
    } else
        ckhexdump("krb4_des_write buf",buf,8);
#ifdef CK_ENCRYPTION
#ifdef KRB524
    (void) des_pcbc_encrypt((len < 8) ? garbage_buf : buf,
                             des_outpkt+4,
                             (len < 8) ? 8 : len,
                             k4_sched,
                             cred.session,
                             ENCRYPT);
#else /* KRB524 */
    (void) des_pcbc_encrypt((Block *)((len < 8) ? garbage_buf : buf),
                             (Block *)(des_outpkt+4),
                             (len < 8) ? 8 : len,
                             k4_sched,
                             &cred.session,
                             ENCRYPT);
#endif /* KRB524 */
#endif /* ENCRYPTION */
    if ( len < 8 )
        ckhexdump("krb4_des_write (post pcbc) garbage_buf",garbage_buf,8);
    else
        ckhexdump("krb4_des_write (post pcbc) buf",buf,8);
    ckhexdump("krb4_des_write (des_outpkt+4)",(des_outpkt+4),8);

    /* tell the other end the real amount, but send an 8-byte padded
    packet */
    len_buf[0] = (len & 0xff000000) >> 24;
    len_buf[1] = (len & 0xff0000) >> 16;
    len_buf[2] = (len & 0xff00) >> 8;
    len_buf[3] = (len & 0xff);
    ckhexdump("krb4_des_write des_outpkt len",des_outpkt,12);
    cc = net_write(fd, des_outpkt, roundup(len,8)+4);
    debug(F111,"net_write","chars written",cc);
    return(len);
}
#endif /* KRB4 */
#endif /* RLOGCODE */

#ifdef KRB524
#ifndef OS2
/* The following functions are missing from the compatibility library */
const char *
krb_get_err_text_entry(r) int r;
{
    extern char krb_err_text[];
    return(krb_err_txt[r]);
}
#endif /* OS2 */
#endif /* KRB524 */
#endif /* CK_KERBEROS */

#ifdef CK_KERBEROS
#ifdef KRB5_U2U
/* Kerberos 5 User to User Client */
int
k5_user_to_user_client_auth()
{
    extern int ttyfd;
    register int retval, i;
    char **srealms;             /* realm(s) of server */
    char *princ;                /* principal in credentials cache */
    krb5_ccache cc;
    krb5_creds creds, *new_creds;
    krb5_data reply, msg, msgtext, princ_data;
    krb5_ticket * ticket = NULL;

    if (retval = k5_get_ccache(k5_context,&cc,NULL))
    {
        com_err("uu-client", retval, "getting credentials cache");
        return(-1);
    }

    memset ((char*)&creds, 0, sizeof(creds));
    if (retval = krb5_cc_get_principal(k5_context, cc, &creds.client))
    {
        com_err("uu-client", retval, "getting principal name");
        return(-1);
    }

    if (retval = krb5_get_host_realm(k5_context, szHostName, &srealms))
    {
        com_err("uu-client", retval, "getting realms for \"%s\"", szHostName);
        return(-1);
    }

    if (retval = krb5_build_principal_ext(k5_context, &creds.server,
                                          krb5_princ_realm(k5_context,
                                                         creds.client)->length,
                                          krb5_princ_realm(k5_context,
                                                         creds.client)->data,
                                          6, "krbtgt",
                                          krb5_princ_realm(k5_context,
                                                         creds.client)->length,
                                          krb5_princ_realm(k5_context,
                                                         creds.client)->data,
                                          0))
    {
        com_err("uu-client", retval, "setting up tgt server name");
        return(-1);
    }

    /* Get TGT from credentials cache */
    if (retval = krb5_get_credentials(k5_context, KRB5_GC_CACHED, cc,
                                       &creds, &new_creds))
    {
        com_err("uu-client", retval, "getting TGT");
        return(-1);
    }

    if (retval = krb5_unparse_name(k5_context, creds.client, &princ)) {
        com_err("uu-client", retval, "printing principal name");
        return(-1);
    }
    i = strlen(princ) + 1;
    princ_data.data = princ;
    princ_data.length = i;              /* include null terminator for
                                           server's convenience */
    retval = krb5_write_message(k5_context,
                                (krb5_pointer) &ttyfd, &princ_data);
    if (retval)
    {
        com_err("uu-client", retval, "sending principal name to server");
        return(-1);
    }
    krb5_free_unparsed_name(k5_context,princ);

    retval = krb5_write_message(k5_context,
                                (krb5_pointer) &ttyfd, &new_creds->ticket);
    if (retval)
    {
        com_err("uu-client", retval, "sending ticket to server");
        return(-1);
    }

    retval = krb5_read_message(k5_context, (krb5_pointer) &ttyfd, &reply);
    if (retval)
    {
        com_err("uu-client", retval, "reading reply from server");
        return(-1);
    }

    if (retval = krb5_auth_con_init(k5_context, &auth_context)) {
        com_err("uu-client", retval, "initializing the auth_context");
        return(-1);
    }

    if (!krb5_d_no_addresses) {
      if (retval = krb5_auth_con_genaddrs(k5_context, auth_context, ttyfd,
                        KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR |
                        KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR)) {
        com_err("uu-client", retval, "generating addrs for auth_context");
        return(-1);
      }
    }

    if (retval = krb5_auth_con_setflags(k5_context, auth_context,
                                        KRB5_AUTH_CONTEXT_DO_SEQUENCE)) {
        com_err("uu-client", retval, "initializing the auth_context flags");
        return(-1);
    }

    if (retval = krb5_auth_con_setuseruserkey(k5_context, auth_context,
                                              &new_creds->keyblock)) {
        com_err("uu-client", retval, "setting useruserkey for authcontext");
        return(-1);
    }

    /* read the ap_req to get the session key */
    retval = krb5_rd_req(k5_context, &auth_context, &reply,
                          NULL, NULL, NULL, &ticket);
    if (retval) {
        com_err("uu-client", retval, "reading AP_REQ from server");
        return(-1);
    }

    if (k5_u2u_read_msg(k5_context,&msg) < 0)
        return(-1);

    if ( strcmp("Kermit implements Kerberos 5 User to User",msg.data) )
        return(-1);
    krb5_free_data_contents(k5_context,&msg);

    msgtext.data = "As do I! :-)";
    msgtext.length = strlen(msgtext.data)+1;

    if (k5_u2u_write_msg(k5_context,&msgtext) < 0)
        return(-1);

    if (retval = krb5_unparse_name(k5_context,
#ifdef HEIMDAL
                                    ticket->client,
#else /* HEIMDAL */
                                    ticket->enc_part2->client,
#endif /* HEIMDAL */
                                    &princ))
        com_err("uu-client", retval, "while unparsing client name");
    else {
        ckstrncpy(szUserNameAuthenticated,princ,UIDBUFLEN);
        validUser = AUTH_VALID;
        authentication_version = AUTHTYPE_KERBEROS_V5;
        if ( !quiet )
            printf("Peer name is \"%s\"\n", princ);
        krb5_free_unparsed_name(k5_context,princ);
    }
    return 0;
}

/* Kerberos 5 User to User Server */

int
k5_user_to_user_server_auth()
{
    krb5_data pname_data, tkt_data;
    int retval;
    krb5_creds creds, *new_creds;
    krb5_ccache cc;
    krb5_data msg, msgtext;
    extern int ttyfd;

    if (retval = krb5_read_message(k5_context,
                                   (krb5_pointer) &ttyfd, &pname_data)) {
        com_err ("uu-server", retval, "reading pname");
        return(-1);
    }
    /* client sends it already null-terminated. */
    if ( !quiet )
        printf ("Peer name is \"%s\".\n", pname_data.data);
    ckstrncpy(szUserNameAuthenticated,pname_data.data,UIDBUFLEN);
    validUser = AUTH_VALID;
    authentication_version = AUTHTYPE_KERBEROS_V5;

    if (retval = krb5_read_message(k5_context,
                                   (krb5_pointer) &ttyfd, &tkt_data)) {
        com_err ("uu-server", retval, "reading ticket data");
        return(-1);
    }

    if (retval = k5_get_ccache(k5_context,&cc,NULL))
    {
        com_err("uu-server", retval, "getting credentials cache");
        return(-1);
    }

    memset ((char*)&creds, 0, sizeof(creds));
    if (retval = krb5_cc_get_principal(k5_context, cc, &creds.client))
    {
        com_err("uu-server", retval, "getting principal name");
        return(-1);
    }

    if (retval = krb5_parse_name(k5_context, pname_data.data, &creds.server))
    {
        com_err("uu-server", retval, "parsing client name");
        return(-1);
    }
    creds.second_ticket = tkt_data;

    if (retval = krb5_get_credentials(k5_context, KRB5_GC_USER_USER,
                                       cc, &creds, &new_creds))
    {
        com_err("uu-server", retval, "getting user-user ticket");
        return(-1);
    }

    /* send a ticket/authenticator to the other side, so it can get the key
       we're using for the krb_safe below. */

    if (retval = krb5_auth_con_init(k5_context, &auth_context)) {
        com_err("uu-server", retval, "making auth_context");
        return(-1);
    }

    if (retval = krb5_auth_con_setflags(k5_context, auth_context,
                                         KRB5_AUTH_CONTEXT_DO_SEQUENCE)) {
        com_err("uu-server", retval, "initializing the auth_context flags");
        return(-1);
    }

    if (!krb5_d_no_addresses) {
      if (retval = krb5_auth_con_genaddrs(k5_context, auth_context, ttyfd,
                                KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR |
                                KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR)) {
        com_err("uu-server", retval, "generating addrs for auth_context");
        return(-1);
      }
    }

    if (retval = krb5_auth_con_setuseruserkey(k5_context, auth_context,
                                              &new_creds->keyblock)) {
        com_err("uu-server", retval, "setting useruserkey for authcontext");
        return(-1);
    }

    if (retval = krb5_mk_req_extended(k5_context, &auth_context,
                                      AP_OPTS_USE_SESSION_KEY |
                                      AP_OPTS_MUTUAL_REQUIRED,
                                      NULL, new_creds, &msg)) {
        com_err("uu-server", retval, "making AP_REQ");
        return(-1);
    }
    retval = krb5_write_message(k5_context, (krb5_pointer) &ttyfd, &msg);
    if (retval) {
        com_err("uu-server", retval, "writing message to client");
        return(-1);
    }
    krb5_free_data_contents(k5_context,&msg);

    msgtext.data = "Kermit implements Kerberos 5 User to User";
    msgtext.length = strlen(msgtext.data)+1;

    if (k5_u2u_write_msg(k5_context,&msgtext) < 0)
        return(-1);

    if (k5_u2u_read_msg(k5_context,&msg) < 0)
        return(-1);

    if ( strcmp("As do I! :-)",msg.data) )
        return(-1);
    krb5_free_data_contents(k5_context,&msg);


    return(0);
}

int
k5_u2u_read_msg(krb5_context context, int fd, krb5_data * msg)
{
    int retval;
    krb5_data reply;

    retval = krb5_read_message(context, (krb5_pointer) &fd, &reply);
    if (retval)
    {
        com_err("uu-client", retval, "reading reply");
        return(-1);
    }

    if (retval = krb5_rd_priv(context, auth_context, &reply, msg, NULL)) {
        com_err("uu-client", retval, "decoding reply");
        return(-1);
    }
    return(0);
}

int
k5_u2u_write_msg(krb5_context context, int fd, krb5_data * msgtext)
{
    int retval;
    krb5_data msg;

    if (retval = krb5_mk_priv(k5_context, auth_context, msgtext, &msg, NULL))
    {
        com_err("uu-server", retval, "encoding message");
        return(-1);
    }

    retval = krb5_write_message(k5_context, (krb5_pointer) &fd, &msg);
    krb5_free_data_contents(k5_context,&msg);
    if (retval)
    {
        com_err("uu-server", retval, "writing message");
        return(-1);
    }
    return(0);
}

int
krb5_u2u_avail(fd)
    int fd;
{
    return(nstored);
}

int
krb5_u2u_read(fd, buf, len)
     int fd;
     register char *buf;
     int len;
{
    int nreturned = 0;
    krb5_data msg;

    debug(F111,"krb5_u2u_read","len",len);

    if ( !buf )
        return(0);

    if (nstored >= len) {
        memcpy(buf, store_ptr, len);        /* safe */
        store_ptr += len;
        nstored -= len;
        return(len);
    } else if (nstored) {
        memcpy(buf, store_ptr, nstored);    /* safe */
        nreturned += nstored;
        buf += nstored;
        len -= nstored;
        nstored = 0;
    }

    if (k5_u2u_read_msg(k5_context, fd, &msg) < 0)
        return(-1);

    if ( msg.length <= len ) {
        memcpy(buf, msg.data, msg.length);
        nreturned += msg.length;
        nstored = 0;
    } else {
        memcpy(buf, msg.data, len);
        nreturned += len;

        if ( msg.length - len < sizeof(storage) ) {
            store_ptr = storage;
            nstored = msg.length - len;
            memcpy(storage,msg.data+len,nstored);
        } else {
            nstored = 0;
            return(-1);
        }
    }
    return(nreturned);
}

int
krb5_u2u_write(fd, buf, len)
     int fd;
     char *buf;
     int len;
{
    krb5_data msg;

    msg.length = len;
    msg.data = buf;

    if ( k5_u2u_write_msg(k5_context, fd, &msg) < 0 )
        return(-1);
    else
        return(len);
}

#endif /* KRB5_U2U */
#endif /* CK_KERBEROS */

#ifdef CK_FORWARD_X
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/
/*  Modified for stand-alone compiling by
 *  Peter 'Luna' Runestig <peter@runestig.com>
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#define Time_t time_t

void
XauDisposeAuth (auth)
Xauth   *auth;
{
    if (auth) {
        if (auth->address) (void) free (auth->address);
        if (auth->number) (void) free (auth->number);
        if (auth->name) (void) free (auth->name);
        if (auth->data) {
            (void) bzero (auth->data, auth->data_length);
            (void) free (auth->data);
        }
        free ((char *) auth);
    }
    return;
}

char *
XauFileName ()
{
    char *slashDotXauthority = "/.Xauthority";
    char    *name;
    static char *buf=NULL;
    static int  bsize=0;
    int     size, namelen;
    extern char * tn_fwdx_xauthority;

    if ( tn_fwdx_xauthority )
        return(tn_fwdx_xauthority);

    if (name = getenv ("XAUTHORITY"))
        return(name);
    name = zhome();
    if ( !name )
        return(NULL);
    namelen = strlen (name);
    size = namelen + strlen(slashDotXauthority) + 1;
    if (size > bsize) {
        if (buf)
            free (buf);
        buf = malloc ((unsigned) size);
        if (!buf)
            return 0;
        bsize = size;
    }
    ckstrncpy (buf, name, bsize);
    if ( name[namelen-1] != '/'
#ifdef OS2
         && name[namelen-1] != '\\'
#endif /* OS2 */
         )
        ckstrncat (buf, slashDotXauthority, bsize);
    else
        ckstrncat (buf, &slashDotXauthority[1], bsize);
    return(buf);
}

static int
binaryEqual (a, b, len)
register char   *a, *b;
register int    len;
{
    while (len--)
        if (*a++ != *b++)
            return 0;
    return 1;
}

#ifndef R_OK
#define R_OK 04
#endif /* R_OK */

Xauth *
XauGetAuthByAddr (family, address_length, address,
                          number_length, number,
                          name_length, name)
unsigned int    family;
unsigned int    address_length;
const char      *address;
unsigned int    number_length;
const char      *number;
unsigned int    name_length;
const char      *name;
{
    FILE    *auth_file;
    char    *auth_name;
    Xauth   *entry;

    auth_name = XauFileName();
    if (!auth_name)
        return 0;
    if (access (auth_name, R_OK) != 0)          /* checks REAL id */
        return 0;
    auth_file = fopen (auth_name, "rb");
    if (!auth_file)
        return 0;
    for (;;) {
        entry = XauReadAuth (auth_file);
        if (!entry)
            break;
        /*
         * Match when:
         *   either family or entry->family are FamilyWild or
         *    family and entry->family are the same
         *  and
         *   either address or entry->address are empty or
         *    address and entry->address are the same
         *  and
         *   either number or entry->number are empty or
         *    number and entry->number are the same
         *  and
         *   either name or entry->name are empty or
         *    name and entry->name are the same
         */

/*      if ((family == FamilyWild || entry->family == FamilyWild ||
             (entry->family == family &&
              address_length == entry->address_length &&
              binaryEqual (entry->address, address, (int)address_length))) &&
            (number_length == 0 || entry->number_length == 0 ||
             (number_length == entry->number_length &&
              binaryEqual (entry->number, number, (int)number_length))) &&
            (name_length == 0 || entry->name_length == 0 ||
             (entry->name_length == name_length &&
              binaryEqual (entry->name, name, (int)name_length)))) */
        /* the original matching code above doesn't seem to meet the matching
         * algorithm, it doesn't check if "address_length == 0 ||
         * entry->address_length == 0". / Luna 2000-02-09
         */
        if ((family == FamilyWild || entry->family == FamilyWild ||
              entry->family == family) &&
            (address_length == 0 || entry->address_length == 0 ||
              (address_length == entry->address_length &&
              binaryEqual (entry->address, address, (int)address_length))) &&
            (number_length == 0 || entry->number_length == 0 ||
             (number_length == entry->number_length &&
              binaryEqual (entry->number, number, (int)number_length))) &&
            (name_length == 0 || entry->name_length == 0 ||
             (entry->name_length == name_length &&
              binaryEqual (entry->name, name, (int)name_length))))
            break;
        XauDisposeAuth (entry);
    }
    (void) fclose (auth_file);
    return entry;
}

static int
read_short (shortp, file)
unsigned short  *shortp;
FILE            *file;
{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
        return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}

static int
read_counted_string (countp, stringp, file)
unsigned short  *countp;
char    **stringp;
FILE    *file;
{
    unsigned short  len;
    char            *data;

    if (read_short (&len, file) == 0)
        return 0;
    if (len == 0) {
        data = 0;
    } else {
        data = malloc ((unsigned) len);
        if (!data)
            return 0;
        if (fread (data, (int) sizeof (char), (int) len, file) != len) {
            bzero (data, len);
            free (data);
            return 0;
        }
    }
    *stringp = data;
    *countp = len;
    return 1;
}

Xauth *
XauReadAuth (auth_file)
FILE    *auth_file;
{
    Xauth   local;
    Xauth   *ret;

    if (read_short (&local.family, auth_file) == 0)
        return 0;
    if (read_counted_string (&local.address_length,
                             &local.address, auth_file) == 0)
        return 0;
    if (read_counted_string (&local.number_length,
                             &local.number, auth_file) == 0) {
        if (local.address) free (local.address);
        return 0;
    }
    if (read_counted_string (&local.name_length,
                             &local.name, auth_file) == 0) {
        if (local.address) free (local.address);
        if (local.number) free (local.number);
        return 0;
    }
    if (read_counted_string (&local.data_length,
                             &local.data, auth_file) == 0) {
        if (local.address) free (local.address);
        if (local.number) free (local.number);
        if (local.name) free (local.name);
        return 0;
    }
    ret = (Xauth *) malloc (sizeof (Xauth));
    if (!ret) {
        if (local.address) free (local.address);
        if (local.number) free (local.number);
        if (local.name) free (local.name);
        if (local.data) {
            bzero (local.data, local.data_length);
            free (local.data);
        }
        return 0;
    }
    *ret = local;
    return ret;
}

static int
write_short (s, file)
unsigned short  s;
FILE            *file;
{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;
    if (fwrite ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
        return 0;
    return 1;
}

static int
write_counted_string (count, string, file)
unsigned short  count;
char    *string;
FILE    *file;
{
    if (write_short (count, file) == 0)
        return 0;
    if (fwrite (string, (int) sizeof (char), (int) count, file) != count)
        return 0;
    return 1;
}

int
XauWriteAuth (auth_file, auth)
FILE    *auth_file;
Xauth   *auth;
{
    if (write_short (auth->family, auth_file) == 0)
        return 0;
    if (write_counted_string (auth->address_length,
                              auth->address, auth_file) == 0)
        return 0;
    if (write_counted_string (auth->number_length,
                              auth->number, auth_file) == 0)
        return 0;
    if (write_counted_string (auth->name_length, auth->name, auth_file) == 0)
        return 0;
    if (write_counted_string (auth->data_length, auth->data, auth_file) == 0)
        return 0;
    return 1;
}

#ifdef KRB5
#ifdef K5_XAUTH
/*
 * functions to encode/decode Kerberos V5 principals
 * into something that can be reasonable spewed over
 * the wire
 *
 * Author: Tom Yu <tlyu@MIT.EDU>
 *
 * Still needs to be fixed up wrt signed/unsigned lengths, but we'll worry
 * about that later.
 */

/*
 * XauKrb5Encode
 *
 * this function encodes the principal passed to it in a format that can
 * easily be dealt with by stuffing it into an X packet.  Encoding is as
 * follows:
 *   length count of the realm name
 *   realm
 *   component count
 *   length of component
 *   actual principal component
 *   etc....
 *
 * Note that this function allocates a hunk of memory, which must be
 * freed to avoid nasty memory leak type things.  All counts are
 * byte-swapped if needed. (except for the total length returned)
 *
 * nevermind.... stuffing the encoded packet in net byte order just to
 * always do the right thing.  Don't have to frob with alignment that way.
 */
int
XauKrb5Encode(princ, outbuf)
    krb5_principal princ;       /* principal to encode */
    krb5_data *outbuf;          /* output buffer */
{
    CARD16 i, numparts, totlen = 0, plen, rlen;
    char *cp, *pdata;

    rlen = krb5_princ_realm(princ)->length;
    numparts = krb5_princ_size(princ);
    totlen = 2 + rlen + 2;      /* include room for realm length
                                   and component count */
    for (i = 0; i < numparts; i++)
        totlen += krb5_princ_component(princ, i)->length + 2;
    /* add 2 bytes each time for length */
    if ((outbuf->data = (char *)malloc(totlen)) == NULL)
        return -1;
    cp = outbuf->data;
    *cp++ = (char)((int)(0xff00 & rlen) >> 8);
    *cp++ = (char)(0x00ff & rlen);
    memcpy(cp, krb5_princ_realm(princ)->data, rlen);    /* safe */
    cp += rlen;
    *cp++ = (char)((int)(0xff00 & numparts) >> 8);
    *cp++ = (char)(0x00ff & numparts);
    for (i = 0; i < numparts; i++)
    {
        plen = krb5_princ_component(princ, i)->length;
        pdata = krb5_princ_component(princ, i)->data;
        *cp++ = (char)((int)(0xff00 & plen) >> 8);
        *cp++ = (char)(0x00ff & plen);
        memcpy(cp, pdata, plen);                        /* safe */
        cp += plen;
    }
    outbuf->length = totlen;
    return 0;
}

/*
 * XauKrb5Decode
 *
 * This function essentially reverses what XauKrb5Encode does.
 * return value: 0 if okay, -1 if malloc fails, -2 if inbuf format bad
 */
int
XauKrb5Decode(inbuf, princ)
    krb5_data inbuf;
    krb5_principal *princ;
{
    CARD16 i, numparts, plen, rlen;
    CARD8 *cp, *pdata;

    if (inbuf.length < 4)
    {
        return -2;
    }
    *princ = (krb5_principal)malloc(sizeof (krb5_principal_data));
    if (*princ == NULL)
        return -1;
    bzero(*princ, sizeof (krb5_principal_data));
    cp = (CARD8 *)inbuf.data;
    rlen = *cp++ << 8;
    rlen |= *cp++;
    if (inbuf.length < 4 + (int)rlen + 2)
    {
        krb5_free_principal(*princ);
        return -2;
    }
    krb5_princ_realm(*princ)->data = (char *)malloc(rlen);
    if (krb5_princ_realm(*princ)->data == NULL)
    {
        krb5_free_principal(*princ);
        return -1;
    }
    krb5_princ_realm(*princ)->length = rlen;
    memcpy(krb5_princ_realm(*princ)->data, cp, rlen);   /* safe */
    cp += rlen;
    numparts = *cp++ << 8;
    numparts |= *cp++;
    krb5_princ_name(*princ) =
        (krb5_data *)malloc(numparts * sizeof (krb5_data));
    if (krb5_princ_name(*princ) == NULL)
    {
        krb5_free_principal(*princ);
        return -1;
    }
    krb5_princ_size(*princ) = 0;
    for (i = 0; i < numparts; i++)
    {
        if (cp + 2 > (CARD8 *)inbuf.data + inbuf.length)
        {
            krb5_free_principal(*princ);
            return -2;
        }
        plen = *cp++ << 8;
        plen |= *cp++;
        if (cp + plen > (CARD8 *)inbuf.data + inbuf.length)
        {
            krb5_free_principal(*princ);
            return -2;
        }
        pdata = (CARD8 *)malloc(plen);
        if (pdata == NULL)
        {
            krb5_free_principal(*princ);
            return -1;
        }
        krb5_princ_component(*princ, i)->data = (char *)pdata;
        krb5_princ_component(*princ, i)->length = plen;
        memcpy(pdata, cp, plen);        /* safe */
        cp += plen;
        krb5_princ_size(*princ)++;
    }
    return 0;
}
#endif /* K5_XAUTH */
#endif /* KRB5 */
#endif /* CK_FORWARD_X */
#endif /* CK_AUTHENTICATION */

/* C K _ A U T H _ I N I T
 * Initialize the Kerberos system for a pending connection
 *   hostname - a reverse DNS lookup of the hostname when possible
 *   ipaddr   - the ip address of the host
 *   username - the name the user wants to connect under not necessarily
 *              the same as principal
 *   socket   - the socket handle (ttyfd in Kermit speak)
 *
 * Returns: 1 on success and 0 on failure
 */

int
#ifdef CK_ANSIC
ck_auth_init( char * hostname, char * ipaddr, char * username, int socket )
#else /* CK_ANSIC */
ck_auth_init( hostname, ipaddr, username, socket )
    char * hostname; char * ipaddr; char *username; int socket;
#endif /* CK_ANSIC */
{
#ifdef CK_AUTHENTICATION
#ifdef OS2
    if ( !ck_security_loaddll() ) {
        TELOPT_ME_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_AUTHENTICATION) = TN_NG_RF;
        return(0);
    }
#endif /* OS2 */
#endif /* CK_AUTHENTICAITON */
#ifdef CK_ENCRYPTION
    if ( !!ck_crypt_is_installed() ) {
        TELOPT_ME_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
        TELOPT_U_MODE(TELOPT_ENCRYPTION) = TN_NG_RF;
    }
#endif /* CK_ENCRYPTION */

    if (!hostname) hostname = "";
    if (!ipaddr) ipaddr = "";
    if (!username) username = "";

    debug(F110,"ck_auth_init Username",username,0);
    debug(F110,"ck_auth_init Hostname",hostname,0);
    debug(F110,"ck_auth_init Ipaddr",ipaddr,0);

    ckstrncpy( szUserName, username, UIDBUFLEN );
    ckstrncpy( szHostName, hostname, UIDBUFLEN );
    ckstrncpy( szIP, ipaddr, 16 );
    szUserNameRequested[0] = '\0';
    szUserNameAuthenticated[0] = '\0';
    validUser = AUTH_REJECT;
    accept_complete = 0;
    authentication_version = AUTHTYPE_NULL;

#ifdef CK_AUTHENTICATION
    auth_how = 0;
    auth_crypt = 0;
    auth_fwd = 0;
    mutual_complete = 0;
    if ( sstelnet )
        str_data[3] = TELQUAL_REPLY;
    else
        str_data[3] = TELQUAL_IS;
#endif /* CK_AUTHENTICATION */

#ifdef CK_SRP
    srp_waitresp = 0;
#endif /* SRP */

#ifdef CK_KERBEROS
#ifdef KRB5
    /* free previous ret_cred  */
    if ( ret_cred ) {
#ifdef CK_ENCRYPTION
#ifdef HEIMDAL
        if ( k5_session_key == &ret_cred->session)
            k5_session_key = NULL;
#else /* HEIMDAL */
        if ( k5_session_key == &ret_cred->keyblock)
            k5_session_key = NULL;
#endif /* HEIMDAL */
#endif /* CK_ENCRYPTION */
        krb5_free_creds(k5_context, ret_cred);
        ret_cred = NULL;
    }
    if (k5_ticket) {
        krb5_free_ticket(k5_context, k5_ticket);
        k5_ticket = NULL;
    }
    /* and context */
    if ( k5_context ) {
        krb5_free_context(k5_context);
        k5_context = NULL;
    }

    /* create k5_context */
    krb5_init_context(&k5_context);
#ifndef MIT_CURRENT
#ifndef NO_KRB5_INIT_ETS
/* This routine is a no-op in Kerberos 1.4.x and later */
/* and in some installations it can't be found in which case */
/* define NO_KRB5_INIT_ETS */
    if (k5_context)
        krb5_init_ets(k5_context);
#endif	/* NO_KRB5_INIT_ETS */
#endif /* MIT_CURRENT */
#ifdef KRB524_CONV
    krb524_init_ets(k5_context);
#endif /* KRB524_CONV */
    memset(&k5_auth,0,sizeof(k5_auth));
    if (auth_context) {
        krb5_auth_con_free(k5_context, auth_context);
        auth_context = 0;
    }
#ifdef CK_ENCRYPTION
    if (k5_session_key) {
        krb5_free_keyblock(k5_context, k5_session_key);
        k5_session_key = 0;
    }
#endif /* ENCRYPTION */
#ifdef TLS_VERIFY
    krb5_tls_verified = 0;
#endif /* TLS_VERIFY */
#endif /* KRB5 */

#ifdef KRB4
#ifdef CK_ENCRYPTION
    /* Initialize buffers used for authentication */
    memset(&k4_session_key, 0, sizeof(k4_session_key));
    memset(&k4_challenge, 0, sizeof(k4_challenge));
#endif /* CK_ENCRYPTION */
#endif /* KRB4 */

#ifdef RLOGCODE
    rlog_encrypt = 0;
#endif /* RLOGCODE */
    nstored = 0;
    store_ptr = storage;
    memset(storage,0,sizeof(storage));
#endif /* CK_KERBEROS */

#ifdef CK_ENCRYPTION
    kstream_destroy();
    if (!kstream_create_from_fd(socket, NULL))
        return(0);
#endif /* CK_ENCRYPTION */
    return(1);
}

void
auth_finished(result) int result; {
    extern char uidbuf[];
    extern int sstelnet;

    validUser = result;
    switch (result) {
    case AUTH_REJECT:           /* Rejected */
        if (sstelnet)
            uidbuf[0] = '\0';
        authentication_version = AUTHTYPE_NULL;
        break;
    case AUTH_UNKNOWN:          /* We don't know who he is, but he's okay */
        if (sstelnet)
            strcpy(uidbuf,"(unknown)");
        break;
    case AUTH_OTHER:            /* We know him, but not his name */
        if (sstelnet)
            strcpy(uidbuf,"(other)");
        break;
    case AUTH_USER:             /* We know he name */
    case AUTH_VALID:            /* We know him, and he needs no password */
        if (sstelnet)
            strcpy(uidbuf,szUserNameRequested);
        break;
    }
}

#ifdef MACOSX
#ifdef KRB5

krb5_error_code
ck_krb5_write_message(krb5_context con, krb5_pointer ptr, krb5_data *data)
{
    int fd = *((int *)ptr);
    long msglen;

    msglen = htonl(data->length);
    if (net_write(fd,(CHAR *)&msglen,4) != 4) {
        return(-1);
    }
    if ( data->length ) {
        if (net_write(fd,data->data,data->length) != data->length) {
            return(-1);
        }
    }
    return(0);
}

krb5_error_code
ck_krb5_read_message( krb5_context context,
                      krb5_pointer ptr,
                      krb5_data * data)
{
    extern int ttyfd;
    int fd = *((int *)ptr);
    long msglen;
    char *p;
    int i, rc;

    if (net_read(fd,&msglen,4) < 0)
        return(-1);

    data->length = ntohl(msglen);
    if ( data->length ) {
        data->data = malloc(data->length);

        i = 0;
        while ( i < data->length ) {
            if ((rc = net_read(fd,&data->data[i],(data->length - i))) < 0)
                return(-1);
            i += rc;
        }
    }
    return(0);
}
#endif /* KRB5 */
#endif /* MACOSX */
#endif /* CK_SECURITY */


