// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iothub_adapter.h"

#include <stdlib.h>

#include "azure_c_shared_utility/threadapi.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothubtransportamqp.h"

#include "consts.h"
#include "local_config.h"
#include "logger.h"
#include "message_schema_consts.h"
#include "tasks/update_twin_task.h"

/**
 * @brief This function is called upon receiving confirmation of the delivery of the IoT Hub message
 * 
 * @param   result                  The result of the callback
 * @param   userContextCallback     The user context for the callback.
 */
static void IoTHubAdapter_SendConfirmCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);

/**
 * @brief This function is called upon receiving updates about the status of the connection to IoT Hub.
 * 
 * @param   result                  The result of the connection.
 * @param   reason                  The reason for this callback.
 * @param   userContextCallback     The user context for the callback.
 */
static void IoTHubAdapter_ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback);

/**
 * @brief This function is calles upon response to patch request send by the IoTHub services
 * 
 * @param   updateState             The current update state of the twin.
 * @param   payload                 The updates of the twin configuration.
 * @param   size                    The size of the payload.
 * @param   userContextCallback     The user context for the callback.
 */
static void IoTHubAdapter_DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t size, void* userContextCallback);


bool IoTHubAdapter_Init(IoTHubAdapter* iotHubAdapter, SyncQueue* twinUpdatesQueue) {

    bool success = true;
    memset(iotHubAdapter, 0, sizeof(*iotHubAdapter));
    
    if (IoTHub_Init() != 0) {
        success = false;
        goto cleanup;
    }
    
    iotHubAdapter->hubInitiated = true;
    iotHubAdapter->twinUpdatesQueue = twinUpdatesQueue;

    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = AMQP_Protocol;

    // Create the iothub handle here
    iotHubAdapter->moduleHandle = IoTHubModuleClient_CreateFromConnectionString(LocalConfiguration_GetConnectionString(), protocol);
    if (iotHubAdapter->moduleHandle == NULL) {
        success = false;
        goto cleanup;
    }

    bool traceOn = true;
    if (IoTHubModuleClient_SetOption(iotHubAdapter->moduleHandle, OPTION_LOG_TRACE, &traceOn) != IOTHUB_CLIENT_OK) {
        success = false;
        goto cleanup;
    }

    // Setting connection status callback to get indication of connection to iothub
    if (IoTHubModuleClient_SetConnectionStatusCallback(iotHubAdapter->moduleHandle, IoTHubAdapter_ConnectionStatusCallback, iotHubAdapter) != IOTHUB_CLIENT_OK) {
        success = false;
        goto cleanup;
    }
    
    if (IoTHubModuleClient_SetModuleTwinCallback(iotHubAdapter->moduleHandle, IoTHubAdapter_DeviceTwinCallback, iotHubAdapter) != IOTHUB_CLIENT_OK) {
        success = false;
        goto cleanup;
    }

    success = AgentTelemetryCounter_Init(&iotHubAdapter->messageCounter);
    if (!success){
        goto cleanup;
    }
        

cleanup:
    if (!success) {
        IoTHubAdapter_Deinit(iotHubAdapter);
    }

    return success;
}

void IoTHubAdapter_Deinit(IoTHubAdapter* iotHubAdapter) {
    AgentTelemetryCounter_Deinit(&iotHubAdapter->messageCounter);

    if (iotHubAdapter->hubInitiated) {
        IoTHub_Deinit();
    }

    if (iotHubAdapter->moduleHandle != NULL) {
        IoTHubModuleClient_Destroy(iotHubAdapter->moduleHandle);
    }

}

bool IoTHubAdapter_Connect(IoTHubAdapter* iotHubAdapter) {
    //FIXME: add some timeout to this function so we will not loop forever
    while (!iotHubAdapter->connected || !iotHubAdapter->hasTwinConfiguration) {
        ThreadAPI_Sleep(100);
    }

    return true;
}

bool IoTHubAdapter_SendMessageAsync(IoTHubAdapter* iotHubAdapter, const void* data, size_t dataSize) {

    bool success = true;
    IOTHUB_MESSAGE_HANDLE messageHandle = NULL;

    messageHandle = IoTHubMessage_CreateFromByteArray(data, dataSize);

    if (messageHandle == NULL) {
        Logger_Warning("unable to create a new IoTHubMessage");
        success = false;
        goto cleanup;
    }

    if (IoTHubMessage_SetAsSecurityMessage(messageHandle) != IOTHUB_MESSAGE_OK) {
        Logger_Warning("failed to set message as security message");
        success = false;
        goto cleanup;
    }

    if (IoTHubModuleClient_SendEventAsync(iotHubAdapter->moduleHandle, messageHandle, IoTHubAdapter_SendConfirmCallback, iotHubAdapter) != IOTHUB_CLIENT_OK) {
        Logger_Warning("failed to hand over the message to IoTHubClient");
        success = false;
        goto cleanup;
    }

    if (dataSize < MESSAGE_BILLING_MULTIPLE){
        AgentTelemetryCounter_IncreaseBy(&iotHubAdapter->messageCounter, &iotHubAdapter->messageCounter.counter.messageCounter.smallMessages, 1);
    }
    AgentTelemetryCounter_IncreaseBy(&iotHubAdapter->messageCounter, &iotHubAdapter->messageCounter.counter.messageCounter.sentMessages, 1);
    Logger_Debug("IoTHubClient accepted the message for delivery");

cleanup:
    if (messageHandle != NULL) {
        IoTHubMessage_Destroy(messageHandle);
    }

    return success;
}

static void IoTHubAdapter_SendConfirmCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback) {
    if (userContextCallback == NULL) {
        Logger_Error("send_confirm_callback error in user context");
        return;
    }

    IoTHubAdapter* adapter = (IoTHubAdapter*)(userContextCallback);
    if (result != IOTHUB_CLIENT_CONFIRMATION_OK){
        AgentTelemetryCounter_IncreaseBy(&adapter->messageCounter, &adapter->messageCounter.counter.messageCounter.failedMessages, 1);
    }
}

static void IoTHubAdapter_ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback) {
    if (userContextCallback == NULL) { 
        Logger_Error("connection_status_callback error in user context");
        return;
    }

    IoTHubAdapter* adapter = (IoTHubAdapter*)(userContextCallback);

    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        adapter->connected = true;
        Logger_Information("The module client is connected to iothub");
    } else {
        adapter->connected = false;
        Logger_Information("The module client has been disconnected");
    }
}

static void IoTHubAdapter_DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t size, void* userContextCallback) {
    Logger_Debug("twin callback started");
    bool success = true;
    UpdateTwinTaskItem* twinTaskItem = NULL;
	IoTHubAdapter* adapter = NULL;

    if (size == 0) {
        success = false;
        goto cleanup;
    }

    if (userContextCallback == NULL) {
        Logger_Error("error in user context");
        success = false;
        goto cleanup;
    }

    adapter = (IoTHubAdapter*)(userContextCallback);

    UpdateTwinTask_InitUpdateTwinTaskItem(&twinTaskItem, payload, size, updateState == DEVICE_TWIN_UPDATE_COMPLETE);

    if (SyncQueue_PushBack(adapter->twinUpdatesQueue, twinTaskItem, sizeof(UpdateTwinTaskItem)) != QUEUE_OK) {
        Logger_Error("Bad allocation");
        //FIXME: how to handlebad allocation?
        success = false;
        goto cleanup;
    }

cleanup:
    if (!success) {
        if (twinTaskItem != NULL) {
            UpdateTwinTask_DeinitUpdateTwinTaskItem(&twinTaskItem);
        }
    } else {
        adapter->hasTwinConfiguration = true;
    }
}
