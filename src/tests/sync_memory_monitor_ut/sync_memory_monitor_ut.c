// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/lock.h"
#include "internal/internal_memory_monitor.h"
#undef ENABLE_MOCKS

#include "memory_monitor.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(sync_memory_monitor_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(MemoryMonitorResultValues, int);
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

TEST_FUNCTION(SyncMemoryMonitor_Init_ExpectSuccess)
{
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Init());
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    bool result = MemoryMonitor_Init();
    ASSERT_IS_TRUE(result);

    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_Init_LockFailed_ExpectFailure)
{
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Init());
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(NULL);
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Deinit());

    bool result = MemoryMonitor_Init();
    ASSERT_IS_FALSE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_Deinit_ExpectSuccess)
{
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Init());
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    bool result = MemoryMonitor_Init();
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Deinit());
    STRICT_EXPECTED_CALL(Lock_Deinit(mockLockHandle));

    MemoryMonitor_Deinit();

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(SyncMemoryMonitor_Consume_ExpectSuccess)
{
    LOCK_HANDLE mockLockHandle = (LOCK_HANDLE)0x1;
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Init());
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(mockLockHandle);

    bool result = MemoryMonitor_Init();
    ASSERT_IS_TRUE(result);

    uint32_t size = 123;
    STRICT_EXPECTED_CALL(Lock(mockLockHandle)).SetReturn(LOCK_OK);
    MemoryMonitorResultValues mockedConsumeResult = MEMORY_MONITOR_MEMORY_EXCEEDED; 
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Consume(size)).SetReturn(mockedConsumeResult);
    STRICT_EXPECTED_CALL(Unlock(mockLockHandle)).SetReturn(LOCK_OK);

    MemoryMonitorResultValues memoryResult = MemoryMonitor_Consume(size);   
    ASSERT_ARE_EQUAL(int, mockedConsumeResult, memoryResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_Release_ExpectSuccess)
{
    uint32_t size = 123;
    STRICT_EXPECTED_CALL(Lock(IGNORED_PTR_ARG)).SetReturn(LOCK_OK);
    MemoryMonitorResultValues mockedConsumeResult = MEMORY_MONITOR_OK; 
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Release(size)).SetReturn(mockedConsumeResult);
    STRICT_EXPECTED_CALL(Unlock(IGNORED_PTR_ARG)).SetReturn(LOCK_OK);

    MemoryMonitorResultValues memoryResult = MemoryMonitor_Release(size);   
    ASSERT_ARE_EQUAL(int, mockedConsumeResult, memoryResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_CurrentConsumption_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock(IGNORED_PTR_ARG)).SetReturn(LOCK_OK);
    MemoryMonitorResultValues mockedConsumeResult = MEMORY_MONITOR_OK; 
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG)).SetReturn(mockedConsumeResult);
    STRICT_EXPECTED_CALL(Unlock(IGNORED_PTR_ARG)).SetReturn(LOCK_OK);

    MemoryMonitorResultValues memoryResult = MemoryMonitor_CurrentConsumption(IGNORED_PTR_ARG);   
    ASSERT_ARE_EQUAL(int, mockedConsumeResult, memoryResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_Consume_LockError_ExpectFailure)
{
    uint32_t size = 123;
    
    STRICT_EXPECTED_CALL(Lock(IGNORED_PTR_ARG)).SetReturn(!LOCK_OK);
   
    MemoryMonitorResultValues memoryResult = MemoryMonitor_Consume(size);   
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_EXCEPTION, memoryResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}

TEST_FUNCTION(SyncMemoryMonitor_Consume_UnlockError_ExpectFailure)
{
    uint32_t size = 123;
    
    STRICT_EXPECTED_CALL(Lock(IGNORED_PTR_ARG)).SetReturn(LOCK_OK);
    MemoryMonitorResultValues mockedConsumeResult = MEMORY_MONITOR_MEMORY_EXCEEDED; 
    STRICT_EXPECTED_CALL(InternalMemoryMonitor_Consume(size)).SetReturn(mockedConsumeResult);
    STRICT_EXPECTED_CALL(Unlock(IGNORED_PTR_ARG)).SetReturn(!LOCK_OK);

    MemoryMonitorResultValues memoryResult = MemoryMonitor_Consume(size);   
    ASSERT_ARE_EQUAL(int, MEMORY_MONITOR_EXCEPTION, memoryResult);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    MemoryMonitor_Deinit();
}


END_TEST_SUITE(sync_memory_monitor_ut)
