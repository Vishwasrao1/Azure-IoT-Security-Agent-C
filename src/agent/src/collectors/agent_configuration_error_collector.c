// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/agent_configuration_error_collector.h"

#include <stdlib.h>
#include <stdio.h>

#include "agent_telemetry_provider.h"
#include "consts.h"
#include "internal/time_utils.h"
#include "internal/time_utils_consts.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "message_schema_consts.h"
#include "twin_configuration.h"
#include "twin_configuration_consts.h"
#include "utils.h"

const char* ERROR_TYPE_CONFLICT = "Conflict";
const char* ERROR_TYPE_NOT_OPTIMAL = "NotOptimal";
const char* ERROR_TYPE_TYPE_MISMATCH = "TypeMismatch";

static time_t lastEvent = {0};

/*
 * @brief serializes the event and push it to the queue
 * 
 * @param   queue       the queue to push the event to
 * @param   eventHandle the event to push
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentConfigurationErrorCollector_PushEvent(SyncQueue* queue, JsonObjectWriterHandle eventHandle);

/*
 * @brief creates a type mismatch message according to the given configuratio budnle status
 * 
 * @param configurationBundleStatus     the configuration bundle status to create the message from
 * @param msgBuffer                     buffer to write the message
 * @param size                          buffer size
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentConfigurationErrorCollector_GenerateTypeMismatchMessage(TwinConfigurationBundleStatus* configurationBundleStatus, char* msgBuffer, uint32_t size) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    if (Utils_ConcatenateToString(&msgBuffer, &size, "Couldn't parse the following conifgurations:") == false){
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    char* format = " %s,";
    if (configurationBundleStatus->maxLocalCacheSize == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, MAX_LOCAL_CACHE_SIZE_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }

    if (configurationBundleStatus->maxMessageSize == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, MAX_MESSAGE_SIZE_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }

    if (configurationBundleStatus->lowPriorityMessageFrequency == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, LOW_PRIORITY_MESSAGE_FREQUENCY_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }
    if (configurationBundleStatus->highPriorityMessageFrequency == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, HIGH_PRIORITY_MESSAGE_FREQUENCY_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }
    if (configurationBundleStatus->snapshotFrequency == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, SNAPSHOT_FREQUENCY_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }
    if (configurationBundleStatus->eventPriorities == CONFIGURATION_TYPE_MISMATCH) {
        if (Utils_ConcatenateToString(&msgBuffer, &size, format, EVENT_PROPERTIES_KEY) == false){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        } 
    }

cleanup:
    if (result == EVENT_COLLECTOR_OK) {
        *(msgBuffer - 2) = 0; //remove last , (comma)
    }

    return result;
}

/* @brief Adds a single configuration error payload item to the given writer
 * 
 * @param   payloadHandle       the handle to write the payload to
 * @param   configurationName   the configuration that caused an error
 * @param   error               the error 
 * @param   msg                 the related message to the error
 * @param   usedConfig          the currently used configuration
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentConfigurationErrorCollector_AddPayload(JsonArrayWriterHandle payloadHandle, const char* configurationName, const char* error, const char* msg, const char* usedConfig){
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle payloadObject = NULL;
    if (JsonObjectWriter_Init(&payloadObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(payloadObject, AGENT_CONFIGURATION_ERROR_CONFIGURATION_NAME_KEY, configurationName) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(payloadObject, AGENT_CONFIGURATION_ERROR_USED_CONFIGURATION_KEY, usedConfig) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(payloadObject, AGENT_CONFIGURATION_ERROR_MESSAGE_KEY, msg) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if (JsonObjectWriter_WriteString(payloadObject, AGENT_CONFIGURATION_ERROR_ERROR_KEY, error) != JSON_WRITER_OK) {
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

/* @brief Validates max local cache size is eauql or greater than max message size
 *
 * @param payloadHandle     the writer to write the payload to
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
static EventCollectorResult AgentConfigurationErrorCollector_ValidateMaxLocalCacheSizeIsHigherThanMaxMessageSize(JsonArrayWriterHandle payloadHandle) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    uint32_t maxCacheConfig = 0, maxMessageSizeConfig = 0;
    char configAsString[12];
    char* msg = NULL;
    if (TwinConfiguration_GetMaxLocalCacheSize(&maxCacheConfig) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (TwinConfiguration_GetMaxMessageSize(&maxMessageSizeConfig) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (maxCacheConfig < maxMessageSizeConfig) {
        msg = "maxLocalCacheSize is lower than maxMessageSize";
        if (sprintf(configAsString, "%d", maxCacheConfig) < 0){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }
        result = AgentConfigurationErrorCollector_AddPayload(payloadHandle, MAX_LOCAL_CACHE_SIZE_KEY, ERROR_TYPE_CONFLICT, msg, configAsString);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
    }

cleanup:
    return result;
}

/* @brief Validates max message size is optimal
 *
 * @param payloadHandle     the writer to write the payload to
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentConfigurationErrorCollector_ValidateMaxMessageSizeOptimal(JsonArrayWriterHandle payloadHandle) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    uint32_t maxMessageSizeConfig;
    char configAsString[12];
    char* msg = NULL;

    if (TwinConfiguration_GetMaxMessageSize(&maxMessageSizeConfig) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (maxMessageSizeConfig % MESSAGE_BILLING_MULTIPLE != 0) {
        msg = "maxMessageSize is not optimal";
        if (sprintf(configAsString, "%d", maxMessageSizeConfig) < 0){
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }
        result = AgentConfigurationErrorCollector_AddPayload(payloadHandle, MAX_MESSAGE_SIZE_KEY, ERROR_TYPE_NOT_OPTIMAL, msg, configAsString);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
    }

cleanup:
    return result;
}

/* @brief Validates twin configuration, if an error is found, add a correspoding payload
 *
 * @param payloadHandle     the writer to write the payload to
 * 
 * @return EVENT_COLLECTOR_OK for sucess
 */
EventCollectorResult AgentConfigurationErrorCollector_ValidateHighPrioFreqIsLowerThanLowPrioFreq(JsonArrayWriterHandle payloadHandle) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    char configAsString[DURATION_MAX_LENGTH] = { '\0' };
    char* msg = NULL;

    uint32_t highPrioFreq = 0, lowPrioFreq = 0;
    if (TwinConfiguration_GetHighPriorityMessageFrequency(&highPrioFreq) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (TwinConfiguration_GetLowPriorityMessageFrequency(&lowPrioFreq) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if (highPrioFreq > lowPrioFreq) {
        msg = "high priority frequency is higher than low priority frequency";
        if (TimeUtils_MillisecondsToISO8601DurationString(highPrioFreq, configAsString, sizeof(configAsString)) == false) {
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }

        result = AgentConfigurationErrorCollector_AddPayload(payloadHandle, HIGH_PRIORITY_MESSAGE_FREQUENCY_KEY, ERROR_TYPE_CONFLICT, msg, configAsString);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
    }

cleanup:
    return result;
}

EventCollectorResult AgentConfigurationErrorCollector_CreateTypeMismatchPayload(JsonArrayWriterHandle payloadHandle, TwinConfigurationUpdateResult* lastUpdateResult) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    char buffer[512] = { 0 };
    result = AgentConfigurationErrorCollector_GenerateTypeMismatchMessage(&(lastUpdateResult->configurationBundleStatus), buffer, sizeof(buffer));
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    char* currentConfiguration = NULL;
    uint32_t configLen;
    if (TwinConfiguration_GetSerializedTwinConfiguration(&currentConfiguration, &configLen) != TWIN_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = AgentConfigurationErrorCollector_AddPayload(payloadHandle, "TwinConfiguration", ERROR_TYPE_TYPE_MISMATCH, buffer, currentConfiguration);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }
cleanup:
    return result;
}
EventCollectorResult AgentConfigurationErrorCollector_GetEvents(SyncQueue* priorityQueue) {
    JsonObjectWriterHandle eventHandle = NULL;
    JsonArrayWriterHandle payloadHandle = NULL;
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    TwinConfigurationUpdateResult lastTwinUpdateResult;
    TwinConfiguration_GetLastTwinUpdateData(&lastTwinUpdateResult);

    if (TimeUtils_GetTimeDiff(lastTwinUpdateResult.lastUpdateTime, lastEvent) == 0) {
        goto cleanup;
    }

    if (JsonObjectWriter_Init(&eventHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddMetadata(eventHandle, EVENT_TRIGGERED_CATEGORY, AGENT_CONFIGURATION_ERROR_EVENT_NAME, EVENT_TYPE_OPERATIONAL_VALUE, AGENT_CONFIGURATION_ERROR_EVENT_SCHEMA_VERSION);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&payloadHandle) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (lastTwinUpdateResult.lastUpdateResult == TWIN_OK) {
        result = AgentConfigurationErrorCollector_ValidateMaxLocalCacheSizeIsHigherThanMaxMessageSize(payloadHandle);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }

        result = AgentConfigurationErrorCollector_ValidateMaxMessageSizeOptimal(payloadHandle);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }

        result = AgentConfigurationErrorCollector_ValidateHighPrioFreqIsLowerThanLowPrioFreq(payloadHandle);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
    } else if (lastTwinUpdateResult.lastUpdateResult == TWIN_PARSE_EXCEPTION) {
        result = AgentConfigurationErrorCollector_CreateTypeMismatchPayload(payloadHandle, &lastTwinUpdateResult);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
    }

    result = GenericEvent_AddPayload(eventHandle, payloadHandle);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;

    }
    
    result = AgentConfigurationErrorCollector_PushEvent(priorityQueue, eventHandle);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

cleanup:
    if (payloadHandle){
        JsonArrayWriter_Deinit(payloadHandle);
    }

    if (eventHandle){
        JsonObjectWriter_Deinit(eventHandle);
    }

    lastEvent = lastTwinUpdateResult.lastUpdateTime;

    return result;
}

EventCollectorResult AgentConfigurationErrorCollector_PushEvent(SyncQueue* queue, JsonObjectWriterHandle eventHandle){
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