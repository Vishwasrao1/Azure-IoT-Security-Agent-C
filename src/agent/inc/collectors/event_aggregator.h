// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef EVENT_AGGREGATOR_H
#define EVENT_AGGREGATOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "json/json_object_reader.h"
#include "synchronized_queue.h"
#include "twin_configuration_event_collectors.h"

/*
 * Result values for all event aggregator functions
 */
typedef enum _EventAggregatorResult {
    EVENT_AGGREGATOR_OK,
    EVENT_AGGREGATOR_DISABLED,
    EVENT_AGGREGATOR_EXCEPTION
} EventAggregatorResult;

/*
 * Configuration struct for Event Aggregator initalization
 */
typedef struct _EventAggregatorConfiguration {
    TwinConfigurationEventType iotEventType;
    const char* event_type;
    const char* event_name;
    const char* payload_schema_version;
} EventAggregatorConfiguration;


typedef struct _EventAggregator *EventAggregatorHandle;

/**
 * @brief Inits event aggregator
 * 
 * @param   iotEventType           The type of the aggregated event
 * @param   configuration       pointer to event aggregator configuration
 * 
 * @return EVENT_AGGREGATOR_OK on success, EVENT_AGGREGATOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventAggregatorResult, EventAggregator_Init, EventAggregatorHandle*, aggregator, EventAggregatorConfiguration*, configuration);

/**
 * @brief Deinits event aggregator
 */
MOCKABLE_FUNCTION(, void, EventAggregator_Deinit, EventAggregatorHandle, aggregator);

/**
 * @brief Aggregates an event payload
 *          Aggregation is based on payload equality
 * 
 * @param   aggregator          Handle to the aggregator
 * @param   eventPayload        The payload to aggregate
 * 
 * @return EVENT_AGGREGATOR_OK on success, EVENT_AGGREGATOR_DISABLED if aggregation is disabled for
 * this aggregator iotEventType and EVENT_AGGREGATOR_EXCPETION otherwise.
 */
MOCKABLE_FUNCTION(, EventAggregatorResult, EventAggregator_AggregateEvent, EventAggregatorHandle, aggregator, JsonObjectWriterHandle, eventPayload);

/**
 * @brief Create events from the aggregated results.
 * This function creates event if aggregation interval has passed 
 * or if event aggregation is disabled (flush events)
 * 
 * @param   aggregator          Handle to the aggregator
 * @param   queue               The queue to push the events to
 * 
 * @return EVENT_AGGREGATOR_OK on success, EVENT_AGGREGATOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventAggregatorResult, EventAggregator_GetAggregatedEvents, EventAggregatorHandle, aggregator, SyncQueue*, queue);

/**
 * @brief Returns if aggregation is enabled for this aggregator type of event
 * 
 * @param   aggregator          Handle to the aggregator
 * @param   isEnable            True if aggregation is enabled else false
 * 
 * @return EVENT_AGGREGATOR_OK on success, EVENT_AGGREGATOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventAggregatorResult, EventAggregator_IsAggregationEnabled, EventAggregatorHandle, aggregator, bool*, isEnabled);

#endif //EVENT_AGGREGATOR_H