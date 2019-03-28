// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_ARRAY_READER_H
#define JSON_ARRAY_READER_H

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <stdint.h>

#include "json/json_defs.h"

/**
 * @brief Initiates a new json reader on the given json.
 * 
 * @param   handle   Out param. The handle to use for the json reader operations.
 * @param   parent   The parent json object reader.
 * @param   name     The name of the array value to initialize this reader from.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonArrayReader_Init, JsonArrayReaderHandle*, handle, JsonObjectReaderHandle, parent, const char*, name);

/**
 * @brief Deinitiate the json reader.
 * 
 * @param   handle  The reader instance to deiniitate.
 */
MOCKABLE_FUNCTION(, void, JsonArrayReader_Deinit, JsonArrayReaderHandle, handle);

/**
 * @brief Returns the number of items in this array.
 * 
 * @param   handle          The reader instance.
 * @param   numOfelements   Out param. On success contains the number of elements in the array.
 * 
 * @return JSON_READER_OK.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonArrayReader_GetSize, JsonArrayReaderHandle, handle, uint32_t*, numOfelements);

/**
 * @brief Returns a json object reader for the item in the given index of this array.
 * 
 * @param   handle          The reader instance.
 * @param   index           The index in the array of the object to read.
 * @param   objectHandle    Out param. On success will contain the handle to the object reader.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonArrayReader_ReadObject, JsonArrayReaderHandle, handle, uint32_t, index, JsonObjectReaderHandle*, objectHandle);

#endif //JSON_ARRAY_READER_H
