// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include <string.h>

#include "collectors/diagnostic_event_collector.h"

#ifdef DISABLE_LOGS

#define Logger_Debug(fmt, ...)
#define Logger_Information(fmt, ...)
#define Logger_Warning(fmt, ...)
#define Logger_Error(fmt, ...)
#define Logger_Fatal(fmt, ...)
#define Logger_SetCorrelation()

#elif TEST_LOG

#include <stdio.h>

#define Logger_Debug(fmt, ...)         printf("Debug: " fmt"\n", ##__VA_ARGS__);
#define Logger_Information(fmt, ...)   printf("Information: " fmt"\n", ##__VA_ARGS__);
#define Logger_Warning(fmt, ...)       printf("Warning: " fmt"\n", ##__VA_ARGS__);
#define Logger_Error(fmt, ...)         printf("Error: " fmt"\n", ##__VA_ARGS__);
#define Logger_Fatal(fmt, ...)         printf("Fatal: " fmt"\n", ##__VA_ARGS__);

#else

void Logger_SendDiagnostic(Severity severity, const char *__restrict __fmt, ...);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_MAX_BUFF                    500
#define Logger_Debug(fmt, ...)          Logger_SendDiagnostic(SEVERITY_DEBUG, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Warning(fmt, ...)        Logger_SendDiagnostic(SEVERITY_WARNING, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Information(fmt, ...)    Logger_SendDiagnostic(SEVERITY_INFORMATION, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Error(fmt, ...)          Logger_SendDiagnostic(SEVERITY_ERROR, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Fatal(fmt, ...)          Logger_SendDiagnostic(SEVERITY_FATAL, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_SetCorrelation()         DiagnosticEventCollector_SetCorrelationId()

#endif

#endif //LOGGER_H