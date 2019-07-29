// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_mocks.h"
#undef ENABLE_MOCKS

#include "os_utils/process_info_handler.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(process_info_handler_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(uid_t, int);
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

TEST_FUNCTION(ProcessInfoHandler_ChangeToRoot_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(geteuid()).SetReturn(2);
    STRICT_EXPECTED_CALL(seteuid(0)).SetReturn(1);

    ProcessInfo info;
    bool result = ProcessInfoHandler_ChangeToRoot(&info);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, 2, info.effectiveUid);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_ChangeToRoot_AlreadyRoot_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(geteuid()).SetReturn(0);

    ProcessInfo info;
    bool result = ProcessInfoHandler_ChangeToRoot(&info);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, 0, info.effectiveUid);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_ChangeToRoot_ExpectFailure)
{
    STRICT_EXPECTED_CALL(geteuid()).SetReturn(2);
    STRICT_EXPECTED_CALL(seteuid(0)).SetReturn(-1);

    ProcessInfo info;
    bool result = ProcessInfoHandler_ChangeToRoot(&info);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_Reset_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(seteuid(2)).SetReturn(1);

    ProcessInfo info;
    info.effectiveUid = 2;
    info.wasSet = true;
    bool result = ProcessInfoHandler_Reset(&info);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_Reset_ExpectFailure)
{
    STRICT_EXPECTED_CALL(seteuid(2)).SetReturn(-1);

    ProcessInfo info;
    info.effectiveUid = 2;
    info.wasSet = true;
    bool result = ProcessInfoHandler_Reset(&info);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_Reset_IdWasNotSet_ExpectSuccess)
{
    ProcessInfo info;
    info.effectiveUid = 2;
    info.wasSet = false;
    bool result = ProcessInfoHandler_Reset(&info);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_SwitchRealAndEffectiveUsers_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(getuid()).SetReturn(1);
    STRICT_EXPECTED_CALL(geteuid()).SetReturn(2);
    STRICT_EXPECTED_CALL(setreuid(2, 1)).SetReturn(1);

    bool result = ProcessInfoHandler_SwitchRealAndEffectiveUsers();
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProcessInfoHandler_SwitchRealAndEffectiveUsers_ExpectFailure)
{
    STRICT_EXPECTED_CALL(getuid()).SetReturn(1);
    STRICT_EXPECTED_CALL(geteuid()).SetReturn(2);
    STRICT_EXPECTED_CALL(setreuid(2, 1)).SetReturn(-1);

    bool result = ProcessInfoHandler_SwitchRealAndEffectiveUsers();
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(process_info_handler_ut)
