#ifdef CK_SSL
#ifndef CK_ANSIC
#define NOPROTO
#endif /* CK_ANSIC */
#include "bio.h"
#include "buffer.h"
#include "x509.h"
#include "pem.h"
#include "ssl.h"

extern BIO *bio_err;
extern SSL *ssl_con;
extern SSL_CTX *ssl_ctx;
extern int ssl_debug_flag;
extern int ssl_only_flag;
extern int ssl_active_flag;
extern int ssl_verify_flag;
extern int ssl_secure_flag;
extern int ssl_verbose_flag;
extern int ssl_disabled_flag;
extern int ssl_cert_required;
extern int ssl_certsok_flag;
extern int ssl_dummy_flag;

extern char *ssl_log_file;
extern char *ssl_rsa_cert_file;
extern char *ssl_rsa_key_file;
extern char *ssl_dsa_cert_file;
extern char *ssl_dh_key_file;
extern char *ssl_cipher_list;

extern SSL_CTX *tls_ctx;
extern SSL *tls_con;
extern int tls_only_flag;
extern int tls_active_flag;
extern int tls_secure_flag;

_PROTOTYP(int ssl_do_init,(int));
_PROTOTYP(int ssl_display_connect_details,(SSL *,int));
_PROTOTYP(int ssl_server_verify_callback,(int, X509_STORE_CTX *));
_PROTOTYP(int ssl_client_verify_callback,(int, X509_STORE_CTX *));

#ifdef OS2
#define SSL_get_error                    ck_SSL_get_error
#define SSL_read                         ck_SSL_read
#define SSL_peek                         ck_SSL_peek
#define SSL_connect                      ck_SSL_connect
#define SSL_set_fd                       ck_SSL_set_fd
#define SSL_free                         ck_SSL_free
#define SSL_shutdown                     ck_SSL_shutdown
#define SSL_write                        ck_SSL_write
#define SSL_pending                      ck_SSL_pending
#define SSL_load_error_strings           ck_SSL_load_error_strings
#define SSL_get_peer_certificate         ck_SSL_get_peer_certificate
#define SSL_CIPHER_get_name              ck_SSL_CIPHER_get_name
#define SSL_get_current_cipher           ck_SSL_get_current_cipher
#define SSL_get_shared_ciphers           ck_SSL_get_shared_ciphers
#define SSL_get_ciphers                  ck_SSL_get_ciphers
#define SSL_get_cipher_list              ck_SSL_get_cipher_list
#define SSL_CTX_set_default_verify_paths ck_SSL_CTX_set_default_verify_paths
#define SSL_use_RSAPrivateKey_file       ck_SSL_use_RSAPrivateKey_file
#define SSL_use_DSAPrivateKey_file       ck_SSL_use_DSAPrivateKey_file
#define SSL_use_PrivateKey_file          ck_SSL_use_PrivateKey_file
#define SSL_use_certificate_file         ck_SSL_use_certificate_file
#define SSL_CTX_use_PrivateKey_file      ck_SSL_CTX_use_PrivateKey_file
#define SSL_CTX_use_certificate_file     ck_SSL_CTX_use_certificate_file
#define SSL_set_verify                   ck_SSL_set_verify
#define SSL_new                          ck_SSL_new
#define SSL_CTX_ctrl                     ck_SSL_CTX_ctrl
#define SSL_CTX_new                      ck_SSL_CTX_new
#define SSL_CTX_free                     ck_SSL_CTX_free
#define SSL_CTX_set_default_passwd_cb    ck_SSL_CTX_set_default_passwd_cb
#define SSLv23_method                    ck_SSLv23_method
#define SSLv3_method                     ck_SSLv3_method
#define TLSv1_method                     ck_TLSv1_method
#define SSLv23_client_method             ck_SSLv23_client_method
#define SSLv3_client_method              ck_SSLv3_client_method
#define TLSv1_client_method              ck_TLSv1_client_method
#define SSLv23_server_method             ck_SSLv23_server_method
#define SSLv3_server_method              ck_SSLv3_server_method
#define TLSv1_server_method              ck_TLSv1_server_method
#define SSL_library_init                 ck_SSL_library_init
#define SSL_state_string                 ck_SSL_state_string
#define SSL_state_string_long            ck_SSL_state_string_long
#define SSL_accept                       ck_SSL_accept
#define SSL_set_cipher_list              ck_SSL_set_cipher_list

#define ERR_print_errors                 ck_ERR_print_errors
#define ERR_print_errors_fp              ck_ERR_print_errors_fp
#define ERR_error_string                 ck_ERR_error_string
#define ERR_get_error                    ck_ERR_get_error

#define BIO_printf                       ck_BIO_printf
#define BIO_ctrl                         ck_BIO_ctrl
#define BIO_new                          ck_BIO_new
#define BIO_s_file                       ck_BIO_s_file
#define BIO_s_mem                        ck_BIO_s_mem
#define BIO_s_null                       ck_BIO_s_null
#define BIO_read                         ck_BIO_read
#define BIO_new_file                     ck_BIO_new_file
#define BIO_free                         ck_BIO_free

#define X509_get_issuer_name             ck_X509_get_issuer_name
#define X509_verify_cert_error_string    ck_X509_verify_cert_error_string
#define X509_NAME_oneline                ck_X509_NAME_oneline
#define X509_get_subject_name            ck_X509_get_subject_name
#define X509_STORE_CTX_get_current_cert  ck_X509_STORE_CTX_get_current_cert
#define X509_get_default_cert_dir        ck_X509_get_default_cert_dir
#define X509_free                        ck_X509_free

#define RSA_free                         ck_RSA_free
#define RSA_generate_key                 ck_RSA_generate_key

#define DH_new                           ck_DH_new
#define DH_free                          ck_DH_free
#define DH_generate_key                  ck_DH_generate_key
#define DH_generate_parameters           ck_DH_generate_parameters

#define DSA_free                         ck_DSA_free
#define DSA_generate_key                 ck_DSA_generate_key
#define DSA_generate_parameters          ck_DSA_generate_parameters

#define PEM_read_bio_DHparams            ck_PEM_read_bio_DHparams
#define BN_bin2bn                        ck_BN_bin2bn
#endif /* OS2 */
#endif /* CK_SSL */
