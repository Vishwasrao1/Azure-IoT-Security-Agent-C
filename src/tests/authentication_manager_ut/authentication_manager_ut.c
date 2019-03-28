// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

#define ENABLE_MOCKS
#include "json/json_object_reader.h"
#include "utils.h"
#include "certificate_manager.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/httpheaders.h"
#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/httpapiexsas.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/platform.h"
#include "os_utils/file_utils.h"
#undef ENABLE_MOCKS

#include "authentication_manager.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static HTTP_HEADERS_HANDLE MOCKED_HTTP_HEADERS_HANDLE = (HTTP_HEADERS_HANDLE)0x59;
static HTTP_HANDLE MOCKED_HTTP_HANDLE = (HTTP_HANDLE)0x58;
static BUFFER_HANDLE MOCKED_BUFFER_HANDLE = (BUFFER_HANDLE)0x57;
static HTTPAPIEX_HANDLE MOCKED_HTTPAPIEX_HANDLE = (HTTPAPIEX_HANDLE)0x56;

HTTPAPIEX_SAS_HANDLE Mocked_HTTPAPIEX_SAS_Create_From_String(const char* key, const char* uriResource, const char* keyName) {
    return malloc(1);
}

void Mocked_HTTPAPIEX_SAS_Destroy(HTTPAPIEX_SAS_HANDLE handle) {
    free(handle);
    return;
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char* DUMMY_STRING = "test";

JsonReaderResult Mocked_JsonObjectReader_ReadString(JsonObjectReaderHandle handle, const char* key, char** output) {
    *output = DUMMY_STRING;
    return JSON_READER_OK;
}

HTTPAPI_RESULT Mocked_HTTPAPI_ExecuteRequest(HTTP_HANDLE handle, HTTPAPI_REQUEST_TYPE requestType, const char* relativePath,
    HTTP_HEADERS_HANDLE httpHeadersHandle, const unsigned char* content,
    size_t contentLength, unsigned int* statusCode,
    HTTP_HEADERS_HANDLE responseHeadersHandle, BUFFER_HANDLE responseContent) {
    *statusCode = 200;
    return HTTPAPI_OK;
}

HTTPAPIEX_RESULT Mocked_HTTPAPIEX_SAS_ExecuteRequest(HTTPAPIEX_SAS_HANDLE sasHandle, HTTPAPIEX_HANDLE handle, HTTPAPI_REQUEST_TYPE requestType, const char* relativePath, HTTP_HEADERS_HANDLE requestHttpHeadersHandle, BUFFER_HANDLE requestContent, unsigned int* statusCode, HTTP_HEADERS_HANDLE responseHeadersHandle, BUFFER_HANDLE responseContent) {
    *statusCode = 200;
    return HTTPAPIEX_OK;
}

static const char* TEST_CERTIFICATE = "batman!";
static const char* TEST_PRIVATE_KEY = "also batman!";
bool Mocked_CertificateManager_LoadFromFile(const char* filePath, char** certificate, char** privateKey) {
    *certificate = strdup(TEST_CERTIFICATE);
    *privateKey = strdup(TEST_PRIVATE_KEY);
    return true;
}

BEGIN_TEST_SUITE(authentication_manager_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    (void)umocktypes_charptr_register_types();
    (void)umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectReaderHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t*, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPI_RESULT, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPI_REQUEST_TYPE, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(BUFFER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTP_HEADERS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPIEX_SAS_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPIEX_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, Mocked_JsonObjectReader_ReadString);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPI_ExecuteRequest, Mocked_HTTPAPI_ExecuteRequest);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_ExecuteRequest, Mocked_HTTPAPIEX_SAS_ExecuteRequest);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_Create_From_String, Mocked_HTTPAPIEX_SAS_Create_From_String);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_Destroy, Mocked_HTTPAPIEX_SAS_Destroy);
    REGISTER_GLOBAL_MOCK_HOOK(CertificateManager_LoadFromFile, Mocked_CertificateManager_LoadFromFile);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    AuthenticationManager_Deinit();
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPI_ExecuteRequest, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_ExecuteRequest, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_Create_From_String, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(CertificateManager_LoadFromFile, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(HTTPAPIEX_SAS_Destroy, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    AuthenticationManager_Init();
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_init)
{
    AuthenticationManager_Deinit();
}

TEST_FUNCTION(AuthenticationManager_InitWithCertificate_ExcpectSuccess)
{
    uint32_t localConnectionStringSize = 500;
    char localConnectionString[500] = "";
    STRICT_EXPECTED_CALL(CertificateManager_LoadFromFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Utils_CopyString("hostName", IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(Utils_CopyString("device", IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);

    STRICT_EXPECTED_CALL(HTTPHeaders_Alloc()).SetReturn(MOCKED_HTTP_HEADERS_HANDLE);
    STRICT_EXPECTED_CALL(HTTPAPI_Init());
    STRICT_EXPECTED_CALL(HTTPAPI_CreateConnection(IGNORED_PTR_ARG)).SetReturn(MOCKED_HTTP_HANDLE);
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(MOCKED_BUFFER_HANDLE);

    STRICT_EXPECTED_CALL(HTTPAPI_SetOption(IGNORED_PTR_ARG, SU_OPTION_X509_CERT, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPI_SetOption(IGNORED_PTR_ARG, SU_OPTION_X509_PRIVATE_KEY, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(HTTPAPI_ExecuteRequest(IGNORED_PTR_ARG, 
                                                HTTPAPI_REQUEST_GET, 
                                                IGNORED_PTR_ARG, 
                                                IGNORED_PTR_ARG, 
                                                NULL, 
                                                0, 
                                                IGNORED_PTR_ARG, 
                                                NULL, 
                                                IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromString(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "authentication.symmetricKey.primaryKey",IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPI_CloseConnection(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPI_Deinit());
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG));

    bool result = AuthenticationManager_InitFromCertificate("someFilePath", "hostName", "device");
    ASSERT_IS_TRUE(result);
    result = AuthenticationManager_GetConnectionString(localConnectionString, &localConnectionStringSize);
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AuthenticationManager_InitWithKey_ExcpectSuccess)
{
    uint32_t localConnectionStringSize = 500;
    char localConnectionString[500] = "";

    STRICT_EXPECTED_CALL(FileUtils_ReadFile("filePath", IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(Utils_CopyString("hostName", IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(Utils_CopyString("deviceId", IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(HTTPAPIEX_SAS_Create_From_String(IGNORED_PTR_ARG, IGNORED_PTR_ARG, NULL));

    STRICT_EXPECTED_CALL(HTTPHeaders_Alloc()).SetReturn(MOCKED_HTTP_HEADERS_HANDLE);
    STRICT_EXPECTED_CALL(HTTPHeaders_AddHeaderNameValuePair(MOCKED_HTTP_HEADERS_HANDLE, "Authorization", ""));
    STRICT_EXPECTED_CALL(HTTPAPIEX_Create(IGNORED_PTR_ARG)).SetReturn(MOCKED_HTTPAPIEX_HANDLE);
    STRICT_EXPECTED_CALL(BUFFER_new()).SetReturn(MOCKED_BUFFER_HANDLE);

    STRICT_EXPECTED_CALL(HTTPAPIEX_SAS_ExecuteRequest(  IGNORED_PTR_ARG,
                                                        IGNORED_PTR_ARG, 
                                                        HTTPAPI_REQUEST_GET, 
                                                        IGNORED_PTR_ARG, 
                                                        IGNORED_PTR_ARG, 
                                                        NULL, 
                                                        IGNORED_PTR_ARG, 
                                                        NULL, 
                                                        IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(BUFFER_u_char(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromString(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "authentication.symmetricKey.primaryKey",IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(HTTPHeaders_Free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(HTTPAPIEX_Destroy(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(BUFFER_delete(IGNORED_PTR_ARG));

    bool result = AuthenticationManager_InitFromSharedAccessKey("filePath","hostName","deviceId");
    ASSERT_IS_TRUE(result);
    result = AuthenticationManager_GetConnectionString(localConnectionString, &localConnectionStringSize);
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(authentication_manager_ut)
