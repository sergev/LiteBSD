/*  C K U A T H . H --  "C-Kermit to Authentication" interface  */

/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1999, 2009,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.
*/

/*
 * Based on a concatenation of all necessary include files distributed with
 * the Kerberos 5 NT Alpha 2 Telnet package from MIT.
 */

#ifndef KRB5_KERMIT_H
#define KRB5_KERMIT_H

#ifndef BOOL
#define BOOL int
#endif

/* Header file for encrypted-stream library.
 * Written by Ken Raeburn (Raeburn@Cygnus.COM).
 * Copyright (C) 1991, 1992, 1994 by Cygnus Support.
 *
 * Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Cygnus Support makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#ifndef K5STREAM_H
#define K5STREAM_H

typedef void *kstream_ptr;                      /* Data send on the kstream */
struct kstream_data_block {
  kstream_ptr ptr;
  size_t length;
};

typedef struct kstream_int {                    /* Object we pass around */
    int fd;                                       /* Open socket descriptor */
    int (*encrypt)(struct kstream_data_block *, /* output */
                   struct kstream_data_block *); /* input */
    int encrypt_type;
    int (*decrypt)(struct kstream_data_block *, /* output */
                   struct kstream_data_block *); /* input */
    int decrypt_type;
} *kstream;

/* Prototypes */

int  kstream_destroy();
void kstream_set_buffer_mode(int);
int  kstream_create_from_fd(int fd, kstream_ptr);
int kstream_write(void *, size_t);
int kstream_read(void *, size_t);

#endif /* K5STREAM_H */

/*
 * Implements Telnet authentication and encryption
 */

#ifndef TELNET_AUTH_H
#define TELNET_AUTH_H

int auth_parse(unsigned char *, int);

int auth_init(kstream);

void auth_destroy(void);

int auth_encrypt(struct kstream_data_block *, struct kstream_data_block *);

int auth_decrypt(struct kstream_data_block *, struct kstream_data_block *);

extern BOOL forward_flag;
extern BOOL forwardable_flag;
extern BOOL forwarded_tickets;
#endif /* TEL_AUTH_H */


/* C-Kermit specific functions */
_PROTOTYP(void auth_finished,(int));
_PROTOTYP(int ck_auth_init, (char *, char *, char *, int));
_PROTOTYP(int ck_tn_auth_valid, (VOID));
_PROTOTYP(int ck_tn_auth_in_progress,(VOID));
_PROTOTYP(int ck_tn_sb_auth, (char *, int));
_PROTOTYP(int ck_tn_sb_encrypt, (char *, int));
_PROTOTYP(int ck_tn_auth_request, (VOID));
_PROTOTYP(void ck_tn_encrypt, (char *, int));
_PROTOTYP(void ck_tn_decrypt, (char *, int));
_PROTOTYP(void ck_tn_encrypt_start, (VOID));
_PROTOTYP(void ck_tn_encrypt_stop, (VOID));
_PROTOTYP(int ck_tn_authenticated, (VOID));
#ifdef CK_ENCRYPTION
_PROTOTYP(int ck_tn_encrypting, (VOID));
_PROTOTYP(int ck_tn_decrypting, (VOID));
#endif /* CK_ENCRYPTION */
#ifdef CK_SSL
_PROTOTYP(int ck_tn_tls_negotiate, (VOID));
_PROTOTYP(int SendSSLAuthSB, (int, void *, int));
#endif /* CK_SSL */

#ifdef CK_KERBEROS
    /* Define MIT_CURRENT to compile the code for use with versions of */
    /* Kerberos later than KRB5 1.0.5.  Note.  This will not compile   */
    /* successfully in Kermit 95 due to the segmentation of crypto     */
    /* into a separate DLL.                                            */

#ifndef KRB5_INIT_ETS
/* krb5_init_ets() is a no-op in Kerberos 1.4.x and later */
/* and in some installations it can't be found so now by default */
/* we don't use it. */
#define NO_KRB5_INIT_ETS
#endif	/* KRB5_INIT_ETS */

#define KRB_DEFTIM 600                  /* Default lifetime (minutes) */

/* Kerberos structure definitions */

struct krb_op_data {                    /* Operational data for all actions */
    int version;                        /* Kerberos version    */
    char * cache;                       /* Kerberos cache file */
};

struct krb4_init_data {                 /* INITIALIZE data structure */
    int lifetime;
    char * principal;                   /* Principal string */
    char * instance;
    char * realm;                       /* Realm string */
    char * password;                    /* Kerberos password */
    int    preauth;                     /* Use preauth mode? */
    int    verbose;                     /* Verbose output? */
};

#define KRB5_NUM_OF_ADDRS 16
struct krb5_init_data {                 /* INITIALIZE data structure */
    int forwardable;                    /* Switch values */
    int proxiable;                      /* Correspond to switch names... */
    int lifetime;
    int renew;
    int renewable;
    int validate;
    char * postdate;
    char * service;
    char * principal;                   /* Principal string */
    char * instance;
    char * realm;                       /* Realm string */
    char * password;                    /* Kerberos password */
    int    preauth;                     /* Use preauth mode? */
    int    verbose;                     /* Verbose output? */
    int    getk4;                       /* Get K4 TGT? */
    char * addrs[KRB5_NUM_OF_ADDRS+1];  /* List of IP Addresses */
    int  no_addresses;                  /* Do not include IP Addresses */
};

struct krb5_list_cred_data {            /* List Credentials data */
    int encryption;
    int flags;
    int addr;
};

_PROTOTYP(int ck_krb5_autoget_TGT, (char *));
_PROTOTYP(int ck_krb5_initTGT, (struct krb_op_data *,struct krb5_init_data *,
                                 struct krb4_init_data *));
_PROTOTYP(int ck_krb5_destroy, (struct krb_op_data *));
_PROTOTYP(int ck_krb5_list_creds, (struct krb_op_data *,
                                    struct krb5_list_cred_data *));
_PROTOTYP(char * ck_krb5_getrealm, (char *));
_PROTOTYP(char * ck_krb5_getprincipal, (char *));
_PROTOTYP(char * ck_krb5_get_cc_name, (VOID));

_PROTOTYP(int ck_krb4_autoget_TGT, (char *));
_PROTOTYP(int ck_krb4_initTGT, (struct krb_op_data *,struct krb4_init_data *));
_PROTOTYP(int ck_krb4_destroy, (struct krb_op_data *));
_PROTOTYP(int ck_krb4_list_creds, (struct krb_op_data *));
_PROTOTYP(char * ck_krb4_getrealm, (VOID));
_PROTOTYP(char * ck_krb4_getprincipal, (VOID));

_PROTOTYP(int ck_krb4_get_tkts, (VOID));
_PROTOTYP(char * ck_krb4_get_next_tkt, (VOID));
_PROTOTYP(int ck_krb4_tkt_isvalid,(char *));
_PROTOTYP(int ck_krb4_is_tgt_valid,(VOID));
_PROTOTYP(int ck_krb4_tkt_time,(char *));

_PROTOTYP(int ck_krb5_get_tkts, (char *));
_PROTOTYP(char * ck_krb5_get_next_tkt, (VOID));
_PROTOTYP(int ck_krb5_tkt_isvalid,(char *,char *));
_PROTOTYP(char * ck_krb5_tkt_flags,(char *,char *));
_PROTOTYP(int ck_krb5_is_tgt_valid,(VOID));
_PROTOTYP(int ck_krb5_tkt_time,(char *,char *));

_PROTOTYP(int krb4_des_avail,(int));
_PROTOTYP(int krb4_des_write,(int,char *,int));
_PROTOTYP(int krb4_des_read, (int,char *,int));
_PROTOTYP(int krb5_des_avail,(int));
_PROTOTYP(int krb5_des_write,(int,char *,int,int));
_PROTOTYP(int krb5_des_read, (int,char *,int,int));
_PROTOTYP(int krb5_u2u_avail,(int));
_PROTOTYP(int krb5_u2u_write,(int,char *,int));
_PROTOTYP(int krb5_u2u_read, (int,char *,int));
_PROTOTYP(int k5_user_to_user_server_auth,(VOID));
_PROTOTYP(int k5_user_to_user_client_auth,(VOID));
#endif /* CK_KERBEROS */

_PROTOTYP(int ck_krb5_is_installed,(void));
_PROTOTYP(int ck_krb4_is_installed,(void));
_PROTOTYP(int ck_srp_is_installed,(void));
_PROTOTYP(int ck_ntlm_is_installed,(void));
_PROTOTYP(int ck_crypt_is_installed,(void));
_PROTOTYP(int ck_ssleay_is_installed,(void));
_PROTOTYP(int ck_gssapi_is_installed,(void));
_PROTOTYP(int ck_krypto_is_installed,(void));

_PROTOTYP(VOID ck_encrypt_send_support,(VOID));
_PROTOTYP(int ck_get_crypt_table,(struct keytab **, int *));
_PROTOTYP(char * ck_krb4_realmofhost,(char *));
_PROTOTYP(char * ck_krb5_realmofhost,(char *));

#define FORWARD  /* allow forwarding of credential */
#ifdef FORWARD
_PROTOTYP(int kerberos5_forward,(VOID));
#endif /* FORWARD */

#define AUTHTYPLSTSZ 8
#endif /*KRB5_KERMIT_H*/
