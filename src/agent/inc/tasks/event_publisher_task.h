// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef EVENT_PUBLISHER_TASK_H
#define EVENT_PUBLISHER_TASK_H

#include <stdbool.h>
#include <time.h>

#include "iothub_adapter.h"
#include "synchronized_queue.h"

typedef struct _EventPublisherTask {

    SyncQueue* operationalEventsQueue;
    SyncQueue* lowPriorityEventQueue;
    SyncQueue* highPriorityEventQueue;
    time_t highPriorityQueueLastExecution;
    time_t lowPriorityQueueLastExecution;
    IoTHubAdapter* iothubAdapter;

} EventPublisherTask;

/**
 * @brief Initiates the task
 * 
 * @param   task                        The task instance to initiate.
 * @param   highPriorityEventQueue      The event queue which contains all high priority events.
 * @param   lowPriorityEventQueue       The event queue which contains all low priority events.
 * @param   operationalEventsQueue      The operational events which contains all operational events.
 * @param   iothubAdapter               The iot hab adapter to use in order to send messages.
 * 
 * @return true on success, false otherwise.
 */
 bool EventPublisherTask_Init(EventPublisherTask* task, SyncQueue* highPriorityEventQueue, SyncQueue* lowPriorityEventQueue, SyncQueue* operationalEventsQueue, IoTHubAdapter* iothubAdapter);

/**
 * @brief Deinitiates the instance.
 * 
 * @param   task    The instacne to deinitiate.
 */
void EventPublisherTask_Deinit(EventPublisherTask* task);

/**
 * @brief Executes the given task.
 * 
 * @param   task    The instance of the task to execute.
 */
void EventPublisherTask_Execute(EventPublisherTask* task);

#endif //EVENT_PUBLISHER_TASK_H