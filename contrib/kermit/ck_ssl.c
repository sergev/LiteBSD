char *cksslv = "SSL/TLS support, 9.0.227, 04 Aug 2010";
/*
  C K _ S S L . C --  OpenSSL Interface for C-Kermit

  Copyright (C) 1985, 2010,
    Trustees of Columbia University in the City of New York.
    All rights reserved.  See the C-Kermit COPYING.TXT file or the
    copyright text in the ckcmai.c module for disclaimer and permissions.

    Author:  Jeffrey E Altman (jaltman@secure-endpoints.com)
               Secure Endpoints Inc., New York City

  Provides:

  . Telnet Auth SSL option compatible with Tim Hudson's hack.
  . Telnet START_TLS option
  . Configuration of certificate and key files
  . Certificate verification and revocation list checks
  . Client certificate to user id routine

  Note: This code is written to be compatible with OpenSSL 0.9.6[abcdefgh]
  and 0.9.7 beta 5 (and, presumably, later).
  It will also compile with version 0.9.5 although that is discouraged
  due to security weaknesses in that release.
*/

#include "ckcsym.h"
#include "ckcdeb.h"

#ifdef CK_SSL
#include "ckcnet.h"
#include "ckuath.h"

#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#include <netinet/in.h>
#ifndef FREEBSD4
#include <arpa/inet.h>
#endif /* FREEBSD4 */
#endif /* UNIX */

#ifdef DEC_TCPIP
#include <time.h>
#include <inet.h>
#endif /* DEC_TCPIP */

#ifdef OS2
extern char exedir[];
#ifdef NT
char * GetAppData(int);
#endif
#endif /* OS2 */

extern int quiet;			/* fdc - Mon Nov 28 11:44:15 2005 */

static int ssl_installed = 1;
#endif /* CK_SSL */
int
ck_ssh_is_installed()
{
#ifdef SSHBUILTIN
#ifdef SSLDLL
#ifdef NT
    extern HINSTANCE hCRYPTO;
#else /* NT */
    extern HMODULE hCRYPTO;
#endif /* NT */
    debug(F111,"ck_ssh_is_installed","hCRYPTO",hCRYPTO);
    return(ssl_installed && (hCRYPTO != NULL));
#else /* SSLDLL */
    return(ssl_installed);
#endif /* SSLDLL */
#else
    return 0;
#endif
}

int
#ifdef CK_ANSIC
ck_ssleay_is_installed(void)
#else
ck_ssleay_is_installed()
#endif
{
#ifdef CK_SSL
#ifdef SSLDLL
#ifdef NT
    extern HINSTANCE hSSL, hCRYPTO;
#else /* NT */
    extern HMODULE hSSL, hCRYPTO;
#endif /* NT */
    debug(F111,"ck_ssleay_is_installed","hSSL",hSSL);
    debug(F111,"ck_ssleay_is_installed","hCRYPTO",hCRYPTO);
    return(ssl_installed && (hSSL != NULL) && (hCRYPTO != NULL));
#else /* SSLDLL */
    return(ssl_installed);
#endif /* SSLDLL */
#else /* CK_SSL */
    return(0);
#endif /* CK_SSL */
}

#ifdef CK_SSL
#include "ckcker.h"
#include "ckucmd.h"                             /* For struct keytab */
#include "ckctel.h"
#include "ck_ssl.h"
#ifdef UNIX
#include <pwd.h>                    /* Password file for home directory */
#endif /* UNIX */
#ifdef OS2
#include <process.h>
#endif /* OS2 */
#ifdef OS2ONLY
#include "ckotcp.h"
#endif /* OS2ONLY */

#ifdef SSLDLL
int ssl_finished_messages = 0;
#else /* SSLDLL */
#ifdef OPENSSL_VERSION_NUMBER
int ssl_finished_messages = (OPENSSL_VERSION_NUMBER >= 0x0090581fL);
#else
!ERROR This module requires OpenSSL 0.9.5a or higher
#endif /* OPENSSL_VERSION_NUMBER */
#endif /* SSLDLL */

static int auth_ssl_valid = 0;
static char *auth_ssl_name = 0;    /* this holds the oneline name */
char ssl_err[SSL_ERR_BFSZ]="";

BIO *bio_err=NULL;
X509_STORE *crl_store = NULL;

#ifndef NOFTP
#ifndef SYSFTP
SSL *ssl_ftp_con             = NULL;
SSL_CTX *ssl_ftp_ctx         = NULL;
SSL *ssl_ftp_data_con        = NULL;
int ssl_ftp_active_flag      = 0;
int ssl_ftp_data_active_flag = 0;
#endif /* SYSFTP */
#endif /* NOFTP */

#ifndef NOHTTP
SSL *tls_http_con            = NULL;
SSL_CTX *tls_http_ctx        = NULL;
int tls_http_active_flag     = 0;
int ssl_http_initialized = 0;
#endif /* NOHTTP */

SSL_CTX *ssl_ctx = NULL;
SSL *ssl_con = NULL;
int ssl_debug_flag = 0;
int ssl_verbose_flag = 0;
int ssl_only_flag = 0;
int ssl_raw_flag = 0;
int ssl_active_flag = 0;
int ssl_verify_flag = SSL_VERIFY_PEER;
int ssl_certsok_flag = 0;
char *ssl_rsa_cert_file = NULL;
char *ssl_rsa_cert_chain_file = NULL;
char *ssl_rsa_key_file = NULL;
char *ssl_dsa_cert_file = NULL;
char *ssl_dsa_cert_chain_file = NULL;
char *ssl_dh_key_file = NULL;
char *ssl_crl_file = NULL;
char *ssl_crl_dir = NULL;
char *ssl_verify_file = NULL;
char *ssl_verify_dir = NULL;
char *ssl_dh_param_file = NULL;
char *ssl_cipher_list = NULL;
char *ssl_rnd_file = NULL;

SSL_CTX *tls_ctx = NULL;
SSL *tls_con = NULL;
int tls_only_flag = 0;
int tls_raw_flag = 0;
int tls_active_flag = 0;

int ssl_initialized = 0;
int ssl_verify_depth = -1; /* used to track depth in verify routines */

/* compile this set to 1 to negotiate SSL/TLS but not actually start it */
int ssl_dummy_flag=0;

extern int inserver;
extern int debses;
extern int accept_complete;
extern char szHostName[], szUserNameRequested[], szUserNameAuthenticated[];

_PROTOTYP(int X509_to_user,(X509 *, char *, int));

static int verbosity = 0;		/* Message control */
static VOID
setverbosity() {
    verbosity = ssl_verbose_flag;
    if (quiet) verbosity = 0;
}

int
#ifdef CK_ANSIC
ssl_server_verify_callback(int ok, X509_STORE_CTX * ctx)
#else /* CK_ANSIC */
ssl_server_verify_callback(ok, ctx)
int ok;
X509_STORE_CTX *ctx;
#endif /* CK_ANSIC */
{
    static char *saved_subject=NULL;
    char *subject=NULL, *issuer=NULL;
    int depth,error;
    X509 *xs = NULL;

    if ( ssl_certsok_flag )
        return(1);

    setverbosity();

    error=X509_STORE_CTX_get_error(ctx);
    depth=X509_STORE_CTX_get_error_depth(ctx);
    xs=X509_STORE_CTX_get_current_cert(ctx);

    if (depth==0) {
        /* clear things */
        if (saved_subject!=NULL) {
            free(saved_subject);
            saved_subject=NULL;
        }
        if (auth_ssl_name!=NULL) {
            free(auth_ssl_name);
            auth_ssl_name=NULL;
        }
    }


    if (ssl_debug_flag && !inserver) {
        printf("ssl:server_verify_callback:depth=%d ok=%d err=%d-%s\r\n",
            depth,ok,error,X509_verify_cert_error_string(error));
    }

    /* first thing is to have a meaningful name for the current
     * certificate that is being verified ... and if we cannot
     * determine that then something is seriously wrong!
     */
    makestr(&subject,
            (char *)X509_NAME_oneline(X509_get_subject_name(xs),NULL,0));
    makestr(&issuer,
            (char *)X509_NAME_oneline(X509_get_issuer_name(xs),NULL,0));
    if (!subject || !subject[0] || !issuer || !issuer[0]) {
        ok = 0;
        goto return_time;
    }

    if (verbosity && !inserver && depth != ssl_verify_depth) {
        printf("[%d] Certificate Subject:\r\n%s\r\n",depth,subject);
        printf("[%d] Certificate Issuer:\r\n%s\r\n",depth,issuer);
        ssl_verify_depth = depth;
    }

    /* make sure that the certificate that has been presented */
    /* has not been revoked (if we have been given a CRL.     */
    ok =  ssl_verify_crl(ok, ctx);

    /* if we have any form of error in secure mode we reject the connection */
    if (error!=X509_V_OK) {
        if (inserver) {
#ifdef CKSYSLOG
            if (ckxsyslog >= SYSLG_LI && ckxlogging) {
                cksyslog(SYSLG_LI, 0,
                          "X.509 Certificate verify failure",
                          (char *) subject,
                          (char *)X509_verify_cert_error_string(error)
                          );
            }
#endif /* CKSYSLOG */

        } else {
            if ( ssl_verify_flag &
                 (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT))
                printf("Error: ");
            else
                printf("Warning: ");
            switch (error) {
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                printf("Certificate is self signed.\r\n");
                break;
            case X509_V_ERR_CERT_HAS_EXPIRED:
                printf("Certificate has expired.\r\n");
                break;
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                printf(
  "Certificate issuer's certificate isn't available locally.\r\n");
                break;
            case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
                printf("Unable to verify leaf signature.\r\n");
                break;
            case X509_V_ERR_CERT_REVOKED:
                printf("Certificate revoked.\r\n");
                break;
            default:
                printf("Error %d while verifying certificate.\r\n",
                       ctx->error);
                break;
            }
        }
        ok = !(ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
    } else {
        /* if we got all the way to the top of the tree then
         * we *can* use this certificate for a username to
         * match ... in all other cases we must not!
         */
        auth_ssl_name = saved_subject;
        saved_subject = NULL;
    }

  return_time:

    /* save the name if at least the first level is okay */
    if (depth == 0 && ok)
        makestr(&saved_subject,subject);

    /* clean up things */
    if (subject!=NULL)
        free(subject);
    if (issuer!=NULL)
        free(issuer);

    return ok;
}

int
#ifdef CK_ANSIC
ssl_client_verify_callback(int ok, X509_STORE_CTX * ctx)
#else
ssl_client_verify_callback(ok, ctx)
int ok;
X509_STORE_CTX *ctx;
#endif
{
    char subject[256]="", issuer[256]="";
    int depth, error, len;
    X509 *xs;

    setverbosity();

    xs=X509_STORE_CTX_get_current_cert(ctx);
    error=X509_STORE_CTX_get_error(ctx);
    depth=X509_STORE_CTX_get_error_depth(ctx);

    if ( ssl_debug_flag )
        printf("ssl:client_verify_callback:depth=%d ok=%d err=%d-%s\r\n",
                depth,ok,error,X509_verify_cert_error_string(error));

    if ( ssl_certsok_flag ) {
        ok = 1;
    }

    /* first thing is to have a meaningful name for the current
     * certificate that is being verified ... and if we cannot
     * determine that then something is seriously wrong!
     */
#ifdef XN_FLAG_SEP_MULTILINE
    X509_NAME_print_ex(bio_err,X509_get_subject_name(xs),4,
                        XN_FLAG_SEP_MULTILINE);
    len = BIO_read(bio_err,subject,256);
    subject[len < 256 ? len : 255] = '\0';
    if (!subject[0]) {
        ERR_print_errors(bio_err);
        len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
        ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
        uq_ok("X.509 Subject Name unavailable", ssl_err, 1, NULL, 0);
        ok=0;
        goto return_time;
    }

    X509_NAME_print_ex(bio_err,X509_get_issuer_name(xs),4,
                        XN_FLAG_SEP_MULTILINE);
    len = BIO_read(bio_err,issuer,256);
    issuer[len < 256 ? len : 255] = '\0';
    if (!issuer[0]) {
        ERR_print_errors(bio_err);
        len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
        ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
        uq_ok("X.509 Issuer Name unavailable", ssl_err, 1, NULL, 0);
        ok=0;
        goto return_time;
    }
#else /* XN_FLAG_SEP_MULTILINE */
    X509_NAME_oneline(X509_get_subject_name(xs),subject,256);
    if (!subject[0]) {
        int len;
        ERR_print_errors(bio_err);
        len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
        ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
        uq_ok("X.509 Subject Name unavailable", ssl_err, 1, NULL, 0);
        ok=0;
        goto return_time;
    }

    X509_NAME_oneline(X509_get_issuer_name(xs),issuer,256);
    if (!issuer[0]) {
        int len;
        ERR_print_errors(bio_err);
        len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
        ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
        uq_ok("X.509 Issuer Name unavailable", ssl_err, 1, NULL, 0);
        ok=0;
        goto return_time;
    }
#endif /* XN_FLAG_SEP_MULTILINE */

    if (verbosity && depth != ssl_verify_depth) {
        printf("[%d] Certificate Subject:\r\n%s\r\n",depth,subject);
        printf("[%d] Certificate Issuer:\r\n%s\r\n",depth,issuer);
        ssl_verify_depth = depth;
    }

    ok = ssl_verify_crl(ok, ctx);

    if ( !ok ) {
        char prefix[1024];
        /* if the server is using a self signed certificate then
         * we need to decide if that is good enough for us to
         * accept ...
         */

        switch ( error ) {
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT: {
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                /* make 100% sure that in secure more we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ckmakxmsg(prefix,1024,
                           "Error: Server has a self-signed certificate\n",
                           "[",ckitoa(depth),"] Certificate Subject=\n",subject,
                           "\n[",ckitoa(depth),"] Certificate Issuer=\n",issuer,
                           NULL,NULL,NULL);

                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);

                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                ckmakxmsg(prefix,1024,
                           "Warning: Server has a self-signed certificate\n",
                           "[",ckitoa(depth),"] Certificate Subject=\n",subject,
                           "\n[",ckitoa(depth),"] Certificate Issuer=\n",issuer,
                           NULL,NULL,NULL);

                ok = uq_ok(prefix,
                           "Continue? (Y/N) ",
                           3, NULL, 0);
                if ( ok < 0 )
                    ok = 0;
                goto return_time;
            }
        }
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                /* make 100% sure that in secure more we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ckmakxmsg(prefix,1024,
                           "Error: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Issuer=\n",issuer,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);

                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                ckmakxmsg(prefix,1024,
                           "Warning: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Issuer=\n",issuer,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                ok = uq_ok(prefix, "Continue (Y/N)", 3, NULL, 0);
                goto return_time;
            }
            break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                int len;
                /* make 100% sure that in secure more we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ASN1_TIME_print(bio_err,X509_get_notBefore(xs));
                len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                ckmakxmsg(prefix,1024,
                           "Error: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           "\nnotBefore=",ssl_err,
                           NULL,NULL,NULL,NULL,NULL,NULL);
                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);
                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                int len;
                ASN1_TIME_print(bio_err,X509_get_notBefore(xs));
                len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                ckmakxmsg(prefix,1024,
                           "Warning: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           "\n    notBefore=",ssl_err,
                           NULL,NULL,NULL,NULL,NULL,NULL);
                ok = uq_ok(prefix, "Continue (Y/N)", 3, NULL, 0);
            }
            break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                int len;
                /* make 100% sure that in secure more we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ASN1_TIME_print(bio_err,X509_get_notAfter(xs));
                len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';

                ckmakxmsg(prefix,1024,
                           "Error: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           "\n    notAfter=",ssl_err,
                           NULL,NULL,NULL,NULL,NULL,NULL);
                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);
   
                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                int len;
                ASN1_TIME_print(bio_err,X509_get_notAfter(xs));
                len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                ckmakxmsg(prefix,1024,
                           "Warning: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           "\n    notAfter=",ssl_err,
                           NULL,NULL,NULL,NULL,NULL,NULL);
                ok = uq_ok(prefix, "Continue (Y/N)", 3, NULL, 0);
            }
            break;
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
            /*
             * When an SSL server sends its certificates to the client there
             * are two" conventions": one is to send the complete certificate
             * chain and the other is to send the whole chain apart from the
             * root.
             *
             * You don't usually need the root because the root is normally
             * stored and trusted locally.
             *
             * So if you get the whole chain it will complain about the self
             * signed certificate whereas if the root is missing it says it
             * can't find the issuer certificate.
             */
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                /* make 100% sure that in secure more we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ckmakxmsg(prefix,1024,
                           "Error: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Issuer=\n",issuer,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);
                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                ckmakxmsg(prefix,1024,
                           "Warning: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Issuer=\n",issuer,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                ok = uq_ok(prefix, "Continue (Y/N)", 3, NULL, 0);
#ifdef NT
                if (ok) {
                    /* if the user decides to accept the certificate
                     * offer to store it for future connections in 
                     * the user's private store
                     */
                    ok = uq_ok(
  "Do you wish to store the certificate to verify future connections?",
                               "Continue (Y/N)", 3, NULL, 0);
                    if (ok)
                        ck_X509_save_cert_to_user_store(xs);
                }
#endif /* NT */
            }
            break;
        case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        case X509_V_ERR_UNABLE_TO_GET_CRL:
        case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
        case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
        case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
        case X509_V_ERR_CERT_SIGNATURE_FAILURE:
        case X509_V_ERR_CRL_SIGNATURE_FAILURE:
        case X509_V_ERR_CRL_NOT_YET_VALID:
        case X509_V_ERR_CRL_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
        case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
        case X509_V_ERR_OUT_OF_MEM:
        case X509_V_ERR_CERT_CHAIN_TOO_LONG:
        case X509_V_ERR_CERT_REVOKED:
        case X509_V_ERR_APPLICATION_VERIFICATION:
        default:
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                /* make 100% sure that in secure mode we drop the
                 * connection if the server does not have a
                 * real certificate!
                 */
                ckmakxmsg(prefix,1024,
                           "Error: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                uq_ok(prefix, "Rejecting Connection", 1, NULL, 0);

                /* sometimes it is really handy to be able to debug things
                * and still get a connection!
                */
                if (ssl_debug_flag) {
                    printf("SSL: debug -> ignoring cert required!\r\n");
                    ok=1;
                } else {
                    ok=0;
                }
                goto return_time;
            } else if (ssl_verify_flag != SSL_VERIFY_NONE) {
                ckmakxmsg(prefix,1024,
                           "Warning: ",
                           (char *)X509_verify_cert_error_string(error),
                           "\nCertificate Subject=\n",subject,
                           NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                ok = uq_ok(prefix, "Continue (Y/N)", 3, NULL, 0);
            }
            break;
        }
    }

  return_time:
    if ( ssl_debug_flag )
        printf("ssl:client_verify_callback => ok: %d\r\n",ok);
    return ok;
}

VOID
#ifdef CK_ANSIC
ssl_client_info_callback(const SSL *s, int where, int ret)
#else
ssl_client_info_callback(s,where,ret)
const SSL *s;
int where;
int ret;
#endif /* CK_ANSIC */
{
    if (inserver || !ssl_debug_flag)
        return;

    setverbosity();

    switch ( where ) {
    case SSL_CB_CONNECT_LOOP:
        printf("SSL_connect:%s %s\r\n",
                SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        break;
    case SSL_CB_CONNECT_EXIT:
        if (ret == 0) {
            printf("SSL_connect:failed in %s %s\r\n",
                    SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        } else if (ret < 0) {
            printf("SSL_connect:error in %s %s\r\n",
                    SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        }
        break;
    case SSL_CB_ACCEPT_LOOP:
        printf("SSL_accept:%s %s\r\n",
                SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        break;
    case SSL_CB_ACCEPT_EXIT:
        if (ret == 0) {
            printf("SSL_accept:failed in %s %s\r\n",
                    SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        } else if (ret < 0) {
            printf("SSL_accept:error in %s %s\r\n",
                    SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        }
        break;
    case SSL_CB_READ_ALERT:
        printf("SSL_read_alert\r\n");
        break;
    case SSL_CB_WRITE_ALERT:
        printf("SSL_write_alert\r\n");
        break;
    case SSL_CB_HANDSHAKE_START:
        printf("SSL_handshake:%s %s\r\n",
                SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        break;
    case SSL_CB_HANDSHAKE_DONE:
        printf("SSL_handshake:%s %s\r\n",
                SSL_state_string((SSL *)s),SSL_state_string_long((SSL *)s));
        break;
    }
}

#ifdef USE_CERT_CB
/* Return 1, client cert is available */
/* Return 0, no client cert is available */
/* Return -1, callback must be called again. SSL_want_x509_lookup() == 1 */
int
#ifdef CK_ANSIC
ssl_client_cert_callback(SSL * s, X509 ** x509, EVP_PKEY ** pkey)
#else /* CK_ANSIC */
ssl_client_cert_callback(s, x509, pkey)
    SSL * s;
    X509 ** x509;
    EVP_PKEY ** pkey;
#endif /* CK_ANSIC */
{
    setverbosity();

    if ( ssl_debug_flag ) {
        const char * cipher_list=SSL_get_cipher(s);
        printf("ssl_client_cert_callback called (%s)\r\n",
                cipher_list?cipher_list:"UNKNOWN");
    }
#ifdef COMMENT
    if ( s == tls_con ) {
        if (tls_load_certs(tls_cts,tls_con,0)) {
            *x509 = SSL_get_certificate(s);
            *pkey = SSL_get_privatekey(s);
            return(1);
        }
    } else if ( s == ssl_con ) {
        if (tls_load_certs(ssl_ctx,ssl_con,0)) {
            *x509 = SSL_get_certificate(s);
            *pkey = SSL_get_privatekey(s);
            return(1);
        }
    }
    return(0);
#else /* COMMENT */
    return(0);
#endif /* COMMENT */
}
#endif /* USE_CERT_CB */

#ifndef MS_CALLBACK
#define MS_CALLBACK
#endif /* MS_CALLBACK */

static RSA MS_CALLBACK *
#ifdef CK_ANSIC
tmp_rsa_cb(SSL * s, int export, int keylength)
#else /* CK_ANSIC */
tmp_rsa_cb(s,export,keylength)
SSL *s;
int export;
int keylength;
#endif /* CK_ANSIC */
{
    static RSA *rsa_tmp=NULL;

#ifndef NO_RSA
    if (rsa_tmp == NULL)
    {
        if (ssl_debug_flag)
            printf("Generating temporary (%d bit) RSA key...\r\n",keylength);

        rsa_tmp=RSA_generate_key(keylength,RSA_F4,NULL,NULL);

        if (ssl_debug_flag)
            printf("\r\n");
    }
#else /* NO_RSA */
    if (ssl_debug_flag)
        printf("Unable to generate temporary RSA key...\r\n");
#endif
    return(rsa_tmp);
}


#ifndef NO_DH
static unsigned char dh512_p[]={
    0xE9,0x4E,0x3A,0x64,0xFA,0x65,0x5F,0xA6,0x44,0xC7,0xFC,0xF1,
    0x16,0x8B,0x11,0x11,0x7A,0xF0,0xB2,0x49,0x80,0x56,0xA3,0xF8,
    0x0F,0x7D,0x01,0x68,0x5D,0xF6,0x8A,0xEA,0x8C,0xDD,0x01,0xDC,
    0x43,0x18,0xE0,0xC4,0x89,0x80,0xE6,0x2D,0x44,0x77,0x45,0xFD,
    0xBA,0xFC,0x43,0x35,0x12,0xC0,0xED,0x32,0xD3,0x16,0xEF,0x51,
    0x09,0x44,0xA2,0xDB,
};
static unsigned char dh512_g[]={
    0x05,
};

static unsigned char dh768_p[]={
    0x8B,0x2A,0x8C,0x6C,0x0F,0x87,0xC7,0x34,0xEE,0x2E,0xFB,0x60,
    0x94,0xB3,0xBF,0x95,0xBA,0x84,0x74,0x86,0xEA,0xE0,0xA4,0x33,
    0xE0,0x8F,0x7C,0x79,0x5C,0x62,0xE2,0x91,0xC5,0x6D,0x68,0xB9,
    0x6C,0x5E,0x4E,0x94,0x0C,0x8E,0x56,0x8E,0xEB,0x98,0x7C,0x6E,
    0x0E,0xF2,0xD5,0xAA,0x22,0x27,0x3F,0x0F,0xAF,0x10,0xB5,0x0B,
    0x16,0xCC,0x05,0x27,0xBB,0x58,0x6D,0x61,0x4B,0x2B,0xAB,0xDC,
    0x6A,0x15,0xBC,0x36,0x75,0x4D,0xEC,0xAB,0xFA,0xB6,0xE1,0xB1,
    0x13,0x70,0xD8,0x77,0xCD,0x5E,0x51,0x77,0x81,0x0D,0x77,0x43,
};
static unsigned char dh768_g[]={
    0x05,
};

static unsigned char dh1024_p[]={
    0xA4,0x75,0xCF,0x35,0x00,0xAF,0x3C,0x17,0xCE,0xB0,0xD0,0x52,
    0x43,0xA0,0x0E,0xFA,0xA2,0xC9,0xBE,0x0B,0x76,0x7A,0xD9,0x2E,
    0xF4,0x97,0xAC,0x02,0x24,0x69,0xF6,0x36,0x4F,0xAB,0xCC,0x43,
    0xC1,0x74,0xFF,0xA3,0xD4,0x04,0x0F,0x11,0x2B,0x6D,0x8C,0x47,
    0xC9,0xCF,0x40,0x93,0x9B,0x7D,0x1E,0x52,0x85,0xB2,0x17,0x55,
    0x9C,0xF2,0x41,0x02,0x2A,0x9D,0x5F,0x24,0x22,0xC6,0x04,0xC4,
    0xAB,0x92,0x6D,0xC7,0xC8,0xF3,0x41,0x58,0x6C,0x86,0xFD,0xB8,
    0x0F,0x2D,0xDD,0xBF,0xA8,0x40,0x0C,0x58,0xC8,0xF2,0x3F,0x18,
    0xEF,0xF1,0x93,0x3E,0xBA,0x16,0x41,0xBE,0x32,0x6C,0xC5,0x63,
    0xFF,0x8A,0x02,0x3D,0xAC,0xD5,0x5A,0x49,0x64,0x34,0x14,0x2E,
    0xFB,0x2E,0xE7,0x39,0x1A,0x0F,0x3C,0x33,
};
static unsigned char dh1024_g[]={
    0x05,
};

static unsigned char dh1536_p[]={
    0xA3,0x2B,0x75,0x0E,0x7B,0x31,0x82,0xCA,0xF2,0xFC,0xF3,0x3D,
    0xCE,0x5F,0xCD,0x5B,0x95,0xF6,0x2F,0xA4,0x5D,0x08,0x26,0xD2,
    0x5F,0xC0,0x3F,0xC5,0xD8,0xA2,0xFE,0x83,0x26,0xBC,0xEB,0x7D,
    0xF0,0x4E,0xD2,0xA6,0xBB,0x3C,0x88,0x63,0xCE,0x98,0xDE,0x08,
    0xE2,0xE1,0xAF,0xE2,0x38,0xA8,0xFA,0x68,0x76,0x8D,0xBF,0xDF,
    0xBB,0x30,0x15,0xFE,0xBD,0x22,0xCC,0x03,0x4E,0x5E,0x33,0xA3,
    0x6D,0xD6,0x68,0x12,0x97,0x17,0x4B,0xB5,0x84,0x5F,0x5F,0xA3,
    0x5C,0x2F,0xA4,0x10,0xC1,0xAD,0xBF,0xAC,0x30,0xCA,0x47,0x64,
    0x63,0xFE,0xEE,0xEE,0xA1,0x64,0x73,0x70,0xAA,0xF9,0xFE,0xC6,
    0xAD,0x5E,0xF6,0xF3,0x9C,0xDF,0x34,0x53,0x34,0x72,0xA6,0xA4,
    0xBB,0x81,0x5A,0x43,0x41,0xFD,0x41,0x05,0x5B,0x77,0x7B,0x84,
    0x03,0xFA,0x8A,0xFA,0xF7,0x8E,0x0F,0xCB,0x51,0xA2,0xB8,0x45,
    0xFF,0x59,0x42,0xEF,0xCF,0xF6,0x25,0x37,0xE2,0x6D,0xFF,0x69,
    0x11,0xF5,0x77,0x59,0x79,0x1C,0x5F,0x05,0xFC,0x7A,0x65,0x81,
    0x03,0x4A,0x78,0xC6,0xE9,0x48,0x73,0xF6,0x10,0xBC,0x99,0x1C,
    0xEE,0x44,0x2F,0x8B,0x70,0xCA,0xA8,0xB6,0x02,0x83,0x3E,0x0B,
};
static unsigned char dh1536_g[]={
    0x05,
};

static unsigned char dh2048_p[]={
    0xFA,0x4E,0xE4,0x3B,0xFA,0xC1,0x87,0xDD,0xE7,0xC6,0x8B,0xE6,
    0x13,0x85,0xBC,0x9B,0x2B,0x8B,0x5B,0x46,0xBB,0x8B,0x86,0x6D,
    0xD7,0xB6,0xD5,0x49,0xC5,0x54,0xF2,0x3E,0xD2,0x39,0x64,0x9B,
    0x0E,0x33,0x39,0x8F,0xFA,0xFA,0xD9,0x78,0xED,0x34,0x82,0x29,
    0x37,0x58,0x4D,0x5D,0x40,0xCB,0x69,0xE3,0x8A,0x9F,0x17,0x0C,
    0x01,0x23,0x6B,0x05,0x01,0xAF,0x33,0xDE,0xDF,0x1A,0xBB,0x7B,
    0x6A,0x9F,0xD8,0xED,0x8D,0x5E,0x44,0x19,0x5B,0xE0,0xB6,0x23,
    0xF9,0x7A,0x96,0x6E,0x94,0x33,0x31,0x49,0xBA,0x84,0xD5,0x12,
    0xD7,0x6D,0xDC,0x35,0x54,0x64,0xA3,0xD8,0x04,0x26,0xC5,0xAF,
    0x7F,0xE3,0xFE,0x6F,0xBE,0xD5,0x17,0x72,0x4B,0xA6,0xD0,0xA7,
    0x5F,0x18,0xF5,0xF0,0x2D,0x11,0x9A,0xF6,0xD5,0x3B,0x6C,0x61,
    0x3C,0x6F,0x8E,0x09,0x4F,0x2C,0xE1,0x26,0x06,0x51,0xB3,0x19,
    0x85,0x85,0x13,0xF9,0xC2,0x6E,0x80,0x28,0x9E,0x8A,0xA0,0x01,
    0x46,0xD1,0x85,0x44,0x8C,0xE6,0xEE,0x7E,0x1E,0x17,0x3D,0xBA,
    0x54,0xFF,0xE8,0x0E,0xDD,0x51,0xF3,0x74,0x7F,0x0D,0x0B,0xAB,
    0xCA,0x84,0x8D,0x24,0x5D,0x56,0xD4,0x47,0x02,0xFC,0x93,0x9F,
    0xAE,0x9B,0x5C,0xDB,0x63,0xEB,0x65,0x01,0x38,0xC2,0x7B,0x30,
    0x1E,0x17,0x1C,0x75,0xF5,0x16,0x3B,0x4F,0x5F,0x41,0x32,0xB5,
    0xFF,0x9E,0x61,0xFD,0xD2,0x62,0x6E,0xFD,0x8A,0x28,0x93,0x59,
    0x2D,0x70,0x14,0x4D,0xE1,0x86,0xD5,0x90,0xB4,0xDF,0x72,0x71,
    0xE0,0xB4,0xD0,0xD6,0x82,0x3A,0x4A,0x04,0x58,0x32,0x0B,0xD3,
    0x51,0x13,0x32,0x63,
};
static unsigned char dh2048_g[]={
    0x02,
};

static DH *
get_dh512()
{
    DH *dh=NULL;

    if ((dh=DH_new()) == NULL)
        return(NULL);
    dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
    dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
        return(NULL);
    return(dh);
}

static DH *
get_dh768()
{
    DH *dh=NULL;

    if ((dh=DH_new()) == NULL)
        return(NULL);
    dh->p=BN_bin2bn(dh768_p,sizeof(dh768_p),NULL);
    dh->g=BN_bin2bn(dh768_g,sizeof(dh768_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
        return(NULL);
    return(dh);
}

static DH *
get_dh1024()
{
    DH *dh=NULL;

    if ((dh=DH_new()) == NULL)
        return(NULL);
    dh->p=BN_bin2bn(dh1024_p,sizeof(dh1024_p),NULL);
    dh->g=BN_bin2bn(dh1024_g,sizeof(dh1024_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
        return(NULL);
    return(dh);
}

static DH *
get_dh1536()
{
    DH *dh=NULL;

    if ((dh=DH_new()) == NULL)
        return(NULL);
    dh->p=BN_bin2bn(dh1536_p,sizeof(dh1536_p),NULL);
    dh->g=BN_bin2bn(dh1536_g,sizeof(dh1536_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
        return(NULL);
    return(dh);
}

static DH *
get_dh2048()
{
    DH *dh=NULL;

    if ((dh=DH_new()) == NULL)
        return(NULL);
    dh->p=BN_bin2bn(dh2048_p,sizeof(dh2048_p),NULL);
    dh->g=BN_bin2bn(dh2048_g,sizeof(dh2048_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
        return(NULL);
    return(dh);
}
#endif /* NO_DH */

static DH MS_CALLBACK *
#ifdef CK_ANSIC
tmp_dh_cb(SSL * s, int export, int keylength)
#else /* CK_ANSIC */
tmp_dh_cb(s,export,keylength)
SSL *s;
int export;
int keylength;
#endif /* CK_ANSIC */
{
    static DH *dh_tmp=NULL;
    BIO *bio=NULL;

#ifndef NO_DH
    if (dh_tmp == NULL)
    {
        if (ssl_dh_param_file  &&
             (bio=BIO_new_file(ssl_dh_param_file,"r")) != NULL)
            dh_tmp=PEM_read_bio_DHparams(bio,NULL,NULL,NULL);
        if (bio != NULL)
            BIO_free(bio);

        if ( dh_tmp == NULL ) {
            if ( keylength < 768 )
                dh_tmp = get_dh512();
            else if ( keylength < 1024 )
                dh_tmp = get_dh768();
            else if ( keylength < 1536 )
                dh_tmp = get_dh1024();
            else if ( keylength < 2048 )
                dh_tmp = get_dh1536();
            else
                dh_tmp = get_dh2048();
        }
    }
#else /* NO_DH */
    if (ssl_debug_flag)
        printf("DH not supported...\r\n");
#endif /* NO_DH */
    return(dh_tmp);
}

static void
ssl_display_comp(SSL * ssl)
{
    if ( quiet )			/* fdc - Mon Nov 28 11:44:15 2005 */
        return;

    if ( !ck_ssleay_is_installed() )
        return;

    if (ssl == NULL)
        return;

    if (ssl->expand == NULL || ssl->expand->meth == NULL)
        printf("Compression: None\r\n");
    else {
        printf("Compression: %s\r\n",ssl->expand->meth->name);
    }
}

int
#ifdef CK_ANSIC
ssl_display_connect_details(SSL * ssl_con, int server, int verbose)
#else /* CK_ANSIC */
ssl_display_connect_details(ssl_con,server,verbose)
SSL *ssl_con;
int server;
int verbose;
#endif /* CK_ANSIC */
{
    X509 *peer;
    SSL_CIPHER * cipher;
    const char *cipher_list;
    char buf[512]="";

    if ( quiet )			/* fdc - Mon Nov 28 11:44:15 2005 */
        return(0);

    if ( !ck_ssleay_is_installed() )
        return(0);

    if ( inserver && !tn_deb )
        return(0);

    /* the cipher list *can* be NULL ... useless but it happens! */
    cipher = SSL_get_current_cipher(ssl_con);
    cipher_list = SSL_CIPHER_get_name(cipher);
    SSL_CIPHER_description(cipher,buf,sizeof(buf));
    if (cipher_list==NULL)
        cipher_list="<NULL>";
    printf("[TLS - %s",buf);
    ssl_display_comp(ssl_con);

    if ( server ) {
        cipher_list=SSL_get_shared_ciphers(ssl_con,buf,512);
        if (cipher_list==NULL)
            cipher_list="<NULL>";
        printf("[TLS - shared ciphers=%s]\r\n",
                cipher_list);
        }       
    if ( server || tn_deb ) {
        peer=SSL_get_peer_certificate(ssl_con);
        if (peer != NULL) {
            X509_NAME_oneline(X509_get_subject_name(peer),buf,512);
            printf("[TLS - subject=%s]\r\n",buf);
            X509_NAME_oneline(X509_get_issuer_name(peer),buf,512);
            printf("[TLS - issuer=%s]\r\n",buf);
            /* X509_free(peer); */
        } else if (!tls_is_krb5(0)) {
            if ( !sstelnet && !tcp_incoming ) {
                printf("[TLS - No certificate provided.]\r\n");
                printf(
     "[TLS - The identity of the host could not be verified.]\r\n");
            }
        }
    }
    return(0);
}

/*
 * Use SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *, void * userdata)
 * to set the value of the userdata.  We are going to use it to store the
 * prompt.
 */

int
#ifdef CK_ANSIC
ssl_passwd_callback(char *buf, int len, int rwflag, VOID * userdata)
#else /* CK_ANSIC */
ssl_passwd_callback(buf,len,rwflag,userdata)
    char * buf; int len; int rwflag; VOID *userdata;
#endif /* CK_ANSIC */
{
    extern char pwbuf[];
    extern int  pwflg, pwcrypt;
    int   ok;
    char *prompt=NULL;

    if ( pwbuf[0] && pwflg ) {
        int n;
        n = ckstrncpy(buf,pwbuf,len);
#ifdef OS2
        if ( pwcrypt )
            ck_encrypt((char *)buf);
#endif /* OS2 */
        return(n);
    }

    if ( userdata == NULL )
        prompt="Enter certificate passphrase: ";
    else
        prompt=(char*)userdata;
    ok = uq_txt(NULL,prompt,2,NULL,buf,len,NULL,DEFAULT_UQ_TIMEOUT);
    return(ok > 0 ? strlen(buf) : 0);
}


/* Attempts to load certificate data into the TLS context structures */
/* Returns 1 on success; 0 on failure */
int
tls_load_certs(SSL_CTX * ctx, SSL * con, int server)
{
    int rc = 1;

    if ( !ck_ssleay_is_installed() )
        return(0);

    debug(F110,"tls_load_certs","SSL_CTX",0);
    debug(F110,"tls_load_certs","SSL",0);
    debug(F110,"tls_load_certs","server",0);

    if ( con ) {
        if (ssl_rsa_cert_file) {
            if ( ssl_debug_flag )
                printf("Loading RSA certificate into SSL\r\n");

            rc = SSL_use_certificate_file(con, ssl_rsa_cert_file,
                                               X509_FILETYPE_PEM);
            if (!rc)
            {
                if ( !quiet || ssl_debug_flag )
                    printf("Error loading certificate from %s\r\n",
                            ssl_rsa_cert_file);
            } else {
                if (!ssl_rsa_key_file || !ssl_rsa_key_file[0])
                    makestr(&ssl_rsa_key_file,ssl_rsa_cert_file);

                rc = SSL_use_PrivateKey_file(con, ssl_rsa_key_file,
                                                  X509_FILETYPE_PEM);
                if (!rc)
                    rc = SSL_use_PrivateKey_file(con, ssl_rsa_cert_file,
                                                      X509_FILETYPE_PEM);
                if (!rc)
                {
                    if ( !quiet || ssl_debug_flag )
                        printf("Error loading key from %s\r\n",
                                ssl_rsa_key_file);
                } else {
                    rc = SSL_check_private_key(con);
                    if (!rc)
                    {
                        if ( ssl_debug_flag )
                            printf(
                "Private key does not match the certificate public key\r\n");
                    }
                }
            }
        }

        if (ssl_dsa_cert_file) {
            if ( ssl_debug_flag )
                printf("Loading DSA certificate into SSL\r\n");

            rc = SSL_use_certificate_file(con, ssl_dsa_cert_file,
                                               X509_FILETYPE_PEM);
            if (!rc)
            {
                if ( ssl_debug_flag ) {
                    printf("Error loading certificate from %s\r\n",
                            ssl_dsa_cert_file);
                }
            } else {
                if (!ssl_dh_key_file || !ssl_dh_key_file[0])
                    makestr(&ssl_dh_key_file,ssl_dsa_cert_file);
                rc = SSL_use_PrivateKey_file(con, ssl_dh_key_file,
                                                  X509_FILETYPE_PEM);
                if (!rc)
                    rc = SSL_use_PrivateKey_file(con, ssl_dsa_cert_file,
                                                      X509_FILETYPE_PEM);
                if (!rc)
                {
                    if ( !quiet || ssl_debug_flag ) {
                        printf("Error loading key from %s\r\n",
                                ssl_dh_key_file);
                    }
                } else {
                    rc = SSL_check_private_key(con);
                    if (!rc)
                    {
                        if ( ssl_debug_flag )
                            printf(
                   "Private key does not match the certificate public key\n");
                    }
                }
            }
        }
    } else {
        if (ssl_rsa_cert_file) {
            if ( ssl_debug_flag )
                printf("Loading RSA certificate into SSL\r\n");

            rc = SSL_CTX_use_certificate_file(ctx, ssl_rsa_cert_file,
                                       X509_FILETYPE_PEM);
            if (!rc)
            {
                if ( !quiet || ssl_debug_flag )
                    printf("Error loading certificate from %s\r\n",
                            ssl_rsa_cert_file);
            } else {
                if (!ssl_rsa_key_file || !ssl_rsa_key_file[0])
                    makestr(&ssl_rsa_key_file,ssl_rsa_cert_file);

                rc = SSL_CTX_use_PrivateKey_file(ctx, ssl_rsa_key_file,
                                                  X509_FILETYPE_PEM);
                if (!rc)
                  rc = SSL_CTX_use_PrivateKey_file(ctx, ssl_rsa_cert_file,
                                                   X509_FILETYPE_PEM);
                if (!rc) {
                    if ( ssl_debug_flag )
                      printf("Error loading key from %s\r\n",ssl_rsa_key_file);
                } else {
                    rc = SSL_CTX_check_private_key(ctx);
                    if (!rc) {
                        if ( ssl_debug_flag )
                          printf(
                "Private key does not match the certificate public key\r\n");
                    }
                }
            }
        }
        if (ssl_dsa_cert_file) {
            if ( ssl_debug_flag )
              printf("Loading DSA certificate into SSL\r\n");

            rc = SSL_CTX_use_certificate_file(ctx, ssl_dsa_cert_file,
                                              X509_FILETYPE_PEM);
            if (!rc) {
                if ( ssl_debug_flag ) {
                    printf("Error loading certificate from %s\r\n",
                           ssl_dsa_cert_file);
                }
            } else {
                if (!ssl_dh_key_file || !ssl_dh_key_file[0])
                    makestr(&ssl_dh_key_file,ssl_dsa_cert_file);
                rc = SSL_CTX_use_PrivateKey_file(ctx, ssl_dh_key_file,
                                                  X509_FILETYPE_PEM);
                if (!rc)
                  rc = SSL_CTX_use_PrivateKey_file(ctx, ssl_dsa_cert_file,
                                                      X509_FILETYPE_PEM);
                if (!rc) {
                    if ( ssl_debug_flag )
                      printf("Error loading key from %s\r\n",ssl_dh_key_file);
                } else {
                    rc = SSL_CTX_check_private_key(ctx);
                    if (!rc) {
                        if ( ssl_debug_flag )
                          printf(
                   "Private key does not match the certificate public key\n");
                    }
                }
            }
        }
    }

    if (ssl_rsa_cert_chain_file && server) {
        int skip1st = 0;
        if (ssl_debug_flag)
            printf("Loading RSA Certificate Chain into SSL\r\n");
        if (!ckstrcmp(ssl_rsa_cert_chain_file,ssl_rsa_cert_file,-1,
#ifdef OS2
                       0
#else
                       1
#endif /* OS2 */
                       ))
            skip1st = 1;
        rc = SSL_CTX_use_certificate_chain_file(ctx,ssl_rsa_cert_chain_file);
        if (!rc && ssl_debug_flag)
                printf("Error loading RSA Certificate Chain into SSL\r\n");
    }
    if (ssl_dsa_cert_chain_file && server) {
        int skip1st = 0;
        if (ssl_debug_flag)
            printf("Loading DSA Certificate Chain into SSL\r\n");
        if (!ckstrcmp(ssl_dsa_cert_chain_file,ssl_dsa_cert_file,-1,
#ifdef OS2
                       0
#else
                       1
#endif /* OS2 */
                       ))
            skip1st = 1;
        rc = SSL_CTX_use_certificate_chain_file(ctx,ssl_dsa_cert_chain_file);
        if (!rc && ssl_debug_flag)
                printf("Error loading DSA Certificate Chain into SSL\r\n");
    }
    return(rc);
}

VOID
#ifdef CK_ANSIC
ssl_once_init(void)
#else
ssl_once_init()
#endif /* CK_ANSIC */
{
    COMP_METHOD * cm;
    char * s;

    if ( !ck_ssleay_is_installed() )
        return;
/*
  OpenSSL does not provide for ABI compatibility between releases prior
  to version 1.0.0.  If the version does not match, it is not safe to
  assume that any function you call takes the same parameters or does
  the same thing with them.  Removing this test prior to the OpenSSL 1.0.0
  release will result in an increase in unexplained or incorrect behaviors.
  The test should be revised once OpenSSL 1.0.0 is released and we see what
  its claims are as to ABI compatibility.
*/
    debug(F111,"Kermit built for OpenSSL",OPENSSL_VERSION_TEXT,SSLEAY_VERSION_NUMBER);
#ifndef OS2ONLY
    debug(F111,"OpenSSL Library",SSLeay_version(SSLEAY_VERSION),
           SSLeay());
    debug(F110,"OpenSSL Library",SSLeay_version(SSLEAY_BUILT_ON),0);
    debug(F110,"OpenSSL Library",SSLeay_version(SSLEAY_CFLAGS),0);
    debug(F110,"OpenSSL Library",SSLeay_version(SSLEAY_PLATFORM),0);

    /* The following test is suggested by Richard Levitte */
    if (((OPENSSL_VERSION_NUMBER ^ SSLeay()) & 0xffffff0f) 
#ifdef OS2
         || ckstrcmp(OPENSSL_VERSION_TEXT,(char *)SSLeay_version(SSLEAY_VERSION),-1,1)
#endif /* OS2 */
         ) {
        ssl_installed = 0;
        debug(F111,"OpenSSL Version does not match.  Built with",
               SSLeay_version(SSLEAY_VERSION),SSLEAY_VERSION_NUMBER);
        printf("?OpenSSL libraries do not match required version:\r\n");
        printf("  . C-Kermit built with %s\r\n",OPENSSL_VERSION_TEXT);
        printf("  . Version found  %s\r\n",SSLeay_version(SSLEAY_VERSION));
        printf("  OpenSSL versions prior to 1.0.0 must be the same.\r\n");    

	s = "R";
#ifdef SOLARIS
	printf("  Set CD_LIBRARY_PATH for %s.\r\n",OPENSSL_VERSION_TEXT);
	s = " Or r";
#endif	/* SOLARIS */

#ifdef HPUX
	printf("  Set SHLIB_PATH for %s.\r\n",OPENSSL_VERSION_TEXT);
	s = " Or r";
#endif	/* HPUX */

#ifdef AIX
	printf("  Set LIBPATH for %s.\r\n",OPENSSL_VERSION_TEXT);
	s = " Or r";
#endif	/* AIX */

#ifdef LINUX
	printf("  Set LD_LIBRARY_PATH for %s.\r\n",OPENSSL_VERSION_TEXT);
	s = " Or r";
#endif	/* LINUX */

        printf(" %sebuild C-Kermit from source on this computer to make \
versions agree.\r\n",s);

#ifdef KTARGET
	{
	    char * s;
	    s = KTARGET;
	    if (!s) s = "";
	    if (!*s) s = "(unknown)";
	    printf("  C-Kermit makefile target: %s\r\n",s);
	}
#endif	/* KTARGET */
        printf("  Or if that is what you did then try to find out why\r\n");
        printf("  the program loader (image activator) is choosing a\r\n");
        printf("  different OpenSSL library than the one specified in \
the build.\r\n\r\n");
        printf("  All SSL/TLS features disabled.\r\n\r\n");
        bleep(BP_FAIL);
#ifdef SSLDLL
        ck_ssl_unloaddll();
        ck_crypto_unloaddll();
#endif /* SSLDLL */
        return;
    }
#endif /* OS2ONLY */

    /* init things so we will get meaningful error messages
     * rather than numbers
     */
    SSL_load_error_strings();

#ifdef SSHBUILTIN
    OPENSSL_add_all_algorithms_noconf();
#else
    /* SSL_library_init() only loads those ciphers needs for SSL  */
    /* These happen to be a similar set to those required for SSH */
    /* but they are not a complete set of ciphers provided by the */
    /* crypto library.                                            */
    SSL_library_init();
#endif /* SSHBUILTIN */

#ifdef ZLIB
    cm = COMP_zlib();
    if (cm != NULL && cm->type != NID_undef) {
        SSL_COMP_add_compression_method(0xe0, cm); /* EAY's ZLIB ID */
    }
#endif /* ZLIB */
    cm = COMP_rle();
    if (cm != NULL && cm->type != NID_undef)
        SSL_COMP_add_compression_method(0xe1, cm); /* EAY's RLE ID */

    /* Ensure the Random number generator has enough entropy */
    if ( !RAND_status() ) {
        char buffer[256]="";
        char randombytes[256];
        int rc1 = -1, rc2 = 1;  /* assume failure and success */

        debug(F110,"ssl_once_init","!RAND_status()",0);

        if ( ssl_rnd_file == NULL ) {
            debug(F110,"ssl_rnd_file","ssl_rnd_file is NULL",0);
            RAND_file_name(buffer,256);
            if ( buffer[0] )
                makestr(&ssl_rnd_file, buffer);
            else
                makestr(&ssl_rnd_file,".rnd");
        }
        debug(F110,"ssl_rnd_file",ssl_rnd_file,0);

        rc1 = RAND_egd(ssl_rnd_file);
        debug(F111,"ssl_once_init","RAND_egd()",rc1);
        if ( rc1 <= 0 ) {
            rc2 = RAND_load_file(ssl_rnd_file, -1);
            debug(F111,"ssl_once_init","RAND_load_file()",rc1);
        }

        if ( rc1 <= 0 && !rc2 )
        {
            time_t t = time(NULL);
            int tlen = sizeof(time_t);
            int pid = getpid();
            int plen = sizeof(int);
            int n;
#ifndef RAND_MAX
#define RAND_MAX 0x7FFF
#endif
            debug(F110,"ssl_once_init","calling RAND_seed()",0);

            RAND_seed((unsigned char *)&t, tlen);
            RAND_seed((unsigned char *)&pid, plen);

            srand((unsigned int)t);
            sprintf(buffer, "%.0f", (((double)(rand()%RAND_MAX)/RAND_MAX)*
                                      (sizeof(randombytes)-128-1)));
            n = (atoi(buffer)+1)%(sizeof(randombytes)-128-1);
            RAND_seed(randombytes, 128);
        }

        if ( !RAND_status() ) {
            debug(F110,"ssl_once_init","Unable to initialize PRNG",0);
            printf(" Unable to load 'random state'\n");
            printf(" SSL and TLS are unavailble.\n");
            printf(" Use SET AUTH SSL RANDOM-FILE <file> command to provide random data.\n");
            printf(" Specified file will be overwritten with new random data after use.\n");
            return;
        }

        if ( ssl_rnd_file ) {
            int rc = RAND_write_file(ssl_rnd_file);
            debug(F111,"ssl_once_init","RAND_write_file()",rc);
        }
    }

#ifdef NT
    // Initialize additional OID types for use when saving certs to a file
    OBJ_create("2.99999.3","SET.ex3","SET x509v3 extension 3");
#endif /* NT */

    /* make sure we have somewhere we can log errors to */
    bio_err=BIO_new(BIO_s_mem());

    debug(F100,"ssl_once_init() complete","",0);
}

int
#ifdef CK_ANSIC
ssl_tn_init(int mode)
#else
ssl_tn_init(mode) int mode;
#endif /* CK_ANSIC */
{
#ifdef KRB5
    extern char * k5_keytab;
    extern char * krb5_d_srv;
#endif /* KRB5 */
    static int last_ssl_mode = -1;
    SSL * ssl_conx=NULL, * tls_conx=NULL;

    ssl_initialized = 0;

    if ( !ck_ssleay_is_installed() )
        return(0);

    debug(F111,"ssl_tn_init","mode",mode);

    if (ssl_debug_flag)
        printf("SSL_DEBUG_FLAG on\r\n");

    if (last_ssl_mode != mode) {
        if (ssl_ctx) {
            SSL_CTX_free(ssl_ctx);
            ssl_ctx = NULL;
        }
        if (tls_ctx) {
            SSL_CTX_free(tls_ctx);
            tls_ctx = NULL;
        }
    }

    if ( (last_ssl_mode != mode) || !ssl_ctx || !tls_ctx ) {
        if ( mode == SSL_CLIENT ) {
            ssl_ctx=(SSL_CTX *)SSL_CTX_new(SSLv23_client_method());
            /* This can fail because we do not have RSA available */
            if ( !ssl_ctx ) {
                debug(F110,"ssl_tn_init","SSLv23_client_method failed",0);
                ssl_ctx=(SSL_CTX *)SSL_CTX_new(SSLv3_client_method());
            }
            if ( !ssl_ctx ) {
                debug(F110,"ssl_tn_init","SSLv3_client_method failed",0);
                last_ssl_mode = -1;
                return(0);
            }
#ifndef COMMENT
            tls_ctx=(SSL_CTX *)SSL_CTX_new(TLSv1_client_method());
#else /* COMMENT */
            tls_ctx=(SSL_CTX *)SSL_CTX_new(SSLv23_client_method());
            /* This can fail because we do not have RSA available */
            if ( !tls_ctx ) {
                debug(F110,"ssl_tn_init","SSLv23_client_method failed",0);
                tls_ctx=(SSL_CTX *)SSL_CTX_new(SSLv3_client_method());
            }
#endif /* COMMENT */
            if ( !tls_ctx ) {
                debug(F110,"ssl_tn_init","TLSv1_client_method failed",0);
                last_ssl_mode = -1;
                return(0);
            }
#ifdef USE_CERT_CB
            SSL_CTX_set_client_cert_cb(ssl_ctx,ssl_client_cert_callback);
            SSL_CTX_set_client_cert_cb(tls_ctx,ssl_client_cert_callback);
#endif /* USE_CERT_CB */
        } else if (mode == SSL_SERVER) {
            /* We are a server */
            ssl_ctx=(SSL_CTX *)SSL_CTX_new(SSLv23_server_method());
            /* This can fail because we do not have RSA available */
            if ( !ssl_ctx ) {
                debug(F110,"ssl_tn_init","SSLv23_server_method failed",0);
                ssl_ctx=(SSL_CTX *)SSL_CTX_new(SSLv3_server_method());
            }
            if ( !ssl_ctx ) {
                debug(F110,"ssl_tn_init","SSLv3_server_method failed",0);
                last_ssl_mode = -1;
                return(0);
            }
#ifdef COMMENT
            tls_ctx=(SSL_CTX *)SSL_CTX_new(TLSv1_server_method());
#else /* COMMENT */
            tls_ctx=(SSL_CTX *)SSL_CTX_new(SSLv23_server_method());
            /* This can fail because we do not have RSA available */
            if ( !tls_ctx ) {
                debug(F110,"ssl_tn_init","SSLv23_server_method failed",0);
                tls_ctx=(SSL_CTX *)SSL_CTX_new(TLSv1_server_method());
            }
#endif /* COMMENT */
            if ( !tls_ctx ) {
                debug(F110,"ssl_tn_init","TLSv1_server_method failed",0);
                last_ssl_mode = -1;
                return(0);
            }
        } else /* Unknown mode */
            return(0);

        if ( !inserver ) {
            SSL_CTX_set_default_passwd_cb(ssl_ctx,
                                   (pem_password_cb *)ssl_passwd_callback);
            SSL_CTX_set_default_passwd_cb(tls_ctx,
                                   (pem_password_cb *)ssl_passwd_callback);
        }

        /* for SSL switch on all the interoperability and bug
         * workarounds so that we will communicate with people
         * that cannot read poorly written specs :-)
         * for TLS be sure to prevent use of SSLv2
         */
        SSL_CTX_set_options(ssl_ctx,SSL_OP_ALL|SSL_OP_NO_SSLv2);
        SSL_CTX_set_options(tls_ctx,
                 SSL_OP_NO_SSLv2|SSL_OP_SINGLE_DH_USE|SSL_OP_EPHEMERAL_RSA);

        SSL_CTX_set_info_callback(ssl_ctx,ssl_client_info_callback);
        SSL_CTX_set_info_callback(tls_ctx,ssl_client_info_callback);

#ifndef COMMENT
        /* Set the proper caching mode */
        if ( mode == SSL_SERVER ) {
            SSL_CTX_set_session_cache_mode(ssl_ctx,SSL_SESS_CACHE_SERVER);
            SSL_CTX_set_session_cache_mode(tls_ctx,SSL_SESS_CACHE_SERVER);
        } else {
            SSL_CTX_set_session_cache_mode(ssl_ctx,SSL_SESS_CACHE_CLIENT);
            SSL_CTX_set_session_cache_mode(tls_ctx,SSL_SESS_CACHE_CLIENT);
        }
        SSL_CTX_set_session_id_context(ssl_ctx,(CHAR *)"1",1);
        SSL_CTX_set_session_id_context(tls_ctx,(CHAR *)"2",1);
#else /* COMMENT */
        SSL_CTX_set_session_cache_mode(ssl_ctx,SSL_SESS_CACHE_OFF);
        SSL_CTX_set_session_cache_mode(tls_ctx,SSL_SESS_CACHE_OFF);
#endif /* COMMENT */
    }

    /* The server uses defaults for the certificate files. */
    /* The client does not.                                */
    if (mode == SSL_SERVER) {
        char cert_filepath[1024];
        const char * defdir = NULL;
        DH * dh = NULL;

        defdir = getenv("SSL_CERT_DIR");
        if ( !defdir ) {
#ifdef OS2
            defdir = exedir;
#else /* OS2 */
            defdir = X509_get_default_cert_dir();
#endif /* OS2 */
            debug(F110,"ssl_tn_init - setting default directory to",defdir,0);
        }
        if ( !defdir )
            defdir = "";

        if (!ssl_rsa_cert_file) {
            /* we need to know the fullpath to the location of the
            * certificate that we will be running with as we cannot
            * be sure of the cwd when we are launched
            */
            sprintf(cert_filepath,"%s/%s",defdir,"telnetd-rsa.pem");
            if (zchki(cert_filepath) > 0)
                makestr(&ssl_rsa_cert_file,cert_filepath);
        }
        if (ssl_rsa_cert_file && !ssl_rsa_key_file) {
            /* we need to know the fullpath to the location of the
            * certificate that we will be running with as we cannot
            * be sure of the cwd when we are launched
            */
            sprintf(cert_filepath,"%s/%s",defdir,"telnetd-rsa-key.pem");
            if (zchki(cert_filepath) > 0)
                makestr(&ssl_rsa_key_file,cert_filepath);
        }
        if (!ssl_dsa_cert_file) {
            /* we need to know the fullpath to the location of the
            * certificate that we will be running with as we cannot
            * be sure of the cwd when we are launched
            */
            sprintf(cert_filepath,"%s/%s",defdir,"telnetd-dsa.pem");
            if (zchki(cert_filepath) > 0)
                makestr(&ssl_dsa_cert_file,cert_filepath);
        }
        if (ssl_dsa_cert_file && !ssl_dh_key_file) {
            /* we need to know the fullpath to the location of the
            * certificate that we will be running with as we cannot
            * be sure of the cwd when we are launched
            */
            sprintf(cert_filepath,"%s/%s",defdir,"telnetd-dsa-key.pem");
            if (zchki(cert_filepath) > 0)
                makestr(&ssl_dh_key_file,cert_filepath);
        }
        if (!ssl_crl_dir) {
            /* we need to know the fullpath to the location of the
            * certificate that we will be running with as we cannot
            * be sure of the cwd when we are launched
            */
            sprintf(cert_filepath,"%s/crl",defdir);
            if (zchki(cert_filepath) > 0)
                makestr(&ssl_crl_dir,cert_filepath);
        }

        if (ssl_only_flag && !tls_load_certs(ssl_ctx,ssl_con,1)) {
            debug(F110,"ssl_tn_init","Unable to load SSL certs",0);
            last_ssl_mode = -1;
            return(0);
        }
        if (tls_only_flag && !tls_load_certs(tls_ctx,tls_con,1)) {
            debug(F110,"ssl_tn_init","Unable to load TLS certs",0);
            last_ssl_mode = -1;
            return(0);
        }

        if ( (last_ssl_mode != mode) || !ssl_ctx || !tls_ctx ) {
            /* we may require a temp 512 bit RSA key because of the
             * wonderful way export things work ... if so we generate
             * one now!
             */

            SSL_CTX_set_tmp_rsa_callback(ssl_ctx, tmp_rsa_cb);
            SSL_CTX_set_tmp_dh_callback( ssl_ctx, tmp_dh_cb);
            SSL_CTX_set_tmp_rsa_callback(tls_ctx, tmp_rsa_cb);
            SSL_CTX_set_tmp_dh_callback( tls_ctx, tmp_dh_cb);

            dh = tmp_dh_cb(NULL,0,512);
            SSL_CTX_set_tmp_dh(ssl_ctx,dh);
            SSL_CTX_set_tmp_dh(tls_ctx,dh);

            /* The following code is only called if we are using a
             * certificate with an RSA public key and where the
             * certificate has a key length less than 512 bits or is
             * marked for signing only.  This is so we can support
             * the greatest legal privacy level with exportable clients.
             */

            if (SSL_CTX_need_tmp_RSA(ssl_ctx) ||
                 SSL_CTX_need_tmp_RSA(tls_ctx))
            {
                RSA *rsa;

                if ( ssl_debug_flag )
                    printf("Generating temp (512 bit) RSA key ...\r\n");
                rsa=RSA_generate_key(512,RSA_F4,NULL,NULL);
                if ( ssl_debug_flag )
                    printf("Generation of temp (512 bit) RSA key done\r\n");

                if (SSL_CTX_need_tmp_RSA(ssl_ctx)) {
                    if (!SSL_CTX_set_tmp_rsa(ssl_ctx,rsa)) {
                        if ( ssl_debug_flag )
                            printf(
  "Failed to assign generated temp RSA key to SSL!\r\n");
                    }
                }
                if (SSL_CTX_need_tmp_RSA(tls_ctx)) {
                    if (!SSL_CTX_set_tmp_rsa(tls_ctx,rsa)) {
                        if ( ssl_debug_flag )
                            printf(
  "Failed to assign generated temp RSA key to TLS!\r\n");
                    }
                }
                RSA_free(rsa);
                if ( ssl_debug_flag )
                    printf("Assigned temp (512 bit) RSA key\r\n");
            }
        }
    }

    /* make sure we will find certificates in the standard
     * location ... otherwise we don't look anywhere for
     * these things which is going to make client certificate
     * exchange rather useless :-)
     * In OS2, default values for ssl_verify_file and ssl_verify_path.
     */

#ifdef OS2
#ifdef NT
    {
        /* The defaults in the SSL crypto library are not appropriate for OS/2 */
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"certs",NULL,NULL);
        if (isdir(path) && 
            SSL_CTX_load_verify_locations(tls_ctx,NULL,path) == 1)  {
            debug(F110,"ssl_tn_init certificate verify dir",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification Directory: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,NULL,path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/certs",NULL,NULL);
        if (isdir(path) &&
            SSL_CTX_load_verify_locations(tls_ctx,NULL,path) == 1)  {
            debug(F110,"ssl_tn_init certificate verify dir",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification Directory: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,NULL,path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/certs",NULL,NULL);
        if (isdir(path) &&
            SSL_CTX_load_verify_locations(tls_ctx,NULL,path) == 1)  {
            debug(F110,"ssl_tn_init certificate verify dir",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification Directory: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,NULL,path);
        }
        ckmakmsg(path,CKMAXPATH,exedir,"ca_certs.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            SSL_CTX_load_verify_locations(tls_ctx,path,NULL) == 1) {
            debug(F110,"ssl_tn_init certificate verify file",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification File: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,path,NULL);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/ca_certs.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            SSL_CTX_load_verify_locations(tls_ctx,path,NULL) == 1) {
            debug(F110,"ssl_tn_init certificate verify file",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification File: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,path,NULL);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/ca_certs.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            SSL_CTX_load_verify_locations(tls_ctx,path,NULL) == 1) {
            debug(F110,"ssl_tn_init certificate verify file",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification File: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,path,NULL);
        }
    }
#else /* NT */
    {
        /* The defaults in the SSL crypto library are not appropriate for OS/2 */
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"certs",NULL,NULL);
        if (isdir(path) && 
            SSL_CTX_load_verify_locations(tls_ctx,NULL,path) == 1)  {
            debug(F110,"ssl_tn_init certificate verify dir",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification Directory: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,NULL,path);
        }
        ckmakmsg(path,CKMAXPATH,exedir,"ca_certs.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            SSL_CTX_load_verify_locations(tls_ctx,path,NULL) == 1) {
            debug(F110,"ssl_tn_init certificate verify file",path,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification File: %s\r\n",path);
            SSL_CTX_load_verify_locations(ssl_ctx,path,NULL);
        }
    }
#endif /* NT */
#else /* OS2 */
    SSL_CTX_set_default_verify_paths(ssl_ctx);
    SSL_CTX_set_default_verify_paths(tls_ctx);
#endif /* OS2 */

    if (ssl_verify_file) {
        if (zchki(ssl_verify_file) > 0 && 
            SSL_CTX_load_verify_locations(tls_ctx,ssl_verify_file,NULL) == 1) {
            debug(F110,"ssl_tn_init certificate verify file",ssl_verify_file,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification File: %s\r\n",ssl_verify_file);
            SSL_CTX_load_verify_locations(ssl_ctx,ssl_verify_file,NULL);
        }
    }
    if (ssl_verify_dir && isdir(ssl_verify_dir)) {
        if (SSL_CTX_load_verify_locations(tls_ctx,NULL,ssl_verify_dir) == 1)  {
            debug(F110,"ssl_tn_init certificate verify dir",ssl_verify_dir,0);
            if (ssl_debug_flag)
                printf("  Certificate Verification Directory: %s\r\n",ssl_verify_dir);
            SSL_CTX_load_verify_locations(ssl_ctx,NULL,ssl_verify_dir);
        }
    }
    if (mode == SSL_SERVER) {
        SSL_CTX_set_verify(ssl_ctx,
                     ssl_verify_flag?ssl_verify_flag|SSL_VERIFY_CLIENT_ONCE:0,
                           ssl_server_verify_callback);
        SSL_CTX_set_verify(tls_ctx,
                     ssl_verify_flag?ssl_verify_flag|SSL_VERIFY_CLIENT_ONCE:0,
                           ssl_server_verify_callback);
    } else {
        SSL_CTX_set_verify(ssl_ctx,ssl_verify_flag,
                           ssl_client_verify_callback);
        SSL_CTX_set_verify(tls_ctx,ssl_verify_flag,
                           ssl_client_verify_callback);
    }

    /* Free the existing CRL Store */
    if (crl_store) {
        X509_STORE_free(crl_store);
        crl_store = NULL;
    }

    /* set up the new CRL Store */
    crl_store = X509_STORE_new();
    if (crl_store) {
#ifdef OS2
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"crls",NULL,NULL);
        if (isdir(path) &&
            X509_STORE_load_locations(crl_store,NULL,path) == 1) {
            debug(F110,"ssl_tn_init crl dir",path,0);
            if (ssl_debug_flag)
                printf("  CRL Directory: %s\r\n",path);
        }
#ifdef NT
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/crls",NULL,NULL);
        if (isdir(path) && 
            X509_STORE_load_locations(crl_store,NULL,path) == 1) {
            debug(F110,"ssl_tn_init crl dir",path,0);
            if (ssl_debug_flag)
                printf("  CRL Directory: %s\r\n",path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/crls",NULL,NULL);
        if (isdir(path) && 
            X509_STORE_load_locations(crl_store,NULL,path) == 1) {
            debug(F110,"ssl_tn_init crl dir",path,0);
            if (ssl_debug_flag)
                printf("  CRL Directory: %s\r\n",path);
        }
#endif /* NT */
        
        ckmakmsg(path,CKMAXPATH,exedir,"ca_crls.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            X509_STORE_load_locations(crl_store,path,NULL) == 1) {
            debug(F110,"ssl_tn_init crl file",path,0);
            if (ssl_debug_flag)
                printf("  CRL File: %s\r\n",path);
        }
#ifdef NT
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/ca_crls.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            X509_STORE_load_locations(crl_store,path,NULL) == 1) {
            debug(F110,"ssl_tn_init crl file",path,0);
            if (ssl_debug_flag)
                printf("  CRL File: %s\r\n",path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/ca_crls.pem",NULL,NULL);
        if (zchki(path) > 0 && 
            X509_STORE_load_locations(crl_store,path,NULL) == 1) {
            debug(F110,"ssl_tn_init crl file",path,0);
            if (ssl_debug_flag)
                printf("  CRL File: %s\r\n",path);
        }
#endif /* NT */
#endif /* OS2 */

        if (ssl_crl_file || ssl_crl_dir) {
            if (ssl_crl_file && zchki(ssl_crl_file) > 0 && 
                X509_STORE_load_locations(crl_store,ssl_crl_file,NULL) == 1) {
                debug(F110,"ssl_tn_init crl file",ssl_crl_file,0);
                if (ssl_debug_flag)
                    printf("  CRL File: %s\r\n",ssl_crl_file);
            }
            if (ssl_crl_dir && isdir(ssl_crl_dir) &&
                X509_STORE_load_locations(crl_store,NULL,ssl_crl_dir) == 1) {
                debug(F110,"ssl_tn_init crl dir",ssl_crl_dir,0);
                if (ssl_debug_flag)
                    printf("  CRL Directory: %s\r\n",ssl_crl_dir);
            }
        } 
#ifndef OS2
        else {
            X509_STORE_set_default_paths(crl_store);
        }
#endif /* OS2 */
    }

#ifndef COMMENT
    ssl_conx = ssl_con;
    ssl_con=(SSL *)SSL_new(ssl_ctx);
    if ( !ssl_con ) {
        debug(F110,"ssl_tn_init","SSL_new(ssl_con) failed",0);
        last_ssl_mode = -1;
        ssl_con = ssl_conx;
        return(0);
    }
    if (ssl_conx) {
        if ( mode == SSL_CLIENT ) {
            SSL_set_session(ssl_con, SSL_get_session(ssl_conx));
        }
#ifdef SSL_KRB5
		if (ssl_conx->kssl_ctx) {
			kssl_ctx_free(ssl_conx->kssl_ctx);
			ssl_conx->kssl_ctx = NULL;
		}
#endif /* SSL_KRB5 */
        SSL_free(ssl_conx);
        ssl_conx = NULL;
    }
    tls_conx = tls_con;
    tls_con=(SSL *)SSL_new(tls_ctx);
    if ( !tls_con ) {
        debug(F110,"ssl_tn_init","SSL_new(tls_con) failed",0);
        last_ssl_mode = -1;
        tls_con = tls_conx;
        return(0);
    }
    if (tls_conx) {
        if ( mode == SSL_CLIENT )
            SSL_set_session(tls_con, SSL_get_session(tls_conx));
#ifdef SSL_KRB5
		if (tls_conx->kssl_ctx) {
			kssl_ctx_free(tls_conx->kssl_ctx);
			tls_conx->kssl_ctx = NULL;
		}
#endif /* SSL_KRB5 */
        SSL_free(tls_conx);
        tls_conx = NULL;
    }
#else /* COMMENT */
    /* I don't know why this does not work to reuse the connection. */
    if ( ssl_con ) {
        SSL_clear(ssl_con);
        SSL_set_session(ssl_con,NULL);
        SSL_set_accept_state(ssl_con) ;
    } else {
        ssl_con=(SSL *)SSL_new(ssl_ctx);
        if (!ssl_con) {
            debug(F110,"ssl_tn_init","SSL_new(ssl_ctx) failed",0);
            last_ssl_mode = -1;
            ssl_con = ssl_conx;
            return(0);
        }
    }

    if ( tls_con ) {
        SSL_clear(tls_con);
        SSL_set_session(tls_con,NULL);
        SSL_set_accept_state(tls_con) ;
    } else {
        tls_con=(SSL *)SSL_new(tls_ctx);
        if ( !tls_con ) {
            debug(F110,"ssl_tn_init","SSL_new(tls_ctx) failed",0);
            last_ssl_mode = -1;
            tls_con = tls_conx;
            return(0);
        }
    }
#endif /* COMMENT */

#ifdef SSL_KRB5
#ifndef KRB5_SERVICE_NAME
#define KRB5_SERVICE_NAME    "host"
#endif

    if (ssl_con->kssl_ctx == NULL)
        ssl_con->kssl_ctx = kssl_ctx_new();
    if (tls_con->kssl_ctx == NULL)
    tls_con->kssl_ctx = kssl_ctx_new();
    if (mode == SSL_SERVER) {
        if (ssl_con->kssl_ctx != NULL)
            kssl_ctx_setstring(ssl_con->kssl_ctx, KSSL_KEYTAB, k5_keytab);
        if (tls_con->kssl_ctx != NULL)
            kssl_ctx_setstring(tls_con->kssl_ctx, KSSL_KEYTAB, k5_keytab);
    } else {
        if (ssl_con->kssl_ctx != NULL)
            kssl_ctx_setstring(ssl_con->kssl_ctx, KSSL_SERVER, szHostName);
        if (tls_con->kssl_ctx != NULL)
            kssl_ctx_setstring(tls_con->kssl_ctx, KSSL_SERVER, szHostName);
    }
    kssl_ctx_setstring(ssl_con->kssl_ctx, KSSL_SERVICE,
                        krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME);
    kssl_ctx_setstring(tls_con->kssl_ctx, KSSL_SERVICE,
                        krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME);
#endif /* SSL_KRB5 */

    if (ssl_cipher_list) {
        SSL_set_cipher_list(ssl_con,ssl_cipher_list);
        SSL_set_cipher_list(tls_con,ssl_cipher_list);
    } else {
        char * p;
        if (p = getenv("SSL_CIPHER")) {
            SSL_set_cipher_list(ssl_con,p);
            SSL_set_cipher_list(tls_con,p);
        } else {
            SSL_set_cipher_list(ssl_con,DEFAULT_CIPHER_LIST);
            SSL_set_cipher_list(tls_con,DEFAULT_CIPHER_LIST);
        }
    }

    ssl_verify_depth = -1;

    if ( ssl_debug_flag )
        printf("SSL/TLS init done!\r\n");

    ssl_initialized = 1;
    last_ssl_mode = mode;
    debug(F110,"ssl_tn_init","done",0);
    return(1);
}

#ifndef NOHTTP
int
#ifdef CK_ANSIC
ssl_http_init(char * hostname)
#else
ssl_http_init(hostname) char * hostname;
#endif /* CK_ANSIC */
{
#ifdef KRB5
    extern char * k5_keytab;
    extern char * krb5_d_srv;
#endif /* KRB5 */
    SSL * tls_conx=NULL;

    ssl_http_initialized = 0;

    if ( !ck_ssleay_is_installed() )
        return(0);
    debug(F110,"ssl_http_init",hostname,0);

    if (ssl_debug_flag)
        printf("SSL_DEBUG_FLAG on\r\n");

    if (!tls_http_ctx ) {
#ifdef COMMENT
        /* too many web servers still do not support TLSv1 */
        tls_http_ctx=(SSL_CTX *)SSL_CTX_new(TLSv1_client_method());
#else /* COMMENT */
        tls_http_ctx=(SSL_CTX *)SSL_CTX_new(SSLv23_client_method());
        /* This can fail because we do not have RSA available */
        if ( !tls_http_ctx ) {
            debug(F110,"ssl_http_init","SSLv23_client_method failed",0);
            tls_http_ctx=(SSL_CTX *)SSL_CTX_new(SSLv3_client_method());
        }
#endif /* COMMENT */
        if ( !tls_http_ctx ) {
            debug(F110,"ssl_http_init","TLSv1_client_method failed",0);
            return(0);
        }
#ifdef USE_CERT_CB
        SSL_CTX_set_client_cert_cb(tls_http_ctx,ssl_client_cert_callback);
#endif /* USE_CERT_CB */
    }

    SSL_CTX_set_default_passwd_cb(tls_http_ctx,
                                  (pem_password_cb *)ssl_passwd_callback);

    /* for SSL switch on all the interoperability and bug
     * workarounds so that we will communicate with people
     * that cannot read poorly written specs :-)
     * for TLS be sure to prevent use of SSLv2
     */
    SSL_CTX_set_options(tls_http_ctx,
            SSL_OP_NO_SSLv2|SSL_OP_SINGLE_DH_USE|SSL_OP_EPHEMERAL_RSA);

    SSL_CTX_set_info_callback(tls_http_ctx,ssl_client_info_callback);

#ifndef COMMENT
    SSL_CTX_set_session_cache_mode(tls_http_ctx,SSL_SESS_CACHE_CLIENT);
    SSL_CTX_set_session_id_context(tls_http_ctx,(CHAR *)"3",1);
#else /* COMMENT */
    SSL_CTX_set_session_cache_mode(tls_http_ctx,SSL_SESS_CACHE_OFF);
#endif /* COMMENT */

    /* make sure we will find certificates in the standard
     * location ... otherwise we don't look anywhere for
     * these things which is going to make client certificate
     * exchange rather useless :-)
     */

#ifdef OS2
#ifdef NT
    {
        /* The defaults in the SSL crypto library are not appropriate for OS/2 */
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"certs",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,NULL,path) == 0)  {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-dir: %s\r\n",path);
        }

        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/certs",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,NULL,path) == 0)  {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-dir: %s\r\n",path);
        }

        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/certs",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,NULL,path) == 0)  {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-dir: %s\r\n",path);
        }

        ckmakmsg(path,CKMAXPATH,exedir,"ca_certs.pem",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-file: %s\r\n",path);
        }

        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/ca_certs.pem",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-file: %s\r\n",path);
        }

        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/ca_certs.pem",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-file: %s\r\n",path);
        }
    }
#else /* NT */
    {
        /* The defaults in the SSL crypto library are not appropriate for OS/2 */
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"certs",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,NULL,path) == 0)  {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-dir: %s\r\n",path);
        }
        ckmakmsg(path,CKMAXPATH,exedir,"ca_certs.pem",NULL,NULL);
        if (SSL_CTX_load_verify_locations(tls_http_ctx,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load path",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load verify-file: %s\r\n",path);
        }
    }
#endif /* NT */
#else /* OS2 */
    SSL_CTX_set_default_verify_paths(tls_http_ctx);
#endif /* OS2 */

    if (ssl_verify_file &&
        SSL_CTX_load_verify_locations(tls_http_ctx,ssl_verify_file,NULL) == 0)  {
        debug(F110,"ssl_http_init unable to load ssl_verify_file",ssl_verify_file,0);
        if (ssl_debug_flag)
            printf("?Unable to load verify-file: %s\r\n",ssl_verify_file);
    }
    if (ssl_verify_dir &&
        SSL_CTX_load_verify_locations(tls_http_ctx,NULL,ssl_verify_dir) == 0)  {
        debug(F110,"ssl_http_init unable to load ssl_verify_dir",ssl_verify_dir,0);
        if (ssl_debug_flag)
            printf("?Unable to load verify-dir: %s\r\n",ssl_verify_dir);
    }

    SSL_CTX_set_verify(tls_http_ctx,ssl_verify_flag,
                           ssl_client_verify_callback);

    /* Free the existing CRL Store */
    if (crl_store) {
        X509_STORE_free(crl_store);
        crl_store = NULL;
    }

    /* set up the new CRL Store */
    crl_store = X509_STORE_new();
    if (crl_store) {
#ifdef OS2
        char path[CKMAXPATH];

        ckmakmsg(path,CKMAXPATH,exedir,"crls",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,NULL,path) == 0) {
            debug(F110,"ssl_http_init unable to load dir",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-dir: %s\r\n",path);
        }
#ifdef NT
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/crls",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,NULL,path) == 0) {
            debug(F110,"ssl_http_init unable to load dir",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-dir: %s\r\n",path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/crls",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,NULL,path) == 0) {
            debug(F110,"ssl_http_init unable to load dir",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-dir: %s\r\n",path);
        }
#endif /* NT */
        
        ckmakmsg(path,CKMAXPATH,exedir,"ca_crls.pem",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load file",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-file: %s\r\n",path);
        }
#ifdef NT
        ckmakmsg(path,CKMAXPATH,GetAppData(1),"kermit 95/ca_crls.pem",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load file",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-file: %s\r\n",path);
        }
        ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/ca_crls.pem",NULL,NULL);
        if (X509_STORE_load_locations(crl_store,path,NULL) == 0) {
            debug(F110,"ssl_http_init unable to load file",path,0);
            if (ssl_debug_flag)
                printf("?Unable to load crl-file: %s\r\n",path);
        }
#endif /* NT */
#endif /* OS2 */

        if (ssl_crl_file || ssl_crl_dir) {
            if (ssl_crl_file &&
                X509_STORE_load_locations(crl_store,ssl_crl_file,NULL) == 0) {
                debug(F110,"ssl_http_init unable to load ssl_crl_file",ssl_crl_file,0);
                if (ssl_debug_flag)
                    printf("?Unable to load crl-file: %s\r\n",ssl_crl_file);
            }
            if (ssl_crl_dir &&
                X509_STORE_load_locations(crl_store,NULL,ssl_crl_dir) == 0) {
                debug(F110,"ssl_http_init unable to load ssl_crl_dir",ssl_crl_dir,0);
                if (ssl_debug_flag)
                    printf("?Unable to load crl-dir: %s\r\n",ssl_crl_dir);
            }
        } else {
            X509_STORE_set_default_paths(crl_store);
        }
    }

#ifndef COMMENT
    tls_conx = tls_http_con;
    tls_http_con=(SSL *)SSL_new(tls_http_ctx);
    if ( !tls_http_con ) {
        debug(F110,"ssl_http_init","SSL_new(tls_http_con) failed",0);
        tls_http_con = tls_conx;
        return(0);
    }
    if (tls_conx) {
        SSL_set_session(tls_http_con, SSL_get_session(tls_conx));
#ifdef SSL_KRB5
		if (tls_conx->kssl_ctx) {
			kssl_ctx_free(tls_conx->kssl_ctx);
			tls_conx->kssl_ctx = NULL;
		}
#endif /* SSL_KRB5 */
        SSL_free(tls_conx);
        tls_conx = NULL;
    }
#else /* COMMENT */
    /* I don't know why this does not work to reuse the connection. */
    if ( tls_http_con ) {
        SSL_clear(tls_http_con);
        SSL_set_session(tls_http_con,NULL);
        SSL_set_accept_state(tls_http_con) ;
    } else {
        tls_http_con=(SSL *)SSL_new(tls_http_ctx);
        if ( !tls_http_con ) {
            debug(F110,"ssl_http_init","SSL_new(tls_http_ctx) failed",0);
            tls_http_con = tls_conx;
            return(0);
        }
    }
#endif /* COMMENT */

#ifdef SSL_KRB5
#ifndef KRB5_SERVICE_NAME
#define KRB5_SERVICE_NAME    "host"
#endif

    if (tls_http_con->kssl_ctx == NULL)
    tls_http_con->kssl_ctx = kssl_ctx_new();
    if (tls_http_con->kssl_ctx != NULL)
        kssl_ctx_setstring(tls_http_con->kssl_ctx, KSSL_SERVER, hostname);

    kssl_ctx_setstring(tls_http_con->kssl_ctx, KSSL_SERVICE,
                        krb5_d_srv ? krb5_d_srv : KRB5_SERVICE_NAME);
#endif /* SSL_KRB5 */

    if (ssl_cipher_list)
        SSL_set_cipher_list(tls_http_con,ssl_cipher_list);
    else {
        char * p;
        if (p = getenv("SSL_CIPHER")) {
            SSL_set_cipher_list(tls_http_con,p);
        } else {
            SSL_set_cipher_list(tls_http_con,DEFAULT_CIPHER_LIST);
        }
    }

    ssl_verify_depth = -1;

    if ( ssl_debug_flag )
        printf("SSL/TLS init done!\r\n");

    ssl_http_initialized = 1;
    return(1);
}
#endif /* NOHTTP */

char *
ssl_get_dNSName(ssl) SSL * ssl;
{
    static char *dns = NULL;
    X509 *server_cert = NULL;
    int i;
    X509_EXTENSION *ext = NULL;
    STACK_OF(GENERAL_NAME) *ialt = NULL;
    GENERAL_NAME *gen = NULL;

    if ( dns ) {
        free(dns);
        dns = NULL;
    }

    if (server_cert = SSL_get_peer_certificate(ssl)) {
        if ((i = X509_get_ext_by_NID(server_cert, NID_subject_alt_name, -1))<0)
            return NULL;
        if (!(ext = X509_get_ext(server_cert, i)))
            return NULL;
        X509V3_add_standard_extensions();
        if (!(ialt = X509V3_EXT_d2i(ext)))
            return NULL;
        for (i = 0; i < sk_GENERAL_NAME_num(ialt); i++) {
            gen = sk_GENERAL_NAME_value(ialt, i);
            if (gen->type == GEN_DNS) {
                if (!gen->d.ia5 || !gen->d.ia5->length)
		  break;
                if (strlen((char *)gen->d.ia5->data) != gen->d.ia5->length) {
                    /* Ignoring IA5String containing null character */
                    continue;
                }
                dns = malloc(gen->d.ia5->length + 1);
                if (dns) {
                    memcpy(dns, gen->d.ia5->data, gen->d.ia5->length);
                    dns[gen->d.ia5->length] = 0;
                }
                break;
            }
        }
        X509V3_EXT_cleanup();
    }
cleanup:
    if (ialt)           sk_GENERAL_NAME_free(ialt);
    if (server_cert)    X509_free(server_cert);
    return dns;
}

char *
ssl_get_commonName(ssl) SSL * ssl; {
    static char name[256];
    int name_text_len;
    int err;
    X509 *server_cert;

    name_text_len = 0;
    if (server_cert = SSL_get_peer_certificate(ssl)) {
        name_text_len =
	    X509_NAME_get_text_by_NID(X509_get_subject_name(server_cert),
				      NID_commonName, name, sizeof(name));
        X509_free(server_cert);
    }
    if (name_text_len <= 0) {
	/* Common Name was empty or not retrieved */
        err = 0;
    } else if (strlen(name) != name_text_len) {
        /* Ignoring Common Name containing null character */
	err = 0;
    } else {
	err = 1;
    }
    if (err > 0)
      return name;
    else
      return NULL;
}

char *
ssl_get_issuer_name(ssl) SSL * ssl;
{
    static char name[256];
    X509 *server_cert;

    name[0] = '\0';
    if (server_cert = SSL_get_peer_certificate(ssl)) {
        X509_NAME_oneline(X509_get_issuer_name(server_cert),name,sizeof(name));
        X509_free(server_cert);
        return name;
    }
    else {
#ifdef COMMENT
      fprintf(stderr, "Warning: No certificate from server!\r\n");
#endif /* COMMENT */
        return NULL;
    }
}

char *
ssl_get_subject_name(ssl) SSL * ssl;
{
    static char name[256];
    X509 *server_cert;

    name[0] = '\0';
    if (server_cert = SSL_get_peer_certificate(ssl)) {
       X509_NAME_oneline(X509_get_subject_name(server_cert),name,sizeof(name));
       X509_free(server_cert);
       return name;
    }
    else
        return NULL;
}

#ifdef COMMENT
#ifdef CK_SSL
            && !(ck_ssleay_is_installed() &&
               (tls_active_flag || ssl_active_flag) &&
               ssl_anonymous_cipher(tls_active_flag?tls_con:ssl_con))
#endif /* CK_SSL */

int
ssl_anonymous_cipher(ssl) SSL * ssl;
{
    X509 * cert;

    if (sstelnet)
        cert = SSL_get_certificate(ssl);
    else
        cert = SSL_get_peer_certificate(ssl);

    if ( cert ) {
        X509_free(cert);
        return 0;
    }
    return 1;
}
#endif /* COMMENT */

/*
  This one is (very much!) based on work by
  Ralf S. Engelschall <rse@engelschall.com>.
  Comments by Ralf.
*/
int
ssl_verify_crl(int ok, X509_STORE_CTX *ctx)
{
    X509_OBJECT obj;
    X509_NAME *subject = NULL;
    X509_NAME *issuer = NULL;
    X509 *xs = NULL;
    X509_CRL *crl = NULL;
    X509_REVOKED *revoked = NULL;
    X509_STORE_CTX * store_ctx = NULL;
    long serial;
    BIO *bio = NULL;
    int i, n, rc;
    char *cp;
    char *cp2;

    /*
     * Unless a revocation store for CRLs was created we
     * cannot do any CRL-based verification, of course.
     */
    if (!crl_store)
        return ok;

    store_ctx = X509_STORE_CTX_new();
    if ( !store_ctx )
        return(ok);

    /*
     * Determine certificate ingredients in advance
     */
    xs      = X509_STORE_CTX_get_current_cert(ctx);
    subject = X509_get_subject_name(xs);
    issuer  = X509_get_issuer_name(xs);

    /*
     * OpenSSL provides the general mechanism to deal with CRLs but does not
     * use them automatically when verifying certificates, so we do it
     * explicitly here. We will check the CRL for the currently checked
     * certificate, if there is such a CRL in the store.
     *
     * We come through this procedure for each certificate in the certificate
     * chain, starting with the root-CA's certificate. At each step we've to
     * both verify the signature on the CRL (to make sure it's a valid CRL)
     * and it's revocation list (to make sure the current certificate isn't
     * revoked).  But because to check the signature on the CRL we need the
     * public key of the issuing CA certificate (which was already processed
     * one round before), we've a little problem. But we can both solve it and
     * at the same time optimize the processing by using the following
     * verification scheme (idea and code snippets borrowed from the GLOBUS
     * project):
     *
     * 1. We'll check the signature of a CRL in each step when we find a CRL
     *    through the _subject_ name of the current certificate. This CRL
     *    itself will be needed the first time in the next round, of course.
     *    But we do the signature processing one round before this where the
     *    public key of the CA is available.
     *
     * 2. We'll check the revocation list of a CRL in each step when
     *    we find a CRL through the _issuer_ name of the current certificate.
     *    This CRLs signature was then already verified one round before.
     *
     * This verification scheme allows a CA to revoke its own certificate as
     * well, of course.
     */

    /*
     * Try to retrieve a CRL corresponding to the _subject_ of
     * the current certificate in order to verify it's integrity.
     */
    memset((char *)&obj, 0, sizeof(obj));
    X509_STORE_CTX_init(store_ctx, crl_store, NULL, NULL);
    rc = X509_STORE_get_by_subject(store_ctx, X509_LU_CRL, subject, &obj);
    X509_STORE_CTX_cleanup(store_ctx);
    crl = obj.data.crl;
    if (rc > 0 && crl != NULL) {
        /*
         * Verify the signature on this CRL
         */
        if (X509_CRL_verify(crl, X509_get_pubkey(xs)) <= 0) {
            fprintf(stderr, "Invalid signature on CRL!\n");
            X509_STORE_CTX_set_error(ctx, X509_V_ERR_CRL_SIGNATURE_FAILURE);
            X509_OBJECT_free_contents(&obj);
            X509_STORE_CTX_free(store_ctx);
            return 0;
        }

        /*
         * Check date of CRL to make sure it's not expired
         */
        i = X509_cmp_current_time(X509_CRL_get_nextUpdate(crl));
        if (i == 0) {
            fprintf(stderr, "Found CRL has invalid nextUpdate field.\n");
            X509_STORE_CTX_set_error(ctx,
                                    X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD);
            X509_OBJECT_free_contents(&obj);
            X509_STORE_CTX_free(store_ctx);
            return 0;
        }
        if (i < 0) {
            fprintf(stderr,
"Found CRL is expired - revoking all certificates until you get updated CRL.\n"
                    );
            X509_STORE_CTX_set_error(ctx, X509_V_ERR_CRL_HAS_EXPIRED);
            X509_OBJECT_free_contents(&obj);
            X509_STORE_CTX_free(store_ctx);
            return 0;
        }
        X509_OBJECT_free_contents(&obj);
    }

    /*
     * Try to retrieve a CRL corresponding to the _issuer_ of
     * the current certificate in order to check for revocation.
     */
    memset((char *)&obj, 0, sizeof(obj));
    X509_STORE_CTX_init(store_ctx, crl_store, NULL, NULL);
    rc = X509_STORE_get_by_subject(store_ctx, X509_LU_CRL, issuer, &obj);
    X509_STORE_CTX_free(store_ctx);		/* calls X509_STORE_CTX_cleanup() */
    crl = obj.data.crl;
    if (rc > 0 && crl != NULL) {
        /*
         * Check if the current certificate is revoked by this CRL
         */
        n = sk_X509_REVOKED_num(X509_CRL_get_REVOKED(crl));
        for (i = 0; i < n; i++) {
            revoked = sk_X509_REVOKED_value(X509_CRL_get_REVOKED(crl), i);
            if (ASN1_INTEGER_cmp(revoked->serialNumber,
                                 X509_get_serialNumber(xs)) == 0) {

                serial = ASN1_INTEGER_get(revoked->serialNumber);
                cp = X509_NAME_oneline(issuer, NULL, 0);
                free(cp);

                X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_REVOKED);
                X509_OBJECT_free_contents(&obj);
                return 0;
            }
        }
        X509_OBJECT_free_contents(&obj);
    }
    return ok;
}

char *
tls_userid_from_client_cert(ssl) SSL * ssl;
{
    static char cn[256];
    static char *r = cn;
    int err;
    X509 *client_cert;

    if (client_cert = SSL_get_peer_certificate(ssl)) {
        /* call the custom function */
        err = X509_to_user(client_cert, cn, sizeof(cn));
        X509_free(client_cert);
        if (err)
            return r = NULL;
        else
            return r;
    }
    else
        return r = NULL;
}

unsigned char **
tls_get_SAN_objs(SSL * ssl, int type)
/* returns NULL or an array of malloc'ed objects of type `type' from the server's
 * subjectAltName, remember to free() them all!
 */
{
#define NUM_SAN_OBJS 64
    static unsigned char *objs[NUM_SAN_OBJS];
    unsigned char **rv = NULL;
    X509 *server_cert = NULL;
    int i, j;
    X509_EXTENSION *ext = NULL;
    STACK_OF(GENERAL_NAME) *ialt = NULL;
    GENERAL_NAME *gen = NULL;

    memset(objs, 0, sizeof(objs));
    if (server_cert = SSL_get_peer_certificate(ssl)) {
        if ((i = X509_get_ext_by_NID(server_cert, NID_subject_alt_name, -1)) < 0)
            goto eject;
        if (!(ext = X509_get_ext(server_cert, i)))
            goto eject;
        X509V3_add_standard_extensions();
        if (!(ialt = X509V3_EXT_d2i(ext)))
            goto eject;
        rv = objs;
        for (i = 0, j = 0; i < sk_GENERAL_NAME_num(ialt) && j < NUM_SAN_OBJS - 2; i++) {
            gen = sk_GENERAL_NAME_value(ialt, i);
            /* The use of V_ASN1_CONTEXT_SPECIFIC is because OpenSSL 0.9.6 defined its
             * types | V_ASN1_CONTEXT_SPECIFIC.  0.9.7 does not.  In case, we are built
             * with one and linked to the other we use this hack.
             */
            if ((gen->type | V_ASN1_CONTEXT_SPECIFIC) == (type | V_ASN1_CONTEXT_SPECIFIC)) {
                if (!gen->d.ia5 || !gen->d.ia5->length)
		  break;
                if (strlen((char *)gen->d.ia5->data) != gen->d.ia5->length) {
                    /* Ignoring IA5String containing null character */
                    continue;
                }
                objs[j] = malloc(gen->d.ia5->length + 1);
                if (objs[j]) {
                    memcpy(objs[j], gen->d.ia5->data, gen->d.ia5->length);
                    objs[j][gen->d.ia5->length] = 0;
                    j++;
                }
            }
        }
        X509V3_EXT_cleanup();
    }
eject:
    if (ialt)           sk_GENERAL_NAME_free(ialt);
    if (server_cert)    X509_free(server_cert);
    return rv;
}


static int
dNSName_cmp(const char *host, const char *dNSName)
{
    int c1 = 1, c2 = 1, num_comp, rv = -1;
    char *p, *p1, *p2, *host_copy=NULL, *dNSName_copy=NULL;

    /* first we count the number of domain name components in both parameters.
     * they should be equal many, or it's not a match
     */
    p = (char *) host;
    while (p = strstr(p, ".")) {
        c1++;
        p++;
    }
    p = (char *) dNSName;
    while (p = strstr(p, ".")) {
        c2++;
        p++;
    }
    if (c1 != c2)
        return -1;
    num_comp = c1;

    makestr(&host_copy,host);
    makestr(&dNSName_copy,dNSName);
    if (host_copy == NULL || dNSName_copy == NULL)
        goto eject;
    /* make substrings by replacing '.' with '\0' */
    p = dNSName_copy;
    while (p = strstr(p, ".")) {
        *p = '\0';
        p++;
    }
    p = host_copy;
    while (p = strstr(p, ".")) {
        *p = '\0';
        p++;
    }

    /* compare each component */
    p1 = host_copy;
    p2 = dNSName_copy;
    for (; num_comp; num_comp--) {
        if (!ckmatch(p2, p1,0,1))
            /* failed match */
            goto eject;
        p1 += strlen(p1) + 1;
        p2 += strlen(p2) + 1;
    }
    /* match ok */
    rv = 0;

  eject:
    if (dNSName_copy)   free(dNSName_copy);
    if (host_copy)      free(host_copy);
    return rv;
}



static int
show_hostname_warning(char *s1, char *s2)
{
    char prefix[1024];
    int ok = 1;
    setverbosity();
    ckmakxmsg(prefix,1024,
              "Warning: Hostname (\"", s1, 
              "\") does not match server's certificate (\"", s2, "\")",
              NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    if (ssl_verify_flag)
        ok = uq_ok(prefix,
                    "Continue? (Y/N) ",
                    3, NULL, 0);
    else if (verbosity)
        printf(prefix);
    return(ok);
}

#ifndef OSF50
#ifndef HPUX10
#ifndef HPUX1100
#ifndef SCO_OSR505
#ifndef OpenBSD
#ifndef FREEBSD4
#ifndef NETBSD15
#ifndef LINUX
#ifndef AIX41
#ifndef UW7
#ifndef IRIX65
#ifndef SOLARIS9
#ifndef SOLARIS8
#ifndef SOLARIS7
#ifndef MACOSX
#ifdef DEC_TCPIP
#define inet_aton INET_ATON
#endif /* DEC_TCPIP */

#ifndef NO_DCL_INET_ATON
static int
inet_aton(char * ipaddress, struct in_addr * ia) {
    struct stringarray * q;
    union {
        unsigned long l;
        unsigned char b[4];
    } dummy;

    q = cksplit(1,0,ipaddress,".","0123456789abcdefACDEF",8,0,0);
    if (q->a_size == 4) {
        dummy.b[0] = atoi(q->a_head[1]);
        dummy.b[1] = atoi(q->a_head[2]);
        dummy.b[2] = atoi(q->a_head[3]);
        dummy.b[3] = atoi(q->a_head[4]);
        ia->s_addr = dummy.l;
        return(ia->s_addr != 0);
    }
    return(0);
}
#endif	/* NO_DCL_INET_ATON */

#endif /* MACOSX */
#endif /* SOLARIS7 */
#endif /* SOLARIS8 */
#endif /* SOLARIS9 */
#endif /* IRIX65 */
#endif /* UW7 */
#endif /* AIX41 */
#endif /* LINUX */
#endif /* NETBSD15 */
#endif /* FREEBSD4 */
#endif /* OpenBSD */
#endif /* SCO_OSR505 */
#endif /* HPUX1100 */
#endif /* HPUX10 */
#endif /* OSF50 */

int
ssl_check_server_name(SSL * ssl, char * hostname)
/* returns 0 if hostname and server's cert matches, else -1 */
{
    char * commonName;
    unsigned char ** dNSName;
    unsigned char ** ipAddress;
    struct in_addr ia;
    int rv;

    setverbosity();
    if (verbosity && !inserver) {
        if (dNSName = tls_get_SAN_objs(ssl,GEN_DNS)) {
            int i = 0;
            for (i = 0; dNSName[i]; i++) {
                printf("Certificate[0] altSubjectName DNS=%s\r\n",dNSName[i]);
                free(dNSName[i]);
            }
        }
        if (ipAddress = tls_get_SAN_objs(ssl,GEN_IPADD)) {
            int i = 0;
            char *server_ip;
            struct in_addr ia;

            for (i = 0; ipAddress[i]; i++) {
                if (ipAddress[i]) {
                    ia.s_addr = *(unsigned long *)ipAddress[i];
                    server_ip = inet_ntoa(ia);
                    printf("Certificate[0] altSubjectName IPAddr=%s\r\n",server_ip);
                }
                free(ipAddress[i]);
            }
            /* ipAddress points to a static - don't free */
        }
        if (dNSName = tls_get_SAN_objs(ssl,GEN_EMAIL)) {
            int i = 0;
            for (i = 0; dNSName[i]; i++) {
                printf("Certificate[0] altSubjectName Email=%s\r\n",dNSName[i]);
                free(dNSName[i]);
            }
        }
        if (dNSName = tls_get_SAN_objs(ssl,GEN_URI)) {
            int i = 0;
            for (i = 0; dNSName[i]; i++) {
                printf("Certificate[0] altSubjectName URI=%s\r\n",dNSName[i]);
                free(dNSName[i]);
            }
        }
        if (dNSName = tls_get_SAN_objs(ssl,GEN_OTHERNAME)) {
            int i = 0;
            for (i = 0; dNSName[i]; i++) {
                printf("Certificate[0] altSubjectName Other=%s\r\n",dNSName[i]);
                free(dNSName[i]);
            }
        }
    }

    /* first we check if `hostname' is in fact an ip address */
    if (inet_aton(hostname, &ia)) {
        ipAddress = tls_get_SAN_objs(ssl,GEN_IPADD);
        if (ipAddress) {
            int i = 0;
            char *server_ip = "UNKNOWN";

            for (i = 0; ipAddress[i]; i++)
                if (*(unsigned long *)ipAddress[i] == ia.s_addr)
                    return 0;

            if (ipAddress[i - 1]) {
                ia.s_addr = *(unsigned long *)ipAddress[i - 1];
                server_ip = inet_ntoa(ia);
            }
            rv = show_hostname_warning(hostname, server_ip) ? 0 : -1;
            for (i = 0; ipAddress[i]; i++)
                free(ipAddress[i]);
        } else {
            rv = show_hostname_warning(hostname, "NO IP IN CERT") ? 0 : -1;
        }
        return(rv);
    }

    /* look for dNSName(s) in subjectAltName in the server's certificate */
    dNSName = tls_get_SAN_objs(ssl,GEN_DNS);
    if (dNSName) {
        int i = 0;
        for (i = 0; dNSName[i]; i++) {
            if (!dNSName_cmp(hostname,(char *)dNSName[i]))
                return 0;
        }
        rv = show_hostname_warning(hostname,
				   (char *)((dNSName[i - 1] == NULL) ? 
			           (char *)"UNKNOWN" : (char *)dNSName[i - 1]))
	     ? 0 : -1;
        for (i = 0; dNSName[i]; i++)
            free(dNSName[i]);
        return rv;
    } else if ((commonName = ssl_get_commonName(ssl))) {
       /* so the server didn't have any dNSName's, check the commonName */
       if (!dNSName_cmp(hostname, commonName))
           return 0;
       else
           return (show_hostname_warning(hostname, commonName) ? 0 : -1);
    }
    return -1;
}

/* Is 'user' authorized to access the system without a login */
int
tls_is_user_valid(SSL * ssl, const char *user)
{
    X509 *client_cert;
    int r = 0;

    if ( !ssl || !user || !user[0] )
        return(0);

    if (!(client_cert = SSL_get_peer_certificate(ssl)))
        return 0;

    /* Use user supplied function */
    r = X509_userok(client_cert,user);

    X509_free(client_cert);
    return r;
}

int
tls_is_anon(int x)
{
    char buf[128];
    SSL_CIPHER * cipher;
    SSL * ssl = NULL;

    switch ( x ) {
#ifndef NOFTP
#ifndef SYSFTP
    case 1:     /* ftp command */
        if ( ssl_ftp_active_flag )
            ssl = ssl_ftp_con;
        else
            return(0);
        break;
    case 2:     /* ftp data */
        if ( ssl_ftp_data_active_flag )
            ssl = ssl_ftp_data_con;
        else
            return(0);
        break;
#endif /* SYSFTP */
#endif /* NOFTP */
    default:
        if (tls_active_flag)
            ssl = tls_con;
        else if (ssl_active_flag)
            ssl = ssl_con;
        else
            return(0);
    }

    cipher = SSL_get_current_cipher(ssl);
    if (SSL_CIPHER_description(cipher,buf,sizeof(buf))) {
        if (ckindex("Au=None",buf,0,0,0) != 0)
            return(1);                  /* anonymous */
        return(0);                  /* known */
    } else {
        /* could not get cipher description.  Assume anonymous */
        return(1);
    }
}

int
tls_is_krb5(int x)
{
    char buf[128];
    SSL_CIPHER * cipher;
    SSL * ssl = NULL;

    switch ( x ) {
#ifndef NOFTP
#ifndef SYSFTP
    case 1:     /* ftp command */
        if ( ssl_ftp_active_flag )
            ssl = ssl_ftp_con;
        else
            return(0);
        break;
    case 2:     /* ftp data */
        if ( ssl_ftp_data_active_flag )
            ssl = ssl_ftp_data_con;
        else
            return(0);
        break;
#endif /* SYSFTP */
#endif /* NOFTP */
#ifndef NOHTTP
    case 3:
        if ( tls_http_active_flag )
            ssl = tls_http_con;
        break;
#endif /* NOHTTP */
    default:
        if (tls_active_flag)
            ssl = tls_con;
        else if (ssl_active_flag)
            ssl = ssl_con;
        else
            return(0);
    }

    cipher = SSL_get_current_cipher(ssl);
    if (cipher && SSL_CIPHER_description(cipher,buf,sizeof(buf))) {
        if (ckindex("Au=KRB5",buf,0,0,0) != 0)
            return(1);                  /* krb5 */
    }
    return(0);                          /* not */
}

int
ssl_get_client_finished(char *buf, int count)
{
#ifdef NO_GET_FINISHED
    return(0);
#else
    if (sstelnet || tcp_incoming) {
        return(SSL_get_peer_finished(ssl_active_flag?ssl_con:tls_con,
                                      buf,count));
    } else {
        return(SSL_get_finished(ssl_active_flag?ssl_con:tls_con,
                                      buf,count));
    }
#endif /* NO_GET_FINISHED */
}

int
ssl_get_server_finished(char *buf, int count)
{
#ifdef NO_GET_FINISHED
    return(0);
#else
    if (sstelnet || tcp_incoming) {
        return(SSL_get_finished(ssl_active_flag?ssl_con:tls_con,
                                      buf,count));
    } else {
        return(SSL_get_peer_finished(ssl_active_flag?ssl_con:tls_con,
                                      buf,count));
    }
#endif /* NO_GET_FINISHED */
}


#ifdef CK_AUTHENTICATION
int
#ifdef CK_ANSIC
ssl_reply(int how, unsigned char *data, int cnt)
#else
ssl_reply(how,data,cnt) int how; unsigned char *data; int cnt;
#endif
{
    char * str=NULL;

    setverbosity();
    data += 4;                          /* Point to status byte */
    cnt  -= 4;

    if(cnt-- < 1) {
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;
    }

    switch(*data++) {
    case SSL_ACCEPT:
        if (tn_deb || debses)
            tn_debug("[SSL - handshake starting]");
        else if ( verbosity )
            printf("[SSL - handshake starting]\r\n");
        debug(F110,"ssl_reply","[SSL - handshake starting]",0);

        /* right ... now we drop into the SSL library */
        if (!ssl_only_flag) {
            if (ssl_dummy_flag) {
                if (tn_deb || debses)
                    tn_debug("[SSL - Dummy Connected]");
                else if ( verbosity ) {
                    printf("[SSL - Dummy Connected]\r\n");
                }
                debug(F110,"ssl_reply","[SSL - Dummy Connected]",0);
                auth_finished(AUTH_UNKNOWN);
                accept_complete = 1;
                return AUTH_SUCCESS;
            }

            if (SSL_connect(ssl_con) <= 0) {
                int len;
                if (tn_deb || debses) {
                    tn_debug("[SSL - FAILED]");
                    ERR_print_errors(bio_err);
                    len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                    ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                    printf(ssl_err);
                } else if ( verbosity ) {
                    printf("[SSL - FAILED]\r\n");
                    ERR_print_errors(bio_err);
                    len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                    ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                    printf(ssl_err);
                }
                debug(F110,"ssl_reply","[SSL - FAILED]",0);
                auth_finished(AUTH_REJECT);
                ttclos(0);
                return AUTH_FAILURE;
            } else {
                if (tn_deb || debses)
                    tn_debug("[SSL - OK]");
                else if ( verbosity ) {
                    printf("[SSL - OK]\r\n");
                }
                debug(F110,"ssl_reply","[SSL - OK]",0);

                ssl_active_flag = 1;
                ssl_display_connect_details(ssl_con,0,verbosity);
            }
        }
        auth_finished(AUTH_UNKNOWN);
        accept_complete = 1;
        break;

    case SSL_REJECT:
        if (tn_deb || debses) {
            tn_debug(
                 "[SSL - failed to switch on SSL - trying plaintext login]");
        } else if ( verbosity ) {
            printf("[SSL - failed to switch on SSL]\r\n");
            printf("Trying plaintext login:\r\n");
        }
        debug(F110,"ssl_reply","[SSL - failed to switch on SSL]",0);
        auth_finished(AUTH_REJECT);
        return AUTH_FAILURE;

    default:
        return AUTH_FAILURE;
    }
    return AUTH_SUCCESS;
}

int
#ifdef CK_ANSIC
ssl_is(unsigned char *data, int cnt)
#else
ssl_is(data,cnt) unsigned char *data; int cnt;
#endif
{
    if ((cnt -= 4) < 1)
        return AUTH_FAILURE;

    setverbosity();
    data += 4;
    switch(*data++) {
    case SSL_START:
        /* server starts the SSL stuff now ... */
        if (!ssl_only_flag) {
            if ( !tls_load_certs(ssl_ctx,ssl_con,1) ) {
                auth_finished(AUTH_REJECT);
                return AUTH_FAILURE;
            }

            if (tn_deb || debses)
                tn_debug("[SSL - handshake starting]");
            else if ( verbosity )
                printf("[SSL - handshake starting]\r\n");
            debug(F110,"ssl_is","[SSL - handshake starting]",0);

            SendSSLAuthSB(SSL_ACCEPT, (void *)0, 0);

            auth_ssl_valid = 1;

            if (ssl_dummy_flag) {
                if (tn_deb || debses)
                    tn_debug("[SSL - Dummy Connected]");
                else if ( verbosity ) {
                    printf("[SSL - Dummy Connected]\r\n");
                }
                debug(F110,"ssl_is","[SSL - Dummy Connected]",0);
                accept_complete = 1;
                auth_finished(AUTH_UNKNOWN);
                return AUTH_SUCCESS;
            }

            if (SSL_accept(ssl_con) <= 0) {
                char errbuf[1024];

                sprintf(errbuf,"[SSL - SSL_accept error: %s",
                         ERR_error_string(ERR_get_error(),NULL));

                if (tn_deb || debses)
                    tn_debug(errbuf);
                else if ( ssl_debug_flag )
                    printf("%s\r\n",errbuf);
                else if ( verbosity )
                    printf("[SSL - SSL_accept error]\r\n");

                debug(F110,"ssl_is",errbuf,0);

                auth_finished(AUTH_REJECT);
                ttclos(0);
                return AUTH_FAILURE;
            }

            if (tn_deb || debses)
                tn_debug("[SSL - OK]");
            else if ( verbosity ) {
                printf("[SSL - OK]\r\n");
            }
            debug(F110,"ssl_is","[SSL - OK]",0);
            ssl_active_flag = 1;
            ssl_display_connect_details(ssl_con,1,verbosity);

            /* now check to see that we got exactly what we
            * wanted from the caller ... if a certificate is
            * required then we make 100% sure that we were
            * given one during the handshake (as it is an optional
            * part of SSL)
            */

#ifdef SSL_KRB5
            if ( tls_is_krb5(0) ) {
                if (ssl_con->kssl_ctx->client_princ)
                    debug(F110,"ssl_is KRB5",ssl_con->kssl_ctx->client_princ,0);
            } else
#endif /* SSL_KRB5 */
            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                X509 * peer = SSL_get_peer_certificate(ssl_con);
                if (peer == NULL) {
                    if (tn_deb || debses)
                        tn_debug("[SSL - peer check failed]");
                    else if (ssl_debug_flag)
                        printf("[SSL - peer check failed]\r\n");
                    debug(F110,"ssl_is","[SSL - peer check failed]",0);

                    /* LOGGING REQUIRED HERE! */
                    auth_finished(AUTH_REJECT);
                    return AUTH_FAILURE;
                }
            }
            auth_finished(AUTH_UNKNOWN);
            accept_complete = 1;
        }
        break;

    default:
        SendSSLAuthSB(SSL_REJECT, (void *) "Unknown option received", -1);
        if (tn_deb || debses)
            tn_debug("[SSL - Unknown option received]");
        else
            printf("Unknown SSL option %d\r\n", data[-1]);
        debug(F111,"ssl_is","[SSL - Unknown option received]",data[-1]);
        auth_ssl_valid = 0;
        auth_finished(AUTH_REJECT);
        return(AUTH_FAILURE);
    }
    return AUTH_SUCCESS;
}

#endif /* CK_AUTHENTICATION */

int
ck_tn_tls_negotiate(VOID)
{
    X509 * peer = NULL;
    char str[256], *uid=NULL;
    extern int sstelnet;

    if ( !ck_ssleay_is_installed() )
        return(-1);

    setverbosity();
    if (sstelnet) {
        /* server starts the TLS stuff now ... */
        if (!tls_only_flag) {
            if ( !tls_load_certs(tls_ctx,tls_con,1) ) {
                auth_finished(AUTH_REJECT);
                return -1;
            }

            if (tn_deb || debses)
                tn_debug("[TLS - handshake starting]");
            else if ( verbosity )
                printf("[TLS - handshake starting]\r\n");
            debug(F110,"ck_tn_tls_negotiate","[TLS - handshake starting]",0);

            if (ssl_dummy_flag) {
                if (tn_deb || debses)
                    tn_debug("[TLS - Dummy Connected]");
                else if ( verbosity ) {
                    printf("[TLS - Dummy Connected]\r\n");
                }
                debug(F110,"ck_tn_tls_negotiate","[TLS - Dummy Connected]",0);
                accept_complete = 1;
                auth_finished(AUTH_REJECT);
                return 0;
            }

            if (SSL_accept(tls_con) <= 0) {
                char errbuf[1024];

                sprintf(errbuf,"[TLS - SSL_accept error: %s",
                         ERR_error_string(ERR_get_error(),NULL));

                if (tn_deb || debses)
                    tn_debug(errbuf);
                else if ( ssl_debug_flag )
                    printf("%s\r\n",errbuf);
                else if ( verbosity )
                    printf("[TLS - SSL_accept error]\r\n");

                debug(F110,"ck_tn_tls_negotiate",errbuf,0);
                auth_finished(AUTH_REJECT);
                return -1;
            }

            if (tn_deb || debses)
                tn_debug("[TLS - OK]");
            else if ( verbosity ) {
                printf("[TLS - OK]\r\n");
            }

            debug(F110,"ck_tn_tls_negotiate","[TLS - OK]",0);
            tls_active_flag = 1;
            ssl_display_connect_details(tls_con,1,verbosity);


#ifdef SSL_KRB5
            if ( tls_is_krb5(0) ) {
                if (tls_con->kssl_ctx->client_princ) {
                    char *p;
                    ckstrncpy(szUserNameAuthenticated,
                               tls_con->kssl_ctx->client_princ,
                               UIDBUFLEN);
                    ckstrncpy(szUserNameRequested,
                               tls_con->kssl_ctx->client_princ,
                               UIDBUFLEN);
                    for ( p = szUserNameRequested; *p ; p++ ) {
                        if ( *p == '@' || *p == '/' ) {
                            *p = '\0';
                            break;
                        }
                    }
                } else {
                    szUserNameRequested[0] = '\0';
                    szUserNameAuthenticated[0] = '\0';
                }
#ifdef CK_LOGIN
                if (zvuser(szUserNameRequested))
                    auth_finished(AUTH_VALID);
                else
#endif /* CK_LOGIN */
                    auth_finished(AUTH_USER);
            } else
#endif /* SSL_KRB5 */
            {
            /* now check to see that we got exactly what we
            * wanted from the caller ... if a certificate is
            * required then we make 100% sure that we were
            * given one during the handshake (as it is an optional
            * part of TLS)
            */
            peer=SSL_get_peer_certificate(tls_con);
            if (peer == NULL) {
                debug(F100,"SSL_get_peer_certificate() == NULL","",0);
                auth_finished(AUTH_REJECT);
                if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                    if (tn_deb || debses)
                        tn_debug("[TLS - peer check failed]");
                    else if (ssl_debug_flag) {
                        printf("[TLS - peer check failed]\r\n");
                    }
                    debug(F110,
                           "ck_tn_tls_negotiate",
                           "[TLS - peer check failed]",
                           0
                           );
                    /* LOGGING REQUIRED HERE! */
                    return -1;
                }
            } else {
                debug(F100,"SSL_get_peer_certificate() != NULL","",0);
                X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                                           NID_commonName,str,
                                           256
                                           );
                if ( verbosity )
                    printf("[TLS - commonName=%s]\r\n",str);

                X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
#ifndef NID_x500UniqueIdentifier
                                           NID_uniqueIdentifier,
#else
                                           NID_x500UniqueIdentifier,
#endif
                                           str,
                                           256
                                           );
                if ( verbosity )
                    printf("[TLS - uniqueIdentifier=%s]\r\n",str);

                /* Try to determine user name */
                uid = tls_userid_from_client_cert(tls_con);
                if ( uid ) {
                    /* This code is very questionable.
                     * How should it behave?
                     * The client has presented a certificate that
                     * contains a username.  We have validated the
                     * certificate but we do not automatically
                     * log the user in unless there is a .tlslogin
                     * file.
                     */

                    ckstrncpy(szUserNameRequested,uid,UIDBUFLEN);
#ifdef CK_LOGIN
                    if (zvuser(uid))
                        auth_finished(AUTH_VALID);
                    else
#endif /* CK_LOGIN */
                        auth_finished(AUTH_USER);
                }
                else {
                    szUserNameRequested[0] = '\0';
                    auth_finished(AUTH_REJECT);
                }
            }
            }
        }
    } else {
        char * str=NULL;

        if (tn_deb || debses)
            tn_debug("[TLS - handshake starting]");
        else if ( verbosity )
            printf("[TLS - handshake starting]\r\n");
        debug(F110,"ck_tn_tls_negotiate","[TLS - handshake starting]",0);

        /* right ... now we drop into the SSL library */
        if (!tls_only_flag) {
            char *subject=NULL, *issuer=NULL, *commonName=NULL, *dNSName=NULL;

            if (ssl_dummy_flag) {
                if (tn_deb || debses)
                    tn_debug("[TLS - Dummy Connected]");
                else if ( verbosity ) {
                    printf("[TLS - Dummy Connected]\r\n");
                }
                debug(F110,"ck_tn_tls_negotiate","[TLS - Dummy Connected]",0);
                auth_finished(AUTH_REJECT);
                accept_complete = 1;
                return 0;
            }

#ifndef USE_CERT_CB
            if (!tls_load_certs(tls_ctx,tls_con,0))
                return(-1);
#endif /* USE_CERT_CB */
            if (SSL_connect(tls_con) <= 0) {
                int len;
                if (tn_deb || debses) {
                    tn_debug("[TLS - FAILED]");
                    ERR_print_errors(bio_err);
                    len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                    ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                    printf(ssl_err);
                } else if ( verbosity ) {
                    printf("[TLS - FAILED]\r\n");
                    ERR_print_errors(bio_err);
                    len = BIO_read(bio_err,ssl_err,SSL_ERR_BFSZ);
                    ssl_err[len < SSL_ERR_BFSZ ? len : SSL_ERR_BFSZ] = '\0';
                    printf(ssl_err);
                }
                debug(F110,"ck_tn_tls_negotiate","[TLS - FAILED]",0);
                auth_finished(AUTH_REJECT);
                return -1;
            }

            tls_active_flag = 1;
            if ( !ssl_certsok_flag && (ssl_verify_flag & SSL_VERIFY_PEER)
                 && !tls_is_krb5(0)) {
                char prmpt[1024];
                subject = ssl_get_subject_name(tls_con);

                if (!subject) {
                    if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
                    {
                        if (tn_deb || debses)
                            tn_debug("[TLS - FAILED]");
                        else if ( verbosity )
                            printf("[TLS - FAILED]\r\n");
                        debug(F110,"ck_tn_tls_negotiate","[TLS - FAILED]",0);
                        auth_finished(AUTH_REJECT);
                        return -1;
                    } else {
                        int ok;
                        ok = uq_ok("Warning: Server didn't provide a certificate",
                                   "Continue? (Y/N)", 3, NULL, 0);
                        if (!ok) {
                            if (tn_deb || debses)
                                tn_debug("[TLS - FAILED]");
                            else if ( verbosity )
                                printf("[TLS - FAILED]\r\n");
                            debug(F110,
                                   "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                            auth_finished(AUTH_REJECT);
                            return -1;
                        }
                    }
                } else if (ssl_check_server_name(tls_con, szHostName)) {
                    if (tn_deb || debses)
                        tn_debug("[TLS - FAILED]");
                    else if ( verbosity )
                        printf("[TLS - FAILED]\r\n");
                    debug(F110,
                           "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                    auth_finished(AUTH_REJECT);
                    return -1;
                }
            }

            if ( ssl_debug_flag && ssl_finished_messages) {
                char msg[32];
                int i, len=32;
                extern char tn_msg[], hexbuf[];

                tn_msg[0] = '\0';
                len = ssl_get_client_finished(msg,len);
                if ( len > 0 ) {
                    for ( i=0;i<len;i++ ) {
                        sprintf(hexbuf,"%02X ",msg[i]);
                        ckstrncat(tn_msg,hexbuf,TN_MSG_LEN);
                    }
                    printf("TLS client finished: %s\r\n",tn_msg);
                }
                tn_msg[0] = '\0';
                len = ssl_get_server_finished(msg,len);
                if ( len > 0 ) {
                    for ( i=0;i<len;i++ ) {
                        sprintf(hexbuf,"%02X ",msg[i]);
                        ckstrncat(tn_msg,hexbuf,TN_MSG_LEN);
                    }
                    printf("TLS server finished: %s\r\n",tn_msg);
                }
            }

            if (tn_deb || debses)
                tn_debug("[TLS - OK]");
            else if ( verbosity )
                printf("[TLS - OK]\r\n");
            debug(F110,"ck_tn_tls_negotiate","[TLS - OK]",0);

            ssl_display_connect_details(tls_con,0,verbosity);
        }
        auth_finished(AUTH_REJECT);
    }
    accept_complete = 1;
    auth_ssl_valid = 1;
    return(0);
}

int
ck_ssl_incoming(fd) int fd;
{
    /* if we are not running in debug then any error
    * stuff from SSL debug *must* not go down
    * the socket (which 0,1,2 are all pointing to by
    * default)
    */

    int timo = 2000;

    setverbosity();
    if ( !ck_ssleay_is_installed() )
        return(-1);

    /* do the SSL stuff now ... before we play with pty's */
    SSL_set_fd(ssl_con,fd);
    SSL_set_fd(tls_con,fd);

    if (tls_only_flag) {
        if (tn_deb || debses)
            tn_debug("[TLS - handshake starting]");
        else if ( verbosity )
            printf("[TLS - handshake starting]\r\n");
        debug(F110,"ck_ssl_incoming","[TLS - handshake starting]",0);

        /* hmm ... only when running talking to things like
        * https servers should we hit this code and then
        * we really don't care *who* we talk to :-)
        */
        if (SSL_accept(tls_con) <= 0) {
            char errbuf[1024];

            sprintf(errbuf,"[TLS - SSL_accept error: %s",
                     ERR_error_string(ERR_get_error(),NULL));

            if (tn_deb || debses)
                tn_debug(errbuf);
            else if ( ssl_debug_flag )
                printf("%s\r\n",errbuf);
            else if ( verbosity )
                printf("[TLS - SSL_accept error]\r\n");

            debug(F110,"ck_ssl_incoming",errbuf,0);
            return(-1);
        } else {
            if (tn_deb || debses)
                tn_debug("[TLS - OK]");
            else if ( verbosity )
                printf("[TLS - OK]\r\n");
            debug(F110,"ck_ssl_incoming","[TLS - OK]",0);
            tls_active_flag = 1;
        }
    } else if (ssl_only_flag) {
        if (tn_deb || debses)
            tn_debug("[SSL - handshake starting]");
        else if ( verbosity )
            printf("[SSL - handshake starting]\r\n");
        debug(F110,"ck_ssl_incoming","[SSL - handshake starting]",0);

        /* hmm ... only when running talking to things like
         * https servers should we hit this code and then
         * we really don't care *who* we talk to :-)
         */
        if (SSL_accept(ssl_con) <= 0) {
            char errbuf[1024];

            sprintf(errbuf,"[SSL - SSL_accept error: %s",
                     ERR_error_string(ERR_get_error(),NULL));

            if (tn_deb || debses)
                tn_debug(errbuf);
            else if ( ssl_debug_flag )
                printf("%s\r\n",errbuf);
            else if ( verbosity )
                printf("[SSL - SSL_accept error]\r\n");

            debug(F110,"ck_ssl_incoming",errbuf,0);
            return(-1);
        } else {
            if (tn_deb || debses)
                tn_debug("[SSL - OK]");
            else if ( verbosity )
            printf("[SSL - OK]\r\n");
            debug(F110,"ssl_is","[SSL - OK]",0);
            ssl_active_flag = 1;
        }
    }
    if (ssl_active_flag || tls_active_flag) {
        X509 *peer;
        char str[256], *uid=NULL;

        /* now check to see that we got exactly what we
         * wanted from the caller ... if a certificate is
         * required then we make 100% sure that we were
         * given on during the handshake (as it is an optional
         * part of SSL and TLS)
         */

        if ( tls_active_flag ) {
            peer=SSL_get_peer_certificate(tls_con);
        } else if ( ssl_active_flag ) {
            peer=SSL_get_peer_certificate(ssl_con);
        }

        if (peer == NULL) {
            debug(F100,"SSL_get_peer_certificate() == NULL","",0);
            auth_finished(AUTH_REJECT);

            if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
                if (tn_deb || debses)
                    tn_debug("[SSL/TLS - peer check failed]");
                else if (ssl_debug_flag) {
                    printf("[SSL/TLS - peer check failed]\r\n");
                }
                debug(F110,
                       "ck_tn_tls_negotiate",
                       "[SSL/TLS - peer check failed]",
                       0
                       );
                /* LOGGING REQUIRED HERE! */
                return -1;
            }

        } else {
            debug(F100,"SSL_get_peer_certificate() != NULL","",0);
            X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                                       NID_commonName,str,
                                       256
                                       );
            printf("[TLS - commonName=%s]\r\n",str);

            X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
#ifndef NID_x500UniqueIdentifier
                                       NID_uniqueIdentifier,
#else   
                                       NID_x500UniqueIdentifier,
#endif
                                       str,256
                                       );
            printf("[TLS - uniqueIdentifier=%s]\r\n",str);

            /* Try to determine user name */
            uid = tls_userid_from_client_cert(tls_con);
            if ( uid ) {
                /* This code is very questionable.
                * How should it behave?
                * The client has presented a certificate that
                * contains a username.  We have validated the
                * certificate but we do not automatically
                * log the user in unless there is a .tlslogin
                * file.
                */

                ckstrncpy(szUserNameRequested,uid,UIDBUFLEN);
#ifdef CK_LOGIN
                if (zvuser(uid))
                    auth_finished(AUTH_VALID);
                else
#endif /* CK_LOGIN */
                    auth_finished(AUTH_USER);
            }
            else {
                szUserNameRequested[0] = '\0';
                auth_finished(AUTH_REJECT);
            }
        }
    }
    return(0);  /* success */
}

int
ck_ssl_outgoing(fd) int fd;
{
    int timo = 2000;

    setverbosity();
    if ( !ck_ssleay_is_installed() )
        return(-1);

    /* bind in the network descriptor */
    SSL_set_fd(ssl_con,fd);
    SSL_set_fd(tls_con,fd);

    /* If we are doing raw TLS then start it now ... */
    if (tls_only_flag) {
#ifndef USE_CERT_CB
        if (!tls_load_certs(tls_ctx,tls_con,0)) {
            debug(F110,"ck_ssl_outgoing","tls_load_certs() failed",0);
            return(-1);
        }
#endif /* USE_CERT_CB */
        if (tn_deb || debses)
            tn_debug("[TLS - handshake starting]");
        else if (verbosity)
            printf("[TLS - handshake starting]\r\n");
        debug(F110,"ck_ssl_outgoing","[TLS - handshake starting]",0);
        if (SSL_connect(tls_con) <= 0) {
            char errbuf[1024];

            sprintf(errbuf,"[TLS - SSL_connect error: %s",
                     ERR_error_string(ERR_get_error(),NULL));

            if (tn_deb || debses)
                tn_debug(errbuf);
            else if ( ssl_debug_flag )
                printf("%s\r\n",errbuf);

            if (tn_deb || debses)
                tn_debug("[TLS - FAILED]");
            else if ( verbosity )
                printf("[TLS - FAILED]\r\n");
            debug(F110,"ck_ssl_outgoing","[TLS - FAILED]",0);
            netclos();
            return(-1);
        } else {
            tls_active_flag = 1;
            if ( !ssl_certsok_flag && (ssl_verify_flag & SSL_VERIFY_PEER) && 
                 !tls_is_krb5(0) ) {
                char *subject = ssl_get_subject_name(tls_con);

                if (!subject) {
                    if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
                    {
                        if (tn_deb || debses)
                            tn_debug("[TLS - FAILED]");
                        else if ( verbosity )
                            printf("[TLS - FAILED]\r\n");
                        debug(F110,"ck_tn_tls_negotiate","[TLS - FAILED]",0);

                        auth_finished(AUTH_REJECT);
                        return -1;
                    } else {
                        char prmpt[1024];
                        int ok;
                        ok = uq_ok("Warning: Server didn't provide a certificate",
                                   "Continue? (Y/N)", 3, NULL, 0);
                        if (!ok) {
                            if (tn_deb || debses)
                                tn_debug("[TLS - FAILED]");
                            else if ( verbosity )
                                printf("[TLS - FAILED]\r\n");
                            debug(F110,
                                   "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                            auth_finished(AUTH_REJECT);
                            return -1;
                        }
                    }
                } else if (ssl_check_server_name(tls_con, szHostName)) {
                    if (tn_deb || debses)
                        tn_debug("[TLS - FAILED]");
                    else if ( verbosity )
                        printf("[TLS - FAILED]\r\n");
                    debug(F110,
                           "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                    auth_finished(AUTH_REJECT);
                    return -1;
                }
            }
            if (tn_deb || debses)
                tn_debug("[TLS - OK]");
            else if (!quiet)
		printf("[TLS - OK]\r\n");
            debug(F110,"ck_ssl_outgoing","[TLS - OK]",0);
            ssl_display_connect_details(tls_con,0,verbosity);
        }
    }
    /* if we are doing raw SSL then start it now ... */
    else if (ssl_only_flag) {
#ifndef USE_CERT_CB
        if (!tls_load_certs(ssl_ctx,ssl_con,0))
            return(-1);
#endif /* USE_CERT_CB */
        if (tn_deb || debses)
            tn_debug("[SSL - handshake starting]");
        else if ( verbosity )
            printf("[SSL - handshake starting]\r\n");
        debug(F110,"ck_ssl_outgoing","[SSL - handshake starting]",0);
        if (SSL_connect(ssl_con) <= 0) {
            if ( ssl_debug_flag ) {
                char errbuf[1024];

                sprintf(errbuf,"[SSL - SSL_connect error: %s",
                         ERR_error_string(ERR_get_error(),NULL));
                printf("%s\r\n",errbuf);
            }
            if (tn_deb || debses)
                tn_debug("[SSL - FAILED]");
            else if ( verbosity )
                printf("[SSL - FAILED]\r\n");
            debug(F110,"ck_ssl_outgoing","[SSL - FAILED]",0);
            return(-1);
        } else {
            ssl_active_flag = 1;

            if ( !ssl_certsok_flag && (ssl_verify_flag & SSL_VERIFY_PEER) &&
                 !tls_is_krb5(0)) {
                char *subject = ssl_get_subject_name(ssl_con);

                if (!subject) {
                    if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
                    {
                        if (tn_deb || debses)
                            tn_debug("[SSL - FAILED]");
                        else if ( verbosity )
                            printf("[SSL - FAILED]\r\n");
                        debug(F110,"ck_tn_tls_negotiate","[SSL - FAILED]",0);

                        auth_finished(AUTH_REJECT);
                        return -1;
                    } else {
                        char prmpt[1024];
                        int ok;
                        ok = uq_ok("Warning: Server didn't provide a certificate",
                                   "Continue? (Y/N)", 3, NULL, 0);
                        if (!ok) {
                            if (tn_deb || debses)
                                tn_debug("[SSL - FAILED]");
                            else if ( verbosity )
                                printf("[SSL - FAILED]\r\n");
                            debug(F110,
                                   "ck_tn_tls_negotiate","[SSL - FAILED]",0);
                            auth_finished(AUTH_REJECT);
                            return -1;
                        }
                    }
                } else if (ssl_check_server_name(ssl_con, szHostName)) {
                    if (tn_deb || debses)
                        tn_debug("[SSL - FAILED]");
                    else if ( verbosity )
                        printf("[SSL - FAILED]\r\n");
                    debug(F110, "ck_tn_tls_negotiate","[SSL - FAILED]",0);
                    auth_finished(AUTH_REJECT);
                    return -1;
                }
            }
            if (tn_deb || debses)
                tn_debug("[SSL - OK]");
	    else if (!quiet)
	        printf("[SSL - OK]\r\n");
            debug(F110,"ck_ssl_outgoing","[SSL - OK]",0);
            ssl_display_connect_details(ssl_con,0,verbosity);
        }
    }
    return(0);  /* success */
}

#ifndef NOHTTP
int
ck_ssl_http_client(fd, hostname) int fd; char * hostname;
{
    int timo = 2000;

    if ( !ck_ssleay_is_installed() )
        return(-1);

    setverbosity();

    /* bind in the network descriptor */
    SSL_set_fd(tls_http_con,fd);

    /* If we are doing raw TLS then start it now ... */
    if (1) {
#ifndef USE_CERT_CB
        if (!tls_load_certs(tls_http_ctx,tls_http_con,0)) {
            debug(F110,"ck_ssl_http_client","tls_load_certs() failed",0);
            return(-1);
        }
#endif /* USE_CERT_CB */
        if (tn_deb || debses)
            tn_debug("[TLS - handshake starting]");
        else if (verbosity)
            printf("[TLS - handshake starting]\r\n");
        debug(F110,"ck_ssl_outgoing","[TLS - handshake starting]",0);
        if (SSL_connect(tls_http_con) <= 0) {
            char errbuf[1024];

            sprintf(errbuf,"[TLS - SSL_connect error: %s",
                     ERR_error_string(ERR_get_error(),NULL));

            if (tn_deb || debses)
                tn_debug(errbuf);
            else if ( ssl_debug_flag )
                printf("%s\r\n",errbuf);

            if (tn_deb || debses)
                tn_debug("[TLS - FAILED]");
            else if ( verbosity )
                printf("[TLS - FAILED]\r\n");
            debug(F110,"ck_ssl_http_client","[TLS - FAILED]",0);
            http_close();
            return(-1);
        } else {
            tls_http_active_flag = 1;
            if ( !ssl_certsok_flag && (ssl_verify_flag & SSL_VERIFY_PEER) &&
                 !tls_is_krb5(3) ) {
                char *subject = ssl_get_subject_name(tls_http_con);

                if (!subject) {
                    if (ssl_verify_flag & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
                    {
                        if (tn_deb || debses)
                            tn_debug("[TLS - FAILED]");
                        else if ( verbosity )
                            printf("[TLS - FAILED]\r\n");
                        debug(F110,"ck_tn_tls_negotiate","[TLS - FAILED]",0);
                        return -1;
                    } else {
                        char prmpt[1024];
                        int ok;
                        ok = uq_ok("Warning: Server didn't provide a certificate",
                                   "Continue? (Y/N)", 3, NULL, 0);
                        if (!ok) {
                            if (tn_deb || debses)
                                tn_debug("[TLS - FAILED]");
                            else if ( verbosity )
                                printf("[TLS - FAILED]\r\n");
                            debug(F110,
                                   "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                            return -1;
                        }
                    }
                } else if (ssl_check_server_name(tls_http_con, hostname)) {
                    if (tn_deb || debses)
                        tn_debug("[TLS - FAILED]");
                    else if ( verbosity )
                        printf("[TLS - FAILED]\r\n");
                    debug(F110,
                           "ck_tn_tls_negotiate","[TLS - FAILED]",0);
                    return -1;
                }
            }

            printf("[TLS - OK]\r\n");
            if (tn_deb || debses)
                tn_debug("[TLS - OK]");
            debug(F110,"ck_ssl_outgoing","[TLS - OK]",0);
            ssl_display_connect_details(tls_http_con,0,verbosity);
        }
    }
    return(0);  /* success */
}
#endif /* NOHTTP */
int
ck_ssl_renegotiate_ciphers()
{

    if ( !ck_ssleay_is_installed() )
        return(0);

    if ( !sstelnet )
        return(0);

    if ( ssl_active_flag )
        return SSL_renegotiate(ssl_con);
    else if ( tls_active_flag )
        return SSL_renegotiate(tls_con);
    return(0);
}

#ifdef NT
int 
ck_X509_save_cert_to_user_store(X509 *cert)
{
#ifdef X509V3_EXT_DUMP_UNKNOWN
    char path[CKMAXPATH];
    char hash[16];
    char * GetAppData(int);
    BIO * out=NULL;

    if ( cert == NULL )
        return(0);

    sprintf(hash,"%08lx",X509_subject_name_hash(cert));
    ckmakmsg(path,CKMAXPATH,GetAppData(0),"kermit 95/certs/",
             hash,".0");

    
    out=BIO_new(BIO_s_file());
    if (out == NULL)
    {
        ERR_print_errors(bio_err);
        return(0);
    }
    if (BIO_write_filename(out,path) <= 0) {
        perror(path);
        return(0);
    }

    X509_print_ex(out, cert, XN_FLAG_SEP_MULTILINE, X509V3_EXT_DUMP_UNKNOWN);
    if (!PEM_write_bio_X509(out,cert)) {
        BIO_printf(bio_err,"unable to write certificate\n");
        ERR_print_errors(bio_err);
        BIO_free_all(out);
        return(0);
    }
    BIO_free_all(out);
    return(1);
#else /* X509V3_EXT_DUMP_UNKNOWN */
    return(0);
#endif /* X509V3_EXT_DUMP_UNKNOWN */
}
#endif /* NT */

#ifndef OS2
/* The following function should be replaced by institution specific */
/* code that will convert an X509 cert structure to a userid for the */
/* purposes of client to host login.  The example code included      */
/* simply returns the UID field of the Subject if it exists.         */

/* X509_to_user() returns 0 if valid userid in 'userid', else -1 */
int
X509_to_user(X509 *peer_cert, char *userid, int len)
{
#ifdef X509_UID_TO_USER
    /* BEGIN EXAMPLE */
    int err;

    if (!(peer_cert && userid) || len <= 0)
        return -1;

    userid[0] = '\0';
    debug(F110,"X509_to_user() subject",
           X509_NAME_oneline(X509_get_subject_name(peer_cert),NULL,0),0);

    /* userid is in cert subject /UID */
    err = X509_NAME_get_text_by_NID(X509_get_subject_name(peer_cert),
#ifndef NID_x500UniqueIdentifier
                                     NID_uniqueIdentifier,
#else
                                     NID_x500UniqueIdentifier,
#endif
                                     userid, len);

    debug(F111,"X509_to_user() userid",userid,err);
    if (err > 0)
        return 0;

    /* END EXAMPLE */
#else /* X509_UID_TO_USER */
#ifdef X509_SUBJECT_ALT_NAME_TO_USER
    /* BEGIN EXAMPLE */
    int i;
    X509_EXTENSION *ext = NULL;
    STACK_OF(GENERAL_NAME) *ialt = NULL;
    GENERAL_NAME *gen = NULL;
    char email[256];

    if (!(peer_cert && userid) || len <= 0)
        return -1;

    userid[0] = '\0';
    email[0] = '\0';
    debug(F110,"X509_to_user() subject",
           X509_NAME_oneline(X509_get_subject_name(peer_cert),NULL,0),0);

    if ((i = X509_get_ext_by_NID(peer_cert, NID_subject_alt_name, -1))<0)
        return -1;
    if (!(ext = X509_get_ext(peer_cert, i)))
        return -1;
    X509V3_add_standard_extensions();
    if (!(ialt = X509V3_EXT_d2i(ext)))
        return -1;
    for (i = 0; i < sk_GENERAL_NAME_num(ialt); i++) {
        gen = sk_GENERAL_NAME_value(ialt, i);
        if (gen->type == GEN_DNS) {
            if (!gen->d.ia5 || !gen->d.ia5->length)
	      break;
            if (strlen(gen->d.ia5->data) != gen->d.ia5->length) {
                /* Ignoring IA5String containing null character */
                continue;
            }
            if ( gen->d.ia5->length + 1 > sizeof(email) ) {
                goto cleanup;
            }
            memcpy(email, gen->d.ia5->data, gen->d.ia5->length);
            email[gen->d.ia5->length] = 0;
            break;
        }
    }
  cleanup:
    X509V3_EXT_cleanup();
    if (ialt)
        sk_GENERAL_NAME_free(ialt);

    debug(F110,"X509_to_user() email",email,0);

    if ( email[0] ) {
        char * domain = NULL;

        /* Find domain */
        for ( i=0 ; email[i] ; i++ ) {
            if ( email[i] == '@' ) {
                email[i] = '\0';
                domain = &email[i+1];
                break;
            }
        }

        if ( domain ) {
            /* XXX - Put code to Verify domain here */

            if ( /* domain is okay */ 1 )
                ckstrncpy(userid,email,len);
        }
    }

    return(userid[0] ? 0 : -1);
    /* END EXAMPLE */
#endif /* X509_SUBJECT_ALT_NAME_TO_USER */
#endif /* X509_UID_TO_USER */
    return -1;
}

/* The following function should be replaced by institution specific */
/* code that will determine whether or not the combination of the    */
/* provided X509 certificate and username is valid for automatic     */
/* login.   Whereas X509_to_user() is used to provide authentication */
/* of the user, the X509_userok() function is used to provide        */
/* authorization.  The certificate passed into X509_userok() does    */
/* need to map to a userid; nor would the userid it would map to     */
/* need to match the userid provided to the function.  There are     */
/* numerous circumstances in which it is beneficial to have the ability */
/* for multiple users to gain access to a common account such as     */
/* 'root' on Unix; or a class account on a web server.  In Unix we   */
/* implement this capability with the ~userid/.tlslogin file which   */
/* a list of X509 certificates which may be used to access the       */
/* account 'userid'.                                                 */

/* X509_to_user() returns 0 if access is denied; 1 is access is permitted */
int
X509_userok(X509 * peer_cert, const char * userid)
{
#ifndef VMS
    /* check if clients cert is in "user"'s ~/.tlslogin file */
    char buf[512];
    int r = 0;
    FILE *fp;
    struct passwd *pwd;
    X509 *file_cert;

    if ( peer_cert == NULL )
        return(0);

    if (!(pwd = getpwnam(userid)))
       return 0;
    if (strlen(pwd->pw_dir) > 500)
       return(0);
    sprintf(buf, "%s/.tlslogin", pwd->pw_dir);

    if (!(fp = fopen(buf, "r")))
        return 0;
    while (!r && (file_cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
        if (!ASN1_STRING_cmp(peer_cert->signature, file_cert->signature))
            r = 1;
        X509_free(file_cert);
    }
    fclose(fp);
    return(r);
#else /* VMS */
    /* Need to implement an appropriate function for VMS */
    return(0);
#endif /* VMS */
}
#endif /* OS2 */
#endif /* CK_SSL */
