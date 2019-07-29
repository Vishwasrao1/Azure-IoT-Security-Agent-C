// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iothub_adapter.h"

#include "test_defs.h"
#include "tasks/update_twin_task.h"
#include "queue.h"

bool IoTHubAdapter_Init(IoTHubAdapter* iotHubAdapter, SyncQueue* twinUpdatesQueue) {
    iotHubAdapter->twinUpdatesQueue = twinUpdatesQueue;
    return true;
}

void IoTHubAdapter_Deinit(IoTHubAdapter* iotHubAdapter) {
    // no implementation 
}

bool IoTHubAdapter_Connect(IoTHubAdapter* iotHubAdapter) {
    bool success = true;
    UpdateTwinTaskItem* twinTaskItem = NULL;
    UpdateTwinTask_InitUpdateTwinTaskItem(&twinTaskItem, currentTwinConfiguration, strlen(currentTwinConfiguration), true);

    if (SyncQueue_PushBack(iotHubAdapter->twinUpdatesQueue, twinTaskItem, sizeof(UpdateTwinTaskItem)) != QUEUE_OK) {
        success = false;
        goto cleanup;
    }

cleanup:
    if (!success) {
        if (twinTaskItem != NULL) {
            UpdateTwinTask_DeinitUpdateTwinTaskItem(&twinTaskItem);
        }
    }
    iotHubAdapter->hasTwinConfiguration = success;

    return success;
}

bool IoTHubAdapter_SendMessageAsync(IoTHubAdapter* iotHubAdapter, const void* data, size_t dataSize) {

    if (Lock(sentMessages.lock) != LOCK_OK) {
        return false;
    }

    sentMessages.items[sentMessages.index].data = strdup(data);
    sentMessages.items[sentMessages.index].dataSize = dataSize;
    sentMessages.index++;

    if (Unlock(sentMessages.lock) != LOCK_OK) {
        return false;
    }

    return true;
}

bool IoTHubAdapter_SetReportedPropertiesAsync(IoTHubAdapter* iotHubAdapter, const void* reportedData, size_t dataSize) {
    return true;
}