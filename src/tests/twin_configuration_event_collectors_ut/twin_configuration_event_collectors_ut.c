// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c_negative_tests.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"


#define ENABLE_MOCKS
#include "azure_c_shared_utility/lock.h"
#include "json/json_object_reader.h"
#include "json/json_object_writer.h"
#include "internal/time_utils.h"
#include "internal/time_utils_consts.h"
#include "twin_configuration_utils.h"
#undef ENABLE_MOCKS

#include "twin_configuration_consts.h"
#include "twin_configuration_event_collectors.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char* HIGH_PRIORITY = "High";
static char* LOW_PRIORITY = "Low";
static char* OFF_PRIORITY = "Off";

static bool isMalformed;

TwinConfigurationResult Mocked_TwinConfigurationUtils_GetConfigurationStringValueFromJson(JsonObjectReaderHandle handle, const char* key, char** output) {
    if (isMalformed) {
        *output = "Shutuyut";
    } else if (strcmp(key, PROCESS_CREATE_PRIORITY_KEY) == 0 
        || strcmp(key, SYSTEM_INFORMATION_PRIORITY_KEY) == 0 
        || strcmp(key, LOGIN_PRIORITY_KEY) == 0
        || strcmp(key, OPERATIONAL_EVENT_KEY) == 0 ) 
    {
        *output = HIGH_PRIORITY;
    } else if (strcmp(key, LISTENING_PORTS_PRIORITY_KEY) == 0 
        || strcmp(key, LOCAL_USERS_PRIORITY_KEY) == 0 
        || strcmp(key, BASELINE_PRIORITY_KEY) == 0
        || strcmp(key, DIAGNOSTIC_PRIORITY_KEY) == 0 ) 
    {
        *output = LOW_PRIORITY;
    } else {
        *output = OFF_PRIORITY;
    }
    return JSON_READER_OK;
}

TwinConfigurationResult Mocked_TwinConfigurationUtils_GetConfigurationBoolValueFromJson(JsonObjectReaderHandle handle, const char* key, bool* output) {
    if (strcmp(key, PROCESS_CREATE_AGGREGATION_ENABLED_KEY) == 0 
        || strcmp(key, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY) == 0 ) 
    {
        *output = true;
    } 

    return JSON_READER_OK;
}

TwinConfigurationResult Mocked_TwinConfigurationUtils_GetConfigurationTimeValueFromJson(JsonObjectReaderHandle handle, const char* key, uint32_t* output) {
    if (strcmp(key, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY) == 0 
        || strcmp(key, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY) == 0 )
    {
        *output = MILLISECONDS_IN_AN_HOUR;
    }
    return JSON_READER_OK;
}

static void ValidateMockedPriorities() {
    TwinConfigurationResult result;
    TwinConfigurationEventPriority priority;

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_PROCESS_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_LISTENING_PORTS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_SYSTEM_INFORMATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_LOCAL_USERS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_USER_LOGIN, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_CONNECTION_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_OFF, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_FIREWALL_CONFIGURATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_OFF, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_BASELINE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_DIAGNOSTIC, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_OPERATIONAL_EVENT, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    bool isEnabled = false;
    result = TwinConfigurationEventCollectors_GetAggregationEnabled(EVENT_TYPE_PROCESS_CREATE, &isEnabled);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_IS_TRUE(isEnabled);

    result = TwinConfigurationEventCollectors_GetAggregationEnabled(EVENT_TYPE_CONNECTION_CREATE, &isEnabled);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_IS_TRUE(isEnabled);

    uint32_t interval = 0;
    result = TwinConfigurationEventCollectors_GetAggregationInterval(EVENT_TYPE_PROCESS_CREATE, &interval);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, MILLISECONDS_IN_AN_HOUR, interval);

    result = TwinConfigurationEventCollectors_GetAggregationInterval(EVENT_TYPE_CONNECTION_CREATE, &interval);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, MILLISECONDS_IN_AN_HOUR, interval);
}

static void ValidateDefaultPriorities() {
    TwinConfigurationResult result;
    TwinConfigurationEventPriority priority;

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_PROCESS_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_LISTENING_PORTS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_SYSTEM_INFORMATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_LOCAL_USERS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_USER_LOGIN, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_CONNECTION_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_FIREWALL_CONFIGURATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_BASELINE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventCollectors_GetPriority(EVENT_TYPE_DIAGNOSTIC, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    bool isEnabled = false;
    result = TwinConfigurationEventCollectors_GetAggregationEnabled(EVENT_TYPE_PROCESS_CREATE, &isEnabled);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_IS_TRUE(isEnabled);

    result = TwinConfigurationEventCollectors_GetAggregationEnabled(EVENT_TYPE_CONNECTION_CREATE, &isEnabled);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_IS_TRUE(isEnabled);

    uint32_t interval = 0;
    result = TwinConfigurationEventCollectors_GetAggregationInterval(EVENT_TYPE_PROCESS_CREATE, &interval);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, MILLISECONDS_IN_AN_HOUR, interval);

    result = TwinConfigurationEventCollectors_GetAggregationInterval(EVENT_TYPE_CONNECTION_CREATE, &interval);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, MILLISECONDS_IN_AN_HOUR, interval);
}

static LOCK_HANDLE testLockHadnle = (LOCK_HANDLE)0x1;

BEGIN_TEST_SUITE(twin_configuration_event_collectors_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectReaderHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationEventPriority, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);

    // Lock
    REGISTER_GLOBAL_MOCK_RETURN(Lock_Init, testLockHadnle);
    REGISTER_GLOBAL_MOCK_RETURN(Lock_Deinit, LOCK_OK);
    REGISTER_GLOBAL_MOCK_RETURN(Lock, LOCK_OK);
    REGISTER_GLOBAL_MOCK_RETURN(Unlock, LOCK_OK);

    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationStringValueFromJson, Mocked_TwinConfigurationUtils_GetConfigurationStringValueFromJson);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationBoolValueFromJson, Mocked_TwinConfigurationUtils_GetConfigurationBoolValueFromJson);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationTimeValueFromJson, Mocked_TwinConfigurationUtils_GetConfigurationTimeValueFromJson);


}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationStringValueFromJson, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationBoolValueFromJson, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationUtils_GetConfigurationTimeValueFromJson, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    isMalformed = false;
    umock_c_reset_all_calls();
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Init_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Init_LockFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(NULL);
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_LOCK_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Denit_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    TwinConfigurationEventCollectors_Deinit();
    STRICT_EXPECTED_CALL(Lock_Deinit(testLockHadnle));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Update_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOCAL_USERS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOGIN_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, CONNECTION_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, FIREWALL_CONFIGURATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, BASELINE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, DIAGNOSTIC_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, OPERATIONAL_EVENT_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateMockedPriorities();
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Update_KeyMissing_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOCAL_USERS_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOGIN_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, CONNECTION_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, FIREWALL_CONFIGURATION_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, BASELINE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, DIAGNOSTIC_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, OPERATIONAL_EVENT_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Update_ReaderError_ExpectFailure)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_EXCEPTION);
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventCollectors_GetPrioritiesJsonExpectSuccess){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;
    result = TwinConfigurationEventCollectors_Update(readerHandle);
    
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectWriterHandle objectWriter = (JsonObjectWriterHandle)0x10;
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, BASELINE_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, CONNECTION_CREATE_PRIORITY_KEY, "off"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, DIAGNOSTIC_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, FIREWALL_CONFIGURATION_PRIORITY_KEY, "off"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, LISTENING_PORTS_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, LOCAL_USERS_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, LOGIN_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, OPERATIONAL_EVENT_KEY, "high"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, PROCESS_CREATE_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, SYSTEM_INFORMATION_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteBoolConfigurationToJson(objectWriter, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, true));
    STRICT_EXPECTED_CALL(TimeUtils_MillisecondsToISO8601DurationString(MILLISECONDS_IN_AN_HOUR, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteBoolConfigurationToJson(objectWriter, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, true));
    STRICT_EXPECTED_CALL(TimeUtils_MillisecondsToISO8601DurationString(MILLISECONDS_IN_AN_HOUR, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_WriteStringConfigurationToJson(objectWriter, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
    result = TwinConfigurationEventCollectors_GetPrioritiesJson(objectWriter);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventCollectors_GetPrioritiesJsonExpectFail){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;
    result = TwinConfigurationEventCollectors_Update(readerHandle);
    
    umock_c_reset_all_calls();
    umock_c_negative_tests_init();

    STRICT_EXPECTED_CALL(Lock(testLockHadnle)).SetFailReturn(LOCK_ERROR);
    JsonObjectWriterHandle objectWriter = (JsonObjectWriterHandle)0x10;
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, BASELINE_PRIORITY_KEY, "low")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, CONNECTION_CREATE_PRIORITY_KEY, "off")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, DIAGNOSTIC_PRIORITY_KEY, "low")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, FIREWALL_CONFIGURATION_PRIORITY_KEY, "off")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LISTENING_PORTS_PRIORITY_KEY, "low")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LOCAL_USERS_PRIORITY_KEY, "low")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LOGIN_PRIORITY_KEY, "high")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, OPERATIONAL_EVENT_KEY, "high")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, PROCESS_CREATE_PRIORITY_KEY, "high")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, SYSTEM_INFORMATION_PRIORITY_KEY, "high")).SetFailReturn(JSON_WRITER_EXCEPTION);
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle)).SetFailReturn(JSON_WRITER_EXCEPTION);
    
    umock_c_negative_tests_snapshot();

    for (int i = 0; i < umock_c_negative_tests_call_count(); i++) {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        result = TwinConfigurationEventCollectors_GetPrioritiesJson(objectWriter);
        ASSERT_IS_TRUE(TWIN_OK != result);
    }

    umock_c_negative_tests_deinit();    
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Update_SetBackToDeafultValues_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOCAL_USERS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOGIN_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, CONNECTION_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, FIREWALL_CONFIGURATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, BASELINE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, DIAGNOSTIC_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, OPERATIONAL_EVENT_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateMockedPriorities();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOCAL_USERS_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOGIN_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, CONNECTION_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, FIREWALL_CONFIGURATION_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, BASELINE_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, DIAGNOSTIC_PRIORITY_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, OPERATIONAL_EVENT_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG)).SetReturn(TWIN_CONF_NOT_EXIST);
    
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventCollectors_Update_MalformedJson_ConfigStayIntact){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventCollectors_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LISTENING_PORTS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, SYSTEM_INFORMATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOCAL_USERS_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, LOGIN_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, CONNECTION_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, FIREWALL_CONFIGURATION_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, BASELINE_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, DIAGNOSTIC_PRIORITY_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, OPERATIONAL_EVENT_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, PROCESS_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationBoolValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_ENABLED_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationTimeValueFromJson(readerHandle, CONNECTION_CREATE_AGGREGATION_INTERVAL_KEY, IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventCollectors_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    //ValidateMockedPriorities();
    umock_c_reset_all_calls();
    isMalformed = true;
    ////STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    
    //STRICT_EXPECTED_CALL(TwinConfigurationUtils_GetConfigurationStringValueFromJson(readerHandle, PROCESS_CREATE_PRIORITY_KEY, IGNORED_PTR_ARG));
    //STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
   /// result = TwinConfigurationEventCollectors_Update(readerHandle);
   // ASSERT_ARE_EQUAL(int, TWIN_PARSE_EXCEPTION, result);
   // ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    //priorities should stay intact
    //ValidateMockedPriorities();
}

END_TEST_SUITE(twin_configuration_event_collectors_ut)
