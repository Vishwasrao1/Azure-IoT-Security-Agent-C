// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef USER_LOGIN_COLLECTOR_H
#define USER_LOGIN_COLLECTOR_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief enum all the logins since the last time this function was called, 
 * create a message for each login and insert them into the queue.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, EventCollectorResult, UserLoginCollector_GetEvents, SyncQueue*, queue);

#endif //USER_LOGIN_COLLECTOR_H