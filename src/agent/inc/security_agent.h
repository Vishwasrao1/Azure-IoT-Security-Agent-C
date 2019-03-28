// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SECURITY_AGENT_H
#define SECURITY_AGENT_H

#include <stdbool.h>

#include "iothub_adapter.h"
#include "scheduler_thread.h"
#include "synchronized_queue.h"
#include "tasks/event_monitor_task.h"
#include "tasks/event_publisher_task.h"
#include "tasks/update_twin_task.h"

typedef struct _SecurityAgentAsyncTask {

    bool taskInitiated;
    SchedulerThread taskThread;
    bool taskThreadInitiated;

} SecurityAgentAsyncTask;

typedef struct _SecurityAgentQueues {
    SyncQueue operationalEventsQueue;
    bool operationalEventsQueueInitiated;

    SyncQueue highPriorityEventQueue;
    bool highPriorityEventQueueInitiated;

    SyncQueue lowPriorityEventQueue;
    bool lowPriorityEventQueueInitiated;

    SyncQueue diagnosticEventQueue;
    bool diagnosticEventQueueInitiated;

    SyncQueue twinUpdatesQueue;
    bool twinUpdatesQueueInitiated;

} SecurityAgentQueues;


typedef struct _SecurityAgent {

    SecurityAgentQueues queues;

    EventMonitorTask monitorTask;
    SecurityAgentAsyncTask asyncMonitorTask;

    EventPublisherTask publisherTask;
    SecurityAgentAsyncTask asyncPublisherTask;

    UpdateTwinTask updateTwinTask;
    SecurityAgentAsyncTask asyncUpdateTwinTask;

    IoTHubAdapter iothubAdapter;
    bool iothubAdapterInitiated;

    bool memoryMonitorInitiated;
    bool twinConfigurationInitiated;
    bool localConfigurationInitiated;
    bool diagnosticEventCollectorInitiated;

} SecurityAgent;

/**
 * @brief Initiates a new security agent.
 * 
 * @param   agent   The security agent istance.
 * 
 * @return true on success, false otherwise.
 */
bool SecurityAgent_Init(SecurityAgent* agent);

/**
 * @brief Deinitiates a new security agent.
 * 
 * @param   agent   The security agent istance.
 */
void SecurityAgent_Deinit(SecurityAgent* agent);

/**
 * @brief Starts the security agent (monitoring and sending security events).
 * 
 * @param   agent   The security agent istance.
 * 
 * @return true on success, false otherwise.
 */
bool SecurityAgent_Start(SecurityAgent* agent);

/**
 * @brief Waits for the agent to finish it run. This function blocks until the agent is done.
 * 
 * @param   agent   The security agent istance.
 */
void SecurityAgent_Wait(SecurityAgent* agent);

/**
 * @brief Stops the agent.
 * 
 * @param   agent   The security agent istance.
 */
void SecurityAgent_Stop(SecurityAgent* agent);

#endif //SECURITY_AGENT_H