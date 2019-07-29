// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOCAL_CONFiG_H
#define LOCAL_CONFiG_H

#include <stdbool.h>

#include "umock_c_prod.h"
#include "macro_utils.h"

#include "consts.h"

typedef enum _LocalConfigurationResultValues {

    LOCAL_CONFIGURATION_OK,
    LOCAL_CONFIGURATION_EXCEPTION
    
} LocalConfigurationResultValues;

/**
 * @brief initialize the local configuration.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, LocalConfigurationResultValues, LocalConfiguration_Init);

/**
 * @brief deinitialize the local configuration.
 */
MOCKABLE_FUNCTION(, void, LocalConfiguration_Deinit);

/**
 * @brief returns the connection string for the IoT hub.
 * 
 * @return the connection string for the IoT hub
 */
MOCKABLE_FUNCTION(, const char*, LocalConfiguration_GetConnectionString);

/**
 * @brief returns the agent id.
 * 
 * @return the agent id
 */
MOCKABLE_FUNCTION(, const char*, LocalConfiguration_GetAgentId);

/**
 * @brief returns the triggered events interval.
 * 
 * @return the triggered events interval
 */
MOCKABLE_FUNCTION(, uint32_t, LocalConfiguration_GetTriggeredEventInterval);

/**
 * @brief returns the timeout to connect to IoTHub.
 * 
 * @return the timeout to connect to IoTHub
 */
MOCKABLE_FUNCTION(, uint32_t, LocalConfiguration_GetConnectionTimeout);

/**
 * @brief are we using dps?.
 * 
 * @return whether dps is being used for authentication
 */
MOCKABLE_FUNCTION(, bool, LocalConfiguration_UseDps);

/**
 * @brief Renew the connection string through the DPS.
 * 
 * @return true on success, false otherwise
 */
MOCKABLE_FUNCTION(, bool, LocalConfiguration_TryRenewConnectionString);

/**
 * @brief returns the minimum severity for local logging.
 * 
 * @return the minimum severity for local logging
 */
MOCKABLE_FUNCTION(, int32_t, LocalConfiguration_GetSystemLoggerMinimumSeverity);

/**
 * @brief returns the minimum severity for diagnostic event.
 * 
 * @return the minimum severity for diagnostic event
 */
MOCKABLE_FUNCTION(, int32_t, LocalConfiguration_GetDiagnosticEventMinimumSeverity);

/**
 * @brief returns the name of the remote configuration object
 * 
 * @return the name of the remote configuration object (the name of the object that holds the agent configuration inside the module twin)
 */
MOCKABLE_FUNCTION(, const char*, LocalConfiguration_GetRemoteConfigurationObjectName);

#endif // LOCAL_CONFiG_H