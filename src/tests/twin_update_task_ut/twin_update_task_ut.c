// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "synchronized_queue.h"
#include "twin_configuration.h"
#include "iothub_adapter.h"
#undef ENABLE_MOCKS

#include "tasks/update_twin_task.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static const char DUMMY_JSON[] =  "{'test' : 'yes', 'a' : 'b'}";
static uint32_t mockedSyncQueueGetSizeSize = 0;
static int mockedSyncQueueGetSizeReturnValue = QUEUE_OK;

int Mocked_SyncQueue_GetSize(SyncQueue* syncQueue, uint32_t* size) { 
    *size = mockedSyncQueueGetSizeSize;
    return mockedSyncQueueGetSizeReturnValue;
}

static int mockedSyncQueuePopFrontReturnValue = QUEUE_OK;
static UpdateTwinState mockedSyncQueuePopFrontTwinState = TWIN_COMPLETE;

int Mocked_SyncQueue_PopFront(SyncQueue* syncQueue, void** data, uint32_t* dataSize) {
    if (mockedSyncQueuePopFrontReturnValue != QUEUE_OK) {
        return mockedSyncQueuePopFrontReturnValue;
    }
    *data = malloc(sizeof(UpdateTwinTaskItem));
    *dataSize = sizeof(UpdateTwinTaskItem);
    UpdateTwinTaskItem* item = (UpdateTwinTaskItem*)(*data);
    item->state = mockedSyncQueuePopFrontTwinState;
    item->twinPayload = strdup(DUMMY_JSON);
    return mockedSyncQueuePopFrontReturnValue;
}

BEGIN_TEST_SUITE(twin_update_task_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(UpdateTwinState, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);

    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, Mocked_SyncQueue_GetSize);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, Mocked_SyncQueue_PopFront);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     

    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_GetSize, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PopFront, NULL);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(UpdateTwinTask_Init_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);
}

TEST_FUNCTION(UpdateTwinTask_Deinit_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 0;
    UpdateTwinTask_Deinit(&task);
    ASSERT_IS_NULL(task.updateQueue);
}

TEST_FUNCTION(UpdateTwinTask_Execute_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&queue, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&queue, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontTwinState = TWIN_COMPLETE;
    STRICT_EXPECTED_CALL(TwinConfiguration_Update(DUMMY_JSON, true));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetSerializedTwinConfiguration(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SetReportedPropertiesAsync(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    UpdateTwinTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UpdateTwinTask_ExecutePartialTwin_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&queue, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&queue, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontTwinState = TWIN_PARTIAL;
    STRICT_EXPECTED_CALL(TwinConfiguration_Update(DUMMY_JSON, false));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetSerializedTwinConfiguration(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubAdapter_SetReportedPropertiesAsync(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));

    UpdateTwinTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UpdateTwinTask_ExecuteQueueIsEmpty_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 0;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&queue, IGNORED_PTR_ARG));

    UpdateTwinTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UpdateTwinTask_ExecutePopFrontReturnsQueueIsEmpty_ExpectSuccess)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&queue, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontReturnValue = QUEUE_IS_EMPTY;
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&queue, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    
    UpdateTwinTask_Execute(&task);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UpdateTwinTask_ExecuteUpdateTwinFailed_ExpectFailure)
{
    UpdateTwinTask task;
    SyncQueue queue;
    IoTHubAdapter client;

    bool result = UpdateTwinTask_Init(&task, &queue, &client);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, &queue, task.updateQueue);

    mockedSyncQueueGetSizeSize = 1;
    mockedSyncQueueGetSizeReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_GetSize(&queue, IGNORED_PTR_ARG));
    mockedSyncQueuePopFrontReturnValue = QUEUE_OK;
    STRICT_EXPECTED_CALL(SyncQueue_PopFront(&queue, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_Update(DUMMY_JSON, true)).SetReturn(TWIN_EXCEPTION);

    UpdateTwinTask_Execute(&task);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UpdateTwinTask_InitUpdateTwinTaskItem_ExpectSuccess)
{
    UpdateTwinTaskItem* twinTaskItem;
    bool result = UpdateTwinTask_InitUpdateTwinTaskItem(&twinTaskItem, NULL, 0, true);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, TWIN_COMPLETE, twinTaskItem->state);
    UpdateTwinTask_DeinitUpdateTwinTaskItem(&twinTaskItem);
    ASSERT_IS_NULL(twinTaskItem);

    result = UpdateTwinTask_InitUpdateTwinTaskItem(&twinTaskItem, NULL, 0, false);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, TWIN_PARTIAL, twinTaskItem->state);
    UpdateTwinTask_DeinitUpdateTwinTaskItem(&twinTaskItem);
    ASSERT_IS_NULL(twinTaskItem);
}

END_TEST_SUITE(twin_update_task_ut)
