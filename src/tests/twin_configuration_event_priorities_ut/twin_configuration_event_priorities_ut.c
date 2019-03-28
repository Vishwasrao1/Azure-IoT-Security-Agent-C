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
#undef ENABLE_MOCKS

#include "twin_configuration_consts.h"
#include "twin_configuration_event_priorities.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static char* HIGH_PRIORITY = "High";
static char* LOW_PRIORITY = "Low";
static char* OFF_PRIORITY = "Off";

static bool isMalformed;

JsonReaderResult Mocked_JsonObjectReader_ReadString(JsonObjectReaderHandle handle, const char* key, char** output) {
    if (isMalformed) {
        *output = "Shutuyut";
    } else if (strcmp(key, "processCreate") == 0 
        || strcmp(key, "systemInformation") == 0 
        || strcmp(key, "login") == 0
        || strcmp(key, "operational") == 0 ) 
    {
        *output = HIGH_PRIORITY;
    } else if (strcmp(key, "listeningPorts") == 0 
        || strcmp(key, "localUsers") == 0 
        || strcmp(key, "osBaseline") == 0
        || strcmp(key, "diagnostic") == 0 ) 
    {
        *output = LOW_PRIORITY;
    } else {
        *output = OFF_PRIORITY;
    }
    return JSON_READER_OK;
}

static void ValidateMockedPriorities() {
    TwinConfigurationResult result;
    TwinConfiguartionEventPriority priority;

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_PROCESS_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_LISTENING_PORTS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_SYSTEM_INFORMATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_LOCAL_USERS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_USER_LOGIN, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_CONNECTION_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_OFF, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_FIREWALL_CONFIGURATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_OFF, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_BASELINE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_DIAGNOSTIC, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_OPERATIONAL_EVENT, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);
}

static void ValidateDefaultPriorities() {
    TwinConfigurationResult result;
    TwinConfiguartionEventPriority priority;

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_PROCESS_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_LISTENING_PORTS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_SYSTEM_INFORMATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_LOCAL_USERS, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_USER_LOGIN, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_HIGH, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_CONNECTION_CREATE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_FIREWALL_CONFIGURATION, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_BASELINE, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);

    result = TwinConfigurationEventPriorities_GetPriority(EVENT_TYPE_DIAGNOSTIC, &priority);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(int, EVENT_PRIORITY_LOW, priority);
}

static LOCK_HANDLE testLockHadnle = (LOCK_HANDLE)0x1;

BEGIN_TEST_SUITE(twin_configuration_event_priorities_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectReaderHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfiguartionEventPriority, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, int);

    // Lock
    REGISTER_GLOBAL_MOCK_RETURN(Lock_Init, testLockHadnle);
    REGISTER_GLOBAL_MOCK_RETURN(Lock_Deinit, LOCK_OK);
    REGISTER_GLOBAL_MOCK_RETURN(Lock, LOCK_OK);
    REGISTER_GLOBAL_MOCK_RETURN(Unlock, LOCK_OK);

    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, Mocked_JsonObjectReader_ReadString);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectReader_ReadString, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    isMalformed = false;
    umock_c_reset_all_calls();
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Init_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Init_LockFailed_ExpectFailure)
{
    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(NULL);
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_LOCK_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Denit_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    TwinConfigurationEventPriorities_Deinit();
    STRICT_EXPECTED_CALL(Lock_Deinit(testLockHadnle));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Update_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "localUsers", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "login", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "connectionCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "firewallConfiguration", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "osBaseline", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "diagnostic", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "operational", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateMockedPriorities();
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Update_KeyMissing_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "localUsers", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "login", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "connectionCreate", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "firewallConfiguration", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "osBaseline", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "diagnostic", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "operational", IGNORED_PTR_ARG));

    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Update_ReaderError_ExpectFailure)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG)).SetReturn(JSON_READER_EXCEPTION);
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventPriorities_GetPrioritiesJsonExpectSuccess){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;
    result = TwinConfigurationEventPriorities_Update(readerHandle);
    
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectWriterHandle objectWriter = (JsonObjectWriterHandle)0x10;
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, BASELINE_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, CONNECTION_CREATE_PRIORITY_KEY, "off"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, DIAGNOSTIC_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, FIREWALL_CONFIGURATION_PRIORITY_KEY, "off"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LISTENING_PORTS_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LOCAL_USERS_PRIORITY_KEY, "low"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, LOGIN_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, OPERATIONAL_EVENT_KEY, "high"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, PROCESS_CREATE_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(objectWriter, SYSTEM_INFORMATION_PRIORITY_KEY, "high"));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
    result = TwinConfigurationEventPriorities_GetPrioritiesJson(objectWriter);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(TwinConfigurationEventPriorities_GetPrioritiesJsonExpectFail){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;
    result = TwinConfigurationEventPriorities_Update(readerHandle);
    
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

        result = TwinConfigurationEventPriorities_GetPrioritiesJson(objectWriter);
        ASSERT_IS_TRUE(TWIN_OK != result);
    }

    umock_c_negative_tests_deinit();    
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Update_SetBackToDeafultValues_ExpectSuccess)
{
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "localUsers", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "login", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "connectionCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "firewallConfiguration", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "osBaseline", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "diagnostic", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "operational", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateMockedPriorities();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "localUsers", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "login", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "connectionCreate", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "firewallConfiguration", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "osBaseline", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "diagnostic", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "operational", IGNORED_PTR_ARG)).SetReturn(JSON_READER_KEY_MISSING);
    
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateDefaultPriorities();
}

TEST_FUNCTION(TwinConfigurationEventPriorities_Update_MalformedJson_ConfigStayIntact){
    STRICT_EXPECTED_CALL(Lock_Init());
    TwinConfigurationResult result = TwinConfigurationEventPriorities_Init();
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);

    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    JsonObjectReaderHandle readerHandle = (JsonObjectReaderHandle)0x10;

    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "listeningPorts", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "systemInformation", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "localUsers", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "login", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "connectionCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "firewallConfiguration", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "osBaseline", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "diagnostic", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "operational", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));

    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ValidateMockedPriorities();
    umock_c_reset_all_calls();
    isMalformed = true;
    STRICT_EXPECTED_CALL(Lock(testLockHadnle));
    
    STRICT_EXPECTED_CALL(JsonObjectReader_ReadString(readerHandle, "processCreate", IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(Unlock(testLockHadnle));
    
    result = TwinConfigurationEventPriorities_Update(readerHandle);
    ASSERT_ARE_EQUAL(int, TWIN_PARSE_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    //priorities should stay intact
    ValidateMockedPriorities();
}

END_TEST_SUITE(twin_configuration_event_priorities_ut)
