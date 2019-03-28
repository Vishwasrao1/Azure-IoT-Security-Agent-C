// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AGENT_TELEMETRY_PROVIDER_H
#define AGENT_TELEMETRY_PROVIDER_H

#include "umock_c_prod.h"
#include "macro_utils.h"

#include "agent_telemetry_counters.h"

/**
 * All the result types of the agent telemetry provider functions
 */
typedef enum _AgentTelemetryProviderResult {
    TELEMETRY_PROVIDER_OK,
    TELEMETRY_PROVIDER_EXCEPTION
} AgentTelemetryProviderResult;

/**
 * Agent metered queues
 */
typedef enum _AgentQueueMeter {
    HIGH_PRIORITY,
    LOW_PRIORITY
} AgentQueueMeter;

/**
 * @brief initialize the agent telemetry provider.
 * 
 * @param lowPriorityQueueCounter   the counter for low priority queue
 * @param highPriorityQueueCounter  the counter for high priority queue
 * @param iotHubCounter             the counter for iot hub sent messages
 * 
 * @return TELEMETRY_PROVIDER_OK on success, TELEMETRY_PROVIDER_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, AgentTelemetryProviderResult, AgentTelemetryProvider_Init, SyncedCounter*, lowPriorityQueueCounter, SyncedCounter*, highPriorityQueueCounter, SyncedCounter*, iotHubCounter);

/**
 * @brief deinitialize the agent telemetry provider.
 */
MOCKABLE_FUNCTION(, void, AgentTelemetryProvider_Deinit);

/**
 * @brief returns counter data of the given queue.
 * 
 * @param   queue               the queue to get the counter data from.
 * @param   counterData         Out param, the counter data of the given queue
 * 
 * @return TELEMETRY_PROVIDER_OK on success, TELEMETRY_PROVIDER_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, AgentTelemetryProviderResult, AgentTelemetryProvider_GetQueueCounterData, AgentQueueMeter, queue, QueueCounter*, counterData);

/**
 * @brief returns counter data of the iot hub client
 * 
 * @param   counterData         Out param, the counter data of the iot hub client
 * 
 * @return TELEMETRY_PROVIDER_OK on success, TELEMETRY_PROVIDER_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, AgentTelemetryProviderResult, AgentTelemetryProvider_GetMessageCounterData, MessageCounter*, counterData);


#endif // AGENT_TELEMETRY_PROVIDER_H