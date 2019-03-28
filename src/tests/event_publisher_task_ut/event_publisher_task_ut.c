// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "internal/time_utils.h"
#include "iothub_adapter.h"
#include "memory_monitor.h"
#include "message_serializer.h"
#include "synchronized_queue.h"
#include "twin_configuration.h"
#undef ENABLE_MOCKS

#include "tasks/event_publisher_task.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static time_t dummyTime;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

MessageSerializerResultValues Mocked_MessageSerializer_CreateSecurityMessage(SyncQueue** queues, uint32_t size, void** buffer) {
    *buffer = strdup("a");
    return MESSAGE_SERIALIZER_OK;
}
static uint32_t mockedSyncQueueGetSizesize = 0;
static int mockedSyncQueueGetSizeReturnValue = QUEUE_OK;

int Mocked_SyncQueue_GetSize(SyncQueue* queue, uint32_t* size){
    *size = mockedSyncQueueGetSizesize;
    return mockedSyncQueueGetSizeReturnValue;
}

static uint32_t mockedCurrentMemoryConsumption = 0;
static uint32_t mockedMaxMessageSize = 0;


MemoryMonitorResultValues Mocked_MemoryMonitor_CurrentConsumption(uint32_t* size) {
    *size = mockedCurrentMemoryConsumption;
    return MEMORY_MONITOR_OK;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetMaxMessageSize(uint32_t* maxMessageSize) {
    *maxMessageSize = mockedMaxMessageSize;
    return TWIN_OK;
}

BEGIN_TEST_SUITE(event_publisher_task_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(MessageSerializerResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(MemoryMonitorResultValues, int);

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(int32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(time_t, unsigned int);

    struct tm tmpTime = {0, 0, 13, 14, 2, 118 }; // date is 'Mar 14 2018 13:02:00'
    dummyTime = mktime(&tmpTime);

    REGISTER_GLOBAL_MOCK_HOOK(MessageSerializer_CreateSecurityMessage, Mocked_MessageSerializer_CreateSecurityMessage);
    REGISTER_GLOBAL_MOCK_HOOK(MemoryMonitor_CurrentConsumption, Mocked_MemoryMonitor_CurrentConsumption);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, Mocked_TwinConfiguration_GetMaxMessageSize);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, Mocked_SyncQueue_GetSize);

}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(MessageSerializer_CreateSecurityMessage, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(MemoryMonitor_CurrentConsumption, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, NULL);

    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
    // the default behavior  is that we haven't passed the maxMessageSize
    mockedCurrentMemoryConsumption = 0;
    mockedMaxMessageSize = mockedCurrentMemoryConsumption + 10;
}

TEST_FUNCTION(EventPublisherTask_Init_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(dummyTime);
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(dummyTime);

    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &operationalEventsQueue, task.operationalEventsQueue);
    ASSERT_ARE_EQUAL(void_ptr, &highPriorityQueue, task.highPriorityEventQueue);
    ASSERT_ARE_EQUAL(void_ptr, &lowPriorityQueue, task.lowPriorityEventQueue);
    ASSERT_ARE_EQUAL(void_ptr, &adapter, task.iothubAdapter);
    ASSERT_ARE_EQUAL(uint32_t, dummyTime, task.highPriorityQueueLastExecution);
    ASSERT_ARE_EQUAL(uint32_t, dummyTime, task.lowPriorityQueueLastExecution);
}

TEST_FUNCTION(EventPublisherTask_Deinit_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(dummyTime);
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(dummyTime);

    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &highPriorityQueue, task.highPriorityEventQueue);
    ASSERT_ARE_EQUAL(void_ptr, &lowPriorityQueue, task.lowPriorityEventQueue);
    ASSERT_ARE_EQUAL(void_ptr, &adapter, task.iothubAdapter);
    ASSERT_ARE_EQUAL(uint32_t, dummyTime, task.highPriorityQueueLastExecution);
    ASSERT_ARE_EQUAL(uint32_t, dummyTime, task.lowPriorityQueueLastExecution);

    EventPublisherTask_Deinit(&task);
    ASSERT_IS_NULL(task.highPriorityEventQueue);
    ASSERT_IS_NULL(task.lowPriorityEventQueue);
    ASSERT_IS_NULL(task.iothubAdapter);
}

TEST_FUNCTION(EventPublisherTask_ExecuteTimeoutsDidNotPass_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // low priority queue

    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

TEST_FUNCTION(EventPublisherTask_ExecuteHigQueueTimeoutLowQueueDidNotTimeout_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(10); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // low priority queue

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SendMessageAsync(&adapter, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

TEST_FUNCTION(EventPublisherTask_ExecuteHigQueueDidNotTimeoutLowQueueTimeout_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(10); // low priority queue

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SendMessageAsync(&adapter, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

TEST_FUNCTION(EventPublisherTask_ExecuteHigQueueTimeoutLowQueueTimeout_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;    
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(10); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(10); // low priority queue

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SendMessageAsync(&adapter, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SendMessageAsync(&adapter, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

TEST_FUNCTION(EventPublisherTask_ExecuteHigQueueTimeoutLowQueueDidNotTimeoutCeateSecurityMessageFailed_ExpectFailure)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(10); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // low priority queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    SyncQueue* queues[] = {&highPriorityQueue, &lowPriorityQueue};
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG)).SetReturn(MESSAGE_SERIALIZER_EXCEPTION);
    
    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

TEST_FUNCTION(EventPublisherTask_Execute_MaxMessageSizeExceeded_ExpectSuccess)
{
    SyncQueue operationalEventsQueue;
    SyncQueue highPriorityQueue;
    SyncQueue lowPriorityQueue;
    IoTHubAdapter adapter;
    EventPublisherTask task;

    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    bool result = EventPublisherTask_Init(&task, &highPriorityQueue, &lowPriorityQueue, &operationalEventsQueue, &adapter);
    ASSERT_IS_TRUE(result);

    mockedCurrentMemoryConsumption = 10;
    mockedMaxMessageSize = 5;
    mockedSyncQueueGetSizesize = 1;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    SyncQueue* queues[] = {&highPriorityQueue, &lowPriorityQueue};
    STRICT_EXPECTED_CALL(MessageSerializer_CreateSecurityMessage(IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SendMessageAsync(&adapter, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // high priority queue
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0); // low priority queue

    EventPublisherTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    EventPublisherTask_Deinit(&task);
}

END_TEST_SUITE(event_publisher_task_ut)
