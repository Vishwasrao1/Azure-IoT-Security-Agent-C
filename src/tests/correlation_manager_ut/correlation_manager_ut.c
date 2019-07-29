// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdint.h>

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/uniqueid.h"
#undef ENABLE_MOCKS

#include "os_utils/correlation_manager.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static const char* mocked_uid = "So long, and thanks for all the fish";
static const size_t mocked_uid_len = 37;
static bool isInitialized = false;

UNIQUEID_RESULT mocked_UniqueId_Generate(char* uid, size_t buffer_size) {
    if (uid == NULL || buffer_size < mocked_uid_len) {
        return UNIQUEID_ERROR;
    }

    size_t size_to_copy = (buffer_size < mocked_uid_len) ? buffer_size : mocked_uid_len;
    strncpy(uid, mocked_uid, size_to_copy);

    return UNIQUEID_OK;
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(correlation_manager_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(UNIQUEID_RESULT, int);

    REGISTER_GLOBAL_MOCK_HOOK(UniqueId_Generate, mocked_UniqueId_Generate);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     

    REGISTER_GLOBAL_MOCK_HOOK(UniqueId_Generate, mocked_UniqueId_Generate);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_init) {
    if (isInitialized) {
        CorrelationManager_Deinit();
        isInitialized = false;
    }
}

TEST_FUNCTION(CorrelationManager_Init_ExpectSuccess)
{
    bool result = CorrelationManager_Init();
    isInitialized = result;
    ASSERT_IS_TRUE(result);
}

TEST_FUNCTION(CorrelationManager_GetCorrelationEmpty_ExpectSuccess)
{
    bool result = CorrelationManager_Init();
    isInitialized = result;
    ASSERT_IS_TRUE(result);

    const char* correlationId = CorrelationManager_GetCorrelation();

    ASSERT_ARE_EQUAL(char_ptr, "00000000-0000-0000-0000-000000000000", correlationId);
}

TEST_FUNCTION(CorrelationManager_SetAndGetCorrelation_ExpectSuccess)
{
    bool result = CorrelationManager_Init();
    isInitialized = result;
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(UniqueId_Generate(IGNORED_PTR_ARG, 37));
    result = CorrelationManager_SetCorrelation();
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    const char* correlation = CorrelationManager_GetCorrelation();
    ASSERT_ARE_EQUAL(char_ptr, mocked_uid, correlation);
}

END_TEST_SUITE(correlation_manager_ut)