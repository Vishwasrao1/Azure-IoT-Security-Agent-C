// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PROCESS_CEATION_COLLECTOR_H
#define PROCESS_CEATION_COLLECTOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief enum all the processes created since the last time this function was called, 
 * create a message for each process and insert them into the queue.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, ProcessCreationCollector_GetEvents, SyncQueue*, queue);

/**
 * @brief initializes the process event collector
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, ProcessCreationCollector_Init);

/**
 * @brief deinitializes the process event collector
 */
MOCKABLE_FUNCTION(, void, ProcessCreationCollector_Deinit);

#endif //PROCESS_CEATION_COLLECTOR_H