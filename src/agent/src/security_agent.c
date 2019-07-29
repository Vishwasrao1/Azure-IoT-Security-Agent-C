// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "security_agent.h"

#include "os_utils/system_logger.h"
#include "agent_telemetry_provider.h"
#include "iothub.h"
#include "iothub_adapter.h"
#include "local_config.h"
#include "logger.h"
#include "memory_monitor.h"
#include "os_utils/process_info_handler.h"
#include "twin_configuration.h"

/**
 * @brief Deinitiate the given queue only if the initiated flag is on.
 * 
 * @param   queue               The queue to deinitiate.
 * @param   queueInitiated      A flag which indicates whether the queue was initiated.
 */
void SecurityAgent_DeinitQueue(SyncQueue* queue, bool queueInitiated);

/**
 * @brief Initiate the given queue and change the flag accordingly.
 * 
 * @param   queue               The queue to initiate.
 * @param   queueInitiated      Out param. A falg which indicates whether the queue was initiated.
 * 
 * @return true upon successful queue initialization, false otherwise.
 */
bool SecurityAgent_InitQueue(SyncQueue* queue, bool* queueInitiated, bool shouldSendLogs);

/**
 * @brief Initiate all the queues of the agent.
 * 
 * @param   agent    The agent instance,
 * 
 * @return true upon successful queue initialization, false otherwise.
 */
bool SecurityAgent_InitAllQueues(SecurityAgent* agent);

/**
 * @brief Stops the given async task (stops the thread, wait and then deinitite it).
 * 
 * @param   asyncTask    The task to stop.
 */
void SecurityAgent_StopAsyncTask(SecurityAgentAsyncTask* asyncTask);

/**
 * @brief Connects the agent to the iot hub and retrives the initial configuration from the hub.
 * 
 * @param   agent    The agent instance,
 * 
 * @return true in case of success, false otherwise.
 */
bool SecurityAgent_ConnectAndUpdateConfiguration(SecurityAgent* agent);

/**
 * @brief Start the given instance of the scheduler (creates and run the thread).
 * 
 * @param   asyncTask       The async task to start.
 * @param   interval        The interval for teh scheduler.
 * @param   taskFunction    The function the scheduler should run.
 * @param   taskParam       The parameters for the given task.
 * 
 * @return ture if the thread was created and started, false otherwise.
 */
bool SecurityAgent_StartAsyncTask(SecurityAgentAsyncTask* asyncTask, uint32_t interval, SchedulerTask taskFunction, void* taskParam);

bool SecurityAgent_Init(SecurityAgent* agent) {
    bool success = true;
    memset(agent, 0, sizeof(*agent));

    if (!Logger_Init()) {
        success = false;
        goto cleanup;
    }
    agent->loggerInitiated = true;
    
    if (IoTHub_Init() != 0) {
        success = false;
        goto cleanup;
    }
    agent->iothubInitiated = true;

    if (LocalConfiguration_Init() != LOCAL_CONFIGURATION_OK) {
        success = false;
        goto cleanup;
    }
    agent->localConfigurationInitiated = true;

    Logger_SetMinimumSeverityForSystemLogger(LocalConfiguration_GetSystemLoggerMinimumSeverity());
    Logger_SetMinimumSeverityForDiagnosticEvent(LocalConfiguration_GetDiagnosticEventMinimumSeverity());

    if (!MemoryMonitor_Init()) {
        success = false;
        goto cleanup;
    }
    agent->memoryMonitorInitiated = true;

    if (TwinConfiguration_Init() != TWIN_OK) {
        success = false;
        goto cleanup;
    }
    agent->twinConfigurationInitiated = true;

    if (!ProcessInfoHandler_SwitchRealAndEffectiveUsers()) {
        success = false;
        goto cleanup;
    }

    if (!SecurityAgent_InitAllQueues(agent)) {
        success = false;
        goto cleanup;
    }

    if (DiagnosticEventCollector_Init(&agent->queues.diagnosticEventQueue) != EVENT_COLLECTOR_OK) {
        success = false;
        goto cleanup;
    }

    agent->diagnosticEventCollectorInitiated = true;
    Logger_SetCorrelation();

    if (AgentTelemetryProvider_Init(&agent->queues.lowPriorityEventQueue.queue.counter, &agent->queues.highPriorityEventQueue.queue.counter, &agent->iothubAdapter.messageCounter) != TELEMETRY_PROVIDER_OK){
        success = false;
        goto cleanup;
    }

    if (!IoTHubAdapter_Init(&agent->iothubAdapter, &agent->queues.twinUpdatesQueue)) {
        Logger_Error("Failed on iothub_adapter_init");
        success = false;
        goto cleanup;
    }
    agent->iothubAdapterInitiated = true;
cleanup:
    if (!success) {
        SecurityAgent_Deinit(agent);
    }
    return success;
}

void SecurityAgent_Deinit(SecurityAgent* agent) {

    SecurityAgent_StopAsyncTask(&agent->asyncPublisherTask);
    if (agent->asyncPublisherTask.taskInitiated) {
        EventPublisherTask_Deinit(&agent->publisherTask);
    }

    SecurityAgent_StopAsyncTask(&agent->asyncMonitorTask);
    if (agent->asyncMonitorTask.taskInitiated) {
        EventMonitorTask_Deinit(&agent->monitorTask);
    }

    SecurityAgent_StopAsyncTask(&agent->asyncUpdateTwinTask);
    if (agent->asyncUpdateTwinTask.taskInitiated) {
        UpdateTwinTask_Deinit(&agent->updateTwinTask);
    }

    if (agent->iothubAdapterInitiated) {
        IoTHubAdapter_Deinit(&agent->iothubAdapter);
    }

    if (agent->twinConfigurationInitiated) {
        TwinConfiguration_Deinit();
    }
    AgentTelemetryProvider_Deinit();

    if (agent->diagnosticEventCollectorInitiated) {
        DiagnosticEventCollector_Deinit();
    }

    SecurityAgent_DeinitQueue(&agent->queues.highPriorityEventQueue, agent->queues.highPriorityEventQueueInitiated);
    SecurityAgent_DeinitQueue(&agent->queues.lowPriorityEventQueue, agent->queues.lowPriorityEventQueueInitiated);
    SecurityAgent_DeinitQueue(&agent->queues.twinUpdatesQueue, agent->queues.twinUpdatesQueueInitiated);
    SecurityAgent_DeinitQueue(&agent->queues.operationalEventsQueue, agent->queues.operationalEventsQueueInitiated);
    SecurityAgent_DeinitQueue(&agent->queues.diagnosticEventQueue, agent->queues.diagnosticEventQueueInitiated);

    if (agent->memoryMonitorInitiated) {
        MemoryMonitor_Deinit();
    }

    if (agent->localConfigurationInitiated) {
        LocalConfiguration_Deinit();
    }

    if (agent->iothubInitiated) {
        IoTHub_Deinit();
    }

    if (agent->loggerInitiated) {
        Logger_Deinit();
    }
}

bool SecurityAgent_Start(SecurityAgent* agent) {
      // init twin update task
    if (!UpdateTwinTask_Init(&agent->updateTwinTask, &agent->queues.twinUpdatesQueue, &agent->iothubAdapter)) {
        return false;
    }
    agent->asyncUpdateTwinTask.taskInitiated = true;

    if (!SecurityAgent_ConnectAndUpdateConfiguration(agent)) {
        return false;
    }
    
    // init & start event published
    if (!EventPublisherTask_Init(&agent->publisherTask, &agent->queues.highPriorityEventQueue, &agent->queues.lowPriorityEventQueue, &agent->queues.operationalEventsQueue, &agent->iothubAdapter)) {
        return false;
    }
    agent->asyncPublisherTask.taskInitiated = true;
    if (!SecurityAgent_StartAsyncTask(&agent->asyncPublisherTask, SCHEDULER_INTERVAL, (SchedulerTask)EventPublisherTask_Execute, &agent->publisherTask)) {
        return false;
    }

    // init & start event monitor
    if (!EventMonitorTask_Init(&agent->monitorTask, &agent->queues.highPriorityEventQueue, &agent->queues.lowPriorityEventQueue, &agent->queues.operationalEventsQueue)) {
        return false;
    }
    if (!SecurityAgent_StartAsyncTask(&agent->asyncMonitorTask, SCHEDULER_INTERVAL, (SchedulerTask)EventMonitorTask_Execute, &agent->monitorTask)) {
        return false;
    }

    // start twin updater
    if (!SecurityAgent_StartAsyncTask(&agent->asyncUpdateTwinTask, TWIN_UPDATE_SCHEDULER_INTERVAL, (SchedulerTask)UpdateTwinTask_Execute, &agent->updateTwinTask)) {
        return false;
    }
    Logger_Information("ASC for IoT Agent initialized!");
    return true;
}

void SecurityAgent_Wait(SecurityAgent* agent) {
    int result;
    ThreadAPI_Join(agent->asyncPublisherTask.taskThread.threadHandle, &result);
    ThreadAPI_Join(agent->asyncMonitorTask.taskThread.threadHandle, &result);
    SchedulerThread_Stop(&agent->asyncUpdateTwinTask.taskThread);
    ThreadAPI_Join(agent->asyncUpdateTwinTask.taskThread.threadHandle, &result);
}

void SecurityAgent_Stop(SecurityAgent* agent) {
    SchedulerThread_Stop(&agent->asyncPublisherTask.taskThread);
    SchedulerThread_Stop(&agent->asyncMonitorTask.taskThread);

    SecurityAgent_Wait(agent);
}

bool SecurityAgent_InitQueue(SyncQueue* queue, bool* queueInitiated, bool shouldSendLogs) {
    if (SyncQueue_Init(queue, shouldSendLogs) != QUEUE_OK) {
        return false;
    }
    *queueInitiated = true;
    return true;
}

void SecurityAgent_DeinitQueue(SyncQueue* queue, bool queueInitiated) {
    if (queueInitiated) {
        SyncQueue_Deinit(queue);
    }
}

bool SecurityAgent_InitAllQueues(SecurityAgent* agent) {
    if (!SecurityAgent_InitQueue(&agent->queues.diagnosticEventQueue, &agent->queues.diagnosticEventQueueInitiated, false)) {
        return false;
    }

    if (!SecurityAgent_InitQueue(&agent->queues.operationalEventsQueue, &agent->queues.operationalEventsQueueInitiated, true)) {
        return false;
    }

    if (!SecurityAgent_InitQueue(&agent->queues.highPriorityEventQueue, &agent->queues.highPriorityEventQueueInitiated, true)) {
        return false;
    }

    if (!SecurityAgent_InitQueue(&agent->queues.lowPriorityEventQueue, &agent->queues.lowPriorityEventQueueInitiated, true)) {
        return false;
    }

    if (!SecurityAgent_InitQueue(&agent->queues.twinUpdatesQueue, &agent->queues.twinUpdatesQueueInitiated, true)) {
        return false;
    }

    return true;
}

bool SecurityAgent_ConnectAndUpdateConfiguration(SecurityAgent* agent) {
    bool success = true;

    if (!IoTHubAdapter_Connect(&agent->iothubAdapter)) {
        Logger_Error("Failed on iothub_adapter_connect");
        success = false;
        goto cleanup;
    }

    uint32_t twinUpdatesQueueSize = 0;
    if (SyncQueue_GetSize(&agent->queues.twinUpdatesQueue, &twinUpdatesQueueSize) != QUEUE_OK) {
        success = false;
        goto cleanup;
    }
    if (twinUpdatesQueueSize == 0) {
        Logger_Warning("Connect finished but no twin configuration was found.");
        success = false;
        goto cleanup;
    }

    UpdateTwinTask_Execute(&agent->updateTwinTask);

cleanup:
    return success;
}

bool SecurityAgent_StartAsyncTask(SecurityAgentAsyncTask* asyncTask, uint32_t interval, SchedulerTask taskFunction, void* taskParam) {
    if (!SchedulerThread_Init(&asyncTask->taskThread, interval, taskFunction, taskParam)) {
        return false;
    }
    asyncTask->taskThreadInitiated = true;

    if (!SchedulerThread_Start(&asyncTask->taskThread)) {
        Logger_Error("Error starting thread");
        return false;
    }

    return true;
}

void SecurityAgent_StopAsyncTask(SecurityAgentAsyncTask* asyncTask) {
    if (asyncTask->taskThreadInitiated) {
        if (SchedulerThread_GetState(&asyncTask->taskThread) == SCHEDULER_THREAD_STARTED) {
            SchedulerThread_Stop(&asyncTask->taskThread);
            int result;
            ThreadAPI_Join(asyncTask->taskThread.threadHandle, &result);
        }
        SchedulerThread_Deinit(&asyncTask->taskThread);
    }
}