// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CORRELATION_MANAGER_H
#define CORRELATION_MANAGER_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief initializes the correlation manager.
 * 
 * @return true if initilization succeeded, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, CorrelationManager_Init);

/**
 * @brief deinitializes the correlation manager.
 */
MOCKABLE_FUNCTION(, void, CorrelationManager_Deinit);

/**
 * @brief gets the current context's correlation.
 * 
 * @return the new correlation uuid.
 */
MOCKABLE_FUNCTION(, const char*, CorrelationManager_GetCorrelation);

/**
 * @brief sets a new correlation to the current context.
 * 
 * @return true if the new correlation had been set successfully, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, CorrelationManager_SetCorrelation);

#endif //CORRELATION_MANAGER_H