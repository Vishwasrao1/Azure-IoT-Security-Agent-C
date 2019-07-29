// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#define ENABLE_MOCKS
#include "parson_mock.h"
#undef ENABLE_MOCKS

#include "json/json_array_writer.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(json_array_writer_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JSON_Status, int);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}


TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(JsonArrayWriter_Init_ExpectSuccess)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    JsonArrayWriter* writerObj = (JsonArrayWriter*)writer;
    ASSERT_ARE_EQUAL(void_ptr, arrayValuePtr, writerObj->rootValue);
    ASSERT_ARE_EQUAL(void_ptr, arrayPtr,writerObj->rootArray);
    ASSERT_IS_TRUE(writerObj->shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayWriter_Deinit(writer);
}

TEST_FUNCTION(JsonArrayWriter_Init_InitArrayFailed_ExpectFailure)
{
    JsonArrayWriterHandle writer = NULL;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(NULL);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(JsonArrayWriter_GetSize_ExpectSuccess)
{
    JsonArrayWriter writer;
    writer.rootArray = (JSON_Array*)0x1;

    JsonArrayWriterHandle writerHandle = (JsonArrayWriterHandle )&writer;

    STRICT_EXPECTED_CALL(json_array_get_count((JSON_Array*)0x1)).SetReturn(19);
    uint32_t size = 0;
    JsonWriterResult result = JsonArrayWriter_GetSize(writerHandle, &size);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(int, 19, size);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(JsonArrayWriter_Init_GetArrayFailed_ExpectFailure)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(json_value_free(arrayValuePtr));
    JsonWriterResult result = JsonArrayWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(JsonArrayWriter_Deinit_ExpectSuccess)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    STRICT_EXPECTED_CALL(json_value_free(arrayValuePtr));
    JsonArrayWriter_Deinit(writer);
    
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(JsonArrayWriter_AddObject_ExpectSuccess)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    JSON_Value* objectValuePtr = (JSON_Value*)0x20;
    JsonObjectWriter objectWriter;
    objectWriter.rootValue = objectValuePtr;
    objectWriter.shouldFree = true;
    STRICT_EXPECTED_CALL(json_array_append_value(arrayPtr, objectValuePtr)).SetReturn(JSONSuccess);

    result = JsonArrayWriter_AddObject(writer, (JsonObjectWriterHandle)&objectWriter);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_IS_FALSE(objectWriter.shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayWriter_Deinit(writer);
}


TEST_FUNCTION(JsonArrayWriter_AddObjectFailed_ExpectFailure)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    JSON_Value* objectValuePtr = (JSON_Value*)0x20;
    JsonObjectWriter objectWriter;
    objectWriter.rootValue = objectValuePtr;
    objectWriter.shouldFree = true;
    STRICT_EXPECTED_CALL(json_array_append_value(arrayPtr, objectValuePtr)).SetReturn(!JSONSuccess);

    result = JsonArrayWriter_AddObject(writer, (JsonObjectWriterHandle)&objectWriter);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_IS_TRUE(objectWriter.shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayWriter_Deinit(writer);
}

TEST_FUNCTION(JsonArrayWriter_Serialize_ExpectSuccess)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    char* expectedBuffer = "abc";
    
    STRICT_EXPECTED_CALL(json_serialize_to_string(arrayValuePtr)).SetReturn(expectedBuffer);

    char* outBuffer = NULL;
    uint32_t outBufferSize = 0;
    result = JsonArrayWriter_Serialize(writer, &outBuffer, &outBufferSize);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, expectedBuffer, outBuffer);
    ASSERT_ARE_EQUAL(int, strlen(expectedBuffer), outBufferSize);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonArrayWriter_Deinit(writer);
}

TEST_FUNCTION(JsonArrayWriter_SerializeFailed_ExpectFailure)
{
    JsonArrayWriterHandle writer = NULL;
    JSON_Value* arrayValuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_array()).SetReturn(arrayValuePtr);
    JSON_Array* arrayPtr = (JSON_Array*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_array(arrayValuePtr)).SetReturn(arrayPtr);

    JsonWriterResult result = JsonArrayWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    STRICT_EXPECTED_CALL(json_serialize_to_string(arrayValuePtr)).SetReturn(NULL);

    char* outBuffer = NULL;
    uint32_t outBufferSize = 0;
    result = JsonArrayWriter_Serialize(writer, &outBuffer, &outBufferSize);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonArrayWriter_Deinit(writer);
}

END_TEST_SUITE(json_array_writer_ut)
