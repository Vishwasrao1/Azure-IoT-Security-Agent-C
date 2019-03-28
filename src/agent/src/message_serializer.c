// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "message_serializer.h"

#include <stdbool.h>
#include <stdlib.h>

#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "local_config.h"
#include "logger.h"
#include "message_schema_consts.h"
#include "twin_configuration.h"

/**
 * @brief Handle adding events to the message from a single queue
 * 
 * @param   queue               Th queue to handle.
 * @param   eventsArray         The events array.
 * @param   currentMessageSize  Pointer to the current size of the security message.
 * @param   maxMessageSize      The max message size.
 * 
 * @return MESSAGE_SERIALIZER_OK on success, otherwise the specific error. The value of the out param is undefined in case of failure.
 */
static MessageSerializerResultValues MessageSerializer_AddEventsFromQueue(SyncQueue* queue, JsonArrayWriterHandle eventsArray, uint32_t* currentMessageSize, uint32_t maxMessageSize);

/**
 * @brief Generated the event list serialization.
 * 
 * @param    queues         The queues to take event from, the serializer create events from the first queue, than the second etc...  
 * @param    len            The length of the queues array            
 * @param    parentObj      The parent object which will hold the list of events.
 * 
 * @return MESSAGE_SERIALIZER_OK on success, otherwise the specific error.
 */
static MessageSerializerResultValues MessageSerializer_GenerateEventList(SyncQueue* queues[], uint32_t len, JsonObjectWriterHandle parentObj);

/**
 * @brief Serialize single event to the array.
 * 
 * @param   queue               The queue to pop the event from
 * @param   eventsArray         The array of events to add the new event to.
 * @param   currentMessageSize  Pointer to the current size of the security message.
 * @param   maxMessageSize      The maximum message size of the security message.
 * 
 * @return MESSAGE_SERIALIZER_OK on success, otherwise the specific error.
 */
static MessageSerializerResultValues MessageSerializer_AddSingleEvent(SyncQueue* queue, JsonArrayWriterHandle eventsArray, uint32_t* currentMessageSize, uint32_t maxMessageSize);

typedef struct _SerializaerSizeLimits {
    uint32_t currentMessageSize;
    uint32_t maxMessageSize;
} SerializaerSizeLimits;

/**
 * @brief A message serializer size limitiation condition. Reutrn true if the data size + current size does not exceed the max size.
 * 
 * @param   data                The item
 * @param   size                The size of the item
 * @param   conditionParams     Struct of SerializaerSizeLimits
 * 
 * @return true if (size + currentMessageSize) < maxMessageSize
 */
static bool sizeLimitationCondition(const void* data, uint32_t size, void* conditionParams) {
    SerializaerSizeLimits* limits = (SerializaerSizeLimits*) conditionParams;
    return ((limits->currentMessageSize + size) < limits->maxMessageSize);
}

static MessageSerializerResultValues MessageSerializer_AddSingleEvent(SyncQueue* queue, JsonArrayWriterHandle eventsArray, uint32_t* currentMessageSize, uint32_t maxMessageSize) {
    MessageSerializerResultValues result = MESSAGE_SERIALIZER_OK;
    void* data = NULL;
    uint32_t dataSize = 0;
    JsonObjectWriterHandle eventWriter = NULL;
    SerializaerSizeLimits limits;
    limits.currentMessageSize = *currentMessageSize;
    limits.maxMessageSize = maxMessageSize;
    int queueResult = SyncQueue_PopFrontIf(queue, sizeLimitationCondition, &limits, &data, &dataSize);
    if (queueResult == QUEUE_IS_EMPTY) {
        result = MESSAGE_SERIALIZER_EMPTY;
        goto cleanup;
    } else if (queueResult == QUEUE_CONDITION_FAILED) {
        result = MESSAGE_SERIALIZER_MEMORY_EXCEEDED;
        goto cleanup;
    } else if (queueResult != QUEUE_OK) {
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }
    
    if (JsonObjectWriter_InitFromString(&eventWriter, data) != JSON_WRITER_OK) {
        Logger_Error("Error parsing event data as json");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_AddObject(eventsArray, eventWriter) != JSON_WRITER_OK) {
        Logger_Error("error while appending the new event to the array");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }
    *currentMessageSize += dataSize;

cleanup:
    if (data != NULL) {
        free(data);
    }

    if (eventWriter != NULL) {
        JsonObjectWriter_Deinit(eventWriter);
    }

    return result;
}

static MessageSerializerResultValues MessageSerializer_AddEventsFromQueue(SyncQueue* queue, JsonArrayWriterHandle eventsArray, uint32_t* currentMessageSize, uint32_t maxMessageSize) {
    MessageSerializerResultValues result = MESSAGE_SERIALIZER_OK;

    uint32_t queueSize = 0;

    if (SyncQueue_GetSize(queue, &queueSize) != QUEUE_OK) {
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    while (queueSize > 0 && *currentMessageSize < maxMessageSize) {
        result = MessageSerializer_AddSingleEvent(queue, eventsArray, currentMessageSize, maxMessageSize);
        if (result == MESSAGE_SERIALIZER_MEMORY_EXCEEDED) {
            result = MESSAGE_SERIALIZER_OK;
            break;
        } else if (result == MESSAGE_SERIALIZER_EMPTY) {
            result = MESSAGE_SERIALIZER_OK;
            break;
        } else if (result != MESSAGE_SERIALIZER_OK) {
            result = MESSAGE_SERIALIZER_EXCEPTION;
            goto cleanup;
        }

        if (SyncQueue_GetSize(queue, &queueSize) != QUEUE_OK) {
            result = MESSAGE_SERIALIZER_EXCEPTION;
            goto cleanup;
        }
    }

cleanup:
    return result;
}

static MessageSerializerResultValues MessageSerializer_GenerateEventList(SyncQueue** queues, uint32_t size, JsonObjectWriterHandle parentObj) { 
    MessageSerializerResultValues result = MESSAGE_SERIALIZER_OK;
    JsonArrayWriterHandle eventsArray = NULL;
    uint32_t currentMessageSize = 0;

    if (size == 0){
        goto cleanup;
    }

    uint32_t maxMessageSize = 0;    
    if (TwinConfiguration_GetMaxMessageSize(&maxMessageSize) != TWIN_OK) {
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&eventsArray) != JSON_WRITER_OK) {
        Logger_Error("Error initializing the new events array");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    for (int i = 0; i < size; i++){
        if (currentMessageSize < maxMessageSize) {
            if (MessageSerializer_AddEventsFromQueue(queues[i], eventsArray, &currentMessageSize, maxMessageSize) != MESSAGE_SERIALIZER_OK) {
                result = MESSAGE_SERIALIZER_PARTIAL;
            }
        }
    }

    if (JsonObjectWriter_WriteArray(parentObj, EVENTS_KEY, eventsArray) != JSON_WRITER_OK) {
        Logger_Error("Error setting events array value to security message");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }
    
cleanup:
    if (eventsArray != NULL) {
        JsonArrayWriter_Deinit(eventsArray);
    }
    if (currentMessageSize == 0) {
        result = MESSAGE_SERIALIZER_EMPTY;
    }

    return result;
}

MessageSerializerResultValues MessageSerializer_CreateSecurityMessage(SyncQueue* queues[], uint32_t len, void** buffer) {
    MessageSerializerResultValues result = MESSAGE_SERIALIZER_OK;

    JsonObjectWriterHandle securityMessageWriter = NULL;
    if (JsonObjectWriter_Init(&securityMessageWriter) != JSON_WRITER_OK) {
        Logger_Error("Error initializing the security message writer");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }
    
    if (JsonObjectWriter_WriteString(securityMessageWriter, AGENT_VERSION_KEY, AGENT_VERSION) != JSON_WRITER_OK) {
        Logger_Error("Error setting the agent version");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(securityMessageWriter, AGENT_ID_KEY, LocalConfiguration_GetAgentId()) != JSON_WRITER_OK) {
        Logger_Error("Error setting the agent version");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(securityMessageWriter, MESSAGE_SCHEMA_VERSION_KEY, DEFAULT_MESSAGE_SCHEMA_VERSION) != JSON_WRITER_OK) {
        Logger_Error("Error setting the agent version");
        result = MESSAGE_SERIALIZER_EXCEPTION;
        goto cleanup;
    }

    result = MessageSerializer_GenerateEventList(queues, len, securityMessageWriter);
    if (result == MESSAGE_SERIALIZER_EMPTY) {
        goto cleanup;
    }

    if (result != MESSAGE_SERIALIZER_OK && result != MESSAGE_SERIALIZER_PARTIAL) {
        goto cleanup;
    }

    uint32_t buffersize = 0;
    if (JsonObjectWriter_Serialize(securityMessageWriter, (char**)buffer, &buffersize) != JSON_WRITER_OK) {
        Logger_Error("Error serialing the security message");
        result = MESSAGE_SERIALIZER_EXCEPTION;
    }

cleanup:
    
    if (securityMessageWriter != NULL) {
        JsonObjectWriter_Deinit(securityMessageWriter);
    }

    if (result != MESSAGE_SERIALIZER_OK && result != MESSAGE_SERIALIZER_PARTIAL) {
        if (*buffer != NULL) {
            free(*buffer);
            *buffer = NULL;
        }
    }

    return result;
}