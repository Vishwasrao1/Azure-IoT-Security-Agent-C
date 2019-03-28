// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOCAL_USERS_CLLECTOR_H
#define LOCAL_USERS_CLLECTOR_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief enum all local users and return a json which represent them according to the schema.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, LocalUsersCollector_GetEvents, SyncQueue*, queue);

#endif //LOCAL_USERS_CLLECTOR_H