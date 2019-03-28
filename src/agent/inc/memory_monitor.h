// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

#include "internal/internal_memory_monitor.h"
#include "umock_c_prod.h"
#include "macro_utils.h"

/**
 * @brief Mermoy monitor which should limit the cache size.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, MemoryMonitor_Init);

/**
 * @brief Deinitiate the memory monitor.
 */
MOCKABLE_FUNCTION(, void, MemoryMonitor_Deinit);

/**
 * @brief Consume the given amount of bytes from the memory limitation.
 * 
 * @param   sizeInBytes    The size one wants to allocate.
 * 
 * @return MEMORY_MONITOR_OK in case there is enough memory, MEMORY_MONITOR_MEMORY_EXCEEDED in case the memory exceeded or error in case of failure.
 */
MOCKABLE_FUNCTION(, MemoryMonitorResultValues, MemoryMonitor_Consume, uint32_t, sizeInBytes);

/**
 * @brief Frees the given amount of bytes from the memory limitation.
 * 
 * @param   size    The size that is now cleared from the limitation
 * 
 * @return MEMORY_MONITOR_OK in case the size was released or error in case of failure.
 */
MOCKABLE_FUNCTION(, MemoryMonitorResultValues, MemoryMonitor_Release, uint32_t, sizeInBytes);

/**
 * @brief Returns the current memory consumption.
 * 
 * @param   sizeInBytes    Out param. The current memory consumption.
 * 
 * @return MEMORY_MONITOR_OK.
 */
MOCKABLE_FUNCTION(, MemoryMonitorResultValues, MemoryMonitor_CurrentConsumption, uint32_t*, sizeInBytes);

#endif //INTERNAL_MEMORY_MONITOR_H