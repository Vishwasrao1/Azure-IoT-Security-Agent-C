// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CLLECTOR_H
#define CLLECTOR_H

#include "collectors/generic_event.h"
#include "synchronized_queue.h"

/**
 * @brief typedef of generic events collector function.
 * 
 * @param   queue  The queue to insert the mesages to.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */ 
typedef EventCollectorResult (*EventCollectorFunc)( SyncQueue* queue);

#endif //CLLECTOR_H