// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SYSTEM_LOGGER_H
#define SYSTEM_LOGGER_H

#include "macro_utils.h"
#include "umock_c_prod.h"
#include "logger_consts.h"

#include <stdbool.h>

/**
 * @brief Initializes the system logger.
 * 
 * @return true on success false otherwise.
 */
MOCKABLE_FUNCTION(, bool, SystemLogger_Init);

/**
 * @brief Queries the state of the system logger.
 * 
 * @return true if the system logger is enabled, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, SystemLogger_IsInitialized);

/**
 * @brief Logs a new message to the system logger.
 * 
 * @param   msg         The message to log.
 * @param   severity    The severity of the message.
 * 
 * @return true on success false otherwise.
 */
MOCKABLE_FUNCTION(, bool, SystemLogger_LogMessage, const char*, msg, Severity, severity);

/**
 * @brief Deinitializes the system logger.
 */
MOCKABLE_FUNCTION(, void, SystemLogger_Deinit);

#endif //SYSTEM_LOGGER_H