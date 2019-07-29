// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "logger.h"

#include "os_utils/system_logger.h"

#include <stdio.h>
#include <stdarg.h>

#if !defined(DISABLE_LOGS) && !defined(TEST_LOG)

static Severity systemLoggerMinimumSeverity;
static Severity diagnosticEventMinimumSeverity;

bool Logger_Init() {
    systemLoggerMinimumSeverity = SEVERITY_DEBUG;
    diagnosticEventMinimumSeverity = SEVERITY_WARNING;

    return SystemLogger_Init();
}

void Logger_Deinit() {
    if (SystemLogger_IsInitialized()) {
        SystemLogger_Deinit();
    }
}

void Logger_LogEvent(Severity severity, const char *__restrict __fmt, ...) {

    if (!SystemLogger_IsInitialized()) {
        SystemLogger_Init();
    }

    va_list args;
    va_start(args, __fmt);
    char buf[LOG_MAX_BUFF];
    if (vsnprintf(buf, LOG_MAX_BUFF, __fmt, args) > 0) {
        if (severity >= systemLoggerMinimumSeverity) {
            SystemLogger_LogMessage(buf, severity);
        }
        if (severity >= diagnosticEventMinimumSeverity && DiagnosticEventCollector_IsInitialized()) {
            DiagnosticEventCollector_AddEvent(buf, severity);
        }
    }
    va_end(args);
}

bool Logger_SetMinimumSeverityForSystemLogger(int32_t severity) {
    if (severity < 0 || severity >= SEVERITY_MAX) {
        Logger_Error("failed setting system logger minimum severity, out of range");
        return false;
    }
    systemLoggerMinimumSeverity = (Severity)severity;
    return true;
}

bool Logger_SetMinimumSeverityForDiagnosticEvent(int32_t severity) {
    if (severity < 0 || severity >= SEVERITY_MAX) {
        Logger_Error("failed setting diagnostic event minimum severity, out of range");
        return false;
    }
    diagnosticEventMinimumSeverity = (Severity)severity;
    return true;
}

#endif