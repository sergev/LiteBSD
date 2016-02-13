/*
  C K U A T 2 . H  --  Kerberos headers for C-Kermit

  Copyright (C) 1985, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

  Author:
    Kerberos IV and V intergration.
    Jeffrey E Altman (jaltman@secure-endpoints.com)
    Secure Endpoints Inc., New York City
*/
/*
 * Based on a concatenation of all necessary include files distributed with
 * the Kerberos 5 NT Alpha 2 Telnet package from MIT.
 */

#ifndef KRB5_TELNET_H
#define KRB5_TELNET_H
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
 *
 *      @(#)encrypt.h   8.1 (Berkeley) 6/4/93
 */

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

#ifdef  CK_ENCRYPTION

#ifndef __ENCRYPTION__
#define __ENCRYPTION__

#define DIR_DECRYPT             1
#define DIR_ENCRYPT             2

#ifndef CK_DES_C
#ifndef NOBLOCKDEF
typedef unsigned char Block[8];
#endif /* NOBLOCKDEF */
typedef unsigned char *BlockT;
#ifndef KRB4    /* already defined in kerberosiv/des.h */
typedef struct des_ks_struct { Block _; } Schedule[16];
#else /* KRB4 */
#ifndef OS2
#ifndef NOBLOCKDEF    /* already defined in kerberosiv/des.h */
typedef struct des_ks_struct { Block _; } Schedule[16];
#endif /* NOBLOCKDEF */
#endif /* OS2 */
#endif /* KRB4 */

#define VALIDKEY(key) (key[0]|key[1]|key[2]|key[3]|key[4]|key[5]|key[6]|key[7])

#define SAMEKEY(k1, k2) (!memcmp((void *)k1, (void *)k2, sizeof(Block)))
#endif /* CK_DES_C */

typedef struct _session_key {
  short         type;
  int           length;
  unsigned char *data;
} Session_Key;

#ifdef __STDC__
typedef struct {
  char  *name;
  int   type;
  void  (*output)(unsigned char *, int);
  int   (*input)(int);
  void  (*init)(int);
  int   (*start)(int, int);
  int   (*is)(unsigned char *, int);
  int   (*reply)(unsigned char *, int);
  int   (*session)(Session_Key *, int);
  int   (*keyid)(int, unsigned char *, int *);
  void  (*printsub)(unsigned char *, int, unsigned char *, int);
} Encryptions;
#if !defined(P)
#define P(x) x
#endif
#else
typedef struct {
  char  *name;
  int   type;
  void  (*output)();
  int   (*input)();
  void  (*init)();
  int   (*start)();
  int   (*is)();
  int   (*reply)();
  int   (*session)();
  int   (*keyid)();
  void  (*printsub)();
} Encryptions;
#if !defined(P)
#define P(x) ()
#endif
#endif

int encrypt_parse(unsigned char *, int);

#ifdef DEBUG
int printsub(char, unsigned char *, size_t);
#endif

#define SK_GENERIC      0       /* Just a string of bits */
#define SK_DES          1       /* Matched Kerberos v5 ENCTYPE_DES */

void encrypt_init P((kstream,int));
Encryptions *findencryption P((int));
void encrypt_send_support P((void));
void encrypt_auto P((int));
void decrypt_auto P((int));
int  encrypt_is P((unsigned char *, int));
int  encrypt_reply P((unsigned char *, int));
void encrypt_start_input P((int));
int  encrypt_session_key P((Session_Key *, int));
int  encrypt_dont_support P((int));
void encrypt_end_input P((void));
void encrypt_start_output P((int));
void encrypt_end_output P((void));
void encrypt_send_request_start P((void));
void encrypt_send_request_end P((void));
void encrypt_send_end P((void));
void encrypt_wait P((void));
int  encrypt_is_encrypting P((void));
void encrypt_send_support P((void));
int  encrypt_send_keyid P((int, unsigned char *, int, int));

int encrypt_cmd P((int, char **));
void encrypt_display P((void));

#ifdef CK_KERBEROS
void krbdes_encrypt P((unsigned char *, int));
int  krbdes_decrypt P((int));
int  krbdes_is P((unsigned char *, int));
int  krbdes_reply P((unsigned char *, int));
void krbdes_init P((int));
int  krbdes_start P((int, int));
void krbdes_session P((Session_Key *, int));
void krbdes_printsub P((unsigned char *, int, unsigned char *, int));
#endif /* CK_KERBEROS */

void cfb64_encrypt P((unsigned char *, int));
int cfb64_decrypt P((int));
void cfb64_init P((int));
int cfb64_start P((int, int));
int cfb64_is P((unsigned char *, int));
int cfb64_reply P((unsigned char *, int));
int cfb64_session P((Session_Key *, int));
int cfb64_keyid P((int, unsigned char *, int *));
void cfb64_printsub P((unsigned char *, int, unsigned char *, int));

void ofb64_encrypt P((unsigned char *, int));
int ofb64_decrypt P((int));
void ofb64_init P((int));
int ofb64_start P((int, int));
int ofb64_is P((unsigned char *, int));
int ofb64_reply P((unsigned char *, int));
int ofb64_session P((Session_Key *, int));
int ofb64_keyid P((int, unsigned char *, int *));
void ofb64_printsub P((unsigned char *, int, unsigned char *, int));

void des3_cfb64_encrypt P((unsigned char *, int));
int  des3_cfb64_decrypt P((int));
void des3_cfb64_init P((int));
int  des3_cfb64_start P((int, int));
int  des3_cfb64_is P((unsigned char *, int));
int  des3_cfb64_reply P((unsigned char *, int));
int  des3_cfb64_session P((Session_Key *, int));
int  des3_cfb64_keyid P((int, unsigned char *, int *));
void des3_cfb64_printsub P((unsigned char *, int, unsigned char *, int));

void des3_ofb64_encrypt P((unsigned char *, int));
int  des3_ofb64_decrypt P((int));
void des3_ofb64_init P((int));
int  des3_ofb64_start P((int, int));
int  des3_ofb64_is P((unsigned char *, int));
int  des3_ofb64_reply P((unsigned char *, int));
int  des3_ofb64_session P((Session_Key *, int));
int  des3_ofb64_keyid P((int, unsigned char *, int *));
void des3_ofb64_printsub P((unsigned char *, int, unsigned char *, int));

#ifdef CAST_ENCRYPTION
void cast_cfb64_encrypt P((unsigned char *, int));
int  cast_cfb64_decrypt P((int));
void cast_cfb64_init P((int));
int  cast_cfb64_start P((int, int));
int  cast_cfb64_is P((unsigned char *, int));
int  cast_cfb64_reply P((unsigned char *, int));
int  cast_cfb64_session P((Session_Key *, int));
int  cast_cfb64_keyid P((int, unsigned char *, int *));
void cast_cfb64_printsub P((unsigned char *, int, unsigned char *, int));

void cast_ofb64_encrypt P((unsigned char *, int));
int  cast_ofb64_decrypt P((int));
void cast_ofb64_init P((int));
int  cast_ofb64_start P((int, int));
int  cast_ofb64_is P((unsigned char *, int));
int  cast_ofb64_reply P((unsigned char *, int));
int  cast_ofb64_session P((Session_Key *, int));
int  cast_ofb64_keyid P((int, unsigned char *, int *));
void cast_ofb64_printsub P((unsigned char *, int, unsigned char *, int));

void castexp_cfb64_encrypt P((unsigned char *, int));
int  castexp_cfb64_decrypt P((int));
void castexp_cfb64_init P((int));
int  castexp_cfb64_start P((int, int));
int  castexp_cfb64_is P((unsigned char *, int));
int  castexp_cfb64_reply P((unsigned char *, int));
int  castexp_cfb64_session P((Session_Key *, int));
int  castexp_cfb64_keyid P((int, unsigned char *, int *));
void castexp_cfb64_printsub P((unsigned char *, int, unsigned char *, int));

void castexp_ofb64_encrypt P((unsigned char *, int));
int  castexp_ofb64_decrypt P((int));
void castexp_ofb64_init P((int));
int  castexp_ofb64_start P((int, int));
int  castexp_ofb64_is P((unsigned char *, int));
int  castexp_ofb64_reply P((unsigned char *, int));
int  castexp_ofb64_session P((Session_Key *, int));
int  castexp_ofb64_keyid P((int, unsigned char *, int *));
void castexp_ofb64_printsub P((unsigned char *, int, unsigned char *, int));
#endif /* CAST_ENCRYPTION */

/* int  des_string_to_key P((char *, Block)); */

#ifdef DEBUG
extern int encrypt_debug_mode;
#endif

int decrypt_ks_hack(unsigned char *, int);

#endif /* __ENCRYPTION__ */
#endif /* ENCRYPTION */

#ifdef CRYPT_DLL
struct _crypt_dll_init {
    int version;

    /* Version 1 variables */
    int (*p_ttol)(char *,int);
    int (*p_dodebug)(int,char *,char *,CK_OFF_T);
    int (*p_dohexdump)(char *,char *,int);
    void (*p_tn_debug)(char *);
    int (*p_vscrnprintf)(char *, ...);

    /* Version 2 variables */
    void * p_k5_context;

    /* Version 3 variables */
    void (*p_install_funcs)(char *,void *);

    /* Version 5 variables */
    unsigned long (*p_reqtelmutex)(unsigned long);
    unsigned long (*p_reltelmutex)(void);
};
#endif /* CRYPT_DLL */

/* per Kerberos v5 protocol spec */
#ifndef ENCTYPE_NULL
#define ENCTYPE_NULL            0x0000
#endif
#ifndef  ENCTYPE_DES_CBC_CRC
#define ENCTYPE_DES_CBC_CRC     0x0001  /* DES cbc mode with CRC-32 */
#endif
#ifndef  ENCTYPE_DES_CBC_MD4
#define ENCTYPE_DES_CBC_MD4     0x0002  /* DES cbc mode with RSA-MD4 */
#endif
#ifndef  ENCTYPE_DES_CBC_MD5
#define ENCTYPE_DES_CBC_MD5     0x0003  /* DES cbc mode with RSA-MD5 */
#endif
#ifndef  ENCTYPE_DES_CBC_RAW
#define ENCTYPE_DES_CBC_RAW     0x0004  /* DES cbc mode raw */
#endif
/* XXX deprecated? */
#ifndef  ENCTYPE_DES3_CBC_SHA
#define ENCTYPE_DES3_CBC_SHA    0x0005  /* DES-3 cbc mode with NIST-SHA */
#endif
#ifndef  ENCTYPE_DES3_CBC_RAW
#define ENCTYPE_DES3_CBC_RAW    0x0006  /* DES-3 cbc mode raw */
#endif
#ifndef  ENCTYPE_DES_HMAC_SHA1
#define ENCTYPE_DES_HMAC_SHA1   0x0008
#endif
#ifndef  ENCTYPE_DES3_CBC_SHA1
#define ENCTYPE_DES3_CBC_SHA1  0x0010
#endif
#ifndef ENCTYPE_AES128_CTS_HMAC_SHA1_96
#define ENCTYPE_AES128_CTS_HMAC_SHA1_96  0x0011
#endif
#ifndef ENCTYPE_AES256_CTS_HMAC_SHA1_96
#define ENCTYPE_AES256_CTS_HMAC_SHA1_96  0x0012
#endif
#ifndef ENCTYPE_ARCFOUR_HMAC
#define ENCTYPE_ARCFOUR_HMAC     0x0017
#endif
#ifndef ENCTYPE_ARCFOUR_HMAC_EXP
#define ENCTYPE_ARCFOUR_HMAC_EXP 0x0018
#endif
#ifndef ENCTYPE_LOCAL_RC4_MD4
#define ENCTYPE_LOCAL_RC4_MD4    0xFFFFFF80
#endif
#ifndef  ENCTYPE_UNKNOWN
#define ENCTYPE_UNKNOWN         0x01ff
#endif
/* local crud */
/* marc's DES-3 with 32-bit length */
#ifndef  ENCTYPE_LOCAL_DES3_HMAC_SHA1
#define ENCTYPE_LOCAL_DES3_HMAC_SHA1 0x7007
#endif
#endif /* KRB5_TELNET_H */
