char *ckcrpv = "Encryption Engine, 9.0.117, 19 Mar 2010";
/*
  C K _ C R P . C  -  Cryptography for C-Kermit"

  Copyright (C) 1998, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Author:
  Jeffrey E Altman (jaltman@secure-endpoints.com)
  Secure Endpoints Inc., New York City
*/

#define CK_CRP_C
#ifdef CK_DES
#ifdef CK_SSL
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */
#endif /* CK_SSL */
#endif /* CK_DES */

#ifdef CRYPT_DLL
#define CK_AUTHENTICATION
#define CK_ENCRYPTION
#define CK_DES
#define CK_CAST
#ifndef LIBDES
#define LIBDES
#endif /* LIBDES */

#define TELCMDS                         /* to define name array */
#define TELOPTS                         /* to define name array */
#define ENCRYPT_NAMES
#endif /* CRYPT_DLL */

#include "ckcsym.h"
#include "ckcdeb.h"
#include "ckcnet.h"

#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */

#ifdef CK_AUTHENTICATION
#ifdef CK_ENCRYPTION
#define ENCRYPTION
#ifdef CK_DES
#define DES_ENCRYPTION
#endif /* CK_DES */
#ifdef  CK_CAST
#define CAST_ENCRYPTION
#endif /* CK_CAST */
#ifdef COMMENT
#define CAST_EXPORT_ENCRYPTION
#endif /* COMMENT */
#endif /* CK_ENCRYPTION */
#endif /* CK_AUTHENTICATION */

#ifdef CK_ENCRYPTION

#include "ckucmd.h"                     /* For struct keytab definition */
#include "ckuath.h"
#include "ckuat2.h"
#ifdef MIT_CURRENT
#include <krb5.h>
#endif /* MIT_CURRENT */

#include <stdlib.h>
#include <string.h>
#ifdef OS2
#include <stdarg.h>
#ifdef OS2ONLY
#include <os2.h>
#endif /* OS2ONLY */
#include "ckosyn.h"
#else /* OS2 */
static char * tmpstring = NULL;
#endif /* OS2 */

#ifndef CAST_OR_EXPORT
#ifdef CAST_ENCRYPTION
#define CAST_OR_EXPORT
#endif /* CAST_ENCRYPTION */
#ifdef CAST_EXPORT_ENCRYPTION
#define CAST_OR_EXPORT
#endif /* CAST_EXPORT_ENCRYPTION */
#endif /* CAST_OR_EXPORT */

#ifdef MACOSX
#undef LIBDES
#endif /* MACOSX */

#ifdef CRYPT_DLL
int cmd_quoting = 0;

#ifndef TELOPT_MACRO
int
telopt_index(opt) int opt; {
    if ( opt >= 0 && opt <= TELOPT_SEND_URL )
        return(opt);
    else if ( opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT )
        return(opt-89);
    else
        return(NTELOPTS);
}

int
telopt_ok(opt) int opt; {
    return((opt >= TELOPT_BINARY && opt <= TELOPT_SEND_URL) ||
        (opt >= TELOPT_PRAGMA_LOGON && opt <= TELOPT_PRAGMA_HEARTBEAT));
}

CHAR *
telopt(opt) int opt; {
    if ( telopt_ok(opt) )
        return(telopts[telopt_index(opt)]);
    else
        return("UNKNOWN");
}
#endif /* TELOPT_MACRO */

static int (*p_ttol)(char *,int)=NULL;
static int (*p_dodebug)(int,char *,char *,CK_OFF_T)=NULL;
static int (*p_dohexdump)(char *,char *,int)=NULL;
static void (*p_tn_debug)(char *)=NULL;
static int (*p_vscrnprintf)(char *, ...)=NULL;
static void * p_k5_context=NULL;
static unsigned long (*p_reqtelmutex)(unsigned long)=NULL;
static unsigned long (*p_reltelmutex)(void)=NULL;

unsigned long
RequestTelnetMutex(unsigned long x)
{
    if ( p_reqtelmutex )
        return p_reqtelmutex(x);
    return 0;
}

unsigned long
ReleaseTelnetMutex(void)
{
    if ( p_reltelmutex )
        return p_reltelmutex();
    return 0;
}

int
ttol(char * s, int n)
{
    if ( p_ttol )
        return(p_ttol(s,n));
    else
        return(-1);
}

int
dodebug(int flag, char * s1, char * s2, CK_OFF_T n)
{
    if ( p_dodebug )
        return(p_dodebug(flag,s1,s2,n));
    else
        return(-1);
}

int
dohexdump( char * s1, char * s2, int n )
{
    if ( p_dohexdump )
        p_dohexdump(s1,s2,n);
    return(0);
}

void
tn_debug( char * s )
{
    if ( p_tn_debug )
        p_tn_debug(s);
}

static char myprtfstr[4096];
int
Vscrnprintf(const char * format, ...) {
    int i, len, rc=0;
    char *cp;
    va_list ap;

    va_start(ap, format);
#ifdef NT
    rc = _vsnprintf(myprtfstr, sizeof(myprtfstr)-1, format, ap);
#else /* NT */
    rc = vsprintf(myprtfstr, format, ap);
#endif /* NT */
    va_end(ap);

    if ( p_vscrnprintf )
        return(p_vscrnprintf(myprtfstr));
    else
        return(-1);
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
    CHAR tmp[8];
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
        ckstrncat(buf,tmp,buflen);
    }
    if (!was_hex)
        ckstrncat(buf,"\" ",buflen);
#else /* COMMENT */
    if (datalen <= 0 || data == NULL)
        return(0);

    for (i = 0; i < datalen; i++) {
        ckstrncat(buf,"\r\n  ",buflen);
        for (j = 0 ; (j < 16); j++) {
            if ((i + j) < datalen)
              sprintf(tmp,
                      "%s%02x ",
                      (j == 8 ? "| " : ""),
                      (CHAR) data[i + j]
                      );
            else
              sprintf(tmp,
                      "%s   ",
                      (j == 8 ? "| " : "")
                      );
            ckstrncat(buf,tmp,buflen);
        }
        ckstrncat(buf," ",buflen);
        for (k = 0; (k < 16) && ((i + k) < datalen); k++) {
            sprintf(tmp,
                     "%s%c",
                     (k == 8 ? " " : ""),
                     isprint(data[i + k]) ? data[i + k] : '.'
                     );
            ckstrncat(buf,tmp,buflen);
        }
        i += j - 1;
    } /* end for */
    ckstrncat(buf,"\r\n  ",buflen);
#endif /* COMMENT */
    return(strlen(buf));
}

#ifdef COMMENT
#define ttol        dll_ttol
#define dodebug     dll_dodebug
#define dohexdump   dll_dohexdump
#define tn_debug    dll_tn_debug
#define Vscrnprintf dll_vscrnprintf
#endif /* COMMENT */

char tn_msg[TN_MSG_LEN], hexbuf[TN_MSG_LEN];   /* from ckcnet.c */
int deblog=1, debses=1, tn_deb=1;
#else /* CRYPT_DLL */
extern char tn_msg[], hexbuf[];         /* from ckcnet.c */
extern int deblog, debses, tn_deb;
#ifdef MIT_CURRENT
extern krb5_context k5_context;
#endif /* MIT_CURRENT */
#endif /* CRYPT_DLL */

#ifdef LIBDES
#ifdef MACOSX
#define des_new_random_key            ck_des_new_random_key
#define des_set_random_generator_seed ck_des_set_random_generator_seed
#define des_key_sched                 ck_des_key_sched
#define des_ecb_encrypt               ck_des_ecb_encrypt
#define des_string_to_key             ck_des_string_to_key
#define des_fixup_key_parity          ck_des_fixup_key_parity
#endif /* MACOSX */
#ifndef UNIX
#define des_new_random_key            des_random_key
#define des_set_random_generator_seed des_random_seed
#endif /* UNIX */
#define des_fixup_key_parity          des_set_odd_parity
#ifdef OPENSSL_097
#define OPENSSL_ENABLE_OLD_DES_SUPPORT
#include <openssl/des.h>
#endif /* OPENSSL_097 */
#else /* LIBDES */
#ifdef UNIX
#define des_set_random_generator_seed(x) des_init_random_number_generator(x)
#endif /* UNIX */
#ifdef OS2
#define des_new_random_key            ck_des_new_random_key
#define des_set_random_generator_seed ck_des_set_random_generator_seed
#define des_key_sched                 ck_des_key_sched
#define des_ecb_encrypt               ck_des_ecb_encrypt
#define des_string_to_key             ck_des_string_to_key
#define des_fixup_key_parity          ck_des_fixup_key_parity
#endif /* OS2 */
#endif /* LIBDES */

#ifdef CK_DES
/* This code comes from Eric Young's libdes package and is not part   */
/* of the standard MIT DES library that is part of Kerberos. However, */
/* it is extremely useful.  So we add it here.                        */


/* Weak and semi week keys as take from
 * %A D.W. Davies
 * %A W.L. Price
 * %T Security for Computer Networks
 * %I John Wiley & Sons
 * %D 1984
 * Many thanks to smb@ulysses.att.com (Steven Bellovin) for the reference
 * (and actual cblock values).
 */
#define NUM_WEAK_KEY    16
static Block weak_keys[NUM_WEAK_KEY]={
        /* weak keys */
        {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
        {0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE},
        {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
        {0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0},
        /* semi-weak keys */
        {0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE},
        {0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01},
        {0x1F,0xE0,0x1F,0xE0,0x0E,0xF1,0x0E,0xF1},
        {0xE0,0x1F,0xE0,0x1F,0xF1,0x0E,0xF1,0x0E},
        {0x01,0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1},
        {0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,0x01},
        {0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,0xFE},
        {0xFE,0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E},
        {0x01,0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E},
        {0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,0x01},
        {0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE},
        {0xFE,0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1}};

int
ck_des_is_weak_key(key)
Block key;
{
    int i;

    for (i=0; i<NUM_WEAK_KEY; i++) {
        /* Added == 0 to comparision, I obviously don't run
        * this section very often :-(, thanks to
        * engineering@MorningStar.Com for the fix
        * eay 93/06/29
        * Another problem, I was comparing only the first 4
        * bytes, 97/03/18 */
        if (memcmp(weak_keys[i],key,sizeof(Block)) == 0)
            return(1);
    }
    return(0);
}

#ifdef UNIX
#ifdef LIBDES
#ifndef MACOSX
/* These functions are not part of Eric Young's DES library */
/* _unix_time_gmt_unixsec                                  */
/* _des_set_random_generator_seed                          */
/* _des_fixup_key_parity   (added in 0.9.5)                */
/* _des_new_random_key                                     */
#include <sys/time.h>

unsigned long
unix_time_gmt_unixsec (usecptr)
    unsigned long  *usecptr;
{
    struct timeval  now;

    (void) gettimeofday (&now, (struct timezone *)0);
    if (usecptr)
        *usecptr = now.tv_usec;
    return now.tv_sec;
}

void
des_set_random_generator_seed(Block B)
{
    des_random_seed(B);
    return;
}

#ifdef COMMENT
/* added to openssl in 0.9.5 */
void
des_fixup_key_parity(Block B)
{
    des_set_odd_parity(B);
    return;
}
#endif /* COMMENT */
int
des_new_random_key(Block B)
{
    int rc=0;
    /* WARNING:
       This might need to have the "rc = " removed because this
       is VOID in later, and maybe even all, versions.
    */       
    rc = des_random_key(B);
    return(rc);
}

#endif /* MACOSX */
#endif /* LIBDES */
#endif /* UNIX */
#endif /* CK_DES */

/*
 * Copyright (c) 1991, 1993
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
 */

/* based on @(#)encrypt.c       8.1 (Berkeley) 6/4/93 */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America may
 * require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
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
 */

#include <stdio.h>

/*
 * These function pointers point to the current routines
 * for encrypting and decrypting data.
 */
/* NOTE: These next two might need to have the "static " removed */

static VOID     (*encrypt_output) P((unsigned char *, int));
static int      (*decrypt_input) P((int));

#ifdef DEBUG
static int encrypt_debug_mode = 1;
static int encrypt_verbose = 1;
#else
static int encrypt_verbose = 1;
static int encrypt_debug_mode = 0;
#endif

static char dbgbuf [16384];

static int decrypt_mode = 0;
static int encrypt_mode = 0;
static int autoencrypt = 1;
static int autodecrypt = 1;
static int havesessionkey = 0;

static kstream EncryptKSGlobalHack = NULL;
static int     EncryptType = ENCTYPE_ANY;

#define typemask(x)     ((x) > 0 ? 1 << ((x)-1) : 0)

static long i_support_encrypt =
        typemask(ENCTYPE_DES_CFB64) | typemask(ENCTYPE_DES_OFB64);
static long i_support_decrypt =
        typemask(ENCTYPE_DES_CFB64) | typemask(ENCTYPE_DES_OFB64);
static long i_wont_support_encrypt = 0;
static long i_wont_support_decrypt = 0;
#define I_SUPPORT_ENCRYPT       (i_support_encrypt & ~i_wont_support_encrypt)
#define I_SUPPORT_DECRYPT       (i_support_decrypt & ~i_wont_support_decrypt)

static long remote_supports_encrypt = 0;
static long remote_supports_decrypt = 0;

/* Make sure that this list is in order of algorithm strength     */
/* as it determines the search order for selecting specific       */
/* encryption choices.  All CFB modes must come before OFB modes. */
static Encryptions encryptions[] = {
#ifdef DES_ENCRYPTION
    { "DES3_CFB64",
          ENCTYPE_DES3_CFB64,
          des3_cfb64_encrypt,
          des3_cfb64_decrypt,
          des3_cfb64_init,
          des3_cfb64_start,
          des3_cfb64_is,
          des3_cfb64_reply,
          des3_cfb64_session,
          des3_cfb64_keyid,
          NULL },
#endif /* DES_ENCRYPTION */
#ifdef CAST_ENCRYPTION
#ifndef CAST_EXPORT_ENCRYPTION
    { "CAST128_CFB64",  ENCTYPE_CAST128_CFB64,
          cast_cfb64_encrypt,
          cast_cfb64_decrypt,
          cast_cfb64_init,
          cast_cfb64_start,
          cast_cfb64_is,
          cast_cfb64_reply,
          cast_cfb64_session,
          cast_cfb64_keyid,
          NULL },
#endif
#endif
#ifdef DES_ENCRYPTION
    { "DES_CFB64",
          ENCTYPE_DES_CFB64,
          cfb64_encrypt,
          cfb64_decrypt,
          cfb64_init,
          cfb64_start,
          cfb64_is,
          cfb64_reply,
          cfb64_session,
          cfb64_keyid,
          NULL },
#endif  /* DES_ENCRYPTION */
#if defined (CAST_EXPORT_ENCRYPTION) || defined(CAST_ENCRYPTION)
    { "CAST5_40_CFB64", ENCTYPE_CAST5_40_CFB64,
          castexp_cfb64_encrypt,
          castexp_cfb64_decrypt,
          castexp_cfb64_init,
          castexp_cfb64_start,
          castexp_cfb64_is,
          castexp_cfb64_reply,
          castexp_cfb64_session,
          castexp_cfb64_keyid,
          NULL },
#endif /* CAST_ENCRYPTION */
#ifdef DES_ENCRYPTION
    { "DES3_OFB64",
          ENCTYPE_DES3_OFB64,
          des3_ofb64_encrypt,
          des3_ofb64_decrypt,
          des3_ofb64_init,
          des3_ofb64_start,
          des3_ofb64_is,
          des3_ofb64_reply,
          des3_ofb64_session,
          des3_ofb64_keyid,
          NULL },
#endif /* DES_ENCRYPTION */
#ifdef CAST_ENCRYPTION
#ifndef CAST_EXPORT_ENCRYPTION
    { "CAST128_OFB64",  ENCTYPE_CAST128_OFB64,
          cast_ofb64_encrypt,
          cast_ofb64_decrypt,
          cast_ofb64_init,
          cast_ofb64_start,
          cast_ofb64_is,
          cast_ofb64_reply,
          cast_ofb64_session,
          cast_ofb64_keyid,
          NULL },
#endif
#endif
#ifdef DES_ENCRYPTION
    { "DES_OFB64",
          ENCTYPE_DES_OFB64,
          ofb64_encrypt,
          ofb64_decrypt,
          ofb64_init,
          ofb64_start,
          ofb64_is,
          ofb64_reply,
          ofb64_session,
          ofb64_keyid,
          NULL },
#endif  /* DES_ENCRYPTION */
#if defined (CAST_EXPORT_ENCRYPTION) || defined(CAST_ENCRYPTION)
    { "CAST5_40_OFB64", ENCTYPE_CAST5_40_OFB64,
          castexp_ofb64_encrypt,
          castexp_ofb64_decrypt,
          castexp_ofb64_init,
          castexp_ofb64_start,
          castexp_ofb64_is,
          castexp_ofb64_reply,
          castexp_ofb64_session,
          castexp_ofb64_keyid,
          NULL },
#endif /* CAST_ENCRYPTION */
    { 0,0,0,0,0,0,0,0,0,0,0  }
};

int
get_crypt_table( struct keytab ** pTable, int * pN )
{
    int i=0,n=0;

    if ( *pTable )
    {
        for ( i=0 ; i < *pN ; i++ )
            free( (*pTable)[i].kwd ) ;
        free ( *pTable )  ;
    }
    *pTable = NULL;
    *pN = 0;

    /* How many encryption types do we have? */
    while ( encryptions[n].name )
        n++;

    if ( n )
    {
        *pTable = malloc( sizeof(struct keytab) * (n+2) ) ;
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

        for ( i=0 ; i < n ; i++ ) {
            char * newstr = NULL, * p;
            int newval = encryptions[i].type;
            int j = 0, len = 0;

#ifdef OS2
            newstr = strdup(encryptions[i].name);
            strlwr(newstr);
#else /* OS2 */
            makestr(&tmpstring,encryptions[i].name);
            newstr = tmpstring;
            tmpstring = NULL;
            for (p = newstr; *p; p++) if (isupper(*p)) *p = tolower(*p);
#endif /* OS2 */

            for (j = 0; j < (*pN); j++) {
                int tempval = 0;
                char * tempstr = NULL;

                if ( strcmp( (*pTable)[j].kwd, newstr ) > 0 )
                {
                    tempval = (*pTable)[j].kwval;
                    tempstr = (*pTable)[j].kwd;
                    (*pTable)[j].kwd = newstr ;
                    (*pTable)[j].kwval = newval;
                    newval = tempval;
                    newstr = tempstr;
                    (*pTable)[j].flgs = 0;
                }
            }
            (*pTable)[*pN].kwd = newstr ;
            (*pTable)[*pN].kwval = newval;
            (*pTable)[*pN].flgs = 0 ;
            (*pN)++ ;
        }
    } else {
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
    }
    return(*pN);
}

static unsigned char str_send[64] = { IAC, SB, TELOPT_ENCRYPTION,
                                      ENCRYPT_SUPPORT };
static unsigned char str_suplen = 0;
static unsigned char str_start[72] = { IAC, SB, TELOPT_ENCRYPTION };
static unsigned char str_end[] = { IAC, SB, TELOPT_ENCRYPTION, 0, IAC, SE };

_PROTOTYP(int  encrypt_request_end, (VOID));
_PROTOTYP(int  encrypt_request_start, (VOID));
_PROTOTYP(int  encrypt_enc_keyid, (unsigned char *, int));
_PROTOTYP(int  encrypt_dec_keyid, (unsigned char *, int));
_PROTOTYP(int  encrypt_support, (unsigned char *, int));
_PROTOTYP(int  encrypt_start, (unsigned char *, int));
_PROTOTYP(int  encrypt_end, (VOID));

_PROTOTYP(int encrypt_ks_stream,(struct kstream_data_block *, /* output */
                                   struct kstream_data_block *)); /* input */

_PROTOTYP(int decrypt_ks_stream,(struct kstream_data_block *, /* output */
                                   struct kstream_data_block *)); /* input */

int
#ifdef CK_ANSIC
encrypt_ks_stream(struct kstream_data_block *i,
                  struct kstream_data_block *o)
#else
encrypt_ks_stream(i,o)
    struct kstream_data_block *i; struct kstream_data_block *o;
#endif
{
    /*
    * this is really quite bogus, since it does an in-place encryption...
    */
    if (encrypt_output) {
        encrypt_output(i->ptr, i->length);
        return 1;
    }
    return 0;
}


int
#ifdef CK_ANSIC
decrypt_ks_stream(struct kstream_data_block *i,
                  struct kstream_data_block *o)
#else
decrypt_ks_stream(i,o)
    struct kstream_data_block *i; struct kstream_data_block *o;
#endif
{
    unsigned int len;
  /*
   * this is really quite bogus, since it does an in-place decryption...
   */
    if (decrypt_input) {
        for (len = 0 ; len < i->length ; len++)
            ((unsigned char *)i->ptr)[len]
                = decrypt_input(((unsigned char *)i->ptr)[len]);
        return 1;
    }
    return 0;
}

int
#ifdef CK_ANSIC
decrypt_ks_hack(unsigned char *buf, int cnt)
#else
decrypt_ks_hack(buf,cnt) unsigned char *buf; int cnt;
#endif
{
    int len;
  /*
   * this is really quite bogus, since it does an in-place decryption...
   */
    for (len = 0 ; len < cnt ; len++)
        buf[len] = decrypt_input(buf[len]);

#ifdef DEBUG
    ckhexdump("decrypt ks hack", buf, cnt);
#endif
    return 1;
}


/*
 * parsedat[0] == the suboption we might be negotiating,
 */
int
#ifdef CK_ANSIC
encrypt_parse(unsigned char *parsedat, int end_sub)
#else
encrypt_parse(parsedat,end_sub) unsigned char *parsedat; int end_sub;
#endif
{
    int rc = 0;

    switch(parsedat[1]) {
    case ENCRYPT_START:
        rc = encrypt_start(parsedat + 2, end_sub - 2);
        break;
    case ENCRYPT_END:
        rc = encrypt_end();
        break;
    case ENCRYPT_SUPPORT:
        rc = encrypt_support(parsedat + 2, end_sub - 2);
        break;
    case ENCRYPT_REQSTART:
        rc = encrypt_request_start();
        break;
    case ENCRYPT_REQEND:
        /*
        * We can always send an REQEND so that we cannot
        * get stuck encrypting.  We should only get this
        * if we have been able to get in the correct mode
        * anyhow.
        */
        rc = encrypt_request_end();
        break;
    case ENCRYPT_IS:
        rc = encrypt_is(parsedat + 2, end_sub - 2);
        break;
    case ENCRYPT_REPLY:
        rc = encrypt_reply(parsedat + 2, end_sub - 2);
        break;
    case ENCRYPT_ENC_KEYID:
        rc = encrypt_enc_keyid(parsedat + 2, end_sub - 2);
        break;
    case ENCRYPT_DEC_KEYID:
        rc = encrypt_dec_keyid(parsedat + 2, end_sub - 2);
        break;
    default:
        rc = -1;
        break;
    }
    return(rc);
}

/* XXX */
Encryptions *
#ifdef CK_ANSIC
findencryption(int type)
#else
findencryption(type) int type;
#endif
{
    Encryptions *ep = encryptions;

    if (!(I_SUPPORT_ENCRYPT & remote_supports_decrypt & typemask(type)))
        return(0);
    while (ep->type && ep->type != type)
        ++ep;
    return(ep->type ? ep : 0);
}

Encryptions *
#ifdef CK_ANSIC
finddecryption(int type)
#else
finddecryption(type) int type;
#endif
{
    Encryptions *ep = encryptions;

    if (!(I_SUPPORT_DECRYPT & remote_supports_encrypt & typemask(type)))
        return(0);
    while (ep->type && ep->type != type)
        ++ep;
    return(ep->type ? ep : 0);
}

#define MAXKEYLEN 64

static struct key_info {
    unsigned char keyid[MAXKEYLEN];
    int keylen;
    int dir;
    int *modep;
    Encryptions *(*getcrypt)();
} ki[2] = {
    { { 0 }, 0, DIR_ENCRYPT, &encrypt_mode, findencryption },
    { { 0 }, 0, DIR_DECRYPT, &decrypt_mode, finddecryption },
};

VOID
#ifdef CK_ANSIC
encrypt_init(kstream iks, int type)
#else
encrypt_init(iks, type) kstream iks; int type;
#endif
{
    Encryptions *ep = encryptions;

    i_support_encrypt = i_support_decrypt = 0;
    remote_supports_encrypt = remote_supports_decrypt = 0;
    i_wont_support_encrypt = i_wont_support_decrypt = 0;
    encrypt_mode = 0;
    decrypt_mode = 0;
    encrypt_output = NULL;
    decrypt_input = NULL;
    ki[0].keylen = 0;
    memset(ki[0].keyid,0,MAXKEYLEN);
    ki[1].keylen = 0;
    memset(ki[1].keyid,0,MAXKEYLEN);
    havesessionkey = 0;
    autoencrypt = 1;
    autodecrypt = 1;

    EncryptKSGlobalHack = iks;
    EncryptType = type;

    str_send[0] = IAC;
    str_send[1] = SB;
    str_send[2] = TELOPT_ENCRYPTION;
    str_send[3] = ENCRYPT_SUPPORT;
    str_suplen = 4;

    while (ep->type) {
        if ( EncryptType == ENCTYPE_ANY ||
             EncryptType == ep->type ) {
#ifdef DEBUG
            if (encrypt_debug_mode) {
                sprintf(dbgbuf, ">>>I will support %s\n",
                         ENCTYPE_NAME(ep->type));       /* safe */
                debug(F110,"encrypt_init",dbgbuf,0);
            }
#endif
            i_support_encrypt |= typemask(ep->type);
            i_support_decrypt |= typemask(ep->type);
            if ((i_wont_support_decrypt & typemask(ep->type)) == 0)
                if ((str_send[str_suplen++] = ep->type) == IAC)
                    str_send[str_suplen++] = IAC;
        }
        if (ep->init)
            (*ep->init)(0);
        ++ep;
    }
    str_send[str_suplen++] = IAC;
    str_send[str_suplen++] = SE;
}

VOID
#ifdef CK_ANSIC
encrypt_send_support(VOID)
#else
encrypt_send_support()
#endif
{
    Encryptions *ep = encryptions;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return;
#endif /* CK_SSL */

    str_send[0] = IAC;
    str_send[1] = SB;
    str_send[2] = TELOPT_ENCRYPTION;
    str_send[3] = ENCRYPT_SUPPORT;
    str_suplen = 4;

    while (ep->type) {
        if ( EncryptType == ENCTYPE_ANY ||
             EncryptType == ep->type ) {
#ifdef DEBUG
            if (encrypt_debug_mode) {
                sprintf(dbgbuf, ">>>I will support %s\n",
                         ENCTYPE_NAME(ep->type));               /* safe */
                debug(F110,"encrypt_send_support",dbgbuf,0);
            }
#endif
            if ((i_wont_support_decrypt & typemask(ep->type)) == 0)
                if ((str_send[str_suplen++] = ep->type) == IAC)
                    str_send[str_suplen++] = IAC;
        }
        ++ep;
    }
    str_send[str_suplen++] = IAC;
    str_send[str_suplen++] = SE;

    /*
    * If the user has requested that decryption start
    * immediatly, then send a "REQUEST START" before
    * we negotiate the type.
    */
    if (autodecrypt)
        encrypt_send_request_start();

    if (deblog || tn_deb || debses) {
        int i;
        sprintf(tn_msg,"TELNET SENT SB %s SUPPORT ",
                 TELOPT(TELOPT_ENCRYPTION));                    /* safe */
        for ( i=4;i<str_suplen-2;i++ ) {
            if ( str_send[i] == IAC ) {
                ckstrncat(tn_msg,"IAC ",TN_MSG_LEN);
                i++;
            }
            ckstrncat(tn_msg,ENCTYPE_NAME(str_send[i]),TN_MSG_LEN);
            ckstrncat(tn_msg," ",TN_MSG_LEN);
        }
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol(str_send, str_suplen);
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    str_suplen = 0;
}

/*
 * Called when ENCRYPT SUPPORT is received.
 */
int
#ifdef CK_ANSIC
encrypt_support(unsigned char *_typelist, int _cnt)
#else
encrypt_support(_typelist, _cnt) unsigned char * _typelist; int _cnt;
#endif
{
    register int type, use_type = 0;
    unsigned char * typelist = _typelist;
    int cnt = _cnt;
    Encryptions *ep;

    debug(F111,"encrypt_support","cnt",cnt);

  /*
   * Forget anything the other side has previously told us.
   */
    remote_supports_decrypt = 0;

    while (cnt-- > 0) {
	debug(F101,"XXX cnt","",cnt);
        type = *typelist++;
	debug(F101,"XXX type","",type);
	debug(F101,"XXX ENCTYPE_ANY","",ENCTYPE_ANY);
        if ( EncryptType == ENCTYPE_ANY ||
             EncryptType == type ) {
#ifdef DEBUG
            if (encrypt_debug_mode) {
                sprintf(dbgbuf, ">>>Remote supports %s (%d)\n",
                         ENCTYPE_NAME(type), type);             /* safe */
                debug(F110,"encrypt_support",dbgbuf,0);
            }
#endif
            if ((type < ENCTYPE_CNT) &&
                 (I_SUPPORT_ENCRYPT & typemask(type))) {
                remote_supports_decrypt |= typemask(type);
                if (use_type == 0)
                    use_type = type;
            }
        }
    }
    debug(F101,"XXX use_type","",use_type);
    if (use_type) {
        ep = findencryption(use_type);
        if (!ep) {
            debug(F111,"encrypt_support","findencryption == NULL",use_type);
            return(-1);
        }
	debug(F100,"XXX ep not NULL","",0);
        type = ep->start ? (*ep->start)(DIR_ENCRYPT, 0) : 0;
	debug(F101,"XXX new type","",type);
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>(*ep->start)() %s returned %d (%s)\n",
                     ENCTYPE_NAME(use_type), type,
                     ENCRYPT_NAME(type));                       /* safe */
            debug(F110,"encrypt_support",dbgbuf,0);
        }
#endif
        if (type < 0) {
            debug(F111,"encrypt_support","type < 0",type);
            return(-1);
        }
        encrypt_mode = use_type;
        if (type == 0)
            encrypt_start_output(use_type);
        debug(F111,"encrypt_support","success",type);
        return(0);
    }
    debug(F111,"encrypt_support","failed",use_type);
    return(-1);
}

int
#ifdef CK_ANSIC
encrypt_is(unsigned char *data, int cnt)
#else
encrypt_is(data, cnt) unsigned char *data; int cnt;
#endif /* CK_ANSIC */
{
    Encryptions *ep;
    register int type, ret;

    if (--cnt < 0)
        return(-1);
    type = *data++;
    if (type < ENCTYPE_CNT)
        remote_supports_encrypt |= typemask(type);
    if (!(ep = finddecryption(type))) {
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>encrypt_is:  "
                     "Can't find type %s (%d) for initial negotiation\n",
                     ENCTYPE_NAME_OK(type)
                     ? ENCTYPE_NAME(type) : "(unknown)",
                     type);                                     /* safe */
            debug(F110,"encrypt_is",dbgbuf,0);
        }
#endif
        return(-1);
    }
    if (!ep->is) {
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>encrypt_is:  "
                     "No initial negotiation needed for type %s (%d)\n",
                     ENCTYPE_NAME_OK(type)
                     ? ENCTYPE_NAME(type) : "(unknown)",
                     type);                                     /* safe */
            debug(F110,"encrypt_is",dbgbuf,0);
        }
#endif
        ret = 0;
    } else {
        ret = (*ep->is)(data, cnt);
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, "encrypt_is:  "
                     "(*ep->is)(%x, %d) returned %s(%d)\n", data, cnt,
                     (ret < 0) ? "FAIL " :
                     (ret == 0) ? "SUCCESS " : "MORE_TO_DO ", ret); /* safe */
            debug(F110,"encrypt_is",dbgbuf,0);
        }
#endif
    }
    if (ret < 0) {
        autodecrypt = 0;
        return(-1);
    } else {
        decrypt_mode = type;
        if (ret == 0 && autodecrypt) {
            encrypt_send_request_start();
        }
    }
    return(0);
}

int
#ifdef CK_ANSIC
encrypt_reply(unsigned char *data, int cnt)
#else
encrypt_reply(data, cnt) unsigned char *data; int cnt;
#endif
{
    Encryptions *ep;
    register int ret, type;

    if (--cnt < 0)
        return(-1);
    type = *data++;
    if (!(ep = findencryption(type))) {
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf,
                    ">>>Can't find type %s (%d) for initial negotiation\n",
                     ENCTYPE_NAME_OK(type)
                     ? ENCTYPE_NAME(type) : "(unknown)",
                     type);                                     /* safe */
            debug(F110,"encrypt_reply",dbgbuf,0);
        }
#endif
        return(-1);
    }
    if (!ep->reply) {
#ifdef DEBUG
        if (encrypt_debug_mode) {
      sprintf(dbgbuf, ">>>No initial negotiation needed for type %s (%d)\n",
               ENCTYPE_NAME_OK(type)
               ? ENCTYPE_NAME(type) : "(unknown)",
               type);                                           /* safe */
            debug(F110,"encrypt_reply",dbgbuf,0);
        }
#endif
        ret = 0;
    } else {
        ret = (*ep->reply)(data, cnt);
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, "(*ep->reply)(%x, %d) returned %s(%d)\n",
                     data, cnt,
                     (ret < 0) ? "FAIL " :
                     (ret == 0) ? "SUCCESS " : "MORE_TO_DO ", ret); /* safe */
            debug(F110,"encrypt_reply",dbgbuf,0);
        }
#endif
    }
#ifdef DEBUG
    if (encrypt_debug_mode) {
        sprintf(dbgbuf, ">>>encrypt_reply returned %d\n", ret); /* safe */
        debug(F110,"encrypt_reply",dbgbuf,0);
    }
#endif
    if (ret < 0) {
        autoencrypt = 0;
        return(-1);
    } else {
        encrypt_mode = type;
        if (ret == 0 && autoencrypt)
            encrypt_start_output(type);
    }
    return(0);
}

/*
 * Called when a ENCRYPT START command is received.
 */
int
#ifdef CK_ANSIC
encrypt_start(unsigned char *data, int cnt)
#else
encrypt_start(data, cnt) unsigned char *data; int cnt;
#endif
{
    Encryptions *ep;

    if (!decrypt_mode) {
        /*
        * Something is wrong.  We should not get a START
        * command without having already picked our
        * decryption scheme.  Send a REQUEST-END to
        * attempt to clear the channel...
        */
        encrypt_send_request_end();
        printf("Authentication error!\n%s\n",
                "Warning, Cannot decrypt input stream!!!");
        return(-1);
    }

    if (ep = finddecryption(decrypt_mode)) {
        if ( decrypt_input != ep->input ) {
            decrypt_input = ep->input;
            EncryptKSGlobalHack->decrypt = decrypt_ks_stream;
            EncryptKSGlobalHack->decrypt_type = ep->type;

            if (encrypt_verbose) {
                sprintf(dbgbuf, "Input is now decrypted with type %s",
                         ENCTYPE_NAME(decrypt_mode));           /* safe */
                debug(F110,"encrypt_start",dbgbuf,0);
                printf("%s\n",dbgbuf);
            }
#ifdef DEBUG
            if (encrypt_debug_mode) {
                sprintf(dbgbuf, ">>>Start to decrypt input with type %s",
                         ENCTYPE_NAME(decrypt_mode));           /* safe */
                debug(F110,"ck_crp",dbgbuf,0);
            }
#endif
        }
    } else {
        char buf[1024];
        sprintf(buf, "Warning, Cannot decrypt type %s (%d)!!!",
                  ENCTYPE_NAME_OK(decrypt_mode)
                  ? ENCTYPE_NAME(decrypt_mode) : "(unknown)",
                  decrypt_mode);                                /* safe */
        printf("Authentication error!\n%s\n",buf);
        encrypt_send_request_end();
        return(-1);
    }
    return(0);
}

int
#ifdef CK_ANSIC
encrypt_dont_support(int type)
#else
encrypt_dont_support(type) int type;
#endif
{
    i_wont_support_encrypt |= typemask(type);
    i_wont_support_decrypt |= typemask(type);
    return(0);
}

int
#ifdef CK_ANSIC
encrypt_session_key(Session_Key *key, int server)
#else
encrypt_session_key(key, server) Session_Key *key; int server;
#endif
{
    Encryptions *ep = encryptions;

    if (havesessionkey)
        return(0);

    havesessionkey = 1;

    while (ep->type) {
        debug(F111,"encrypt_session_key",ep->name,ep->type);
        if (ep->session) {
            if ((*ep->session)(key, server) < 0) {
                i_wont_support_encrypt |= typemask(ep->type);
                i_wont_support_decrypt |= typemask(ep->type);
            }
        }
        ++ep;
    }
    debug(F111,"encrypt_session_key (done)",ep->name,ep->type);
    return(0);
}

/*
 * Called when ENCRYPT END is received.
 */
int
#ifdef CK_ANSIC
encrypt_end(VOID)
#else
encrypt_end()
#endif
{
    decrypt_input = NULL;
    EncryptKSGlobalHack->decrypt = NULL;
    EncryptKSGlobalHack->decrypt_type = ENCTYPE_ANY;
#ifdef DEBUG
    if (encrypt_debug_mode) {
        sprintf(dbgbuf, ">>>Input is back to clear text");      /* safe */
        debug(F110,"encrypt_end",dbgbuf,0);
    }
#endif
    if (encrypt_verbose) {
        sprintf(dbgbuf, "Input is now clear text");             /* safe */
        debug(F110,"encrypt_end",dbgbuf,0);
        printf("%s\n",dbgbuf);
    }
    return(0);
}

/*
 * Called when ENCRYPT REQUEST-END is received.
 */
int
#ifdef CK_ANSIC
encrypt_request_end(VOID)
#else
encrypt_request_end()
#endif
{
    encrypt_send_end();
    return(0);
}

/*
 * Called when ENCRYPT REQUEST-START is received.  If we receive
 * this before a type is picked, then that indicates that the
 * other side wants us to start encrypting data as soon as we
 * can.
 */
int
#ifdef CK_ANSIC
encrypt_request_start(VOID)
#else
encrypt_request_start()
#endif
{
    if (encrypt_mode != 0)
        encrypt_start_output(encrypt_mode);
    return(0);
}

static unsigned char str_keyid[(MAXKEYLEN*2)+5] = {
    IAC, SB, TELOPT_ENCRYPTION
};
_PROTOTYP(int encrypt_keyid,(struct key_info *,unsigned char *,int));

int
#ifdef CK_ANSIC
encrypt_enc_keyid(unsigned char *keyid, int len)
#else
encrypt_enc_keyid(keyid, len) unsigned char *keyid; int len;
#endif
{
    return(encrypt_keyid(&ki[1], keyid, len));
}

int
#ifdef CK_ANSIC
encrypt_dec_keyid(unsigned char *keyid, int len)
#else
encrypt_dec_keyid(keyid, len) unsigned char *keyid; int len;
#endif /* CK_ANSIC */
{
    return(encrypt_keyid(&ki[0], keyid, len));
}

int
#ifdef CK_ANSIC
encrypt_keyid(struct key_info *kp, unsigned char *keyid, int len)
#else
encrypt_keyid(kp, keyid, len)
    struct key_info *kp; unsigned char *keyid; int len;
#endif
{
    Encryptions *ep;
    int dir = kp->dir;
    register int ret = 0;

    if (!(ep = (*kp->getcrypt)(*kp->modep))) {
        if (len == 0)
            return(-1);
        kp->keylen = 0;
    } else if (len == 0 || len > MAXKEYLEN) {
        /*
        * Empty option or Key too long, indicates a failure.
        */
        if (kp->keylen == 0)
            return(-1);
        kp->keylen = 0;
        if (ep->keyid)
            (void)(*ep->keyid)(dir, kp->keyid, &kp->keylen);

    } else if ((len != kp->keylen) || (memcmp(keyid, kp->keyid, len) != 0)) {
        /*
        * Length or contents are different
        */
        kp->keylen = len;
        memcpy(kp->keyid, keyid, len);          /* length < MAXKEYLEN */
        if (ep->keyid)
            (void)(*ep->keyid)(dir, kp->keyid, &kp->keylen);
    } else {
        if (ep->keyid)
            ret = (*ep->keyid)(dir, kp->keyid, &kp->keylen);
        if ((ret == 0) && (dir == DIR_ENCRYPT) && autoencrypt)
            encrypt_start_output(*kp->modep);
        return(0);
    }

    encrypt_send_keyid(dir, kp->keyid, kp->keylen, 0);
    return(0);
}

int
#ifdef CK_ANSIC
encrypt_send_keyid(int dir, unsigned char *keyid, int keylen, int saveit)
#else
encrypt_send_keyid(dir, keyid, keylen, saveit)
     int dir; unsigned char *keyid; int keylen; int saveit;
#endif
{
    unsigned char *strp;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return(0);
#endif /* CK_SSL */

    str_keyid[3] = (dir == DIR_ENCRYPT)
        ? ENCRYPT_ENC_KEYID : ENCRYPT_DEC_KEYID;
    if (saveit && keylen <= MAXKEYLEN) {
        struct key_info *kp = &ki[(dir == DIR_ENCRYPT) ? 0 : 1];
        memcpy(kp->keyid, keyid, keylen);
        kp->keylen = keylen;
    }

    for (strp = &str_keyid[4]; keylen > 0; --keylen) {
        if ((*strp++ = *keyid++) == IAC)
            *strp++ = IAC;
    }
    *strp++ = IAC;
    *strp++ = SE;

    if (deblog || tn_deb || debses) {
        int i;
        sprintf(tn_msg,"TELNET SENT SB %s %s ",
                 TELOPT(TELOPT_ENCRYPTION),
                 (dir == DIR_ENCRYPT) ? "ENC-KEYID" : "DEC-KEYID"); /* safe */
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_keyid[4],strp-str_keyid-2-4);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol(str_keyid, strp - str_keyid);
#ifdef OS2
    ReleaseTelnetMutex();
#endif
    return(0);
}

VOID
#ifdef CK_ANSIC
encrypt_auto(int on)
#else
encrypt_auto(on) int on;
#endif
{
    if (on < 0)
        autoencrypt ^= 1;
    else
        autoencrypt = on ? 1 : 0;
}

VOID
#ifdef CK_ANSIC
decrypt_auto(int on)
#else
decrypt_auto(on) int on;
#endif
{
    if (on < 0)
        autodecrypt ^= 1;
    else
        autodecrypt = on ? 1 : 0;
}

VOID
#ifdef CK_ANSIC
encrypt_start_output(int type)
#else
encrypt_start_output(type) int type;
#endif
{
    Encryptions *ep;
    register unsigned char *p;
    register int i;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return;
#endif /* CK_SSL */

    if (!(ep = findencryption(type))) {
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>Can't encrypt with type %s (%d)\n",
                     ENCTYPE_NAME_OK(type)
                     ? ENCTYPE_NAME(type) : "(unknown)",
                     type);                                     /* safe */
            debug(F110,"encrypt_start_output",dbgbuf,0);
        }
#endif
        return;
    }
    if (ep->start) {
        i = (*ep->start)(DIR_ENCRYPT, 0);
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>Encrypt start: %s (%d) %s\n",
                     (i < 0) ? "failed" :
                     "initial negotiation in progress",
                     i, ENCTYPE_NAME(type));                    /* safe */
            debug(F110,"encrypt_start_output",dbgbuf,0);
        }
#endif
        if (i)
            return;
    }

    if ( encrypt_output != ep->output ) {
        p = str_start;
        *p++ = IAC;
        *p++ = SB;
        *p++ = TELOPT_ENCRYPTION;
        *p++ = ENCRYPT_START;
        for (i = 0; i < ki[0].keylen; ++i) {
            if (( *p++ = ki[0].keyid[i]) == IAC)
                *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,"TELNET SENT SB %s START ",
                     TELOPT(TELOPT_ENCRYPTION));                /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_start[4],p-str_start-2-4);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(str_start, p - str_start);
#ifdef OS2
        ReleaseTelnetMutex();
#endif

  /*
   * If we are already encrypting in some mode, then
   * encrypt the ring (which includes our request) in
   * the old mode, mark it all as "clear text" and then
   * switch to the new mode.
   */
        encrypt_output = ep->output;
        EncryptKSGlobalHack->encrypt = encrypt_ks_stream;
        EncryptKSGlobalHack->encrypt_type = type;
        encrypt_mode = type;
#ifdef DEBUG
        if (encrypt_debug_mode) {
            sprintf(dbgbuf, ">>>Started to encrypt output with type %s",
                     ENCTYPE_NAME(type));                       /* safe */
            debug(F110,"encrypt_start_output",dbgbuf,0);
        }
#endif
        if (encrypt_verbose) {
            sprintf(dbgbuf, "Output is now encrypted with type %s",
                     ENCTYPE_NAME(type));                       /* safe */
            debug(F110,"encrypt_start_output",dbgbuf,0);
            printf("%s\n",dbgbuf);
        }
    }
}

VOID
#ifdef CK_ANSIC
encrypt_send_end(VOID)
#else
encrypt_send_end()
#endif
{
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return;
#endif /* CK_SSL */

    if (!encrypt_output)
        return;

    str_end[0] = IAC;
    str_end[1] = SB;
    str_end[2] = TELOPT_ENCRYPTION;
    str_end[3] = ENCRYPT_END;
    str_end[4] = IAC;
    str_end[5] = SE;

    if (deblog || tn_deb || debses) {
        int i;
        sprintf(tn_msg,"TELNET SENT SB %s END IAC SE",
                 TELOPT(TELOPT_ENCRYPTION));                    /* safe */
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol(str_end, sizeof(str_end));
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    encrypt_output = 0;
    EncryptKSGlobalHack->encrypt = NULL;
    EncryptKSGlobalHack->encrypt_type = ENCTYPE_ANY;
#ifdef DEBUG
    if (encrypt_debug_mode) {
        sprintf(dbgbuf, ">>>Output is back to clear text");     /* safe */
        debug(F110,"encrypt_send_end",dbgbuf,0);
    }
#endif
    if (encrypt_verbose) {
        sprintf(dbgbuf, "Output is now clear text");            /* safe */
        debug(F110,"encrypt_send_end",dbgbuf,0);
        printf("%s\n",dbgbuf);
    }
}

VOID
#ifdef CK_ANSIC
encrypt_send_request_start(VOID)
#else
encrypt_send_request_start()
#endif
{
    register unsigned char *p;
    register int i;

#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return;
#endif /* CK_SSL */

    p = str_start;
    *p++ = IAC;
    *p++ = SB;
    *p++ = TELOPT_ENCRYPTION;
    *p++ = ENCRYPT_REQSTART;
    for (i = 0; i < ki[1].keylen; ++i) {
        if (( *p++ = ki[1].keyid[i]) == IAC)
            *p++ = IAC;
    }
    *p++ = IAC;
    *p++ = SE;

    if (deblog || tn_deb || debses) {
        int i;
        sprintf(tn_msg,"TELNET SENT SB %s REQUEST-START ",
                 TELOPT(TELOPT_ENCRYPTION));                    /* safe */
        tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&str_start[4],p-str_start-2-4);
        ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol(str_start, p - str_start);
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    if (encrypt_debug_mode) {
        sprintf(dbgbuf, ">>>Request input to be encrypted\n");  /* safe */
        debug(F110,"encrypt_send_request_start",dbgbuf,0);
    }
}

VOID
#ifdef CK_ANSIC
encrypt_send_request_end(VOID)
#else
encrypt_send_request_end()
#endif
{
#ifdef CK_SSL
    if (TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
        return;
#endif /* CK_SSL */

    str_end[0] = IAC;
    str_end[1] = SB;
    str_end[2] = TELOPT_ENCRYPTION;
    str_end[3] = ENCRYPT_REQEND;
    str_end[4] = IAC;
    str_end[5] = SE;

    if (deblog || tn_deb || debses) {
        int i;
        sprintf(tn_msg,"TELNET SENT SB %s REQEND IAC SE",
                 TELOPT(TELOPT_ENCRYPTION));                    /* safe */
        debug(F100,tn_msg,"",0);
        if (tn_deb || debses) tn_debug(tn_msg);
    }
#ifdef OS2
    RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
    ttol(str_end, sizeof(str_end));
#ifdef OS2
    ReleaseTelnetMutex();
#endif

    if (encrypt_debug_mode) {
        sprintf(dbgbuf, ">>>Request input to be clear text\n"); /* safe */
        debug(F110,"encrypt_send_request_end",dbgbuf,0);
    }
}

int
#ifdef CK_ANSIC
encrypt_is_encrypting(VOID)
#else
encrypt_is_encrypting()
#endif
{
    if (encrypt_output)
        return 1;
    return 0;
}

int
#ifdef CK_ANSIC
encrypt_is_decrypting(VOID)
#else
encrypt_is_decrypting()
#endif
{
    if (decrypt_input)
        return 1;
    return 0;
}

#ifdef DEBUG
void
encrypt_debug(mode)
     int mode;
{
    encrypt_debug_mode = mode;
}
#endif

#ifdef CK_DES
/*-
 * Copyright (c) 1991, 1993
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
 */

/* based on @(#)enc_des.c       8.1 (Berkeley) 6/4/93 */

#define CFB     0
#define OFB     1

#define NO_SEND_IV      1
#define NO_RECV_IV      2
#define NO_KEYID        4
#define IN_PROGRESS     (NO_SEND_IV|NO_RECV_IV|NO_KEYID)
#define SUCCESS         0
#define xFAILED         -1

Schedule test_sched;

struct des_stinfo {
    Block               str_output;
    Block               str_feed;
    Block               str_iv;
    Block               str_ikey;
#ifdef MIT_CURRENT
    unsigned char       str_keybytes[8];
    krb5_keyblock       str_key;
#else /* MIT_CURRENT */
    Schedule            str_sched;
    int                 str_index;
#endif /* MIT_CURRENT */
    int                 str_flagshift;
};

struct des_fb {
#ifndef MIT_CURRENT
    Block krbdes_key;
    Schedule krbdes_sched;
#endif /* MIT_CURRENT */
    Block temp_feed;
    unsigned char fb_feed[64];
    int need_start;
    int state[2];
    int keyid[2];
    int once;
#ifdef MIT_CURRENT
    int validkey;
#endif /* MIT_CURRENT */
    struct des_stinfo  streams[2];
};
static struct des_fb des_fb[2];

struct des3_stinfo {
    Block               str_output;
    Block               str_feed;
    Block               str_iv;
    Block               str_ikey[3];
    Schedule            str_sched[3];
    int                 str_index;
    int                 str_flagshift;
};

struct des3_fb {
#ifndef MIT_CURRENT
    Block krbdes_key[3];
    Schedule krbdes_sched[3];
#endif /* MIT_CURRENT */
    Block temp_feed;
    unsigned char fb_feed[64];
    int need_start;
    int state[2];
    int keyid[2];
    int once;
#ifdef MIT_CURRENT
    int validkey;
#endif /* MIT_CURRENT */
    struct des3_stinfo streams[2];
};
static struct des3_fb des3_fb[2];

struct keyidlist {
    char        *keyid;
    int keyidlen;
    char        *key;
    int keylen;
    int flags;
} keyidlist [] = {
    { "\0", 1, 0, 0, 0 },               /* default key of zero */
    { 0, 0, 0, 0, 0 }
};

#define KEYFLAG_MASK    03

#define KEYFLAG_NOINIT  00
#define KEYFLAG_INIT    01
#define KEYFLAG_OK      02
#define KEYFLAG_BAD     03

#define KEYFLAG_SHIFT   2

#define SHIFT_VAL(a,b)  (KEYFLAG_SHIFT*((a)+((b)*2)))

#define FB64_IV         1
#define FB64_IV_OK      2
#define FB64_IV_BAD     3
#define FB64_CHALLENGE  4
#define FB64_RESPONSE   5

void fb64_stream_iv P((Block, struct des_stinfo *));
void fb64_init P((struct des_fb *));
static int fb64_start P((struct des_fb *, int, int));
int fb64_is P((unsigned char *, int, struct des_fb *));
int fb64_reply P((unsigned char *, int, struct des_fb *));
static int fb64_session P((Session_Key *, int, struct des_fb *));
void fb64_stream_key P((Block, struct des_stinfo *));
int fb64_keyid P((int, unsigned char *, int *, struct des_fb *));

#ifdef MIT_CURRENT
static void
#ifdef CK_ANSIC
ecb_encrypt(struct des_stinfo *stp, Block in, Block out)
#else /* CKANSIC */
ecb_encrypt(stp, in, out)
    struct des_stinfo *stp;
    Block in;
    Block out;
#endif /* CK_ANSIC */
{
    krb5_error_code code;
    krb5_data din;
    krb5_enc_data dout;

    din.length = 8;
    din.data = in;

    dout.ciphertext.length = 8;
    dout.ciphertext.data = out;
    dout.enctype = ENCTYPE_UNKNOWN;

#ifdef CRYPT_DLL
    code = krb5_c_encrypt(*p_k5_context, &stp->str_key, 0, 0,
                           &din, &dout);
#else /* CRYPT_DLL */
    code = krb5_c_encrypt(k5_context, &stp->str_key, 0, 0,
                           &din, &dout);
#endif /* CRYPT_DLL */
    /* XXX I'm not sure what to do if this fails */
    if (code)
        com_err("libtelnet", code, "encrypting stream data");
}
#endif /* MIT_CURRENT */

void
cfb64_init(server)
    int server;
{
    fb64_init(&des_fb[CFB]);
    des_fb[CFB].fb_feed[4] = ENCTYPE_DES_CFB64;
    des_fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, CFB);
    des_fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, CFB);
}

void
ofb64_init(server)
    int server;
{
    fb64_init(&des_fb[OFB]);
    des_fb[OFB].fb_feed[4] = ENCTYPE_DES_OFB64;
    des_fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, OFB);
    des_fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, OFB);
}

void
fb64_init(fbp)
    register struct des_fb *fbp;
{
    memset((void *)fbp, 0, sizeof(*fbp));
    fbp->state[0] = fbp->state[1] = xFAILED;
    fbp->fb_feed[0] = IAC;
    fbp->fb_feed[1] = SB;
    fbp->fb_feed[2] = TELOPT_ENCRYPTION;
    fbp->fb_feed[3] = ENCRYPT_IS;
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 *       2: Not yet.  Other things (like getting the key from
 *          Kerberos) have to happen before we can continue.
 */
int
cfb64_start(dir, server)
    int dir;
    int server;
{
    return(fb64_start(&des_fb[CFB], dir, server));
}
int
ofb64_start(dir, server)
    int dir;
    int server;
{
    return(fb64_start(&des_fb[OFB], dir, server));
}

static int
fb64_start(fbp, dir, server)
    struct des_fb *fbp;
    int dir;
    int server;
{
    int x;
    unsigned char *p;
    register int state;

    switch (dir) {
    case DIR_DECRYPT:
        /*
        * This is simply a request to have the other side
        * start output (our input).  He will negotiate an
        * IV so we need not look for it.
        */
        state = fbp->state[dir-1];
        if (state == xFAILED)
            state = IN_PROGRESS;
        break;

    case DIR_ENCRYPT:
        state = fbp->state[dir-1];
        if (state == xFAILED)
            state = IN_PROGRESS;
        else if ((state & NO_SEND_IV) == 0)
            break;

#ifdef MIT_CURRENT
        if (!fbp->validkey) {
            fbp->need_start = 1;
            break;
        }
#else /* MIT_CURRENT */
        if (!VALIDKEY(fbp->krbdes_key)) {
            fbp->need_start = 1;
            break;
        }
#endif /* MIT_CURRENT */
        state &= ~NO_SEND_IV;
        state |= NO_RECV_IV;
        /*
        * Create a random feed and send it over.
        */
#ifdef MIT_CURRENT
        {
            krb5_data d;
            krb5_error_code code;

            d.data = fbp->temp_feed;
            d.length = sizeof(fbp->temp_feed);

#ifdef CRYPT_DLL
            if (code = krb5_c_random_make_octets(*p_k5_context,&d))
                return(xFAILED);
#else /* CRYPT_DLL */
            if (code = krb5_c_random_make_octets(k5_context,&d))
                return(xFAILED);
#endif /* CRYPT_DLL */
        }

#else /* MIT_CURRENT */
        des_new_random_key(fbp->temp_feed);
        des_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
                         fbp->krbdes_sched, 1);
#endif /* MIT_CURRENT */
        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_IS;
        p++;
        *p++ = FB64_IV;
        for (x = 0; x < sizeof(Block); ++x) {
            if (( *p++ = fbp->temp_feed[x]) == IAC)
                *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s IS %s FB64_IV ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    default:
        return(xFAILED);
    }
    return(fbp->state[dir-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
int
cfb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(fb64_is(data, cnt, &des_fb[CFB]));
}

int
ofb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(fb64_is(data, cnt, &des_fb[OFB]));
}

int
fb64_is(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct des_fb *fbp;
{
    unsigned char *p;
    register int state = fbp->state[DIR_DECRYPT-1];

    if (cnt-- < 1)
        goto failure;

#ifdef CK_SSL
    if (!TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
#endif /* CK_SSL */
    switch (*data++) {
    case FB64_IV:
        if (cnt != sizeof(Block)) {
#ifdef DEBUG
            if (encrypt_debug_mode)
                printf("CFB64: initial vector failed on size\r\n");
#endif
            state = xFAILED;
            goto failure;
        }

#ifdef DEBUG
        if (encrypt_debug_mode) {
            printf("CFB64: initial vector received\r\n");
            printf("Initializing Decrypt stream\r\n");
        }
#endif
        fb64_stream_iv((void *)data, &fbp->streams[DIR_DECRYPT-1]);

        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_OK;
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s REPLY %s FB64_IV_OK ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        state = IN_PROGRESS;
        break;

    default:
#if 0
        if (encrypt_debug_mode) {
            printf("Unknown option type: %d\r\n", *(data-1));
            printf("\r\n");
        }
#endif
        /* FALL THROUGH */
      failure:
        /*
        * We failed.  Send an FB64_IV_BAD option
        * to the other side so it will know that
        * things failed.
        */
        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_BAD;
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s REPLY %s FB64_IV_BAD ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    }
    return(fbp->state[DIR_DECRYPT-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
int
cfb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(fb64_reply(data, cnt, &des_fb[CFB]));
}
int
ofb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(fb64_reply(data, cnt, &des_fb[OFB]));
}


int
fb64_reply(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct des_fb *fbp;
{
    register int state = fbp->state[DIR_ENCRYPT-1];

    if (cnt-- < 1)
        goto failure;

    switch (*data++) {
    case FB64_IV_OK:
        fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        if (state == xFAILED)
            state = IN_PROGRESS;
        state &= ~NO_RECV_IV;
        encrypt_send_keyid(DIR_ENCRYPT, (unsigned char *)"\0", 1, 1);
        break;

    case FB64_IV_BAD:
        memset(fbp->temp_feed, 0, sizeof(Block));
        fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        state = xFAILED;
        break;

    default:
#if 0
        if (encrypt_debug_mode) {
            printf("Unknown option type: %d\r\n", data[-1]);
            printf("\r\n");
        }
#endif
        /* FALL THROUGH */
      failure:
        state = xFAILED;
        break;
    }
    return(fbp->state[DIR_ENCRYPT-1] = state);
}

int
cfb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(fb64_session(key, server, &des_fb[CFB]));
}

int
ofb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(fb64_session(key, server, &des_fb[OFB]));
}

static int
fb64_session(key, server, fbp)
    Session_Key *key;
    int server;
    struct des_fb *fbp;
{
    int rc=0;
    int use2keys;
    struct des_stinfo * s_stream;
    struct des_stinfo * c_stream;

    if(server) {
        s_stream = &fbp->streams[DIR_ENCRYPT-1];
        c_stream = &fbp->streams[DIR_DECRYPT-1];
    }
    else {
        s_stream = &fbp->streams[DIR_DECRYPT-1];
        c_stream = &fbp->streams[DIR_ENCRYPT-1];
    }

    if (!key || key->length < sizeof(Block)) {
        CHAR buf[80];
        sprintf((char *)buf,"Can't set DES session key (%d < %d)",
                key ? key->length : 0, sizeof(Block));          /* safe */
#ifdef DEBUG
        if (encrypt_debug_mode)
            printf("%s\r\n",buf);
#endif
        debug(F110,"fb64_session",buf,0);
        return(-1);
    }
    use2keys = (key->type == SK_DES ||
                 key->length < 2 * sizeof(Block)) ? 0 : 1;
#ifdef MIT_CURRENT
    if(use2keys) {
        memcpy((void *) fbp->keybytes,
                 (void *) (key->data + sizeof(Block)), sizeof(Block));
        des_fixup_key_parity(fbp->);
        fb64_stream_key(fbp->krbdes_key, s_stream);
    }

    memcpy((void *)fbp->krbdes_key, (void *)key->data, sizeof(Block));
    if (key->type != SK_DES)
        des_fixup_key_parity(fbp->krbdes_key);

    if(!use2keys)
        fb64_stream_key(fbp->krbdes_key, s_stream);
    fb64_stream_key(fbp->krbdes_key, c_stream);
    fbp->validkey = 1;

    fb64_stream_key(key->data, &fbp->streams[DIR_ENCRYPT-1]);
    fb64_stream_key(key->data, &fbp->streams[DIR_DECRYPT-1]);
#else /* MIT_CURRENT */
    if(use2keys) {
        memcpy((void *) fbp->krbdes_key,
                 (void *) (key->data + sizeof(Block)), sizeof(Block));
        des_fixup_key_parity(fbp->krbdes_key);
        fb64_stream_key(fbp->krbdes_key, s_stream);
    }

    memcpy((void *)fbp->krbdes_key, (void *)key->data, sizeof(Block));
    if (key->type != SK_DES)
        des_fixup_key_parity(fbp->krbdes_key);

    if(!use2keys)
        fb64_stream_key(fbp->krbdes_key, s_stream);
    fb64_stream_key(fbp->krbdes_key, c_stream);

    if (fbp->once == 0) {
        des_set_random_generator_seed(fbp->krbdes_key);
        fbp->once = 1;
    }

    memset(fbp->krbdes_sched,0,sizeof(Schedule));
    ckhexdump("fb64_session_key",fbp->krbdes_key,8);

    rc = des_key_sched(fbp->krbdes_key, fbp->krbdes_sched);
    if ( rc == -1 ) {
        printf("?Invalid DES key specified for encryption\n");
        debug(F110,"fb64_session_key",
               "invalid DES Key specified for encryption",0);
    } else if ( rc == -2 ) {
        printf("?Weak DES key specified for encryption\n");
        debug(F110,"fb64_session_key",
               "weak DES Key specified for encryption",0);
    } else if ( rc != 0 ) {
        printf("?Key Schedule not created by encryption\n");
        debug(F110,"fb64_session_key",
               "Key Schedule not created by encryption",0);
    }

    ckhexdump("fb64_session_key schedule",fbp->krbdes_sched,8*16);
#endif /* MIT_CURRENT */
    /*
    * Now look to see if krbdes_start() was was waiting for
    * the key to show up.  If so, go ahead an call it now
    * that we have the key.
    */
    if (fbp->need_start) {
        fbp->need_start = 0;
        fb64_start(fbp, DIR_ENCRYPT, server);
    }
    return(0);
}

/*
 * We only accept a keyid of 0.  If we get a keyid of
 * 0, then mark the state as SUCCESS.
 */
int
cfb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(fb64_keyid(dir, kp, lenp, &des_fb[CFB]));
}

int
ofb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(fb64_keyid(dir, kp, lenp, &des_fb[OFB]));
}

int
fb64_keyid(dir, kp, lenp, fbp)
    int dir, *lenp;
    unsigned char *kp;
    struct des_fb *fbp;
{
    register int state = fbp->state[dir-1];

    if (*lenp != 1 || (*kp != '\0')) {
        *lenp = 0;
        return(state);
    }

    if (state == xFAILED)
        state = IN_PROGRESS;

    state &= ~NO_KEYID;

    return(fbp->state[dir-1] = state);
}

#if 0
void
fb64_printsub(data, cnt, buf, buflen, type)
    unsigned char *data, *buf, *type;
    int cnt, buflen;
{
    char lbuf[64];
    register int i;
    char *cp;

    buf[buflen-1] = '\0';               /* make sure it's NULL terminated */
    buflen -= 1;

    switch(data[2]) {
    case FB64_IV:
        sprintf(lbuf, "%s_IV", type);
        cp = lbuf;
        goto common;

    case FB64_IV_OK:
        sprintf(lbuf, "%s_IV_OK", type);
        cp = lbuf;
        goto common;

    case FB64_IV_BAD:
        sprintf(lbuf, "%s_IV_BAD", type);
        cp = lbuf;
        goto common;

    case FB64_CHALLENGE:
        sprintf(lbuf, "%s_CHALLENGE", type);
        cp = lbuf;
        goto common;

    case FB64_RESPONSE:
        sprintf(lbuf, "%s_RESPONSE", type);
        cp = lbuf;
        goto common;

    default:
        sprintf(lbuf, " %d (unknown)", data[2]);
        cp = lbuf;
      common:
        for (; (buflen > 0) && (*buf = *cp++); buf++)
            buflen--;
        for (i = 3; i < cnt; i++) {
            sprintf(lbuf, " %d", data[i]);
            for (cp = lbuf; (buflen > 0) && (*buf = *cp++); buf++)
                buflen--;
        }
        break;
    }
}

void
cfb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    fb64_printsub(data, cnt, buf, buflen, "CFB64");
}

void
ofb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    fb64_printsub(data, cnt, buf, buflen, "OFB64");
}
#endif

void
fb64_stream_iv(seed, stp)
    Block seed;
    register struct des_stinfo *stp;
{
    int rc=0;

    memcpy(stp->str_iv,     seed, sizeof(Block));
    memcpy(stp->str_output, seed, sizeof(Block));

    memset(stp->str_sched,0,sizeof(Schedule));

    ckhexdump("fb64_stream_iv",stp->str_ikey,8);

#ifndef MIT_CURRENT
    rc = des_key_sched(stp->str_ikey, stp->str_sched);
    if ( rc == -1 ) {
        printf("?Invalid DES key specified for encryption\r\n");
        debug(F110,"fb64_stream_iv",
               "invalid DES Key specified for encryption",0);
    } else if ( rc == -2 ) {
        printf("?Weak DES key specified for encryption\r\n");
        debug(F110,"fb64_stream_iv",
               "weak DES Key specified for encryption",0);
    } else if ( rc != 0 ) {
        printf("?Key Schedule not created by encryption\r\n");
        debug(F110,"fb64_stream_iv",
               "Key Schedule not created by encryption",0);
    }
    ckhexdump("fb64_stream_iv schedule",stp->str_sched,8*16);
#endif /* MIT_CURRENT */

    stp->str_index = sizeof(Block);
}

void
fb64_stream_key(key, stp)
    Block key;
    register struct des_stinfo *stp;
{
    int rc = 0;

#ifdef MIT_CURRENT
    memcpy(stp->str_keybytes, key, sizeof(Block));
    stp->str_key.length = 8;
    stp->str_key.contents = stp->str_keybytes;
    /* the original version of this code uses des ecb mode, but
    it only ever does one block at a time.  cbc with a zero iv
    is identical */
    stp->str_key.enctype = ENCTYPE_DES_CBC_RAW;
#else /* MIT_CURRENT */
    memcpy(stp->str_ikey, key, sizeof(Block));

    memset(stp->str_sched,0,sizeof(Schedule));

    ckhexdump("fb64_stream_key",key,8);

    rc = des_key_sched(key, stp->str_sched);
    if ( rc == -1 ) {
        printf("?Invalid DES key specified for encryption\r\n");
        debug(F110,"fb64_stream_key",
               "invalid DES Key specified for encryption",0);
    } else if ( rc == -2 ) {
        printf("?Weak DES key specified for encryption\r\n");
        debug(F110,"fb64_stream_key",
               "weak DES Key specified for encryption",0);
    } else if ( rc != 0 ) {
        printf("?Key Schedule not created by encryption\r\n");
        debug(F110,"fb64_stream_key",
               "Key Schedule not created by encryption",0);
    }
    ckhexdump("fb64_stream_key schedule",stp->str_sched,8*16);
#endif /* MIT_CURRENT */

    memcpy(stp->str_output, stp->str_iv, sizeof(Block));

    stp->str_index = sizeof(Block);
}

/*
 * DES 64 bit Cipher Feedback
 *
 *     key --->+-----+
 *          +->| DES |--+
 *          |  +-----+  |
 *          |           v
 *  INPUT --(--------->(+)+---> DATA
 *          |             |
 *          +-------------+
 *
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = DES(iV, key)
 *      On = Dn ^ Vn
 *      V(n+1) = DES(On, key)
 */

void
cfb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    register struct des_stinfo *stp = &des_fb[CFB].streams[DIR_ENCRYPT-1];
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
#ifdef MIT_CURRENT
            ecb_encrypt(stp, stp->str_output, b);
#else /* MIT_CURRENT */
            des_ecb_encrypt(stp->str_output, b, stp->str_sched, 1);
#endif /* MIT_CURRENT */
            memcpy(stp->str_feed,b,sizeof(Block));
            index = 0;
        }

        /* On encryption, we store (feed ^ data) which is cypher */
        *s = stp->str_output[index] = (stp->str_feed[index] ^ *s);
        s++;
        index++;
    }
    stp->str_index = index;
}

int
cfb64_decrypt(data)
    int data;
{
    register struct des_stinfo *stp = &des_fb[CFB].streams[DIR_DECRYPT-1];
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
#ifdef MIT_CURRENT
        ecb_encrypt(stp, stp->str_output, b);
#else /* MIT_CURRENT */
        des_ecb_encrypt(stp->str_output, b, stp->str_sched, 1);
#endif /* MIT_CURRENT */
        memcpy(stp->str_feed, b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    /* On decryption we store (data) which is cypher. */
    stp->str_output[index] = data;
    return(data ^ stp->str_feed[index]);
}

/*
 * DES 64 bit Output Feedback
 *
 * key --->+-----+
 *      +->| DES |--+
 *      |  +-----+  |
 *      +-----------+
 *                  v
 *  INPUT -------->(+) ----> DATA
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = DES(iV, key)
 *      V(n+1) = DES(Vn, key)
 *      On = Dn ^ Vn
 */
void
ofb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    register struct des_stinfo *stp = &des_fb[OFB].streams[DIR_ENCRYPT-1];
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
#ifdef MIT_CURRENT
            ecb_encrypt(stp, stp->str_feed, b);
#else /* MIT_CURRENT */
            des_ecb_encrypt(stp->str_feed, b, stp->str_sched, 1);
#endif /* MIT_CURRENT */
            memcpy(stp->str_feed,b,sizeof(Block));
            index = 0;
        }
        *s++ ^= stp->str_feed[index];
        index++;
    }
    stp->str_index = index;
}

int
ofb64_decrypt(data)
    int data;
{
    register struct des_stinfo *stp = &des_fb[OFB].streams[DIR_DECRYPT-1];
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
#ifdef MIT_CURRENT
        ecb_encrypt(stp, stp->str_feed, b);
#else /* MIT_CURRENT */
        des_ecb_encrypt(stp->str_feed, b, stp->str_sched, 1);
#endif /* MIT_CURRENT */
        memcpy(stp->str_feed, b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    return(data ^ stp->str_feed[index]);
}


void des3_fb64_stream_iv P((Block, struct des3_stinfo *));
void des3_fb64_init P((struct des3_fb *));
static int des3_fb64_start P((struct des3_fb *, int, int));
int des3_fb64_is P((unsigned char *, int, struct des3_fb *));
int des3_fb64_reply P((unsigned char *, int, struct des3_fb *));
static int des3_fb64_session P((Session_Key *, int, struct des3_fb *));
void des3_fb64_stream_key P((Block *, struct des3_stinfo *));
int des3_fb64_keyid P((int, unsigned char *, int *, struct des3_fb *));

void
des3_cfb64_init(server)
    int server;
{
    des3_fb64_init(&des3_fb[CFB]);
    des3_fb[CFB].fb_feed[4] = ENCTYPE_DES3_CFB64;
    des3_fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, CFB);
    des3_fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, CFB);
}

void
des3_ofb64_init(server)
    int server;
{
    des3_fb64_init(&des3_fb[OFB]);
    des3_fb[OFB].fb_feed[4] = ENCTYPE_DES3_OFB64;
    des3_fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, OFB);
    des3_fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, OFB);
}

void
des3_fb64_init(fbp)
    register struct des3_fb *fbp;
{
    memset((void *)fbp, 0, sizeof(*fbp));
    fbp->state[0] = fbp->state[1] = xFAILED;
    fbp->fb_feed[0] = IAC;
    fbp->fb_feed[1] = SB;
    fbp->fb_feed[2] = TELOPT_ENCRYPTION;
    fbp->fb_feed[3] = ENCRYPT_IS;
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 *       2: Not yet.  Other things (like getting the key from
 *          Kerberos) have to happen before we can continue.
 */
int
des3_cfb64_start(dir, server)
    int dir;
    int server;
{
    return(des3_fb64_start(&des3_fb[CFB], dir, server));
}
int
des3_ofb64_start(dir, server)
    int dir;
    int server;
{
    return(des3_fb64_start(&des3_fb[OFB], dir, server));
}

static int
des3_fb64_start(fbp, dir, server)
    struct des3_fb *fbp;
    int dir;
    int server;
{
    int x;
    unsigned char *p;
    register int state;

    switch (dir) {
    case DIR_DECRYPT:
        /*
        * This is simply a request to have the other side
        * start output (our input).  He will negotiate an
        * IV so we need not look for it.
        */
        state = fbp->state[dir-1];
        if (state == xFAILED)
            state = IN_PROGRESS;
        break;

    case DIR_ENCRYPT:
        state = fbp->state[dir-1];
        if (state == xFAILED)
            state = IN_PROGRESS;
        else if ((state & NO_SEND_IV) == 0)
            break;

        if (!VALIDKEY(fbp->krbdes_key[0]) ||
            !VALIDKEY(fbp->krbdes_key[1]) ||
            !VALIDKEY(fbp->krbdes_key[2]) ) {
            fbp->need_start = 1;
            break;
        }
        state &= ~NO_SEND_IV;
        state |= NO_RECV_IV;
        /*
        * Create a random feed and send it over.
        */
        des_new_random_key(fbp->temp_feed);
#ifdef LIBDES
        des_ecb3_encrypt(fbp->temp_feed, fbp->temp_feed,
                         fbp->krbdes_sched[0],
                         fbp->krbdes_sched[1],
                         fbp->krbdes_sched[2],
                         1);
#else /* LIBDES */
        des_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
                         fbp->krbdes_sched[0], 1);
        des_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
                         fbp->krbdes_sched[1], 0);
        des_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
                         fbp->krbdes_sched[2], 1);
#endif /* LIBDES */

        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_IS;
        p++;
        *p++ = FB64_IV;
        for (x = 0; x < sizeof(Block); ++x) {
            if (( *p++ = fbp->temp_feed[x]) == IAC)
                *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s IS %s FB64_IV ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    default:
        return(xFAILED);
    }
    return(fbp->state[dir-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
int
des3_cfb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(des3_fb64_is(data, cnt, &des3_fb[CFB]));
}

int
des3_ofb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(des3_fb64_is(data, cnt, &des3_fb[OFB]));
}

int
des3_fb64_is(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct des3_fb *fbp;
{
    unsigned char *p;
    register int state = fbp->state[DIR_DECRYPT-1];

    if (cnt-- < 1)
        goto failure;

#ifdef CK_SSL
    if (!TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
#endif /* CK_SSL */
    switch (*data++) {
    case FB64_IV:
        if (cnt != sizeof(Block)) {
#ifdef DEBUG
            if (encrypt_debug_mode)
                printf("DES3_FB64: initial vector failed on size\r\n");
#endif
            state = xFAILED;
            goto failure;
        }

#ifdef DEBUG
        if (encrypt_debug_mode) {
            printf("DES3_FB64: initial vector received\r\n");
            printf("Initializing Decrypt stream\r\n");
        }
#endif
        des3_fb64_stream_iv((void *)data, &fbp->streams[DIR_DECRYPT-1]);

        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_OK;
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s REPLY %s FB64_IV_OK ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        state = IN_PROGRESS;
        break;

    default:
#if 0
        if (encrypt_debug_mode) {
            printf("Unknown option type: %d\r\n", *(data-1));
            printf("\r\n");
        }
#endif
        /* FALL THROUGH */
      failure:
        /*
        * We failed.  Send an FB64_IV_BAD option
        * to the other side so it will know that
        * things failed.
        */
        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_BAD;
        *p++ = IAC;
        *p++ = SE;

        if (deblog || tn_deb || debses) {
            int i;
            sprintf(tn_msg,
                     "TELNET SENT SB %s REPLY %s FB64_IV_BAD ",
                     TELOPT(fbp->fb_feed[2]),
                     enctype_names[fbp->fb_feed[4]]); /* safe */
            tn_hex((CHAR *)tn_msg,TN_MSG_LEN,&fbp->fb_feed[6],
		(p-fbp->fb_feed)-2-6);
            ckstrncat(tn_msg,"IAC SE",TN_MSG_LEN);
            debug(F100,tn_msg,"",0);
            if (tn_deb || debses) tn_debug(tn_msg);
        }
#ifdef OS2
        RequestTelnetMutex( SEM_INDEFINITE_WAIT );
#endif
        ttol(fbp->fb_feed, p - fbp->fb_feed);
#ifdef OS2
        ReleaseTelnetMutex();
#endif
        break;
    }
    return(fbp->state[DIR_DECRYPT-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
int
des3_cfb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(des3_fb64_reply(data, cnt, &des3_fb[CFB]));
}
int
des3_ofb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(des3_fb64_reply(data, cnt, &des3_fb[OFB]));
}


int
des3_fb64_reply(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct des3_fb *fbp;
{
    register int state = fbp->state[DIR_ENCRYPT-1];

    if (cnt-- < 1)
        goto failure;

    switch (*data++) {
    case FB64_IV_OK:
        des3_fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        if (state == xFAILED)
            state = IN_PROGRESS;
        state &= ~NO_RECV_IV;
        encrypt_send_keyid(DIR_ENCRYPT, (unsigned char *)"\0", 1, 1);
        break;

    case FB64_IV_BAD:
        memset(fbp->temp_feed, 0, sizeof(Block));
        des3_fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        state = xFAILED;
        break;

    default:
#if 0
        if (encrypt_debug_mode) {
            printf("Unknown option type: %d\r\n", data[-1]);
            printf("\r\n");
        }
#endif
        /* FALL THROUGH */
      failure:
        state = xFAILED;
        break;
    }
    return(fbp->state[DIR_ENCRYPT-1] = state);
}

int
des3_cfb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(des3_fb64_session(key, server, &des3_fb[CFB]));
}

int
des3_ofb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(des3_fb64_session(key, server, &des3_fb[OFB]));
}

static int
des3_fb64_session(key, server, fbp)
    Session_Key *key;
    int server;
    struct des3_fb *fbp;
{
    int rc=0,i=0;
    int keys2use=0;
    struct des3_stinfo * s_stream;
    struct des3_stinfo * c_stream;

    if(server) {
        s_stream = &fbp->streams[DIR_ENCRYPT-1];
        c_stream = &fbp->streams[DIR_DECRYPT-1];
    }
    else {
        s_stream = &fbp->streams[DIR_DECRYPT-1];
        c_stream = &fbp->streams[DIR_ENCRYPT-1];
    }

    keys2use = key->length / sizeof(Block);
    if (!key || (key->type == SK_DES) || (keys2use < 2)) {
        CHAR buf[80];
        sprintf((char *)buf,"Can't set 3DES session key (%d < %d)",
                key ? key->length : 0, 2 * (int)sizeof(Block));	/* safe */
#ifdef DEBUG
        if (encrypt_debug_mode)
            printf("%s\r\n",buf);
#endif
        debug(F110,"des3_fb64_session",buf,0);
        return(-1);
    }

    debug(F111,"des3_fb64_session","keys2use",keys2use);
    /* Compute the first set of keys / key order */
    switch ( keys2use ) {
    case 2:
        memcpy((void *)fbp->krbdes_key[0], (void *)key->data, sizeof(Block));
        memcpy((void *) fbp->krbdes_key[1],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *)fbp->krbdes_key[2], (void *)key->data, sizeof(Block));
        break;
    case 3:
    default:
        memcpy((void *)fbp->krbdes_key[0], (void *)key->data, sizeof(Block));
        memcpy((void *) fbp->krbdes_key[1],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *) fbp->krbdes_key[2],
                 (void *) (key->data + 2*sizeof(Block)), sizeof(Block));
        break;
    }
    ckhexdump("des3_session_key key->data",key->data,sizeof(Block));
    ckhexdump("des3_session_key fbp->krbdes_key[0]",
            fbp->krbdes_key[0],
            sizeof(Block)
            );
    if (fbp->once == 0) {
        des_set_random_generator_seed(fbp->krbdes_key[0]);
        fbp->once = 1;
    }

    for ( i=0;i<3;i++ )
        des_fixup_key_parity(fbp->krbdes_key[i]);
    des3_fb64_stream_key(fbp->krbdes_key, s_stream);


    /* Compute the second set of keys / key order */
    switch ( keys2use ) {
    case 2:
        memcpy((void *) fbp->krbdes_key[0],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *)fbp->krbdes_key[1], (void *)key->data, sizeof(Block));
        memcpy((void *) fbp->krbdes_key[2],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        break;
    case 3:
        memcpy((void *) fbp->krbdes_key[0],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *) fbp->krbdes_key[1],
                 (void *) (key->data + 2*sizeof(Block)), sizeof(Block));
        memcpy((void *)fbp->krbdes_key[2], (void *)key->data, sizeof(Block));
        break;
    case 4:
        memcpy((void *) fbp->krbdes_key[0],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *) fbp->krbdes_key[1],
                 (void *) (key->data + 3*sizeof(Block)), sizeof(Block));
        memcpy((void *)fbp->krbdes_key[2], (void *)key->data, sizeof(Block));
        break;
    case 5:
        memcpy((void *) fbp->krbdes_key[0],(void *)(key->data + sizeof(Block)),
                 sizeof(Block));
        memcpy((void *) fbp->krbdes_key[1],
                 (void *) (key->data + 3*sizeof(Block)), sizeof(Block));
        memcpy((void *)fbp->krbdes_key[2],
                 (void *)(key->data + 4*sizeof(Block)), sizeof(Block));
        break;
    case 6:
        memcpy((void *) fbp->krbdes_key[0],
                 (void *) (key->data + 3*sizeof(Block)), sizeof(Block));
        memcpy((void *)fbp->krbdes_key[1],
                 (void *)(key->data + 4*sizeof(Block)), sizeof(Block));
        memcpy((void *) fbp->krbdes_key[2],
                 (void *) (key->data + 5 *sizeof(Block)),  sizeof(Block));
        break;
    }

    for ( i=0;i<3;i++ )
        des_fixup_key_parity(fbp->krbdes_key[i]);
    des3_fb64_stream_key(fbp->krbdes_key, c_stream);

    /* now use the second set of keys to build the default Key Schedule */
    /* which is used for generating the IV.                             */
    for ( i=0;i<3;i++ ) {
        memset(fbp->krbdes_sched[i],0,sizeof(Schedule));

        rc = des_key_sched(fbp->krbdes_key[i], fbp->krbdes_sched[i]);
        if ( rc == -1 ) {
            printf("?Invalid DES key specified for encryption [DES3,%s]\r\n",
                    server?"server":"client");
            debug(F110,"des3_fb64_stream_iv",
                   "invalid DES Key specified for encryption",0);
        } else if ( rc == -2 ) {
            printf("?Weak DES key specified for encryption\r\n");
            debug(F110,"des3_fb64_stream_iv",
                   "weak DES Key specified for encryption",0);
        } else if ( rc != 0 ) {
            printf("?Key Schedule not created by encryption\r\n");
            debug(F110,"des3_fb64_stream_iv",
                   "Key Schedule not created by encryption",0);
        }
        ckhexdump("des3_fb64_session_key schedule",fbp->krbdes_sched[i],8*16);
    }
    /*
    * Now look to see if krbdes_start() was was waiting for
    * the key to show up.  If so, go ahead an call it now
    * that we have the key.
    */
    if (fbp->need_start) {
        fbp->need_start = 0;
        des3_fb64_start(fbp, DIR_ENCRYPT, server);
    }
    return(0);
}

/*
 * We only accept a keyid of 0.  If we get a keyid of
 * 0, then mark the state as SUCCESS.
 */
int
des3_cfb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(des3_fb64_keyid(dir, kp, lenp, &des3_fb[CFB]));
}

int
des3_ofb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(des3_fb64_keyid(dir, kp, lenp, &des3_fb[OFB]));
}

int
des3_fb64_keyid(dir, kp, lenp, fbp)
    int dir, *lenp;
    unsigned char *kp;
    struct des3_fb *fbp;
{
    register int state = fbp->state[dir-1];

    if (*lenp != 1 || (*kp != '\0')) {
        *lenp = 0;
        return(state);
    }

    if (state == xFAILED)
        state = IN_PROGRESS;

    state &= ~NO_KEYID;

    return(fbp->state[dir-1] = state);
}

#if 0
void
des3_fb64_printsub(data, cnt, buf, buflen, type)
    unsigned char *data, *buf, *type;
    int cnt, buflen;
{
    char lbuf[64];
    register int i;
    char *cp;

    buf[buflen-1] = '\0';               /* make sure it's NULL terminated */
    buflen -= 1;

    switch(data[2]) {
    case FB64_IV:
        sprintf(lbuf, "%s_IV", type);
        cp = lbuf;
        goto common;

    case FB64_IV_OK:
        sprintf(lbuf, "%s_IV_OK", type);
        cp = lbuf;
        goto common;

    case FB64_IV_BAD:
        sprintf(lbuf, "%s_IV_BAD", type);
        cp = lbuf;
        goto common;

    case FB64_CHALLENGE:
        sprintf(lbuf, "%s_CHALLENGE", type);
        cp = lbuf;
        goto common;

    case FB64_RESPONSE:
        sprintf(lbuf, "%s_RESPONSE", type);
        cp = lbuf;
        goto common;

    default:
        sprintf(lbuf, " %d (unknown)", data[2]);
        cp = lbuf;
      common:
        for (; (buflen > 0) && (*buf = *cp++); buf++)
            buflen--;
        for (i = 3; i < cnt; i++) {
            sprintf(lbuf, " %d", data[i]);
            for (cp = lbuf; (buflen > 0) && (*buf = *cp++); buf++)
                buflen--;
        }
        break;
    }
}

void
des3_cfb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    des3_fb64_printsub(data, cnt, buf, buflen, "CFB64");
}

void
des3_ofb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    des3_fb64_printsub(data, cnt, buf, buflen, "OFB64");
}
#endif

void
des3_fb64_stream_iv(seed, stp)
    Block seed;
    register struct des3_stinfo *stp;
{
    int rc=0, i = 0;;

    memcpy(stp->str_iv,     seed, sizeof(Block));
    memcpy(stp->str_output, seed, sizeof(Block));
    for ( i=0;i<3;i++ ) {
        memset(stp->str_sched[i],0,sizeof(Schedule));

        ckhexdump("des3_fb64_stream_iv",stp->str_ikey[i],8);

        rc = des_key_sched(stp->str_ikey[i], stp->str_sched[i]);
        if ( rc == -1 ) {
            printf("?Invalid DES key specified for encryption [DES3 iv]\r\n");
            debug(F110,"des3_fb64_stream_iv",
                   "invalid DES Key specified for encryption",0);
        } else if ( rc == -2 ) {
            printf("?Weak DES key specified for encryption\r\n");
            debug(F110,"des3_fb64_stream_iv",
                   "weak DES Key specified for encryption",0);
        } else if ( rc != 0 ) {
            printf("?Key Schedule not created by encryption\r\n");
            debug(F110,"des3_fb64_stream_iv",
                   "Key Schedule not created by encryption",0);
        }
        ckhexdump("des3_fb64_stream_iv schedule",stp->str_sched[i],8*16);
    }
    stp->str_index = sizeof(Block);
}

void
des3_fb64_stream_key(key, stp)
    Block * key;
    register struct des3_stinfo *stp;
{
    int rc = 0, i = 0;

    for ( i=0;i<3;i++ ) {
        memcpy(stp->str_ikey[i], key[i], sizeof(Block));

        memset(stp->str_sched[i],0,sizeof(Schedule));

        ckhexdump("des3_fb64_stream_key",key[i],8);

        rc = des_key_sched(key[i], stp->str_sched[i]);
        if ( rc == -1 ) {
            printf("?Invalid DES key specified for encryption [DES3 key]\r\n");
            debug(F110,"des3_fb64_stream_key",
                   "invalid DES Key specified for encryption",0);
        } else if ( rc == -2 ) {
            printf("?Weak DES key specified for encryption\r\n");
            debug(F110,"des3_fb64_stream_key",
                   "weak DES Key specified for encryption",0);
        } else if ( rc != 0 ) {
            printf("?Key Schedule not created by encryption\r\n");
            debug(F110,"des3_fb64_stream_key",
                   "Key Schedule not created by encryption",0);
        }
        ckhexdump("des3_fb64_stream_key schedule",stp->str_sched[i],8*16);
    }

    memcpy(stp->str_output, stp->str_iv, sizeof(Block));
    stp->str_index = sizeof(Block);
}

/*
 * DES3 64 bit Cipher Feedback
 *
 *                key1       key2       key3
 *                 |          |          |
 *                 v          v          v
 *             +-------+  +-------+  +-------+
 *          +->| DES-e |->| DES-d |->| DES-e |-- +
 *          |  +-------+  +-------+  +-------+   |
 *          |                                    v
 *  INPUT --(-------------------------------->(+)+---> DATA
 *          |                                    |
 *          +------------------------------------+
 *
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = DES-e(DES-d(DES-e(iV, key1),key2),key3)
 *      On = Dn ^ Vn
 *      V(n+1) = DES-e(DES-d(DES-e(On, key1),key2),key3)
 */

void
des3_cfb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    register struct des3_stinfo *stp = &des3_fb[CFB].streams[DIR_ENCRYPT-1];
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
#ifdef LIBDES
            des_ecb3_encrypt(stp->str_output, b, stp->str_sched[0],
                              stp->str_sched[1], stp->str_sched[2], 1);
#else /* LIBDES */
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[0], 1);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[1], 0);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[2], 1);
#endif /* LIBDES */
            memcpy(stp->str_feed,b,sizeof(Block));
            index = 0;
        }

        /* On encryption, we store (feed ^ data) which is cypher */
        *s = stp->str_output[index] = (stp->str_feed[index] ^ *s);
        s++;
        index++;
    }
    stp->str_index = index;
}

int
des3_cfb64_decrypt(data)
    int data;
{
    register struct des3_stinfo *stp = &des3_fb[CFB].streams[DIR_DECRYPT-1];
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
#ifdef LIBDES
        des_ecb3_encrypt(stp->str_output, b, stp->str_sched[0],
                          stp->str_sched[1], stp->str_sched[2], 1);
#else /* LIBDES */
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[0], 1);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[1], 0);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[2], 1);
#endif /* LIBDES */
        memcpy(stp->str_feed, b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    /* On decryption we store (data) which is cypher. */
    stp->str_output[index] = data;
    return(data ^ stp->str_feed[index]);
}

/*
 * DES3 64 bit Output Feedback
 *
 *
 *                key1       key2       key3
 *                 |          |          |
 *                 v          v          v
 *             +-------+  +-------+  +-------+
 *          +->| DES-e |->| DES-d |->| DES-e |-- +
 *          |  +-------+  +-------+  +-------+   |
 *          +------------------------------------+
 *                                               v
 *  INPUT ------------------------------------->(+) ----> DATA
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = DES-e(DES-d(DES-e(iV, key1),key2),key3)
 *      V(n+1) = DES-e(DES-d(DES-e(Vn, key1),key2),key3)
 *      On = Dn ^ Vn
 */
void
des3_ofb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    register struct des3_stinfo *stp = &des3_fb[OFB].streams[DIR_ENCRYPT-1];
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
#ifdef LIBDES
            des_ecb3_encrypt(stp->str_feed, b, stp->str_sched[0],
                             stp->str_sched[1], stp->str_sched[2], 1);
#else /* LIBDES */
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[0], 1);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[1], 0);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[2], 1);
#endif /* LIBDES */
            memcpy(stp->str_feed,b,sizeof(Block));
            index = 0;
        }
        *s++ ^= stp->str_feed[index];
        index++;
    }
    stp->str_index = index;
}

int
des3_ofb64_decrypt(data)
    int data;
{
    register struct des3_stinfo *stp = &des3_fb[OFB].streams[DIR_DECRYPT-1];
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
#ifdef LIBDES
        des_ecb3_encrypt(stp->str_feed, b, stp->str_sched[0],
                          stp->str_sched[1], stp->str_sched[2], 1);
#else /* LIBDES */
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[0], 1);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[1], 0);
            des_ecb_encrypt(stp->str_output, b,
                             stp->str_sched[2], 1);
#endif /* LIBDES */
        memcpy(stp->str_feed, b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    return(data ^ stp->str_feed[index]);
}
#endif /* CK_DES */

#ifdef CK_CAST
/*-
 * Copyright (c) 1991, 1993
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
 */

/*
 * Copyright (c) 1997 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation.
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

#include <stdio.h>
#ifdef  __STDC__
#include <stdlib.h>
#endif

/*
 * cast.h
 * Author: Tom Wu
 *
 * Type and function declarations for CAST.
 */

#ifndef _CAST_H_
#define _CAST_H_

#ifndef P
#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif /* __STDC__ */
#endif /* P */

#ifndef LITTLE_ENDIAN
#ifndef BIG_ENDIAN
#ifndef WORDS_BIGENDIAN
#define LITTLE_ENDIAN 1
#endif /* WORDS_BIGENDIAN */
#endif /* BIG_ENDIAN */
#endif /* LITTLE_ENDIAN */

typedef unsigned int uint32;    /* Must be 32 bits */
typedef uint32 * uint32p;
typedef unsigned char uint8;
typedef uint8 * uint8p;

typedef struct {
  struct CastSubkeyPair {
    uint32 Km;
    uint32 Kr;
  } K[16];
  int ksize;
} CastKeySched;

/*
 * cast*_key_sched(schedule, key)
 *
 * Initializes the CAST key schedule "schedule" according to the given key.
 * The different setup routines accept different length keys:
 *
 *   ck_cast5_40_key_sched: 40-bit/5-byte (12 round) keys
 *   ck_cast5_64_key_sched: 64-bit/8-byte (12 round) keys
 *   ck_cast5_80_key_sched: 80-bit/10-byte (12 round) keys
 *   ck_cast128_key_sched: 128-bit/16-byte (16 round) keys
 */

extern void ck_cast5_40_key_sched P((CastKeySched *, uint8 *));
extern void ck_cast5_64_key_sched P((CastKeySched *, uint8 *));
extern void ck_cast5_80_key_sched P((CastKeySched *, uint8 *));
extern void  ck_cast128_key_sched P((CastKeySched *, uint8 *));

/*
 * ck_cast_ecb_encrypt(output, input, schedule, mode)
 * ck_cast_ecb_crypt(data, schedule, mode)
 *
 * Encrypts the 64-bit "input" according to the CAST key schedule
 * "schedule" and places the result in "output".  If "mode" is 0,
 * ck_cast_ecb_encrypt will encrypt, otherwise it will decrypt.
 * "Output" and "input" can point to the same memory, in which case
 * en/decryption will be performed in place.
 *
 * ck_cast_ecb_crypt accepts input in the form of an array of two
 * 32-bit words and performs encryption/decryption in place.
 */

extern void ck_cast_ecb_encrypt P((uint8 *, uint8 *, CastKeySched *, int));
extern void ck_cast_ecb_crypt P((uint32 *, CastKeySched *, int));

#endif /* CAST_H */

extern encrypt_debug_mode;

#define CFB_40  0
#define OFB_40  1
#ifdef CAST_EXPORT_ENCRYPTION
#define FB_CNT  2
#else
#define CFB_128 2
#define OFB_128 3
#define FB_CNT  4
#endif

#define NO_SEND_IV      1
#define NO_RECV_IV      2
#define NO_KEYID        4
#define IN_PROGRESS     (NO_SEND_IV|NO_RECV_IV|NO_KEYID)
#define SUCCESS         0
#define cFAILED         -1


struct cast_fb {
        Block temp_feed;
        unsigned char fb_feed[64];
        int key_isset;
        int need_start;
        int state[2];
        struct cast_stinfo {
                Block           str_output;
                Block           str_feed;
                Block           str_iv;
                CastKeySched    str_sched;
                int             str_index;
        } streams[2];
};

static struct cast_fb cast_fb[FB_CNT];

#define FB64_IV         1
#define FB64_IV_OK      2
#define FB64_IV_BAD     3


static void cast_fb64_stream_iv P((Block, struct cast_stinfo *));
static void cast_fb64_init P((struct cast_fb *));
static int cast_fb64_start P((struct cast_fb *, int, int));
static int cast_fb64_is P((unsigned char *, int, struct cast_fb *));
static int cast_fb64_reply P((unsigned char *, int, struct cast_fb *));
static int cast_fb64_session P((Session_Key *, int, struct cast_fb *, int));
static void cast_fb64_stream_key P((Block, struct cast_stinfo *, int));
static int cast_fb64_keyid P((int, unsigned char *, int *, struct cast_fb *));
static void _cast_cfb64_encrypt P((unsigned char *,int, struct cast_stinfo *));
static int _cast_cfb64_decrypt P((int, struct cast_stinfo *));
static void _cast_ofb64_encrypt P((unsigned char *,int, struct cast_stinfo *));
static int _cast_ofb64_decrypt P((int, struct cast_stinfo *));

#ifndef CAST_EXPORT_ENCRYPTION
void
cast_cfb64_init(server)
        int server;
{
        cast_fb64_init(&cast_fb[CFB_128]);
        cast_fb[CFB_128].fb_feed[4] = ENCTYPE_CAST128_CFB64;
}

void
cast_ofb64_init(server)
        int server;
{
        cast_fb64_init(&cast_fb[OFB_128]);
        cast_fb[OFB_128].fb_feed[4] = ENCTYPE_CAST128_OFB64;
}
#endif

void
castexp_cfb64_init(server)
        int server;
{
        cast_fb64_init(&cast_fb[CFB_40]);
        cast_fb[CFB_40].fb_feed[4] = ENCTYPE_CAST5_40_CFB64;
}

void
castexp_ofb64_init(server)
        int server;
{
        cast_fb64_init(&cast_fb[OFB_40]);
        cast_fb[OFB_40].fb_feed[4] = ENCTYPE_CAST5_40_OFB64;
}

static void
cast_fb64_init(fbp)
    register struct cast_fb *fbp;
{
    memset((void *)fbp, 0, sizeof(*fbp));
    fbp->key_isset = 0;
    fbp->state[0] = fbp->state[1] = cFAILED;
    fbp->fb_feed[0] = IAC;
    fbp->fb_feed[1] = SB;
    fbp->fb_feed[2] = TELOPT_ENCRYPTION;
    fbp->fb_feed[3] = ENCRYPT_IS;
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 *       2: Not yet.  Other things (like getting the key from
 *          Kerberos) have to happen before we can continue.
 */
#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_start(dir, server)
    int dir;
    int server;
{
    return(cast_fb64_start(&cast_fb[CFB_128], dir, server));
}

int
cast_ofb64_start(dir, server)
    int dir;
    int server;
{
    return(cast_fb64_start(&cast_fb[OFB_128], dir, server));
}
#endif

int
castexp_cfb64_start(dir, server)
    int dir;
    int server;
{
    return(cast_fb64_start(&cast_fb[CFB_40], dir, server));
}

int
castexp_ofb64_start(dir, server)
    int dir;
    int server;
{
    return(cast_fb64_start(&cast_fb[OFB_40], dir, server));
}

static int
cast_fb64_start(fbp, dir, server)
    struct cast_fb *fbp;
    int dir;
    int server;
{
    Block b;
    int x;
    unsigned char *p;
    register int state;

    switch (dir) {
    case DIR_DECRYPT:
        /*
         * This is simply a request to have the other side
         * start output (our input).  He will negotiate an
         * IV so we need not look for it.
         */
        state = fbp->state[dir-1];
        if (state == cFAILED)
            state = IN_PROGRESS;
        break;

    case DIR_ENCRYPT:
        state = fbp->state[dir-1];
        if (state == cFAILED)
            state = IN_PROGRESS;
        else if ((state & NO_SEND_IV) == 0)
            break;

        if (!fbp->key_isset) {
            fbp->need_start = 1;
            break;
        }
        state &= ~NO_SEND_IV;
        state |= NO_RECV_IV;
#ifdef DEBUG
        if (encrypt_debug_mode)
            printf("Creating new feed\r\n");
#endif
        /*
         * Create a random feed and send it over.
         */
        ck_cast_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
                          &fbp->streams[dir-1].str_sched, 0);

        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_IS;
        p++;
        *p++ = FB64_IV;
        for (x = 0; x < sizeof(Block); ++x) {
            if ((*p++ = fbp->temp_feed[x]) == IAC)
                *p++ = IAC;
        }
        *p++ = IAC;
        *p++ = SE;

        ttol(fbp->fb_feed, p - fbp->fb_feed);
        break;
    default:
        return(cFAILED);
    }
    return(fbp->state[dir-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_is(data, cnt, &cast_fb[CFB_128]));
}

int
cast_ofb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_is(data, cnt, &cast_fb[OFB_128]));
}
#endif

int
castexp_cfb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_is(data, cnt, &cast_fb[CFB_40]));
}

int
castexp_ofb64_is(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_is(data, cnt, &cast_fb[OFB_40]));
}

static int
cast_fb64_is(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct cast_fb *fbp;
{
    int x;
    unsigned char *p;
    Block b;
    register int state = fbp->state[DIR_DECRYPT-1];

    if (cnt-- < 1)
        goto failure;

#ifdef CK_SSL
    if (!TELOPT_SB(TELOPT_START_TLS).start_tls.me_follows)
#endif /* CK_SSL */
    switch (*data++) {
    case FB64_IV:
        if (cnt != sizeof(Block)) {
#ifdef DEBUG
            if (encrypt_debug_mode)
                printf("FB64: initial vector failed on size\r\n");
#endif
            state = cFAILED;
            goto failure;
        }
#ifdef DEBUG
        if (encrypt_debug_mode)
            printf("FB64: initial vector received\r\n");

        if (encrypt_debug_mode)
            printf("Initializing Decrypt stream\r\n");
#endif
        cast_fb64_stream_iv((void *)data, &fbp->streams[DIR_DECRYPT-1]);

        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_OK;
        *p++ = IAC;
        *p++ = SE;

        ttol(fbp->fb_feed, p - fbp->fb_feed);
        state = IN_PROGRESS;
        break;

    default:
        /* unknown option type */
        /* FALL THROUGH */
      failure:
        /*
        * We failed.  Send an FB64_IV_BAD option
        * to the other side so it will know that
        * things failed.
        */
        p = fbp->fb_feed + 3;
        *p++ = ENCRYPT_REPLY;
        p++;
        *p++ = FB64_IV_BAD;
        *p++ = IAC;
        *p++ = SE;

        ttol(fbp->fb_feed, p - fbp->fb_feed);
        break;
    }
    return(fbp->state[DIR_DECRYPT-1] = state);
}

/*
 * Returns:
 *      -1: some error.  Negotiation is done, encryption not ready.
 *       0: Successful, initial negotiation all done.
 *       1: successful, negotiation not done yet.
 */
#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_reply(data, cnt, &cast_fb[CFB_128]));
}

int
cast_ofb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_reply(data, cnt, &cast_fb[OFB_128]));
}
#endif

int
castexp_cfb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_reply(data, cnt, &cast_fb[CFB_40]));
}

int
castexp_ofb64_reply(data, cnt)
    unsigned char *data;
    int cnt;
{
    return(cast_fb64_reply(data, cnt, &cast_fb[OFB_40]));
}

static int
cast_fb64_reply(data, cnt, fbp)
    unsigned char *data;
    int cnt;
    struct cast_fb *fbp;
{
    int x;
    unsigned char *p;
    Block b;
    register int state = fbp->state[DIR_ENCRYPT-1];

    if (cnt-- < 1)
        goto failure;

    switch (*data++) {
    case FB64_IV_OK:
        cast_fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        if (state == cFAILED)
            state = IN_PROGRESS;
        state &= ~NO_RECV_IV;
        encrypt_send_keyid(DIR_ENCRYPT, (unsigned char *)"\0", 1, 1);
        break;

    case FB64_IV_BAD:
        memset(fbp->temp_feed, 0, sizeof(Block));
        cast_fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
        state = cFAILED;
        break;

    default:
#if 0
        if (encrypt_debug_mode) {
            printf("Unknown option type: %d\r\n", data[-1]);
            printd(data, cnt);
            printf("\r\n");
        }
#endif
        /* FALL THROUGH */
      failure:
        state = cFAILED;
        break;
    }
    return(fbp->state[DIR_ENCRYPT-1] = state);
}

#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(cast_fb64_session(key, server, &cast_fb[CFB_128], 1));
}

int
cast_ofb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(cast_fb64_session(key, server, &cast_fb[OFB_128], 1));
}
#endif

int
castexp_cfb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(cast_fb64_session(key, server, &cast_fb[CFB_40], 0));
}

int
castexp_ofb64_session(key, server)
    Session_Key *key;
    int server;
{
    return(cast_fb64_session(key, server, &cast_fb[OFB_40], 0));
}

#define CAST128_KEYLEN  16      /* 128 bits */
#define CAST5_40_KEYLEN  5      /*  40 bits */

static int
cast_fb64_session(key, server, fbp, fs)
    Session_Key *key;
    int server;
    struct cast_fb *fbp;
    int fs;
{
    int klen;
    unsigned char * kptr;

    if(fs)
        klen = CAST128_KEYLEN;
    else
        klen = CAST5_40_KEYLEN;

    if (!key || key->length < klen) {
        CHAR buf[80];
        sprintf((char *)buf,"Can't set CAST session key (%d < %d)",
                key ? key->length : 0, klen);                   /* safe */
#ifdef DEBUG
        if (encrypt_debug_mode)
            printf("%s\r\n",buf);
#endif
        debug(F110,"cast_fb64_session",buf,0);
        return(cFAILED);
    }
    if(key->length < 2 * klen)
        kptr = key->data;
    else
        kptr = key->data + klen;

    if(server) {
        cast_fb64_stream_key(kptr, &fbp->streams[DIR_ENCRYPT-1], fs);
        cast_fb64_stream_key(key->data, &fbp->streams[DIR_DECRYPT-1], fs);
    }
    else {
        cast_fb64_stream_key(kptr, &fbp->streams[DIR_DECRYPT-1], fs);
        cast_fb64_stream_key(key->data, &fbp->streams[DIR_ENCRYPT-1], fs);
    }

    /* Stuff leftovers into the feed */
    if(key->length >= 2 * klen + sizeof(Block))
        memcpy(fbp->temp_feed, key->data + 2 * klen, sizeof(Block));
    else {
#ifdef COMMENT
        /* This is a better way of erasing the password */
        /* but we do not want to link in libsrp         */
        t_random(fbp->temp_feed, sizeof(Block));
#else
        memset(fbp->temp_feed, 0, sizeof(Block));
#endif
    }

    fbp->key_isset = 1;
    /*
    * Now look to see if cast_fb64_start() was was waiting for
    * the key to show up.  If so, go ahead an call it now
    * that we have the key.
    */
    if (fbp->need_start) {
        fbp->need_start = 0;
        cast_fb64_start(fbp, DIR_ENCRYPT, server);
    }
    return(0);
}

/*
 * We only accept a keyid of 0.  If we get a keyid of
 * 0, then mark the state as SUCCESS.
 */
#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(cast_fb64_keyid(dir, kp, lenp, &cast_fb[CFB_128]));
}

int
cast_ofb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(cast_fb64_keyid(dir, kp, lenp, &cast_fb[OFB_128]));
}
#endif

int
castexp_cfb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(cast_fb64_keyid(dir, kp, lenp, &cast_fb[CFB_40]));
}

int
castexp_ofb64_keyid(dir, kp, lenp)
    int dir, *lenp;
    unsigned char *kp;
{
    return(cast_fb64_keyid(dir, kp, lenp, &cast_fb[OFB_40]));
}

static int
cast_fb64_keyid(dir, kp, lenp, fbp)
    int dir, *lenp;
    unsigned char *kp;
    struct cast_fb *fbp;
{
    register int state = fbp->state[dir-1];

    if (*lenp != 1 || (*kp != '\0')) {
        *lenp = 0;
        return(state);
    }

    if (state == cFAILED)
        state = IN_PROGRESS;

    state &= ~NO_KEYID;

    return(fbp->state[dir-1] = state);
}

static void
cast_fb64_printsub(data, cnt, buf, buflen, type)
    unsigned char *data, *buf, *type;
    int cnt, buflen;
{
    char lbuf[64];
    register int i;
    char *cp;

    buf[buflen-1] = '\0';               /* make sure it's NULL terminated */
    buflen -= 1;

    switch(data[2]) {
    case FB64_IV:
        sprintf(lbuf, "%s_IV", type);
        cp = lbuf;
        goto common;

    case FB64_IV_OK:
        sprintf(lbuf, "%s_IV_OK", type);
        cp = lbuf;
        goto common;

    case FB64_IV_BAD:
        sprintf(lbuf, "%s_IV_BAD", type);
        cp = lbuf;
        goto common;

    default:
        sprintf(lbuf, " %d (unknown)", data[2]);
        cp = lbuf;
      common:
        for (; (buflen > 0) && (*buf = *cp++); buf++)
            buflen--;
        for (i = 3; i < cnt; i++) {
            sprintf(lbuf, " %d", data[i]);
            for (cp = lbuf; (buflen > 0) && (*buf = *cp++); buf++)
                buflen--;
        }
        break;
    }
}

void
cast_cfb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    cast_fb64_printsub(data, cnt, buf, buflen, "CFB64");
}

void
cast_ofb64_printsub(data, cnt, buf, buflen)
    unsigned char *data, *buf;
    int cnt, buflen;
{
    cast_fb64_printsub(data, cnt, buf, buflen, "OFB64");
}

static void
cast_fb64_stream_iv(seed, stp)
    Block seed;
    register struct cast_stinfo *stp;
{
    memcpy((void *)stp->str_iv, (void *)seed, sizeof(Block));
    memcpy((void *)stp->str_output, (void *)seed, sizeof(Block));

    stp->str_index = sizeof(Block);
}

static void
cast_fb64_stream_key(key, stp, fs)
    unsigned char * key;
    register struct cast_stinfo *stp;
    int fs;
{
#ifndef CAST_EXPORT_ENCRYPTION
    if(fs)
        ck_cast128_key_sched(&stp->str_sched, key);
    else
#endif
        ck_cast5_40_key_sched(&stp->str_sched, key);

    memcpy((void *)stp->str_output, (void *)stp->str_iv, sizeof(Block));

    stp->str_index = sizeof(Block);
}

/*
 * CAST 64 bit Cipher Feedback
 *
 *     key --->+------+
 *          +->| CAST |--+
 *          |  +------+  |
 *          |            v
 *  INPUT --(---------->(+)+---> DATA
 *          |              |
 *          +--------------+
 *
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = CAST(iV, key)
 *      On = Dn ^ Vn
 *      V(n+1) = CAST(On, key)
 */
#ifndef CAST_EXPORT_ENCRYPTION
void
cast_cfb64_encrypt(s, c)
        register unsigned char *s;
        int c;
{
    _cast_cfb64_encrypt(s, c, &cast_fb[CFB_128].streams[DIR_ENCRYPT-1]);
}
#endif

void
castexp_cfb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    _cast_cfb64_encrypt(s, c, &cast_fb[CFB_40].streams[DIR_ENCRYPT-1]);
}

static void
_cast_cfb64_encrypt(s, c, stp)
    register unsigned char *s;
    int c;
    register struct cast_stinfo *stp;
{
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
            ck_cast_ecb_encrypt(b, stp->str_output, &stp->str_sched, 0);
            memcpy((void *)stp->str_feed, (void *)b, sizeof(Block));
            index = 0;
        }

        /* On encryption, we store (feed ^ data) which is cypher */
        *s = stp->str_output[index] = (stp->str_feed[index] ^ *s);
        s++;
        index++;
    }
    stp->str_index = index;
}

#ifndef CAST_EXPORT_ENCRYPTION
int
cast_cfb64_decrypt(data)
    int data;
{
    return _cast_cfb64_decrypt(data, &cast_fb[CFB_128].streams[DIR_DECRYPT-1]);
}
#endif

int
castexp_cfb64_decrypt(data)
    int data;
{
    return _cast_cfb64_decrypt(data, &cast_fb[CFB_40].streams[DIR_DECRYPT-1]);
}

static int
_cast_cfb64_decrypt(data, stp)
    int data;
    register struct cast_stinfo *stp;
{
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
        ck_cast_ecb_encrypt(b, stp->str_output, &stp->str_sched, 0);
        memcpy((void *)stp->str_feed, (void *)b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    /* On decryption we store (data) which is cypher. */
    stp->str_output[index] = data;
    return(data ^ stp->str_feed[index]);
}

/*
 * CAST 64 bit Output Feedback
 *
 * key --->+------+
 *      +->| CAST |--+
 *      |  +------+  |
 *      +------------+
 *                   v
 *  INPUT --------->(+) ----> DATA
 *
 * Given:
 *      iV: Initial vector, 64 bits (8 bytes) long.
 *      Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *      On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *      V0 = CAST(iV, key)
 *      V(n+1) = CAST(Vn, key)
 *      On = Dn ^ Vn
 */
#ifndef CAST_EXPORT_ENCRYPTION
void
cast_ofb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
    _cast_ofb64_encrypt(s, c, &cast_fb[OFB_128].streams[DIR_ENCRYPT-1]);
}
#endif

void
castexp_ofb64_encrypt(s, c)
    register unsigned char *s;
    int c;
{
  _cast_ofb64_encrypt(s, c, &cast_fb[OFB_40].streams[DIR_ENCRYPT-1]);
}

static void
_cast_ofb64_encrypt(s, c, stp)
    register unsigned char *s;
    int c;
    register struct cast_stinfo *stp;
{
    register int index;

    index = stp->str_index;
    while (c-- > 0) {
        if (index == sizeof(Block)) {
            Block b;
            ck_cast_ecb_encrypt(b, stp->str_feed, &stp->str_sched, 0);
            memcpy((void *)stp->str_feed, (void *)b, sizeof(Block));
            index = 0;
        }
        *s++ ^= stp->str_feed[index];
        index++;
    }
    stp->str_index = index;
}

#ifndef CAST_EXPORT_ENCRYPTION
int
cast_ofb64_decrypt(data)
    int data;
{
    return _cast_ofb64_decrypt(data, &cast_fb[OFB_128].streams[DIR_DECRYPT-1]);
}
#endif

int
castexp_ofb64_decrypt(data)
    int data;
{
    return _cast_ofb64_decrypt(data, &cast_fb[OFB_40].streams[DIR_DECRYPT-1]);
}

static int
_cast_ofb64_decrypt(data, stp)
    int data;
    register struct cast_stinfo *stp;
{
    int index;

    if (data == -1) {
        /*
        * Back up one byte.  It is assumed that we will
        * never back up more than one byte.  If we do, this
        * may or may not work.
        */
        if (stp->str_index)
            --stp->str_index;
        return(0);
    }

    index = stp->str_index++;
    if (index == sizeof(Block)) {
        Block b;
        ck_cast_ecb_encrypt(b, stp->str_feed, &stp->str_sched, 0);
        memcpy((void *)stp->str_feed, (void *)b, sizeof(Block));
        stp->str_index = 1;     /* Next time will be 1 */
        index = 0;              /* But now use 0 */
    }

    return(data ^ stp->str_feed[index]);
}

/*
 * Copyright (c) 1997 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation.
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

/*
 * cast.c
 * Author: Tom Wu
 *
 * An implementation of the CAST-128 encryption algorithm, as
 * specified in RFC 2144.
 */

/* The first four S-boxes are for encryption/decryption */

static uint32 S1[] = {
  0x30fb40d4, 0x9fa0ff0b, 0x6beccd2f, 0x3f258c7a, 0x1e213f2f, 0x9c004dd3,
  0x6003e540, 0xcf9fc949, 0xbfd4af27, 0x88bbbdb5, 0xe2034090, 0x98d09675,
  0x6e63a0e0, 0x15c361d2, 0xc2e7661d, 0x22d4ff8e, 0x28683b6f, 0xc07fd059,
  0xff2379c8, 0x775f50e2, 0x43c340d3, 0xdf2f8656, 0x887ca41a, 0xa2d2bd2d,
  0xa1c9e0d6, 0x346c4819, 0x61b76d87, 0x22540f2f, 0x2abe32e1, 0xaa54166b,
  0x22568e3a, 0xa2d341d0, 0x66db40c8, 0xa784392f, 0x004dff2f, 0x2db9d2de,
  0x97943fac, 0x4a97c1d8, 0x527644b7, 0xb5f437a7, 0xb82cbaef, 0xd751d159,
  0x6ff7f0ed, 0x5a097a1f, 0x827b68d0, 0x90ecf52e, 0x22b0c054, 0xbc8e5935,
  0x4b6d2f7f, 0x50bb64a2, 0xd2664910, 0xbee5812d, 0xb7332290, 0xe93b159f,
  0xb48ee411, 0x4bff345d, 0xfd45c240, 0xad31973f, 0xc4f6d02e, 0x55fc8165,
  0xd5b1caad, 0xa1ac2dae, 0xa2d4b76d, 0xc19b0c50, 0x882240f2, 0x0c6e4f38,
  0xa4e4bfd7, 0x4f5ba272, 0x564c1d2f, 0xc59c5319, 0xb949e354, 0xb04669fe,
  0xb1b6ab8a, 0xc71358dd, 0x6385c545, 0x110f935d, 0x57538ad5, 0x6a390493,
  0xe63d37e0, 0x2a54f6b3, 0x3a787d5f, 0x6276a0b5, 0x19a6fcdf, 0x7a42206a,
  0x29f9d4d5, 0xf61b1891, 0xbb72275e, 0xaa508167, 0x38901091, 0xc6b505eb,
  0x84c7cb8c, 0x2ad75a0f, 0x874a1427, 0xa2d1936b, 0x2ad286af, 0xaa56d291,
  0xd7894360, 0x425c750d, 0x93b39e26, 0x187184c9, 0x6c00b32d, 0x73e2bb14,
  0xa0bebc3c, 0x54623779, 0x64459eab, 0x3f328b82, 0x7718cf82, 0x59a2cea6,
  0x04ee002e, 0x89fe78e6, 0x3fab0950, 0x325ff6c2, 0x81383f05, 0x6963c5c8,
  0x76cb5ad6, 0xd49974c9, 0xca180dcf, 0x380782d5, 0xc7fa5cf6, 0x8ac31511,
  0x35e79e13, 0x47da91d0, 0xf40f9086, 0xa7e2419e, 0x31366241, 0x051ef495,
  0xaa573b04, 0x4a805d8d, 0x548300d0, 0x00322a3c, 0xbf64cddf, 0xba57a68e,
  0x75c6372b, 0x50afd341, 0xa7c13275, 0x915a0bf5, 0x6b54bfab, 0x2b0b1426,
  0xab4cc9d7, 0x449ccd82, 0xf7fbf265, 0xab85c5f3, 0x1b55db94, 0xaad4e324,
  0xcfa4bd3f, 0x2deaa3e2, 0x9e204d02, 0xc8bd25ac, 0xeadf55b3, 0xd5bd9e98,
  0xe31231b2, 0x2ad5ad6c, 0x954329de, 0xadbe4528, 0xd8710f69, 0xaa51c90f,
  0xaa786bf6, 0x22513f1e, 0xaa51a79b, 0x2ad344cc,0x7b5a41f0, 0xd37cfbad,
  0x1b069505, 0x41ece491, 0xb4c332e6, 0x032268d4, 0xc9600acc, 0xce387e6d,
  0xbf6bb16c, 0x6a70fb78, 0x0d03d9c9, 0xd4df39de, 0xe01063da, 0x4736f464,
  0x5ad328d8, 0xb347cc96, 0x75bb0fc3, 0x98511bfb, 0x4ffbcc35, 0xb58bcf6a,
  0xe11f0abc, 0xbfc5fe4a, 0xa70aec10, 0xac39570a,0x3f04442f, 0x6188b153,
  0xe0397a2e, 0x5727cb79, 0x9ceb418f, 0x1cacd68d, 0x2ad37c96, 0x0175cb9d,
  0xc69dff09, 0xc75b65f0, 0xd9db40d8, 0xec0e7779, 0x4744ead4, 0xb11c3274,
  0xdd24cb9e, 0x7e1c54bd, 0xf01144f9, 0xd2240eb1, 0x9675b3fd, 0xa3ac3755,
  0xd47c27af, 0x51c85f4d, 0x56907596, 0xa5bb15e6,0x580304f0, 0xca042cf1,
  0x011a37ea, 0x8dbfaadb, 0x35ba3e4a, 0x3526ffa0, 0xc37b4d09, 0xbc306ed9,
  0x98a52666, 0x5648f725, 0xff5e569d, 0x0ced63d0, 0x7c63b2cf, 0x700b45e1,
  0xd5ea50f1, 0x85a92872, 0xaf1fbda7, 0xd4234870, 0xa7870bf3, 0x2d3b4d79,
  0x42e04198, 0x0cd0ede7, 0x26470db8, 0xf881814c,0x474d6ad7, 0x7c0c5e5c,
  0xd1231959, 0x381b7298, 0xf5d2f4db, 0xab838653, 0x6e2f1e23, 0x83719c9e,
  0xbd91e046, 0x9a56456e, 0xdc39200c, 0x20c8c571, 0x962bda1c, 0xe1e696ff,
  0xb141ab08, 0x7cca89b9, 0x1a69e783, 0x02cc4843, 0xa2f7c579, 0x429ef47d,
  0x427b169c, 0x5ac9f049, 0xdd8f0f00, 0x5c8165bf
};

static uint32 S2[] = {
  0x1f201094, 0xef0ba75b, 0x69e3cf7e, 0x393f4380, 0xfe61cf7a, 0xeec5207a,
  0x55889c94, 0x72fc0651, 0xada7ef79, 0x4e1d7235, 0xd55a63ce, 0xde0436ba,
  0x99c430ef, 0x5f0c0794, 0x18dcdb7d, 0xa1d6eff3, 0xa0b52f7b, 0x59e83605,
  0xee15b094, 0xe9ffd909, 0xdc440086, 0xef944459, 0xba83ccb3, 0xe0c3cdfb,
  0xd1da4181, 0x3b092ab1, 0xf997f1c1, 0xa5e6cf7b, 0x01420ddb, 0xe4e7ef5b,
  0x25a1ff41, 0xe180f806, 0x1fc41080, 0x179bee7a, 0xd37ac6a9, 0xfe5830a4,
  0x98de8b7f, 0x77e83f4e, 0x79929269, 0x24fa9f7b, 0xe113c85b, 0xacc40083,
  0xd7503525, 0xf7ea615f, 0x62143154, 0x0d554b63, 0x5d681121, 0xc866c359,
  0x3d63cf73, 0xcee234c0, 0xd4d87e87, 0x5c672b21, 0x071f6181, 0x39f7627f,
  0x361e3084, 0xe4eb573b, 0x602f64a4, 0xd63acd9c, 0x1bbc4635, 0x9e81032d,
  0x2701f50c, 0x99847ab4, 0xa0e3df79, 0xba6cf38c, 0x10843094, 0x2537a95e,
  0xf46f6ffe, 0xa1ff3b1f, 0x208cfb6a, 0x8f458c74, 0xd9e0a227, 0x4ec73a34,
  0xfc884f69, 0x3e4de8df, 0xef0e0088, 0x3559648d, 0x8a45388c, 0x1d804366,
  0x721d9bfd, 0xa58684bb, 0xe8256333, 0x844e8212, 0x128d8098, 0xfed33fb4,
  0xce280ae1, 0x27e19ba5, 0xd5a6c252, 0xe49754bd, 0xc5d655dd, 0xeb667064,
  0x77840b4d, 0xa1b6a801, 0x84db26a9, 0xe0b56714, 0x21f043b7, 0xe5d05860,
  0x54f03084, 0x066ff472, 0xa31aa153, 0xdadc4755, 0xb5625dbf, 0x68561be6,
  0x83ca6b94, 0x2d6ed23b, 0xeccf01db, 0xa6d3d0ba, 0xb6803d5c, 0xaf77a709,
  0x33b4a34c, 0x397bc8d6, 0x5ee22b95, 0x5f0e5304, 0x81ed6f61, 0x20e74364,
  0xb45e1378, 0xde18639b, 0x881ca122, 0xb96726d1, 0x8049a7e8, 0x22b7da7b,
  0x5e552d25, 0x5272d237, 0x79d2951c, 0xc60d894c, 0x488cb402, 0x1ba4fe5b,
  0xa4b09f6b, 0x1ca815cf, 0xa20c3005, 0x8871df63, 0xb9de2fcb, 0x0cc6c9e9,
  0x0beeff53, 0xe3214517, 0xb4542835, 0x9f63293c, 0xee41e729, 0x6e1d2d7c,
  0x50045286, 0x1e6685f3, 0xf33401c6, 0x30a22c95, 0x31a70850, 0x60930f13,
  0x73f98417, 0xa1269859, 0xec645c44, 0x52c877a9, 0xcdff33a6, 0xa02b1741,
  0x7cbad9a2, 0x2180036f, 0x50d99c08, 0xcb3f4861, 0xc26bd765, 0x64a3f6ab,
  0x80342676, 0x25a75e7b, 0xe4e6d1fc, 0x20c710e6, 0xcdf0b680, 0x17844d3b,
  0x31eef84d, 0x7e0824e4, 0x2ccb49eb, 0x846a3bae, 0x8ff77888, 0xee5d60f6,
  0x7af75673, 0x2fdd5cdb, 0xa11631c1, 0x30f66f43, 0xb3faec54, 0x157fd7fa,
  0xef8579cc, 0xd152de58, 0xdb2ffd5e, 0x8f32ce19, 0x306af97a, 0x02f03ef8,
  0x99319ad5, 0xc242fa0f, 0xa7e3ebb0, 0xc68e4906, 0xb8da230c, 0x80823028,
  0xdcdef3c8, 0xd35fb171, 0x088a1bc8, 0xbec0c560, 0x61a3c9e8, 0xbca8f54d,
  0xc72feffa, 0x22822e99, 0x82c570b4, 0xd8d94e89, 0x8b1c34bc, 0x301e16e6,
  0x273be979, 0xb0ffeaa6, 0x61d9b8c6, 0x00b24869, 0xb7ffce3f, 0x08dc283b,
  0x43daf65a, 0xf7e19798, 0x7619b72f, 0x8f1c9ba4, 0xdc8637a0, 0x16a7d3b1,
  0x9fc393b7, 0xa7136eeb, 0xc6bcc63e, 0x1a513742, 0xef6828bc, 0x520365d6,
  0x2d6a77ab, 0x3527ed4b, 0x821fd216, 0x095c6e2e, 0xdb92f2fb, 0x5eea29cb,
  0x145892f5, 0x91584f7f, 0x5483697b, 0x2667a8cc, 0x85196048, 0x8c4bacea,
  0x833860d4, 0x0d23e0f9, 0x6c387e8a, 0x0ae6d249, 0xb284600c, 0xd835731d,
  0xdcb1c647, 0xac4c56ea, 0x3ebd81b3, 0x230eabb0, 0x6438bc87, 0xf0b5b1fa,
  0x8f5ea2b3, 0xfc184642, 0x0a036b7a, 0x4fb089bd, 0x649da589, 0xa345415e,
  0x5c038323, 0x3e5d3bb9, 0x43d79572, 0x7e6dd07c, 0x06dfdf1e, 0x6c6cc4ef,
  0x7160a539, 0x73bfbe70, 0x83877605, 0x4523ecf1
};

static uint32 S3[] = {
  0x8defc240, 0x25fa5d9f, 0xeb903dbf, 0xe810c907, 0x47607fff, 0x369fe44b,
  0x8c1fc644, 0xaececa90, 0xbeb1f9bf, 0xeefbcaea, 0xe8cf1950, 0x51df07ae,
  0x920e8806, 0xf0ad0548, 0xe13c8d83, 0x927010d5, 0x11107d9f, 0x07647db9,
  0xb2e3e4d4, 0x3d4f285e, 0xb9afa820, 0xfade82e0, 0xa067268b, 0x8272792e,
  0x553fb2c0, 0x489ae22b, 0xd4ef9794, 0x125e3fbc, 0x21fffcee, 0x825b1bfd,
  0x9255c5ed, 0x1257a240, 0x4e1a8302, 0xbae07fff, 0x528246e7, 0x8e57140e,
  0x3373f7bf, 0x8c9f8188, 0xa6fc4ee8, 0xc982b5a5, 0xa8c01db7, 0x579fc264,
  0x67094f31, 0xf2bd3f5f, 0x40fff7c1, 0x1fb78dfc, 0x8e6bd2c1, 0x437be59b,
  0x99b03dbf, 0xb5dbc64b, 0x638dc0e6, 0x55819d99, 0xa197c81c, 0x4a012d6e,
  0xc5884a28, 0xccc36f71, 0xb843c213, 0x6c0743f1, 0x8309893c, 0x0feddd5f,
  0x2f7fe850, 0xd7c07f7e, 0x02507fbf, 0x5afb9a04, 0xa747d2d0, 0x1651192e,
  0xaf70bf3e, 0x58c31380, 0x5f98302e, 0x727cc3c4, 0x0a0fb402, 0x0f7fef82,
  0x8c96fdad, 0x5d2c2aae, 0x8ee99a49, 0x50da88b8, 0x8427f4a0, 0x1eac5790,
  0x796fb449, 0x8252dc15, 0xefbd7d9b, 0xa672597d, 0xada840d8, 0x45f54504,
  0xfa5d7403, 0xe83ec305, 0x4f91751a, 0x925669c2, 0x23efe941, 0xa903f12e,
  0x60270df2, 0x0276e4b6, 0x94fd6574, 0x927985b2, 0x8276dbcb, 0x02778176,
  0xf8af918d, 0x4e48f79e, 0x8f616ddf, 0xe29d840e, 0x842f7d83, 0x340ce5c8,
  0x96bbb682, 0x93b4b148, 0xef303cab, 0x984faf28, 0x779faf9b, 0x92dc560d,
  0x224d1e20, 0x8437aa88, 0x7d29dc96, 0x2756d3dc, 0x8b907cee, 0xb51fd240,
  0xe7c07ce3, 0xe566b4a1, 0xc3e9615e, 0x3cf8209d, 0x6094d1e3, 0xcd9ca341,
  0x5c76460e, 0x00ea983b, 0xd4d67881, 0xfd47572c, 0xf76cedd9, 0xbda8229c,
  0x127dadaa, 0x438a074e, 0x1f97c090, 0x081bdb8a, 0x93a07ebe, 0xb938ca15,
  0x97b03cff, 0x3dc2c0f8, 0x8d1ab2ec, 0x64380e51, 0x68cc7bfb, 0xd90f2788,
  0x12490181, 0x5de5ffd4, 0xdd7ef86a, 0x76a2e214, 0xb9a40368, 0x925d958f,
  0x4b39fffa, 0xba39aee9, 0xa4ffd30b, 0xfaf7933b, 0x6d498623, 0x193cbcfa,
  0x27627545, 0x825cf47a, 0x61bd8ba0, 0xd11e42d1, 0xcead04f4, 0x127ea392,
  0x10428db7, 0x8272a972, 0x9270c4a8, 0x127de50b, 0x285ba1c8, 0x3c62f44f,
  0x35c0eaa5, 0xe805d231, 0x428929fb, 0xb4fcdf82, 0x4fb66a53, 0x0e7dc15b,
  0x1f081fab, 0x108618ae, 0xfcfd086d, 0xf9ff2889, 0x694bcc11, 0x236a5cae,
  0x12deca4d, 0x2c3f8cc5, 0xd2d02dfe, 0xf8ef5896, 0xe4cf52da, 0x95155b67,
  0x494a488c, 0xb9b6a80c, 0x5c8f82bc, 0x89d36b45, 0x3a609437, 0xec00c9a9,
  0x44715253, 0x0a874b49, 0xd773bc40, 0x7c34671c, 0x02717ef6, 0x4feb5536,
  0xa2d02fff, 0xd2bf60c4, 0xd43f03c0, 0x50b4ef6d, 0x07478cd1, 0x006e1888,
  0xa2e53f55, 0xb9e6d4bc, 0xa2048016, 0x97573833, 0xd7207d67, 0xde0f8f3d,
  0x72f87b33, 0xabcc4f33, 0x7688c55d, 0x7b00a6b0, 0x947b0001, 0x570075d2,
  0xf9bb88f8, 0x8942019e, 0x4264a5ff, 0x856302e0, 0x72dbd92b, 0xee971b69,
  0x6ea22fde, 0x5f08ae2b, 0xaf7a616d, 0xe5c98767, 0xcf1febd2, 0x61efc8c2,
  0xf1ac2571, 0xcc8239c2, 0x67214cb8, 0xb1e583d1, 0xb7dc3e62, 0x7f10bdce,
  0xf90a5c38, 0x0ff0443d, 0x606e6dc6, 0x60543a49, 0x5727c148, 0x2be98a1d,
  0x8ab41738, 0x20e1be24, 0xaf96da0f, 0x68458425, 0x99833be5, 0x600d457d,
  0x282f9350, 0x8334b362, 0xd91d1120, 0x2b6d8da0, 0x642b1e31, 0x9c305a00,
  0x52bce688, 0x1b03588a, 0xf7baefd5, 0x4142ed9c, 0xa4315c11, 0x83323ec5,
  0xdfef4636, 0xa133c501, 0xe9d3531c, 0xee353783
};

static uint32 S4[] = {
  0x9db30420, 0x1fb6e9de, 0xa7be7bef, 0xd273a298, 0x4a4f7bdb, 0x64ad8c57,
  0x85510443, 0xfa020ed1, 0x7e287aff, 0xe60fb663, 0x095f35a1, 0x79ebf120,
  0xfd059d43, 0x6497b7b1, 0xf3641f63, 0x241e4adf, 0x28147f5f, 0x4fa2b8cd,
  0xc9430040, 0x0cc32220, 0xfdd30b30, 0xc0a5374f, 0x1d2d00d9, 0x24147b15,
  0xee4d111a, 0x0fca5167, 0x71ff904c, 0x2d195ffe, 0x1a05645f, 0x0c13fefe,
  0x081b08ca, 0x05170121, 0x80530100, 0xe83e5efe, 0xac9af4f8, 0x7fe72701,
  0xd2b8ee5f, 0x06df4261, 0xbb9e9b8a, 0x7293ea25, 0xce84ffdf, 0xf5718801,
  0x3dd64b04, 0xa26f263b, 0x7ed48400, 0x547eebe6, 0x446d4ca0, 0x6cf3d6f5,
  0x2649abdf, 0xaea0c7f5, 0x36338cc1, 0x503f7e93, 0xd3772061, 0x11b638e1,
  0x72500e03, 0xf80eb2bb, 0xabe0502e, 0xec8d77de, 0x57971e81, 0xe14f6746,
  0xc9335400, 0x6920318f, 0x081dbb99, 0xffc304a5, 0x4d351805, 0x7f3d5ce3,
  0xa6c866c6, 0x5d5bcca9, 0xdaec6fea, 0x9f926f91, 0x9f46222f, 0x3991467d,
  0xa5bf6d8e, 0x1143c44f, 0x43958302, 0xd0214eeb, 0x022083b8, 0x3fb6180c,
  0x18f8931e, 0x281658e6, 0x26486e3e, 0x8bd78a70, 0x7477e4c1, 0xb506e07c,
  0xf32d0a25, 0x79098b02, 0xe4eabb81, 0x28123b23, 0x69dead38, 0x1574ca16,
  0xdf871b62, 0x211c40b7, 0xa51a9ef9, 0x0014377b, 0x041e8ac8, 0x09114003,
  0xbd59e4d2, 0xe3d156d5, 0x4fe876d5, 0x2f91a340, 0x557be8de, 0x00eae4a7,
  0x0ce5c2ec, 0x4db4bba6, 0xe756bdff, 0xdd3369ac, 0xec17b035, 0x06572327,
  0x99afc8b0, 0x56c8c391, 0x6b65811c, 0x5e146119, 0x6e85cb75, 0xbe07c002,
  0xc2325577, 0x893ff4ec, 0x5bbfc92d, 0xd0ec3b25, 0xb7801ab7, 0x8d6d3b24,
  0x20c763ef, 0xc366a5fc, 0x9c382880, 0x0ace3205, 0xaac9548a, 0xeca1d7c7,
  0x041afa32, 0x1d16625a, 0x6701902c, 0x9b757a54, 0x31d477f7, 0x9126b031,
  0x36cc6fdb, 0xc70b8b46, 0xd9e66a48, 0x56e55a79, 0x026a4ceb, 0x52437eff,
  0x2f8f76b4, 0x0df980a5, 0x8674cde3, 0xedda04eb, 0x17a9be04, 0x2c18f4df,
  0xb7747f9d, 0xab2af7b4, 0xefc34d20, 0x2e096b7c, 0x1741a254, 0xe5b6a035,
  0x213d42f6, 0x2c1c7c26, 0x61c2f50f, 0x6552daf9, 0xd2c231f8, 0x25130f69,
  0xd8167fa2, 0x0418f2c8, 0x001a96a6, 0x0d1526ab, 0x63315c21, 0x5e0a72ec,
  0x49bafefd, 0x187908d9, 0x8d0dbd86, 0x311170a7, 0x3e9b640c, 0xcc3e10d7,
  0xd5cad3b6, 0x0caec388, 0xf73001e1, 0x6c728aff, 0x71eae2a1, 0x1f9af36e,
  0xcfcbd12f, 0xc1de8417, 0xac07be6b, 0xcb44a1d8, 0x8b9b0f56, 0x013988c3,
  0xb1c52fca, 0xb4be31cd, 0xd8782806, 0x12a3a4e2, 0x6f7de532, 0x58fd7eb6,
  0xd01ee900, 0x24adffc2, 0xf4990fc5, 0x9711aac5, 0x001d7b95, 0x82e5e7d2,
  0x109873f6, 0x00613096, 0xc32d9521, 0xada121ff, 0x29908415, 0x7fbb977f,
  0xaf9eb3db, 0x29c9ed2a, 0x5ce2a465, 0xa730f32c, 0xd0aa3fe8, 0x8a5cc091,
  0xd49e2ce7, 0x0ce454a9, 0xd60acd86, 0x015f1919, 0x77079103, 0xdea03af6,
  0x78a8565e, 0xdee356df, 0x21f05cbe, 0x8b75e387, 0xb3c50651, 0xb8a5c3ef,
  0xd8eeb6d2, 0xe523be77, 0xc2154529, 0x2f69efdf, 0xafe67afb, 0xf470c4b2,
  0xf3e0eb5b, 0xd6cc9876, 0x39e4460c, 0x1fda8538, 0x1987832f, 0xca007367,
  0xa99144f8, 0x296b299e, 0x492fc295, 0x9266beab, 0xb5676e69, 0x9bd3ddda,
  0xdf7e052f, 0xdb25701c, 0x1b5e51ee, 0xf65324e6, 0x6afce36c, 0x0316cc04,
  0x8644213e, 0xb7dc59d0, 0x7965291f, 0xccd6fd43, 0x41823979, 0x932bcdf6,
  0xb657c34d, 0x4edfd282, 0x7ae5290c, 0x3cb9536b, 0x851e20fe, 0x9833557e,
  0x13ecf0b0, 0xd3ffb372, 0x3f85c5c1, 0x0aef7ed2
};

/* Encrypt/decrypt one 64-bit block of data */

void
ck_cast_ecb_encrypt(out, in, sched, mode)
    uint8p out;
    uint8p in;
    CastKeySched * sched;
    int mode;           /* zero means encrypt */
{
    uint32 t[2];

#ifdef LITTLE_ENDIAN
    t[0] = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | in[3];
    t[1] = (in[4] << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
#else
    t[0] = *(uint32p) in;
    t[1] = *(uint32p) (in + 4);
#endif

    ck_cast_ecb_crypt(t, sched, mode);

#ifdef LITTLE_ENDIAN
    out[0] = (t[0] >> 24) & 0xff;
    out[1] = (t[0] >> 16) & 0xff;
    out[2] = (t[0] >> 8) & 0xff;
    out[3] = t[0] & 0xff;
    out[4] = (t[1] >> 24) & 0xff;
    out[5] = (t[1] >> 16) & 0xff;
    out[6] = (t[1] >> 8) & 0xff;
    out[7] = t[1] & 0xff;
#else
    *(uint32p) out = t[0];
    *(uint32p) (out + 4) = t[1];
#endif
}

void
ck_cast_ecb_crypt(data, sched, mode)
    uint32p data;
    CastKeySched * sched;
    int mode;
{
    register uint32 L, R, temp;
    register struct CastSubkeyPair * kp;
    register uint8p Ia, Ib, Ic, Id;
    uint32 I;

#ifdef LITTLE_ENDIAN
    Id = (uint8p) &I;
    Ic = Id + 1;
    Ib = Ic + 1;
    Ia = Ib + 1;
#else
    Ia = (uint8p) &I;
    Ib = Ia + 1;
    Ic = Ib + 1;
    Id = Ic + 1;
#endif

    L = data[0];
    R = data[1];

#define type0(left,right)       \
      temp = kp->Km + right;\
      I = (temp << kp->Kr) | (temp >> (32 - kp->Kr));\
      left ^= ((S1[*Ia] ^ S2[*Ib]) - S3[*Ic]) + S4[*Id];

#define type1(left,right)       \
      temp = kp->Km ^ right;\
      I = (temp << kp->Kr) | (temp >> (32 - kp->Kr));\
      left ^= ((S1[*Ia] - S2[*Ib]) + S3[*Ic]) ^ S4[*Id];

#define type2(left,right)       \
      temp = kp->Km - right;\
      I = (temp << kp->Kr) | (temp >> (32 - kp->Kr));\
      left ^= ((S1[*Ia] + S2[*Ib]) ^ S3[*Ic]) - S4[*Id];

    if(mode) {
#ifndef CAST_EXPORT_ENCRYPTION
        if(sched->ksize > 10) {
            kp = &sched->K[15];
            type0(L, R); --kp;
            type2(R, L); --kp;
            type1(L, R); --kp;
            type0(R, L); --kp;
        }
        else
#endif
        kp = &sched->K[11];
        type2(L, R); --kp;
        type1(R, L); --kp;
        type0(L, R); --kp;
        type2(R, L); --kp;
        type1(L, R); --kp;
        type0(R, L); --kp;
        type2(L, R); --kp;
        type1(R, L); --kp;
        type0(L, R); --kp;
        type2(R, L); --kp;
        type1(L, R); --kp;
        type0(R, L);
    }
    else {
        kp = &sched->K[0];
        type0(L, R); ++kp;
        type1(R, L); ++kp;
        type2(L, R); ++kp;
        type0(R, L); ++kp;
        type1(L, R); ++kp;
        type2(R, L); ++kp;
        type0(L, R); ++kp;
        type1(R, L); ++kp;
        type2(L, R); ++kp;
        type0(R, L); ++kp;
        type1(L, R); ++kp;
        type2(R, L); ++kp;
#ifndef CAST_EXPORT_ENCRYPTION
        if(sched->ksize > 10) {
            type0(L, R); ++kp;
            type1(R, L); ++kp;
            type2(L, R); ++kp;
            type0(R, L);
        }
#endif
    }

    data[0] = R;
    data[1] = L;
}

/* The last four S-boxes are for key schedule setup */

static uint32 S5[] = {
  0x7ec90c04, 0x2c6e74b9, 0x9b0e66df, 0xa6337911, 0xb86a7fff, 0x1dd358f5,
  0x44dd9d44, 0x1731167f, 0x08fbf1fa, 0xe7f511cc, 0xd2051b00, 0x735aba00,
  0x2ab722d8, 0x386381cb, 0xacf6243a, 0x69befd7a, 0xe6a2e77f, 0xf0c720cd,
  0xc4494816, 0xccf5c180, 0x38851640, 0x15b0a848, 0xe68b18cb, 0x4caadeff,
  0x5f480a01, 0x0412b2aa, 0x259814fc, 0x41d0efe2, 0x4e40b48d, 0x248eb6fb,
  0x8dba1cfe, 0x41a99b02, 0x1a550a04, 0xba8f65cb, 0x7251f4e7, 0x95a51725,
  0xc106ecd7, 0x97a5980a, 0xc539b9aa, 0x4d79fe6a, 0xf2f3f763, 0x68af8040,
  0xed0c9e56, 0x11b4958b, 0xe1eb5a88, 0x8709e6b0, 0xd7e07156, 0x4e29fea7,
  0x6366e52d, 0x02d1c000, 0xc4ac8e05, 0x9377f571, 0x0c05372a, 0x578535f2,
  0x2261be02, 0xd642a0c9, 0xdf13a280, 0x74b55bd2, 0x682199c0, 0xd421e5ec,
  0x53fb3ce8, 0xc8adedb3, 0x28a87fc9, 0x3d959981, 0x5c1ff900, 0xfe38d399,
  0x0c4eff0b, 0x062407ea, 0xaa2f4fb1, 0x4fb96976, 0x90c79505, 0xb0a8a774,
  0xef55a1ff, 0xe59ca2c2, 0xa6b62d27, 0xe66a4263, 0xdf65001f, 0x0ec50966,
  0xdfdd55bc, 0x29de0655, 0x911e739a, 0x17af8975, 0x32c7911c, 0x89f89468,
  0x0d01e980, 0x524755f4, 0x03b63cc9, 0x0cc844b2, 0xbcf3f0aa, 0x87ac36e9,
  0xe53a7426, 0x01b3d82b, 0x1a9e7449, 0x64ee2d7e, 0xcddbb1da, 0x01c94910,
  0xb868bf80, 0x0d26f3fd, 0x9342ede7, 0x04a5c284, 0x636737b6, 0x50f5b616,
  0xf24766e3, 0x8eca36c1, 0x136e05db, 0xfef18391, 0xfb887a37, 0xd6e7f7d4,
  0xc7fb7dc9, 0x3063fcdf, 0xb6f589de, 0xec2941da, 0x26e46695, 0xb7566419,
  0xf654efc5, 0xd08d58b7, 0x48925401, 0xc1bacb7f, 0xe5ff550f, 0xb6083049,
  0x5bb5d0e8, 0x87d72e5a, 0xab6a6ee1, 0x223a66ce, 0xc62bf3cd, 0x9e0885f9,
  0x68cb3e47, 0x086c010f, 0xa21de820, 0xd18b69de, 0xf3f65777, 0xfa02c3f6,
  0x407edac3, 0xcbb3d550, 0x1793084d, 0xb0d70eba, 0x0ab378d5, 0xd951fb0c,
  0xded7da56, 0x4124bbe4, 0x94ca0b56, 0x0f5755d1, 0xe0e1e56e, 0x6184b5be,
  0x580a249f, 0x94f74bc0, 0xe327888e, 0x9f7b5561, 0xc3dc0280, 0x05687715,
  0x646c6bd7, 0x44904db3, 0x66b4f0a3, 0xc0f1648a, 0x697ed5af, 0x49e92ff6,
  0x309e374f, 0x2cb6356a, 0x85808573, 0x4991f840, 0x76f0ae02, 0x083be84d,
  0x28421c9a, 0x44489406, 0x736e4cb8, 0xc1092910, 0x8bc95fc6, 0x7d869cf4,
  0x134f616f, 0x2e77118d, 0xb31b2be1, 0xaa90b472, 0x3ca5d717, 0x7d161bba,
  0x9cad9010, 0xaf462ba2, 0x9fe459d2, 0x45d34559, 0xd9f2da13, 0xdbc65487,
  0xf3e4f94e, 0x176d486f, 0x097c13ea, 0x631da5c7, 0x445f7382, 0x175683f4,
  0xcdc66a97, 0x70be0288, 0xb3cdcf72, 0x6e5dd2f3, 0x20936079, 0x459b80a5,
  0xbe60e2db, 0xa9c23101, 0xeba5315c, 0x224e42f2, 0x1c5c1572, 0xf6721b2c,
  0x1ad2fff3, 0x8c25404e, 0x324ed72f, 0x4067b7fd, 0x0523138e, 0x5ca3bc78,
  0xdc0fd66e, 0x75922283, 0x784d6b17, 0x58ebb16e, 0x44094f85, 0x3f481d87,
  0xfcfeae7b, 0x77b5ff76, 0x8c2302bf, 0xaaf47556, 0x5f46b02a, 0x2b092801,
  0x3d38f5f7, 0x0ca81f36, 0x52af4a8a, 0x66d5e7c0, 0xdf3b0874, 0x95055110,
  0x1b5ad7a8, 0xf61ed5ad, 0x6cf6e479, 0x20758184, 0xd0cefa65, 0x88f7be58,
  0x4a046826, 0x0ff6f8f3, 0xa09c7f70, 0x5346aba0, 0x5ce96c28, 0xe176eda3,
  0x6bac307f, 0x376829d2, 0x85360fa9, 0x17e3fe2a, 0x24b79767, 0xf5a96b20,
  0xd6cd2595, 0x68ff1ebf, 0x7555442c, 0xf19f06be, 0xf9e0659a, 0xeeb9491d,
  0x34010718, 0xbb30cab8, 0xe822fe15, 0x88570983, 0x750e6249, 0xda627e55,
  0x5e76ffa8, 0xb1534546, 0x6d47de08, 0xefe9e7d4
};

static uint32 S6[] = {
  0xf6fa8f9d, 0x2cac6ce1, 0x4ca34867, 0xe2337f7c, 0x95db08e7, 0x016843b4,
  0xeced5cbc, 0x325553ac, 0xbf9f0960, 0xdfa1e2ed, 0x83f0579d, 0x63ed86b9,
  0x1ab6a6b8, 0xde5ebe39, 0xf38ff732, 0x8989b138, 0x33f14961, 0xc01937bd,
  0xf506c6da, 0xe4625e7e, 0xa308ea99, 0x4e23e33c, 0x79cbd7cc, 0x48a14367,
  0xa3149619, 0xfec94bd5, 0xa114174a, 0xeaa01866, 0xa084db2d, 0x09a8486f,
  0xa888614a, 0x2900af98, 0x01665991, 0xe1992863, 0xc8f30c60, 0x2e78ef3c,
  0xd0d51932, 0xcf0fec14, 0xf7ca07d2, 0xd0a82072, 0xfd41197e, 0x9305a6b0,
  0xe86be3da, 0x74bed3cd, 0x372da53c, 0x4c7f4448, 0xdab5d440, 0x6dba0ec3,
  0x083919a7, 0x9fbaeed9, 0x49dbcfb0, 0x4e670c53, 0x5c3d9c01, 0x64bdb941,
  0x2c0e636a, 0xba7dd9cd, 0xea6f7388, 0xe70bc762, 0x35f29adb, 0x5c4cdd8d,
  0xf0d48d8c, 0xb88153e2, 0x08a19866, 0x1ae2eac8, 0x284caf89, 0xaa928223,
  0x9334be53, 0x3b3a21bf, 0x16434be3, 0x9aea3906, 0xefe8c36e, 0xf890cdd9,
  0x80226dae, 0xc340a4a3, 0xdf7e9c09, 0xa694a807, 0x5b7c5ecc, 0x221db3a6,
  0x9a69a02f, 0x68818a54, 0xceb2296f, 0x53c0843a, 0xfe893655, 0x25bfe68a,
  0xb4628abc, 0xcf222ebf, 0x25ac6f48, 0xa9a99387, 0x53bddb65, 0xe76ffbe7,
  0xe967fd78, 0x0ba93563, 0x8e342bc1, 0xe8a11be9, 0x4980740d, 0xc8087dfc,
  0x8de4bf99, 0xa11101a0, 0x7fd37975, 0xda5a26c0, 0xe81f994f, 0x9528cd89,
  0xfd339fed, 0xb87834bf, 0x5f04456d, 0x22258698, 0xc9c4c83b, 0x2dc156be,
  0x4f628daa, 0x57f55ec5, 0xe2220abe, 0xd2916ebf, 0x4ec75b95, 0x24f2c3c0,
  0x42d15d99, 0xcd0d7fa0, 0x7b6e27ff, 0xa8dc8af0, 0x7345c106, 0xf41e232f,
  0x35162386, 0xe6ea8926, 0x3333b094, 0x157ec6f2, 0x372b74af, 0x692573e4,
  0xe9a9d848, 0xf3160289, 0x3a62ef1d, 0xa787e238, 0xf3a5f676, 0x74364853,
  0x20951063, 0x4576698d, 0xb6fad407, 0x592af950, 0x36f73523, 0x4cfb6e87,
  0x7da4cec0, 0x6c152daa, 0xcb0396a8, 0xc50dfe5d, 0xfcd707ab, 0x0921c42f,
  0x89dff0bb, 0x5fe2be78, 0x448f4f33, 0x754613c9, 0x2b05d08d, 0x48b9d585,
  0xdc049441, 0xc8098f9b, 0x7dede786, 0xc39a3373, 0x42410005, 0x6a091751,
  0x0ef3c8a6, 0x890072d6, 0x28207682, 0xa9a9f7be, 0xbf32679d, 0xd45b5b75,
  0xb353fd00, 0xcbb0e358, 0x830f220a, 0x1f8fb214, 0xd372cf08, 0xcc3c4a13,
  0x8cf63166, 0x061c87be, 0x88c98f88, 0x6062e397, 0x47cf8e7a, 0xb6c85283,
  0x3cc2acfb, 0x3fc06976, 0x4e8f0252, 0x64d8314d, 0xda3870e3, 0x1e665459,
  0xc10908f0, 0x513021a5, 0x6c5b68b7, 0x822f8aa0, 0x3007cd3e, 0x74719eef,
  0xdc872681, 0x073340d4, 0x7e432fd9, 0x0c5ec241, 0x8809286c, 0xf592d891,
  0x08a930f6, 0x957ef305, 0xb7fbffbd, 0xc266e96f, 0x6fe4ac98, 0xb173ecc0,
  0xbc60b42a, 0x953498da, 0xfba1ae12, 0x2d4bd736, 0x0f25faab, 0xa4f3fceb,
  0xe2969123, 0x257f0c3d, 0x9348af49, 0x361400bc, 0xe8816f4a, 0x3814f200,
  0xa3f94043, 0x9c7a54c2, 0xbc704f57, 0xda41e7f9, 0xc25ad33a, 0x54f4a084,
  0xb17f5505, 0x59357cbe, 0xedbd15c8, 0x7f97c5ab, 0xba5ac7b5, 0xb6f6deaf,
  0x3a479c3a, 0x5302da25, 0x653d7e6a, 0x54268d49, 0x51a477ea, 0x5017d55b,
  0xd7d25d88, 0x44136c76, 0x0404a8c8, 0xb8e5a121, 0xb81a928a, 0x60ed5869,
  0x97c55b96, 0xeaec991b, 0x29935913, 0x01fdb7f1, 0x088e8dfa, 0x9ab6f6f5,
  0x3b4cbf9f, 0x4a5de3ab, 0xe6051d35, 0xa0e1d855, 0xd36b4cf1, 0xf544edeb,
  0xb0e93524, 0xbebb8fbd, 0xa2d762cf, 0x49c92f54, 0x38b5f331, 0x7128a454,
  0x48392905, 0xa65b1db8, 0x851c97bd, 0xd675cf2f
};

static uint32 S7[] = {
  0x85e04019, 0x332bf567, 0x662dbfff, 0xcfc65693, 0x2a8d7f6f, 0xab9bc912,
  0xde6008a1, 0x2028da1f, 0x0227bce7, 0x4d642916, 0x18fac300, 0x50f18b82,
  0x2cb2cb11, 0xb232e75c, 0x4b3695f2, 0xb28707de, 0xa05fbcf6, 0xcd4181e9,
  0xe150210c, 0xe24ef1bd, 0xb168c381, 0xfde4e789, 0x5c79b0d8, 0x1e8bfd43,
  0x4d495001, 0x38be4341, 0x913cee1d, 0x92a79c3f, 0x089766be, 0xbaeeadf4,
  0x1286becf, 0xb6eacb19, 0x2660c200, 0x7565bde4, 0x64241f7a, 0x8248dca9,
  0xc3b3ad66, 0x28136086, 0x0bd8dfa8, 0x356d1cf2, 0x107789be, 0xb3b2e9ce,
  0x0502aa8f, 0x0bc0351e, 0x166bf52a, 0xeb12ff82, 0xe3486911, 0xd34d7516,
  0x4e7b3aff, 0x5f43671b, 0x9cf6e037, 0x4981ac83, 0x334266ce, 0x8c9341b7,
  0xd0d854c0, 0xcb3a6c88, 0x47bc2829, 0x4725ba37, 0xa66ad22b, 0x7ad61f1e,
  0x0c5cbafa, 0x4437f107, 0xb6e79962, 0x42d2d816, 0x0a961288, 0xe1a5c06e,
  0x13749e67, 0x72fc081a, 0xb1d139f7, 0xf9583745, 0xcf19df58, 0xbec3f756,
  0xc06eba30, 0x07211b24, 0x45c28829, 0xc95e317f, 0xbc8ec511, 0x38bc46e9,
  0xc6e6fa14, 0xbae8584a, 0xad4ebc46, 0x468f508b, 0x7829435f, 0xf124183b,
  0x821dba9f, 0xaff60ff4, 0xea2c4e6d, 0x16e39264, 0x92544a8b, 0x009b4fc3,
  0xaba68ced, 0x9ac96f78, 0x06a5b79a, 0xb2856e6e, 0x1aec3ca9, 0xbe838688,
  0x0e0804e9, 0x55f1be56, 0xe7e5363b, 0xb3a1f25d, 0xf7debb85, 0x61fe033c,
  0x16746233, 0x3c034c28, 0xda6d0c74, 0x79aac56c, 0x3ce4e1ad, 0x51f0c802,
  0x98f8f35a, 0x1626a49f, 0xeed82b29, 0x1d382fe3, 0x0c4fb99a, 0xbb325778,
  0x3ec6d97b, 0x6e77a6a9, 0xcb658b5c, 0xd45230c7, 0x2bd1408b, 0x60c03eb7,
  0xb9068d78, 0xa33754f4, 0xf430c87d, 0xc8a71302, 0xb96d8c32, 0xebd4e7be,
  0xbe8b9d2d, 0x7979fb06, 0xe7225308, 0x8b75cf77, 0x11ef8da4, 0xe083c858,
  0x8d6b786f, 0x5a6317a6, 0xfa5cf7a0, 0x5dda0033, 0xf28ebfb0, 0xf5b9c310,
  0xa0eac280, 0x08b9767a, 0xa3d9d2b0, 0x79d34217, 0x021a718d, 0x9ac6336a,
  0x2711fd60, 0x438050e3, 0x069908a8, 0x3d7fedc4, 0x826d2bef, 0x4eeb8476,
  0x488dcf25, 0x36c9d566, 0x28e74e41, 0xc2610aca, 0x3d49a9cf, 0xbae3b9df,
  0xb65f8de6, 0x92aeaf64, 0x3ac7d5e6, 0x9ea80509, 0xf22b017d, 0xa4173f70,
  0xdd1e16c3, 0x15e0d7f9, 0x50b1b887, 0x2b9f4fd5, 0x625aba82, 0x6a017962,
  0x2ec01b9c, 0x15488aa9, 0xd716e740, 0x40055a2c, 0x93d29a22, 0xe32dbf9a,
  0x058745b9, 0x3453dc1e, 0xd699296e, 0x496cff6f, 0x1c9f4986, 0xdfe2ed07,
  0xb87242d1, 0x19de7eae, 0x053e561a, 0x15ad6f8c, 0x66626c1c, 0x7154c24c,
  0xea082b2a, 0x93eb2939, 0x17dcb0f0, 0x58d4f2ae, 0x9ea294fb, 0x52cf564c,
  0x9883fe66, 0x2ec40581, 0x763953c3, 0x01d6692e, 0xd3a0c108, 0xa1e7160e,
  0xe4f2dfa6, 0x693ed285, 0x74904698, 0x4c2b0edd, 0x4f757656, 0x5d393378,
  0xa132234f, 0x3d321c5d, 0xc3f5e194, 0x4b269301, 0xc79f022f, 0x3c997e7e,
  0x5e4f9504, 0x3ffafbbd, 0x76f7ad0e, 0x296693f4, 0x3d1fce6f, 0xc61e45be,
  0xd3b5ab34, 0xf72bf9b7, 0x1b0434c0, 0x4e72b567, 0x5592a33d, 0xb5229301,
  0xcfd2a87f, 0x60aeb767, 0x1814386b, 0x30bcc33d, 0x38a0c07d, 0xfd1606f2,
  0xc363519b, 0x589dd390, 0x5479f8e6, 0x1cb8d647, 0x97fd61a9, 0xea7759f4,
  0x2d57539d, 0x569a58cf, 0xe84e63ad, 0x462e1b78, 0x6580f87e, 0xf3817914,
  0x91da55f4, 0x40a230f3, 0xd1988f35, 0xb6e318d2, 0x3ffa50bc, 0x3d40f021,
  0xc3c0bdae, 0x4958c24c, 0x518f36b2, 0x84b1d370, 0x0fedce83, 0x878ddada,
  0xf2a279c7, 0x94e01be8, 0x90716f4b, 0x954b8aa3
};

static uint32 S8[] = {
  0xe216300d, 0xbbddfffc, 0xa7ebdabd, 0x35648095, 0x7789f8b7, 0xe6c1121b,
  0x0e241600, 0x052ce8b5, 0x11a9cfb0, 0xe5952f11, 0xece7990a, 0x9386d174,
  0x2a42931c, 0x76e38111, 0xb12def3a, 0x37ddddfc, 0xde9adeb1, 0x0a0cc32c,
  0xbe197029, 0x84a00940, 0xbb243a0f, 0xb4d137cf, 0xb44e79f0, 0x049eedfd,
  0x0b15a15d, 0x480d3168, 0x8bbbde5a, 0x669ded42, 0xc7ece831, 0x3f8f95e7,
  0x72df191b, 0x7580330d, 0x94074251, 0x5c7dcdfa, 0xabbe6d63, 0xaa402164,
  0xb301d40a, 0x02e7d1ca, 0x53571dae, 0x7a3182a2, 0x12a8ddec, 0xfdaa335d,
  0x176f43e8, 0x71fb46d4, 0x38129022, 0xce949ad4, 0xb84769ad, 0x965bd862,
  0x82f3d055, 0x66fb9767, 0x15b80b4e, 0x1d5b47a0, 0x4cfde06f, 0xc28ec4b8,
  0x57e8726e, 0x647a78fc, 0x99865d44, 0x608bd593, 0x6c200e03, 0x39dc5ff6,
  0x5d0b00a3, 0xae63aff2, 0x7e8bd632, 0x70108c0c, 0xbbd35049, 0x2998df04,
  0x980cf42a, 0x9b6df491, 0x9e7edd53, 0x06918548, 0x58cb7e07, 0x3b74ef2e,
  0x522fffb1, 0xd24708cc, 0x1c7e27cd, 0xa4eb215b, 0x3cf1d2e2, 0x19b47a38,
  0x424f7618, 0x35856039, 0x9d17dee7, 0x27eb35e6, 0xc9aff67b, 0x36baf5b8,
  0x09c467cd, 0xc18910b1, 0xe11dbf7b, 0x06cd1af8, 0x7170c608, 0x2d5e3354,
  0xd4de495a, 0x64c6d006, 0xbcc0c62c, 0x3dd00db3, 0x708f8f34, 0x77d51b42,
  0x264f620f, 0x24b8d2bf, 0x15c1b79e, 0x46a52564, 0xf8d7e54e, 0x3e378160,
  0x7895cda5, 0x859c15a5, 0xe6459788, 0xc37bc75f, 0xdb07ba0c, 0x0676a3ab,
  0x7f229b1e, 0x31842e7b, 0x24259fd7, 0xf8bef472, 0x835ffcb8, 0x6df4c1f2,
  0x96f5b195, 0xfd0af0fc, 0xb0fe134c, 0xe2506d3d, 0x4f9b12ea, 0xf215f225,
  0xa223736f, 0x9fb4c428, 0x25d04979, 0x34c713f8, 0xc4618187, 0xea7a6e98,
  0x7cd16efc, 0x1436876c, 0xf1544107, 0xbedeee14, 0x56e9af27, 0xa04aa441,
  0x3cf7c899, 0x92ecbae6, 0xdd67016d, 0x151682eb, 0xa842eedf, 0xfdba60b4,
  0xf1907b75, 0x20e3030f, 0x24d8c29e, 0xe139673b, 0xefa63fb8, 0x71873054,
  0xb6f2cf3b, 0x9f326442, 0xcb15a4cc, 0xb01a4504, 0xf1e47d8d, 0x844a1be5,
  0xbae7dfdc, 0x42cbda70, 0xcd7dae0a, 0x57e85b7a, 0xd53f5af6, 0x20cf4d8c,
  0xcea4d428, 0x79d130a4, 0x3486ebfb, 0x33d3cddc, 0x77853b53, 0x37effcb5,
  0xc5068778, 0xe580b3e6, 0x4e68b8f4, 0xc5c8b37e, 0x0d809ea2, 0x398feb7c,
  0x132a4f94, 0x43b7950e, 0x2fee7d1c, 0x223613bd, 0xdd06caa2, 0x37df932b,
  0xc4248289, 0xacf3ebc3, 0x5715f6b7, 0xef3478dd, 0xf267616f, 0xc148cbe4,
  0x9052815e, 0x5e410fab, 0xb48a2465, 0x2eda7fa4, 0xe87b40e4, 0xe98ea084,
  0x5889e9e1, 0xefd390fc, 0xdd07d35b, 0xdb485694, 0x38d7e5b2, 0x57720101,
  0x730edebc, 0x5b643113, 0x94917e4f, 0x503c2fba, 0x646f1282, 0x7523d24a,
  0xe0779695, 0xf9c17a8f, 0x7a5b2121, 0xd187b896, 0x29263a4d, 0xba510cdf,
  0x81f47c9f, 0xad1163ed, 0xea7b5965, 0x1a00726e, 0x11403092, 0x00da6d77,
  0x4a0cdd61, 0xad1f4603, 0x605bdfb0, 0x9eedc364, 0x22ebe6a8, 0xcee7d28a,
  0xa0e736a0, 0x5564a6b9, 0x10853209, 0xc7eb8f37, 0x2de705ca, 0x8951570f,
  0xdf09822b, 0xbd691a6c, 0xaa12e4f2, 0x87451c0f, 0xe0f6a27a, 0x3ada4819,
  0x4cf1764f, 0x0d771c2b, 0x67cdb156, 0x350d8384, 0x5938fa0f, 0x42399ef3,
  0x36997b07, 0x0e84093d, 0x4aa93e61, 0x8360d87b, 0x1fa98b0c, 0x1149382c,
  0xe97625a5, 0x0614d1b7, 0x0e25244b, 0x0c768347, 0x589e8d82, 0x0d2059d1,
  0xa466bb1e, 0xf8da0a82, 0x04f19130, 0xba6e4ec0, 0x99265164, 0x1ee7230d,
  0x50b2ad80, 0xeaee6801, 0x8db2a283, 0xea8bf59e
};

/* Initialize a key schedule from a 128-bit key */

static void
cast_key_sched(sched, key)
     CastKeySched * sched;
     uint8p key;
{
    uint8p x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF;
    uint8p z0, z1, z2, z3, z4, z5, z6, z7, z8, z9, zA, zB, zC, zD, zE, zF;
    uint32 X03, X47, X8B, XCF, Z03, Z47, Z8B, ZCF;

#ifdef LITTLE_ENDIAN
    x3 = (uint8p) &X03;  x2 = x3 + 1;  x1 = x2 + 1;  x0 = x1 + 1;
    x7 = (uint8p) &X47;  x6 = x7 + 1;  x5 = x6 + 1;  x4 = x5 + 1;
    xB = (uint8p) &X8B;  xA = xB + 1;  x9 = xA + 1;  x8 = x9 + 1;
    xF = (uint8p) &XCF;  xE = xF + 1;  xD = xE + 1;  xC = xD + 1;
    z3 = (uint8p) &Z03;  z2 = z3 + 1;  z1 = z2 + 1;  z0 = z1 + 1;
    z7 = (uint8p) &Z47;  z6 = z7 + 1;  z5 = z6 + 1;  z4 = z5 + 1;
    zB = (uint8p) &Z8B;  zA = zB + 1;  z9 = zA + 1;  z8 = z9 + 1;
    zF = (uint8p) &ZCF;  zE = zF + 1;  zD = zE + 1;  zC = zD + 1;
#else
    x0 = (uint8p) &X03;  x1 = x0 + 1;  x2 = x1 + 1;  x3 = x2 + 1;
    x4 = (uint8p) &X47;  x5 = x4 + 1;  x6 = x5 + 1;  x7 = x6 + 1;
    x8 = (uint8p) &X8B;  x9 = x8 + 1;  xA = x9 + 1;  xB = xA + 1;
    xC = (uint8p) &XCF;  xD = xC + 1;  xE = xD + 1;  xF = xE + 1;
    z0 = (uint8p) &Z03;  z1 = z0 + 1;  z2 = z1 + 1;  z3 = z2 + 1;
    z4 = (uint8p) &Z47;  z5 = z4 + 1;  z6 = z5 + 1;  z7 = z6 + 1;
    z8 = (uint8p) &Z8B;  z9 = z8 + 1;  zA = z9 + 1;  zB = zA + 1;
    zC = (uint8p) &ZCF;  zD = zC + 1;  zE = zD + 1;  zF = zE + 1;
#endif

#ifdef LITTLE_ENDIAN
    *x0 = key[0];
    *x1 = key[1];
    *x2 = key[2];
    *x3 = key[3];
    *x4 = key[4];
    *x5 = key[5];
    *x6 = key[6];
    *x7 = key[7];
    *x8 = key[8];
    *x9 = key[9];
    *xA = key[10];
    *xB = key[11];
    *xC = key[12];
    *xD = key[13];
    *xE = key[14];
    *xF = key[15];
#else
    X03 = *(uint32p) key;
    X47 = *(uint32p) (key + 4);
    X8B = *(uint32p) (key + 8);
    XCF = *(uint32p) (key + 12);
#endif

  /* First half of key schedule */

    Z03 = X03 ^ S5[*xD] ^ S6[*xF] ^ S7[*xC] ^ S8[*xE] ^ S7[*x8];
    Z47 = X8B ^ S5[*z0] ^ S6[*z2] ^ S7[*z1] ^ S8[*z3] ^ S8[*xA];
    Z8B = XCF ^ S5[*z7] ^ S6[*z6] ^ S7[*z5] ^ S8[*z4] ^ S5[*x9];
    ZCF = X47 ^ S5[*zA] ^ S6[*z9] ^ S7[*zB] ^ S8[*z8] ^ S6[*xB];

    sched->K[0].Km = S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2];
    sched->K[1].Km = S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6];
    sched->K[2].Km = S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9];
    sched->K[3].Km = S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC];

    X03 = Z8B ^ S5[*z5] ^ S6[*z7] ^ S7[*z4] ^ S8[*z6] ^ S7[*z0];
    X47 = Z03 ^ S5[*x0] ^ S6[*x2] ^ S7[*x1] ^ S8[*x3] ^ S8[*z2];
    X8B = Z47 ^ S5[*x7] ^ S6[*x6] ^ S7[*x5] ^ S8[*x4] ^ S5[*z1];
    XCF = ZCF ^ S5[*xA] ^ S6[*x9] ^ S7[*xB] ^ S8[*x8] ^ S6[*z3];

    sched->K[4].Km = S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8];
    sched->K[5].Km = S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD];
    sched->K[6].Km = S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3];
    sched->K[7].Km = S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7];

    Z03 = X03 ^ S5[*xD] ^ S6[*xF] ^ S7[*xC] ^ S8[*xE] ^ S7[*x8];
    Z47 = X8B ^ S5[*z0] ^ S6[*z2] ^ S7[*z1] ^ S8[*z3] ^ S8[*xA];
    Z8B = XCF ^ S5[*z7] ^ S6[*z6] ^ S7[*z5] ^ S8[*z4] ^ S5[*x9];
    ZCF = X47 ^ S5[*zA] ^ S6[*z9] ^ S7[*zB] ^ S8[*z8] ^ S6[*xB];

    sched->K[8].Km = S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9];
    sched->K[9].Km = S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC];
    sched->K[10].Km = S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2];
    sched->K[11].Km = S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6];

    X03 = Z8B ^ S5[*z5] ^ S6[*z7] ^ S7[*z4] ^ S8[*z6] ^ S7[*z0];
    X47 = Z03 ^ S5[*x0] ^ S6[*x2] ^ S7[*x1] ^ S8[*x3] ^ S8[*z2];
    X8B = Z47 ^ S5[*x7] ^ S6[*x6] ^ S7[*x5] ^ S8[*x4] ^ S5[*z1];
    XCF = ZCF ^ S5[*xA] ^ S6[*x9] ^ S7[*xB] ^ S8[*x8] ^ S6[*z3];

    sched->K[12].Km = S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3];
    sched->K[13].Km = S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7];
    sched->K[14].Km = S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8];
    sched->K[15].Km = S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD];

  /* Second half of key schedule - just like first half */

    Z03 = X03 ^ S5[*xD] ^ S6[*xF] ^ S7[*xC] ^ S8[*xE] ^ S7[*x8];
    Z47 = X8B ^ S5[*z0] ^ S6[*z2] ^ S7[*z1] ^ S8[*z3] ^ S8[*xA];
    Z8B = XCF ^ S5[*z7] ^ S6[*z6] ^ S7[*z5] ^ S8[*z4] ^ S5[*x9];
    ZCF = X47 ^ S5[*zA] ^ S6[*z9] ^ S7[*zB] ^ S8[*z8] ^ S6[*xB];

    sched->K[0].Kr = (S5[*z8] ^ S6[*z9] ^ S7[*z7] ^ S8[*z6] ^ S5[*z2]) & 0x1f;
    sched->K[1].Kr = (S5[*zA] ^ S6[*zB] ^ S7[*z5] ^ S8[*z4] ^ S6[*z6]) & 0x1f;
    sched->K[2].Kr = (S5[*zC] ^ S6[*zD] ^ S7[*z3] ^ S8[*z2] ^ S7[*z9]) & 0x1f;
    sched->K[3].Kr = (S5[*zE] ^ S6[*zF] ^ S7[*z1] ^ S8[*z0] ^ S8[*zC]) & 0x1f;

    X03 = Z8B ^ S5[*z5] ^ S6[*z7] ^ S7[*z4] ^ S8[*z6] ^ S7[*z0];
    X47 = Z03 ^ S5[*x0] ^ S6[*x2] ^ S7[*x1] ^ S8[*x3] ^ S8[*z2];
    X8B = Z47 ^ S5[*x7] ^ S6[*x6] ^ S7[*x5] ^ S8[*x4] ^ S5[*z1];
    XCF = ZCF ^ S5[*xA] ^ S6[*x9] ^ S7[*xB] ^ S8[*x8] ^ S6[*z3];

    sched->K[4].Kr = (S5[*x3] ^ S6[*x2] ^ S7[*xC] ^ S8[*xD] ^ S5[*x8]) & 0x1f;
    sched->K[5].Kr = (S5[*x1] ^ S6[*x0] ^ S7[*xE] ^ S8[*xF] ^ S6[*xD]) & 0x1f;
    sched->K[6].Kr = (S5[*x7] ^ S6[*x6] ^ S7[*x8] ^ S8[*x9] ^ S7[*x3]) & 0x1f;
    sched->K[7].Kr = (S5[*x5] ^ S6[*x4] ^ S7[*xA] ^ S8[*xB] ^ S8[*x7]) & 0x1f;

    Z03 = X03 ^ S5[*xD] ^ S6[*xF] ^ S7[*xC] ^ S8[*xE] ^ S7[*x8];
    Z47 = X8B ^ S5[*z0] ^ S6[*z2] ^ S7[*z1] ^ S8[*z3] ^ S8[*xA];
    Z8B = XCF ^ S5[*z7] ^ S6[*z6] ^ S7[*z5] ^ S8[*z4] ^ S5[*x9];
    ZCF = X47 ^ S5[*zA] ^ S6[*z9] ^ S7[*zB] ^ S8[*z8] ^ S6[*xB];

    sched->K[8].Kr = (S5[*z3] ^ S6[*z2] ^ S7[*zC] ^ S8[*zD] ^ S5[*z9]) & 0x1f;
    sched->K[9].Kr = (S5[*z1] ^ S6[*z0] ^ S7[*zE] ^ S8[*zF] ^ S6[*zC]) & 0x1f;
    sched->K[10].Kr = (S5[*z7] ^ S6[*z6] ^ S7[*z8] ^ S8[*z9] ^ S7[*z2]) & 0x1f;
    sched->K[11].Kr = (S5[*z5] ^ S6[*z4] ^ S7[*zA] ^ S8[*zB] ^ S8[*z6]) & 0x1f;

    X03 = Z8B ^ S5[*z5] ^ S6[*z7] ^ S7[*z4] ^ S8[*z6] ^ S7[*z0];
    X47 = Z03 ^ S5[*x0] ^ S6[*x2] ^ S7[*x1] ^ S8[*x3] ^ S8[*z2];
    X8B = Z47 ^ S5[*x7] ^ S6[*x6] ^ S7[*x5] ^ S8[*x4] ^ S5[*z1];
    XCF = ZCF ^ S5[*xA] ^ S6[*x9] ^ S7[*xB] ^ S8[*x8] ^ S6[*z3];

    sched->K[12].Kr = (S5[*x8] ^ S6[*x9] ^ S7[*x7] ^ S8[*x6] ^ S5[*x3]) & 0x1f;
    sched->K[13].Kr = (S5[*xA] ^ S6[*xB] ^ S7[*x5] ^ S8[*x4] ^ S6[*x7]) & 0x1f;
    sched->K[14].Kr = (S5[*xC] ^ S6[*xD] ^ S7[*x3] ^ S8[*x2] ^ S7[*x8]) & 0x1f;
    sched->K[15].Kr = (S5[*xE] ^ S6[*xF] ^ S7[*x1] ^ S8[*x0] ^ S8[*xD]) & 0x1f;
}

/* Initialize with a full-strength 128-bit key */

#ifndef CAST_EXPORT_ENCRYPTION
void
ck_cast128_key_sched(sched, key)
    CastKeySched * sched;
    uint8 * key;
{
    sched->ksize = 16;
    cast_key_sched(sched, key);
}
#endif

/* Handle reduced-keysize variants */

static void
cast5_key_sched(sched, key, sz)
    CastKeySched * sched;
    uint8 * key;
    int sz;
{
    uint8 buf[16];

    sched->ksize = sz;
    memset(buf, 0, sizeof(buf));
    memcpy(buf, key, sz);
    cast_key_sched(sched, buf);
}

/* 40, 64, and 80-bit keys - all use 12 rounds */

void
ck_cast5_40_key_sched(sched, key)
    CastKeySched * sched;
    uint8 * key;
{
    cast5_key_sched(sched, key, 5);
}

#ifndef CAST_EXPORT_ENCRYPTION
void
ck_cast5_64_key_sched(sched, key)
     CastKeySched * sched;
     uint8 * key;
{
    cast5_key_sched(sched, key, 8);
}

void
ck_cast5_80_key_sched(sched, key)
     CastKeySched * sched;
     uint8 * key;
{
    cast5_key_sched(sched, key, 10);
}
#endif /* CAST_EXPORT_ENCRYPTION */
#endif /* CK_CAST */

#ifdef CRYPT_DLL
static char *
ck_crypt_dll_version()
{
    return(ckcrpv);
}

int
crypt_dll_init( struct _crypt_dll_init * init )
{
#ifdef LIBDES
    extern int des_check_key;
    extern void libdes_dll_init(struct _crypt_dll_init *);
    des_check_key = 1;
#endif /* LIBDES */

    if ( init->version >= 1 ) {
        p_ttol = init->p_ttol;
        p_dodebug = init->p_dodebug;
        p_dohexdump = init->p_dohexdump;
        p_tn_debug = init->p_tn_debug;
        p_vscrnprintf = init->p_vscrnprintf;
        if ( init->version == 1 )
            return(1);
    }
    if ( init->version >= 2 ) {
        /* This is a k5_context but we don't want to include krb5.h */
        p_k5_context = (void *) init->p_k5_context;
        if ( init->version == 2 )
            return(1);
    }
    if ( init->version >= 3 ) {
        init->p_install_funcs("encrypt_parse",encrypt_parse);
        init->p_install_funcs("encrypt_init",encrypt_init);
        init->p_install_funcs("encrypt_session_key",encrypt_session_key);
        init->p_install_funcs("encrypt_send_request_start",
                              encrypt_send_request_start
                              );
        init->p_install_funcs("encrypt_request_start",encrypt_request_start);
        init->p_install_funcs("encrypt_send_request_end",
                              encrypt_send_request_end
                              );
        init->p_install_funcs("encrypt_request_end",encrypt_request_end);
        init->p_install_funcs("encrypt_send_end",encrypt_send_end);
        init->p_install_funcs("encrypt_send_support",encrypt_send_support);
        init->p_install_funcs("encrypt_is_encrypting",encrypt_is_encrypting);
        init->p_install_funcs("encrypt_is_decrypting",encrypt_is_decrypting);
        init->p_install_funcs("get_crypt_table",get_crypt_table);
        init->p_install_funcs("des_is_weak_key",ck_des_is_weak_key);
        libdes_dll_init(init);
        if (init->version == 3)
          return(1);
    }
    if ( init->version >= 4 ) {
        init->p_install_funcs("crypt_dll_version",ck_crypt_dll_version);
        if (init->version == 4)
          return(1);
    }

    if ( init->version >= 5 ) {
        p_reqtelmutex = init->p_reqtelmutex;
        p_reltelmutex = init->p_reltelmutex;
        if (init->version == 5)
          return(1);
    }

    if ( init->version >= 6 ) {
        init->p_install_funcs("encrypt_dont_support",encrypt_dont_support);
        if ( init->version == 6 )
            return(1);
        /* when adding new versions; migrate the next two lines */
        init->version = 6;
        return(1);
    }
    return(0);
}

#undef malloc
#undef realloc
#undef free
#undef strdup

static void
fatal(char *msg) {
    if (!msg) msg = "";

    printf(msg);
    exit(1);        /* Exit indicating failure */
}

void *
kmalloc(size_t size)
{
    void *ptr;

    if (size == 0) {
        fatal("kmalloc: zero size");
    }
    ptr = malloc(size);
    if (ptr == NULL) {
        fatal("kmalloc: out of memory");
    }
    return ptr;
}

void *
krealloc(void *ptr, size_t new_size)
{
    void *new_ptr;

    if (new_size == 0) {
        fatal("krealloc: zero size");
    }
    if (ptr == NULL)
        new_ptr = malloc(new_size);
    else
        new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) {
        fatal("krealloc: out of memory");
    }
    return new_ptr;
}

void
kfree(void *ptr)
{
    if (ptr == NULL) {
        printf("kfree: NULL pointer given as argument");
        return;
    }
    free(ptr);
}

char *
kstrdup(const char *str)
{
    size_t len;
    char *cp;

    if (str == NULL) {
        fatal("kstrdup: NULL pointer given as argument");
    }
    len = strlen(str) + 1;
    cp = kmalloc(len);
    if (cp)
        memcpy(cp, str, len);
    return cp;
}
#endif /* CRYPT_DLL */
#endif /* CK_ENCRYPTION */
