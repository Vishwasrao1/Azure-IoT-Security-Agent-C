// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/correlation_manager.h"

#include <pthread.h>
#include <stdlib.h>

#include "azure_c_shared_utility/uniqueid.h"

static const char EMPTY_GUID[] = "00000000-0000-0000-0000-000000000000";
static const int UUID_STRING_SIZE = 37;
pthread_key_t correlationIdKey;

void CorrelationId_Deinit(void* correlationId) {
    free(correlationId);
    pthread_setspecific(correlationIdKey, NULL);
}

bool CorrelationManager_Init() {
    return pthread_key_create(&correlationIdKey, CorrelationId_Deinit) == 0;
}

void CorrelationManager_Deinit() {
    char* currentCorrelationId = (char*)pthread_getspecific(correlationIdKey);
    if (currentCorrelationId != NULL) {
        CorrelationId_Deinit(currentCorrelationId);
    }
    pthread_key_delete(correlationIdKey);
}

const char* CorrelationManager_GetCorrelation() {
    const char* correlationId = (char*)pthread_getspecific(correlationIdKey);
    if (correlationId == NULL) {
        return EMPTY_GUID;
     }

     return correlationId;
}

bool CorrelationManager_SetCorrelation() {
    char* correlationId = (char*)malloc(sizeof(char) * UUID_STRING_SIZE);
    memset(correlationId, 0, UUID_STRING_SIZE);
    if (UniqueId_Generate(correlationId, UUID_STRING_SIZE) != UNIQUEID_OK) {
        return false;
    }

    // clearing previously allocated correlations
    char* currentCorrelationId = (char*)pthread_getspecific(correlationIdKey);
    if (currentCorrelationId != NULL) {
        CorrelationId_Deinit(currentCorrelationId);
    }

    pthread_setspecific(correlationIdKey, (void*)correlationId);
    return true;
}