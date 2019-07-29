// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "collectors/generic_event.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "os_utils/groups_iterator.h"
#include "os_utils/users_iterator.h"
#include "synchronized_queue.h"
#include "utils.h"
#undef ENABLE_MOCKS

#include "collectors/local_users_collector.h"
#include "message_schema_consts.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

JsonWriterResult Mocked_JsonObjectWriter_Init(JsonObjectWriterHandle* writer) {
    *writer = (JsonObjectWriterHandle)0x5;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init(JsonArrayWriterHandle* writer) {
    *writer = (JsonArrayWriterHandle)0x6;
    return JSON_WRITER_OK;
}

UserIteratorResults Mocked_UsersIterator_Init(UsersIteratorHandle* iterator) {
    *iterator = (UsersIteratorHandle)0x7;
    return USER_ITERATOR_OK;
}

UserIteratorResults Mocked_UsersIterator_CreateGroupsIterator(UsersIteratorHandle iterator, GroupsIteratorHandle* groupsIterator) {
    *groupsIterator = (GroupsIteratorHandle)0x8;
    return USER_ITERATOR_OK;
}

BEGIN_TEST_SUITE(local_users_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(UserIteratorResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(UsersIteratorHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(int32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(GroupsIteratorHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init);
    REGISTER_GLOBAL_MOCK_HOOK(UsersIterator_Init, Mocked_UsersIterator_Init);
    REGISTER_GLOBAL_MOCK_HOOK(UsersIterator_CreateGroupsIterator, Mocked_UsersIterator_CreateGroupsIterator);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(UsersIterator_Init, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(UsersIterator_CreateGroupsIterator, NULL);

    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(LocalUsersCollector_GetEvents_ExpectSuccess)
{
    SyncQueue mockedQueue;
    const char* expectedUsername = "asdfg";
    const char* expectedGroupName1 = "group1";
    const char* expectedGroupName2 = "group2";
    
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_PERIODIC_CATEGORY, LOCAL_USERS_NAME, EVENT_TYPE_SECURITY_VALUE, LOCAL_USERS_PAYLOAD_SCHEMA_VERSION)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(UsersIterator_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(UsersIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(USER_ITERATOR_HAS_NEXT);

    // adds user
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(UsersIterator_GetUsername(IGNORED_PTR_ARG)).SetReturn(expectedUsername);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, LOCAL_USERS_USER_NAME_KEY, expectedUsername)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(UsersIterator_GetUserId(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(USER_ITERATOR_OK).CopyOutArgumentBuffer_outBuffer("5", 1);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, LOCAL_USERS_USER_ID_KEY, "5")).SetReturn(JSON_WRITER_OK);

    //adds groups for user
    STRICT_EXPECTED_CALL(UsersIterator_CreateGroupsIterator(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GroupsIterator_GetGroupsCount(IGNORED_PTR_ARG)).SetReturn(2);
    
    // calculate size - group 1
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_Next(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_GetName(IGNORED_PTR_ARG)).SetReturn(expectedGroupName1);
    STRICT_EXPECTED_CALL(GroupsIterator_GetId(IGNORED_PTR_ARG)).SetReturn(78);
    STRICT_EXPECTED_CALL(Utils_IntegerToString(78, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);

    // calculate size - group 2
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_Next(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_GetName(IGNORED_PTR_ARG)).SetReturn(expectedGroupName2);
    STRICT_EXPECTED_CALL(GroupsIterator_GetId(IGNORED_PTR_ARG)).SetReturn(5);
    STRICT_EXPECTED_CALL(Utils_IntegerToString(5, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);

    // end
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(false);

    STRICT_EXPECTED_CALL(GroupsIterator_Reset(IGNORED_PTR_ARG));

    // group 1
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_Next(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_GetName(IGNORED_PTR_ARG)).SetReturn(expectedGroupName1);
    STRICT_EXPECTED_CALL(GroupsIterator_GetId(IGNORED_PTR_ARG)).SetReturn(78);
    STRICT_EXPECTED_CALL(Utils_IntegerToString(78, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);

    // group 2
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_Next(IGNORED_PTR_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(GroupsIterator_GetName(IGNORED_PTR_ARG)).SetReturn(expectedGroupName2);
    STRICT_EXPECTED_CALL(GroupsIterator_GetId(IGNORED_PTR_ARG)).SetReturn(5);
    STRICT_EXPECTED_CALL(Utils_IntegerToString(5, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(true);

    // end of groups
    STRICT_EXPECTED_CALL(GroupsIterator_HasNext(IGNORED_PTR_ARG)).SetReturn(false);

    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, LOCAL_USERS_GROUP_NAMES_KEY, "group1;group2")).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, LOCAL_USERS_GROUP_IDS_KEY, ";")).SetReturn(JSON_WRITER_OK);

    STRICT_EXPECTED_CALL(GroupsIterator_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(UsersIterator_GetNext(IGNORED_PTR_ARG)).SetReturn(USER_ITERATOR_STOP);
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);

    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);

    STRICT_EXPECTED_CALL(UsersIterator_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));

    EventCollectorResult result = LocalUsersCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(local_users_collector_ut)
