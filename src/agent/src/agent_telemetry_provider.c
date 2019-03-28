// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include "agent_telemetry_provider.h"

/*
 * Agent telemetry provider object definition
 */
typedef struct _AgentTelemetryProvider {
    SyncedCounter* lowPriorityQueueCounter;
    SyncedCounter* highPriorityQueueCounter;
    SyncedCounter* iotHubCounter;
} AgentTelemetryProvider;

/*
 * Agent telemetry provider instance
 */
static AgentTelemetryProvider agentTelemetryProvider = { 0 };

AgentTelemetryProviderResult AgentTelemetryProvider_Init(SyncedCounter* lowPriorityQueueCounter, SyncedCounter* highPriorityQueueCounter, SyncedCounter* iotHubCounter) {
    agentTelemetryProvider.lowPriorityQueueCounter = lowPriorityQueueCounter;
    agentTelemetryProvider.highPriorityQueueCounter = highPriorityQueueCounter;
    agentTelemetryProvider.iotHubCounter = iotHubCounter;
    
    return TELEMETRY_PROVIDER_OK;
}

void AgentTelemetryProvider_Deinit() {
    agentTelemetryProvider.lowPriorityQueueCounter = NULL;
    agentTelemetryProvider.highPriorityQueueCounter = NULL;;
    agentTelemetryProvider.iotHubCounter = NULL;
}

AgentTelemetryProviderResult AgentTelemetryProvider_GetQueueCounterData(AgentQueueMeter queue, QueueCounter* counterData) {
    AgentTelemetryProviderResult result = TELEMETRY_PROVIDER_OK;
    Counter data;
    if (queue == HIGH_PRIORITY){
        if (AgentTelemetryCounter_SnapshotAndReset(agentTelemetryProvider.highPriorityQueueCounter, &data) == false){
            result = TELEMETRY_PROVIDER_EXCEPTION;
            goto cleanup;
        } 
    } else if (queue == LOW_PRIORITY){
        if (AgentTelemetryCounter_SnapshotAndReset(agentTelemetryProvider.lowPriorityQueueCounter, &data) == false) {
            result = TELEMETRY_PROVIDER_EXCEPTION;
            goto cleanup;
        }
    }

    counterData->collected = data.queueCounter.collected;
    counterData->dropped = data.queueCounter.dropped;
cleanup:
    return result;
}

AgentTelemetryProviderResult AgentTelemetryProvider_GetMessageCounterData(MessageCounter* counterData) {
    AgentTelemetryProviderResult result = TELEMETRY_PROVIDER_OK;
    Counter data;
    if (AgentTelemetryCounter_SnapshotAndReset(agentTelemetryProvider.iotHubCounter, &data) == false) {
            result = TELEMETRY_PROVIDER_EXCEPTION;
            goto cleanup;
    } 
    
    counterData->failedMessages = data.messageCounter.failedMessages;
    counterData->smallMessages = data.messageCounter.smallMessages;
    counterData->sentMessages = data.messageCounter.sentMessages;
cleanup:
    return result;
}