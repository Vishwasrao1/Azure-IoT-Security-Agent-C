// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "agent_telemetry_counters.h"

bool AgentTelemetryCounter_Init(SyncedCounter* counter){
    memset(&counter->counter, 0, sizeof(Counter));

    counter->lock = Lock_Init();
    if (counter->lock == NULL){
        return false;
    }

    return true;
}

void AgentTelemetryCounter_Deinit(SyncedCounter* counter){
    if (counter->lock){
        Lock_Deinit(counter->lock);
    }

    counter->lock = NULL;
}

bool AgentTelemetryCounter_SnapshotAndReset(SyncedCounter* inCounter, Counter* outData){
    memset(outData, 0, sizeof(Counter));
    if (Lock(inCounter->lock) != LOCK_OK){
        return false;
    }

    memcpy(outData, &(inCounter->counter), sizeof(Counter));
    memset(&(inCounter->counter), 0, sizeof(Counter));

    if (Unlock(inCounter->lock) != LOCK_OK) {
        return false;
    }

    return true;
}

bool AgentTelemetryCounter_IncreaseBy(SyncedCounter* counter, uint32_t* countInstance, uint32_t amount){
        
    if (Lock(counter->lock) != LOCK_OK){
        return false;
    }

    *countInstance += amount;

    if (Unlock(counter->lock) != LOCK_OK) {
        return false;
    }

    return true;
}