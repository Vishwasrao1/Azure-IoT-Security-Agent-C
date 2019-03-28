// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/diagnostic_event_collector.h"

#include <stdint.h>
#include <stdlib.h>

#include "internal/time_utils.h"
#include "json/json_defs.h"
#include "json/json_object_writer.h"
#include "json/json_array_writer.h"
#include "message_schema_consts.h"
#include "os_utils/os_utils.h"
#include "os_utils/correlation_manager.h"
#include "utils.h"

/**
 * @brief adds a payload to the json array.
 * 
 * @param   event                       The event.
 * @param   diagnosticEventsArray       The json array.
 * 
 * @return EVENT_COLLECTOR_OK on success, and event will be contained the json array.
 */
EventCollectorResult DiagnosticEventCollector_AddPayload(DiagnosticEvent* event, JsonArrayWriterHandle diagnosticEventsArray);

/**
 * @brief adds a single dignostic event to the event queue.
 * 
 * @param   event                       The event.
 * @param   priorityQueue               The event queue.
 * 
 * @return EVENT_COLLECTOR_OK on success, and event will be contained the json array.
 */
EventCollectorResult DiagnosticEventCollector_GetSingleEvent(DiagnosticEvent* event, SyncQueue* priorityQueue);

/**
 * @brief innitializes a dignostic event.
 * 
 * @param   output                      The event to init.
 * @param   outputSize                  The size event.
 * @param   message                     The message.
 * @param   severity                    The severity.
 * 
 * @return EVENT_COLLECTOR_OK on success.
 */
EventCollectorResult DiagnosticEventCollector_InitDiagnosticEvent(DiagnosticEvent** output, uint32_t* outputSize, char* message, Severity severity);

/**
 * @brief Deinnitializes a dignostic event.
 * 
 * @param   event    The event.
 */
void DiagnosticEventCollector_DeinitDiagnosticEvent(DiagnosticEvent* event);

/**
 * @brief Converts an enum to a string.
 * 
 * @param   severity    a severity.
 * 
 * @returns the string represntation of the severity enum.
 */
char* DiagnosticEventCollector_ConvertToString(Severity severity);

typedef struct _DiagnosticEventCollector {
    SyncQueue* eventsQueue;
} DiagnosticEventCollector;

static DiagnosticEventCollector diagnosticEventCollector = { NULL };

EventCollectorResult DiagnosticEventCollector_Init(SyncQueue* eventsQueue) {
    diagnosticEventCollector.eventsQueue = eventsQueue;
    CorrelationManager_Init();

    return EVENT_COLLECTOR_OK;
}

void DiagnosticEventCollector_Deinit() {

    void* currentData;
    uint32_t currentDataSize;
    uint32_t queueSize = 0;

    if (SyncQueue_GetSize(diagnosticEventCollector.eventsQueue, &queueSize) != QUEUE_OK) {
        goto cleanup;
    }

    while (queueSize > 0) {
        if (SyncQueue_PopFront(diagnosticEventCollector.eventsQueue, &currentData, &currentDataSize) != QUEUE_OK) {
            goto cleanup;
        }

        DiagnosticEventCollector_DeinitDiagnosticEvent(currentData);
        if (SyncQueue_GetSize(diagnosticEventCollector.eventsQueue, &queueSize) != QUEUE_OK) {
            goto cleanup;
        }
    }

cleanup:
    diagnosticEventCollector.eventsQueue = NULL;
    CorrelationManager_Deinit();
}

bool DiagnosticEventCollector_IsInitialized() {
    return (diagnosticEventCollector.eventsQueue != NULL);
}

EventCollectorResult DiagnosticEventCollector_AddEvent(char* message, Severity severity) {
    DiagnosticEvent *event = NULL;
    uint32_t eventSize;
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    if (DiagnosticEventCollector_InitDiagnosticEvent(&event, &eventSize, message, severity) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (SyncQueue_PushBack(diagnosticEventCollector.eventsQueue, event, eventSize) != QUEUE_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (result == EVENT_COLLECTOR_EXCEPTION && event != NULL) {
        DiagnosticEventCollector_DeinitDiagnosticEvent(event);
    }

    return result;
}

EventCollectorResult DiagnosticEventCollector_GetEvents(SyncQueue* priorityQueue) {
    uint32_t eventQueueSize;
    DiagnosticEvent* event = NULL;
    uint32_t eventSize;

    if (SyncQueue_GetSize(diagnosticEventCollector.eventsQueue, &eventQueueSize) != QUEUE_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    for (int counter = eventQueueSize; counter > 0; counter--) {
        event = NULL;
        int queueResult = SyncQueue_PopFront(diagnosticEventCollector.eventsQueue, (void**)&event, &eventSize);

        // this case can only happen if the queue had been popped by another thread, this shouldn't happen in the current implementation
        if (queueResult == QUEUE_IS_EMPTY) {
            return EVENT_COLLECTOR_OK;
        } else if (queueResult != QUEUE_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }

        EventCollectorResult result = DiagnosticEventCollector_GetSingleEvent(event, priorityQueue);

        DiagnosticEventCollector_DeinitDiagnosticEvent(event);

        if (result != EVENT_COLLECTOR_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    }

    return EVENT_COLLECTOR_OK;
}

EventCollectorResult DiagnosticEventCollector_AddPayload(DiagnosticEvent* event, JsonArrayWriterHandle diagnosticEventsArray) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    JsonObjectWriterHandle eventObject = NULL;
    if (JsonObjectWriter_Init(&eventObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(eventObject, DIAGNOSTIC_MESSAGE_KEY, event->message) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(eventObject,DIAGNOSTIC_SEVERITY_KEY, DiagnosticEventCollector_ConvertToString(event->severity)) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(eventObject, DIAGNOSTIC_PROCESSID_KEY, event->processId) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteInt(eventObject, DIAGNOSTIC_THREAD_KEY, event->threadId) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteString(eventObject, DIAGNOSTIC_CORRELATION_KEY, event->correlationId) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_AddObject(diagnosticEventsArray, eventObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (eventObject != NULL) {
        JsonObjectWriter_Deinit(eventObject);
    }

    return result;
}

EventCollectorResult DiagnosticEventCollector_GetSingleEvent(DiagnosticEvent* event, SyncQueue* priorityQueue) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonArrayWriterHandle diagnosticEventsArray = NULL;
    JsonObjectWriterHandle rootObject = NULL;
    char* buffer = NULL;
    uint32_t bufferSize;

    if (JsonObjectWriter_Init(&rootObject) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (GenericEvent_AddMetadataWithTimes(rootObject, EVENT_TRIGGERED_CATEGORY, DIAGNOSTIC_NAME, EVENT_TYPE_DIAGNOSTIC_VALUE, DIAGNOSTIC_PAYLOAD_SCHEMA_VERSION, &(event->timeLocal)) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&diagnosticEventsArray) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (DiagnosticEventCollector_AddPayload(event, diagnosticEventsArray) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddPayload(rootObject, diagnosticEventsArray);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    if (JsonObjectWriter_Serialize(rootObject, &buffer, &bufferSize) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (SyncQueue_PushBack(priorityQueue, buffer, bufferSize) != QUEUE_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        free(buffer);
        goto cleanup;
    }

cleanup:
    if (diagnosticEventsArray != NULL) {
        JsonArrayWriter_Deinit(diagnosticEventsArray);
    }

    if (rootObject != NULL) {
        JsonObjectWriter_Deinit(rootObject);
    }

    return result;
}

EventCollectorResult DiagnosticEventCollector_InitDiagnosticEvent(DiagnosticEvent** output, uint32_t* outputSize, char* message, Severity severity) {
    DiagnosticEvent *event = malloc(sizeof(DiagnosticEvent));
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    if (event == NULL) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (Utils_CreateStringCopy(&event->message, message) == false) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    event->severity = severity;
    event->processId = OsUtils_GetProcessId();
    event->threadId = OsUtils_GetThreadId();
    event->timeLocal = TimeUtils_GetCurrentTime();

    if (Utils_CreateStringCopy(&event->correlationId, CorrelationManager_GetCorrelation()) == false) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    *output = event;
    *outputSize = sizeof(DiagnosticEvent) + sizeof(DiagnosticEvent*) + strlen(message) +1;

cleanup:
    if (result != EVENT_COLLECTOR_OK && event != NULL) {
        DiagnosticEventCollector_DeinitDiagnosticEvent(event);
    }

    return result;
}

bool DiagnosticEventCollector_SetCorrelationId() {
     return CorrelationManager_SetCorrelation();
}

void DiagnosticEventCollector_DeinitDiagnosticEvent(DiagnosticEvent* event) {
    if (event->message != NULL) {
        free(event->message);
    }
    if (event->correlationId != NULL) {
        free(event->correlationId);
    }
    free(event);
}

char* DiagnosticEventCollector_ConvertToString(Severity severity) {
    switch (severity) {
        case SEVERITY_DEBUG: return (char*) DIAGNOSTIC_SEVERITY_DEBUG_VALUE;
        case SEVERITY_INFORMATION: return (char*) DIAGNOSTIC_SEVERITY_INFORMATION_VALUE;
        case SEVERITY_WARNING: return (char*) DIAGNOSTIC_SEVERITY_WARNING_VALUE;
        case SEVERITY_ERROR: return (char*) DIAGNOSTIC_SEVERITY_ERROR_VALUE;
        case SEVERITY_FATAL: return (char*) DIAGNOSTIC_SEVERITY_FATAL_VALUE;
        default: return (char*) DIAGNOSTIC_SEVERITY_WARNING_VALUE;
    }
}