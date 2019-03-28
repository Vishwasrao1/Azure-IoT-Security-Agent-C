// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "json/json_array_reader.h"

#include <stdlib.h>

#include "json/json_reader.h"

JsonReaderResult JsonArrayReader_Init(JsonArrayReaderHandle* handle, JsonObjectReaderHandle parent, const char* name) {
    JsonReaderResult result =  JSON_READER_OK;
    JsonObjectReader* objectReader = (JsonObjectReader*)parent;

    if (json_object_dothas_value_of_type(objectReader->rootObject, name, JSONArray) != 1) {
        return JSON_READER_KEY_MISSING;
    }

    JsonArrayReader* reader = malloc(sizeof(JsonObjectReader));
    if (reader == NULL) {
        result = JSON_READER_EXCEPTION;
        goto cleanup;
    }
    memset(reader, 0, sizeof(*reader));
    
    reader->rootArray = json_object_dotget_array(objectReader->rootObject, name);
    if (reader->rootArray == NULL) {
        result = JSON_READER_EXCEPTION;
        goto cleanup;
    }

    *handle = (JsonArrayReaderHandle)reader;

cleanup:
    if (result != JSON_READER_OK) {
        if (reader != NULL) {
            JsonArrayReader_Deinit((JsonArrayReaderHandle)reader);
        }
    }

    return result;
}

void JsonArrayReader_Deinit(JsonArrayReaderHandle handle) {
    JsonArrayReader* reader = (JsonArrayReader*)handle;
    if (reader != NULL) {
        free(reader);
    }
}

JsonReaderResult JsonArrayReader_GetSize(JsonArrayReaderHandle handle, uint32_t* numOfelements) {
    JsonArrayReader* reader = (JsonArrayReader*)handle;
    *numOfelements = json_array_get_count(reader->rootArray);
    return JSON_READER_OK;
}

JsonReaderResult JsonArrayReader_ReadObject(JsonArrayReaderHandle handle, uint32_t index, JsonObjectReaderHandle* objectHandle) {
    JsonArrayReader* reader = (JsonArrayReader*)handle;
    JSON_Object* innerObject = json_array_get_object(reader->rootArray, index);
    if (innerObject == NULL) {
        return JSON_READER_EXCEPTION;
    }
    return JsonObjectReader_InternalInit(objectHandle, innerObject);
}

