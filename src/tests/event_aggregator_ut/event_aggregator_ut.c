// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdint.h>

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

#include "json/json_object_writer.h"

#define ENABLE_MOCKS
#include "synchronized_queue.h"
#include "twin_configuration_event_collectors.h"
#undef ENABLE_MOCKS

#include "collectors/event_aggregator.h"
#include "internal/time_utils_consts.h"


static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static bool isAggregationEnabled = false;
static TwinConfigurationEventType tested_event_type = EVENT_TYPE_PROCESS_CREATE;
static uint32_t aggregationInterval = MILLISECONDS_IN_AN_HOUR;
static const char* jsonPayload1 = "{ \"p1\" : \"v1\", \"p2\" : \"v2\" }"; 
static const char* jsonPayload2 = "{ \"p1\" : \"v3\", \"p2\" : \"v4\" }"; 
static JsonObjectWriterHandle payload1Handle;
static JsonObjectWriterHandle payload2Handle;
static uint32_t msgCounter = 0;
static EventAggregatorHandle aggregatorUnderTest;

TwinConfigurationResult Mocked_TwinConfiguration_GetAggregationEnabled(TwinConfigurationEventType eventType, bool* isEnabled) {
    *isEnabled = isAggregationEnabled;
    return TWIN_OK;
}

TwinConfigurationResult Mocked_TwinConfiguration_GetAggregationInterval(TwinConfigurationEventType eventType, uint32_t* interval) {
    *interval = aggregationInterval;
    return TWIN_OK;
}

int Mocked_SyncQueue_PushBack(SyncQueue* syncQueue, void* data, uint32_t dataSize) {
    if (data != NULL) {
        free(data);
    }
    msgCounter++;

    return QUEUE_OK;
}

void InitAggregator(EventAggregatorHandle* aggregator) {
    EventAggregatorConfiguration configuration = {
        .iotEventType = EVENT_TYPE_PROCESS_CREATE,
        .event_type = "SomeType",
        .event_name = "SomeName",
        .payload_schema_version = "ver"
    };
    
    EventAggregatorResult result = EventAggregator_Init(aggregator, &configuration);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_OK);
}

BEGIN_TEST_SUITE(event_aggregator_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationEventType, int);
    REGISTER_UMOCK_ALIAS_TYPE(TwinConfigurationResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);

    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationEventCollectors_GetAggregationEnabled, Mocked_TwinConfiguration_GetAggregationEnabled);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationEventCollectors_GetAggregationInterval, Mocked_TwinConfiguration_GetAggregationInterval);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PushBack, Mocked_SyncQueue_PushBack);

    JsonObjectWriter_InitFromString(&payload1Handle, jsonPayload1);
    JsonObjectWriter_InitFromString(&payload2Handle, jsonPayload2);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
    
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationEventCollectors_GetAggregationEnabled, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(TwinConfigurationEventCollectors_GetAggregationInterval, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(SyncQueue_PushBack, NULL);

    JsonObjectWriter_Deinit(payload2Handle);
    JsonObjectWriter_Deinit(payload1Handle);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    InitAggregator(&aggregatorUnderTest);
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_init)
{
    EventAggregator_Deinit(aggregatorUnderTest);
}

TEST_FUNCTION(EventAggregator_Init_ExpectSuccess)
{
}

TEST_FUNCTION(EventAggregator_Init_ExpectFail)
{
    EventAggregatorConfiguration configuration = {
        .iotEventType = EVENT_TYPE_PROCESS_CREATE,
        .event_type = NULL,
        .event_name = NULL,
        .payload_schema_version = NULL
    };
    
    EventAggregatorHandle aggregator;
    EventAggregatorResult result = EventAggregator_Init(&aggregator, NULL);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_EXCEPTION);

    result = EventAggregator_Init(&aggregator, &configuration);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_EXCEPTION);

    configuration.event_type = "AAA";
    configuration.event_name = "BBB";
    result = EventAggregator_Init(&aggregator, &configuration);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_EXCEPTION);

    //maybe continue
    
}

TEST_FUNCTION(EventAggregator_IsAggregationEnabled_ExpectSuccess)
{
    bool acutalIsEnabled;
    isAggregationEnabled = false;
    EventAggregatorResult result = EventAggregator_IsAggregationEnabled(aggregatorUnderTest, &acutalIsEnabled);
    ASSERT_ARE_EQUAL(int, acutalIsEnabled, isAggregationEnabled);

    
    isAggregationEnabled = true;
    result = EventAggregator_IsAggregationEnabled(aggregatorUnderTest, &acutalIsEnabled);
    ASSERT_ARE_EQUAL(int, acutalIsEnabled, isAggregationEnabled);
}

TEST_FUNCTION(EventAggregator_AggregateEvent_AggregationDisabled_ExpectFail) {
    isAggregationEnabled = false;
    EventAggregatorResult result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_DISABLED, result);
}

TEST_FUNCTION(EventAggregator_AggregateEvent_AggregationEnabled_ExpectSuccess) {
    isAggregationEnabled = true;
    EventAggregatorResult result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
}

TEST_FUNCTION(EventAggregator_GetAggregatedEvents_ToggleAggregationEnabledToDisabled) {
    isAggregationEnabled = true;
    aggregationInterval = MILLISECONDS_IN_A_YEAR;
    EventAggregatorResult result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    
    SyncQueue queue;
    msgCounter = 0;
    isAggregationEnabled = false;
    result = EventAggregator_GetAggregatedEvents(aggregatorUnderTest, &queue);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_OK);
    ASSERT_ARE_EQUAL(int, 1, msgCounter);
}

TEST_FUNCTION(EventAggregator_GetAggregatedEvents_IntervalHasNotPassed) {
    isAggregationEnabled = true;
    aggregationInterval = MILLISECONDS_IN_A_YEAR;
    EventAggregatorResult result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    
    SyncQueue queue;
    msgCounter = 0;
    result = EventAggregator_GetAggregatedEvents(aggregatorUnderTest, &queue);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_OK);
    ASSERT_ARE_EQUAL(int, 0, msgCounter);
}


TEST_FUNCTION(EventAggregator_GetAggregatedEvents_NoEventsIntervalPassed) {
    isAggregationEnabled = true;
    aggregationInterval = 0;
    SyncQueue queue;
    msgCounter = 0;
    EventAggregatorResult result = EventAggregator_GetAggregatedEvents(aggregatorUnderTest, &queue);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_OK);
    ASSERT_ARE_EQUAL(int, 0, msgCounter);
}

TEST_FUNCTION(EventAggregator_FunctionalTest) {
    isAggregationEnabled = true;
    aggregationInterval = 0;
    EventAggregatorResult result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload2Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload1Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);
    result = EventAggregator_AggregateEvent(aggregatorUnderTest, payload2Handle);
    ASSERT_ARE_EQUAL(int, EVENT_AGGREGATOR_OK, result);

    SyncQueue queue;
    msgCounter = 0;
    isAggregationEnabled = false;
    result = EventAggregator_GetAggregatedEvents(aggregatorUnderTest, &queue);
    ASSERT_ARE_EQUAL(int, result, EVENT_AGGREGATOR_OK);
    ASSERT_ARE_EQUAL(int, 2, msgCounter);
}

END_TEST_SUITE(event_aggregator_ut)
