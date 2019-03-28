// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/generic_event.h"

#include "azure_c_shared_utility/uniqueid.h"

#include "internal/time_utils.h"
#include "message_schema_consts.h"

static const int MAX_TIME_AS_STRING_LENGTH = 25;
#define EVENT_ID_SIZE 37

EventCollectorResult GenericEvent_AddMetadata(JsonObjectWriterHandle eventWriter, const char* eventCategory, const char* eventName, const char* eventType, const char* eventPayloadVersion) {
    time_t currentTime = TimeUtils_GetCurrentTime();
    return GenericEvent_AddMetadataWithTimes(eventWriter, eventCategory, eventName, eventType, eventPayloadVersion, &currentTime);
}

EventCollectorResult GenericEvent_AddMetadataWithTimes(JsonObjectWriterHandle eventWriter, const char* eventCategory, const char* eventName, const char* eventType, const char* eventPayloadVersion, time_t* eventLocalTime) {
    if (JsonObjectWriter_WriteString(eventWriter, EVENT_CATEGORY_KEY, eventCategory) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteString(eventWriter, EVENT_TYPE_KEY, eventType) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteString(eventWriter, EVENT_NAME_KEY, eventName) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteString(eventWriter, EVENT_PAYLOAD_SCHEMA_VERSION_KEY, eventPayloadVersion) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    char eventId[EVENT_ID_SIZE] = "";
    UNIQUEID_RESULT uuidResult = UniqueId_Generate(eventId, sizeof(eventId));
    if (uuidResult != UNIQUEID_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    if (JsonObjectWriter_WriteString(eventWriter, EVENT_ID_KEY, eventId) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    char timeStr[MAX_TIME_AS_STRING_LENGTH];
    uint32_t timeStrLength = MAX_TIME_AS_STRING_LENGTH;
    memset(timeStr, 0, timeStrLength);

    if (!TimeUtils_GetTimeAsString(eventLocalTime, timeStr, &timeStrLength)) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    if (JsonObjectWriter_WriteString(eventWriter, EVENT_LOCAL_TIMESTAMP_KEY, timeStr) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    timeStrLength = MAX_TIME_AS_STRING_LENGTH;
    memset(timeStr, 0, timeStrLength);
    if (!TimeUtils_GetLocalTimeAsUTCTimeAsString(eventLocalTime, timeStr, &timeStrLength)) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    if (JsonObjectWriter_WriteString(eventWriter, EVENT_UTC_TIMESTAMP_KEY, timeStr) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    return EVENT_COLLECTOR_OK;
}

EventCollectorResult GenericEvent_AddPayload(JsonObjectWriterHandle eventWriter, JsonArrayWriterHandle payloadWriter) {
    uint32_t elements_count = 0;
    
    JsonWriterResult result = JsonArrayWriter_GetSize(payloadWriter, &elements_count);
    if (result != JSON_WRITER_OK){
        return EVENT_COLLECTOR_EXCEPTION;
    }

    bool isEmpty = (elements_count == 0);
    if (JsonObjectWriter_WriteBool(eventWriter, EVENT_IS_EMPTY_KEY, isEmpty) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    
    if (JsonObjectWriter_WriteArray(eventWriter, PAYLOAD_KEY, payloadWriter) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    return EVENT_COLLECTOR_OK;
}