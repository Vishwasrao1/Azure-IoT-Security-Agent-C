// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SCHEDULER_THREAD_H
#define SCHEDULER_THREAD_H

#include <stdbool.h>
#include <stdint.h>

#include "azure_c_shared_utility/threadapi.h"

typedef void (*SchedulerTask)(void* params);

typedef enum _SchedulerThreadState {
    SCHEDULER_THREAD_CREATED,
    SCHEDULER_THREAD_STARTED,
    SCHEDULER_THREAD_STOPPED
} SchedulerThreadState;

typedef struct _SchedulerThread {

    THREAD_HANDLE threadHandle;
    uint32_t schedulerInterval;
    SchedulerTask task;
    void* taskParam;
    bool continueRunning;
    SchedulerThreadState state;

} SchedulerThread;

/**
 * @brief Initiates the a new scheduler instance.
 * 
 * @param   scheduler           Out param. The instance to initiate.
 * @param   schedulerInterval   The intravle between the task execution.
 * @param   task                The task to execute.
 * @param   taskParam           The parameters to the task.
 * 
 * @return true on success, false otherwise.  The value of the out params is undefined in case of failure.
 */
bool SchedulerThread_Init(SchedulerThread* scheduler, uint32_t schedulerInterval, SchedulerTask task, void* taskParam);

/**
 * @brief Deinitiates the scheduler instance.
 * 
 * @param   scheduler   The instance to deinitiate.
 */
void SchedulerThread_Deinit(SchedulerThread* scheduler);

/**
 * @brief Starts the scheduler.
 * 
 * @param   scheduler    The scheduler instance.
 * 
 * @return true on success, false otherwise.
 */
bool SchedulerThread_Start(SchedulerThread* scheduler);

/**
 * @brief Mark the scheduler to stop.
 * 
 * @param   scheduler   The scheduler instance.
 */
void SchedulerThread_Stop(SchedulerThread* scheduler);

/**
 * @brief returns the state of the thread (e.g. created\started\stopped).
 * 
 * @param   scheduler   The scheduler instance.
 * 
 * @return The state of the thread.
 */
SchedulerThreadState SchedulerThread_GetState(SchedulerThread* scheduler);

#endif //SCHEDULER_THREAD_H