// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "json/json_object_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "internal/time_utils.h"
#include "json/json_array_reader.h"
#include "json/json_reader.h"
#include "logger.h"
#include "utils.h"

/**
 * @brief Extract the value of the given key with the given type from the json as JSONValue.
 * 
 * @param   reader  The json reader instance.
 * @param   key     The key to read.
 * @param   output  Out param. The value of the field.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
static JsonReaderResult JsonObjectReader_GetValueOfType(JsonObjectReader* reader, const char* key, JSON_Value_Type type, JSON_Value ** outObject);

/**
 * @brief Initiates a new json reader on the given json.
 * 
 * @param   handle      Out param. The handle to use for the json reader operations.
 * @param   data        can be either a file name or a string containning the json.
 * @param   isString    boolean representing whther the input is a file name or string.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
static JsonReaderResult JsonObjectReader_Init(JsonObjectReaderHandle* handle, const char* data, bool isString);


JsonReaderResult JsonObjectReader_InitFromString(JsonObjectReaderHandle* handle, const char* data) {
    return JsonObjectReader_Init(handle, data, true);
}

JsonReaderResult JsonObjectReader_InitFromFile(JsonObjectReaderHandle* handle, const char* fileName) {
    return JsonObjectReader_Init(handle, fileName, false);
}

void JsonObjectReader_Deinit(JsonObjectReaderHandle handle) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    if (reader != NULL) {
        if (reader->rootValue != NULL && reader->shouldFree) {
            json_value_free(reader->rootValue);
        }
        free(reader);
    }
}

JsonReaderResult JsonObjectReader_StepIn(JsonObjectReaderHandle handle, const char* key) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    JSON_Value* val = NULL;
    JsonReaderResult result = JsonObjectReader_GetValueOfType(reader, key, JSONObject, &val);
    if (result != JSON_READER_OK) {
        return result;
    }

    JSON_Object* newRoot = json_value_get_object(val);
    if (newRoot == NULL) {
        return JSON_READER_EXCEPTION;
    }
    reader->rootObject = newRoot;
    return JSON_READER_OK;
}

JsonReaderResult JsonObjectReader_StepOut(JsonObjectReaderHandle handle) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    JSON_Value* value = json_object_get_wrapping_value(reader->rootObject);
    if (value == NULL) {
        return JSON_READER_EXCEPTION;
    }
    JSON_Value* parentValue = json_value_get_parent(value);
    if (parentValue == NULL) {
        return JSON_READER_EXCEPTION;
    }
    JSON_Object* newRoot = json_value_get_object(parentValue);
    if (newRoot == NULL) {
        return JSON_READER_EXCEPTION;
    }
    reader->rootObject = newRoot;
    return JSON_READER_OK;
}

JsonReaderResult JsonObjectReader_ReadTimeInMilliseconds(JsonObjectReaderHandle handle, const char* key, uint32_t* output) { 
    char* timeValue = NULL;
    JsonReaderResult result = JsonObjectReader_ReadString(handle, key, &timeValue);
    if (result != JSON_READER_OK) {
        return result;
    }
        
    if(TimeUtils_ISO8601DurationStringToMilliseconds(timeValue, output) == false) {
        result = JSON_READER_PARSE_ERROR;
    }

    return result;
}

JsonReaderResult JsonObjectReader_ReadInt(JsonObjectReaderHandle handle, const char* key, int32_t* output) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    JSON_Value* value = NULL;
    
    JsonReaderResult result = JsonObjectReader_GetValueOfType(reader, key, JSONNumber, &value);
    if (result != JSON_READER_OK) {
        return result;
    }

    *output = json_value_get_number(value);
 
    return JSON_READER_OK;
}

JsonReaderResult JsonObjectReader_ReadString(JsonObjectReaderHandle handle, const char* key, char ** output) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    JSON_Value* value = NULL;
    
    JsonReaderResult result = JsonObjectReader_GetValueOfType(reader, key, JSONString, &value);
    if (result != JSON_READER_OK) {
        return result;
    }

    *output = (char*) json_value_get_string(value);
    if (*output == NULL) {
        return JSON_READER_EXCEPTION;
    }

    return JSON_READER_OK;
}

static JsonReaderResult JsonObjectReader_GetValueOfType(JsonObjectReader* reader, const char* key, JSON_Value_Type type, JSON_Value ** outObject){
    if (json_object_dothas_value(reader->rootObject, key) != true){
        return JSON_READER_KEY_MISSING;
    }

    JSON_Value* value = json_object_dotget_value(reader->rootObject, key);
    if (value == NULL) {
        return JSON_READER_EXCEPTION;
    }

    JSON_Value_Type actualType = json_value_get_type(value);
    if (actualType == JSONError) {
        return JSON_READER_EXCEPTION;
    } else if (actualType == JSONNull) {
        return JSON_READER_VALUE_IS_NULL;
    } else if (actualType != type) {
        return JSON_READER_PARSE_ERROR;
    }

    *outObject = value;
    return JSON_READER_OK;
}

static JsonReaderResult JsonObjectReader_Init(JsonObjectReaderHandle* handle, const char* data, bool isString) {
    JsonReaderResult result =  JSON_READER_OK;

    JsonObjectReader* reader = malloc(sizeof(JsonObjectReader));
    if (reader == NULL) {
        result = JSON_READER_EXCEPTION;
        goto cleanup;
    }
    memset(reader, 0, sizeof(JsonObjectReader));
    reader->shouldFree = true;

    if (isString) {
        reader->rootValue = json_parse_string(data);
    } else {
        reader->rootValue = json_parse_file(data);
    }

    if (reader->rootValue == NULL) {
        result = JSON_READER_EXCEPTION;
        goto cleanup;
    }   

    reader->rootObject = json_value_get_object(reader->rootValue);
    if (reader->rootObject == NULL) {
        result = JSON_READER_EXCEPTION;
        goto cleanup;
    }

    *handle = (JsonObjectReaderHandle)reader;

cleanup:
    if (result != JSON_READER_OK) {
        if (reader != NULL) {
            JsonObjectReader_Deinit((JsonObjectReaderHandle)reader);
        }
    }
    return result;
}

JsonReaderResult JsonObjectReader_ReadArray(JsonObjectReaderHandle handle, const char* key, JsonArrayReaderHandle* output) {
    return JsonArrayReader_Init(output, handle, key);
}

JsonReaderResult JsonObjectReader_ReadObject(JsonObjectReaderHandle handle, const char* key, JsonObjectReaderHandle* output) {    
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    if (json_object_dothas_value_of_type(reader->rootObject, key, JSONObject) == 0) {
        return JSON_READER_KEY_MISSING;
    }
    
    JSON_Object* subObject = json_object_dotget_object(reader->rootObject, key);
    if (subObject == NULL) {
        return JSON_READER_EXCEPTION;
    }

    JsonObjectReader* newReader = malloc(sizeof(JsonObjectReader));
    if (newReader == NULL) {
        return JSON_READER_EXCEPTION;
    }
    memset(newReader, 0, sizeof(JsonObjectReader));
    newReader->shouldFree = false;
    newReader->rootObject = subObject;
    *output = (JsonObjectReaderHandle)newReader;

    return JSON_READER_OK;
}

JsonReaderResult JsonObjectReader_ReadBool(JsonObjectReaderHandle handle, const char* key, bool* output) {
    JsonObjectReader* reader = (JsonObjectReader*)handle;
    JSON_Value* value = NULL;

    JsonReaderResult result = JsonObjectReader_GetValueOfType(reader, key, JSONBoolean, &value);
    if (result != JSON_READER_OK) {
        return result;
    }

    *output = json_value_get_boolean(value);
    return JSON_READER_OK;
}