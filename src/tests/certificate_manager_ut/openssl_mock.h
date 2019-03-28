// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

MOCKABLE_FUNCTION(, EVP_PKEY*, PEM_read_PrivateKey, FILE*, fp, EVP_PKEY**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, X509*, PEM_read_X509, FILE*, fp, X509**, x, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, void, X509_free, X509*, a);
MOCKABLE_FUNCTION(, void, EVP_PKEY_free, EVP_PKEY*, key);
MOCKABLE_FUNCTION(, int, PEM_write_bio_PrivateKey, BIO*, bp, EVP_PKEY*, x, const EVP_CIPHER*, enc, unsigned char*, kstr, int, klen, pem_password_cb*, cb, void*, u);
MOCKABLE_FUNCTION(, int, PEM_write_bio_X509, BIO*, bp, X509*, x);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
MOCKABLE_FUNCTION(, BIO_METHOD*, BIO_s_mem);
MOCKABLE_FUNCTION(, BIO*, BIO_new, BIO_METHOD*, type);
#else
MOCKABLE_FUNCTION(, const BIO_METHOD*, BIO_s_mem);
MOCKABLE_FUNCTION(, BIO*, BIO_new, const BIO_METHOD*, type);
#endif

MOCKABLE_FUNCTION(, int, BIO_read, BIO*, b, void*, buf, int, len);
MOCKABLE_FUNCTION(, int, BIO_free, BIO*, a);
MOCKABLE_FUNCTION(, int, PKCS12_parse, PKCS12*, p12, const char*, pass, EVP_PKEY**, pkey, X509**, cert, STACK_OF(X509)**, ca);
MOCKABLE_FUNCTION(, PKCS12*, d2i_PKCS12_fp, FILE*, fp, PKCS12**, p12);
MOCKABLE_FUNCTION(, void, PKCS12_free, PKCS12*, a);
MOCKABLE_FUNCTION(, long, BIO_ctrl, BIO*, bp, int, cmd, long, larg, void*, parg);
