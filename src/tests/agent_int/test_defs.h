// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_CONSTS_H
#define TEST_CONSTS_H

#include <stdlib.h>

#include "azure_c_shared_utility/lock.h"

extern const char* currentTwinConfiguration;

extern uint32_t processCreateMessageCounter;
extern uint32_t listeningPortsMessageCounter;

typedef struct _TestMessage {

    uint32_t dataSize;
    char* data;

} TestMessage;

typedef struct _TestMessageQueue {
    
    uint32_t index;
    LOCK_HANDLE lock;
    TestMessage items[50];

} TestMessageQueue;

TestMessageQueue sentMessages;

#endif //TEST_CONSTS_H