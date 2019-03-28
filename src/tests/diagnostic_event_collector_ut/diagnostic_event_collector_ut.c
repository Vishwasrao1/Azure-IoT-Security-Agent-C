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
#include "json/json_object_writer.h"
#include "json/json_array_writer.h"
#include "os_utils/os_utils.h"
#include "os_utils/correlation_manager.h"
#include "internal/time_utils.h"
#include "synchronized_queue.h"
#include "collectors/generic_event.h"
#undef ENABLE_MOCKS

#include "collectors/diagnostic_event_collector.h"
#include "message_schema_consts.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

int Mocked_SyncQueue_PushBack(SyncQueue* syncQueue, void* data, uint32_t dataSize) {
    DiagnosticEvent* testData = (DiagnosticEvent*)(data);
    if (data != NULL) {
        if (testData->message != NULL) {
            free(testData->message);
        }
        if (testData->correlationId != NULL) {
            free(testData->correlationId);
        }
        free(data);
    }

    return QUEUE_OK;
}

#define TEST_MESSAGE "Wingardium Leviosa"
#define TEST_PROCESS_ID 42
#define TEST_THREAD_ID 13
#define TEST_CORRELATION_ID "Couscous"
static DiagnosticEvent TEST_DIAGNOSTIC_EVENT = { TEST_MESSAGE, SEVERITY_WARNING, TEST_PROCESS_ID, TEST_THREAD_ID, 0, TEST_CORRELATION_ID };

int Mocked_SyncQueue_PopFront(SyncQueue* syncQueue, void** data, uint32_t* dataSize) {
    *data = &TEST_DIAGNOSTIC_EVENT;
    *dataSize = 0;
    return QUEUE_OK;
}

int Mocked_InternalSyncQueue_PopFront(SyncQueue* syncQueue, void** data, uint32_t* dataSize) {
    *data = malloc(sizeof(DiagnosticEvent));
    DiagnosticEvent* event = ((DiagnosticEvent*)*data);
    memcpy(*data, (char*)&TEST_DIAGNOSTIC_EVENT, sizeof(TEST_DIAGNOSTIC_EVENT));
    
    event->message = malloc(strlen(TEST_MESSAGE) + 1);
    memcpy(event->message, TEST_MESSAGE, strlen(TEST_MESSAGE));
    event->message[strlen(TEST_MESSAGE)] = '\0';

    event->correlationId = malloc(strlen(TEST_CORRELATION_ID) + 1);
    memcpy(event->correlationId, TEST_CORRELATION_ID, strlen(TEST_CORRELATION_ID));
    event->correlationId[strlen(TEST_CORRELATION_ID)] = '\0';

    *dataSize = 0;
    return QUEUE_OK;
}

static int TEST_SIZE = 4;
int Mocked_SyncQueue_GetSize(SyncQueue* syncQueue, uint32_t* size) {
    *size = TEST_SIZE;
    return QUEUE_OK;
}

static time_t dummyTime;

BEGIN_TEST_SUITE(diagnostic_event_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(time_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);

    REGISTER_GLOBAL_MOCK_RETURN(CorrelationManager_GetCorrelation, TEST_CORRELATION_ID);
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
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PushBack, Mocked_SyncQueue_PushBack);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, Mocked_SyncQueue_PopFront);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, Mocked_SyncQueue_GetSize);
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PushBack, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, NULL);
}

TEST_FUNCTION(DiagnosticEventCollector_Init_ExpectSuccess)
{
    SyncQueue internalEventsQueue;

    EventCollectorResult result = DiagnosticEventCollector_Init(&internalEventsQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    TEST_SIZE = 0;
    DiagnosticEventCollector_Deinit();
}

TEST_FUNCTION(DiagnosticEventCollector_AddEvent_ExpectSuccess)
{
    SyncQueue internalEventsQueue;

    STRICT_EXPECTED_CALL(CorrelationManager_Init());
    DiagnosticEventCollector_Init(&internalEventsQueue);

    STRICT_EXPECTED_CALL(OsUtils_GetProcessId());
    STRICT_EXPECTED_CALL(OsUtils_GetThreadId());
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime());
    STRICT_EXPECTED_CALL(CorrelationManager_GetCorrelation());
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&internalEventsQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    DiagnosticEventCollector_AddEvent("uninteresting message", SEVERITY_INFORMATION);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    TEST_SIZE = 0;
    DiagnosticEventCollector_Deinit();
}

TEST_FUNCTION(DiagnosticEventCollector_GetEvents_ExpectSuccess)
{
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, Mocked_InternalSyncQueue_PopFront);

    SyncQueue internalEventsQueue, priorityQueue;

    STRICT_EXPECTED_CALL(CorrelationManager_Init());
    DiagnosticEventCollector_Init(&internalEventsQueue);

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&internalEventsQueue, IGNORED_PTR_ARG));

    TEST_SIZE = 4;
    for (int event=0; event < TEST_SIZE; event++) {
        STRICT_EXPECTED_CALL(SyncQueue_PopFront(&internalEventsQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(GenericEvent_AddMetadataWithTimes(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, DIAGNOSTIC_NAME, EVENT_TYPE_DIAGNOSTIC_VALUE, DIAGNOSTIC_PAYLOAD_SCHEMA_VERSION, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
        STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, DIAGNOSTIC_MESSAGE_KEY, TEST_MESSAGE));
        STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG,DIAGNOSTIC_SEVERITY_KEY, DIAGNOSTIC_SEVERITY_WARNING_VALUE));
        STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, DIAGNOSTIC_PROCESSID_KEY, TEST_PROCESS_ID));
        STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, DIAGNOSTIC_THREAD_KEY, TEST_THREAD_ID));
        STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, DIAGNOSTIC_CORRELATION_KEY, TEST_CORRELATION_ID));
        STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(SyncQueue_PushBack(&priorityQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    }

    EventCollectorResult result = DiagnosticEventCollector_GetEvents(&priorityQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    TEST_SIZE = 0;
    DiagnosticEventCollector_Deinit();
}

TEST_FUNCTION(DiagnosticEventCollector_GetEvents_QueueIsEmptyInLoop_ExpectSuccess)
{
    SyncQueue internalEventsQueue;
    SyncQueue priorityQueue;

    STRICT_EXPECTED_CALL(CorrelationManager_Init());
    DiagnosticEventCollector_Init(&internalEventsQueue);

    TEST_SIZE = 4;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&internalEventsQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&internalEventsQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(QUEUE_IS_EMPTY);

    EventCollectorResult result = DiagnosticEventCollector_GetEvents(&priorityQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    TEST_SIZE = 0;
    DiagnosticEventCollector_Deinit();
}

TEST_FUNCTION(DiagnosticEventCollector_GetEvents_ExpectFailures)
{
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, Mocked_InternalSyncQueue_PopFront);

    SyncQueue internalEventsQueue, priorityQueue;
    TEST_SIZE = 4;

    // non-failed function
    STRICT_EXPECTED_CALL(CorrelationManager_Init());
    DiagnosticEventCollector_Init(&internalEventsQueue);

    umock_c_negative_tests_init();

    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&internalEventsQueue, IGNORED_PTR_ARG)).SetFailReturn(!QUEUE_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&internalEventsQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(QUEUE_MAX_MEMORY_EXCEEDED);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadataWithTimes(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, DIAGNOSTIC_NAME, EVENT_TYPE_DIAGNOSTIC_VALUE, DIAGNOSTIC_PAYLOAD_SCHEMA_VERSION, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, DIAGNOSTIC_MESSAGE_KEY, TEST_MESSAGE)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG,DIAGNOSTIC_SEVERITY_KEY, DIAGNOSTIC_SEVERITY_WARNING_VALUE)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, DIAGNOSTIC_PROCESSID_KEY, TEST_PROCESS_ID)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(IGNORED_PTR_ARG, DIAGNOSTIC_THREAD_KEY, TEST_THREAD_ID)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, DIAGNOSTIC_CORRELATION_KEY, TEST_CORRELATION_ID)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);
    STRICT_EXPECTED_CALL (JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&priorityQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(!QUEUE_OK);

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 0) {
            continue;
        }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        EventCollectorResult result = DiagnosticEventCollector_GetEvents(&priorityQueue);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }

    umock_c_negative_tests_deinit();
    TEST_SIZE = 0;
    DiagnosticEventCollector_Deinit();
}

END_TEST_SUITE(diagnostic_event_collector_ut)