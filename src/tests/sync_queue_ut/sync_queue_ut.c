// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdint.h>

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/lock.h"
#include "queue.h"
#undef ENABLE_MOCKS

#include "synchronized_queue.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

bool alwaysTrueCondition(const void* data, uint32_t size, void* params) {
    return true;
}

BEGIN_TEST_SUITE(sync_queue_ut)


TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(QueuePopCondition, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
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

TEST_FUNCTION(SyncQueue_Init_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_Init_QueueInitFailed_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).SetReturn(!QUEUE_OK).ValidateAllArguments();

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_NOT_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SyncQueue_Init_QueueInitSuceedLockInitFailed_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(NULL);
    STRICT_EXPECTED_CALL(Queue_Deinit(&syncQueue.queue)).ValidateAllArguments();
    
    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_NOT_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SyncQueue_Deinit_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);
    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    STRICT_EXPECTED_CALL(Queue_Deinit(&syncQueue.queue)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Deinit(mockLockHandle)).ValidateAllArguments();

    SyncQueue_Deinit(&syncQueue);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SyncQueue_SyncQueue_PushBack_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    void* data = "abcde";
    uint32_t dataSize = 5;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();
    //FIXME: do this to all return values?
    int queueReturnValue = QUEUE_MAX_MEMORY_EXCEEDED;
    STRICT_EXPECTED_CALL(Queue_PushBack(&syncQueue.queue, data, dataSize)).SetReturn(queueReturnValue).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();

    // test
    result = SyncQueue_PushBack(&syncQueue, data, dataSize);
    ASSERT_ARE_EQUAL(int, queueReturnValue, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_SyncQueue_PopFront_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    void* data = NULL;
    uint32_t dataSize = 0;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Queue_PopFront(&syncQueue.queue, &data, &dataSize)).SetReturn(QUEUE_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();

    // test
    result = SyncQueue_PopFront(&syncQueue, &data, &dataSize);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_SyncQueue_PopFrontIf_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    void* conditionParams = NULL;
    void* data = NULL;
    uint32_t dataSize = 0;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Queue_PopFrontIf(&syncQueue.queue, alwaysTrueCondition, &conditionParams, &data, &dataSize)).SetReturn(QUEUE_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();

    // test
    result = SyncQueue_PopFrontIf(&syncQueue, alwaysTrueCondition, &conditionParams ,&data, &dataSize);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_GetSize_ExpectSuccess)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    uint32_t size = 0;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Queue_GetSize(&syncQueue.queue, &size)).SetReturn(QUEUE_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();

    // test
    result = SyncQueue_GetSize(&syncQueue, &size);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_GetSize_LockFail_ExpectLockException)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true)).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    uint32_t size = 0;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_ERROR).ValidateAllArguments();

    // test
    result = SyncQueue_GetSize(&syncQueue, &size);
    ASSERT_ARE_EQUAL(int, SYNC_QUEUE_LOCK_EXCEPTION, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

TEST_FUNCTION(SyncQueue_GetSize_UnlockFail_ExpectLockException)
{
    SyncQueue syncQueue;
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(Queue_Init(&syncQueue.queue, true));
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    int result = SyncQueue_Init(&syncQueue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);
    
    uint32_t size = 0;

    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Queue_GetSize(&syncQueue.queue, &size)).SetReturn(QUEUE_OK).ValidateAllArguments();
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_ERROR).ValidateAllArguments();

    // test
    result = SyncQueue_GetSize(&syncQueue, &size);
    ASSERT_ARE_EQUAL(int, SYNC_QUEUE_LOCK_EXCEPTION, result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    SyncQueue_Deinit(&syncQueue);
}

END_TEST_SUITE(sync_queue_ut)
