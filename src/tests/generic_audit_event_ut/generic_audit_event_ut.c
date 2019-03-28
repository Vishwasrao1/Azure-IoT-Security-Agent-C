// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_utils/linux/audit/audit_search.h"
#include "json/json_object_writer.h"
#undef ENABLE_MOCKS

#include "collectors/linux/generic_audit_event.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char* MOCKED_STRING_VALUE = "I'm sorry, the old Taylor can't come to the phone right now";
static int MOCKED_INT_VALUE = 29;
static JsonObjectWriterHandle MOCKED_JSON_WRITER = (JsonObjectWriterHandle)0x1;
static AuditSearch MOCKED_AUDIT_SEARCH;
static const char* READ_FIELD_NAME = "field";
static const char* WRITE_JSON_KEY = "json key";

AuditSearchResultValues Mocked_AuditSearch_ReadString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    *output = MOCKED_STRING_VALUE;
    return AUDIT_SEARCH_OK;
}

AuditSearchResultValues Mocked_AuditSearch_InterpretString(AuditSearch* auditSearch, const char* fieldName, const char** output) {
    *output = MOCKED_STRING_VALUE;
    return AUDIT_SEARCH_OK;
}
AuditSearchResultValues Mocked_AuditSearch_ReadInt(AuditSearch* auditSearch, const char* fieldName, int* output) {
    *output = MOCKED_INT_VALUE;
    return AUDIT_SEARCH_OK;
}

BEGIN_TEST_SUITE(generic_audit_event_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(AuditSearchResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*)

    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadString, Mocked_AuditSearch_ReadString);
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_InterpretString, Mocked_AuditSearch_InterpretString);
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadInt, Mocked_AuditSearch_ReadInt);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_InterpretString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(AuditSearch_ReadInt, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(GenericAuditEvent_HandleStringValue_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_STRING_VALUE)).SetReturn(JSON_WRITER_OK);

    EventCollectorResult result = GenericAuditEvent_HandleStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleStringValue_ReadFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleStringValue_WriteFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_STRING_VALUE)).SetReturn(JSON_WRITER_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleStringValue_FieldDoesNotExist_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    EventCollectorResult result = GenericAuditEvent_HandleStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    STRICT_EXPECTED_CALL(AuditSearch_ReadString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    result = GenericAuditEvent_HandleStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, false);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleInterpretStringValue_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_InterpretString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_STRING_VALUE)).SetReturn(JSON_WRITER_OK);

    EventCollectorResult result = GenericAuditEvent_HandleInterpretStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleInterpretStringValue_ReadFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_InterpretString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleInterpretStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleInterpretStringValue_WriteFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_InterpretString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_STRING_VALUE)).SetReturn(JSON_WRITER_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleInterpretStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleInterpretStringValue_FieldDoesNotExist_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_InterpretString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    EventCollectorResult result = GenericAuditEvent_HandleInterpretStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    STRICT_EXPECTED_CALL(AuditSearch_InterpretString(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    result = GenericAuditEvent_HandleInterpretStringValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, false);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleIntValue_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadInt(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_INT_VALUE)).SetReturn(JSON_WRITER_OK);

    EventCollectorResult result = GenericAuditEvent_HandleIntValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleIntValue_ReadFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadInt(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleIntValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleIntValue_WriteFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadInt(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteInt(MOCKED_JSON_WRITER, WRITE_JSON_KEY, MOCKED_INT_VALUE)).SetReturn(JSON_WRITER_EXCEPTION);

    EventCollectorResult result = GenericAuditEvent_HandleIntValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericAuditEvent_HandleIntValue_FieldDoesNotExist_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(AuditSearch_ReadInt(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    EventCollectorResult result = GenericAuditEvent_HandleIntValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, true);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    STRICT_EXPECTED_CALL(AuditSearch_ReadInt(&MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, IGNORED_PTR_ARG)).SetReturn(AUDIT_SEARCH_FIELD_DOES_NOT_EXIST);
    result = GenericAuditEvent_HandleIntValue(MOCKED_JSON_WRITER, &MOCKED_AUDIT_SEARCH, READ_FIELD_NAME, WRITE_JSON_KEY, false);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_RECORD_HAS_ERRORS, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(generic_audit_event_ut)
