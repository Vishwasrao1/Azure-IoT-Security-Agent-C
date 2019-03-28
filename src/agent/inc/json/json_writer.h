// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <stdbool.h>

#include "parson.h"

typedef struct _JsonObjectWriter {

    JSON_Value* rootValue;
    JSON_Object* rootObject;
    bool shouldFree;

} JsonObjectWriter;

typedef struct _JsonArrayWriter {

    JSON_Value* rootValue;
    JSON_Array* rootArray;
    bool shouldFree;

} JsonArrayWriter;

#endif //JSON_WRITER_H