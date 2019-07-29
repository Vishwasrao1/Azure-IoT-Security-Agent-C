// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_DEFS_H
#define JSON_DEFS_H

typedef enum _JsonWriterResult {
    JSON_WRITER_OK,
    JSON_WRITER_EXCEPTION
} JsonWriterResult;

typedef struct JsonObjectWriter* JsonObjectWriterHandle;
typedef struct JsonArrayWriter* JsonArrayWriterHandle;

typedef enum _JsonReaderResult {
    JSON_READER_OK,
    JSON_READER_KEY_MISSING,
    JSON_READER_VALUE_IS_NULL,
    JSON_READER_PARSE_ERROR,
    JSON_READER_EXCEPTION
} JsonReaderResult;

typedef struct JsonObjectReader* JsonObjectReaderHandle;
typedef struct JsonArrayReader* JsonArrayReaderHandle;


#endif //JSON_DEFS_H