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
#include "agent_telemetry_counters.h"
#include "twin_configuration.h"

#undef ENABLE_MOCKS

#include "iothub_adapter.h"
#include "tasks/update_twin_task.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

const TRANSPORT_PROVIDER* MQTT_Protocol(void) {
    return NULL;
}

static LOCK_HANDLE MOCKED_LOCK = (LOCK_HANDLE)0x42;

BEGIN_TEST_SUITE(iothub_adapter_mqtt_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

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
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LOCK_RESULT, void*);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);

    umocktypes_bool_register_types();
    umocktypes_charptr_register_types();

    REGISTER_GLOBAL_MOCK_RETURN(LocalConfiguration_Init, LOCAL_CONFIGURATION_OK);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(IoTHubAdapter_Init_ExpectSuccess)
{
    IoTHubAdapter adapter;
    SyncQueue queue;

    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(MOCKED_LOCK);
    STRICT_EXPECTED_CALL(Lock(MOCKED_LOCK)).SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_AUTO_URL_ENCODE_DECODE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_LOG_TRACE, IGNORED_PTR_ARG)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetConnectionStatusCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetModuleTwinCallback(mockHandle, IGNORED_PTR_ARG, &adapter)).SetReturn(IOTHUB_CLIENT_OK);
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Init(IGNORED_PTR_ARG)).SetReturn(true); 
    STRICT_EXPECTED_CALL(Unlock(MOCKED_LOCK)).SetReturn(LOCK_OK);
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(void_ptr, 0x1, adapter.moduleHandle);
    ASSERT_IS_FALSE(adapter.hasTwinConfiguration);
    ASSERT_IS_FALSE(adapter.connected);
    ASSERT_ARE_EQUAL(void_ptr, &queue, adapter.twinUpdatesQueue);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(IoTHubAdapter_Init_SetAutoUrlOptionFailed_ExpectFailure)
{
    IoTHubAdapter adapter;
    SyncQueue queue;
    IOTHUB_MODULE_CLIENT_HANDLE mockHandle = (IOTHUB_MODULE_CLIENT_HANDLE)0x1;

    STRICT_EXPECTED_CALL(Lock_Init()).SetReturn(MOCKED_LOCK);
    STRICT_EXPECTED_CALL(Lock(MOCKED_LOCK)).SetReturn(LOCK_OK);
    STRICT_EXPECTED_CALL(LocalConfiguration_GetConnectionString()).SetReturn("");
    STRICT_EXPECTED_CALL(IoTHubModuleClient_CreateFromConnectionString(IGNORED_PTR_ARG, IGNORED_PTR_ARG)).SetReturn(mockHandle);
    STRICT_EXPECTED_CALL(IoTHubModuleClient_SetOption(mockHandle, OPTION_AUTO_URL_ENCODE_DECODE, IGNORED_PTR_ARG)).SetReturn(!IOTHUB_CLIENT_OK);
    
    STRICT_EXPECTED_CALL(AgentTelemetryCounter_Deinit(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(IoTHubModuleClient_Destroy(mockHandle));
    STRICT_EXPECTED_CALL(Unlock(MOCKED_LOCK)).SetReturn(LOCK_OK);
    
    bool result = IoTHubAdapter_Init(&adapter, &queue);
    
    ASSERT_IS_FALSE(result);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(iothub_adapter_mqtt_ut)
