// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "memory_monitor.h"

bool MemoryMonitor_Init() {
    InternalMemoryMonitor_Init();
    return true;
}

void MemoryMonitor_Deinit() {
    InternalMemoryMonitor_Deinit();
}

MemoryMonitorResultValues MemoryMonitor_Consume(uint32_t sizeInBytes) {
    return InternalMemoryMonitor_Consume(sizeInBytes);
}

MemoryMonitorResultValues MemoryMonitor_Release(uint32_t sizeInBytes) {
    return InternalMemoryMonitor_Release(sizeInBytes);
}

MemoryMonitorResultValues MemoryMonitor_CurrentConsumption(uint32_t* sizeInBytes) {
    return InternalMemoryMonitor_CurrentConsumption(sizeInBytes);
}