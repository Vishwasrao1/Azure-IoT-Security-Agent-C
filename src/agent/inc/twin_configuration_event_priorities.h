// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TWIN_CONFIGURATIONEVENT_PRIORITIES_H
#define TWIN_CONFIGURATIONEVENT_PRIORITIES_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "twin_configuration_consts.h"
#include "twin_configuration_defs.h"

#include "json/json_object_reader.h"
#include "json/json_object_writer.h"

typedef enum _TwinConfiguartionEventPriority {

    EVENT_PRIORITY_OPERATIONAL,
    EVENT_PRIORITY_HIGH,
    EVENT_PRIORITY_LOW,
    EVENT_PRIORITY_OFF

} TwinConfiguartionEventPriority;

typedef enum _TwinConfiguartionEventType {

    EVENT_TYPE_BASELINE,
    EVENT_TYPE_CONNECTION_CREATE,
    EVENT_TYPE_FIREWALL_CONFIGURATION,
    EVENT_TYPE_LISTENING_PORTS,
    EVENT_TYPE_LOCAL_USERS,
    EVENT_TYPE_PROCESS_CREATE,
    EVENT_TYPE_SYSTEM_INFORMATION,
    EVENT_TYPE_USER_LOGIN,
    EVENT_TYPE_DIAGNOSTIC,
    EVENT_TYPE_OPERATIONAL_EVENT

} TwinConfiguartionEventType;


/**
 * @brief initialize the global event priorities configuration with default values
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventPriorities_Init);

/**
 * @brief deinitiate the given event priorities twin configuration.
 */
MOCKABLE_FUNCTION(, void, TwinConfigurationEventPriorities_Deinit);

/**
 * @brief updates the event priorities.
 * 
 * @param   propertiesReader    The properties json obect reader.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventPriorities_Update, JsonObjectReaderHandle, propertiesReader);

/**
 * @brief Returns the priority of the wanted event type.
 * 
 * @param   eventType   The wanted event type.
 * @param   priority     Out param. The wanted priority.
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventPriorities_GetPriority, TwinConfiguartionEventType, eventType, TwinConfiguartionEventPriority*, priority);

/**
 * @brief writes event priorities object.
 * 
 * @param   prioritiesJson   Initialized json object writer
 * 
 * @return TWIN_OK on success or an error code upon failure
 */
MOCKABLE_FUNCTION(, TwinConfigurationResult, TwinConfigurationEventPriorities_GetPrioritiesJson, JsonObjectWriterHandle, prioritiesJson);

#endif //TWIN_CONFIGURATIONEVENT_PRIORITIES_H