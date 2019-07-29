// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "tasks/update_twin_task.h"

#include <stdlib.h>

#include "twin_configuration.h"
#include "logger.h"

/**
 * @brief   Updates device twin with new configuration
 * 
 * @param   the iothub client to update the twin with
 * 
 * @return  true upon success
 */
static bool UpdateTwinTask_UpdateTwinReportedProperties(IoTHubAdapter* iothubClient);


bool UpdateTwinTask_Init(UpdateTwinTask* task, SyncQueue* updateQueue, IoTHubAdapter* client) {
    task->updateQueue = updateQueue;
    task->iothubClient = client;
    return true;
}

void UpdateTwinTask_Deinit(UpdateTwinTask* task) {
    void* currentData;
    uint32_t currentDataSize;
    uint32_t queueSize = 0;

    if (SyncQueue_GetSize(task->updateQueue, &queueSize) != QUEUE_OK) {
        goto cleanup;
    }

    while (queueSize > 0) {
        if (SyncQueue_PopFront(task->updateQueue, &currentData, &currentDataSize) != QUEUE_OK) {
            goto cleanup;
        }

        UpdateTwinTask_DeinitUpdateTwinTaskItem(currentData);
        
        if (SyncQueue_GetSize(task->updateQueue, &queueSize) != QUEUE_OK) {
            goto cleanup;
        }
    }
    
cleanup:
    task->updateQueue = NULL;
    task->iothubClient = NULL;
}

void UpdateTwinTask_Execute(UpdateTwinTask* task) {
    //FIXME: we need to have a select for the queue?

    bool success = true;
    UpdateTwinTaskItem* workItem = NULL;

    uint32_t queueSize = 0;
    if (SyncQueue_GetSize(task->updateQueue, &queueSize) != QUEUE_OK) {
        //FIXME: do we want an error here?
        success = false;
        goto cleanup;
    }

    if (queueSize == 0) {
        goto cleanup;
    }
    
    uint32_t bufferSize;
    int queueResult = SyncQueue_PopFront(task->updateQueue, (void**)&workItem, &bufferSize);
    if (queueResult == QUEUE_IS_EMPTY) {
        goto cleanup;
    }
    
    if (queueResult != QUEUE_OK) {
        //FIXME: do we want an error here?
        success = false;
        goto cleanup;
    }

    if (bufferSize != sizeof(UpdateTwinTaskItem)) {
        //FIXME: do we want an error here?
        success = false;
        goto cleanup;
    }

    bool complete = (workItem->state == TWIN_COMPLETE) ? true : false;
    TwinConfigurationResult updateResult = TwinConfiguration_Update(workItem->twinPayload, complete);
    if (updateResult != TWIN_OK) {
        //FIXME: do we want an error here?
        success = false;
        if (updateResult != TWIN_PARSE_EXCEPTION) {
            goto cleanup; // else configuration is valid
        }
    }

    if (UpdateTwinTask_UpdateTwinReportedProperties(task->iothubClient) == false) {
        success = false;
        goto cleanup;
    }

cleanup:
    if (workItem != NULL) {
        if (workItem->twinPayload != NULL) {
            free(workItem->twinPayload);
        }
        free(workItem);        
    }

    if (!success) {
        //FIXME: handle error
    }
}

static bool UpdateTwinTask_UpdateTwinReportedProperties(IoTHubAdapter* iothubClient) {
    bool success = true;
    char* twinJson = NULL;
    uint32_t jsonSize = 0;

    if (TwinConfiguration_GetSerializedTwinConfiguration(&twinJson, &jsonSize) != TWIN_OK) {
        success = false;
        goto cleanup;
    }

    if (IoTHubAdapter_SetReportedPropertiesAsync(iothubClient, twinJson, jsonSize) != true) {
        success = false;
        goto cleanup;
    }

cleanup:
    if (twinJson != NULL){
        free(twinJson);
    }

    return success;
}

bool UpdateTwinTask_InitUpdateTwinTaskItem(UpdateTwinTaskItem** twinTaskItem, const unsigned char* payload, size_t size, bool isComplete) {
    bool result = true;
    *twinTaskItem = malloc(sizeof(UpdateTwinTaskItem));
    UpdateTwinTaskItem* twinItem = *twinTaskItem;

    if (twinItem == NULL) {
        Logger_Error("Bad allocation");
        //FIXME: how to handlebad allocation?
        result = false;
        goto cleanup;
    }
    memset(twinItem, 0, sizeof(*twinItem));

    twinItem->twinPayload = malloc(size + 1);
    if (twinItem->twinPayload == NULL) {
        Logger_Error("Bad allocation");
        //FIXME: how to handlebad allocation?
        result = false;
        goto cleanup;
    }
    memcpy(twinItem->twinPayload, payload, size);
    ((char*)(twinItem->twinPayload))[size] = '\x0';
    
    if (isComplete) {
        twinItem->state = TWIN_COMPLETE;
    } else {
        twinItem->state = TWIN_PARTIAL;
    }

cleanup:
    if (!result && twinItem != NULL) {
        UpdateTwinTask_DeinitUpdateTwinTaskItem(&twinItem);
    }

    return result;
}

void UpdateTwinTask_DeinitUpdateTwinTaskItem(UpdateTwinTaskItem** twinTaskItem) {
    UpdateTwinTaskItem* twinItem = *twinTaskItem;
    if (twinItem->twinPayload != NULL) {
        free(twinItem->twinPayload);
        twinItem->twinPayload = NULL;
    }
    free(*twinTaskItem);
    *twinTaskItem = NULL;
}