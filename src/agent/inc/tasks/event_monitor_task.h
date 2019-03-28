// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef EVENT_MONITOR_TASK_H
#define EVENT_MONITOR_TASK_H

#include <stdbool.h>
#include <time.h>

#include "synchronized_queue.h"

typedef struct _EventMonitorTask {
    
    SyncQueue* operationalEventsQueue;
    SyncQueue* highPriorityQueue;
    SyncQueue* lowPriorityQueue;
    time_t lastPeriodicExecution;
    time_t lastTriggeredExecution;

} EventMonitorTask;

/**
 * @brief Initiates the event monitor task
 * 
 * @param   task                        The task instance to initiate.
 * @param   highPriorityQueue           The high priority event queue.
 * @param   lowPriorityQueue            The low priority event queue.
 * @param   operationalEventsQueue      The operational events queue.
 * 
 * @return true on uccess, false otherwise.
 */
bool EventMonitorTask_Init(EventMonitorTask* task, SyncQueue* highPriorityQueue, SyncQueue* lowPriorityQueue, SyncQueue* operationalEventsQueue);

/**
 * @brief Executes the given task.
 * 
 * @param   task    The instance of the task to execute.
 */
void EventMonitorTask_Execute(EventMonitorTask* task);

/**
 * @brief Deinitiates the task
 * 
 * @param   task    The instance to deinitiate.
 */
void EventMonitorTask_Deinit(EventMonitorTask* task);

#endif //EVENT_MONITOR_TASK_H