// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#define ENABLE_MOCKS
#include "twin_configuration.h"
#undef ENABLE_MOCKS

#include "internal/internal_memory_monitor.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static uint32_t mockedMaxLocalCacheSize = 0;
TwinConfigurationResult Mocked_TwinConfiguration_GetMaxLocalCacheSize(uint32_t* maxLocalCacheSize) {
    *maxLocalCacheSize = mockedMaxLocalCacheSize;
    return TWIN_OK;
}

BEGIN_TEST_SUITE(internal_memory_monitor_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(MemoryMonitorResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);

    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxLocalCacheSize, Mocked_TwinConfiguration_GetMaxLocalCacheSize);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxLocalCacheSize, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(InternalMemoryMonitor_Consume_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Consume_TwoItems_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Consume(3);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Consume_MemoryExceeded_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(15);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_MEMORY_EXCEEDED, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Consume_TwinFailed_ExpectFailure)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG)).SetReturn(!TWIN_OK);
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(15);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_EXCEPTION, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Release_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Release(1);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);
    result = InternalMemoryMonitor_Release(1);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Release_TwoItems_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Release(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_Release_InvalidSize_ExpectFailure)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));

    MemoryMonitorResultValues result = InternalMemoryMonitor_Release(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_INVALID_RELEASE_SIZE, result);

    InternalMemoryMonitor_Deinit();
}

TEST_FUNCTION(InternalMemoryMonitor_CurrentConsumption_ExpectSuccess)
{
    InternalMemoryMonitor_Init();
    mockedMaxLocalCacheSize = 10;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));

    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(7);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Release(3);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Release(1);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    result = InternalMemoryMonitor_Consume(5);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);

    uint32_t size = 0;
    result = InternalMemoryMonitor_CurrentConsumption(&size);
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_OK, result);
    ASSERT_ARE_EQUAL(int, 8, size);

    InternalMemoryMonitor_Deinit();
}

END_TEST_SUITE(internal_memory_monitor_ut)
