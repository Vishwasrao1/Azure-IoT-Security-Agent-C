// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef INTERNAL_MEMORY_MONITOR_H
#define INTERNAL_MEMORY_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

#include "umock_c_prod.h"
#include "macro_utils.h"

typedef enum _MemoryMonitorResultValues {

    MEMORY_MONITOR_OK,
    MEMORY_MONITOR_MEMORY_EXCEEDED,
    MEMORY_MONITOR_INVALID_RELEASE_SIZE,
    MEMORY_MONITOR_EXCEPTION
    
} MemoryMonitorResultValues;

/**
 * @brief An internal mermoy monitor which should limit the cache size. This is used in the regular\synced memory monitor.
 */
MOCKABLE_FUNCTION(, void, InternalMemoryMonitor_Init);

/**
 * @brief Deinitiate the memory monitor.
 */
MOCKABLE_FUNCTION(, void, InternalMemoryMonitor_Deinit);

/**
 * @brief Consume the given amount of bytes from the memory limitation.
 * 
 * @param   sizeInBytes    The size one wants to allocate.
 * 
 * @return MEMORY_MONITOR_OK in case there is enough memory, MEMORY_MONITOR_MEMORY_EXCEEDED in case the memory exceeded or error in case of failure.
 */

MOCKABLE_FUNCTION(, MemoryMonitorResultValues, InternalMemoryMonitor_Consume, uint32_t, sizeInBytes);

/**
 * @brief Frees the given amount of bytes from the memory limitation.
 * 
 * @param   sizeInBytes    The size that is now cleared from the limitation
 * 
 * @return MEMORY_MONITOR_OK in case the size was released or error in case of failure.
 */
MOCKABLE_FUNCTION(, MemoryMonitorResultValues, InternalMemoryMonitor_Release, uint32_t, sizeInBytes);

/**
 * @brief Returns the current memory consumption.
 * 
 * @param   sizeInBytes    Out param. The current memory consumption.
 * 
 * @return MEMORY_MONITOR_OK.
 */
MOCKABLE_FUNCTION(, MemoryMonitorResultValues, InternalMemoryMonitor_CurrentConsumption, uint32_t*, sizeInBytes);

#endif //INTERNAL_MEMORY_MONITOR_H