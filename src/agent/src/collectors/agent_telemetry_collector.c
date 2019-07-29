// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/agent_telemetry_collector.h"

#include <stdlib.h>

#include "agent_telemetry_provider.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "message_schema_consts.h"

const char* HIGH_PRIO_QUEUE_NAME = "High";
const char* LOW_PRIO_QUEUE_NAME = "Low";

/*
 * @brief serializes the event and push it to the queue
 * 
 * @param   queue       the queue to push the event to
 * @param   eventHandle the event to push
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentTelemetryCollector_PushEvent(SyncQueue* queue, JsonObjectWriterHandle eventHandle);

/*
 * @brief creates new dropped event payload
 * 
 * @param   queueCounterData       the counter data to create the payload from
 * @param   queueName              the queue name of the current counter data
 * @param   JsonArrayWriterHandle  payload handle, the payload will be written in this payload object
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentTelemetryCollector_AddDroppedEventsStatsPayload(QueueCounter* queueCounterData, const char* queueName, JsonArrayWriterHandle payloadWriter);

/*
 * @brief creates new message statistics payload
 * 
 * @param   queueCounterData       the counter data to create the payload from
 * @param   JsonArrayWriterHandle  payload handle, the payload will be written in this payload object
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentTelemetryCollector_AddMessageStatisticsPayload(MessageCounter* counterData,JsonArrayWriterHandle payloadHandle);

/*
 * @brief creates new dropped event and push it to the queue
 * 
 * @param   queue       the queue to push the event to.
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentTelemetryCollector_AddDroppedEventsEvent(SyncQueue* queue);

/*
 * @brief creates new message statistics and push it to the queue
 * 
 * @param   queue       the queue to push the event to.
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentTelemetryCollector_AddMessageStatisticsEvent(SyncQueue* queue);

EventCollectorResult AgentTelemetryCollector_GetEvents(SyncQueue* priorityQueue) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    result = AgentTelemetryCollector_AddDroppedEventsEvent(priorityQueue);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    result = AgentTelemetryCollector_AddMessageStatisticsEvent(priorityQueue);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

cleanup:
    return result;
}

EventCollectorResult AgentTelemetryCollector_AddQueueCounterPayload(JsonArrayWriterHandle payloadHandle, AgentQueueMeter queueMeter){
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    QueueCounter counterData = {0};
    if (AgentTelemetryProvider_GetQueueCounterData(queueMeter, &counterData) != TELEMETRY_PROVIDER_OK){
        result =  EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    const char* queueName = queueMeter == HIGH_PRIORITY ? HIGH_PRIO_QUEUE_NAME : LOW_PRIO_QUEUE_NAME;
    result = AgentTelemetryCollector_AddDroppedEventsStatsPayload(&counterData, queueName, payloadHandle);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }
cleanup:
    return result;
}

EventCollectorResult AgentTelemetryCollector_AddDroppedEventsEvent(SyncQueue* queue){
    JsonObjectWriterHandle eventHandle = NULL;
    JsonArrayWriterHandle payloadHandle = NULL;
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    if (JsonObjectWriter_Init(&eventHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddMetadata(eventHandle, EVENT_PERIODIC_CATEGORY, AGENT_TELEMETRY_DROPPED_EVENTS_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_TELEMETRY_DROPPED_EVENTS_SCHEMA_VERSION);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&payloadHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = AgentTelemetryCollector_AddQueueCounterPayload(payloadHandle, HIGH_PRIORITY);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    result = AgentTelemetryCollector_AddQueueCounterPayload(payloadHandle, LOW_PRIORITY);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }
    
    result = GenericEvent_AddPayload(eventHandle, payloadHandle);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    result = AgentTelemetryCollector_PushEvent(queue, eventHandle);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

cleanup:
    if (payloadHandle){
        JsonArrayWriter_Deinit(payloadHandle);
    }

    if (eventHandle){
        JsonObjectWriter_Deinit(eventHandle);
    }

    return result;
}

EventCollectorResult AgentTelemetryCollector_AddMessageStatisticsEvent(SyncQueue* queue){
    JsonObjectWriterHandle eventHandle = NULL;
    JsonArrayWriterHandle payloadHandle = NULL;
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    if (JsonObjectWriter_Init(&eventHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddMetadata(eventHandle, EVENT_PERIODIC_CATEGORY, AGENT_TELEMETRY_MESSAGE_STATISTICS_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_TELEMETRY_MESSAGE_STATISTICS_SCHEMA_VERSION);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&payloadHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    MessageCounter counterData;
    if (AgentTelemetryProvider_GetMessageCounterData(&counterData) != TELEMETRY_PROVIDER_OK){
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = AgentTelemetryCollector_AddMessageStatisticsPayload(&counterData, payloadHandle);
    if (result != EVENT_COLLECTOR_OK){
            goto cleanup;
    }

    result = GenericEvent_AddPayload(eventHandle, payloadHandle);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

    result = AgentTelemetryCollector_PushEvent(queue, eventHandle);
    if (result != EVENT_COLLECTOR_OK){
        goto cleanup;
    }

cleanup:
    if (payloadHandle != NULL){
        JsonArrayWriter_Deinit(payloadHandle);
    }

    if (eventHandle != NULL){
        JsonObjectWriter_Deinit(eventHandle);
    }

    return result;
}

EventCollectorResult AgentTelemetryCollector_PushEvent(SyncQueue* queue, JsonObjectWriterHandle eventHandle){
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    char* buffer = NULL;
    uint32_t bufferSize = 0;

    if (JsonObjectWriter_Serialize(eventHandle, &buffer, &bufferSize) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (SyncQueue_PushBack(queue, buffer, bufferSize) != 0) {
        result = EVENT_COLLECTOR_EXCEPTION;        
        free(buffer);
        goto cleanup;
    }
cleanup:
    return result;
}

EventCollectorResult AgentTelemetryCollector_AddDroppedEventsStatsPayload(QueueCounter* queueCounterData, const char* queueName, JsonArrayWriterHandle payloadWriter){
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle payloadObject = NULL;

    if (JsonObjectWriter_Init(&payloadObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(payloadObject, AGENT_TELEMETRY_QUEUE_EVENTS_KEY, queueName) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(payloadObject, AGENT_TELEMETRY_COLLECTED_EVENTS_KEY, queueCounterData->collected) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(payloadObject, AGENT_TELEMETRY_DROPPED_EVENTS_KEY, queueCounterData->dropped) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION; 
        goto cleanup;
    }
    
    if (JsonArrayWriter_AddObject(payloadWriter, payloadObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (payloadObject != NULL) {
        JsonObjectWriter_Deinit(payloadObject);
    }

    return result;
}

EventCollectorResult AgentTelemetryCollector_AddMessageStatisticsPayload(MessageCounter* counterData,JsonArrayWriterHandle payloadHandle){
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle payloadObject = NULL;

    if (JsonObjectWriter_Init(&payloadObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(payloadObject, AGENT_TELEMETRY_MESSAGES_SENT_KEY, counterData->sentMessages) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(payloadObject, AGENT_TELEMETRY_MESSAGES_FAILED_KEY, counterData->failedMessages) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(payloadObject, AGENT_TELEMETRY_MESSAGES_UNDER_4KB_KEY, counterData->smallMessages) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if (JsonArrayWriter_AddObject(payloadHandle, payloadObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (payloadObject != NULL) {
        JsonObjectWriter_Deinit(payloadObject);
    }

    return result;
}