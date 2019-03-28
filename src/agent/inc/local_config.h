// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOCAL_CONFiG_H
#define LOCAL_CONFiG_H

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
MOCKABLE_FUNCTION(, char*, LocalConfiguration_GetConnectionString);

/**
 * @brief returns the agent id.
 * 
 * @return the agent id
 */
MOCKABLE_FUNCTION(, char*, LocalConfiguration_GetAgentId);

/**
 * @brief returns the triggered events interval.
 * 
 * @return the triggered events interval
 */
MOCKABLE_FUNCTION(, uint32_t, LocalConfiguration_GetTriggeredEventInterval);

#endif // LOCAL_CONFiG_H