// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_OBJECT_READER_H
#define JSON_OBJECT_READER_H

#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "json/json_defs.h"

/**
 * @brief Initiates a new json reader on the given json.
 * 
 * @param   handle  Out param. The handle to use for the json reader operations.
 * @param   data    The json the reader should use.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_InitFromString, JsonObjectReaderHandle*, handle, const char*, data);

/**
 * @brief Initiates a new json reader on the given json.
 * 
 * @param   handle      Out param. The handle to use for the json reader operations.
 * @param   fileName    The json file name.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_InitFromFile, JsonObjectReaderHandle*, handle, const char*, fileName);

/**
 * @brief Deinitiate the json reader.
 * 
 * @param   handle  The reader instance to deiniitate.
 */
MOCKABLE_FUNCTION(, void, JsonObjectReader_Deinit, JsonObjectReaderHandle, handle);

/**
 * @brief Sets the new root of the json reader to be the given key. The operation is unreversible.
 * 
 * @param   handle  Out param. The handle to use for the json reader operations.
 * @param   data    The json the reader should use.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure. The state of the reader will be the same in case of error.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_StepIn, JsonObjectReaderHandle, handle, const char*, key);

/**
 * @brief Reads the given key from the json as integer.
 * 
 * @param   handle  The json reader instance.
 * @param   key     The key to read.
 * @param   output  Out param. The integer value of the field.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_ReadInt, JsonObjectReaderHandle, handle, const char*, key, int32_t*, output);

/**
 * @brief Reads the given key from the json as string.
 * 
 * @param   handle  The json reader instance.
 * @param   key     The key to read.
 * @param   output  Out param. The string value of the field.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_ReadString, JsonObjectReaderHandle, handle, const char*, key, char**, output);

/**
 * @brief Reads the given key from the json as time (format of hh:mm:ss).
 * 
 * @param   handle  The json reader instance.
 * @param   key     The key to read.
 * @param   output  Out param. The time value of the field.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_ReadTimeInMilliseconds, JsonObjectReaderHandle, handle, const char*, key, uint32_t*, output);

/**
 * @brief Reads the given key from the json and return an array reader.
 * 
 * @param   handle       The json reader instance.
 * @param   key          The key to read.
 * @param   output       Out param. An array reader to use in order to read the wanted array.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_ReadArray, JsonObjectReaderHandle, handle, const char*, key, JsonArrayReaderHandle*, output);

/**
 * @brief Reads the given key from the json and return a json object reader.
 * 
 * @param   handle       The json reader instance.
 * @param   key          The key to read.
 * @param   output       Out param. An object reader to use.
 * 
 * @return JSON_READER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonReaderResult, JsonObjectReader_ReadObject, JsonObjectReaderHandle, handle, const char*, key, JsonObjectReaderHandle*, output);

#endif //JSON_OBJECT_READER_H