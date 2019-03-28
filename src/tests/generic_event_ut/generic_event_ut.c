// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/uniqueid.h"
#include "internal/time_utils.h"
#include "json/json_object_writer.h"
#include "json/json_array_writer.h"
#undef ENABLE_MOCKS

#include "collectors/generic_event.h"
#include "message_schema_consts.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static const char localTimeStr[] = "2019-03-13T08:04:45+0300";
static const char utcTimeStr[] = "2019-03-13T08:04:45+0000";

static JsonObjectWriterHandle mockHandle = (JsonObjectWriterHandle)0x1;
static JsonArrayWriterHandle mockPayload = (JsonArrayWriterHandle) 0x10;
static const char eventCategory[] = "Abc";
static const char eventType[] = "myType";
static const char eventName[] = "Mnb Sdf";
static const char eventVersion[] = "1.2.0 rtyu";

static JsonWriterResult getSizeHookResult;
static uint getSizeHookElementCount;

bool Mocked_TimeUtils_GetTimeAsString(time_t* currentTime, char* output, uint32_t* outputSize) {
    memcpy(output, localTimeStr, sizeof(localTimeStr));
    *outputSize = sizeof(localTimeStr);
    return true;
}

bool Mocked_TimeUtils_GetLocalTimeAsUTCTimeAsString(time_t* currentLocalTime, char* output, uint32_t* outputSize) {
    memcpy(output, utcTimeStr, sizeof(utcTimeStr));
    *outputSize = sizeof(localTimeStr);
    return true;
}

JsonWriterResult getSizeHookFunction(JsonArrayWriterHandle writerHandle, uint* elementCount){
    *elementCount = getSizeHookElementCount;
    return getSizeHookResult;
}

static const char GUID[] = "df7e6af8-0c12-44db-a2b8-eaa19ea712af";
UNIQUEID_RESULT Mocked_UniqueId_Generate(char* uid, size_t len) {
    ASSERT_ARE_EQUAL(int, sizeof(GUID), len);
    memcpy(uid, GUID, len);
    return UNIQUEID_OK;
}

void GenericEvent_AddMetadataWithTimes_SetExpectedValues() {
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_CATEGORY_KEY, eventCategory)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_TYPE_KEY, eventType)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_NAME_KEY, eventName)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_PAYLOAD_SCHEMA_VERSION_KEY, eventVersion)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(UniqueId_Generate(IGNORED_PTR_ARG, 37));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_ID_KEY, GUID)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeAsString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_LOCAL_TIMESTAMP_KEY, localTimeStr)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(TimeUtils_GetLocalTimeAsUTCTimeAsString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_UTC_TIMESTAMP_KEY, utcTimeStr)).SetReturn(JSON_WRITER_OK);
}

void GenericEvent_AddMetadataWithTimes_SetFailExpectedValues() {
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_CATEGORY_KEY, eventCategory)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_TYPE_KEY, eventType)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_NAME_KEY, eventName)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_PAYLOAD_SCHEMA_VERSION_KEY, eventVersion)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(UniqueId_Generate(IGNORED_PTR_ARG, 37)).SetFailReturn(UNIQUEID_ERROR);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_ID_KEY, GUID)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeAsString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(false);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_LOCAL_TIMESTAMP_KEY, localTimeStr)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(TimeUtils_GetLocalTimeAsUTCTimeAsString(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(false);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(mockHandle, EVENT_UTC_TIMESTAMP_KEY, utcTimeStr)).SetFailReturn(!JSON_WRITER_OK);
}

void GenericEvent_AddPayload_SetExpectedValues(bool isEmpty){
    getSizeHookElementCount = isEmpty == true ? 0 : 5;
    getSizeHookResult = JSON_WRITER_OK;
    STRICT_EXPECTED_CALL(JsonArrayWriter_GetSize(mockPayload, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteBool(mockHandle, EVENT_IS_EMPTY_KEY, isEmpty)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteArray(mockHandle, PAYLOAD_KEY, mockPayload)).SetReturn(JSON_WRITER_OK);
}

void GenericEvent_AddPayload_SetFailExpectedValues(bool isEmpty){
    getSizeHookElementCount = isEmpty == true ? 0 : 5;
    getSizeHookResult = JSON_WRITER_EXCEPTION;
    STRICT_EXPECTED_CALL(JsonArrayWriter_GetSize(mockPayload, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteBool(mockHandle, EVENT_IS_EMPTY_KEY, isEmpty)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteArray(mockHandle, PAYLOAD_KEY, mockPayload)).SetFailReturn(!JSON_WRITER_OK);
}

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(generic_event_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(UNIQUEID_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(time_t, int);
    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();

    REGISTER_GLOBAL_MOCK_HOOK(UniqueId_Generate, Mocked_UniqueId_Generate);
    REGISTER_GLOBAL_MOCK_HOOK(TimeUtils_GetTimeAsString, Mocked_TimeUtils_GetTimeAsString);
    REGISTER_GLOBAL_MOCK_HOOK(TimeUtils_GetLocalTimeAsUTCTimeAsString, Mocked_TimeUtils_GetLocalTimeAsUTCTimeAsString);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_GetSize, getSizeHookFunction);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(UniqueId_Generate, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TimeUtils_GetTimeAsString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TimeUtils_GetLocalTimeAsUTCTimeAsString, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_GetSize, NULL);

    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(GenericEvent_AddMetadata_ExpectSuccess)
{
    time_t mockedTime;
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(mockedTime);
    GenericEvent_AddMetadataWithTimes_SetExpectedValues();

    EventCollectorResult result = GenericEvent_AddMetadata(mockHandle, eventCategory, eventName, eventType, eventVersion);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericEvent_AddMetadata_ExpectFailure)
{
    umock_c_negative_tests_init();

    time_t mockedTime;
    STRICT_EXPECTED_CALL(TimeUtils_GetCurrentTime()).SetReturn(mockedTime);
    GenericEvent_AddMetadataWithTimes_SetFailExpectedValues();

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 0) {
            continue;
        }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        EventCollectorResult result = GenericEvent_AddMetadata(mockHandle, eventCategory, eventName, eventType, eventVersion);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(GenericEvent_AddMetadataWithTimes_ExpectSuccess)
{
    time_t eventTime;
    GenericEvent_AddMetadataWithTimes_SetExpectedValues();

    EventCollectorResult result = GenericEvent_AddMetadataWithTimes(mockHandle, eventCategory, eventName, eventType, eventVersion, &eventTime);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericEvent_AddMetadataWithTimes_ExpectFailure)
{
    umock_c_negative_tests_init();

    time_t eventTime;
    GenericEvent_AddMetadataWithTimes_SetFailExpectedValues();

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        EventCollectorResult result = GenericEvent_AddMetadataWithTimes(mockHandle, eventCategory, eventName, eventType, eventVersion, &eventTime);
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }
    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(GenericEvent_AddEmptyPayload_ExpectSuccess)
{
    GenericEvent_AddPayload_SetExpectedValues(true);

    EventCollectorResult result = GenericEvent_AddPayload(mockHandle, mockPayload);
    
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericEvent_AddPayload_ExpectSuccess)
{
    GenericEvent_AddPayload_SetExpectedValues(false);

    EventCollectorResult result = GenericEvent_AddPayload(mockHandle, mockPayload);
    
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(GenericEvent_AddEmptyPayload_ExpectFail)
{
    umock_c_negative_tests_init();

    GenericEvent_AddPayload_SetFailExpectedValues(true);

    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        if (i == 0){
            getSizeHookResult = JSON_WRITER_EXCEPTION;
        } else {
            getSizeHookResult = JSON_WRITER_OK;
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);
        }

        EventCollectorResult result = GenericEvent_AddPayload(mockHandle, mockPayload);
        
        ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_EXCEPTION, result);
    }

    umock_c_negative_tests_deinit();
}


END_TEST_SUITE(generic_event_ut)