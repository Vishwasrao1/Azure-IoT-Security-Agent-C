// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef DIAGNOSTIC_EVENT_COLLECTOR_H
#define DIAGNOSTIC_EVENT_COLLECTOR_H

#include <time.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "synchronized_queue.h"
#include "collectors/generic_event.h"

typedef enum _Severity {
    SEVERITY_DEBUG,
    SEVERITY_INFORMATION,
    SEVERITY_WARNING,
    SEVERITY_ERROR,
    SEVERITY_FATAL
} Severity;

typedef struct _DiagnosticEvent {
    char* message;
    Severity severity;
    int processId;
    unsigned int threadId;
    time_t timeLocal;
    char* correlationId;
} DiagnosticEvent;

/**
 * @brief Initializes the diagnostic events generator, used to send diagnostic events.
 * 

 * @param   eventsQueue                 The internal event queue to store events.
 * 
 * @return true on EVENT_COLLECTOR_OK.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, DiagnosticEventCollector_Init, SyncQueue*, eventsQueue);

/**
 * @brief Deinitializes the diagnostic events generator, used to send diagnostic events.
 */
MOCKABLE_FUNCTION(, void, DiagnosticEventCollector_Deinit);

/**
 * @brief checks if the event collector had been initialized.
 * 
 * @return true in case it was initilized, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, DiagnosticEventCollector_IsInitialized);

/**
 * @brief Generates a diagnostic event.
 * 
 * @param   message                         The message.
 * @param   severity                        The severity of the message.
 * 
 * @return EVENT_COLLECTOR_OK on success and generate an event.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, DiagnosticEventCollector_AddEvent, char*, message, Severity, severity);

/**
 * @brief Get the current events in the system in a thread safe manner.
 * 
 * @param   data                        The payload generated.
 * @param   priorityQueue               The queue to insret the events to.
 * 
 * @return EVENT_COLLECTOR_OK on success, and data will contain the event's payload and inserted into the queue.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, DiagnosticEventCollector_GetEvents, SyncQueue*, priorityQueue);

/**
 * @brief Sets the current correlation.
 * 
 * @return true in case of success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, DiagnosticEventCollector_SetCorrelationId);


#endif //DIAGNOSTIC_EVENT_COLLECTOR_H