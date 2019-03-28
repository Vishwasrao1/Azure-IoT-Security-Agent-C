// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AGENT_TELEMETRY_COUNTER_H
#define AGENT_TELEMETRY_COUNTER_H
#include <stdint.h>
#include <stdbool.h>

#include "azure_c_shared_utility/lock.h"
#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * A struct which represents queue performance counter, for metering the queue
 */
typedef struct _QueueCounter {
    
    uint32_t collected;
    uint32_t dropped;

} QueueCounter;

/**
 * A struct which represents iot hub client performance counter
 **/
typedef struct _MessagesCounter {

    uint32_t sentMessages;
    uint32_t smallMessages;
    uint32_t failedMessages;

} MessageCounter;

/**
 * A union which represents a counter
 **/
typedef union _Counter {
    MessageCounter messageCounter;
    QueueCounter queueCounter;
} Counter;

/**
 * A struct which a synced counter
 **/
typedef struct _SyncedCounter {
    Counter counter;
    LOCK_HANDLE lock;
} SyncedCounter;

/**
 * @brief initialize the counter
 * 
 * @param counter   the counter to init.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, AgentTelemetryCounter_Init, SyncedCounter*, counter);

/**
 * @brief deinitialize the counter.
 * 
 * @param counter   the counter to deinit
 */
MOCKABLE_FUNCTION(, void, AgentTelemetryCounter_Deinit, SyncedCounter*, counter);

/**
 * @brief takes a snapshot of current counter state and resets the counter
 * 
 * @param inCounter       The counter to take the snapshot from.
 * @param outData    the snapshot data
 * 
 * @return true on success.
 */
MOCKABLE_FUNCTION(, bool, AgentTelemetryCounter_SnapshotAndReset, SyncedCounter*, inCounter, Counter*, outData);

/**
 * @brief increases the counter by the given amount
 * 
 * @param counter       counter handle.
 * @param countInstance the field to increase
 * @param amount        the amount to increase
 * 
 * @return TELEMETRY_PROVIDER_OK on success, TELEMETRY_PROVIDER_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, bool, AgentTelemetryCounter_IncreaseBy, SyncedCounter*, counter, uint32_t*, countInstance, uint32_t, amount);


#endif // AGENT_TELEMETRY_COUNTER_H