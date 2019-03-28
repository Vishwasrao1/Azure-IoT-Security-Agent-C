// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "agent_telemetry_counters.h"

/**
 * All the result types of the queue functions
 */
typedef enum _QueueResultValues {

    QUEUE_OK,
    QUEUE_CONDITION_FAILED,
    QUEUE_MAX_MEMORY_EXCEEDED,
    QUEUE_IS_EMPTY,
    QUEUE_MEMORY_EXCEPTION
    
} QueueResultValues;

/**
 * A struct which represents an item in the queue
 */
typedef struct _QueueItem
{
    void* data;
    uint32_t dataSize;
    void* nextItem;
    void* prevItem; 
    
} QueueItem;

/**
 * A queue struct
 */
typedef struct _Queue
{
    QueueItem* firstItem; 
    QueueItem* lastItem;
    uint32_t numberOfElements;
    bool shouldSendLogs;
    SyncedCounter counter;
} Queue;

/**
 * @brief A condition function for poping elements from the queue.
 * 
 * @param   data                The data of the top elements of the queue.
 * @param   dataSize            The data size of the top elements of the queue. 
 * @param   conditionParams     Extra user defined parameters for this function.
 * 
 * @return true if the elements should be poped, false otherwise.
 */
typedef bool (*QueuePopCondition)(const void* data, uint32_t dataSize, void* conditionParams);

/**
 * @brief initiates the given queue
 * 
 * @prama   queue                   The queue to initiate
 * @prama   qushouldSendLogseue     whther the queue should send out logs
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, QueueResultValues, Queue_Init, Queue*, queue, bool, shouldSendLogs);

/**
 * @brief deinitiate the given queue (free its memory)
 * 
 * @param   queue   The queue to deinitiate
 */
MOCKABLE_FUNCTION(, void, Queue_Deinit, Queue*, queue);

/**
 * @brief push an item to the end of the queue
 * 
 * @param   queue       The queue to push to
 * @param   data        The data to push to the queue
 * @param   dataSize    The size of the data we push into the queue
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, QueueResultValues, Queue_PushBack, Queue*, queue, void*, data, uint32_t, dataSize);

/**
 * Pops an item from the beginning of the queue
 * 
 * @param   queue     The queue to push to
 * @param   data      out param containing the data of the item that was poped from the queue
 * @param   dataSize  out param containing the size of the data that was poped from the queue
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
 MOCKABLE_FUNCTION(, QueueResultValues, Queue_PopFront, Queue*, queue, void**, data, uint32_t*, dataSize);

/**
 * Pops an item from the beginning of the queue only if the condition erturns true on this item.
 * 
 * @param   queue               The queue to push to
 * @param   data                out param containing the data of the item that was poped from the queue
 * @param   dataSize            out param containing the size of the data that was poped from the queue
 * @param   condition           A condition for poping the elements from the queue. 
 * @param   conditionParams     Extra parameters for the condition function.
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, QueueResultValues, Queue_PopFrontIf, Queue*, queue, QueuePopCondition, condition, void*, conditionParams, void**, data, uint32_t*, dataSize);

/**
 * @brief returns the queue size
 * 
 * @param   queue   The queue whom size we want to get
 * @param   size    Out param. The size of the queue in bytes.
 * 
 * @return always returns QUEUE_OK.
 */
MOCKABLE_FUNCTION(, QueueResultValues, Queue_GetSize, Queue*, queue, uint32_t*, size);

#endif //QUEUE_H