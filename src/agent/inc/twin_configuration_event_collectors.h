// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TWIN_CONFIGURATION_EVENT_COLLECTORS_H
#define TWIN_CONFIGURATION_EVENT_COLLECTORS_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "twin_configuration_consts.h"
#include "twin_configuration_defs.h"

#include <stdbool.h>

#include "json/json_object_reader.h"
#include "json/json_object_writer.h"

typedef enum _TwinConfigurationEventPriority {

    EVENT_PRIORITY_OPERATIONAL,
    EVENT_PRIORITY_HIGH,
    EVENT_PRIORITY_LOW,
    EVENT_PRIORITY_OFF

} TwinConfigurationEventPriority;

/**
 * @brief initialize the global event priorities configuration with default values
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_Init);

/**
 * @brief deinitiate the given event priorities twin configuration.
 */
MOCKABLE_FUNCTION(, void, TwinConfigurationEventCollectors_Deinit);

/**
 * @brief updates the event priorities.
 * 
 * @param   propertiesReader    The properties json obect reader.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_Update, JsonObjectReaderHandle, propertiesReader);

/**
 * @brief Returns the priority of the wanted event type.
 * 
 * @param   eventType   The wanted event type.
 * @param   priority     Out param. The wanted priority.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_GetPriority, TwinConfigurationEventType, eventType, TwinConfigurationEventPriority*, priority);

/**
 * @brief writes event priorities object.
 * 
 * @param   prioritiesJson   Initialized json object writer
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_GetPrioritiesJson, JsonObjectWriterHandle, prioritiesJson);

/**
 * @brief Returns if aggregation is enabled on the wanted event type.
 * 
 * @param   eventType   The wanted event type.
 * @param   isEnabled   Out param. is aggregation enabled.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_GetAggregationEnabled, TwinConfigurationEventType, eventType, bool*, isEnabled);

/**
 * @brief Returns the aggregation interval of the wanted event type.
 * 
 * @param   eventType   The wanted event type.
 * @param   interval    Out param. The wanted interval.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventCollectors_GetAggregationInterval, TwinConfigurationEventType, eventType, uint32_t*, interval);

#endif //TWIN_CONFIGURATION_EVENT_COLLECTORS_H