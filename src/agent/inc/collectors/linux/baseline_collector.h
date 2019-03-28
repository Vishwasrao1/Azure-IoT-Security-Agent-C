// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef BASELINE_COLLECTOR_H
#define BASELINE_COLLECTOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief run baseline rules and add to the queue all events that failed.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, BaselineCollector_GetEvents, SyncQueue*, queue);

#endif //BASELINE_COLLECTOR_H