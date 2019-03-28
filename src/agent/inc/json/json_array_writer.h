// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_ARRAY_WRITER_H
#define JSON_ARRAY_WRITER_H

#include <stdbool.h>
#include <stdint.h>

#include "umock_c_prod.h"
#include "macro_utils.h"

#include "json/json_defs.h"

/**
 * @brief Initiates a new json array writer on the given json.
 * 
 * @param   writer  Out param. The handle to use for the json writer operations.
 * 
 * @return JSON_WRITER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonWriterResult, JsonArrayWriter_Init, JsonArrayWriterHandle*, writer);

/**
 * @brief Deinitiate the json array writer.
 * 
 * @param   writer  The reader instance to deiniitate.
 */
MOCKABLE_FUNCTION(, void, JsonArrayWriter_Deinit, JsonArrayWriterHandle, writer);

/**
 * @brief Adds a new item to this array.
 * 
 * @param   writer  The json array writer instance.
 * @param   item    The new item to add.
 * 
 * @return JSON_WRITER_OK on success, an indicative error in failure.
 */
MOCKABLE_FUNCTION(, JsonWriterResult, JsonArrayWriter_AddObject, JsonArrayWriterHandle, writer, JsonObjectWriterHandle, item);

/**
 * @brief Serialize the given array to char*.
 * 
 * @param   writer  The json writer instance.
 * @param   output  Out param. The serialized object.
 * @param   size    Out param. The size of the serialized output.
 * 
 * @return JSON_WRITER_OK on success, an indicative error in failure. 
 */
MOCKABLE_FUNCTION(, JsonWriterResult, JsonArrayWriter_Serialize, JsonArrayWriterHandle, writer, char**, output, uint32_t*, size);

/**
 * @brief Returns the number of items in this array.
 * 
 * @param   handle          The writer instance.
 * @param   numOfelements   Out param. On success contains the number of elements in the array.
 * 
 * @return JSON_WRITER_OK.
 */
MOCKABLE_FUNCTION(, JsonWriterResult, JsonArrayWriter_GetSize, JsonArrayWriterHandle, handle, uint32_t*, numOfelements);

#endif //JSON_ARRAY_WRITER_H