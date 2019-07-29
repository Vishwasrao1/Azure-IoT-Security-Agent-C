// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/lock.h"
#undef ENABLE_MOCKS
#include "agent_telemetry_counters.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(agent_telemetry_counter_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);
    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int);

}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(AgentTelemetryCounters_InitSuccess){
    SyncedCounter counter;
    Counter emptyCounter;
    memset(&emptyCounter, 0, sizeof(Counter));

    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn((LOCK_HANDLE)0x1);

    bool result = AgentTelemetryCounter_Init(&counter);
    int memDiff = memcmp(&counter.counter, &emptyCounter, sizeof(Counter));
    
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, 0, memDiff);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentTelemetryCounters_InitFail){
    SyncedCounter counter; 
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(NULL);

    ASSERT_IS_FALSE(AgentTelemetryCounter_Init(&counter));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentTelemetryCounters_Deinit){
    SyncedCounter counter;
    counter.lock = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Lock_Deinit((LOCK_HANDLE)0x1));
    
    AgentTelemetryCounter_Deinit(&counter);
    ASSERT_IS_NULL(counter.lock);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentTelemetryCounters_SnapshotAndResetSuccess){
    Counter mockedData;
    SyncedCounter mockedCounter;
    Counter emptyCounter;

    memset(&emptyCounter, 0, sizeof(Counter));

    mockedData.messageCounter.sentMessages = 3;
    mockedData.messageCounter.smallMessages = 2;
    mockedData.messageCounter.failedMessages = 1;
    
    mockedCounter.lock = (LOCK_HANDLE) 0x1;
    mockedCounter.counter = mockedData;

    Counter dataOut;
    STRICT_EXPECTED_CALL(Lock((LOCK_HANDLE)0x1));
    STRICT_EXPECTED_CALL(Unlock((LOCK_HANDLE)0x1));
    
    bool result = AgentTelemetryCounter_SnapshotAndReset(&mockedCounter, &dataOut);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, mockedData.messageCounter.sentMessages, dataOut.messageCounter.sentMessages);
    ASSERT_ARE_EQUAL(int, mockedData.messageCounter.smallMessages, dataOut.messageCounter.smallMessages);
    ASSERT_ARE_EQUAL(int, mockedData.messageCounter.failedMessages, dataOut.messageCounter.failedMessages);
    ASSERT_ARE_EQUAL(int, mockedCounter.counter.messageCounter.sentMessages, 0);
    ASSERT_ARE_EQUAL(int, mockedCounter.counter.messageCounter.smallMessages, 0);
    ASSERT_ARE_EQUAL(int, mockedCounter.counter.messageCounter.failedMessages, 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentTelemetryCounters_SnapshotAndResetSuccessFail){
    SyncedCounter mockedCounter;
    mockedCounter.lock = (LOCK_HANDLE) 0x1;
    Counter dataOut;
    
    umock_c_negative_tests_init();
    STRICT_EXPECTED_CALL(Lock((LOCK_HANDLE)0x1)).SetFailReturn(LOCK_ERROR);
    STRICT_EXPECTED_CALL(Unlock((LOCK_HANDLE)0x1)).SetFailReturn(LOCK_ERROR);

    umock_c_negative_tests_snapshot();
    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        bool result = AgentTelemetryCounter_SnapshotAndReset(&mockedCounter, &dataOut);
        ASSERT_IS_FALSE(result);
    }
}
END_TEST_SUITE(agent_telemetry_counter_ut)
