// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#define ENABLE_MOCKS
#include "agent_telemetry_counters.h"

#undef ENABLE_MOCKS

#include "agent_telemetry_provider.h"


static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static SyncedCounter highPrioCounter;
static SyncedCounter lowPrioCounter;
static SyncedCounter iothubCounter;

bool getCounterData(SyncedCounter* counter, Counter* counterData){
    if (counter == & highPrioCounter){
        counterData->queueCounter.dropped = 1;
        counterData->queueCounter.collected = 2;
    } else if (counter == & lowPrioCounter){
        counterData->queueCounter.dropped = 3;
        counterData->queueCounter.collected = 4;
    } else if (counter == &iothubCounter){
        counterData->messageCounter.sentMessages = 3;
        counterData->messageCounter.smallMessages = 1;
        counterData->messageCounter.failedMessages = 2;
    } 

    return true;
}

BEGIN_TEST_SUITE(agent_telemetry_provider_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
  
    REGISTER_UMOCK_ALIAS_TYPE(AgentTelemetryProviderResult, int);

    REGISTER_GLOBAL_MOCK_HOOK(AgentTelemetryCounter_SnapshotAndReset, getCounterData);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(AgentTelemetryProvider_GethighPrioQueueCounterDataExpectSucess)
{  
    QueueCounter counterData;

    AgentTelemetryProviderResult result = AgentTelemetryProvider_Init(&lowPrioCounter, &highPrioCounter, &iothubCounter);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);

    result = AgentTelemetryProvider_GetQueueCounterData(HIGH_PRIORITY, &counterData);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);
    ASSERT_ARE_EQUAL(int, 2, counterData.collected);
    ASSERT_ARE_EQUAL(int, 1, counterData.dropped);
}

TEST_FUNCTION(AgentTelemetryProvider_GetlowPrioQueueCounterDataExpectSucess)
{  
    QueueCounter counterData;

    AgentTelemetryProviderResult result = AgentTelemetryProvider_Init(&lowPrioCounter, &highPrioCounter, &iothubCounter);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);

    result = AgentTelemetryProvider_GetQueueCounterData(LOW_PRIORITY, &counterData);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);
    ASSERT_ARE_EQUAL(int, 4, counterData.collected);
    ASSERT_ARE_EQUAL(int, 3, counterData.dropped);
}

TEST_FUNCTION(AgentTelemetryProvider_GetMessageCounterDataExpectSucess)
{  
    MessageCounter counterData;

    AgentTelemetryProviderResult result = AgentTelemetryProvider_Init(&lowPrioCounter, &highPrioCounter, &iothubCounter);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);

    result = AgentTelemetryProvider_GetMessageCounterData(&counterData);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);
    ASSERT_ARE_EQUAL(int, 3, counterData.sentMessages);
    ASSERT_ARE_EQUAL(int, 2, counterData.failedMessages);
    ASSERT_ARE_EQUAL(int, 1, counterData.smallMessages);
}

TEST_FUNCTION(AgentTelemetryProvider_GetlowPrioQueueCounterDataExpectFail)
{  
    MessageCounter counterData;

    AgentTelemetryProviderResult result = AgentTelemetryProvider_Init(&lowPrioCounter, &highPrioCounter, &iothubCounter);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);

    result = AgentTelemetryProvider_GetMessageCounterData(&counterData);
    ASSERT_ARE_EQUAL(int, TELEMETRY_PROVIDER_OK, result);
    ASSERT_ARE_EQUAL(int, 3, counterData.sentMessages);
    ASSERT_ARE_EQUAL(int, 2, counterData.failedMessages);
    ASSERT_ARE_EQUAL(int, 1, counterData.smallMessages);
}

END_TEST_SUITE(agent_telemetry_provider_ut)