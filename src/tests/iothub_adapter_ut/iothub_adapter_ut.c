// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdint.h>
#include <stdbool.h>

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/threadapi.h"

#include "iothub.h"
#include "iothub_module_client.h"
#include "iothub_client.h"
#include "iothub_client_options.h"

#include "local_config.h"
#include "synchronized_queue.h"
#include "tasks/update_twin_task.h"
#include "twin_configuration.h"
#include "agent_telemetry_counters.h"

#undef ENABLE_MOCKS
#include "consts.h"
#include "iothub_adapter.h"
#include "message_schema_consts.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

const TRANSPORT_PROVIDER* AMQP_Protocol(void) {
    return NULL;
}

IOTHUB_CLIENT_CONNECTION_STATUS_CALLBACK connectionCallback = NULL;
void* connectionContext = NULL;
IOTHUB_CLIENT_RESULT Mocked_IoTHubModuleClient_SetConnectionStatusCallback(IOTHUB_MODULE_CLIENT_HANDLE iotHubModuleClientHandle, IOTHUB_CLIENT_CONNECTION_STATUS_CALLBACK connectionStatusCallback, void* userContextCallback) {
    connectionCallback = connectionStatusCallback;
    connectionContext = userContextCallback;
    return IOTHUB_CLIENT_OK;
}

IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK twinCallback = NULL;
void* twinContext = NULL;
const char* DUMMY_TWIN_CONF = "{\"a\" : \"b\"}";
IOTHUB_CLIENT_RESULT Mocked_IoTHubModuleClient_SetModuleTwinCallback(IOTHUB_MODULE_CLIENT_HANDLE iotHubModuleClientHandle, IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK moduleTwinCallback, void* userContextCallback) {
    twinCallback = moduleTwinCallback;
    twinContext = userContextCallback;
    return IOTHUB_CLIENT_OK;
}

IOTHUB_CLIENT_RESULT Mocked_IoTHubModuleClient_SendEventAsync(IOTHUB_MODULE_CLIENT_HANDLE iotHubClientHandle, IOTHUB_MESSAGE_HANDLE eventMessageHandle, IOTHUB_CLIENT_EVENT_CONFIRMATION_CALLBACK eventConfirmationCallback, void* userContextCallback) {
    eventConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_OK, userContextCallback);
    return IOTHUB_CLIENT_OK;    
}

BEGIN_TEST_SUITE(iothub_adapter_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_CONNECTION_STATUS_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_DEVICE_TWIN_CALLBACK, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_EVENT_CONFIRMATION_CALLBACK, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_RESULT, int);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_CLIENT_TRANSPORT_PROVIDER, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MESSAGE_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MESSAGE_RESULT, void*);
    REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MODULE_CLIENT_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_GLOBAL_MOCK_RETURN(LocalConfiguration_Init, LOCAL_CONFIGURATION_OK);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetConnectionStatusCallback, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetModuleTwinCallback, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SendEventAsync, NULL);

    connectionCallback = NULL;
    connectionContext = NULL;
    twinCallback = NULL;
    twinContext = NULL;
}

TEST_FUNCTION(IoTHubAdapter_Init_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true); 
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, 0x1, adapter.moduleHandle);
    ASSERT_IS_FALSE(adapter.hasTwinConfiguration);
    ASSERT_IS_FALSE(adapter.connected);
    ASSERT_ARE_EQUAL(void_ptr, &queue, adapter.twinUpdatesQueue);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_HubInitFailed_ExpectFailure)
{
   IoTHubAdapter adapter;
   SyncQueue queue;
    
    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(1);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));

    bool result = IoTHubAdapter_Init(&adapter, &queue);

    ASSERT_IS_FALSE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_CreateFromConnectionStringFailed_ExpectFailure)
{
   IoTHubAdapter adapter;
   SyncQueue queue;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn((IOTHUB_MODULE_CLIENT_HANDLE)NULL);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHub_Deinit());

    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_FALSE(result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_SetLogOptionFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;
    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(!IOTHUB_CLIENT_OK);
    
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHub_Deinit());
    STRICT_EXPECTED_CALL(IoTHubModuleClient_Destroy(mockHandle));
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_FALSE(result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_SetConnectionCallbackFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;
    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(!IOTHUB_CLIENT_OK);

    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHub_Deinit());
    STRICT_EXPECTED_CALL(IoTHubModuleClient_Destroy(mockHandle));
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_FALSE(result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_SetDeviceTwinCallbackFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;
    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(!IOTHUB_CLIENT_OK);
    
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHub_Deinit());
    STRICT_EXPECTED_CALL(IoTHubModuleClient_Destroy(mockHandle));
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_FALSE(result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Deinit_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHub_Deinit());
    STRICT_EXPECTED_CALL(IoTHubModuleClient_Destroy(mockHandle));
    IoTHubAdapter_Deinit(&adapter);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Connect_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetConnectionStatusCallback, Mocked_IoTHubModuleClient_SetConnectionStatusCallback);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetModuleTwinCallback, Mocked_IoTHubModuleClient_SetModuleTwinCallback);

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter));
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter));
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);
    bool result = IoTHubAdapter_Init(&adapter, &queue);

    ASSERT_IS_TRUE(result);

    // calling the callbacks to ensure connection establishment
    twinCallback(DEVICE_TWIN_UPDATE_COMPLETE, DUMMY_TWIN_CONF, strlen(DUMMY_TWIN_CONF), twinContext);
    connectionCallback(IOTHUB_CLIENT_CONNECTION_AUTHENTICATED, IOTHUB_CLIENT_CONNECTION_OK, connectionContext);
    umock_c_reset_all_calls();

    result = IoTHubAdapter_Connect(&adapter);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);
    SyncQueue_Deinit(&queue);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetConnectionStatusCallback, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SetModuleTwinCallback, NULL);
}

TEST_FUNCTION(IoTHubAdapter_SendMessageAsync_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    char* dataToSend = "This is a message";
    IOTHUB_MESSAGE_HANDLE mockedMessageHandle = (IOTHUB_MESSAGE_HANDLE)(0x2);
    STRICT_EXPECTED_CALL(IoTHubMessage_CreateFromByteArray(dataToSend, strlen(dataToSend))).SetReturn(mockedMessageHandle);
    STRICT_EXPECTED_CALL(IoTHubMessage_SetAsSecurityMessage(mockedMessageHandle)).SetReturn(IOTHUB_MESSAGE_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SendEventAsync(mockHandle, mockedMessageHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(IoTHubMessage_Destroy(mockedMessageHandle));

    result = IoTHubAdapter_SendMessageAsync(&adapter, dataToSend, strlen(dataToSend));
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);   
}

TEST_FUNCTION(IoTHubAdapter_SendMessageAsync_CallMessageSentCallback_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);

    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    char* dataToSend = "This is a message";
    IOTHUB_MESSAGE_HANDLE mockedMessageHandle = (IOTHUB_MESSAGE_HANDLE)(0x2);
    STRICT_EXPECTED_CALL(IoTHubMessage_CreateFromByteArray(dataToSend, strlen(dataToSend))).SetReturn(mockedMessageHandle);
    STRICT_EXPECTED_CALL(IoTHubMessage_SetAsSecurityMessage(mockedMessageHandle)).SetReturn(IOTHUB_MESSAGE_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SendEventAsync(mockHandle, mockedMessageHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(IoTHubMessage_Destroy(mockedMessageHandle));

    result = IoTHubAdapter_SendMessageAsync(&adapter, dataToSend, strlen(dataToSend));
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);   
}

TEST_FUNCTION(IoTHubAdapter_SendMessageAsync_CallMessageConfirmationCallback_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;
    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SendEventAsync, Mocked_IoTHubModuleClient_SendEventAsync);

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);

    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    char* dataToSend = "This is a message";
    IOTHUB_MESSAGE_HANDLE mockedMessageHandle = (IOTHUB_MESSAGE_HANDLE)(0x2);
    STRICT_EXPECTED_CALL(IoTHubMessage_CreateFromByteArray(dataToSend, strlen(dataToSend))).SetReturn(mockedMessageHandle);
    STRICT_EXPECTED_CALL(IoTHubMessage_SetAsSecurityMessage(mockedMessageHandle)).SetReturn(IOTHUB_MESSAGE_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SendEventAsync(mockHandle, mockedMessageHandle, IGNORED_PTR_ARG, &adapter));
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_IncreaseBy(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(IoTHubMessage_Destroy(mockedMessageHandle));

    result = IoTHubAdapter_SendMessageAsync(&adapter, dataToSend, strlen(dataToSend));
    ASSERT_IS_TRUE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);
    REGISTER_GLOBAL_MOCK_HOOK(IoTHubModuleClient_SendEventAsync, NULL);
}

TEST_FUNCTION(IoTHubAdapter_SendMessageAsync_CreateMessageFromBytesFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);

    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    char* dataToSend = "This is a message";
    STRICT_EXPECTED_CALL(IoTHubMessage_CreateFromByteArray(dataToSend, strlen(dataToSend))).SetReturn(NULL);
    
    result = IoTHubAdapter_SendMessageAsync(&adapter, dataToSend, strlen(dataToSend));
    ASSERT_IS_FALSE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);   
}

TEST_FUNCTION(IoTHubAdapter_SendMessageAsync_SetAsSecurityMessageFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(IoTHub_Init()).SetReturn(0);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true);
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    
    char* dataToSend = "This is a message";
    IOTHUB_MESSAGE_HANDLE mockedMessageHandle = (IOTHUB_MESSAGE_HANDLE)(0x2);
    STRICT_EXPECTED_CALL(IoTHubMessage_CreateFromByteArray(dataToSend, strlen(dataToSend))).SetReturn(mockedMessageHandle);
    STRICT_EXPECTED_CALL(IoTHubMessage_SetAsSecurityMessage(mockedMessageHandle)).SetReturn(!IOTHUB_MESSAGE_OK);
    STRICT_EXPECTED_CALL(IoTHubMessage_Destroy(mockedMessageHandle));

    result = IoTHubAdapter_SendMessageAsync(&adapter, dataToSend, strlen(dataToSend));
    ASSERT_IS_FALSE(result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    IoTHubAdapter_Deinit(&adapter);   
}

END_TEST_SUITE(iothub_adapter_ut)
