// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "tasks/event_publisher_task.h"

#include <stdint.h>
#include <stdlib.h>

#include "internal/time_utils.h"
#include "logger.h"
#include "memory_monitor.h"
#include "message_serializer.h"
#include "twin_configuration.h"

/**
 * @brief Dequeue all the events in the queue and send them to the hub
 * 
 * @param   task            The task instance.
 * @param   mainQueue       The main message queue.
 * @param   paddingQueue    A queue to add messages from in case the main queue does not have enough data.
 * 
 * @return true on success, false otherwise.
 */

bool EventPublisherTask_SendEvents(EventPublisherTask* task, SyncQueue* mainQueue, SyncQueue* paddingQueue);

bool EventPublisherTask_Init(EventPublisherTask* task, SyncQueue* highPriorityEventQueue, SyncQueue* lowPriorityEventQueue, SyncQueue* operationalEventsQueue, IoTHubAdapter* iothubAdapter) {
    task->operationalEventsQueue = operationalEventsQueue;
    task->lowPriorityEventQueue = lowPriorityEventQueue;
    task->highPriorityEventQueue = highPriorityEventQueue;
    task->iothubAdapter = iothubAdapter;
    task->highPriorityQueueLastExecution = TimeUtils_GetCurrentTime();
    task->lowPriorityQueueLastExecution = TimeUtils_GetCurrentTime();
    return true;
}

void EventPublisherTask_Deinit(EventPublisherTask* task) {
    task->lowPriorityEventQueue = NULL;
    task->highPriorityEventQueue = NULL;
    task->iothubAdapter = NULL;
}

void EventPublisherTask_Execute(EventPublisherTask* task) {

    uint32_t highPriorityQueueFrequency = 0;
    if (TwinConfiguration_GetHighPriorityMessageFrequency(&highPriorityQueueFrequency) != TWIN_OK) {
        return;
    }

    uint32_t lowPriorityQueueFrequency = 0;
    if (TwinConfiguration_GetLowPriorityMessageFrequency(&lowPriorityQueueFrequency) != TWIN_OK) {
        return;
    }

    uint32_t maxMessageSize = 0;
    if (TwinConfiguration_GetMaxMessageSize(&maxMessageSize) != TWIN_OK) {
        return;
    }
    
    uint32_t currentMemoryConsumption = 0;
    if (MemoryMonitor_CurrentConsumption(&currentMemoryConsumption) != MEMORY_MONITOR_OK) {
        return;
    }

    time_t currentTime = TimeUtils_GetCurrentTime();

    if (currentMemoryConsumption > maxMessageSize) {
        // If we got to the max message size in the queue, we are sending the message as high priority even if there
        // weren't any actual high priority events
        EventPublisherTask_SendEvents(task, task->highPriorityEventQueue, task->lowPriorityEventQueue);
        task->highPriorityQueueLastExecution = currentTime;
    }

    // time is in seconds so we convert it to milliseconds here
    uint32_t highPriorityQueueTimeDiff = TimeUtils_GetTimeDiff(currentTime, task->highPriorityQueueLastExecution);
    uint32_t lowPriorityQueueTimeDiff = TimeUtils_GetTimeDiff(currentTime, task->lowPriorityQueueLastExecution);
    
    if (highPriorityQueueTimeDiff > highPriorityQueueFrequency) {
        EventPublisherTask_SendEvents(task, task->highPriorityEventQueue, task->lowPriorityEventQueue);
        task->highPriorityQueueLastExecution = currentTime;
    }
    
    if (lowPriorityQueueTimeDiff > lowPriorityQueueFrequency) {
        EventPublisherTask_SendEvents(task, task->lowPriorityEventQueue, task->highPriorityEventQueue);
        task->lowPriorityQueueLastExecution = currentTime;
    }
}

bool EventPublisherTask_SendEvents(EventPublisherTask* task, SyncQueue* mainQueue, SyncQueue* paddingQueue) {
    void* buffer = NULL;
    bool result = true;

    uint32_t queueSize = 0;
    if (SyncQueue_GetSize(mainQueue, &queueSize) != QUEUE_OK){
        return false;
    }

    if (queueSize == 0){
        return true;
    }

    SyncQueue* queuesOrder[] = {task->operationalEventsQueue, mainQueue, paddingQueue};
    MessageSerializerResultValues serializationResult = MessageSerializer_CreateSecurityMessage(queuesOrder, 3, &buffer);
    if (serializationResult != MESSAGE_SERIALIZER_OK && serializationResult != MESSAGE_SERIALIZER_PARTIAL) {
        return false;
    }

    if (buffer != NULL) {
        uint32_t size = strlen(buffer);
        if (!IoTHubAdapter_SendMessageAsync(task->iothubAdapter, buffer, size)) {
                //FIXME: do we want to stop sedning message in this case?
                result = false;
                Logger_Error("error sending a message to the hub");
        }
        
        free(buffer);
    }

    return result;
}
