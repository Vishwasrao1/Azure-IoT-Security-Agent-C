// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdint.h>

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"


#define ENABLE_MOCKS
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "local_config.h"
#include "synchronized_queue.h"
#include "twin_configuration.h"
#undef ENABLE_MOCKS

#include "json/json_defs.h"
#include "local_config.h"
#include "message_schema_consts.h"
#include "message_serializer.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static const char DUMMY_JSON[] =  "{ \"test\" : \"yes\", \"a\" : \"b\"}";
static SyncQueue mainQueue;
static SyncQueue paddingQueue;

static uint32_t mainQueueMockedSyncQueueGetSizeMainSize = 0;
static uint32_t paddingQueueMockedSyncQueueGetSizeSize = 0;
static int mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
static int mockedSyncQueuePopFrontReturnValue = QUEUE_OK;
static uint32_t mockedGetMaxSizeValue = 0;
static TwinConfigurationResult mockedGetMaxSizeReturnValue = TWIN_OK;
static JsonObjectWriterHandle mockedObjectWriterHandle = (JsonObjectWriterHandle)0x1;
static JsonArrayWriterHandle mockedArrayWriterHandle = (JsonArrayWriterHandle)0x2;
static char TEST_AGENT_ID[] = "ea05af2d-7397-4a1b-9ec7-3dc15e762a69";

int Mocked_SyncQueue_GetSize(SyncQueue* syncQueue, uint32_t* size) { 
    if (syncQueue == &mainQueue) {
        *size = mainQueueMockedSyncQueueGetSizeMainSize;
    } else {
        *size = paddingQueueMockedSyncQueueGetSizeSize;
    }
    
    return mockedSyncQueueGetSizeReturnValue;
}

int Mocked_SyncQueue_PopFrontIf(SyncQueue* syncQueue, QueuePopCondition condition, void* conditionParams, void** data, uint32_t* dataSize) {
    if (mockedSyncQueuePopFrontReturnValue != QUEUE_OK) {
        return mockedSyncQueuePopFrontReturnValue;
    }

    if (!condition(NULL, strlen(DUMMY_JSON), conditionParams)) {
        return QUEUE_CONDITION_FAILED;
    }

    *data = strdup(DUMMY_JSON);
    *dataSize = strlen(DUMMY_JSON);
    if (syncQueue == &mainQueue) {
        mainQueueMockedSyncQueueGetSizeMainSize--;
    } else {
        paddingQueueMockedSyncQueueGetSizeSize--;
    }
    return mockedSyncQueuePopFrontReturnValue;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetMaxMessageSize(uint32_t* maxMessageSize) {
    if (mockedGetMaxSizeReturnValue != TWIN_OK) {
        return mockedGetMaxSizeReturnValue;
    }
    *maxMessageSize = mockedGetMaxSizeValue;
    return mockedGetMaxSizeReturnValue;
}

JsonWriterResult Mocked_JsonObjectWriter_Init_SetWriterToNotNull(JsonObjectWriterHandle* writer) {
    *writer = mockedObjectWriterHandle;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonObjectWriter_InitFromString_SetWriterToNotNull(JsonObjectWriterHandle* writer, const char* json) {
    *writer = mockedObjectWriterHandle;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init_SetWriterToNotNull(JsonArrayWriterHandle* writer) {
    *writer = mockedArrayWriterHandle;
    return JSON_WRITER_OK;
}

BEGIN_TEST_SUITE(message_serializer_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(MessageSerializerResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueuePopCondition, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);

    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, Mocked_SyncQueue_GetSize);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFrontIf, Mocked_SyncQueue_PopFrontIf);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, Mocked_TwinConfiguration_GetMaxMessageSize);
    REGISTER_GLOBAL_MOCK_RETURN(LocalConfiguration_GetAgentId, TEST_AGENT_ID);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init_SetWriterToNotNull);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_InitFromString, Mocked_JsonObjectWriter_InitFromString_SetWriterToNotNull);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init_SetWriterToNotNull);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);

    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_InitFromString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(MessageSerializer_CreateSecurityMessage_MainQueueHasDataPaddingQueueIsEmpty_ExpectSuccess)
{
    char* buffer = NULL;
    mainQueueMockedSyncQueueGetSizeMainSize = 1;
    paddingQueueMockedSyncQueueGetSizeSize = 0;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;

    mockedGetMaxSizeReturnValue = TWIN_OK;
    mockedGetMaxSizeValue = strlen(DUMMY_JSON) + 1;

    // write the beginning of the message
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_VERSION_KEY, AGENT_VERSION)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetAgentId());
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_ID_KEY, TEST_AGENT_ID)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, MESSAGE_SCHEMA_VERSION_KEY, DEFAULT_MESSAGE_SCHEMA_VERSION)).SetReturn(JSON_WRITER_OK);

    // initial settings
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));

    // write to the array
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));

    // write all elements fron main queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFrontIf(&mainQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_InitFromString(IGNORED_PTR_ARG, DUMMY_JSON));
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(mockedArrayWriterHandle, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));

    // write all elements fron padding queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&paddingQueue, IGNORED_PTR_ARG));

    // writes the array and serialize
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteArray(mockedObjectWriterHandle, EVENTS_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(mockedArrayWriterHandle));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(mockedObjectWriterHandle, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));

    SyncQueue* queues[] = {&mainQueue, &paddingQueue};
    MessageSerializerResultValues result = MessageSerializer_CreateSecurityMessage(queues, 2, (void**)&buffer);

    ASSERT_ARE_EQUAL(int, MESSAGE_SERIALIZER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(MessageSerializer_CreateSecurityMessage_MainQueueHasDataPaddingQueueHasDataMaxMessageSizeReached_ExpectSuccess)
{
    char* buffer = NULL;
    mainQueueMockedSyncQueueGetSizeMainSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;

    paddingQueueMockedSyncQueueGetSizeSize = 1;

    mockedGetMaxSizeReturnValue = TWIN_OK;
    mockedGetMaxSizeValue = strlen(DUMMY_JSON) + 1;

     // write the beginning of the message
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_VERSION_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetAgentId());
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_ID_KEY, TEST_AGENT_ID)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, MESSAGE_SCHEMA_VERSION_KEY, DEFAULT_MESSAGE_SCHEMA_VERSION)).SetReturn(JSON_WRITER_OK);

    // initial settings
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));

    // write to the array
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));

    // write all elements fron main queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFrontIf(&mainQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_InitFromString(IGNORED_PTR_ARG, DUMMY_JSON));
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(mockedArrayWriterHandle, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));

    // write all elements fron padding queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&paddingQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFrontIf(&paddingQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // writes the array and serialize
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteArray(mockedObjectWriterHandle, EVENTS_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(mockedArrayWriterHandle));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(mockedObjectWriterHandle, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));

    SyncQueue* queues[] = {&mainQueue, &paddingQueue};
    MessageSerializerResultValues result = MessageSerializer_CreateSecurityMessage(queues, 2, (void**)&buffer);

    ASSERT_ARE_EQUAL(int, MESSAGE_SERIALIZER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(MessageSerializer_CreateSecurityMessage_MainQueueHasDataPaddingQueueHasData_ExpectSuccess)
{
    char* buffer = NULL;
    mainQueueMockedSyncQueueGetSizeMainSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;

    paddingQueueMockedSyncQueueGetSizeSize = 1;

    mockedGetMaxSizeReturnValue = TWIN_OK;
    mockedGetMaxSizeValue = strlen(DUMMY_JSON) * 3;

    // write the beginning of the message
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_VERSION_KEY, AGENT_VERSION)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetAgentId());
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, AGENT_ID_KEY, TEST_AGENT_ID)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockedObjectWriterHandle, MESSAGE_SCHEMA_VERSION_KEY, DEFAULT_MESSAGE_SCHEMA_VERSION)).SetReturn(JSON_WRITER_OK);

    // initial settings
    //STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));

    // write to the array
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));

    // write all elements fron main queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFrontIf(&mainQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_InitFromString(IGNORED_PTR_ARG, DUMMY_JSON));
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(mockedArrayWriterHandle, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&mainQueue, IGNORED_PTR_ARG));

    // write all elements fron padding queue
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&paddingQueue, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(SyncQueue_PopFrontIf(&paddingQueue, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_InitFromString(IGNORED_PTR_ARG, DUMMY_JSON));
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(mockedArrayWriterHandle, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&paddingQueue, IGNORED_PTR_ARG));

    // writes the array and serialize
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteArray(mockedObjectWriterHandle, EVENTS_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(mockedArrayWriterHandle));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(mockedObjectWriterHandle, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(mockedObjectWriterHandle));

    SyncQueue* queues[] = {&mainQueue, &paddingQueue};
    MessageSerializerResultValues result = MessageSerializer_CreateSecurityMessage(queues, 2, (void**)&buffer);

    ASSERT_ARE_EQUAL(int, MESSAGE_SERIALIZER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // freeing local buffer
    free(buffer);
}

END_TEST_SUITE(message_serializer_ut)
