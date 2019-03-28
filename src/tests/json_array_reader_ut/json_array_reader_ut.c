// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#define ENABLE_MOCKS
#include "json/json_reader.h"
#include "parson_mock.h"
#undef ENABLE_MOCKS

#include "json/json_array_reader.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(json_array_reader_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
    REGISTER_UMOCK_ALIAS_TYPE(JSON_Value_Type, int);
    REGISTER_UMOCK_ALIAS_TYPE(JSON_Status, int);
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

TEST_FUNCTION(JsonArrayReader_Init_ExpectSuccess)
{
    JsonObjectReader objectReader;
    objectReader.rootObject = (JSON_Object*)0x1;
    JsonObjectReaderHandle objectReaderHandle = (JsonObjectReaderHandle)&objectReader;
    
    JsonArrayReaderHandle readerHandle = NULL;
    
    STRICT_EXPECTED_CALL(json_object_dothas_value_of_type((const JSON_Object*)0x1, "items", JSONArray)).SetReturn(1);
    STRICT_EXPECTED_CALL(json_object_dotget_array((const JSON_Object*)0x1, "items")).SetReturn((JSON_Array*)0x2);
    JsonReaderResult result = JsonArrayReader_Init(&readerHandle, objectReaderHandle, "items");
    ASSERT_ARE_EQUAL(int, JSON_READER_OK, result);
    JsonArrayReader* reader = (JsonArrayReader*)readerHandle;
    ASSERT_ARE_EQUAL(void_ptr, 0x2, reader->rootArray);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayReader_Deinit(readerHandle);
}

TEST_FUNCTION(JsonArrayReader_Init_TypeMismatch_ExpectSuccess)
{
    JsonObjectReader objectReader;
    objectReader.rootObject = (JSON_Object*)0x1;
    JsonObjectReaderHandle objectReaderHandle = (JsonObjectReaderHandle)&objectReader;
    
    JsonArrayReaderHandle readerHandle = NULL;
    STRICT_EXPECTED_CALL(json_object_dothas_value_of_type((const JSON_Object*)0x1, "items", JSONArray)).SetReturn(0);
    JsonReaderResult result = JsonArrayReader_Init(&readerHandle, objectReaderHandle, "items");
    ASSERT_ARE_EQUAL(int, JSON_READER_KEY_MISSING, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayReader_Deinit(readerHandle);
}

TEST_FUNCTION(JsonArrayReader_Init_GetArrayFailed_ExpectFailure)
{
    JsonObjectReader objectReader;
    objectReader.rootObject = (JSON_Object*)0x1;
    JsonObjectReaderHandle objectReaderHandle = (JsonObjectReaderHandle)&objectReader;
    
    JsonArrayReaderHandle readerHandle = NULL;
    STRICT_EXPECTED_CALL(json_object_dothas_value_of_type((const JSON_Object*)0x1, "items", JSONArray)).SetReturn(1);
    STRICT_EXPECTED_CALL(json_object_dotget_array((const JSON_Object*)0x1, "items")).SetReturn(NULL);
    JsonReaderResult result = JsonArrayReader_Init(&readerHandle, objectReaderHandle, "items");
    ASSERT_ARE_EQUAL(int, JSON_READER_EXCEPTION, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    JsonArrayReader_Deinit(readerHandle);
}

TEST_FUNCTION(JsonArrayReader_GetSize_ExpectSuccess)
{
    JsonArrayReader reader;
    reader.rootArray = (JSON_Array*)0x1;
    JsonArrayReaderHandle readerHandle = (JsonArrayReaderHandle)&reader;

    STRICT_EXPECTED_CALL(json_array_get_count((JSON_Array*)0x1)).SetReturn(19);
    uint32_t size = 0;
    JsonReaderResult result = JsonArrayReader_GetSize(readerHandle, &size);
    ASSERT_ARE_EQUAL(int, JSON_READER_OK, result);
    ASSERT_ARE_EQUAL(int, 19, size);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(JsonArrayReader_GetObject_ExpectSuccess)
{
    JsonArrayReader reader;
    reader.rootArray = (JSON_Array*)0x1;
    JsonArrayReaderHandle readerHandle = (JsonArrayReaderHandle)&reader;

    JSON_Object* mockedInnerOject = (JSON_Object*)0x2;
    JsonObjectReaderHandle objctReaderHandle = NULL;
    STRICT_EXPECTED_CALL(json_array_get_object((JSON_Array*)0x1, 4)).SetReturn(mockedInnerOject);
    STRICT_EXPECTED_CALL(JsonObjectReader_InternalInit(&objctReaderHandle, mockedInnerOject)).SetReturn(JSON_READER_OK);
    
    JsonReaderResult result = JsonArrayReader_ReadObject(readerHandle, 4, &objctReaderHandle);
    ASSERT_ARE_EQUAL(int, JSON_READER_OK, result);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(json_array_reader_ut)
