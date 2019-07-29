// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include <string.h>

#include "collectors/diagnostic_event_collector.h"

#ifdef DISABLE_LOGS

#define Logger_Init() true
#define Logger_Deinit()
#define Logger_Debug(fmt, ...)
#define Logger_Information(fmt, ...)
#define Logger_Warning(fmt, ...)
#define Logger_Error(fmt, ...)
#define Logger_Fatal(fmt, ...)
#define Logger_SetCorrelation()
#define Logger_SetMinimumSeverityForSystemLogger(severity) true
#define Logger_SetMinimumSeverityForDiagnosticEvent(severity) true

#elif TEST_LOG

#include <stdio.h>

#define Logger_Init() true
#define Logger_Deinit()
#define Logger_Debug(fmt, ...)         printf("Debug: " fmt"\n", ##__VA_ARGS__);
#define Logger_Information(fmt, ...)   printf("Information: " fmt"\n", ##__VA_ARGS__);
#define Logger_Warning(fmt, ...)       printf("Warning: " fmt"\n", ##__VA_ARGS__);
#define Logger_Error(fmt, ...)         printf("Error: " fmt"\n", ##__VA_ARGS__);
#define Logger_Fatal(fmt, ...)         printf("Fatal: " fmt"\n", ##__VA_ARGS__);
#define Logger_SetMinimumSeverityForSystemLogger(severity) true
#define Logger_SetMinimumSeverityForDiagnosticEvent(severity) true

#else

bool Logger_Init();
void Logger_Deinit();
void Logger_LogEvent(Severity severity, const char *__restrict __fmt, ...);
bool Logger_SetMinimumSeverityForSystemLogger(int32_t severity);
bool Logger_SetMinimumSeverityForDiagnosticEvent(int32_t severity);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define Logger_Debug(fmt, ...)          Logger_LogEvent(SEVERITY_DEBUG, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Warning(fmt, ...)        Logger_LogEvent(SEVERITY_WARNING, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Information(fmt, ...)    Logger_LogEvent(SEVERITY_INFORMATION, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Error(fmt, ...)          Logger_LogEvent(SEVERITY_ERROR, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_Fatal(fmt, ...)          Logger_LogEvent(SEVERITY_FATAL, "[%s] " fmt, __FILENAME__, ##__VA_ARGS__)
#define Logger_SetCorrelation()         DiagnosticEventCollector_SetCorrelationId()

#endif

#define LOG_MAX_BUFF                    500

#endif //LOGGER_H