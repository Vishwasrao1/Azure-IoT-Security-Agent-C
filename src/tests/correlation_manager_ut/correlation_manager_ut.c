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
#include "pthread_mock.h"
#include "azure_c_shared_utility/uniqueid.h"
#undef ENABLE_MOCKS

#include "os_utils/correlation_manager.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

int mocked_pthread_setspecific(pthread_key_t __key, const void *__pointer) {
    if (__pointer != NULL) {
        free((void*)__pointer);
    }
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(correlation_manager_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(keyDestroyer, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pthread_key_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(UNIQUEID_RESULT, int);

    REGISTER_GLOBAL_MOCK_HOOK(pthread_setspecific, mocked_pthread_setspecific);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);

    REGISTER_GLOBAL_MOCK_HOOK(pthread_setspecific, NULL);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(CorrelationManager_Init_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(pthread_key_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    bool result = CorrelationManager_Init();
    ASSERT_IS_TRUE(result);
}

TEST_FUNCTION(CorrelationManager_GetCorrelation_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(pthread_key_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    bool result = CorrelationManager_Init();
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(pthread_getspecific(IGNORED_NUM_ARG)).SetReturn((void*)"Aureliano");
    const char* correlationId = CorrelationManager_GetCorrelation();

    ASSERT_ARE_EQUAL(char_ptr, "Aureliano", correlationId);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(CorrelationManager_GetCorrelationEmpty_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(pthread_key_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    bool result = CorrelationManager_Init();
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(pthread_getspecific(IGNORED_NUM_ARG)).SetReturn(NULL);
    const char* correlationId = CorrelationManager_GetCorrelation();

    ASSERT_ARE_EQUAL(char_ptr, "00000000-0000-0000-0000-000000000000", correlationId);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(CorrelationManager_SetCorrelation_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(pthread_key_create(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    bool result = CorrelationManager_Init();
    ASSERT_IS_TRUE(result);
    char* currentCorrelation = strdup("Buendia");

    STRICT_EXPECTED_CALL(UniqueId_Generate(IGNORED_PTR_ARG, 37));
    STRICT_EXPECTED_CALL(pthread_getspecific(IGNORED_NUM_ARG)).SetReturn(currentCorrelation);
    STRICT_EXPECTED_CALL(pthread_setspecific(IGNORED_NUM_ARG, NULL));;
    STRICT_EXPECTED_CALL(pthread_setspecific(IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    result = CorrelationManager_SetCorrelation();
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(correlation_manager_ut)