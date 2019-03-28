// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "synchronized_queue.h"

int SyncQueue_Init(SyncQueue* syncQueue, bool shouldSendLogs) {
    QueueResultValues result = Queue_Init(&syncQueue->queue, shouldSendLogs);
    if (result != QUEUE_OK) {
        return result;
    }

    syncQueue->lock = Lock_Init();
    if (syncQueue->lock == NULL) {
        Queue_Deinit(&syncQueue->queue);
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }

    return QUEUE_OK;
}

void SyncQueue_Deinit(SyncQueue* syncQueue) {
    Queue_Deinit(&syncQueue->queue);

    if (syncQueue->lock != NULL) {
        Lock_Deinit(syncQueue->lock);
    }
}

int SyncQueue_PushBack(SyncQueue* syncQueue, void* data, uint32_t dataSize) {
    if (Lock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }
    
    QueueResultValues result = Queue_PushBack(&syncQueue->queue, data, dataSize);
                                
    if (Unlock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }
    return result;
}

int SyncQueue_PopFront(SyncQueue* syncQueue, void** data, uint32_t* dataSize) {
    if (Lock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }          
    
    QueueResultValues result = Queue_PopFront(&syncQueue->queue, data, dataSize);
                                
    if (Unlock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }
    return result;
}

int SyncQueue_PopFrontIf(SyncQueue* syncQueue, QueuePopCondition condition, void* conditionParams, void** data, uint32_t* dataSize) {
    if (Lock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }          
    
    QueueResultValues result = Queue_PopFrontIf(&syncQueue->queue, condition, conditionParams, data, dataSize);
                                
    if (Unlock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }
    return result;
}

int SyncQueue_GetSize(SyncQueue* syncQueue, uint32_t* size) {
    if (Lock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }          
    
    QueueResultValues result = Queue_GetSize(&syncQueue->queue, size);
                                
    if (Unlock(syncQueue->lock) != LOCK_OK) {
        return SYNC_QUEUE_LOCK_EXCEPTION;
    }
    return result;
}