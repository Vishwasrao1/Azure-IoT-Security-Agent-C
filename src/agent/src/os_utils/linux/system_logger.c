// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/system_logger.h"
#include "consts.h"

#include <stdbool.h>
#include <stdarg.h>
#include <syslog.h>

// This array should be kept aligned with the Severity enum in os_utils/system_logger.h
static const int logPriorities[SEVERITY_MAX] = {
    LOG_DEBUG,      // SEVERITY_DEBUG
    LOG_INFO,       // SEVERITY_INFORMATION
    LOG_WARNING,    // SEVERITY_WARNING
    LOG_ERR,        // SEVERITY_ERROR
    LOG_CRIT        // SEVERITY_FATAL
};

static bool isInitialized = false;

bool SystemLogger_Init() {
    openlog(AGENT_NAME, LOG_PID, LOG_USER);
    isInitialized = true;
    return true;
}

bool SystemLogger_IsInitialized() {
    return isInitialized;
}

bool SystemLogger_LogMessage(const char * msg, Severity severity) {
    if (severity < 0 || severity >= SEVERITY_MAX) {
        return false;
    }
    syslog(logPriorities[severity], "%s", msg);
    return true;
}

void SystemLogger_Deinit() {
    isInitialized = false;
    closelog();
}