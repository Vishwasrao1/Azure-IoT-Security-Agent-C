// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "json/json_reader.h"

#include <stdlib.h>

JsonReaderResult JsonObjectReader_InternalInit(JsonObjectReaderHandle* handle, JSON_Object* object) {
    JsonObjectReader* reader = malloc(sizeof(JsonObjectReader));
    if (reader == NULL) {
        return JSON_READER_EXCEPTION;
    }
    memset(reader, 0, sizeof(JsonObjectReader));
    reader->shouldFree = false;
    reader->rootObject = object;
    *handle = (JsonObjectReaderHandle)reader;

    return JSON_READER_OK;
}
