// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AGENT_ERRORS_H
#define AGENT_ERRORS_H

#include "umock_c_prod.h"
#include "macro_utils.h"

#include "logger.h"

typedef enum _ErrorSubCodes {

    SUBCODE_MISSING_CONFIGURATION,
    SUBCODE_CANT_PARSE_CONFIGURATION,
    SUBCODE_TIMEOUT,
    SUBCODE_FILE_NOT_EXIST,
    SUBCODE_FILE_PERMISSIONS,
    SUBCODE_FILE_FORMAT,
    SUBCODE_UNAUTHORIZED,
    SUBCODE_NOT_FOUND,
    SUBCODE_OTHER
    
} ErrorSubCodes;

typedef enum _ErrorCodes {

    ERROR_LOCAL_CONFIGURATION,
    ERROR_REMOTE_CONFIGURATION,
    ERROR_IOT_HUB_AUTHENTICATION,
    ERROR_OTHER
    
} ErrorCodes;

/**
 * @brief Logs agent error
 * 
 * @param code      Error code
 * @param subCode   Error subcode
 * @param __fmt     Error message
 */
void AgentErrors_LogError(ErrorCodes code, ErrorSubCodes subCode, const char *__restrict __fmt, ...);

#endif