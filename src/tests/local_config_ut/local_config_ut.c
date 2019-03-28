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
#include "authentication_manager.h"
#include "json/json_object_reader.h"
#include "os_utils/file_utils.h"
#include "os_utils/os_utils.h"
#include "utils.h"
#undef ENABLE_MOCKS

#include "local_config.h"

static char DUMMY_DIRECTORY[] = "someplace";
char* Mocked_GetExecutableDirectory() {
    return strdup(DUMMY_DIRECTORY);
}

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char* MOCKED_STRING = "test";
JsonReaderResult Mocked_JsonObjectReader_ReadString(JsonObjectReaderHandle handle, const char* key, char** output) {
    *output = MOCKED_STRING;
    return JSON_READER_OK;
}

static const char* sharedAccessKey = "robencha";
FileResults Mocked_FileUtils_ReadFile(const char* filename, void* data, uint32_t readCount, bool maxCount) {
    memcpy(data, &sharedAccessKey, sizeof(sharedAccessKey));
    return FILE_UTILS_OK;
}

bool Mocked_AuthenticationManager_GetConnectionString(char* connectionString, uint32_t* connectionStringSize) {
    sprintf(connectionString, "I'm Batman!");
    return true;
}

BEGIN_TEST_SUITE(local_config_ut)

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
    REGISTER_UMOCK_ALIAS_TYPE(FileResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);

    REGISTER_GLOBAL_MOCK_HOOK(GetExecutableDirectory, Mocked_GetExecutableDirectory);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, Mocked_JsonObjectReader_ReadString);
    REGISTER_GLOBAL_MOCK_HOOK(FileUtils_ReadFile, Mocked_FileUtils_ReadFile);
    REGISTER_GLOBAL_MOCK_HOOK(AuthenticationManager_GetConnectionString, Mocked_AuthenticationManager_GetConnectionString);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(FileUtils_ReadFile, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    umock_c_reset_all_calls();
}

TEST_FUNCTION(LocalConfiguration_InitJsonAndValidateSecurityModuleKeyAuthentication_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(GetExecutableDirectory());
    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Configuration"));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AgentId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadTimeInMilliseconds(IGNORED_PTR_ARG, "TriggerdEventsInterval", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Authentication"));

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "DeviceId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "HostName", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AuthenticationMethod", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "Identity", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "FilePath", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SecurityModule", true)).SetReturn(true);
    
    STRICT_EXPECTED_CALL(FileUtils_ReadFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, true));
    STRICT_EXPECTED_CALL(AuthenticationManager_GenerateConnectionStringFromSharedAccessKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);

    int result = LocalConfiguration_Init();
    ASSERT_ARE_EQUAL(int, LOCAL_CONFIGURATION_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    LocalConfiguration_Deinit();
}

TEST_FUNCTION(LocalConfiguration_InitJsonAndValidateDeviceKeyAuthentication_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(GetExecutableDirectory());
    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Configuration"));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AgentId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadTimeInMilliseconds(IGNORED_PTR_ARG, "TriggerdEventsInterval", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Authentication"));

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "DeviceId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "HostName", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AuthenticationMethod", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "Identity", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "FilePath", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SecurityModule", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "Device", true)).SetReturn(true);
    
    STRICT_EXPECTED_CALL(AuthenticationManager_Init()).SetReturn(true);;
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SasToken", true)).SetReturn(true);

    STRICT_EXPECTED_CALL(AuthenticationManager_InitFromSharedAccessKey(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(AuthenticationManager_GetConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(AuthenticationManager_Deinit());

    int result = LocalConfiguration_Init();
    ASSERT_ARE_EQUAL(int, LOCAL_CONFIGURATION_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    LocalConfiguration_Deinit();
}

TEST_FUNCTION(LocalConfiguration_InitJsonAndValidateDeviceCertificateAuthentication_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(GetExecutableDirectory());
    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Configuration"));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AgentId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadTimeInMilliseconds(IGNORED_PTR_ARG, "TriggerdEventsInterval", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Authentication"));

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "DeviceId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "HostName", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AuthenticationMethod", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "Identity", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "FilePath", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SecurityModule", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "Device", true)).SetReturn(true);
    
    STRICT_EXPECTED_CALL(AuthenticationManager_Init()).SetReturn(true);;
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SasToken", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SelfSignedCertificate", true)).SetReturn(true);

    STRICT_EXPECTED_CALL(AuthenticationManager_InitFromCertificate(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(AuthenticationManager_GetConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(AuthenticationManager_Deinit());

    int result = LocalConfiguration_Init();
    ASSERT_ARE_EQUAL(int, LOCAL_CONFIGURATION_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    LocalConfiguration_Deinit();
}

TEST_FUNCTION(LocalConfiguration_InitJsonAndValidateDeviceNonExistingAuthentication_ExpectFailure)
{
    STRICT_EXPECTED_CALL(GetExecutableDirectory());
    STRICT_EXPECTED_CALL(JsonObjectReader_InitFromFile(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Configuration"));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AgentId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Utils_CreateStringCopy(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadTimeInMilliseconds(IGNORED_PTR_ARG, "TriggerdEventsInterval", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_StepIn(IGNORED_PTR_ARG, "Authentication"));

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "DeviceId", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "HostName", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "AuthenticationMethod", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "Identity", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(IGNORED_PTR_ARG, "FilePath", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SecurityModule", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "Device", true)).SetReturn(true);
    
    STRICT_EXPECTED_CALL(AuthenticationManager_Init()).SetReturn(true);;
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SasToken", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(Utils_UnsafeAreStringsEqual(IGNORED_PTR_ARG, "SelfSignedCertificate", true)).SetReturn(false);
    STRICT_EXPECTED_CALL(AuthenticationManager_Deinit());

    int result = LocalConfiguration_Init();
    ASSERT_ARE_EQUAL(int, LOCAL_CONFIGURATION_EXCEPTION, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    LocalConfiguration_Deinit();
}

END_TEST_SUITE(local_config_ut)
