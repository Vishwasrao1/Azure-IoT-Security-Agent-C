// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "json/json_array_writer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "json/json_writer.h"

JsonWriterResult JsonArrayWriter_Init(JsonArrayWriterHandle* writer) {
    JsonWriterResult result = JSON_WRITER_OK;

    JsonArrayWriter* writerObj = malloc(sizeof(JsonArrayWriter));
    if (writerObj == NULL) {
        result = JSON_WRITER_EXCEPTION;
        goto cleanup;
    }
    memset(writerObj, 0, sizeof(JsonArrayWriter));
    writerObj->shouldFree = true;

    writerObj->rootValue = json_value_init_array();
    if (writerObj->rootValue == NULL) {
        result = JSON_WRITER_EXCEPTION;
        goto cleanup;
    }

    writerObj->rootArray = json_value_get_array(writerObj->rootValue);
    if (writerObj->rootArray == NULL) {
        result = JSON_WRITER_EXCEPTION;
        goto cleanup;
    }

    *writer = (JsonArrayWriterHandle)writerObj;

cleanup:
    if (result != JSON_WRITER_OK) {
        if (writerObj != NULL) {
            JsonArrayWriter_Deinit((JsonArrayWriterHandle)writerObj);
        }
    }
    return result;
}

void JsonArrayWriter_Deinit(JsonArrayWriterHandle writer) {
    JsonArrayWriter* writerObj = (JsonArrayWriter*)writer;
     if (writerObj != NULL) {
        if (writerObj->shouldFree) {
            if (writerObj->rootValue != NULL) {
                json_value_free(writerObj->rootValue);
            }
        }
        free(writerObj);
    }
}

JsonWriterResult JsonArrayWriter_AddObject(JsonArrayWriterHandle writer, JsonObjectWriterHandle item) {
    JsonArrayWriter* arrayWriter = (JsonArrayWriter*)writer;
    JsonObjectWriter* objectWriter = (JsonObjectWriter*)item;

    if (json_array_append_value(arrayWriter->rootArray, objectWriter->rootValue) != JSONSuccess) {
        return JSON_WRITER_EXCEPTION;
    }

    objectWriter->shouldFree = false;   
    return JSON_WRITER_OK;
}

JsonWriterResult JsonArrayWriter_Serialize(JsonArrayWriterHandle writer, char** output, uint32_t* size) {
    JsonArrayWriter* writerObj = (JsonArrayWriter*)writer;

    *output = json_serialize_to_string(writerObj->rootValue);
    if (*output == NULL) {
        return JSON_WRITER_EXCEPTION;
    }
    *size = strlen(*output);

    return JSON_WRITER_OK;
}

JsonWriterResult JsonArrayWriter_GetSize(JsonArrayWriterHandle handle, uint32_t* numOfelements) {
    JsonArrayWriter* writer = (JsonArrayWriter*)handle;
    *numOfelements = json_array_get_count(writer->rootArray);
    return JSON_WRITER_OK;
}