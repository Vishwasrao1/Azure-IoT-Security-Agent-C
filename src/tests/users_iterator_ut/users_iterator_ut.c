// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

#define ENABLE_MOCKS
#include "os_utils/groups_iterator.h"
#include "os_users_mock.h"
#undef ENABLE_MOCKS

#include "os_utils/users_iterator.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(users_iterator_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(int32_t, unsigned int);
    REGISTER_UMOCK_ALIAS_TYPE(UserIteratorResults, int);
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

TEST_FUNCTION(UsersIterator_Init_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;

    STRICT_EXPECTED_CALL(setpwent());

    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

TEST_FUNCTION(UsersIterator_Deinit_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;

    STRICT_EXPECTED_CALL(setpwent());

    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);

    STRICT_EXPECTED_CALL(endpwent());
    UsersIterator_Deinit(iteratorHandle);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UsersIterator_GetNext_HasNext_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;

    STRICT_EXPECTED_CALL(setpwent());
    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    struct passwd currentUser;
    STRICT_EXPECTED_CALL(getpwent()).SetReturn(&currentUser);

    result = UsersIterator_GetNext(iteratorHandle);

    ASSERT_ARE_EQUAL(int, USER_ITERATOR_HAS_NEXT, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

TEST_FUNCTION(UsersIterator_GetNext_NoMoreUsers_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;

    STRICT_EXPECTED_CALL(setpwent());
    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    STRICT_EXPECTED_CALL(getpwent()).SetReturn(NULL);
    
    result = UsersIterator_GetNext(iteratorHandle);

    ASSERT_ARE_EQUAL(int, USER_ITERATOR_STOP, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

TEST_FUNCTION(UsersIterator_GetUsernameAndId_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;
    int32_t currentId = 11;
    const char currentUsername[] = "useruser";
    struct passwd currentUser;
    currentUser.pw_uid = currentId;
    currentUser.pw_name = (char*)currentUsername;

    STRICT_EXPECTED_CALL(setpwent());
    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    STRICT_EXPECTED_CALL(getpwent()).SetReturn(&currentUser);

    result = UsersIterator_GetNext(iteratorHandle);

    ASSERT_ARE_EQUAL(int, USER_ITERATOR_HAS_NEXT, result);
    ASSERT_ARE_EQUAL(char_ptr, currentUsername, UsersIterator_GetUsername(iteratorHandle));

    char userIdBuffer[3] = {0};
    int32_t userIdBufferSize = sizeof(userIdBuffer);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, UsersIterator_GetUserId(iteratorHandle, userIdBuffer, &userIdBufferSize));
    ASSERT_IS_TRUE(strcmp(userIdBuffer, "11") == 0);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

TEST_FUNCTION(UsersIterator_CreateGroupsIterator_ExpectSuccess)
{
    UsersIteratorHandle iteratorHandle = NULL;
    GroupsIteratorHandle groupIteratorHandle = NULL;
  
    STRICT_EXPECTED_CALL(setpwent());
    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    STRICT_EXPECTED_CALL(GroupsIterator_Init(&groupIteratorHandle, IGNORED_PTR_ARG)).SetReturn(true);

    result = UsersIterator_CreateGroupsIterator(iteratorHandle, &groupIteratorHandle);
    
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

TEST_FUNCTION(UsersIterator_CreateGroupsIterator_ExpectFailure)
{
    UsersIteratorHandle iteratorHandle = NULL;
    GroupsIteratorHandle groupIteratorHandle = NULL;
  
    STRICT_EXPECTED_CALL(setpwent());
    UserIteratorResults result = UsersIterator_Init(&iteratorHandle);
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_OK, result);
    STRICT_EXPECTED_CALL(GroupsIterator_Init(&groupIteratorHandle, IGNORED_PTR_ARG)).SetReturn(false);

    result = UsersIterator_CreateGroupsIterator(iteratorHandle, &groupIteratorHandle);
    
    ASSERT_ARE_EQUAL(int, USER_ITERATOR_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    UsersIterator_Deinit(iteratorHandle);
}

END_TEST_SUITE(users_iterator_ut)
