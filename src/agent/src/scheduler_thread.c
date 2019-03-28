// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "scheduler_thread.h"

#include <stdlib.h>
#include <string.h>

#include "logger.h"

/**
 * @brief The main function of the scheduler thread.
 * 
 * @param   params  The params for this thread. Should be the scheduler instance.
 * 
 * @return always 0.
 */
static int SchedulerThread_MainFunc(void* params);

bool SchedulerThread_Init(SchedulerThread* scheduler, uint32_t schedulerInterval, SchedulerTask task, void* taskParam) {
    scheduler->threadHandle = NULL;
    scheduler->schedulerInterval = schedulerInterval;
    scheduler->task = task;
    scheduler->taskParam = taskParam;
    scheduler->continueRunning = true;
    scheduler->state = SCHEDULER_THREAD_CREATED;
    return true;
}

void SchedulerThread_Deinit(SchedulerThread* scheduler) {
    memset(scheduler, 0, sizeof(*scheduler));
}

bool SchedulerThread_Start(SchedulerThread* scheduler) {
    if (scheduler->state != SCHEDULER_THREAD_CREATED) {
        return false;
    }

    if(ThreadAPI_Create(&scheduler->threadHandle, SchedulerThread_MainFunc, (void*)scheduler) != THREADAPI_OK) {
        return false;
    }

    scheduler->state = SCHEDULER_THREAD_STARTED;
    return true;
}

static int SchedulerThread_MainFunc(void* params) {
    if (params == NULL) {
        return 0;
    }

    SchedulerThread* scheduler = (SchedulerThread*)(params);

    while(scheduler->continueRunning) {
        Logger_SetCorrelation();
        scheduler->task(scheduler->taskParam);
        ThreadAPI_Sleep(scheduler->schedulerInterval);
    }

    scheduler->state = SCHEDULER_THREAD_STOPPED;
    return 0;
}

void SchedulerThread_Stop(SchedulerThread* scheduler) {
    scheduler->continueRunning = false;
}

SchedulerThreadState SchedulerThread_GetState(SchedulerThread* scheduler) {
    return scheduler->state;
}