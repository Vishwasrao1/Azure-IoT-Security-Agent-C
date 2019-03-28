// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "collectors/generic_event.h"
#include "agent_telemetry_provider.h"
#include "synchronized_queue.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#undef ENABLE_MOCKS
#include "collectors/agent_telemetry_collector.h"
#include "message_schema_consts.h"



static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

JsonWriterResult Mocked_JsonObjectWriter_Init(JsonObjectWriterHandle* writer) {
    *writer = (JsonObjectWriterHandle)0x1;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init(JsonArrayWriterHandle* writer) {
    *writer = (JsonArrayWriterHandle)0x2;
    return JSON_WRITER_OK;
}

BEGIN_TEST_SUITE(agent_telemetry_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);
    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(AgentQueueMeter, int);
    REGISTER_UMOCK_ALIAS_TYPE(AgentTelemetryProviderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(AgentTelemetryProviderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayReaderHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectReaderHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init);  
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);

    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

void setupEventInitExpectSuccess(const char* eventName, const char * eventSchemaVersion){
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, eventName, EVENT_TYPE_OPERATIONAL_VALUE, eventSchemaVersion)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
}

void setupAddDroppedEventsPayloadAddExpectSuccess(AgentQueueMeter queue){
    STRICT_EXPECTED_CALL(AgentTelemetryProvider_GetQueueCounterData(queue, IGNORED_PTR_ARG)).SetReturn(TELEMETRY_PROVIDER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    char* queueName = queue == HIGH_PRIORITY ? "High" : "Low";
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_TELEMETRY_QUEUE_EVENTS_KEY, queueName)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_COLLECTED_EVENTS_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_DROPPED_EVENTS_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

void setupAddMessageStatisticsPayloadAddExpectSuccess(){
    STRICT_EXPECTED_CALL(AgentTelemetryProvider_GetMessageCounterData(IGNORED_PTR_ARG)).SetReturn(TELEMETRY_PROVIDER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_SENT_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_FAILED_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_UNDER_4KB_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

void setupPushEventExpectSuccess(SyncQueue* queue){
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(queue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);
}

void setupCleanUpExpectSuccess(){
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

TEST_FUNCTION(AgentTelemetryProvider_GetEventsExpectSuccess)
{  
    SyncQueue queue = {NULL};
    //create event metadata
    setupEventInitExpectSuccess(AGENT_TELEMETRY_DROPPED_EVENTS_NAME, AGENT_TELEMETRY_DROPPED_EVENTS_SCHEMA_VERSION);
    setupAddDroppedEventsPayloadAddExpectSuccess(HIGH_PRIORITY);
    setupAddDroppedEventsPayloadAddExpectSuccess(LOW_PRIORITY);
    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();  

    //message stats
    setupEventInitExpectSuccess(AGENT_TELEMETRY_MESSAGE_STATISTICS_NAME, AGENT_TELEMETRY_MESSAGE_STATISTICS_SCHEMA_VERSION);
    setupAddMessageStatisticsPayloadAddExpectSuccess();
    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();    
    
    
    EventCollectorResult result = AgentTelemetryCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentTelemetryProvider_GetEventsFail)
{  
    umock_c_negative_tests_init();
    
    SyncQueue queue = {NULL}; 
    //create event metadata
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, AGENT_TELEMETRY_DROPPED_EVENTS_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_TELEMETRY_DROPPED_EVENTS_SCHEMA_VERSION)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);

    STRICT_EXPECTED_CALL(AgentTelemetryProvider_GetQueueCounterData(HIGH_PRIORITY, IGNORED_PTR_ARG)).SetFailReturn(!TELEMETRY_PROVIDER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_TELEMETRY_QUEUE_EVENTS_KEY, "High")).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_COLLECTED_EVENTS_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_DROPPED_EVENTS_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(AgentTelemetryProvider_GetQueueCounterData(LOW_PRIORITY, IGNORED_PTR_ARG)).SetFailReturn(!TELEMETRY_PROVIDER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_TELEMETRY_QUEUE_EVENTS_KEY, "Low")).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_COLLECTED_EVENTS_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_DROPPED_EVENTS_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&queue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(1);

    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    //create event metadata
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, AGENT_TELEMETRY_MESSAGE_STATISTICS_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_TELEMETRY_MESSAGE_STATISTICS_SCHEMA_VERSION)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);

    STRICT_EXPECTED_CALL(AgentTelemetryProvider_GetMessageCounterData(IGNORED_PTR_ARG)).SetFailReturn(!TELEMETRY_PROVIDER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_SENT_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_FAILED_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, AGENT_TELEMETRY_MESSAGES_UNDER_4KB_KEY, IGNORED_NUM_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(1);

    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    umock_c_negative_tests_snapshot();
    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 9 || i == 16 || i == 20 || i == 21 || i == 31 || i == 35 || i == 36) {
            // skip deinit since they don't have a fail return
            continue;
        }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        EventCollectorResult result = AgentTelemetryCollector_GetEvents(&queue);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }

    umock_c_negative_tests_deinit();
}

END_TEST_SUITE(agent_telemetry_collector_ut)
