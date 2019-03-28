// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_groups_mock.h"
#undef ENABLE_MOCKS

#include "os_utils/groups_iterator.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char MOCKED_GROUP_NAME[] = "groupy";
static const __gid_t MOCKED_GROUP_ID = 23;

static const struct group MOCKED_GROUP = {MOCKED_GROUP_NAME,
                                          NULL,
                                          23,
                                          NULL};

int Mocked_getgrouplist(const char* user, gid_t group, gid_t* groups, int* ngroups) {
    *ngroups = 1;
    return -1;
}


BEGIN_TEST_SUITE(groups_iterator_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(gid_t, int);
    REGISTER_GLOBAL_MOCK_HOOK(getgrouplist, Mocked_getgrouplist);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(getgrouplist, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(GroupsIterator_Init_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_HasNext_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    result = GroupsIterator_HasNext(handle);
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_Next_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    struct group* groupPtr = (struct group*)&MOCKED_GROUP;
    STRICT_EXPECTED_CALL(getgrgid(IGNORED_NUM_ARG)).SetReturn(groupPtr);
    result = GroupsIterator_Next(handle);
    ASSERT_IS_TRUE(result);
    const char* currentName = GroupsIterator_GetName(handle);
    ASSERT_ARE_EQUAL(char_ptr, MOCKED_GROUP_NAME, currentName);
    uint32_t currentId = GroupsIterator_GetId(handle);
    ASSERT_ARE_EQUAL(int, MOCKED_GROUP_ID, currentId);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_NextFailed_ExpectFailure)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(getgrgid(IGNORED_NUM_ARG)).SetReturn(NULL);
    result = GroupsIterator_Next(handle);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_NextHasNext_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    struct group* groupPtr = (struct group*)&MOCKED_GROUP;
    STRICT_EXPECTED_CALL(getgrgid(IGNORED_NUM_ARG)).SetReturn(groupPtr);
    result = GroupsIterator_Next(handle);
    ASSERT_IS_TRUE(result);
    result = GroupsIterator_HasNext(handle);
    ASSERT_IS_FALSE(result);

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_NextHasNextRestHasNext_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    struct group* groupPtr = (struct group*)&MOCKED_GROUP;
    STRICT_EXPECTED_CALL(getgrgid(IGNORED_NUM_ARG)).SetReturn(groupPtr);
    result = GroupsIterator_Next(handle);
    ASSERT_IS_TRUE(result);
    result = GroupsIterator_HasNext(handle);
    ASSERT_IS_FALSE(result);
    GroupsIterator_Reset(handle);
    result = GroupsIterator_HasNext(handle);
    ASSERT_IS_TRUE(result);

    GroupsIterator_Deinit(handle);
}

TEST_FUNCTION(GroupsIterator_GetGroupCount_ExpectSuccess)
{
    struct passwd user;
    user.pw_name = "aaa";
    user.pw_gid = 7;
    GroupsIteratorHandle handle;

    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(getgrouplist("aaa", 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(-1);

    bool result = GroupsIterator_Init(&handle, &user);
    ASSERT_IS_TRUE(result);

    uint32_t groupCount = GroupsIterator_GetGroupsCount(handle);
    ASSERT_ARE_EQUAL(int, 1, groupCount);

    GroupsIterator_Deinit(handle);
}

END_TEST_SUITE(groups_iterator_ut)
