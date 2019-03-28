// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "openssl_mock.h"
#include "std_mocks.h"
#include "utils.h"
#undef ENABLE_MOCKS

#include "certificate_manager.h"

static FILE* MOCKED_FILE = (FILE*)0x59;
static X509* MOCKED_X509 = (X509*)0x58;
static BIO* MOCKED_BIO = (BIO*)0x60;
static EVP_PKEY* MOCKED_EVP = (EVP_PKEY*)0x56;
static PKCS12* MOCKED_PKCS12 = (PKCS12*)0x55;
static size_t MOCKED_BUFFER_LENGTH = 10;

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

PKCS12* Mocked_d2i_PKCS12_fp(FILE* fp, PKCS12** p12) {
    *p12 = MOCKED_PKCS12;
    return *p12;
}

int Mocked_PKCS12_parse(PKCS12 *p12, const char *pass, EVP_PKEY **pkey, X509 **cert, STACK_OF(X509) **ca) {
    *pkey = MOCKED_EVP;
    *cert = MOCKED_X509;
    return 1;
}

BEGIN_TEST_SUITE(certificate_manager_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);
    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();
    REGISTER_GLOBAL_MOCK_HOOK(d2i_PKCS12_fp, Mocked_d2i_PKCS12_fp);
    REGISTER_GLOBAL_MOCK_HOOK(PKCS12_parse, Mocked_PKCS12_parse);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    REGISTER_GLOBAL_MOCK_HOOK(d2i_PKCS12_fp, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(PKCS12_parse, NULL);
}


TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(CertificateManager_LoadPemFromFile_expectSuccess)
{
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, ".pem", false)).SetReturn(true);
    STRICT_EXPECTED_CALL(fopen("file.pem", "rb")).SetReturn(MOCKED_FILE);
    
    STRICT_EXPECTED_CALL(PEM_read_X509(IGNORED_PTR_ARG, NULL, NULL, NULL)).SetReturn(MOCKED_X509);
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG)).SetReturn(MOCKED_BIO);
    STRICT_EXPECTED_CALL(PEM_write_bio_X509(MOCKED_BIO, MOCKED_X509)).SetReturn(1);
    STRICT_EXPECTED_CALL(BIO_ctrl(MOCKED_BIO, BIO_CTRL_INFO, IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(MOCKED_BUFFER_LENGTH);
    STRICT_EXPECTED_CALL(BIO_read(MOCKED_BIO, IGNORED_PTR_ARG, MOCKED_BUFFER_LENGTH));
    STRICT_EXPECTED_CALL(BIO_free(MOCKED_BIO));

    STRICT_EXPECTED_CALL(PEM_read_PrivateKey(IGNORED_PTR_ARG, NULL, NULL, NULL)).SetReturn(MOCKED_EVP);
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG)).SetReturn(MOCKED_BIO);
    STRICT_EXPECTED_CALL(PEM_write_bio_PrivateKey(MOCKED_BIO, MOCKED_EVP, 0, NULL, 0, NULL, NULL)).SetReturn(1);
    STRICT_EXPECTED_CALL(BIO_ctrl(MOCKED_BIO, BIO_CTRL_INFO, IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(MOCKED_BUFFER_LENGTH);
    STRICT_EXPECTED_CALL(BIO_read(MOCKED_BIO, IGNORED_PTR_ARG, MOCKED_BUFFER_LENGTH));
    STRICT_EXPECTED_CALL(BIO_free(MOCKED_BIO));

    STRICT_EXPECTED_CALL(fclose(MOCKED_FILE));
    STRICT_EXPECTED_CALL(X509_free(MOCKED_X509));
    STRICT_EXPECTED_CALL(EVP_PKEY_free(MOCKED_EVP));

    char* certificate = NULL;
    char* privateKey = NULL;
    bool result = CertificateManager_LoadFromFile("file.pem", &certificate, &privateKey);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    free(certificate);
    free(privateKey);
}

TEST_FUNCTION(CertificateManager_LoadPKCS12FromFile_expectSuccess)
{
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, ".pem", false)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, ".pfx", false)).SetReturn(true);
    
    STRICT_EXPECTED_CALL(fopen("file.pfx", "rb")).SetReturn(MOCKED_FILE);
    STRICT_EXPECTED_CALL(d2i_PKCS12_fp(MOCKED_FILE, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(PKCS12_parse(IGNORED_PTR_ARG, NULL, IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL));
    
    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG)).SetReturn(MOCKED_BIO);
    STRICT_EXPECTED_CALL(PEM_write_bio_X509(MOCKED_BIO, IGNORED_PTR_ARG)).SetReturn(1);
    STRICT_EXPECTED_CALL(BIO_ctrl(MOCKED_BIO, BIO_CTRL_INFO, IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(MOCKED_BUFFER_LENGTH);
    STRICT_EXPECTED_CALL(BIO_read(MOCKED_BIO, IGNORED_PTR_ARG, MOCKED_BUFFER_LENGTH));
    STRICT_EXPECTED_CALL(BIO_free(MOCKED_BIO));

    STRICT_EXPECTED_CALL(BIO_s_mem());
    STRICT_EXPECTED_CALL(BIO_new(IGNORED_PTR_ARG)).SetReturn(MOCKED_BIO);
    STRICT_EXPECTED_CALL(PEM_write_bio_PrivateKey(MOCKED_BIO, IGNORED_PTR_ARG, 0, NULL, 0, NULL, NULL)).SetReturn(1);
    STRICT_EXPECTED_CALL(BIO_ctrl(MOCKED_BIO, BIO_CTRL_INFO, IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(MOCKED_BUFFER_LENGTH);
    STRICT_EXPECTED_CALL(BIO_read(MOCKED_BIO, IGNORED_PTR_ARG, MOCKED_BUFFER_LENGTH));
    STRICT_EXPECTED_CALL(BIO_free(MOCKED_BIO));

    STRICT_EXPECTED_CALL(fclose(MOCKED_FILE));
    STRICT_EXPECTED_CALL(X509_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(EVP_PKEY_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(PKCS12_free(IGNORED_PTR_ARG));

    char* certificate = NULL;
    char* privateKey = NULL;
    bool result = CertificateManager_LoadFromFile("file.pfx", &certificate, &privateKey);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    free(certificate);
    free(privateKey);
}

END_TEST_SUITE(certificate_manager_ut)
