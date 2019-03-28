// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include "queue.h"

#include <stdlib.h>

#include "logger.h"
#include "memory_monitor.h"
#include "agent_telemetry_counters.h"

static uint32_t Queue_CalculateItemSize(uint32_t dataSize) {
    // each item in the list has a struct cllocate for it, a pointer to this struct and the data size for the data itself
    return dataSize + sizeof(QueueItem) + sizeof(QueueItem*);
}

QueueResultValues Queue_Init(Queue* queue, bool shouldSendLogs) {
    queue->numberOfElements = 0;
    queue->firstItem = NULL;
    queue->lastItem = NULL;
    queue->shouldSendLogs = shouldSendLogs;
    if (!AgentTelemetryCounter_Init(&(queue->counter))){
        return QUEUE_MEMORY_EXCEPTION;
    }

    return QUEUE_OK;
}

void Queue_Deinit(Queue* queue) {
    AgentTelemetryCounter_Deinit(&queue->counter);
    
    void* currentData;
    uint32_t currentDataSize;
    while (queue->numberOfElements > 0) {
        if (Queue_PopFront(queue, &currentData, &currentDataSize) == QUEUE_OK) {
            free(currentData);
        }
    }
}

QueueResultValues Queue_PushBack(Queue* queue, void* data, uint32_t dataSize) {
    int result = QUEUE_OK;
    bool memoryConsumed = false;

    result = MemoryMonitor_Consume(Queue_CalculateItemSize(dataSize));
    if (result != MEMORY_MONITOR_OK) {
        if (result == MEMORY_MONITOR_MEMORY_EXCEEDED) {
            if (queue->shouldSendLogs) { 
                Logger_Information("Max cache size exceeded"); 
            }
            result = QUEUE_MAX_MEMORY_EXCEEDED;
            AgentTelemetryCounter_IncreaseBy(&queue->counter, &queue->counter.counter.queueCounter.dropped, 1);
        }
        else if (result == MEMORY_MONITOR_EXCEPTION) {
            if (queue->shouldSendLogs) { 
                Logger_Error("critical memory exception"); 
            }
            result = QUEUE_MEMORY_EXCEPTION;
        }
        goto cleanup;
    }
    memoryConsumed = true;

    QueueItem* newItem = (QueueItem*)malloc(sizeof(QueueItem));
    if (newItem == NULL) {
        result = QUEUE_MEMORY_EXCEPTION;
        goto cleanup;
    }

    newItem->data = data;
    newItem->dataSize = dataSize;
    newItem->nextItem = NULL;

    if (queue->firstItem == NULL) {
        // queue is empty
        queue->firstItem = newItem;
        queue->lastItem = newItem;
        newItem->prevItem = NULL;
    } else {
        queue->lastItem->nextItem = newItem;
        newItem->prevItem = queue->lastItem;
        queue->lastItem = newItem;
    }
    ++queue->numberOfElements;
cleanup:
    if (result != QUEUE_OK) {
        if (memoryConsumed) {
            MemoryMonitor_Release(Queue_CalculateItemSize(dataSize));
        }
    }

    AgentTelemetryCounter_IncreaseBy(&queue->counter, &queue->counter.counter.queueCounter.collected, 1);
    return result;
}

QueueResultValues Queue_PopFront(Queue* queue, void** data, uint32_t* dataSize) {
    if (queue->numberOfElements == 0) {
        return QUEUE_IS_EMPTY;
    }

    QueueItem* item = queue->firstItem;
    *data = item->data;
    *dataSize = item->dataSize;

    if (queue->firstItem->nextItem == NULL) {
        queue->firstItem = NULL;
        queue->lastItem = NULL;
    } else {
        queue->firstItem = item->nextItem;
        queue->firstItem->prevItem = NULL;
    }

    --queue->numberOfElements;
    MemoryMonitor_Release(Queue_CalculateItemSize(*dataSize));
    // we allocated the item itself while inserting it, so we soquld free its memory here
    free(item); 
    return QUEUE_OK;
}

QueueResultValues Queue_PopFrontIf(Queue* queue, QueuePopCondition condition, void* extraParams, void** data, uint32_t* dataSize) {
    if (queue->numberOfElements == 0) {
        return QUEUE_IS_EMPTY;
    }

    QueueItem* item = queue->firstItem;
    if (!condition(item->data, item->dataSize, extraParams)) {
        return QUEUE_CONDITION_FAILED;
    }
    return Queue_PopFront(queue, data, dataSize);
}

QueueResultValues Queue_GetSize(Queue* queue, uint32_t* size) {
    *size = queue->numberOfElements;
    return QUEUE_OK;
}