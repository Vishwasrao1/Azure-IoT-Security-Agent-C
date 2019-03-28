// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LISTENING_PORTS_CLLECTOR_H
#define LISTENING_PORTS_CLLECTOR_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief enum all currently tcp\udp open ports.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, ListeningPortCollector_GetEvents, SyncQueue*, queue);

#endif //LISTENING_PORTS_CLLECTOR_H