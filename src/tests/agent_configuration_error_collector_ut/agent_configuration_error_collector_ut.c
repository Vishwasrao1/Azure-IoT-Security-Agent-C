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
#include "internal/time_utils.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "synchronized_queue.h"
#include "twin_configuration.h"
#undef ENABLE_MOCKS

#include "collectors/agent_configuration_error_collector.h"
#include "consts.h"
#include "message_schema_consts.h"
#include "twin_configuration_consts.h"
#include "twin_configuration_defs.h"
#include "utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static TwinConfigurationResult lastUpdateResult;
static TwinConfigurationBundleStatus mockedStatus;
static time_t lastUpdateTime;
static uint32_t mockedMaxCache;
static uint32_t mockedMaxMessageSize;
static uint32_t mockedHighPrioFreq;
static uint32_t mockedLowPrioFreq;

void Mocked_TwinConfiguration_GetLastTwinUpdateData(TwinConfigurationUpdateResult* outResult) {
    outResult->configurationBundleStatus = mockedStatus;
    outResult->lastUpdateResult = lastUpdateResult;
    outResult->lastUpdateTime = lastUpdateTime;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetMaxLocalCacheSize(uint32_t* maxCache) {
    *maxCache = mockedMaxCache;
    return TWIN_OK;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetMaxMessageSize(uint32_t* maxMessageSize) {
    *maxMessageSize = mockedMaxMessageSize;
    return TWIN_OK;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetHighPriorityMessageFrequency(uint32_t* highPrioFreq) {
    *highPrioFreq = mockedHighPrioFreq;
    return TWIN_OK;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetLowPriorityMessageFrequency(uint32_t* lowPrioFreq) {
    *lowPrioFreq = mockedLowPrioFreq;
    return TWIN_OK;
}

JsonWriterResult Mocked_JsonObjectWriter_Init(JsonObjectWriterHandle* handle) {
    *handle = (JsonObjectWriterHandle)0x2;
    return JSON_WRITER_OK;
}

JsonWriterResult Mocked_JsonArrayWriter_Init(JsonArrayWriterHandle* handle) {
    *handle = (JsonArrayWriterHandle)0x3;
    return JSON_WRITER_OK;
}

BEGIN_TEST_SUITE(agent_configuration_error_collector_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);
    umock_c_init(on_umock_c_error);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationStatus, int);
    REGISTER_UMOCK_ALIAS_TYPE(EventCollectorResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JsonArrayWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonObjectWriterHandle, void*);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(QueueResultValues, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(int32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(time_t, int);


    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetLastTwinUpdateData, Mocked_TwinConfiguration_GetLastTwinUpdateData);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxLocalCacheSize, Mocked_TwinConfiguration_GetMaxLocalCacheSize);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, Mocked_TwinConfiguration_GetMaxMessageSize);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetHighPriorityMessageFrequency, Mocked_TwinConfiguration_GetHighPriorityMessageFrequency);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetLowPriorityMessageFrequency, Mocked_TwinConfiguration_GetLowPriorityMessageFrequency);  
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, Mocked_JsonObjectWriter_Init);  
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, Mocked_JsonArrayWriter_Init);  
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetLastTwinUpdateData, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxLocalCacheSize, NULL);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetMaxMessageSize, NULL);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetHighPriorityMessageFrequency, NULL);  
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfiguration_GetLowPriorityMessageFrequency, NULL);  
    REGISTER_GLOBAL_MOCK_HOOK(JsonObjectWriter_Init, NULL);  
    REGISTER_GLOBAL_MOCK_HOOK(JsonArrayWriter_Init, NULL);  

    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

void setupEventInitExpectSuccess() {
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLastTwinUpdateData(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(5);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, AGENT_CONFIGURATION_ERROR_EVENT_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_CONFIGURATION_ERROR_EVENT_SCHEMA_VERSION));
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG));
}

void setupAddPayloadExpectSuccess(){
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_CONFIGURATION_NAME_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_USED_CONFIGURATION_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_MESSAGE_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_ERROR_KEY, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

void setupPushEventExpectSuccess(SyncQueue* queue){
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(queue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(QUEUE_OK);
}

void setupCleanUpExpectSuccess(){
    STRICT_EXPECTED_CALL(JsonArrayWriter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

void setupEventInitExpectFail() {
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLastTwinUpdateData(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(5);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(GenericEvent_AddMetadata(IGNORED_PTR_ARG, EVENT_TRIGGERED_CATEGORY, AGENT_CONFIGURATION_ERROR_EVENT_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_CONFIGURATION_ERROR_EVENT_SCHEMA_VERSION)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
}

void setupAddPayloadExpectFail(){
    STRICT_EXPECTED_CALL(JsonObjectWriter_Init(IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_CONFIGURATION_NAME_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_USED_CONFIGURATION_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_MESSAGE_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_WriteString(IGNORED_PTR_ARG, AGENT_CONFIGURATION_ERROR_ERROR_KEY, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonArrayWriter_AddObject(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Deinit(IGNORED_PTR_ARG));
}

void setupPushEventExpectFail(SyncQueue* queue){
    STRICT_EXPECTED_CALL(GenericEvent_AddPayload(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!EVENT_COLLECTOR_OK);
    STRICT_EXPECTED_CALL(JsonObjectWriter_Serialize(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!JSON_WRITER_OK);
    STRICT_EXPECTED_CALL(SyncQueue_PushBack(queue, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetFailReturn(!QUEUE_OK);
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_maxLocalCacheValidateExpectSuccess) {
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectSuccess();
    mockedMaxCache = 1000;
    mockedMaxMessageSize = 4096;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 100;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    setupAddPayloadExpectSuccess();
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));

    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();

    EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_maxLocalCacheValidateExpectFail) {
    umock_c_negative_tests_init();
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectFail();
    mockedMaxCache = 1000;
    mockedMaxMessageSize = 4096;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 100;
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    setupAddPayloadExpectFail();
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(TWIN_EXCEPTION);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(TWIN_EXCEPTION);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);

    setupPushEventExpectFail(&queue);
    setupCleanUpExpectSuccess();

    umock_c_negative_tests_snapshot();

    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < count; i++) {
        if (i == 0 || i == 1 || i == 7 || i == 13 || i == count - 2 || i == count -1 )
            continue;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
        ASSERT_IS_TRUE(result != EVENT_COLLECTOR_OK);
    }

    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_maxMessageSizeIsNopOptimalExpectFail) {
    umock_c_negative_tests_init();
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectFail();
    mockedMaxCache = 5000;
    mockedMaxMessageSize = 3000;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 100;
    //validate maxLocalCache
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    //validate optimal msg size
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    setupAddPayloadExpectFail();
    //validate prio freqs
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);

    setupPushEventExpectFail(&queue);
    setupCleanUpExpectSuccess();

    umock_c_negative_tests_snapshot();

    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < count; i++) {
        if (i == 0 || i == 1 || i== 8 ||i == 14 || i == count - 2 || i == count -1 )
            continue;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
        ASSERT_IS_TRUE(result != EVENT_COLLECTOR_OK);
    }

    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_maxMessageSizeIsNopOptimalExpectSuccess) {
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectSuccess();
    mockedMaxCache = 5000;
    mockedMaxMessageSize = 3000;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 100;
    //validate maxLocalCache
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    //validate optimal msg size
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    setupAddPayloadExpectSuccess();
    //validate prio freqs
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));

    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();

    EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_HighPrioFreqIsHighThanLowPrioExpectFail) {
    umock_c_negative_tests_init();
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectFail();
    mockedMaxCache = 5000;
    mockedMaxMessageSize = 4096;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 30000;
    //validate maxLocalCache
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    //validate optimal msg size
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    //validate prio freqs
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    STRICT_EXPECTED_CALL(TimeUtils_MillisecondsToISO8601DurationString(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(false);

    setupAddPayloadExpectFail();

    setupPushEventExpectFail(&queue);
    setupCleanUpExpectSuccess();

    umock_c_negative_tests_snapshot();

    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < count; i++) {
        if (i == 0 || i == 1 || i == 11 || i == 17 || i == count - 2 || i == count -1 )
            continue;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
        ASSERT_IS_TRUE(result != EVENT_COLLECTOR_OK);
    }

    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_HighPrioFreqIsHighThanLowPrioExpectSuccess) {
    lastUpdateResult = TWIN_OK;
    SyncQueue queue;
    setupEventInitExpectSuccess();
    mockedMaxCache = 5000;
    mockedMaxMessageSize = 4096;
    mockedLowPrioFreq = 2000;
    mockedHighPrioFreq = 30000;
    //validate maxLocalCache
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxLocalCacheSize(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    //validate optimal msg size
    STRICT_EXPECTED_CALL(TwinConfiguration_GetMaxMessageSize(IGNORED_PTR_ARG));
    //validate prio freqs
    STRICT_EXPECTED_CALL(TwinConfiguration_GetHighPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLowPriorityMessageFrequency(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_MillisecondsToISO8601DurationString(IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG)).SetReturn(true);

    setupAddPayloadExpectSuccess();

    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();

    EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_TestTypeMismatchExpectFail) {
    umock_c_negative_tests_init();
    lastUpdateResult = TWIN_PARSE_EXCEPTION;
    SyncQueue queue;
    setupEventInitExpectFail();
    
    STRICT_EXPECTED_CALL(TwinConfiguration_GetSerializedTwinConfiguration(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetFailReturn(!TWIN_OK);
    setupAddPayloadExpectFail();

    setupPushEventExpectFail(&queue);
    setupCleanUpExpectSuccess();
    
    umock_c_negative_tests_snapshot();
    int count = umock_c_negative_tests_call_count();
    for (int i = 0; i < count; i++) {
        if (i == 0 || i == 1 || i == 6 || i == 12 || i == count - 2 || i == count -1 )
            continue;
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);
        EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
        ASSERT_IS_TRUE(result != EVENT_COLLECTOR_OK);
    }

    umock_c_negative_tests_deinit();
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_TestTypeMismatchExpectSuccess) {
    lastUpdateResult = TWIN_PARSE_EXCEPTION;
    SyncQueue queue;
    setupEventInitExpectSuccess();
    
    STRICT_EXPECTED_CALL(TwinConfiguration_GetSerializedTwinConfiguration(IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    setupAddPayloadExpectSuccess();

    setupPushEventExpectSuccess(&queue);
    setupCleanUpExpectSuccess();

    EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(AgentConfiguratioErrorCollector_TestNoEventIfEventAlreadySentForCurrentTwin) {
    STRICT_EXPECTED_CALL(TwinConfiguration_GetLastTwinUpdateData(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(TimeUtils_GetTimeDiff(IGNORED_NUM_ARG, IGNORED_NUM_ARG)).SetReturn(0);
    
    SyncQueue queue;
    EventCollectorResult result = AgentConfigurationErrorCollector_GetEvents(&queue);
    ASSERT_ARE_EQUAL(int, EVENT_COLLECTOR_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(agent_configuration_error_collector_ut)
