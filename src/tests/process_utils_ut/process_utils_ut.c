// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_mocks.h"
#undef ENABLE_MOCKS

#include "os_utils/process_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(process_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
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

TEST_FUNCTION(ProccesUtils_Execute_ExpectSuccess)
{
    FILE* mockedStream = (FILE*) 0x1;
    const char* command = "abc def";
    uint32_t dataSize = 56;
    char buffer[56] = "";

    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(mockedStream);
    STRICT_EXPECTED_CALL(clearerr(mockedStream));
    STRICT_EXPECTED_CALL(fread(IGNORED_PTR_ARG, 1, dataSize, mockedStream)).SetReturn(dataSize);
    STRICT_EXPECTED_CALL(ferror(mockedStream)).SetReturn(0);
    STRICT_EXPECTED_CALL(feof(mockedStream)).SetReturn(1);
    STRICT_EXPECTED_CALL(pclose(mockedStream));

    bool result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, 56, dataSize);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    dataSize = 56;
    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(mockedStream);
    STRICT_EXPECTED_CALL(clearerr(mockedStream));
    STRICT_EXPECTED_CALL(fread(IGNORED_PTR_ARG, 1, dataSize, mockedStream)).SetReturn(10);
    STRICT_EXPECTED_CALL(ferror(mockedStream)).SetReturn(0);
    STRICT_EXPECTED_CALL(pclose(mockedStream));

    result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(int, 10, dataSize);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(ProccesUtils_Execute_ExpectFailure)
{
    FILE* mockedStream = (FILE*) 0x1;
    const char* command = "abc def";
    uint32_t dataSize = 56;
    char buffer[56] = "";

    // popen failed
    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(NULL);
    bool result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // fread failed - error is set
    dataSize = 56;
    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(mockedStream);
    STRICT_EXPECTED_CALL(clearerr(mockedStream));
    STRICT_EXPECTED_CALL(fread(IGNORED_PTR_ARG, 1, dataSize, mockedStream)).SetReturn(dataSize);
    STRICT_EXPECTED_CALL(ferror(mockedStream)).SetReturn(1);
    STRICT_EXPECTED_CALL(pclose(mockedStream));
    result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    
    // fread failed - more data to read in file
    dataSize = 56;
    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(mockedStream);
    STRICT_EXPECTED_CALL(clearerr(mockedStream));
    STRICT_EXPECTED_CALL(fread(IGNORED_PTR_ARG, 1, dataSize, mockedStream)).SetReturn(dataSize);
    STRICT_EXPECTED_CALL(ferror(mockedStream)).SetReturn(0);
    STRICT_EXPECTED_CALL(feof(mockedStream)).SetReturn(0);
    STRICT_EXPECTED_CALL(pclose(mockedStream));
    result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    
    // pclose failed - process not found
    dataSize = 56;
    STRICT_EXPECTED_CALL(popen(command, "r")).SetReturn(mockedStream);
    STRICT_EXPECTED_CALL(clearerr(mockedStream));
    STRICT_EXPECTED_CALL(fread(IGNORED_PTR_ARG, 1, dataSize, mockedStream)).SetReturn(10);
    STRICT_EXPECTED_CALL(ferror(mockedStream)).SetReturn(0);
    STRICT_EXPECTED_CALL(pclose(mockedStream)).SetReturn(32512);
    result = ProcessUtils_Execute(command, buffer, &dataSize);
    ASSERT_IS_FALSE(result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(process_utils_ut)
