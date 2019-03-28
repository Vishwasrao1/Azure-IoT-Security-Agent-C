// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/listening_ports_collector.h"

#include <stdlib.h>

#include "collectors/generic_event.h"
#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "message_schema_consts.h"
#include "os_utils/listening_ports_iterator.h"

#define MAX_RECORD_VALUE_LENGTH 128
static const char* TCP_PROTOCL = "tcp";
static const char* UDP_PROTOCL = "udp";

/**
 * @brief Adds the ports payload to the object writer.
 * 
 * @param   listeningPortsEventWriter   The parent object writer to writes the paload to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult ListeningPortCollector_AddPorts(JsonObjectWriterHandle listeningPortsEventWriter);

/**
 * @brief Adds the ports payload to the object writer.
 * 
 * @param   listeningPortsEventWriter   The parent object writer to writes the paload to.
 * @param   type                        The type of the ports.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult ListeningPortCollector_AddPortsByType(JsonArrayWriterHandle listeningPortsPayloadArray, ListeningPortsType type);

/**
 * @brief Adds a single netstat revord to the payload array
 * 
 * @param   listeningPortsPayloadArray  The json array writer of the payloads
 * @param   portsIterator               The ports iterator
 * @param   type                        The tyoe of the ports.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult ListeningPortCollector_AddSingleRecord(JsonArrayWriterHandle listeningPortsPayloadArray, ListeningPortsIteratorHandle portsIterator, ListeningPortsType type);


EventCollectorResult ListeningPortCollector_AddPortsByType(JsonArrayWriterHandle listeningPortsPayloadArray, ListeningPortsType type) {

    EventCollectorResult result = EVENT_COLLECTOR_OK;
    ListeningPortsIteratorHandle portsIterator = NULL;

    if (ListenintPortsIterator_Init(&portsIterator, type) != LISTENING_PORTS_ITERATOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    ListeningPortsIteratorResults iteratorResult = ListenintPortsIterator_GetNext(portsIterator);
    while (iteratorResult == LISTENING_PORTS_ITERATOR_HAS_NEXT) {

        if (ListeningPortCollector_AddSingleRecord(listeningPortsPayloadArray, portsIterator, type) != EVENT_COLLECTOR_OK) {
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }

        iteratorResult = ListenintPortsIterator_GetNext(portsIterator);
    }

    if (iteratorResult != LISTENING_PORTS_ITERATOR_NO_MORE_DATA) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (portsIterator != NULL) {
        ListenintPortsIterator_Deinit(portsIterator);
    }
    return result;
}

EventCollectorResult ListeningPortCollector_AddSingleRecord(JsonArrayWriterHandle listeningPortsPayloadArray, ListeningPortsIteratorHandle portsIterator, ListeningPortsType type) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle payloadWriter = NULL;

    if (JsonObjectWriter_Init(&payloadWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (type == LISTENING_PORTS_TCP) {
        if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_PROTOCOL_KEY, TCP_PROTOCL) != JSON_WRITER_OK) {
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }   
    } else if (type == LISTENING_PORTS_UDP) {
        if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_PROTOCOL_KEY, UDP_PROTOCL) != JSON_WRITER_OK) {
            result = EVENT_COLLECTOR_EXCEPTION;
            goto cleanup;
        }  
    }

    char value[MAX_RECORD_VALUE_LENGTH] = "";
    if (ListenintPortsIterator_GetLocalAddress(portsIterator, value, sizeof(value)) != LISTENING_PORTS_ITERATOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_LOCAL_ADDRESS_KEY, value) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }   
    
    memset(value, 0, sizeof(value));
    if (ListenintPortsIterator_GetLocalPort(portsIterator, value, sizeof(value)) != LISTENING_PORTS_ITERATOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_LOCAL_PORT_KEY, value) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }   

    memset(value, 0, sizeof(value));
    if (ListenintPortsIterator_GetRemoteAddress(portsIterator, value, sizeof(value)) != LISTENING_PORTS_ITERATOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_REMOTE_ADDRESS_KEY, value) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }   

    memset(value, 0, sizeof(value));
    if (ListenintPortsIterator_GetRemotePort(portsIterator, value, sizeof(value)) != LISTENING_PORTS_ITERATOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    if (JsonObjectWriter_WriteString(payloadWriter, LISTENING_PORTS_REMOTE_PORT_KEY, value) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_AddObject(listeningPortsPayloadArray, payloadWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (payloadWriter != NULL) {
        JsonObjectWriter_Deinit(payloadWriter);
    }
    return result;
}

EventCollectorResult ListeningPortCollector_AddPorts(JsonObjectWriterHandle listeningPortsEventWriter) {

    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonArrayWriterHandle listeningPortsPayloadArray = NULL;

    if (JsonArrayWriter_Init(&listeningPortsPayloadArray) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (ListeningPortCollector_AddPortsByType(listeningPortsPayloadArray, LISTENING_PORTS_TCP) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if (ListeningPortCollector_AddPortsByType(listeningPortsPayloadArray, LISTENING_PORTS_UDP) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = GenericEvent_AddPayload(listeningPortsEventWriter, listeningPortsPayloadArray); 
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

cleanup:
    if (listeningPortsPayloadArray != NULL) {
        JsonArrayWriter_Deinit(listeningPortsPayloadArray);
    }
    return result;
}

EventCollectorResult ListeningPortCollector_GetEvents(SyncQueue* queue) {

    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle listeningPortsEventWriter = NULL;
    char* messageBuffer = NULL;

    if (JsonObjectWriter_Init(&listeningPortsEventWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (GenericEvent_AddMetadata(listeningPortsEventWriter, EVENT_PERIODIC_CATEGORY, LISTENING_PORTS_NAME, EVENT_TYPE_SECURITY_VALUE, LISTENING_PORTS_PAYLOAD_SCHEMA_VERSION) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (ListeningPortCollector_AddPorts(listeningPortsEventWriter) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }    
    
    uint32_t messageBufferSize = 0;
    if (JsonObjectWriter_Serialize(listeningPortsEventWriter, &messageBuffer, &messageBufferSize) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (SyncQueue_PushBack(queue, messageBuffer, messageBufferSize) != QUEUE_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:

    if (result != EVENT_COLLECTOR_OK) {
        if (messageBuffer !=  NULL) {
            free(messageBuffer);
        }
    }
    
    if (listeningPortsEventWriter != NULL) {
        JsonObjectWriter_Deinit(listeningPortsEventWriter);
    }

    return result;
}