// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>

#include "agent_errors.h"
#include "utils.h"

#define AGENT_ERROR_FORMAT "ASC for IoT agent encounterd an error! Error in: %s, reason: %s, extra details: %s"

//Error Codes
static const char* IOT_AUTHENTICATION = "Authentication";
static const char* LOCAL_CONFIGURATION = "Local Configuration";
static const char* REMOTE_CONFIGURATION = "Remote Configuration";

//Error subcodes
static const char* CANT_PARSE_CONFIGURATION = "Cant Parse Configuration";
static const char* FILE_FORMAT = "File Format";
static const char* FILE_NOT_EXIST = "File Not Exist";
static const char* FILE_PERMISSIONS = "File Permissions";
static const char* MISSING_CONFIGURATION = "Missing Configuration";
static const char* NOT_FOUND = "Not Found";
static const char* OTHER = "Other";
static const char* TIMEOUT = "Timeout";
static const char* UNAUTHORIZED = "Unauthorized";

const char* AgentErrors_ErrorCodeToString(ErrorCodes code) {
    switch (code) {
    case ERROR_LOCAL_CONFIGURATION :
        return LOCAL_CONFIGURATION;
    case ERROR_REMOTE_CONFIGURATION :
        return REMOTE_CONFIGURATION;
    case ERROR_IOT_HUB_AUTHENTICATION :
        return IOT_AUTHENTICATION;
    default:
        return OTHER;
    }
}

const char* AgentErrors_ErrorSubCodeToString(ErrorSubCodes subCode) {
    switch (subCode) {
    case SUBCODE_MISSING_CONFIGURATION :
        return MISSING_CONFIGURATION;
    case SUBCODE_CANT_PARSE_CONFIGURATION :
        return CANT_PARSE_CONFIGURATION;
    case SUBCODE_TIMEOUT : 
        return TIMEOUT;
    case SUBCODE_FILE_NOT_EXIST :
        return FILE_NOT_EXIST;
    case SUBCODE_FILE_PERMISSIONS :
        return FILE_PERMISSIONS;
    case SUBCODE_FILE_FORMAT :
        return FILE_FORMAT;
    case SUBCODE_UNAUTHORIZED : 
        return UNAUTHORIZED;
    case SUBCODE_NOT_FOUND : 
        return NOT_FOUND;
    case SUBCODE_OTHER :
    default:
        return OTHER;
    }
}

void AgentErrors_LogError(ErrorCodes errorCode, ErrorSubCodes errorSubCode, const char *__restrict __fmt, ...) {
    const char* codeAsString = AgentErrors_ErrorCodeToString(errorCode);
    const char* subCodeAsString = AgentErrors_ErrorSubCodeToString(errorSubCode);
    va_list args;
    va_start(args, __fmt);
    char extraInfo[LOG_MAX_BUFF];
    if (vsnprintf(extraInfo, LOG_MAX_BUFF, __fmt, args) > 0) {
        Logger_Fatal(AGENT_ERROR_FORMAT, codeAsString, subCodeAsString, extraInfo); 
    } else {
        Logger_Fatal(AGENT_ERROR_FORMAT, codeAsString, subCodeAsString, ""); 
    }
    va_end(args);
}