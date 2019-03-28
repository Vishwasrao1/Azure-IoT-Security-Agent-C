// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AGENT_TELEMETRY_COLLECTOR_H
#define AGENT_TELEMETRY_COLLECTOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief get all telemetry based operational events
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, AgentTelemetryCollector_GetEvents, SyncQueue*, queue);

#endif //AGENT_TELEMETRY_COLLECTOR_H