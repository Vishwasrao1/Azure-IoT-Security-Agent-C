// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "std_mocks.h"
#undef ENABLE_MOCKS

#include <string.h>
#include <errno.h>
#include "os_utils/file_utils.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

static const int WRITE_CREATE_TRUNC_FLAG = 1 | 64 | 512;
static const int READ_FLAG = 0;
static const char* FILE_PATH = "aaa/bbb/ccc/d.txt";
static const char* OPEN_MODE = "rb";


BEGIN_TEST_SUITE(file_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(FileResults, int);
    REGISTER_UMOCK_ALIAS_TYPE(ssize_t, long long);

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

TEST_FUNCTION(FileUtils_WriteToFile_ExpectSuccess)
{
    const char* data = "123asd13";
    int dataLen = strlen(data);

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, WRITE_CREATE_TRUNC_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(write(mockedFd, data, dataLen)).SetReturn(dataLen);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_WriteToFile(FILE_PATH, data, dataLen);
    ASSERT_ARE_EQUAL(int, FILE_UTILS_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_WriteToFile_OpenFailed_ExpectFailure)
{
    const char* data = "123asd13";
    int dataLen = strlen(data);

    STRICT_EXPECTED_CALL(open(FILE_PATH, WRITE_CREATE_TRUNC_FLAG)).SetReturn(-1);

    FileResults result = FileUtils_WriteToFile(FILE_PATH, data, dataLen);
    ASSERT_ARE_EQUAL(int, FILE_UTILS_ERROR, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_WriteToFile_WriteFailed_ExpectFailure)
{
    const char* data = "123asd13";
    int dataLen = strlen(data);

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, WRITE_CREATE_TRUNC_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(write(mockedFd, data, dataLen)).SetReturn(-1);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_WriteToFile(FILE_PATH, data, dataLen);
    ASSERT_ARE_EQUAL(int, FILE_UTILS_ERROR, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_WriteToFile_WritePartial_ExpectFailure)
{
    const char* data = "123asd13";
    int dataLen = strlen(data);

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, WRITE_CREATE_TRUNC_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(write(mockedFd, data, dataLen)).SetReturn(dataLen - 1);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_WriteToFile(FILE_PATH, data, dataLen);
    ASSERT_ARE_EQUAL(int, FILE_UTILS_SIZE_MISMATCH, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_ReadFile_ExpectSuccess)
{
    char* data = NULL;
    int dataLen = 10;

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, READ_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(read(mockedFd, data, dataLen)).SetReturn(dataLen);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_ReadFile(FILE_PATH, data, dataLen, false);
    ASSERT_ARE_EQUAL(int, FILE_UTILS_OK, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_ReadFile_PathDoesNotExist_ExpectFailure)
{
    char* data = NULL;
    int dataLen = 10;

    STRICT_EXPECTED_CALL(open(FILE_PATH, READ_FLAG)).SetReturn(-1);

    errno = ENOENT;
    FileResults result = FileUtils_ReadFile(FILE_PATH, data, dataLen, false);
    errno = 0;

    ASSERT_ARE_EQUAL(int, FILE_UTILS_FILE_NOT_FOUND, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_ReadFile_Error_ExpectFailure)
{
    char* data = NULL;
    int dataLen = 10;

    STRICT_EXPECTED_CALL(open(FILE_PATH, READ_FLAG)).SetReturn(-1);

    errno = !ENOENT;
    FileResults result = FileUtils_ReadFile(FILE_PATH, data, dataLen, false);
    errno = 0;

    ASSERT_ARE_EQUAL(int, FILE_UTILS_ERROR, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_ReadFile_ReadFailed_ExpectFailure)
{
    char* data = NULL;
    int dataLen = 10;

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, READ_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(read(mockedFd, data, dataLen)).SetReturn(-1);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_ReadFile(FILE_PATH, data, dataLen, false);

    ASSERT_ARE_EQUAL(int, FILE_UTILS_ERROR, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_ReadFile_ReadPartial_ExpectFailure)
{
    char* data = NULL;
    int dataLen = 10;

    int mockedFd = 1;
    STRICT_EXPECTED_CALL(open(FILE_PATH, READ_FLAG)).SetReturn(mockedFd);
    STRICT_EXPECTED_CALL(read(mockedFd, data, dataLen)).SetReturn(dataLen - 1);
    STRICT_EXPECTED_CALL(close(mockedFd));

    FileResults result = FileUtils_ReadFile(FILE_PATH, data, dataLen, false);

    ASSERT_ARE_EQUAL(int, FILE_UTILS_SIZE_MISMATCH, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_OpenFile_ExpecteSuccess)
{
    FILE* expectedFile = (FILE*)0x10;
    STRICT_EXPECTED_CALL(fopen(FILE_PATH, OPEN_MODE)).SetReturn(expectedFile);
    
    FILE* actualFile; 
    FileResults result = FileUtils_OpenFile(FILE_PATH, OPEN_MODE, &actualFile);

    ASSERT_ARE_EQUAL(int, FILE_UTILS_OK, result);
    ASSERT_IS_TRUE(actualFile == expectedFile);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}


TEST_FUNCTION(FileUtils_OpenFile_FileNotExist_ExpectFail)
{
    STRICT_EXPECTED_CALL(fopen(FILE_PATH, OPEN_MODE)).SetReturn(NULL);
    
    FILE* actualFile; 
    errno = ENOENT;
    FileResults result = FileUtils_OpenFile(FILE_PATH, OPEN_MODE, &actualFile);

    ASSERT_ARE_EQUAL(int, FILE_UTILS_FILE_NOT_FOUND, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(FileUtils_OpenFile_FileNotAccesible_ExpectFail)
{
    STRICT_EXPECTED_CALL(fopen(FILE_PATH, OPEN_MODE)).SetReturn(NULL);
    
    FILE* actualFile; 
    errno = EACCES;
    FileResults result = FileUtils_OpenFile(FILE_PATH, OPEN_MODE, &actualFile);

    ASSERT_ARE_EQUAL(int, FILE_UTILS_NOPERM, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(file_utils_ut)
