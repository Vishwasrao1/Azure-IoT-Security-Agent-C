// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SYNCHRONIZED_QUEUE_H
#define SYNCHRONIZED_QUEUE_H

#include <stdint.h>

#include "azure_c_shared_utility/lock.h"
#include "macro_utils.h"
#include "umock_c_prod.h"

#include "queue.h"

typedef enum _SyncQueueResultValues {

    SYNC_QUEUE_LOCK_EXCEPTION = 100 // we start from here since sync queue may return all of queue regular result values and the new values
    
} SyncQueueResultValues;

typedef struct _SyncQueue {

    Queue queue; 
    LOCK_HANDLE lock;

} SyncQueue;

/**
 * @brief Initiate the queue
 * 
 * @param   syncQueue           The instance to initiate.
 * @param   shouldSendLogs      Whether this instance should send out logs.
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, int, SyncQueue_Init, SyncQueue*, syncQueue, bool, shouldSendLogs);

/**
 * @brief Deinitiate the given queue and free it memory
 * 
 * @param   syncQueue   the instance to deinitiate.
 */
MOCKABLE_FUNCTION(, void, SyncQueue_Deinit, SyncQueue*, syncQueue);

/**
 * @brief Push an item to the end of the queue
 * 
 * @param   syncQueue   The instance to push the item to.
 * @param   data        The data to push to the end of the queue.
 * @param   dataSize    The size of the data we want ot push to the queue.
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, int, SyncQueue_PushBack, SyncQueue*, syncQueue, void*, data, uint32_t, dataSize);

/**
 * @brief Pops an item from the beginning of the queue
 * 
 * @param   syncQueue   The instance to push the item to.
 * @param   data        Out param. The data we poped from the queue.
 * @param   dataSize    Out param. The size of the data we poped from the queue.
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, int, SyncQueue_PopFront, SyncQueue*, syncQueue, void**, data, uint32_t*, dataSize);

/**
 * Pops an item from the beginning of the queue only if the condition erturns true on this item.
 * 
 * @param   syncQueue           The queue to push to
 * @param   data                out param containing the data of the item that was poped from the queue
 * @param   dataSize            out param containing the size of the data that was poped from the queue
 * @param   condition           A condition for poping the elements from the queue. 
 * @param   conditionParams     Extra parameters for the condition function.
 * 
 * @return QUEUE_OK on success or an error code upon failure.
 */
MOCKABLE_FUNCTION(, int, SyncQueue_PopFrontIf, SyncQueue*, syncQueue, QueuePopCondition, condition, void*, conditionParams, void**, data, uint32_t*, dataSize);

/**
 * @brief Returns the queue size
 * 
 * @param   syncQueue   The queue whom size we want to get.
 * @param   size        Out param. The size of the queue.
 * 
 * @return always return QUEUE_OK.
 */
MOCKABLE_FUNCTION(, int, SyncQueue_GetSize, SyncQueue*, syncQueue, uint32_t*, size);

#endif //SYNCHRONIZED_QUEUE_H