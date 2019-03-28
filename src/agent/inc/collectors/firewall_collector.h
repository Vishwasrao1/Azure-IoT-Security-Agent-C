// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef FIREWALL_COLLECTOR_H
#define FIREWALL_COLLECTOR_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief enum all firewall rules and add them as a message to the queue.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, FirewallCollector_GetEvents, SyncQueue*, queue);

#endif //FIREWALL_COLLECTOR_H