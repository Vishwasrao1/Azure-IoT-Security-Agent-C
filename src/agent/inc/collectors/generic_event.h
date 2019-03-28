// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GENERIC_EVENT_H
#define GENERIC_EVENT_H

#include <stdbool.h>
#include <time.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "json/json_object_writer.h"
#include "json/json_array_writer.h"


typedef enum _EventCollectorResult {
    EVENT_COLLECTOR_OK,
    EVENT_COLLECTOR_RECORD_HAS_ERRORS,
    EVENT_COLLECTOR_RECORD_FILTERED,
    EVENT_COLLECTOR_OUT_OF_MEM,
    EVENT_COLLECTOR_EXCEPTION
} EventCollectorResult;

/**
 * @brief adds generic metadata to the event.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   eventCategory           The category of the event.
 * @param   eventName               The name of the event.
 * @param   eventType               The type of the event.
 * @param   eventPayloadVersion     The message schema version of the payload.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericEvent_AddMetadata, JsonObjectWriterHandle, eventWriter, const char*, eventCategory, const char*, eventName, const char*, eventType, const char*, eventPayloadVersion);

/**
 * @brief adds generic metadata to the event.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   eventCategory           The category of the event.
 * @param   eventName               The name of the event.
 * @param   eventType               The type of the event.
 * @param   eventPayloadVersion     The message schema version of the payload.
 * @param   eventLocalTime          The time of the event.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericEvent_AddMetadataWithTimes, JsonObjectWriterHandle, eventWriter, const char*, eventCategory, const char*, eventName, const char*, eventType, const char*, eventPayloadVersion, time_t*, eventLocalTime);

/**
 * @brief add payload to event.
 * 
 * @param   eventWriter             A handle to the writer of the event object.
 * @param   payloadWriter           A handle to the writer of the payload array object.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, GenericEvent_AddPayload, JsonObjectWriterHandle, eventWriter, JsonArrayWriterHandle, payloadWriter);

#endif //GENERIC_EVENT_H