// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#include "json/json_object_writer.h"
#include "json/json_array_writer.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(json_writer_with_parson_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    REGISTER_UMOCK_ALIAS_TYPE(JsonWriterResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, unsigned int);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}


TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}


TEST_FUNCTION(JsonWriterWithParson_TestSimpleObject_ExpectSuccess)
{
    JsonObjectWriterHandle writer;
    JsonWriterResult result = JsonObjectWriter_Init(&writer);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteString(writer, "name", "Sherlock Holmes");
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteInt(writer, "street number", 221);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteBool(writer, "mysterious", true);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    char* buffer = NULL;
    uint32_t size = 0;

    result = JsonObjectWriter_Serialize(writer, &buffer, &size);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    const char* expectedValue = "{\"name\":\"Sherlock Holmes\",\"street number\":221,\"mysterious\":true}";
    ASSERT_ARE_EQUAL(char_ptr, expectedValue, buffer);
    ASSERT_ARE_EQUAL(int, strlen(expectedValue), size);

    free(buffer);
    JsonObjectWriter_Deinit(writer);
}

TEST_FUNCTION(JsonWriterWithParson_TestSimpleArrayWithObjects_ExpectSuccess)
{
    JsonObjectWriterHandle objectWriter;
    JsonWriterResult result = JsonObjectWriter_Init(&objectWriter);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteString(objectWriter, "name", "Sherlock");
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    JsonArrayWriterHandle arrayWriter;
    result = JsonArrayWriter_Init(&arrayWriter);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonArrayWriter_AddObject(arrayWriter, objectWriter);

    char* buffer = NULL;
    uint32_t size = 0;

    result = JsonArrayWriter_Serialize(arrayWriter, &buffer, &size);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    const char* expectedValue = "[{\"name\":\"Sherlock\"}]";
    ASSERT_ARE_EQUAL(char_ptr, expectedValue, buffer);
    ASSERT_ARE_EQUAL(int, strlen(expectedValue), size);

    free(buffer);
    JsonArrayWriter_Deinit(arrayWriter);
    JsonObjectWriter_Deinit(objectWriter);
}


TEST_FUNCTION(JsonWriterWithParson_TestObjectWithArrayWithObjects_ExpectSuccess)
{
    JsonObjectWriterHandle innerObjectWriter;
    JsonWriterResult result = JsonObjectWriter_Init(&innerObjectWriter);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteString(innerObjectWriter, "name", "Sherlock");
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    JsonArrayWriterHandle arrayWriter;
    result = JsonArrayWriter_Init(&arrayWriter);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonArrayWriter_AddObject(arrayWriter, innerObjectWriter);

    JsonObjectWriterHandle outerObjectWriter;
    result = JsonObjectWriter_Init(&outerObjectWriter);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteString(outerObjectWriter, "name", "watson");
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);

    result = JsonObjectWriter_WriteArray(outerObjectWriter, "friends", arrayWriter);

    char* buffer = NULL;
    uint32_t size = 0;

    result = JsonObjectWriter_Serialize(outerObjectWriter, &buffer, &size);
    ASSERT_ARE_EQUAL(int, JSON_WRITER_OK, result);
    const char* expectedValue = "{\"name\":\"watson\",\"friends\":[{\"name\":\"Sherlock\"}]}";
    ASSERT_ARE_EQUAL(char_ptr, expectedValue, buffer);
    ASSERT_ARE_EQUAL(int, strlen(expectedValue), size);

    free(buffer);
    JsonArrayWriter_Deinit(arrayWriter);
    JsonObjectWriter_Deinit(innerObjectWriter);
    JsonObjectWriter_Deinit(outerObjectWriter);
}


END_TEST_SUITE(json_writer_with_parson_ut)
