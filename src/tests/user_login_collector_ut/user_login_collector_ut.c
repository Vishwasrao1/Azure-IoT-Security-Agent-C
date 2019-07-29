// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "collectors/generic_event.h"
#include "os_utils/linux/audit/audit_search.h"
#include "collectors/linux/generic_audit_event.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "synchronized_queue.h"
#undef ENABLE_MOCKS

#include "collectors/user_login_collector.h"
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

static char* MOCKED_REAL_REMOTE_ADDR = "1.2.3.4";
static char* MOCKED_UNKNOWN_REMOTE_ADDR = "?";
static char* MOCKED_CONNECTION_RESAULT = "success";
static char* MOCKED_REMOTE_ADDRESS;

AuditSearchResultValues Mocked_AuditSearch_ReadString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    if (strcmp(fieldName, "addr") == 0) {
        *output = MOCKED_REMOTE_ADDRESS;
        return AUDIT_SEARCH_OK;
    } else if (strcmp(fieldName, "res") == 0) {
        *output = MOCKED_CONNECTION_RESAULT;
        return AUDIT_SEARCH_OK;
    }
    ASSERT_FAIL("Unkown value to Mocked_AuditSearch_ReadString");
}

void ValidateGetEvents(SyncQueue* mockedQueue, bool shouldValidateRemoteAddress, bool shouldValiadteOptional) {
    STRICT_EXPECTED_CALL(AuditSearch_InitMultipleSearchCriteria(IGNORED_PTR_ARG, AUDIT_SEARCH_CRITERIA_TYPE, IGNORED_PTR_ARG, 2, "/var/tmp/userLoginCheckpoint")).SetReturn(AUDIT_SEARCH_OK);
    STRICT_EXPECTED_CALL(AuditSearch_GetNext(IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_HAS_MORE_DATA);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(AuditSearch_GetEventTime(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadataWithTimes(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, USER_LOGIN_NAME, EVENT_TYPE_SECURITY_VALUE, USER_LOGIN_PAYLOAD_SCHEMA_VERSION, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);

    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleIntValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "pid", USER_LOGIN_PROCESS_ID_KEY, false)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleStringValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "id", USER_LOGIN_USER_ID_KEY, true)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleInterpretStringValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "acct", USER_LOGIN_USERNAME_KEY, true)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleInterpretStringValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "exe", USER_LOGIN_EXECUTABLE_KEY, false)).SetReturn(EVENT_COLLECTOR_OK);

    if (shouldValiadteOptional) {
        STRICT_EXPECTED_CALL(AuditSearch_ReadString(IGNORED_PTR_ARG, "addr", IGNORED_PTR_ARG));
        if (shouldValidateRemoteAddress) {
            STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, USER_LOGIN_REMOTE_ADDRESS_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
        }
    } else {
        STRICT_EXPECTED_CALL(AuditSearch_ReadString(IGNORED_PTR_ARG, "addr", IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    }
    
    STRICT_EXPECTED_CALL(AuditSearch_ReadString(IGNORED_PTR_ARG, "res", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, USER_LOGIN_RESULT_KEY, IGNORED_NUM_ARG)).SetReturn(JSON_WRITER_OK);
 
    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleStringValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "op", USER_LOGIN_OPERATION_KEY, true)).SetReturn(EVENT_COLLECTOR_OK);
    
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);

    STRICT_EXPECTED_CALL(AuditSearch_GetNext(IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_NO_MORE_DATA);
    STRICT_EXPECTED_CALL(AuditSearch_SetCheckpoint(IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_OK);
    STRICT_EXPECTED_CALL(AuditSearch_Deinit(IGNORED_PTR_ARG)); 
}

BEGIN_TEST_SUITE(user_login_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(AuditSearchResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(AuditSearchCriteria, int);

    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadString, Mocked_AuditSearch_ReadString);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadString, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
    MOCKED_REMOTE_ADDRESS = MOCKED_REAL_REMOTE_ADDR;
}

TEST_FUNCTION(UserLoginCollector_GetEvents_ExpectSuccess)
{
    SyncQueue mockedQueue;
    ValidateGetEvents(&mockedQueue, true, true);
    bool result = UserLoginCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UserLoginCollector_GetEvents_UnknownRemoteAddress_ExpectSuccess)
{
    SyncQueue mockedQueue;
    ValidateGetEvents(&mockedQueue, false, true);    
    MOCKED_REMOTE_ADDRESS = MOCKED_UNKNOWN_REMOTE_ADDR;
    bool result = UserLoginCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


TEST_FUNCTION(UserLoginCollector_GetEvents_NoOptionalFields_ExpectSuccess)
{
    SyncQueue mockedQueue;
    ValidateGetEvents(&mockedQueue, false, false);
    bool result = UserLoginCollector_GetEvents(&mockedQueue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(UserLoginCollector_GetEvents_ExpectFailure)
{

    SyncQueue mockedQueue;
    umock_c_negative_tests_init();

    const char* loginTypes[] = {"USER_LOGIN", "USER_AUTH"}; 
    STRICT_EXPECTED_CALL(AuditSearch_InitMultipleSearchCriteria(IGNORED_PTR_ARG, AUDIT_SEARCH_CRITERIA_TYPE, loginTypes, 2, "/var/tmp/userLoginCheckpoint")).SetFailReturn(!AUDIT_SEARCH_OK);
    // This does not have a fail valie since it is importatn for the flow. Skip this on negative tests
    STRICT_EXPECTED_CALL(AuditSearch_GetNext(IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_HAS_MORE_DATA);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(AuditSearch_GetEventTime(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!AUDIT_SEARCH_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadataWithTimes(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, USER_LOGIN_NAME, EVENT_TYPE_SECURITY_VALUE, USER_LOGIN_PAYLOAD_SCHEMA_VERSION, IGNORED_PTR_ARG)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);

    // we can't have a fail return to all fields since the negative tests count on the fact that 0 is the value of success
    STRICT_EXPECTED_CALL(GenericAuditEvent_HandleIntValue(IGNORED_PTR_ARG, IGNORED_PTR_ARG, "pid", USER_LOGIN_PROCESS_ID_KEY, false)).SetFailReturn(EVENT_COLLECTOR_EXCEPTION);

    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(&mockedQueue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(!QUEUE_OK);

    // This does not have a fail valie since it is importatn for the flow. Skip this on negative tests
    STRICT_EXPECTED_CALL(AuditSearch_GetNext(IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_NO_MORE_DATA);
    STRICT_EXPECTED_CALL(AuditSearch_SetCheckpoint(IGNORED_PTR_ARG)).SetFailReturn(!AUDIT_SEARCH_OK);
    // This does not have a fail valie since it is importatn for the flow. Skip this on negative tests
    STRICT_EXPECTED_CALL(AuditSearch_Deinit(IGNORED_PTR_ARG)); 

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        if (i == 1 || i == 13 || i == 15) {
            // skip on the non-SetFailReturn calls
            continue;
        }

        EventCollectorResult result = UserLoginCollector_GetEvents(&mockedQueue);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }
    
    umock_c_negative_tests_deinit();

}

END_TEST_SUITE(user_login_collector_ut)
