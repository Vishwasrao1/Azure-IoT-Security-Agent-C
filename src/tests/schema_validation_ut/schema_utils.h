// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "synchronized_queue.h"

typedef enum _SchemaValidationResult {
    SCHEMA_VALIDATION_OK,
    SCHEMA_VALIDATION_ERROR
} SchemaValidationResult;

SchemaValidationResult validate_schema(SyncQueue* eventQueue);