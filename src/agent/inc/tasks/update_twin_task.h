// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UPDATE_TWIN_TASK_H
#define UPDATE_TWIN_TASK_H

#include <stdbool.h>

#include "iothub_adapter.h"
#include "macro_utils.h"
#include "synchronized_queue.h"
#include "umock_c_prod.h"

typedef enum _UpdateTwinState {

    TWIN_COMPLETE,
    TWIN_PARTIAL

} UpdateTwinState;

typedef struct _UpdateTwinTaskItem {

    UpdateTwinState state;
    void* twinPayload;

} UpdateTwinTaskItem;

typedef struct _UpdateTwinTask {

    SyncQueue* updateQueue;
    IoTHubAdapter* iothubClient;

} UpdateTwinTask;

/**
 * @brief Initiates the twin updater task, which update the twin configuration.
 * 
 * @param   task            The task instance to initiate.
 * @param   updateQueue     The queue with the new twin configuration
 * @param   iothubClient    The iothub client to set reported properties with
 * 
 * @return true on uccess, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, UpdateTwinTask_Init, UpdateTwinTask*, task, SyncQueue*, updateQueue, IoTHubAdapter*, client);

/**
 * @brief Initiates the twin task itemfrom a given payload.
 * 
 * @param   twinTaskItem    the task item to be initialized.
 * @param   payload         the data to be initialized with.
 * @param   size            the size of the data.
 * @param   isComplete      whether the update is complete or partial.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, UpdateTwinTask_InitUpdateTwinTaskItem, UpdateTwinTaskItem**, twinTaskItem, const unsigned char*, payload, size_t, size, bool, isComplete);

/**
 * @brief Executes the given task.
 * 
 * @param   task    The instance of the task to execute.
 */
MOCKABLE_FUNCTION(, void, UpdateTwinTask_Execute, UpdateTwinTask*, task);

/**
 * @brief Deinitiates the task
 * 
 * @param   task    The instance to deinitiate.
 */
MOCKABLE_FUNCTION(, void, UpdateTwinTask_Deinit, UpdateTwinTask*, task);

/**
 * @brief Deinitiates the twin task itemfrom a given payload.
 * 
 * @param   twinTaskItem    the task item to be deinitialized.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, void,  UpdateTwinTask_DeinitUpdateTwinTaskItem, UpdateTwinTaskItem**, twinTaskItem);

#endif //UPDATE_TWIN_TASK_H