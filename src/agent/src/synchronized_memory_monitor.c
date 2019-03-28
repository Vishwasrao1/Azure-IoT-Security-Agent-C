// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "memory_monitor.h"

#include "azure_c_shared_utility/lock.h"

static LOCK_HANDLE memoryMonitorLock = NULL;

bool MemoryMonitor_Init() {
    InternalMemoryMonitor_Init();
    
    memoryMonitorLock = Lock_Init();
    if (memoryMonitorLock == NULL) {
        InternalMemoryMonitor_Deinit();
        return false;
    }

    return true;
}

void MemoryMonitor_Deinit() {
    InternalMemoryMonitor_Deinit();
    if (memoryMonitorLock != NULL) {
        Lock_Deinit(memoryMonitorLock);
    }
}

MemoryMonitorResultValues MemoryMonitor_Consume(uint32_t sizeInBytes) {
    if (Lock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }          
    
    MemoryMonitorResultValues result = InternalMemoryMonitor_Consume(sizeInBytes);
                                
    if (Unlock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }

    return result;
}

MemoryMonitorResultValues MemoryMonitor_Release(uint32_t sizeInBytes) {
    if (Lock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }          
    
    MemoryMonitorResultValues result = InternalMemoryMonitor_Release(sizeInBytes);
                                
    if (Unlock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }

    return result;
}

MemoryMonitorResultValues MemoryMonitor_CurrentConsumption(uint32_t* sizeInBytes) {
    if (Lock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }          
    
    MemoryMonitorResultValues result = InternalMemoryMonitor_CurrentConsumption(sizeInBytes);
                                
    if (Unlock(memoryMonitorLock) != LOCK_OK) {
        return MEMORY_MONITOR_EXCEPTION;
    }

    return result;
}