// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#define ENABLE_MOCKS
#include "os_utils/system_logger.h"
#include "collectors/diagnostic_event_collector.h"
#undef ENABLE_MOCKS

#include "logger.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

bool Mocked_SystemLogger_Init() {
    return true;
}

BEGIN_TEST_SUITE(logger_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(Severity, int);

    REGISTER_GLOBAL_MOCK_HOOK(SystemLogger_Init, Mocked_SystemLogger_Init);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    REGISTER_GLOBAL_MOCK_HOOK(SystemLogger_Init, NULL);
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(Logger_SetMinimumSeverityForSystemLogger_ExpectSuccess)
{
    bool result = false;
    for (int32_t i = 0; i < SEVERITY_MAX; ++i) {
        result = Logger_SetMinimumSeverityForSystemLogger(i);
        ASSERT_IS_TRUE(result);
    }
}

TEST_FUNCTION(Logger_SetMinimumSeverityForSystemLoggerOutOfRange_ExpectFailure)
{
    bool result = false;
    result = Logger_SetMinimumSeverityForSystemLogger(-1);
    ASSERT_IS_FALSE(result);
    result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_MAX);
    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(Logger_SetMinimumSeverityForDiagnosticEvent_ExpectSuccess)
{
    bool result = false;
    for (int32_t i = 0; i < SEVERITY_MAX; ++i) {
        result = Logger_SetMinimumSeverityForDiagnosticEvent(i);
        ASSERT_IS_TRUE(result);
    }
}

TEST_FUNCTION(Logger_SetMinimumSeverityForDiagnosticEventOutOfRange_ExpectFailure)
{
    bool result = false;
    result = Logger_SetMinimumSeverityForDiagnosticEvent(-1);
    ASSERT_IS_FALSE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_MAX);
    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(Logger_SendDiagnostic_SendBoth)
{
    bool result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_DEBUG);
    ASSERT_IS_TRUE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_DEBUG);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(SystemLogger_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(SystemLogger_LogMessage(IGNORED_PTR_ARG, SEVERITY_DEBUG));
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_AddEvent(IGNORED_PTR_ARG, SEVERITY_DEBUG));

    Logger_Debug("Some message %d\n", 0);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(Logger_SendDiagnostic_SendBoth_ValidateSystemLoggerInitialization)
{
    bool result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_DEBUG);
    ASSERT_IS_TRUE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_DEBUG);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(SystemLogger_IsInitialized()).SetReturn(false);
    STRICT_EXPECTED_CALL(SystemLogger_Init());
    STRICT_EXPECTED_CALL(SystemLogger_LogMessage(IGNORED_PTR_ARG, SEVERITY_DEBUG));
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_AddEvent(IGNORED_PTR_ARG, SEVERITY_DEBUG));

    Logger_Debug("Some message %d\n", 0);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(Logger_SendDiagnostic_SendNone)
{
    bool result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_ERROR);
    ASSERT_IS_TRUE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_ERROR);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(SystemLogger_IsInitialized()).SetReturn(true);

    Logger_Information("Some message %d\n", 0);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(Logger_SendDiagnostic_SendToSystemLoggerOnly)
{
    bool result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_INFORMATION);
    ASSERT_IS_TRUE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_ERROR);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(SystemLogger_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(SystemLogger_LogMessage(IGNORED_PTR_ARG, SEVERITY_INFORMATION));

    Logger_Information("Some message %d\n", 0);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(Logger_SendDiagnostic_SendDiagnosticEventOnly)
{
    bool result = Logger_SetMinimumSeverityForSystemLogger(SEVERITY_ERROR);
    ASSERT_IS_TRUE(result);
    result = Logger_SetMinimumSeverityForDiagnosticEvent(SEVERITY_INFORMATION);
    ASSERT_IS_TRUE(result);

    STRICT_EXPECTED_CALL(SystemLogger_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_IsInitialized()).SetReturn(true);
    STRICT_EXPECTED_CALL(DiagnosticEventCollector_AddEvent(IGNORED_PTR_ARG, SEVERITY_INFORMATION));

    Logger_Information("Some message %d\n", 0);

    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(logger_ut)
