// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "internal/internal_memory_monitor.h"

#include "consts.h"
#include "twin_configuration.h"

static uint32_t currentConsumptionInBytes;
static uint32_t memoryLimitInBytes;

void InternalMemoryMonitor_Init() {
    memoryLimitInBytes = DEFAULT_MAX_LOCAL_CACHE_SIZE;
    currentConsumptionInBytes = 0;
}

void InternalMemoryMonitor_Deinit() {
    currentConsumptionInBytes = 0;
    memoryLimitInBytes = 0;
}

MemoryMonitorResultValues InternalMemoryMonitor_Consume(uint32_t size) {
    if (TwinConfiguration_GetMaxLocalCacheSize(&memoryLimitInBytes) != TWIN_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }

    if (currentConsumptionInBytes + size > memoryLimitInBytes) {
        return MEMORY_MONITOR_MEMORY_EXCEEDED;
    }

    currentConsumptionInBytes += size;
    return MEMORY_MONITOR_OK;
}

MemoryMonitorResultValues InternalMemoryMonitor_Release(uint32_t size) {
    if (size > currentConsumptionInBytes) {
        return MEMORY_MONITOR_INVALID_RELEASE_SIZE;
    }

    currentConsumptionInBytes -= size;
    return MEMORY_MONITOR_OK;
}

MemoryMonitorResultValues InternalMemoryMonitor_CurrentConsumption(uint32_t* sizeInBytes) {
    *sizeInBytes = currentConsumptionInBytes;
    return MEMORY_MONITOR_OK;
}
