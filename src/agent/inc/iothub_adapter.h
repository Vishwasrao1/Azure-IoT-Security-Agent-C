// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUB_ADAPTER_H
#define IOTHUB_ADAPTER_H

#include <stdbool.h>

#include "iothub_module_client.h"

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "agent_telemetry_counters.h"
#include "synchronized_queue.h"

typedef struct _IoTHubAdapter {

    IOTHUB_MODULE_CLIENT_HANDLE moduleHandle;
    bool hasTwinConfiguration;
    bool connected;
    bool hubInitiated;
    SyncQueue* twinUpdatesQueue;
    SyncedCounter messageCounter;

} IoTHubAdapter;

/**
 * @brief Initiate a new IoT hub module client
 * 
 * @param   iotHubAdapter       The adapter to initiate.
 * @param   twinUpdatesQueue    The queue which will contain all the twin updates
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, IoTHubAdapter_Init, IoTHubAdapter*, iotHubAdapter, SyncQueue*, twinUpdatesQueue);

/**
 * @brief Deinitialize the IoT hub module client
 * 
 * @param   iotHubAdapter   The adapter to deinitiate.
 */
MOCKABLE_FUNCTION(, void, IoTHubAdapter_Deinit, IoTHubAdapter*, iotHubAdapter);

/**
 * @brief Connect the module client to the hub
 * 
 * @param   iotHubAdapter   The adapter to connect with.
 * 
 * @return  true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, IoTHubAdapter_Connect, IoTHubAdapter*, iotHubAdapter);

/**
 * @brief Send message a-sync to the hub.
 * 
 * @param   iotHubAdapter   The adapter to send data with.
 * @param   data            The data to send.
 * @param   dataSize        The size of the data we want to send.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, IoTHubAdapter_SendMessageAsync, IoTHubAdapter*, iotHubAdapter, const void*, data, size_t, dataSize);

#endif //IOTHUB_ADAPTER_H