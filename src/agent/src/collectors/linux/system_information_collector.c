// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/system_information_collector.h"

#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "message_schema_consts.h"

/**
 * @brief write the memory info to the given json writer.
 * 
 * @param   sysinfoPayloadWriter  The json payloa writer to add the memory info to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult SystemInformationCollector_AddMemoryInformation(JsonObjectWriterHandle sysinfoPayloadWriter);

/**
 * @brief write the os info (os, version, etc) to the given json writer.
 * 
 * @param   sysinfoPayloadWriter  The json payloa writer to add the memory info to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult SystemInformationCollector_AddOsInformation(JsonObjectWriterHandle sysinfoPayloadWriter);

/**
 * @brief Generates the full kernel version from the given info struct and write it to the json.
 * 
 * @param   sysinfoPayloadWriter    The json object writer to write with.
 * @param   machineInfo             The system info struct.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult SystemInformationCollector_WriteFullKernelVersion(JsonObjectWriterHandle sysinfoPayloadWriter, struct utsname* machineInfo);

EventCollectorResult SystemInformationCollector_WriteFullKernelVersion(JsonObjectWriterHandle sysinfoPayloadWriter, struct utsname* machineInfo) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    char* kernelVersionSummary = NULL;

    // include space and null
    uint32_t releaseLength = strlen(machineInfo->release);
    uint32_t versionLength = strlen(machineInfo->version);
    uint32_t totalSize = releaseLength + 1 + versionLength + 1;

    kernelVersionSummary = calloc(totalSize, 1);
    if (kernelVersionSummary == NULL) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    memcpy(kernelVersionSummary, machineInfo->release, releaseLength);
    memcpy(kernelVersionSummary + releaseLength, " ", 1);
    memcpy(kernelVersionSummary + releaseLength + 1, machineInfo->version, versionLength);

    if (JsonObjectWriter_WriteString(sysinfoPayloadWriter, SYSTEM_INFORMATION_OS_VERSION_KEY, kernelVersionSummary) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }


cleanup:
    if (kernelVersionSummary != NULL) {
        free(kernelVersionSummary);
    }
    return result;
}


EventCollectorResult SystemInformationCollector_AddMemoryInformation(JsonObjectWriterHandle sysinfoPayloadWriter) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    __kernel_ulong_t totalSizeInKb = info.totalram / info.mem_unit / 1024;
    __kernel_ulong_t freeSizeInKb = info.freeram / info.mem_unit / 1024;

    if (JsonObjectWriter_WriteInt(sysinfoPayloadWriter, SYSTEM_INFORMATION_TOTAL_PHYSICAL_MEMORY_KEY, totalSizeInKb) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteInt(sysinfoPayloadWriter, SYSTEM_INFORMATION_FREE_PHYSICAL_MEMORY_KEY, freeSizeInKb) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    return EVENT_COLLECTOR_OK;

}

EventCollectorResult SystemInformationCollector_AddOsInformation(JsonObjectWriterHandle sysinfoPayloadWriter) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    
    struct utsname machineInfo;
    if (uname(&machineInfo) != 0) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteString(sysinfoPayloadWriter, SYSTEM_INFORMATION_OS_NAME_KEY, machineInfo.sysname) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    result = SystemInformationCollector_WriteFullKernelVersion(sysinfoPayloadWriter, &machineInfo);
    if (result != EVENT_COLLECTOR_OK) {
        return result;
    }

    if (JsonObjectWriter_WriteString(sysinfoPayloadWriter, SYSTEM_INFORMATION_OS_ARCHITECTURE_KEY, machineInfo.machine) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    if (JsonObjectWriter_WriteString(sysinfoPayloadWriter, SYSTEM_INFORMATION_HOST_NAME_KEY, machineInfo.nodename) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    return EVENT_COLLECTOR_OK;
}

EventCollectorResult SystemInformationCollector_GeneratePayload(JsonArrayWriterHandle sysinfoPayloadArray) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle sysinfoPayloadWriter = NULL;

    if (JsonObjectWriter_Init(&sysinfoPayloadWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = SystemInformationCollector_AddOsInformation(sysinfoPayloadWriter);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    result = SystemInformationCollector_AddMemoryInformation(sysinfoPayloadWriter);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    if (JsonArrayWriter_AddObject(sysinfoPayloadArray, sysinfoPayloadWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:

    if (sysinfoPayloadWriter != NULL) {
        JsonObjectWriter_Deinit(sysinfoPayloadWriter);
    }
    return result;
}


EventCollectorResult SystemInformationCollector_GetEvents(SyncQueue* queue) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle sysinfoEventWriter = NULL;
    JsonArrayWriterHandle sysinfoPayloadArray = NULL;
    char* messageBuffer = NULL;

    if (JsonObjectWriter_Init(&sysinfoEventWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (GenericEvent_AddMetadata(sysinfoEventWriter, EVENT_PERIODIC_CATEGORY, SYSTEM_INFORMATION_NAME, EVENT_TYPE_SECURITY_VALUE, SYSTEM_INFORMATION_PAYLOAD_SCHEMA_VERSION) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&sysinfoPayloadArray) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = SystemInformationCollector_GeneratePayload(sysinfoPayloadArray);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    result = GenericEvent_AddPayload(sysinfoEventWriter, sysinfoPayloadArray); 
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    uint32_t messageBufferSize = 0;
    if (JsonObjectWriter_Serialize(sysinfoEventWriter, &messageBuffer, &messageBufferSize) != JSON_WRITER_OK) {
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

    if (sysinfoPayloadArray != NULL) {
        JsonArrayWriter_Deinit(sysinfoPayloadArray);
    }
    
    if (sysinfoEventWriter != NULL) {
        JsonObjectWriter_Deinit(sysinfoEventWriter);
    }

    return result;
}