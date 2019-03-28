// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "synchronized_queue.h"

typedef enum _MessageSerializerResultValues {

    MESSAGE_SERIALIZER_OK,
    MESSAGE_SERIALIZER_MEMORY_EXCEEDED,
    MESSAGE_SERIALIZER_EMPTY,
    MESSAGE_SERIALIZER_PARTIAL,
    MESSAGE_SERIALIZER_EXCEPTION
    
} MessageSerializerResultValues;


/**
 * @brief Serialize all messages from the qiven queue.
 * 
 * @param   queues          array of queues to serialize events from, the method empties the queues in an orderd way
 * @param   len             The length of the queues array
 * @param   buffer          Out param. The buffer that will contain the data on success.
 *  
 * @return a buffer which represents tue serialization of the queue. In case of faliure NULL is returned.
 */
MOCKABLE_FUNCTION(, MessageSerializerResultValues, MessageSerializer_CreateSecurityMessage, SyncQueue**, queues, uint32_t, len, void**, buffer);

#endif //MESSAGE_SERIALIZER_H