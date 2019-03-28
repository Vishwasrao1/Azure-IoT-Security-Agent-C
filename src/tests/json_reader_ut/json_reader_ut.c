// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"
#include "umock_c.h"

#include "json/json_reader.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(json_reader_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);
    REGISTER_UMOCK_ALIAS_TYPE(JsonReaderResult, int);
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

TEST_FUNCTION(JsonObjectReader_InternalInit_ExpectSuccess)
{
    JsonObjectReaderHandle objectReaderHandle = NULL;
    JSON_Object* mockedObject = (JSON_Object*)0x1;
    
    JsonReaderResult result = JsonObjectReader_InternalInit(&objectReaderHandle, mockedObject);
    ASSERT_ARE_EQUAL(int, JSON_READER_OK, result);

    JsonObjectReader* reader = (JsonObjectReader*)objectReaderHandle;
    ASSERT_ARE_EQUAL(void_ptr, 0x1, reader->rootObject);
    ASSERT_IS_FALSE(reader->shouldFree);

    free(reader);
}

END_TEST_SUITE(json_reader_ut)
