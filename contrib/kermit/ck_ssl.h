/*
  C K _ S S L . H --  OpenSSL Interface Header for C-Kermit

  Copyright (C) 1985, 2005,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

    Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
               Secure Endpoints Inc., New York City
*/

#ifdef CK_SSL
#ifndef CK_ANSIC
#define NOPROTO
#endif /* CK_ANSIC */

#ifdef COMMENT                          /* Not for C-Kermit 7.1 */
#ifdef KRB5
#ifndef NOSSLK5
#ifndef SSL_KRB5
#define SSL_KRB5
#endif /* SSL_KRB5 */
#endif /* NOSSLK5 */
#endif /* KRB5 */
#endif /* COMMENT */

#ifdef OS2
#ifndef ZLIB
#define ZLIB
#endif /* ZLIB */
#endif /* OS2 */

#ifdef ZLIB
#include <openssl/comp.h>
#endif /* ZLIB */
/* We place the following to avoid loading openssl/mdc2.h since it 
 * relies on the OpenSSL des.h.  Since we do not need the MDC2 
 * definitions there is no reason to have it included by openssl/evp.h
 */
#define OPENSSL_NO_MDC2
#ifdef OPENSSL_100
#define OPENSSL_098
#endif	/* OPENSSL_100 */
#ifdef OPENSSL_098
#define OPENSSL_097
#endif /* OPENSSL_098 */
#ifdef CK_DES
#include <openssl/des.h>
#endif	/* CK_DES */
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/blowfish.h>
#include <openssl/dh.h>
#include <openssl/rc4.h>
#include <openssl/cast.h>
#include <openssl/dsa.h>
#include <openssl/rsa.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#ifdef SSL_KRB5
#include <openssl/kssl.h>
#endif /* SSL_KRB5 */

extern BIO *bio_err;
extern SSL *ssl_con;
extern SSL_CTX *ssl_ctx;
extern int ssl_debug_flag;
extern int ssl_only_flag;
extern int ssl_active_flag;
extern int ssl_verify_flag;
extern int ssl_verbose_flag;
extern int ssl_certsok_flag;
extern int ssl_dummy_flag;
extern int ssl_verify_depth;

extern char *ssl_rsa_cert_file;
extern char *ssl_rsa_cert_chain_file;
extern char *ssl_rsa_key_file;
extern char *ssl_dsa_cert_file;
extern char *ssl_dsa_cert_chain_file;
extern char *ssl_dh_key_file;
extern char *ssl_cipher_list;
extern char *ssl_crl_file;
extern char *ssl_crl_dir;
extern char *ssl_verify_file;
extern char *ssl_verify_dir;
extern char *ssl_dh_param_file;
extern char *ssl_rnd_file;

extern SSL_CTX *tls_ctx;
extern SSL *tls_con;
extern int tls_only_flag;
extern int tls_active_flag;
extern int x509_cert_valid;
extern X509_STORE *crl_store;

extern int ssl_raw_flag;
extern int tls_raw_flag;

#ifndef NOHTTP
extern SSL_CTX *tls_http_ctx;
extern SSL *tls_http_con;
extern int tls_http_active_flag;
#endif /* NOHTTP */

extern int ssl_initialized;

_PROTOTYP(VOID ssl_once_init,(void));
_PROTOTYP(int ssl_tn_init,(int));
_PROTOTYP(int ssl_http_init,(char *));
_PROTOTYP(int ck_ssl_http_client,(int,char *));
_PROTOTYP(int ssl_display_connect_details,(SSL *,int,int));
_PROTOTYP(int ssl_server_verify_callback,(int, X509_STORE_CTX *));
_PROTOTYP(int ssl_client_verify_callback,(int, X509_STORE_CTX *));
_PROTOTYP(int ssl_reply,(int, unsigned char *, int));
_PROTOTYP(int ssl_is,(unsigned char *, int));
_PROTOTYP(int ck_ssl_incoming,(int));
_PROTOTYP(int ck_ssl_outgoing,(int));
_PROTOTYP(int tls_is_user_valid,(SSL *, const char *));
_PROTOTYP(char * ssl_get_dnsName,(SSL *));
_PROTOTYP(char * ssl_get_commonName,(SSL *));
_PROTOTYP(char * ssl_get_issuer_name,(SSL *));
_PROTOTYP(char * ssl_get_subject_name,(SSL *));
_PROTOTYP(int ssl_get_client_finished,(char *, int));
_PROTOTYP(int ssl_get_server_finished,(char *, int));
_PROTOTYP(int ssl_passwd_callback,(char *, int, int, VOID *));
_PROTOTYP(VOID ssl_client_info_callback,(const SSL *,int, int));
_PROTOTYP(int ssl_anonymous_cipher,(SSL * ssl));
_PROTOTYP(int tls_load_certs,(SSL_CTX * ctx, SSL * con, int server));
_PROTOTYP(int ssl_verify_crl,(int, X509_STORE_CTX *));
_PROTOTYP(int tls_is_krb5,(int));
_PROTOTYP(int X509_userok,(X509 *,const char *));
_PROTOTYP(int ck_X509_save_cert_to_user_store,(X509 *));
/* SMS 2007/02/15 */
_PROTOTYP(int ssl_check_server_name,(SSL * ssl, char * hostname));
#ifdef OS2
#include "ckosslc.h"
#include "ckossl.h"
#endif /* OS2 */

#define SSL_CLIENT 0
#define SSL_SERVER 1
#define SSL_HTTP   2

#define SSL_ERR_BFSZ 4096

#ifdef SSL_KRB5
#define DEFAULT_CIPHER_LIST "HIGH:MEDIUM:LOW:+KRB5:+ADH:+EXP"
#else
#define DEFAULT_CIPHER_LIST "HIGH:MEDIUM:LOW:+ADH:+EXP"
#endif /* SSL_KRB5 */
#endif /* CK_SSL */
