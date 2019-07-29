// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umocktypes_bool.h"
#include "umockalloc.h"

#define ENABLE_MOCKS
#include "twin_configuration.h"
#include "memory_monitor.h"
#include "local_config.h"
#include "agent_telemetry_counters.h"
#undef ENABLE_MOCKS

#include "queue.h"
#include "logger.h"
#include <stdint.h>

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

bool alwaysTrueCondition(const void* data, uint32_t size, void* params) {
    return true;
}

bool alwaysFalseCondition(const void* data, uint32_t size, void* params) {
    return false;
}

BEGIN_TEST_SUITE(queue_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    (void)umocktypes_charptr_register_types();
    (void)umocktypes_bool_register_types();
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(MemoryMonitorResultValues, int);
    
    umock_c_reset_all_calls();
    
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true); 
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
}

TEST_FUNCTION(Queue_Init_ExpectSuccess)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    ASSERT_ARE_EQUAL(int, 0, queue.numberOfElements);
    ASSERT_IS_NULL(queue.firstItem);
    ASSERT_IS_NULL(queue.lastItem);

    Queue_Deinit(&queue);
}

TEST_FUNCTION(Queue_PushBackAndPop_ExpectSuccess)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* firstMessage = strdup("first message");
    char* secondMessage = strdup("second message");
    unsigned int size;

    result = Queue_PushBack(&queue, firstMessage, strlen(firstMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    result = Queue_PushBack(&queue, secondMessage, strlen(secondMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 2, size);

    char* output;
    unsigned int messageSize;
    result = Queue_PopFront(&queue, (void**)&output, &messageSize);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, firstMessage, output);
    ASSERT_ARE_EQUAL(int, strlen(firstMessage) + 1, messageSize);

    Queue_Deinit(&queue);

    // free the message popped from the queue
    free(output);
}

TEST_FUNCTION(Queue_PushBackAndPopIfConditionReutrnsTrue_ExpectSuccess)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* firstMessage = strdup("first message");
    char* secondMessage = strdup("second message");
    unsigned int size;

    result = Queue_PushBack(&queue, firstMessage, strlen(firstMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    result = Queue_PushBack(&queue, secondMessage, strlen(secondMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 2, size);

    char* output;
    unsigned int messageSize;
    result = Queue_PopFrontIf(&queue, alwaysTrueCondition, NULL, (void**)&output, &messageSize);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, firstMessage, output);
    ASSERT_ARE_EQUAL(int, strlen(firstMessage) + 1, messageSize);

    Queue_Deinit(&queue);

    // free the message popped from the queue
    free(output);
}

TEST_FUNCTION(Queue_PushBackAndPopIfConditionReutrnsFalse_ExpectSuccess)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* firstMessage = strdup("first message");
    char* secondMessage = strdup("second message");
    unsigned int size;

    result = Queue_PushBack(&queue, firstMessage, strlen(firstMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    result = Queue_PushBack(&queue, secondMessage, strlen(secondMessage)+1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 2, size);

    char* output;
    unsigned int messageSize;
    result = Queue_PopFrontIf(&queue, alwaysFalseCondition, NULL, (void**)&output, &messageSize);
    ASSERT_ARE_EQUAL(int, QUEUE_CONDITION_FAILED, result);
    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 2, size);

    Queue_Deinit(&queue);
}

TEST_FUNCTION(Queue_MaxLocalCacheSizeExceeded_ExpectMaxMemoryExceeded)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* message = strdup("first message");
    unsigned int size;
    umock_c_reset_all_calls();
    
    STRICT_EXPECTED_CALL(MemoryMonitor_Consume(IGNORED_NUM_ARG)).SetReturn(MEMORY_MONITOR_MEMORY_EXCEEDED).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true).IgnoreAllArguments();
    result = Queue_PushBack(&queue, message, strlen(message) + 1);
    ASSERT_ARE_EQUAL(int, QUEUE_MAX_MEMORY_EXCEEDED, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    STRICT_EXPECTED_CALL(MemoryMonitor_Consume(IGNORED_NUM_ARG)).SetReturn(MEMORY_MONITOR_OK).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true).IgnoreAllArguments();
    result = Queue_PushBack(&queue, message, strlen(message) + 1);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 1, size);

    Queue_Deinit(&queue);
}

TEST_FUNCTION(Queue_PopEmpty_ExpectQueueIsEmpty)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* firstMessage;
    unsigned int size;
    unsigned int messageSize;

    result = Queue_PopFront(&queue, (void**)firstMessage, &messageSize);
    ASSERT_ARE_EQUAL(int, QUEUE_IS_EMPTY, result);

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 0, size);

    Queue_Deinit(&queue);
}


TEST_FUNCTION(Queue_MemoryMonitorExcpetion_ExpectMemoryException)
{
    Queue queue;
    int result = Queue_Init(&queue, true);
    ASSERT_ARE_EQUAL(int, QUEUE_OK, result);

    char* firstMessage = strdup("first message");
    unsigned int size, messageSize;
    messageSize = strlen(firstMessage)+1;
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(MemoryMonitor_Consume(0)).SetReturn(MEMORY_MONITOR_EXCEPTION).IgnoreAllArguments();
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true).IgnoreAllArguments();

    result = Queue_PushBack(&queue, firstMessage, messageSize);
    ASSERT_ARE_EQUAL(int, QUEUE_MEMORY_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    Queue_GetSize(&queue, &size);
    ASSERT_ARE_EQUAL(int, 0, size);
    ASSERT_ARE_EQUAL(int, 0, queue.numberOfElements);

    Queue_Deinit(&queue);

    // free the message, as push to queue failed
    free(firstMessage);
}


END_TEST_SUITE(queue_ut)
