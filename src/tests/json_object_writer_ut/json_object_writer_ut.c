// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#define ENABLE_MOCKS
#include "parson_mock.h"
#undef ENABLE_MOCKS

#include "json/json_object_writer.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(json_object_writer_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JSON_Status, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
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

TEST_FUNCTION(JsonObjectWriter_Init_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    JsonObjectWriter* writerObj = (JsonObjectWriter*)writer;
    ASSERT_ARE_EQUAL(void_ptr, valuePtr, writerObj->rootValue);
    ASSERT_ARE_EQUAL(void_ptr, objectPtr,writerObj->rootObject);
    ASSERT_IS_TRUE(writerObj->shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_Init_InitObjectFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(NULL);
    
    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_Init_GetObjectFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(json_value_free(valuePtr));

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_InitFromString_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    const char* json = "{\"a\": \"b\"}";
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_parse_string(json)).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_InitFromString(&writer, json);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    JsonObjectWriter* writerObj = (JsonObjectWriter*)writer;
    ASSERT_ARE_EQUAL(void_ptr, valuePtr, writerObj->rootValue);
    ASSERT_ARE_EQUAL(void_ptr, objectPtr,writerObj->rootObject);
    ASSERT_IS_TRUE(writerObj->shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_InitFromString_ParseStringFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    const char* json = "{\"a\": \"b\"}";
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_parse_string(json)).SetReturn(NULL);

    JsonWriterResult result = JsonObjectWriter_InitFromString(&writer, json);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_Deinit_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    STRICT_EXPECTED_CALL(json_value_free(valuePtr));

    JsonObjectWriter_Deinit(writer);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  
}

TEST_FUNCTION(JsonObjectWriter_WriteString_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    const char* value = "value";
    STRICT_EXPECTED_CALL(json_object_set_string(objectPtr, key, value)).SetReturn(JSONSuccess);

    result = JsonObjectWriter_WriteString(writer, key, value);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_WriteStringFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    const char* value = "value";
    STRICT_EXPECTED_CALL(json_object_set_string(objectPtr, key, value)).SetReturn(!JSONSuccess);

    result = JsonObjectWriter_WriteString(writer, key, value);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_WriteInt_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    int value = 3;
    STRICT_EXPECTED_CALL(json_object_set_number(objectPtr, key, value)).SetReturn(JSONSuccess);

    result = JsonObjectWriter_WriteInt(writer, key, value);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_WriteIntFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    int value = 3;
    STRICT_EXPECTED_CALL(json_object_set_number(objectPtr, key, value)).SetReturn(!JSONSuccess);

    result = JsonObjectWriter_WriteInt(writer, key, value);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}
		
TEST_FUNCTION(JsonObjectWriter_WriteBool_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);
    JsonWriterResult result = JsonObjectWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    bool value = true;
    STRICT_EXPECTED_CALL(json_object_set_boolean(objectPtr, key, (int)value)).SetReturn(JSONSuccess);
    result = JsonObjectWriter_WriteBool(writer, key, value);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  
    JsonObjectWriter_Deinit(writer);
}
TEST_FUNCTION(JsonObjectWriter_WriteBoolFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);
    JsonWriterResult result = JsonObjectWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    const char* key = "key";
    bool value = true;
    STRICT_EXPECTED_CALL(json_object_set_boolean(objectPtr, key, (int)value)).SetReturn(!JSONSuccess);
    result = JsonObjectWriter_WriteBool(writer, key, value);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  
    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_WriteArray_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    JSON_Value* arrayValuePtr = (JSON_Value*)0x20;
    JsonArrayWriter arrayWriter;
    arrayWriter.rootValue = arrayValuePtr;
    arrayWriter.shouldFree = true;
    const char* key = "key";
    STRICT_EXPECTED_CALL(json_object_set_value(objectPtr, key, arrayValuePtr)).SetReturn(JSONSuccess);

    result = JsonObjectWriter_WriteArray(writer, key, (JsonArrayWriterHandle)&arrayWriter);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_IS_FALSE(arrayWriter.shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_WriteArrayFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    
    JSON_Value* arrayValuePtr = (JSON_Value*)0x20;
    JsonArrayWriter arrayWriter;
    arrayWriter.rootValue = arrayValuePtr;
    arrayWriter.shouldFree = true;
    const char* key = "key";
    STRICT_EXPECTED_CALL(json_object_set_value(objectPtr, key, arrayValuePtr)).SetReturn(!JSONSuccess);

    result = JsonObjectWriter_WriteArray(writer, key, (JsonArrayWriterHandle)&arrayWriter);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_IS_TRUE(arrayWriter.shouldFree);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_Serialize_ExpectSuccess)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    char* expectedBuffer = "abc";
    
    STRICT_EXPECTED_CALL(json_serialize_to_string(valuePtr)).SetReturn(expectedBuffer);

    char* outBuffer = NULL;
    uint32_t outBufferSize = 0;
    result = JsonObjectWriter_Serialize(writer, &outBuffer, &outBufferSize);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, expectedBuffer, outBuffer);
    ASSERT_ARE_EQUAL(int, strlen(expectedBuffer), outBufferSize);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonObjectWriter_SerializeFailed_ExpectFailure)
{
    JsonObjectWriterHandle writer = NULL;
    JSON_Value* valuePtr = (JSON_Value*)0x1;
    STRICT_EXPECTED_CALL(json_value_init_object()).SetReturn(valuePtr);
    JSON_Object* objectPtr = (JSON_Object*)0x2;
    STRICT_EXPECTED_CALL(json_value_get_object(valuePtr)).SetReturn(objectPtr);

    JsonWriterResult result = JsonObjectWriter_Init(&writer);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    STRICT_EXPECTED_CALL(json_serialize_to_string(valuePtr)).SetReturn(NULL);

    char* outBuffer = NULL;
    uint32_t outBufferSize = 0;
    result = JsonObjectWriter_Serialize(writer, &outBuffer, &outBufferSize);

    ASSERT_ARE_EQUAL(int, JSON_WRITER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());  

    JsonObjectWriter_Deinit(writer);
}

END_TEST_SUITE(json_object_writer_ut)
