// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_READER_H
#define JSON_READER_H

#include <stdbool.h>

#include "macro_utils.h"
#include "umock_c_prod.h"
#include "parson.h"

#include "json/json_defs.h"

typedef struct _JsonObjectReader {

    JSON_Value* rootValue;
    JSON_Object* rootObject;
    bool shouldFree;

} JsonObjectReader;

typedef struct _JsonArrayReader {
    
    JSON_Array* rootArray;

} JsonArrayReader;

/**
 * @brief Initiates a new json reader the the given JSON_Object.
 *        This function is for internal usage.
 * 
 * @param   handle    Out param. The handle to use for the json reader operations.
 * @param   object    The json object to init this reader from.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_InternalInit, JsonObjectReaderHandle*, handle, JSON_Object*, object);

#endif //JSON_READER_H
