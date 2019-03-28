// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

#if !defined(DISABLE_LOGS) && !defined(TEST_LOG)

void Logger_SendDiagnostic(Severity severity, const char *__restrict __fmt, ...) {

    if (!DiagnosticEventCollector_IsInitialized()) {
        return;
    }

    va_list args;
    va_start(args, __fmt);
    char buf[LOG_MAX_BUFF];
    if (vsnprintf(buf, LOG_MAX_BUFF, __fmt, args) > 0) {
        DiagnosticEventCollector_AddEvent(buf, severity);
    }
    va_end(args);
}

#endif